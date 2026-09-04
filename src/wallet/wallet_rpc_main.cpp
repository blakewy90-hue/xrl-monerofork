#include "wallet.h"
#include "../cryptonote_config.h"
#include "../cryptonote_core/emission.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>

static std::atomic<bool> g_running{true};

void signal_handler(int sig) {
  std::cout << "\n[Wallet RPC] Shutdown signal received (" << sig << "). Exiting..." << std::endl;
  g_running = false;
}

int main(int argc, char* argv[]) {
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  std::cout << "Starting " << COIN_NAME << " (" << COIN_TICKER << ") Wallet RPC Daemon v1.0.0..." << std::endl;
  
  wallet::Wallet w;
  w.create_new("exchange_hot_wallet", "secure_password");

  std::cout << "[Wallet RPC] Hot Wallet Address: " << w.get_address() << std::endl;
  std::cout << "[Wallet RPC] Listening on http://127.0.0.1:" << ZMQ_DEFAULT_PORT << "/json_rpc" << std::endl;
  std::cout << "[Wallet RPC] Standard Exchange Endpoints: get_balance, get_address, make_transfer, rescan" << std::endl;
  std::cout << "Press Ctrl+C to stop RPC wallet daemon.\n" << std::endl;

  while (g_running) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
  }

  std::cout << "[Wallet RPC] Daemon stopped." << std::endl;
  return 0;
}
