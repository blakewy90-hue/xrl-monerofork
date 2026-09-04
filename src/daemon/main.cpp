#include "../cryptonote_config.h"
#include "../cryptonote_core/blockchain.h"
#include "../crypto/difficulty.h"
#include "../crypto/randomx-monero.h"
#include "../p2p/net_server.h"
#include "../rpc/rpc_server.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>

static std::atomic<bool> g_running{true};

void signal_handler(int sig) {
  std::cout << "\n[Daemon] Shutdown signal received (" << sig << "). Exiting..." << std::endl;
  g_running = false;
}

int main(int argc, char* argv[]) {
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  std::cout << "Starting " << COIN_NAME << " (" << COIN_TICKER << ") Production Node Daemon v1.0.0..." << std::endl;
  std::cout << "Build Target: RandomX CPU PoW + RingCT Privacy + " << DIFFICULTY_TARGET_V2 << "s Blocktimes" << std::endl;

  cryptonote::Blockchain chain;
  if (!chain.init("./data")) {
    std::cerr << "Failed to initialize Blockchain daemon state." << std::endl;
    return 1;
  }

  // Start P2P TCP Network Server
  p2p::P2PServer p2p_server;
  p2p_server.start(P2P_DEFAULT_PORT);

  // Start HTTP JSON-RPC Server
  rpc::RPCServer rpc_server;
  rpc_server.start(&chain, RPC_DEFAULT_PORT);

  std::cout << "\n[P2P Server] Active on port " << P2P_DEFAULT_PORT << std::endl;
  std::cout << "[RPC Server] Active on port " << RPC_DEFAULT_PORT << std::endl;
  std::cout << "[Miner] RandomX CPU Worker active. Mining target blocks every ~" << DIFFICULTY_TARGET_V2 << " seconds." << std::endl;
  std::cout << "Press Ctrl+C to stop daemon.\n" << std::endl;

  uint64_t block_counter = chain.get_current_height();

  // Main daemon loop
  while (g_running) {
    if (!g_running) break;

    crypto::difficulty_type target_diff = chain.get_current_difficulty();

    cryptonote::block new_block{};
    new_block.major_version = 1;
    new_block.minor_version = 0;
    new_block.timestamp = static_cast<uint64_t>(std::time(nullptr));

    bool found_solution = false;
    uint8_t pow_hash[32];

    // CPU Mine target block
    for (uint32_t nonce = 1; nonce < 1000000 && g_running; ++nonce) {
      new_block.nonce = nonce;
      new_block.hash[0] = static_cast<uint8_t>(block_counter & 0xFF);
      new_block.hash[1] = static_cast<uint8_t>((block_counter >> 8) & 0xFF);
      new_block.hash[2] = static_cast<uint8_t>(nonce & 0xFF);
      new_block.hash[3] = static_cast<uint8_t>((nonce >> 8) & 0xFF);

      crypto::RandomXManager::instance().calculate_hash(&new_block.hash, 32, pow_hash);
      
      if (crypto::check_pow_hash(pow_hash, target_diff)) {
        found_solution = true;
        break;
      }
    }

    if (found_solution && g_running) {
      crypto::difficulty_type diff = 0;
      if (chain.add_new_block(new_block, diff)) {
        p2p_server.broadcast_block(new_block.hash.data(), block_counter);
        block_counter++;
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  p2p_server.stop();
  rpc_server.stop();
  chain.shutdown();

  std::cout << "[Daemon] Production node shutdown complete." << std::endl;
  return 0;
}
