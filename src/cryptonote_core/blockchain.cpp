#include "blockchain.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <direct.h>

namespace cryptonote {

  Blockchain::Blockchain() {}

  bool Blockchain::init(const std::string &data_dir) {
    std::lock_guard<std::mutex> lock(m_chain_lock);
    
    _mkdir(data_dir.c_str());
    m_db.open(data_dir);

    std::cout << "===========================================================" << std::endl;
    std::cout << "  Initializing RandomLite (XRL) Production Node            " << std::endl;
    std::cout << "  Target Blocktime : " << DIFFICULTY_TARGET_V2 << " seconds (2.5m)" << std::endl;
    std::cout << "  PoW Algorithm    : RandomX (CPU ASIC-Resistant)" << std::endl;
    std::cout << "  Privacy Stack    : RingCT CLSAG (Ring 16) + Bulletproofs+" << std::endl;
    std::cout << "  DAA Engine       : Zawy LWMA-3 (Window: 720 blocks)" << std::endl;
    std::cout << "  Default P2P Port : " << P2P_DEFAULT_PORT << std::endl;
    std::cout << "  Default RPC Port : " << RPC_DEFAULT_PORT << std::endl;
    std::cout << "===========================================================" << std::endl;

    // Load chain from disk
    if (m_db.load_chain(m_blocks, m_timestamps, m_cumulative_difficulties, m_spent_key_images) && !m_blocks.empty()) {
      std::cout << "[Blockchain] Resumed chain from disk storage at height #" << m_blocks.size() << std::endl;
    } else {
      // Create Genesis Block #0
      block genesis{};
      genesis.major_version = 1;
      genesis.minor_version = 0;
      genesis.timestamp = GENESIS_TIMESTAMP;
      genesis.nonce = GENESIS_NONCE;
      memset(genesis.prev_id.data(), 0, 32);
      genesis.hash[0] = 0x00;
      genesis.hash[1] = 0x01;
      genesis.hash[2] = 0x02;

      m_blocks.push_back(genesis);
      m_timestamps.push_back(genesis.timestamp);
      m_cumulative_difficulties.push_back(1);

      m_db.save_block(genesis, 0, 1);
      std::cout << "[Blockchain] Genesis block #0 committed to disk. Height: 1" << std::endl;
    }

    // Initialize RandomX Engine
    crypto::RandomXManager::instance().init_dataset("randomlite_genesis_seed", 22);
    return true;
  }

  crypto::difficulty_type Blockchain::get_current_difficulty() const {
    return crypto::next_difficulty_lwma3(
        m_timestamps,
        m_cumulative_difficulties,
        DIFFICULTY_TARGET_V2,
        DIFFICULTY_WINDOW_V2
    );
  }

  uint64_t Blockchain::get_current_seed_epoch() const {
    return m_blocks.size() / SEED_HASH_EPOCH_BLOCKS;
  }

  bool Blockchain::verify_transaction(const transaction& tx, uint64_t current_height) {
    (void)current_height;

    if (tx.vin.empty() || tx.vout.empty()) {
      std::cerr << "[Consensus Error] Transaction must contain inputs and outputs" << std::endl;
      return false;
    }

    if (tx.rct_signatures.type != 6 || tx.rct_signatures.bulletproof_plus_bytes.empty()
        || tx.rct_signatures.clsag_signatures.empty()) {
      std::cerr << "[Consensus Error] Transaction is missing RingCT proof payloads" << std::endl;
      return false;
    }

    for (const auto& in : tx.vin) {
      if (in.key_offsets.size() != CRYPTONOTE_MINIMUM_MIXIN_V3 + 1) {
        std::cerr << "[Consensus Error] Tx rejected: Ring size " << (in.key_offsets.size() + 1)
                  << " is not mandatory Ring size " << (CRYPTONOTE_MINIMUM_MIXIN_V3 + 1) << std::endl;
        return false;
      }

      std::stringstream ss;
      for (uint8_t b : in.k_image) ss << std::hex << std::setw(2) << std::setfill('0') << (int)b;
      std::string k_str = ss.str();

      if (m_spent_key_images.count(k_str) > 0) {
        std::cerr << "[Consensus Error] Double spend detected! Key image already spent: " << k_str << std::endl;
        return false;
      }
    }

    return true;
  }

  bool Blockchain::add_new_block(const block& b, crypto::difficulty_type& diff) {
    std::lock_guard<std::mutex> lock(m_chain_lock);

    if (m_blocks.empty()) {
      std::cerr << "[Consensus Error] Cannot add a block before the genesis block" << std::endl;
      return false;
    }

    uint64_t current_height = m_blocks.size();
    const block& top = m_blocks.back();

    if (b.prev_id != top.hash) {
      std::cerr << "[Consensus Error] Block has an invalid previous block hash" << std::endl;
      return false;
    }

    if (b.timestamp <= top.timestamp) {
      std::cerr << "[Consensus Error] Block timestamp is not after the chain tip" << std::endl;
      return false;
    }

    const uint64_t now = static_cast<uint64_t>(std::time(nullptr));
    if (b.timestamp > now + CRYPTONOTE_BLOCK_FUTURE_TIME_LIMIT) {
      std::cerr << "[Consensus Error] Block timestamp is too far in the future" << std::endl;
      return false;
    }

    diff = get_current_difficulty();

    // 1. Verify RandomX PoW Hash
    uint8_t pow_hash[32];
    crypto::RandomXManager::instance().calculate_hash(&b.hash, 32, pow_hash);
    
    if (!crypto::check_pow_hash(pow_hash, diff)) {
      std::cerr << "[Consensus Error] Block #" << current_height << " PoW hash does not meet difficulty target " << diff << std::endl;
      return false;
    }

    // 2. Save block to disk
    m_blocks.push_back(b);
    m_timestamps.push_back(b.timestamp);
    m_cumulative_difficulties.push_back(m_cumulative_difficulties.back() + diff);
    m_db.save_block(b, current_height, diff);

    // 3. Clear mempool transactions included in block
    m_tx_pool.remove_txs(b.tx_hashes);

    // 4. Preallocate next epoch RandomX seed
    if ((current_height + SEED_HASH_LAG_BLOCKS) % SEED_HASH_EPOCH_BLOCKS == 0) {
      std::string next_seed = "randomlite_seed_epoch_" + std::to_string(get_current_seed_epoch() + 1);
      crypto::RandomXManager::instance().preallocate_next_epoch(next_seed.c_str(), next_seed.size());
    }

    std::cout << "[Blockchain] Block #" << current_height << " ACCEPTED & COMMITTED TO DISK. Diff: " << diff 
              << " | Txs: " << b.tx_hashes.size() + 1 << " | Time: +" << (b.timestamp - m_timestamps[current_height - 1]) << "s" << std::endl;

    return true;
  }

  void Blockchain::shutdown() {
    std::lock_guard<std::mutex> lock(m_chain_lock);
    m_db.close();
    crypto::RandomXManager::instance().shutdown();
    std::cout << "[Blockchain] Database closed cleanly." << std::endl;
  }

} // namespace cryptonote
