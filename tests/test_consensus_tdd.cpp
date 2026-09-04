// =================================================================
//  RandomLite (XRL) TDD Integration Test Suite
//  Tests:
//    1. 100-Block Epoch Difficulty Retargeting Anchor
//    2. 4-Miner Concurrent Mining & Target Blocktime
//    3. Network Stealth Tx, RingCT 16, Mempool, Block Template
// =================================================================

#include "../src/cryptonote_config.h"
#include "../src/crypto/difficulty.h"
#include "../src/crypto/randomx-monero.h"
#include "../src/cryptonote_core/blockchain.h"
#include "../src/cryptonote_core/tx_pool.h"
#include "../src/wallet/wallet.h"
#include <iostream>
#include <vector>
#include <cassert>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>

// ─── Helpers ────────────────────────────────────────────────────
static int tests_passed = 0;
static int tests_total  = 0;

#define TEST_ASSERT(cond, msg)                                       \
  do {                                                               \
    ++tests_total;                                                   \
    if (!(cond)) {                                                   \
      std::cerr << "  FAIL: " << (msg) << std::endl;                \
      return false;                                                  \
    }                                                                \
    ++tests_passed;                                                  \
  } while (0)

// ─── TEST 1: 100-Block Epoch Retargeting ────────────────────────
bool test_100_block_epoch_retarget() {
  std::cout << "\n[TEST 1/3] 100-Block Epoch Difficulty Retargeting Anchor" << std::endl;

  const uint64_t target = DIFFICULTY_TARGET_V2;    // 150 s

  // --- Sub-test A: Blocks arriving 3x too FAST → difficulty must INCREASE ---
  {
    std::vector<uint64_t>                 ts;
    std::vector<crypto::difficulty_type>  cd;
    uint64_t t   = 1700000000;
    uint64_t cum = 0;
    const uint64_t base_diff = 5000;

    for (int i = 0; i <= 120; ++i) {
      ts.push_back(t);
      cum += base_diff;
      cd.push_back(cum);
      t += 50;  // 3x too fast
    }

    crypto::difficulty_type d = crypto::next_difficulty_lwma3(ts, cd, target, 100);
    std::cout << "  [Fast blocks] Base: " << base_diff << " → After: " << d << std::endl;
    TEST_ASSERT(d > base_diff, "Difficulty must increase when blocks arrive faster than target");
  }

  // --- Sub-test B: Blocks arriving 3.3x too SLOW → difficulty must DECREASE ---
  {
    std::vector<uint64_t>                 ts;
    std::vector<crypto::difficulty_type>  cd;
    uint64_t t   = 1700000000;
    uint64_t cum = 0;
    const uint64_t base_diff = 5000;

    for (int i = 0; i <= 120; ++i) {
      ts.push_back(t);
      cum += base_diff;
      cd.push_back(cum);
      t += 500;  // 3.3x too slow
    }

    crypto::difficulty_type d = crypto::next_difficulty_lwma3(ts, cd, target, 100);
    std::cout << "  [Slow blocks] Base: " << base_diff << " → After: " << d << std::endl;
    TEST_ASSERT(d < base_diff, "Difficulty must decrease when blocks arrive slower than target");
  }

  std::cout << "  PASSED: Epoch retarget anchors within 100 blocks." << std::endl;
  return true;
}

