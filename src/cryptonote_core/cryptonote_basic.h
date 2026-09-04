#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <array>
#include "../cryptonote_config.h"
#include "../crypto/difficulty.h"

namespace cryptonote {

  typedef std::array<uint8_t, 32> crypto_hash;
  typedef std::array<uint8_t, 32> public_key;
  typedef std::array<uint8_t, 32> key_image;

  // Transaction Input Types
  struct txin_to_key {
    uint64_t amount;
    std::vector<uint64_t> key_offsets; // Ring member output indices (Ring size = 16)
    key_image k_image; // Spent key image to prevent double-spending
  };

  // Transaction Output Types (Stealth Address Output)
  struct txout_to_key {
    public_key key; // Stealth 1-time output key P = H(rA)G + B
  };

  struct tx_out {
    uint64_t amount; // 0 for RingCT confidential transactions
    txout_to_key target;
  };

  // RingCT Signature Structure (CLSAG + Bulletproofs+)
  struct rct_signatures {
    uint8_t type; // CLSAG RingCT = 5, Bulletproofs+ = 6
    uint64_t txnFee;
    std::vector<public_key> pseudoOuts;
    std::vector<uint8_t> bulletproof_plus_bytes; // Compact Zero-Knowledge Range Proofs
    std::vector<uint8_t> clsag_signatures;       // CLSAG ring signature payload
  };

  // Transaction Structure
  struct transaction {
    uint8_t version;
    uint64_t unlock_time;
    std::vector<txin_to_key> vin;
    std::vector<tx_out> vout;
    std::vector<uint8_t> extra; // Tx pubkey & payment ID
    rct_signatures rct_signatures;

    crypto_hash hash;
  };

  // Block Header Structure
  struct block_header {
    uint8_t major_version;
    uint8_t minor_version;
    uint64_t timestamp;
    crypto_hash prev_id;
    uint32_t nonce;
  };

  // Block Structure
  struct block : public block_header {
    transaction miner_tx;
    std::vector<crypto_hash> tx_hashes;
    crypto_hash hash;
  };

} // namespace cryptonote
