#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <memory>

namespace crypto {

  /**
   * RandomX Engine for RandomLite (XRL) Fast Block PoW Verification
   */
  class RandomXManager {
  public:
    static RandomXManager& instance();

    // Initialize RandomX Fast Mode dataset in 2.08 GB RAM
    bool init_dataset(const void* key, size_t key_size);

    // Pre-allocate next epoch seed dataset in background worker thread
    void preallocate_next_epoch(const void* next_key, size_t key_size);

    // Calculate PoW hash for block header + nonce
    bool calculate_hash(const void* input, size_t input_size, uint8_t output[32]);

    // Release RAM resources
    void shutdown();

  private:
    RandomXManager() = default;
    ~RandomXManager() = default;
    
    bool m_initialized = false;
  };

} // namespace crypto
