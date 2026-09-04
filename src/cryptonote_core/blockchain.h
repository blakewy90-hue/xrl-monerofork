#pragma once

#include "cryptonote_basic.h"
#include "tx_pool.h"
#include "../crypto/difficulty.h"
#include "../crypto/randomx-monero.h"
#include "../blockchain_db/lmdb_storage.h"
#include <unordered_set>
#include <map>
#include <vector>
#include <mutex>

namespace cryptonote {

  class Blockchain {
  public:
    Blockchain();
    ~Blockchain() = default;

    // Initialize chain state & load disk DB
    bool init(const std::string &data_dir = "./data");

    // Process & Add new block to main chain tip
    bool add_new_block(const block& b, crypto::difficulty_type& diff);

    // Validate Transaction (Stealth address, spent key images, Ring size 16, Bulletproofs+)
    bool verify_transaction(const transaction& tx, uint64_t current_height);

    // Mempool interface
    TxMemoryPool& get_tx_pool() { return m_tx_pool; }

    // Get current tip height & difficulty
    uint64_t get_current_height() const { return m_blocks.size(); }
    crypto::difficulty_type get_current_difficulty() const;
    block get_top_block() const { return m_blocks.back(); }

    // Seed Epoch calculations
    uint64_t get_current_seed_epoch() const;

    // Save and shutdown DB
    void shutdown();

  private:
    std::vector<block> m_blocks;
    std::vector<uint64_t> m_timestamps;
    std::vector<crypto::difficulty_type> m_cumulative_difficulties;
    
    // Mempool
    TxMemoryPool m_tx_pool;

    // LMDB Persistence
    blockchain_db::PersistentDatabase m_db;
    std::unordered_set<std::string> m_spent_key_images;

    mutable std::mutex m_chain_lock;
  };

} // namespace cryptonote
