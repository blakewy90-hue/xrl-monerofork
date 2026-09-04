// Copyright (c) 2026, The RandomLite (XRL) Project
//
// Deterministic genesis block generator for RandomLite (XRL).
//
// Unlike the historical genesis coins (which used a one-off random tx key), this
// tool derives the genesis miner transaction from a FIXED, published seed. Anyone
// can re-run this tool with the same seed and obtain the identical GENESIS_TX hex,
// which makes the genesis block auditable and reproducible.
//
// Usage:
//   xrl-genesis-tool <seed_string> [nonce]
//
// Output:
//   The serialized genesis coinbase transaction as a hex blob (GENESIS_TX) and the
//   derived genesis address, ready to paste into cryptonote_config.h.

#include <cstring>
#include <iostream>
#include <string>

#include "crypto/crypto.h"
#include "crypto/crypto-ops.h"
#include "crypto/hash.h"
#include "cryptonote_basic/account.h"
#include "cryptonote_basic/cryptonote_basic.h"
#include "cryptonote_basic/cryptonote_format_utils.h"
#include "cryptonote_config.h"
#include "common/util.h"
#include "string_tools.h"
#include "mlocker.h"

using namespace cryptonote;

namespace
{
  // Derive a deterministic keypair from an arbitrary seed string.
  // Uses cn_fast_hash -> ec_scalar (already reduced) -> generate_keys(recover).
  void derive_keypair_from_seed(const std::string& seed, crypto::public_key& pub, crypto::secret_key& sec)
  {
    crypto::ec_scalar scalar;
    crypto::hash_to_scalar(seed.data(), seed.size(), scalar);
    crypto::secret_key recovery;
    std::memcpy(epee::unwrap(recovery).data, scalar.data, 32);
    crypto::generate_keys(pub, sec, recovery, true);
  }
}

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "Usage: xrl-genesis-tool <seed_string> [nonce]\n";
    return 1;
  }

  const std::string seed = argv[1];
  const uint32_t nonce = (argc >= 3) ? static_cast<uint32_t>(std::stoul(argv[2])) : 10001u;

  // ---- 1. Deterministic genesis account -----------------------------------
  // Derive spend + view keys from the seed so the genesis address is reproducible.
  crypto::public_key spend_pub;
  crypto::secret_key spend_sec;
  derive_keypair_from_seed(seed + ":spend", spend_pub, spend_sec);

  crypto::public_key view_pub;
  crypto::secret_key view_sec;
  derive_keypair_from_seed(seed + ":view", view_pub, view_sec);

  account_public_address genesis_addr;
  genesis_addr.m_spend_public_key = spend_pub;
  genesis_addr.m_view_public_key  = view_pub;

  // ---- 2. Deterministic genesis tx key ------------------------------------
  crypto::public_key txkey_pub;
  crypto::secret_key txkey_sec;
  derive_keypair_from_seed(seed + ":txkey", txkey_pub, txkey_sec);

  // ---- 3. Build the v1 genesis miner transaction --------------------------
  transaction tx;
  tx.version = 1;
  tx.unlock_time = CRYPTONOTE_MINED_MONEY_UNLOCK_WINDOW;
  tx.vin.clear();
  tx.vout.clear();
  tx.extra.clear();

  if (!add_tx_pub_key_to_extra(tx, txkey_pub))
  {
    std::cerr << "failed to add tx pub key to extra\n";
    return 1;
  }
  if (!sort_tx_extra(tx.extra, tx.extra))
  {
    std::cerr << "failed to sort extra\n";
    return 1;
  }

  txin_gen in;
  in.height = 0;
  tx.vin.push_back(in);

  uint64_t block_reward = 0;
  if (!get_block_reward(0, 0, 0, block_reward, 1))
  {
    std::cerr << "get_block_reward failed\n";
    return 1;
  }

  // Genesis: single output (no decomposition), exactly like the canonical form.
  crypto::key_derivation derivation = {};
  if (!crypto::generate_key_derivation(genesis_addr.m_view_public_key, txkey_sec, derivation))
  {
    std::cerr << "generate_key_derivation failed\n";
    return 1;
  }
  crypto::public_key out_eph_public_key = {};
  if (!crypto::derive_public_key(derivation, 0, genesis_addr.m_spend_public_key, out_eph_public_key))
  {
    std::cerr << "derive_public_key failed\n";
    return 1;
  }

  tx_out out;
  out.amount = block_reward;
  out.target = txout_to_key(out_eph_public_key);
  tx.vout.push_back(out);

  // ---- 4. Serialize to hex -------------------------------------------------
  const blobdata tx_blob = t_serializable_object_to_blob(tx);
  const std::string tx_hex = epee::string_tools::buff_to_hex_nodelimer(tx_blob);

  std::cout << "// Deterministic RandomLite (XRL) genesis, seed = \"" << seed << "\"\n";
  std::cout << "// Genesis block reward (atomic units): " << block_reward << "\n";
  std::cout << "std::string const GENESIS_TX = \"" << tx_hex << "\";\n";
  std::cout << "uint32_t const GENESIS_NONCE = " << nonce << ";\n";

  return 0;
}
