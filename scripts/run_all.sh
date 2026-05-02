#!/bin/bash
set -e

cd "$(dirname "$0")/.."

./scripts/build.sh

mkdir -p results

echo "=== Baseline ==="
./build/baseline 10000 > results/baseline_results.jsonl
python3 tools/summarize.py results/baseline_results.jsonl
echo ""

echo "=== Fixed Buffers ==="
./build/fixed_buffers_sim 10000 > results/fixed_buffers_results.jsonl
python3 tools/summarize.py results/fixed_buffers_results.jsonl
echo ""

echo "=== Polling vs Interrupt ==="
echo "Interrupt (Mode 0):"
./build/polling_sim 10000 0 > results/interrupt_results.jsonl
python3 tools/summarize.py results/interrupt_results.jsonl

echo ""
echo "Polling (Mode 1):"
./build/polling_sim 10000 1 > results/polling_results.jsonl
python3 tools/summarize.py results/polling_results.jsonl
