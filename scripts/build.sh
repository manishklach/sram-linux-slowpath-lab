#!/bin/bash
set -e

# Change to the root of the lab directory
cd "$(dirname "$0")/.."

mkdir -p build
gcc -O3 benchmarks/baseline.c -o build/baseline
gcc -O3 benchmarks/fixed_buffers_sim.c -o build/fixed_buffers_sim
gcc -O3 benchmarks/polling_sim.c -o build/polling_sim
gcc -O3 benchmarks/io_uring_real.c -o benchmarks/io_uring_real

echo "Build complete."
