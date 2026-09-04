#pragma once

#include <cstdint>
#include <string>

namespace config {

  // Network Identity & Coin Branding
  #define CRYPTONOTE_NAME                         "randomlite"
  #define CRYPTONOTE_BLOCKS_FILENAME              "blockchain.bin"
  #define CRYPTONOTE_BLOCKS_INDEX_FILENAME        "blockindexes.bin"
  #define CRYPTONOTE_POOLDATA_FILENAME            "mempool.bin"
  #define P2P_NET_DATA_FILENAME                   "p2pstate.bin"
  #define MINER_CONFIG_FILE_NAME                  "miner_config.json"
  
  // Coin Ticker and Display Decimals
  #define COIN_NAME                               "RandomLite"
  #define COIN_TICKER                             "XRL"
  #define COIN_UNITS                              UINT64_C(1000000000000) // 12 decimals (like XMR)
  
  // Total Money Supply (18.4 Million XRL before Tail Emission)
  #define MONEY_SUPPLY                            ((uint64_t)(-1)) // (uint64_t)-1 or ~18.4M base
  #define EMISSION_SPEED_FACTOR_PER_MINUTE        20
  #define FINAL_SUBSIDY_PER_MINUTE                UINT64_C(600000000000) // 0.6 XRL per minute tail emission

  // Consensus Parameters (FAST BLOCKTIME + RANDOMX)
  // Target block time: 150 seconds (2.5 minutes like Litecoin)
  #define DIFFICULTY_TARGET_V2                    150
  #define CRYPTONOTE_BLOCK_FUTURE_TIME_LIMIT       (2 * 60 * 60)
  #define DIFFICULTY_WINDOW_V2                    720 // 720 blocks (~30 hours) sliding window
  #define DIFFICULTY_LAG_V2                       15
  #define DIFFICULTY_CUT_V2                       60

  // RandomX Seed Hash Epoch Configuration
  // Monero uses 2048 blocks @ 120s (~2.84 days).
  // For 150s blocks, 1440 blocks = exactly 2.5 days per epoch rotation.
  #define SEED_HASH_EPOCH_BLOCKS                  1440
  #define SEED_HASH_LAG_BLOCKS                    64

  // Output Spendable Maturity (Lock time)
  // 10 blocks @ 150s = 25 minutes spendable lock maturity
  #define CRYPTONOTE_DEFAULT_TX_SPENDABLE_AGE     10

  // Privacy Ring Signature Anonymity Set Size
  // Ring size 16 = 1 real spent input + 15 decoy outputs
  #define CRYPTONOTE_MINIMUM_MIXIN_V3             15 // Ring size 16

  // Dynamic Block Size Limit
  // 300,000 bytes (300 KB) base full-reward zone
  #define CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE_V2   300000
  #define CRYPTONOTE_COINBASE_BLOB_RESERVED_SIZE          600
  #define CRYPTONOTE_DISPLAY_DECIMALS                     12

  // Network Ports
  #define P2P_DEFAULT_PORT                        28080
  #define RPC_DEFAULT_PORT                        28081
  #define ZMQ_DEFAULT_PORT                        28082
  
  // Network Magic Bytes (Unique byte identifier for P2P handshake)
  const uint8_t P2P_NETWORK_ID[16] = {
    0x52, 0x41, 0x4e, 0x44, 0x4f, 0x4d, 0x4c, 0x49, // "R A N D O M L I"
    0x54, 0x45, 0x50, 0x52, 0x49, 0x56, 0x30, 0x31  // "T E P R I V 0 1"
  };

  // Hardcoded Genesis Block Nonce & Tx Public Key
  #define GENESIS_NONCE                           10001
  #define GENESIS_TIMESTAMP                       1788461926
  #define GENESIS_HASH_HEX                        "e61140f040d8c5263246e8e21a447db88cadb7de9d04590ab0daaad16d773d16"
  #define GENESIS_COINBASE_TX_HEX                 "010a01ff000180809860029b2e4c0271c0011e761505c8e22c9545465e94b2361665a3d75c50c058728a4c148286"

  // Seed Node IP List (Mainnet Bootstrappers)
  const char* const SEED_NODES[] = {
    "node1.randomlite.org:28080",
    "node2.randomlite.org:28080",
    "seed.xrl-privacy.net:28080"
  };

} // namespace config
