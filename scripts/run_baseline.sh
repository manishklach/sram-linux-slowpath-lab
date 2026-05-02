#!/bin/bash
set -e

cd "$(dirname "$0")/.."

mkdir -p results
echo "Running baseline..."
./build/baseline 10000 > results/baseline_results.jsonl
python3 tools/summarize.py results/baseline_results.jsonl
