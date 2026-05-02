#!/bin/bash
set -e

cd "$(dirname "$0")/.."

echo "Building fastpath runtime..."
mkdir -p build
gcc -O3 runtime/fastpath.c -o build/fastpath

mkdir -p results

echo "Running Fast Path (Pinned)..."
taskset -c 0 ./build/fastpath 10000 > results/fastpath_pinned.jsonl

echo "Done. Results in results/fastpath_pinned.jsonl"
