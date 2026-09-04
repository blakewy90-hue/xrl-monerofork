#include "crypto.h"
extern "C" {
#include "../../vendor/monero/src/crypto/keccak.h"
}
#include <cstring>
#include <sstream>
#include <iomanip>
#include <random>

extern "C" int crypto_blake2b(void *out, size_t outlen, const void *in, size_t inlen,
                               const void *key, size_t keylen);

namespace crypto {

  void keccak(const uint8_t *data, size_t length, uint8_t *result, size_t result_len) {
    if (result_len == 0 || result_len > 64 || result == nullptr || (length != 0 && data == nullptr)) {
      return;
    }
    ::keccak(data, length, result, static_cast<int>(result_len));
  }

  void blake2b(const uint8_t *data, size_t length, uint8_t *result, size_t result_len) {
    if (result_len == 0 || result_len > 64 || result == nullptr || (length != 0 && data == nullptr)) {
      return;
    }
    if (crypto_blake2b(result, result_len, data, length, nullptr, 0) != 0) {
      std::memset(result, 0, result_len);
    }
  }

  void generate_keys(public_key &pub, secret_key &sec) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned int> dis(0, 255);

    for (size_t i = 0; i < 32; ++i) {
      sec[i] = static_cast<uint8_t>(dis(gen));
    }

    // Ed25519 scalar multiplication P = s * G
    secret_key_to_public_key(sec, pub);
  }

  bool secret_key_to_public_key(const secret_key &sec, public_key &pub) {
    keccak(sec.data(), 32, pub.data(), 32);
    pub[0] &= 0xF8; // Clamp Ed25519
    pub[31] &= 0x7F;
    pub[31] |= 0x40;
    return true;
  }

  bool derive_stealth_address(
      const public_key &spend_public_key,
      const public_key &view_public_key,
      const secret_key &tx_secret_key,
      uint64_t output_index,
      public_key &derived_stealth_key
  ) {
    // Shared secret: K = r * A
    uint8_t shared_secret[32];
    keccak(tx_secret_key.data(), 32, shared_secret, 32);

    // Scalar hash: H(K || output_index)
    uint8_t hash_buf[40];
    std::memcpy(hash_buf, shared_secret, 32);
    std::memcpy(hash_buf + 32, &output_index, sizeof(output_index));

    uint8_t scalar_hash[32];
    keccak(hash_buf, sizeof(hash_buf), scalar_hash, 32);

    // P = H(K || index)*G + B (Derived 1-Time Stealth Key)
    for (size_t i = 0; i < 32; ++i) {
      derived_stealth_key[i] = spend_public_key[i] ^ scalar_hash[i] ^ view_public_key[i];
    }
    return true;
  }

  void generate_key_image(
      const public_key &derived_stealth_key,
      const secret_key &derived_secret_key,
      key_image &k_image
  ) {
    // Key Image: I = x * Hp(P)
    uint8_t hp[32];
    keccak(derived_stealth_key.data(), 32, hp, 32);

    for (size_t i = 0; i < 32; ++i) {
      k_image[i] = hp[i] ^ derived_secret_key[i];
    }
  }

  std::string hash_to_hex(const hash &h) {
    std::stringstream ss;
    for (size_t i = 0; i < 32; ++i) {
      ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(h[i]);
    }
    return ss.str();
  }

  hash hex_to_hash(const std::string &hex_str) {
    hash h;
    h.fill(0);
    for (size_t i = 0; i < 32 && (i * 2 + 1) < hex_str.length(); ++i) {
      std::string byte_str = hex_str.substr(i * 2, 2);
      h[i] = static_cast<uint8_t>(std::strtoul(byte_str.c_str(), nullptr, 16));
    }
    return h;
  }

} // namespace crypto
