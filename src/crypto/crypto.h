#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <array>

namespace crypto {

  typedef std::array<uint8_t, 32> hash;
  typedef std::array<uint8_t, 32> public_key;
  typedef std::array<uint8_t, 32> secret_key;
  typedef std::array<uint8_t, 32> key_image;

  // Keccak-256 (Monero standard hashing)
  void keccak(const uint8_t *data, size_t length, uint8_t *result, size_t result_len = 32);

  // Blake2b Fast Hash
  void blake2b(const uint8_t *data, size_t length, uint8_t *result, size_t result_len = 32);

  // Key Generation & Stealth Address Primitives
  void generate_keys(public_key &pub, secret_key &sec);
  bool secret_key_to_public_key(const secret_key &sec, public_key &pub);
  
  // Derivation: P = H(rA)G + B (Stealth Address 1-Time Key)
  bool derive_stealth_address(
      const public_key &spend_public_key,
      const public_key &view_public_key,
      const secret_key &tx_secret_key,
      uint64_t output_index,
      public_key &derived_stealth_key
  );

  // Generate spent key image to prevent double-spending
  void generate_key_image(
      const public_key &derived_stealth_key,
      const secret_key &derived_secret_key,
      key_image &k_image
  );

  // Convert hash to hexadecimal string
  std::string hash_to_hex(const hash &h);
  hash hex_to_hash(const std::string &hex_str);

} // namespace crypto
