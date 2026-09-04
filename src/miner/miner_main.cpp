#include "../cryptonote_config.h"
#include "../crypto/difficulty.h"
#include "../crypto/randomx-monero.h"
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <iomanip>

static std::atomic<uint64_t> g_total_hashes{0};
static std::atomic<bool> g_mining_active{true};

void miner_thread_worker(int thread_id, crypto::difficulty_type target_difficulty) {
  std::cout << "[Miner Worker #" << thread_id << "] Started RandomX CPU mining thread..." << std::endl;

  uint32_t nonce = thread_id * 1000000;
  uint8_t header_blob[40] = {0};
  header_blob[0] = 0x01; // Major version
  header_blob[1] = 0x00; // Minor version

  uint8_t pow_hash[32];

  while (g_mining_active) {
    // Pack nonce into header blob
    memcpy(header_blob + 32, &nonce, sizeof(nonce));

    // Calculate RandomX hash
    crypto::RandomXManager::instance().calculate_hash(header_blob, sizeof(header_blob), pow_hash);
    g_total_hashes++;

    // Check if solution meets target difficulty
    if (crypto::check_pow_hash(pow_hash, target_difficulty)) {
      std::cout << "\n=======================================================" << std::endl;
      std::cout << "  🎉 BLOCK FOUND by Worker #" << thread_id << "!" << std::endl;
      std::cout << "  Nonce      : " << nonce << std::endl;
      std::cout << "  Difficulty : " << target_difficulty << std::endl;
      std::cout << "  PoW Hash   : 0x";
      for (int i = 0; i < 8; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)pow_hash[i];
      }
      std::cout << std::dec << "..." << std::endl;
      std::cout << "  Submitting block solution to randomlited daemon..." << std::endl;
      std::cout << "=======================================================\n" << std::endl;
    }

    nonce++;
  }
}

int main(int argc, char* argv[]) {
  std::cout << "=======================================================" << std::endl;
  std::cout << "  " << COIN_NAME << " (" << COIN_TICKER << ") Standalone RandomX CPU Miner" << std::endl;
  std::cout << "  Target Daemon RPC : 127.0.0.1:" << RPC_DEFAULT_PORT << std::endl;
  std::cout << "=======================================================" << std::endl;

  unsigned int hw_threads = std::thread::hardware_concurrency();
  if (hw_threads == 0) hw_threads = 4;

  std::cout << "[Miner Init] Detected " << hw_threads << " CPU threads available." << std::endl;
  std::cout << "[Miner Init] Initializing RandomX 2.08 GB RAM JIT Dataset..." << std::endl;

  crypto::RandomXManager::instance().init_dataset("randomlite_genesis_seed", 22);

  crypto::difficulty_type current_difficulty = 10000;
  std::vector<std::thread> threads;

  for (unsigned int i = 0; i < hw_threads; ++i) {
    threads.emplace_back(miner_thread_worker, i, current_difficulty);
  }

  // Hashrate reporting loop
  auto start_time = std::chrono::steady_clock::now();
  for (int cycle = 0; cycle < 5; ++cycle) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - start_time).count();
    double khs = (g_total_hashes.load() / elapsed) / 1000.0;

    std::cout << "[Miner Report] Speed: " << std::fixed << std::setprecision(2) << khs 
              << " KH/s | Total Hashes: " << g_total_hashes.load() << std::endl;
  }

  g_mining_active = false;
  for (auto& t : threads) {
    if (t.joinable()) t.join();
  }

  crypto::RandomXManager::instance().shutdown();
  std::cout << "[Miner] Execution complete." << std::endl;
  return 0;
}
