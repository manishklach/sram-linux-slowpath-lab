#!/bin/bash
set -e

cd "$(dirname "$0")/.."

echo "Building benchmarks..."
./scripts/build.sh

mkdir -p results

echo "Running Baseline (Pinned)..."
taskset -c 0 ./build/baseline 10000 > results/baseline_pinned.jsonl

echo "Running Fixed Buffers (Pinned)..."
taskset -c 0 ./build/fixed_buffers_sim 10000 > results/fixed_buffers_pinned.jsonl

echo "Running Interrupt Sim (Pinned)..."
taskset -c 0 ./build/polling_sim 10000 0 > results/interrupt_pinned.jsonl

echo "Running Polling Sim (Pinned)..."
taskset -c 0 ./build/polling_sim 10000 1 > results/polling_pinned.jsonl

echo ""
echo "=== Comparison Report ==="
python3 tools/compare.py
