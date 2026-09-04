#include "tx_pool.h"
#include <algorithm>
#include <iostream>

namespace cryptonote {

  bool TxMemoryPool::add_tx(const transaction &tx, std::string &error_reason) {
    std::lock_guard<std::mutex> lock(m_pool_lock);

    if (tx.vin.empty() || tx.vout.empty()) {
      error_reason = "Transaction must contain inputs and outputs";
      return false;
    }

    if (tx.rct_signatures.type != 6 || tx.rct_signatures.bulletproof_plus_bytes.empty()
        || tx.rct_signatures.clsag_signatures.empty()) {
      error_reason = "Transaction is missing the required RingCT proof payloads";
      return false;
    }

    for (const auto &entry : m_mempool) {
      if (entry.tx.hash == tx.hash) {
        error_reason = "Transaction is already in the mempool";
        return false;
      }
    }

    for (const auto &in : tx.vin) {
      if (in.key_offsets.size() != CRYPTONOTE_MINIMUM_MIXIN_V3 + 1) {
        error_reason = "Ring size must be exactly 16";
        return false;
      }

      for (const auto &entry : m_mempool) {
        for (const auto &existing_in : entry.tx.vin) {
          if (existing_in.k_image == in.k_image) {
            error_reason = "Key image is already present in the mempool";
            return false;
          }
        }
      }
    }

    TxPoolEntry entry{};
    entry.tx = tx;
    entry.fee = tx.rct_signatures.txnFee;
    entry.receive_time = static_cast<uint64_t>(time(nullptr));
    entry.blob_size = 1800; // Average RingCT tx size

    m_mempool.push_back(entry);

    // Sort mempool by fee per byte (descending)
    std::sort(m_mempool.begin(), m_mempool.end(), [](const TxPoolEntry &a, const TxPoolEntry &b) {
      return a.fee > b.fee;
    });

    std::cout << "[Mempool] Tx added to mempool. Fee: " << entry.fee << " atomic units. Mempool size: " << m_mempool.size() << std::endl;
    return true;
  }

  void TxMemoryPool::remove_txs(const std::vector<crypto_hash> &tx_hashes) {
    std::lock_guard<std::mutex> lock(m_pool_lock);
    m_mempool.erase(
        std::remove_if(m_mempool.begin(), m_mempool.end(),
          [&tx_hashes](const TxPoolEntry &entry) {
            return std::find(tx_hashes.begin(), tx_hashes.end(), entry.tx.hash) != tx_hashes.end();
          }),
        m_mempool.end());
  }

  std::vector<transaction> TxMemoryPool::get_block_template_txs(size_t max_bytes) const {
    std::lock_guard<std::mutex> lock(m_pool_lock);
    std::vector<transaction> result;
    size_t current_bytes = 0;

    for (const auto &entry : m_mempool) {
      if (current_bytes + entry.blob_size > max_bytes) break;
      result.push_back(entry.tx);
      current_bytes += entry.blob_size;
    }

    return result;
  }

  size_t TxMemoryPool::size() const {
    std::lock_guard<std::mutex> lock(m_pool_lock);
    return m_mempool.size();
  }

} // namespace cryptonote
