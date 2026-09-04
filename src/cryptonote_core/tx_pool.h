#pragma once

#include "cryptonote_basic.h"
#include <vector>
#include <map>
#include <mutex>
#include <unordered_set>

namespace cryptonote {

  struct TxPoolEntry {
    transaction tx;
    uint64_t fee;
    uint64_t receive_time;
    size_t blob_size;
  };

  class TxMemoryPool {
  public:
    TxMemoryPool() = default;
    ~TxMemoryPool() = default;

    // Add transaction to mempool (validates fees and ring signature structure)
    bool add_tx(const transaction &tx, std::string &error_reason);

    // Remove transactions included in a mined block
    void remove_txs(const std::vector<crypto_hash> &tx_hashes);

    // Get sorted top transactions for block template creation
    std::vector<transaction> get_block_template_txs(size_t max_bytes) const;

    // Get count of mempool transactions
    size_t size() const;

  private:
    std::vector<TxPoolEntry> m_mempool;
    std::unordered_set<std::string> m_mempool_key_images;
    mutable std::mutex m_pool_lock;
  };

} // namespace cryptonote
