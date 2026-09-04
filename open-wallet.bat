@echo off
setlocal EnableExtensions

rem RandomLite (XRL) testnet wallet operator
set "ROOT=%~dp0"
set "BIN=%ROOT%build\monero-xrl\bin"
set "WALLET_DIR=%ROOT%testnet-wallet"
set "WALLET_NAME=miner_wallet"
set "WALLET_PASS=passwordpass123"

rem Dependency DLLs (LLVM-MinGW + vcpkg + unbound)
for /d %%D in ("%LOCALAPPDATA%\Microsoft\WinGet\Packages\MartinStorsjo.LLVM-MinGW.UCRT*") do (
  for /d %%M in ("%%D\llvm-mingw-*-ucrt-x86_64") do set "MINGW_BIN=%%M\bin"
)
if defined MINGW_BIN set "PATH=%MINGW_BIN%;%PATH%"
set "PATH=%ROOT%vendor\vcpkg\installed\x64-mingw-dynamic\bin;%PATH%"
if exist "%ROOT%vendor\vcpkg\downloads\unbound-stage\mingw64\bin" (
  set "PATH=%ROOT%vendor\vcpkg\downloads\unbound-stage\mingw64\bin;%PATH%"
)

if not exist "%WALLET_DIR%\%WALLET_NAME%.keys" (
  echo [ERROR] Wallet file '%WALLET_NAME%' not found in %WALLET_DIR%
  pause
  exit /b 1
)

echo ===================================================
echo   RandomLite XRL Interactive Wallet Console
echo   Target Wallet : %WALLET_NAME%
echo   Daemon Address: 127.0.0.1:48081
echo ===================================================
echo.

"%BIN%\monero-wallet-cli.exe" --testnet --wallet-file "%WALLET_DIR%\%WALLET_NAME%" --password "%WALLET_PASS%" --daemon-address 127.0.0.1:48081

echo.
echo [INFO] Wallet session terminated.
pause