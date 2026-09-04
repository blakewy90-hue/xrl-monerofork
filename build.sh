#!/usr/bin/env bash
set -e

echo "==================================================="
echo "  Building RandomLite (XRL) Fast-Block Daemon"
echo "==================================================="

mkdir -p build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo "Build successful! Executable is located at build/randomlited"
