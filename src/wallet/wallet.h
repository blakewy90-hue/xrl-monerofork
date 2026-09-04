#pragma once

#include "../cryptonote_config.h"
#include "../cryptonote_core/cryptonote_basic.h"
#include "../cryptonote_core/emission.h"
#include <string>
#include <vector>
#include <map>

namespace wallet {

  struct WalletTransfer {
    std::string tx_hash;
    uint64_t amount;
    uint64_t height;
    bool is_incoming;
    bool is_unlocked;
    std::string recipient_address;
  };

  class Wallet {
  public:
    Wallet() = default;
    ~Wallet() = default;

    // Create a new wallet with random 25-word mnemonic seed
    bool create_new(const std::string& wallet_name, const std::string& password);

    // Open existing wallet file
    bool load(const std::string& wallet_name, const std::string& password);

    // Get Primary XRL Stealth Address
    std::string get_address() const { return m_address; }

    // Get 25-Word Mnemonic Recovery Seed
    std::string get_mnemonic() const { return m_mnemonic; }

    // Get Total and Unlocked Balance (Locked until 10 block maturity)
    void get_balance(uint64_t& total_balance, uint64_t& unlocked_balance) const;

    // Rescan blockchain for incoming stealth outputs & coinbase block rewards
    uint64_t rescan_blockchain(const std::string& data_dir = "./data");

    // Create & Sign RingCT Transaction (CLSAG 16 + Bulletproofs+)
    bool create_transfer(
        const std::string& dst_address,
        uint64_t amount,
        cryptonote::transaction& tx_out,
        std::string& err_msg
    );

    // Get transaction history
    std::vector<WalletTransfer> get_history() const { return m_history; }

  private:
    std::string m_filename;
    std::string m_address;
    std::string m_mnemonic;

    // Keys
    cryptonote::public_key m_spend_public_key;
    cryptonote::public_key m_view_public_key;
    std::array<uint8_t, 32> m_spend_private_key;
    std::array<uint8_t, 32> m_view_private_key;

    // State
    uint64_t m_total_balance = 0;
    uint64_t m_unlocked_balance = 0;
    std::vector<WalletTransfer> m_history;
  };

} // namespace wallet
