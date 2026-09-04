@echo off
setlocal EnableExtensions EnableDelayedExpansion

rem ======================================================================
rem  RandomLite (XRL) All-in-One Miner
rem  Starts: daemon + solo mining + wallet console
rem  Usage : xrl-miner.bat [--threads N] [--no-wallet] [--no-mine]
rem ======================================================================

set "ROOT=%~dp0"
set "BIN=%ROOT%build\monero-xrl\bin"
set "DATA=%ROOT%testnet-data"
set "WALLET_DIR=%ROOT%testnet-wallet"
set "WALLET_NAME=miner_wallet"
set "WALLET_PASS=passwordpass123"
set "ADDR=9vsFFqswBvDciXAs71NeSV4bQFKD7Aq9g2XUJiD5jaAi6u5YRWbEmuuSNT33viRQYSF2ukugSNRC5bmD4Naktben4Xig5sS"
set "RPC_PORT=48081"
set "P2P_PORT=48080"
set "THREADS=2"
set "DO_WALLET=1"
set "DO_MINE=1"

rem ---- parse args ----
:parse_args
if "%~1"=="" goto args_done
if /i "%~1"=="--threads"   ( set "THREADS=%~2" & shift & shift & goto parse_args )
if /i "%~1"=="--no-wallet" ( set "DO_WALLET=0" & shift & goto parse_args )
if /i "%~1"=="--no-mine"   ( set "DO_MINE=0" & shift & goto parse_args )
if /i "%~1"=="--help"      ( goto show_help )
shift
goto parse_args
:args_done

rem ---- locate runtime DLL dirs (MinGW + vcpkg + unbound) ----
for /d %%D in ("%LOCALAPPDATA%\Microsoft\WinGet\Packages\MartinStorsjo.LLVM-MinGW.UCRT*") do (
  for /d %%M in ("%%D\llvm-mingw-*-ucrt-x86_64") do set "MINGW_BIN=%%M\bin"
)
if defined MINGW_BIN set "PATH=%MINGW_BIN%;%PATH%"
set "PATH=%ROOT%vendor\vcpkg\installed\x64-mingw-dynamic\bin;%PATH%"
if exist "%ROOT%vendor\vcpkg\downloads\unbound-stage\mingw64\bin" (
  set "PATH=%ROOT%vendor\vcpkg\downloads\unbound-stage\mingw64\bin;%PATH%"
)

rem ---- sanity checks ----
if not exist "%BIN%\monerod.exe" (
  echo [ERROR] Missing %BIN%\monerod.exe - run build-monero.bat first.
  pause & exit /b 1
)
if not exist "%BIN%\monero-wallet-cli.exe" (
  echo [ERROR] Missing %BIN%\monero-wallet-cli.exe - run build-monero.bat first.
  pause & exit /b 1
)
if not exist "%WALLET_DIR%\%WALLET_NAME%.keys" (
  echo [ERROR] Wallet '%WALLET_NAME%' not found in %WALLET_DIR%
  pause & exit /b 1
)

if not exist "%DATA%" mkdir "%DATA%"

echo ===================================================
echo   RandomLite XRL All-in-One Miner
echo   Daemon RPC : 127.0.0.1:%RPC_PORT%
echo   P2P        : %P2P_PORT%
echo   Mining     : %THREADS% thread(s)
echo   Wallet     : %WALLET_NAME%
echo ===================================================
echo.

rem ---- 1. Start daemon in a separate window ----
echo [1/3] Starting monerod daemon...
start "XRL Daemon" /min "%BIN%\monerod.exe" ^
  --testnet --data-dir "%DATA%" --offline --disable-dns-checkpoints ^
  --rpc-bind-ip 127.0.0.1 --rpc-bind-port %RPC_PORT% ^
  --p2p-bind-port %P2P_PORT% --log-level 1

rem ---- 2. Wait for daemon RPC to come up ----
echo [2/3] Waiting for daemon RPC on port %RPC_PORT%...
set /a RETRIES=0
:wait_rpc
curl -s -m 3 http://127.0.0.1:%RPC_PORT%/get_height >NUL 2>NUL
if not errorlevel 1 goto rpc_up
set /a RETRIES+=1
if %RETRIES% GEQ 30 (
  echo [ERROR] Daemon RPC did not respond after 30 seconds.
  goto cleanup
)
timeout /t 1 /nobreak >NUL
goto wait_rpc
:rpc_up
echo       Daemon is up.

rem ---- 3. Start mining (uses /start_mining URI endpoint, not /json_rpc) ----
if "%DO_MINE%"=="1" (
  echo [3/3] Starting solo mining ^(%THREADS% threads^)...
  set "MINE_BODY={\"miner_address\":\"%ADDR%\",\"threads_count\":%THREADS%,\"do_background_mining\":false,\"ignore_battery\":true}"
  set "MINE_JSON=%TEMP%\xrl_start_mine.json"
  > "!MINE_JSON!" echo !MINE_BODY!
  curl -s -m 10 http://127.0.0.1:%RPC_PORT%/start_mining -d @"!MINE_JSON!" -H "Content-Type: application/json"
  echo.
) else (
  echo [3/3] Mining skipped (--no-mine).
)

rem ---- 4. Open wallet console ----
if "%DO_WALLET%"=="1" (
  echo.
  echo Opening wallet console ^(separate window^)...
  start "XRL Wallet" "%BIN%\monero-wallet-cli.exe" ^
    --testnet --wallet-file "%WALLET_DIR%\%WALLET_NAME%" ^
    --password "%WALLET_PASS%" --daemon-address 127.0.0.1:%RPC_PORT%
)

echo.
echo ===================================================
echo   All components started.
echo   Close the "XRL Daemon" window to stop everything.
echo ===================================================
echo.
echo Press any key to stop the daemon and exit...
pause >NUL

:cleanup
echo.
echo Stopping daemon...
taskkill /f /im monerod.exe >NUL 2>NUL
echo Done.
endlocal
exit /b 0

:show_help
echo.
echo RandomLite XRL All-in-One Miner
echo.
echo   xrl-miner.bat [options]
echo.
echo   --threads N    Number of mining threads (default: 2)
echo   --no-wallet    Do not open the wallet console
echo   --no-mine      Start daemon only, skip mining
echo   --help         Show this help
echo.
endlocal
exit /b 0
