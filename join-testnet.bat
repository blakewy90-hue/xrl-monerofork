@echo off
setlocal EnableExtensions EnableDelayedExpansion

rem ======================================================================
rem  RandomLite (XRL) Testnet Participant
rem  Joins a hosted testnet via one or more seed nodes, then mines.
rem  Usage : join-testnet.bat <SEED_IP> [SEED2_IP] [--threads N] [--no-mine]
rem ======================================================================

set "ROOT=%~dp0"
set "BIN=%ROOT%build\monero-xrl\bin"
set "DATA=%ROOT%testnet-data"
set "WALLET_DIR=%ROOT%testnet-wallet-v2"
set "WALLET_NAME=xrl_miner"
set "WALLET_PASS=passwordpass123"
set "RPC_PORT=48081"
set "ZMQ_PORT=48082"
set "THREADS=2"
set "DO_MINE=1"
set "PEER_ARGS="

rem ---- first positional args are seed IPs until a --flag appears ----
:parse_args
if "%~1"=="" goto args_done
if /i "%~1"=="--threads" ( set "THREADS=%~2" & shift & shift & goto parse_args )
if /i "%~1"=="--no-mine" ( set "DO_MINE=0" & shift & goto parse_args )
if /i "%~1"=="--help"    ( goto show_help )
echo %~1 | findstr /r "^[0-9a-zA-Z.-]*$" >NUL && (
  set "PEER_ARGS=!PEER_ARGS! --add-peer %~1:48080"
)
shift
goto parse_args
:args_done

if "%PEER_ARGS%"=="" (
  echo [ERROR] At least one seed IP/hostname is required.
  goto show_help
)

for /d %%D in ("%LOCALAPPDATA%\Microsoft\WinGet\Packages\MartinStorsjo.LLVM-MinGW.UCRT*") do (
  for /d %%M in ("%%D\llvm-mingw-*-ucrt-x86_64") do set "MINGW_BIN=%%M\bin"
)
if defined MINGW_BIN set "PATH=%MINGW_BIN%;%PATH%"
set "PATH=%ROOT%vendor\vcpkg\installed\x64-mingw-dynamic\bin;%PATH%"
if exist "%ROOT%vendor\vcpkg\downloads\unbound-stage\mingw64\bin" (
  set "PATH=%ROOT%vendor\vcpkg\downloads\unbound-stage\mingw64\bin;%PATH%"
)

if not exist "%BIN%\monerod.exe" (
  echo [ERROR] Missing %BIN%\monerod.exe - run build-monero.bat first.
  pause & exit /b 1
)
if not exist "%DATA%" mkdir "%DATA%"

echo ===================================================
echo   RandomLite XRL Testnet Participant
echo   Seeds :%PEER_ARGS%
echo   Mining: %THREADS% thread(s)
echo ===================================================

echo [1/3] Starting daemon and syncing from seeds...
start "XRL Node" /min "%BIN%\monerod.exe" ^
  --testnet --data-dir "%DATA%" --disable-dns-checkpoints ^
  --p2p-bind-ip 127.0.0.1 --p2p-bind-port 48080 ^
  --rpc-bind-ip 127.0.0.1 --rpc-bind-port %RPC_PORT% ^
  --zmq-rpc-bind-ip 127.0.0.1 --zmq-rpc-bind-port %ZMQ_PORT% ^
  --allow-local-ip %PEER_ARGS% --log-level 1

echo [2/3] Waiting for RPC...
set /a RETRIES=0
:wait_rpc
curl -s -m 3 http://127.0.0.1:%RPC_PORT%/get_height >NUL 2>NUL
if not errorlevel 1 goto rpc_up
set /a RETRIES+=1
if %RETRIES% GEQ 60 ( echo [ERROR] Daemon RPC timeout. & goto cleanup )
timeout /t 1 /nobreak >NUL
goto wait_rpc
:rpc_up

if not exist "%WALLET_DIR%\%WALLET_NAME%.keys" (
  echo [INFO] No wallet found - generate one with open-wallet.bat or monero-wallet-cli.
  echo [INFO] Skipping mining (no miner address available).
  goto done
)

rem Read the primary address from the wallet's address file if present
set "ADDR="
if exist "%WALLET_DIR%\%WALLET_NAME%.address.txt" set /p ADDR=<"%WALLET_DIR%\%WALLET_NAME%.address.txt"
if "%ADDR%"=="" (
  echo [INFO] Address file missing. Skipping auto-mining; start mining manually:
  echo        curl http://127.0.0.1:%RPC_PORT%/start_mining -d "{\"miner_address\":\"<addr>\",\"threads_count\":%THREADS%}"
  goto done
)

if "%DO_MINE%"=="1" (
  echo [3/3] Starting mining once daemon reports synchronized...
  set /a W=0
  :wait_sync
  curl -s -m 3 http://127.0.0.1:%RPC_PORT%/get_info | findstr /c:"\"synchronized\": true" >NUL 2>NUL
  if not errorlevel 1 goto synced
  set /a W+=1
  if %W% GEQ 300 ( echo [WARN] Not synchronized after 5 min; mining anyway. & goto synced )
  timeout /t 2 /nobreak >NUL
  goto wait_sync
  :synced
  set "MINE_JSON=%TEMP%\xrl_join_mine.json"
  > "!MINE_JSON!" echo {"miner_address":"%ADDR%","threads_count":%THREADS%,"do_background_mining":false,"ignore_battery":true}
  curl -s -m 10 http://127.0.0.1:%RPC_PORT%/start_mining -d @"!MINE_JSON!" -H "Content-Type: application/json"
  echo.
) else (
  echo [3/3] Mining skipped (--no-mine).
)

:done
echo.
echo Node is running in the "XRL Node" window. Press any key here to shut it down...
pause >NUL
:cleanup
taskkill /f /im monerod.exe >NUL 2>NUL
endlocal
exit /b 0

:show_help
echo.
echo RandomLite XRL Testnet Participant
echo.
echo   join-testnet.bat ^<SEED_IP^> [SEED2_IP] [--threads N] [--no-mine]
echo.
echo   SEED_IP       IP/hostname of a hosted seed node (P2P port 48080 assumed)
echo   --threads N   Mining threads (default: 2)
echo   --no-mine     Sync only, do not mine
echo.
endlocal
exit /b 0