// ─── TEST 2: 4 Concurrent Miners ───────────────────────────────
bool test_4_miners_concurrent() {
  std::cout << "\n[TEST 2/3] 4-Miner Concurrent Mining & Block Production" << std::endl;

  // Fresh chain in isolated test directory
  cryptonote::Blockchain chain;
  chain.init("./test_data_tdd");

  std::atomic<bool>     running{true};
  std::atomic<uint32_t> total_blocks{0};
  const uint32_t        goal = 20;

  auto t0 = std::chrono::steady_clock::now();

  std::vector<std::thread> miners;
  for (int id = 1; id <= 4; ++id) {
    miners.emplace_back([&, id]() {
      uint32_t nonce = static_cast<uint32_t>(id) * 10000000u;

      while (running.load() && total_blocks.load() < goal) {
        // Get current difficulty once, then grind many nonces against it
        crypto::difficulty_type target_diff = chain.get_current_difficulty();

        // Inner grind loop: try 50000 nonces before re-checking state
        for (uint32_t attempt = 0; attempt < 50000 && running.load(); ++attempt) {
          cryptonote::block b{};
          b.major_version = 1;
          b.minor_version = 0;
          b.timestamp     = static_cast<uint64_t>(std::time(nullptr));
          b.nonce         = nonce++;

          // Build unique hash material from nonce
          std::memset(b.hash.data(), 0, 32);
          b.hash[0] = static_cast<uint8_t>(id);
          b.hash[1] = static_cast<uint8_t>(nonce & 0xFF);
          b.hash[2] = static_cast<uint8_t>((nonce >> 8) & 0xFF);
          b.hash[3] = static_cast<uint8_t>((nonce >> 16) & 0xFF);
          b.hash[4] = static_cast<uint8_t>((nonce >> 24) & 0xFF);

          uint8_t pow_hash[32];
          crypto::RandomXManager::instance().calculate_hash(
              b.hash.data(), 32, pow_hash);

          if (crypto::check_pow_hash(pow_hash, target_diff)) {
            crypto::difficulty_type diff = 0;
            if (chain.add_new_block(b, diff)) {
              uint32_t n = ++total_blocks;
              std::cout << "  [Miner #" << id << "] Block #"
                        << chain.get_current_height()
                        << "  diff=" << diff << std::endl;
              if (n >= goal) { running.store(false); return; }
              break; // re-fetch difficulty for next block
            }
          }
        }
      }
    });
  }

  // Timeout guard: 120 seconds max
  auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(120);
  while (running.load() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  running.store(false);
  for (auto& m : miners) if (m.joinable()) m.join();

  auto t1 = std::chrono::steady_clock::now();
  double elapsed = std::chrono::duration<double>(t1 - t0).count();
  double avg_bt  = (total_blocks > 0) ? elapsed / total_blocks.load() : 0;

  std::cout << "  Blocks mined  : " << total_blocks.load() << std::endl;
  std::cout << "  Wall time     : " << elapsed << " s" << std::endl;
  std::cout << "  Avg block time: " << avg_bt  << " s" << std::endl;

  TEST_ASSERT(total_blocks.load() >= goal,
    "4 miners must produce at least 20 blocks within timeout");

  chain.shutdown();
  std::cout << "  PASSED: 4 miners produced blocks concurrently." << std::endl;
  return true;
}

// ─── TEST 3: Stealth Tx + RingCT + Mempool ─────────────────────
bool test_transactions_and_mempool() {
  std::cout << "\n[TEST 3/3] Stealth Address Transfer, RingCT-16 & Mempool" << std::endl;

  wallet::Wallet sender, recipient;
  sender.create_new("tdd_sender",    "pw1");
  recipient.create_new("tdd_recip",  "pw2");

  std::cout << "  Sender   : " << sender.get_address()    << std::endl;
  std::cout << "  Recipient: " << recipient.get_address()  << std::endl;

  TEST_ASSERT(!sender.get_address().empty(),    "Sender address must exist");
  TEST_ASSERT(!recipient.get_address().empty(),  "Recipient address must exist");
  TEST_ASSERT(sender.get_address() != recipient.get_address(),
    "Two wallets must have different stealth addresses");

  // Create RingCT transfer
  cryptonote::transaction tx;
  std::string err;
  uint64_t amount = UINT64_C(50) * COIN_UNITS;  // 50 XRL

  bool ok = sender.create_transfer(recipient.get_address(), amount, tx, err);
  TEST_ASSERT(ok, "create_transfer must succeed");

  std::cout << "  RingCT type        : " << (int)tx.rct_signatures.type << std::endl;
  std::cout << "  Ring size (offsets) : " << tx.vin[0].key_offsets.size() << std::endl;
  std::cout << "  BP+ proof bytes    : " << tx.rct_signatures.bulletproof_plus_bytes.size() << std::endl;

  TEST_ASSERT(tx.rct_signatures.type == 6,
    "Signature type must be 6 (CLSAG + Bulletproofs+)");
  TEST_ASSERT(tx.vin[0].key_offsets.size() == (CRYPTONOTE_MINIMUM_MIXIN_V3 + 1),
    "Ring size must be 16 (MIXIN 15 + 1)");
  TEST_ASSERT(!tx.rct_signatures.bulletproof_plus_bytes.empty(),
    "Bulletproofs+ proof must be non-empty");

  // Mempool admission
  cryptonote::TxMemoryPool pool;
  std::string pool_err;
  bool added = pool.add_tx(tx, pool_err);
  TEST_ASSERT(added, "Transaction must be accepted into mempool");
  TEST_ASSERT(pool.size() == 1, "Mempool must have exactly 1 tx");

  // Block template extraction
  auto tpl = pool.get_block_template_txs(300000);
  TEST_ASSERT(tpl.size() == 1, "Block template must include the mempool tx");

  std::cout << "  PASSED: Stealth addresses, CLSAG-16, BP+, mempool & block template verified." << std::endl;
  return true;
}

// ─── main ───────────────────────────────────────────────────────
int main() {
  std::cout << "=================================================================" << std::endl;
  std::cout << "   RandomLite (XRL) TDD Integration Test Suite                    " << std::endl;
  std::cout << "=================================================================" << std::endl;

  bool t1 = test_100_block_epoch_retarget();
  bool t2 = test_4_miners_concurrent();
  bool t3 = test_transactions_and_mempool();

  std::cout << "\n=================================================================" << std::endl;
  if (t1 && t2 && t3) {
    std::cout << "   ALL " << tests_passed << "/" << tests_total
              << " ASSERTIONS PASSED" << std::endl;
  } else {
    std::cout << "   SOME TESTS FAILED (" << tests_passed << "/"
              << tests_total << " passed)" << std::endl;
  }
  std::cout << "=================================================================" << std::endl;

  return (t1 && t2 && t3) ? 0 : 1;
}
