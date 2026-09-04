@echo off
setlocal EnableExtensions EnableDelayedExpansion

rem ======================================================================
rem  RandomLite (XRL) Seed Node Host
rem  Runs a persistent, publicly reachable seed node.
rem  Usage : host-seed-node.bat [--mainnet] [--data-dir PATH] [--no-firewall]
rem  Note  : firewall rule creation requires an elevated (admin) shell.
rem ======================================================================

set "ROOT=%~dp0"
set "BIN=%ROOT%build\monero-xrl\bin"
set "NET=--testnet"
set "NETNAME=testnet"
set "P2P_PORT=48080"
set "RPC_PORT=48081"
set "ZMQ_PORT=48082"
set "DATA=%ROOT%seed-node-data"
set "DO_FIREWALL=1"

:parse_args
if "%~1"=="" goto args_done
if /i "%~1"=="--mainnet"     ( set "NET=" & set "NETNAME=mainnet" & set "P2P_PORT=28080" & set "RPC_PORT=28081" & set "ZMQ_PORT=28082" & shift & goto parse_args )
if /i "%~1"=="--data-dir"    ( set "DATA=%~2" & shift & shift & goto parse_args )
if /i "%~1"=="--no-firewall" ( set "DO_FIREWALL=0" & shift & goto parse_args )
if /i "%~1"=="--help"        ( goto show_help )
shift
goto parse_args
:args_done

rem ---- runtime DLLs (binaries have staged DLLs, but keep PATH as fallback) ----
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
echo   RandomLite XRL Seed Node  (%NETNAME%)
echo   P2P (public) : 0.0.0.0:%P2P_PORT%
echo   RPC (local)  : 127.0.0.1:%RPC_PORT%   [not exposed]
echo   ZMQ (local)  : 127.0.0.1:%ZMQ_PORT%   [not exposed]
echo   Data dir     : %DATA%
echo ===================================================

rem ---- firewall rule for inbound P2P (best effort, needs admin) ----
if "%DO_FIREWALL%"=="1" (
  netsh advfirewall firewall show rule name="XRL P2P %P2P_PORT%" >NUL 2>NUL
  if errorlevel 1 (
    echo [INFO] Creating inbound firewall rule for TCP %P2P_PORT% ^(needs admin^)...
    netsh advfirewall firewall add rule name="XRL P2P %P2P_PORT%" dir=in action=allow protocol=TCP localport=%P2P_PORT% >NUL 2>NUL
    if errorlevel 1 (
      echo [WARN] Could not add firewall rule. Re-run as Administrator, or open TCP %P2P_PORT% manually.
    ) else (
      echo [OK] Firewall rule added: XRL P2P %P2P_PORT%
    )
  ) else (
    echo [OK] Firewall rule already exists.
  )
)

echo.
echo [INFO] Starting seed node. Keep this window open 24/7.
echo [INFO] Miners join with: monerod %NET% --add-peer ^<THIS_HOST_IP^>:%P2P_PORT%
echo.

"%BIN%\monerod.exe" %NET% --data-dir "%DATA%" --disable-dns-checkpoints ^
  --p2p-bind-ip 0.0.0.0 --p2p-bind-port %P2P_PORT% ^
  --rpc-bind-ip 127.0.0.1 --rpc-bind-port %RPC_PORT% ^
  --zmq-rpc-bind-ip 127.0.0.1 --zmq-rpc-bind-port %ZMQ_PORT% ^
  --log-level 1

echo.
echo [INFO] Seed node stopped.
pause
endlocal
exit /b 0

:show_help
echo.
echo RandomLite XRL Seed Node Host
echo.
echo   host-seed-node.bat [options]
echo.
echo   --mainnet       Host mainnet seed (default: testnet)
echo   --data-dir PATH Custom blockchain data directory
echo   --no-firewall   Skip Windows Firewall rule creation
echo   --help          Show this help
echo.
endlocal
exit /b 0
