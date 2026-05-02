#!/bin/bash

# io_uring Low-Latency Mode Validation Script
# Requires a kernel with the IORING_SETUP_LOW_LATENCY patch applied.

ITERATIONS=5000
RESULTS_DIR="results"
mkdir -p $RESULTS_DIR

echo "Building benchmark..."
gcc -O3 benchmarks/low_latency_ring.c -o benchmarks/low_latency_ring

echo "Step 1: Running Normal io_uring Mode..."
./benchmarks/low_latency_ring 0 $ITERATIONS > $RESULTS_DIR/io_uring_normal.jsonl

echo "Step 2: Running Low-Latency io_uring Mode..."
# This may fail with -EINVAL if the kernel patch is not applied.
./benchmarks/low_latency_ring 1 $ITERATIONS > $RESULTS_DIR/io_uring_low_latency.jsonl 2>/dev/null

if [ $? -ne 0 ]; then
    echo "WARNING: Low-Latency mode failed. Is the kernel patch applied?"
    exit 1
fi

echo "Step 3: Comparing Results..."
# Reusing the existing compare.py if it supports these JSONL files
# Or we can just print a summary here.
python3 scripts/compare.py $RESULTS_DIR/io_uring_normal.jsonl $RESULTS_DIR/io_uring_low_latency.jsonl

echo "Validation complete. See $RESULTS_DIR for raw data."
