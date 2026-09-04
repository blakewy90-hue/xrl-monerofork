cmake_minimum_required(VERSION 3.14)
project(randomlite CXX C)

# ============================================================================
#  AUTHORITATIVE PRODUCTION BUILD: use build-monero.bat (vendored Monero).
#  The targets below are the ORIGINAL pre-migration prototype. They contain
#  placeholder crypto / mock RandomX / mock LMDB and are NOT production-safe.
#  They are kept only for historical reference and are DISABLED by default.
#  Enable explicitly with:  cmake -DXRL_BUILD_PROTOTYPE=ON
# ============================================================================
option(XRL_BUILD_PROTOTYPE "Build the legacy prototype stub binaries (NOT production)" OFF)
if(NOT XRL_BUILD_PROTOTYPE)
  message(STATUS "XRL: prototype stubs disabled. Use build-monero.bat for the real production build.")
  return()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
  add_compile_options(/W3 /O2 /EHsc)
else()
  add_compile_options(-Wall -Wextra -O3 -pthread)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vendor/monero/compat)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vendor/monero/contrib/epee/include)

# Shared Core Sources
set(CRYPTO_SOURCES
  src/crypto/crypto.cpp
  src/crypto/difficulty.cpp
  src/crypto/randomx-monero.cpp
  vendor/monero/src/crypto/keccak.c
  vendor/monero/src/crypto/blake2b.c
  vendor/monero/compat/memwipe.c
)

set(CORE_SOURCES
  src/blockchain_db/lmdb_storage.cpp
  src/cryptonote_core/blockchain.cpp
  src/cryptonote_core/emission.cpp
  src/cryptonote_core/tx_pool.cpp
)

set(NETWORK_SOURCES
  src/p2p/net_server.cpp
  src/rpc/rpc_server.cpp
)

set(WALLET_SOURCES
  src/wallet/wallet.cpp
)

find_package(Threads REQUIRED)

# Target 1: Node Daemon (randomlited)
add_executable(randomlited
  src/daemon/main.cpp
  ${CRYPTO_SOURCES}
  ${CORE_SOURCES}
  ${NETWORK_SOURCES}
)
target_link_libraries(randomlited PRIVATE Threads::Threads)

# Target 2: Standalone CPU Miner (randomlite-miner)
add_executable(randomlite-miner
  src/miner/miner_main.cpp
  ${CRYPTO_SOURCES}
)
target_link_libraries(randomlite-miner PRIVATE Threads::Threads)

# Target 3: Wallet CLI (randomlite-wallet-cli)
add_executable(randomlite-wallet-cli
  src/wallet/wallet_main.cpp
  ${CORE_SOURCES}
  ${WALLET_SOURCES}
  ${CRYPTO_SOURCES}
)
target_link_libraries(randomlite-wallet-cli PRIVATE Threads::Threads)

# Target 4: Exchange Wallet RPC (randomlite-wallet-rpc)
add_executable(randomlite-wallet-rpc
  src/wallet/wallet_rpc_main.cpp
  ${CORE_SOURCES}
  ${WALLET_SOURCES}
  ${CRYPTO_SOURCES}
)
target_link_libraries(randomlite-wallet-rpc PRIVATE Threads::Threads)

# Target 5: TDD Test Suite (test_consensus_tdd)
add_executable(test_consensus_tdd
  tests/test_consensus_tdd.cpp
  ${CORE_SOURCES}
  ${WALLET_SOURCES}
  ${CRYPTO_SOURCES}
)
target_link_libraries(test_consensus_tdd PRIVATE Threads::Threads)

message(STATUS "RandomLite (XRL) Targets Configured: Daemon, Miner, Wallet CLI, Wallet RPC, TDD Tests.")
