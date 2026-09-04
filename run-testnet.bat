@echo off
setlocal EnableExtensions EnableDelayedExpansion

rem RandomLite (XRL) solo testnet launcher - DYNAMIC DIFFICULTY MODE
set "ROOT=%~dp0"
set "BIN=%ROOT%build\monero-xrl\bin"
set "DATA=%ROOT%testnet-data"
set "WALLET_DIR=%ROOT%testnet-wallet"
set "WALLET_NAME=miner_wallet"
set "ADDR=9vsFFqswBvDciXAs71NeSV4bQFKD7Aq9g2XUJiD5jaAi6u5YRWbEmuuSNT33viRQYSF2ukugSNRC5bmD4Naktben4Xig5sS"

rem Dependency DLLs (LLVM-MinGW + vcpkg + unbound)
for /d %%D in ("%LOCALAPPDATA%\Microsoft\WinGet\Packages\MartinStorsjo.LLVM-MinGW.UCRT*") do (
  for /d %%M in ("%%D\llvm-mingw-*-ucrt-x86_64") do set "MINGW_BIN=%%M\bin"
)
if defined MINGW_BIN set "PATH=%MINGW_BIN%;%PATH%"
set "PATH=%ROOT%vendor\vcpkg\installed\x64-mingw-dynamic\bin;%PATH%"
if exist "%ROOT%vendor\vcpkg\downloads\unbound-stage\mingw64\bin" (
  set "PATH=%ROOT%vendor\vcpkg\downloads\unbound-stage\mingw64\bin;%PATH%"
)

if not exist "%BIN%\monerod.exe" (
  echo [ERROR] Missing %BIN%\monerod.exe — run build-monero.bat first.
  pause
  exit /b 1
)

if not exist "%DATA%" mkdir "%DATA%"

echo ===================================================
echo   RandomLite XRL Automated Solo TESTNET
echo   Mode     : DYNAMIC DIFFICULTY ADJUSTMENT
echo   P2P/RPC  : 48080 / 48081
echo ===================================================

rem 1. Build an isolated background VBScript trigger file (8 second delay)
set "TRIGGER_VBS=%TEMP%\xrl_trigger.vbs"
echo WScript.Sleep 8000 > "%TRIGGER_VBS%"
echo Set objShell = CreateObject("WScript.Shell") >> "%TRIGGER_VBS%"
echo objShell.Run "curl -s http://127.0.0 -d ""{\""jsonrpc\"":\""2.0\"",\""id\"":\""0\"",\""method\"":\""start_mining\"",\""params\"":{\""miner_address\"":\""%ADDR%\"",\""threads_count\"":2,\""do_background_mining\"":false,\""ignore_battery\"":true}}"" -H ""Content-Type: application/json""", 0, False >> "%TRIGGER_VBS%"

echo [INFO] Firing clean background mining trigger...
start /b wscript.exe "%TRIGGER_VBS%"

rem 2. Run the daemon without the fixed difficulty flag
echo [INFO] Launching monerod daemon...
"%BIN%\monerod.exe" --testnet --data-dir "%DATA%" --offline --disable-dns-checkpoints --rpc-bind-ip 127.0.0.1 --rpc-bind-port 48081 --p2p-bind-port 48080 --log-level 1

rem 3. Cleanup temp file after daemon exits
del "%TRIGGER_VBS%" >nul 2>&1
echo.
echo [INFO] Daemon stopped.
pause