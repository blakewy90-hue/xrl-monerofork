@echo off
echo ===================================================
echo   Building RandomLite (XRL) Production Suite ^& Tests
echo ===================================================

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" > NUL

if not exist build mkdir build
set MONERO_INCLUDES=/Ivendor\monero\compat /Ivendor\monero\contrib\epee\include
set MONERO_HASH_SOURCES=vendor\monero\src\crypto\keccak.c vendor\monero\src\crypto\blake2b.c vendor\monero\compat\memwipe.c
del /q build\*.obj 2>NUL

echo [1/5] Compiling randomlited production daemon...
cl /std:c++17 /EHsc /O2 /Isrc %MONERO_INCLUDES% /Fo:build\ /Fe:build\randomlited.exe src\daemon\main.cpp src\crypto\crypto.cpp src\crypto\difficulty.cpp src\crypto\randomx-monero.cpp %MONERO_HASH_SOURCES% src\blockchain_db\lmdb_storage.cpp src\cryptonote_core\blockchain.cpp src\cryptonote_core\emission.cpp src\cryptonote_core\tx_pool.cpp src\p2p\net_server.cpp src\rpc\rpc_server.cpp

if %ERRORLEVEL% NEQ 0 (
  echo [ERROR] Failed to compile daemon!
  exit /b %ERRORLEVEL%
)

echo [2/5] Compiling randomlite-miner standalone CPU miner...
cl /std:c++17 /EHsc /O2 /Isrc %MONERO_INCLUDES% /Fo:build\ /Fe:build\randomlite-miner.exe src\miner\miner_main.cpp src\crypto\crypto.cpp src\crypto\difficulty.cpp src\crypto\randomx-monero.cpp %MONERO_HASH_SOURCES%

if %ERRORLEVEL% NEQ 0 (
  echo [ERROR] Failed to compile CPU miner!
  exit /b %ERRORLEVEL%
)

echo [3/5] Compiling randomlite-wallet-cli wallet executable...
cl /std:c++17 /EHsc /O2 /Isrc %MONERO_INCLUDES% /Fo:build\ /Fe:build\randomlite-wallet-cli.exe src\wallet\wallet_main.cpp src\wallet\wallet.cpp src\crypto\crypto.cpp src\crypto\difficulty.cpp src\crypto\randomx-monero.cpp %MONERO_HASH_SOURCES% src\blockchain_db\lmdb_storage.cpp src\cryptonote_core\blockchain.cpp src\cryptonote_core\emission.cpp src\cryptonote_core\tx_pool.cpp

if %ERRORLEVEL% NEQ 0 (
  echo [ERROR] Failed to compile Wallet CLI!
  exit /b %ERRORLEVEL%
)

echo [4/5] Compiling randomlite-wallet-rpc exchange daemon...
cl /std:c++17 /EHsc /O2 /Isrc %MONERO_INCLUDES% /Fo:build\ /Fe:build\randomlite-wallet-rpc.exe src\wallet\wallet_rpc_main.cpp src\wallet\wallet.cpp src\crypto\crypto.cpp src\crypto\difficulty.cpp src\crypto\randomx-monero.cpp %MONERO_HASH_SOURCES% src\blockchain_db\lmdb_storage.cpp src\cryptonote_core\blockchain.cpp src\cryptonote_core\emission.cpp src\cryptonote_core\tx_pool.cpp

if %ERRORLEVEL% NEQ 0 (
  echo [ERROR] Failed to compile Wallet RPC!
  exit /b %ERRORLEVEL%
)

echo [5/5] Compiling test_consensus_tdd TDD test suite...
cl /std:c++17 /EHsc /O2 /Isrc %MONERO_INCLUDES% /Fo:build\ /Fe:build\test_consensus_tdd.exe tests\test_consensus_tdd.cpp src\wallet\wallet.cpp src\crypto\crypto.cpp src\crypto\difficulty.cpp src\crypto\randomx-monero.cpp %MONERO_HASH_SOURCES% src\blockchain_db\lmdb_storage.cpp src\cryptonote_core\blockchain.cpp src\cryptonote_core\emission.cpp src\cryptonote_core\tx_pool.cpp

if %ERRORLEVEL% NEQ 0 (
  echo [ERROR] Failed to compile TDD test suite!
  exit /b %ERRORLEVEL%
)

echo.
echo ===================================================
echo   PRODUCTION BUILD ^& TEST SUITE SUCCESSFUL!
echo   Daemon Executable     : build\randomlited.exe
echo   CPU Miner Executable  : build\randomlite-miner.exe
echo   Wallet CLI Executable : build\randomlite-wallet-cli.exe
echo   Wallet RPC Executable : build\randomlite-wallet-rpc.exe
echo   TDD Test Suite        : build\test_consensus_tdd.exe
echo ===================================================
