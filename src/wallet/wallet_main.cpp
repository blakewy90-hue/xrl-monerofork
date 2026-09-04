#include "wallet.h"
#include "../cryptonote_config.h"
#include "../cryptonote_core/emission.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

void print_help() {
  std::cout << "\n=======================================================" << std::endl;
  std::cout << "  " << COIN_NAME << " (" << COIN_TICKER << ") Wallet CLI Commands" << std::endl;
  std::cout << "=======================================================" << std::endl;
  std::cout << "  create <name>            - Create a new wallet" << std::endl;
  std::cout << "  rescan                   - Scan blockchain disk DB for mined rewards" << std::endl;
  std::cout << "  address                  - Show primary XRL stealth address" << std::endl;
  std::cout << "  balance                  - View total & unlocked balance" << std::endl;
  std::cout << "  transfer <addr> <amt>    - Send RingCT transaction (CLSAG 16)" << std::endl;
  std::cout << "  seed                     - Display 25-word recovery seed" << std::endl;
  std::cout << "  history                  - View transaction history" << std::endl;
  std::cout << "  tokenomics               - View XRL supply & emission breakdown" << std::endl;
  std::cout << "  exit                     - Exit wallet CLI\n" << std::endl;
}

void print_tokenomics() {
  std::cout << "\n=======================================================" << std::endl;
  std::cout << "  " << COIN_NAME << " (" << COIN_TICKER << ") Tokenomics & Emission Profile" << std::endl;
  std::cout << "=======================================================" << std::endl;
  std::cout << "  Base Money Supply    : 18,400,000 XRL" << std::endl;
  std::cout << "  Tail Emission        : 0.6 XRL per 150s block (~210,240 XRL/year)" << std::endl;
  std::cout << "  Emission Factor      : 20 (Exponential decay curve)" << std::endl;
  std::cout << "  Target Block Time    : 150 seconds (2.5 minutes)" << std::endl;
  std::cout << "  Output Lock Maturity : 10 blocks (25 minutes real time)" << std::endl;
  std::cout << "  Decimals             : 12 (1 XRL = 1,000,000,000,000 atomic units)" << std::endl;

  std::cout << "\n  [Emission Schedule Preview]" << std::endl;
  uint64_t generated = 0;
  for (uint64_t block = 0; block <= 1000000; block += 250000) {
    uint64_t reward = cryptonote::EmissionEngine::get_block_reward(generated, block);
    std::cout << "  Height #" << std::setw(7) << std::left << block 
              << " | Block Reward: " << cryptonote::EmissionEngine::print_money(reward) << std::endl;
    generated += reward * 250000;
  }
  std::cout << "=======================================================\n" << std::endl;
}

int main(int argc, char* argv[]) {
  std::cout << "Starting " << COIN_NAME << " (" << COIN_TICKER << ") Wallet CLI v1.0.0..." << std::endl;
  
  wallet::Wallet w;
  w.create_new("default_wallet", "password123");
  w.rescan_blockchain("./data");

  std::cout << "\nWallet loaded & synced with blockchain disk DB!" << std::endl;
  std::cout << "Address: " << w.get_address() << std::endl;
  
  uint64_t total = 0, unlocked = 0;
  w.get_balance(total, unlocked);
  std::cout << "Balance: " << cryptonote::EmissionEngine::print_money(total) << " (Unlocked: "
            << cryptonote::EmissionEngine::print_money(unlocked) << ")" << std::endl;

  print_help();

  std::string line;
  while (true) {
    std::cout << "xrl-wallet> ";
    if (!std::getline(std::cin, line)) break;
    if (line.empty()) continue;

    std::stringstream ss(line);
    std::string cmd;
    ss >> cmd;

    if (cmd == "exit" || cmd == "quit") {
      std::cout << "Exiting wallet." << std::endl;
      break;
    } else if (cmd == "help") {
      print_help();
    } else if (cmd == "create") {
      std::string wname = "new_wallet";
      ss >> wname;
      w.create_new(wname, "password123");
      std::cout << "New Wallet Created!" << std::endl;
      std::cout << "Stealth Address: " << w.get_address() << std::endl;
      std::cout << "25-Word Mnemonic Seed:\n" << w.get_mnemonic() << std::endl;
    } else if (cmd == "rescan") {
      w.rescan_blockchain("./data");
    } else if (cmd == "address") {
      std::cout << "Stealth Address: " << w.get_address() << std::endl;
    } else if (cmd == "balance") {
      w.rescan_blockchain("./data");
      w.get_balance(total, unlocked);
      std::cout << "Total Balance    : " << cryptonote::EmissionEngine::print_money(total) << std::endl;
      std::cout << "Unlocked Balance : " << cryptonote::EmissionEngine::print_money(unlocked) << std::endl;
      std::cout << "Lock Maturity    : 10 blocks (25 minutes spendable age)" << std::endl;
    } else if (cmd == "seed") {
      std::cout << "25-Word Mnemonic Seed:\n" << w.get_mnemonic() << std::endl;
    } else if (cmd == "tokenomics") {
      print_tokenomics();
    } else if (cmd == "history") {
      auto history = w.get_history();
      std::cout << "\nTransaction History (" << history.size() << " records):" << std::endl;
      for (const auto& tx : history) {
        std::cout << "  [TX] " << tx.tx_hash.substr(0, 16) << "... | Amount: " 
                  << cryptonote::EmissionEngine::print_money(tx.amount) 
                  << " -> " << tx.recipient_address.substr(0, 16) << "..." << std::endl;
      }
    } else if (cmd == "transfer") {
      std::string dst_addr;
      double amount_dbl;
      if (ss >> dst_addr >> amount_dbl) {
        uint64_t amount_atomic = static_cast<uint64_t>(amount_dbl * COIN_UNITS);
        cryptonote::transaction tx;
        std::string err;
        if (w.create_transfer(dst_addr, amount_atomic, tx, err)) {
          std::cout << "\n=======================================================" << std::endl;
          std::cout << "  🎉 RingCT Transaction Created & Signed!" << std::endl;
          std::cout << "  Anonymity Ring Size : 16 (1 real + 15 decoys)" << std::endl;
          std::cout << "  Range Proof Format  : Bulletproofs+ (Zero Knowledge)" << std::endl;
          std::cout << "  Transaction Fee     : 0.001 XRL" << std::endl;
          std::cout << "  Destination Address : " << dst_addr << std::endl;
          std::cout << "  Broadcasting transaction to randomlited P2P network..." << std::endl;
          std::cout << "=======================================================\n" << std::endl;
        } else {
          std::cerr << "Transfer Error: " << err << std::endl;
        }
      } else {
        std::cout << "Usage: transfer <destination_address> <amount_in_xrl>" << std::endl;
      }
    } else {
      std::cout << "Unknown command: '" << cmd << "'. Type 'help' for available commands." << std::endl;
    }
  }

  return 0;
}
