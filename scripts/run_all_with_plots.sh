#!/bin/bash
set -e

cd "$(dirname "$0")/.."

echo "Running full experiment suite with visualization..."

# Build
./scripts/build.sh

mkdir -p results

# Run all pinned benchmarks
echo "Running benchmarks..."
taskset -c 0 ./build/baseline 10000 > results/baseline_pinned.jsonl
taskset -c 0 ./build/fixed_buffers_sim 10000 > results/fixed_buffers_pinned.jsonl
taskset -c 0 ./build/polling_sim 10000 0 > results/interrupt_pinned.jsonl
taskset -c 0 ./build/polling_sim 10000 1 > results/polling_pinned.jsonl

echo "Generating comparison report..."
python3 tools/compare.py > results/comparison_table.md
cat results/comparison_table.md

echo "Generating percentile breakdowns..."
python3 tools/percentiles.py results/*_pinned.jsonl > results/percentiles_breakdown.txt
cat results/percentiles_breakdown.txt

echo "Generating plots..."
# Note: Requires matplotlib installed
python3 tools/plot_latency.py results/*_pinned.jsonl

echo "Experiment complete. See results/ and results/plots/"
