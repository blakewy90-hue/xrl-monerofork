@echo off
setlocal

where cmake >NUL 2>NUL
if errorlevel 1 (
  echo [ERROR] CMake is required to build the Monero-based XRL fork.
  exit /b 1
)

rem Prefer Ninja + LLVM-MinGW when available (matches existing build\monero-xrl)
for /d %%D in ("%LOCALAPPDATA%\Microsoft\WinGet\Packages\MartinStorsjo.LLVM-MinGW.UCRT*") do (
  for /d %%M in ("%%D\llvm-mingw-*-ucrt-x86_64") do set "MINGW_BIN=%%M\bin"
)
if defined MINGW_BIN set "PATH=%MINGW_BIN%;%PATH%"

for /d %%D in ("%LOCALAPPDATA%\Microsoft\WinGet\Packages\Ninja-build.Ninja*") do (
  if exist "%%D\ninja.exe" set "PATH=%%D;%PATH%"
)

if not exist build\monero-xrl mkdir build\monero-xrl

if not exist build\monero-xrl\build.ninja if not exist build\monero-xrl\Makefile (
  where ninja >NUL 2>NUL
  if not errorlevel 1 (
    cmake -S vendor\monero -B build\monero-xrl -G Ninja ^
      -DCMAKE_BUILD_TYPE=Release ^
      -DMANUAL_SUBMODULES=1 ^
      -DBUILD_TESTS=OFF ^
      -DBUILD_DOCUMENTATION=OFF ^
      -DBUILD_DEBUG_UTILITIES=OFF ^
      -DBUILD_GUI=OFF ^
      -DBUILD_GUI_DEPS=OFF ^
      -DUSE_READLINE=OFF ^
      -DUSE_DEVICE_TREZOR=OFF ^
      -DUSE_DEVICE_LEDGER=OFF
  ) else (
    cmake -S vendor\monero -B build\monero-xrl -G "MinGW Makefiles" ^
      -DCMAKE_BUILD_TYPE=Release ^
      -DMANUAL_SUBMODULES=1 ^
      -DBUILD_TESTS=OFF ^
      -DBUILD_DOCUMENTATION=OFF ^
      -DBUILD_DEBUG_UTILITIES=OFF ^
      -DBUILD_GUI=OFF ^
      -DBUILD_GUI_DEPS=OFF ^
      -DUSE_READLINE=OFF ^
      -DUSE_DEVICE_TREZOR=OFF ^
      -DUSE_DEVICE_LEDGER=OFF
  )
  if errorlevel 1 exit /b %errorlevel%
)

cmake --build build\monero-xrl --target daemon simplewallet wallet_rpc_server xrl-genesis-tool --parallel
if errorlevel 1 exit /b %errorlevel%

rem --- Stage runtime DLLs next to the binaries so monerod/wallet run without PATH hacks ---
set "BIN=build\monero-xrl\bin"
if not "%MINGW_BIN%"=="" (
  copy /y "%MINGW_BIN%\*.dll" "%BIN%\" >NUL 2>NUL
)
if exist vendor\vcpkg\installed\x64-mingw-dynamic\bin (
  copy /y vendor\vcpkg\installed\x64-mingw-dynamic\bin\*.dll "%BIN%\" >NUL 2>NUL
)
if exist vendor\vcpkg\downloads\unbound-stage\mingw64\bin (
  copy /y vendor\vcpkg\downloads\unbound-stage\mingw64\bin\*.dll "%BIN%\" >NUL 2>NUL
)

echo.
echo ===================================================
echo   XRL Monero fork build successful
echo   Daemon       : build\monero-xrl\bin\monerod.exe
echo   Wallet CLI   : build\monero-xrl\bin\monero-wallet-cli.exe
echo   Solo testnet : run-testnet.bat
echo ===================================================
