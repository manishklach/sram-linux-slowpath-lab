#!/bin/bash

# io_uring Kernel Patch Validation Script
# Measures latency difference between Normal and LOW_LATENCY mode.

ITERATIONS=5000
RESULTS_DIR="results"
mkdir -p $RESULTS_DIR

echo "Building benchmark..."
gcc -O3 benchmarks/io_uring_real.c -o benchmarks/io_uring_real

# 1. Run Normal Mode
echo "Running Normal mode..."
./benchmarks/io_uring_real 0 $ITERATIONS > $RESULTS_DIR/io_uring_normal.jsonl

# 2. Run Low-Latency Mode
echo "Running Low-Latency mode..."
# Note: If this fails with -EINVAL, the kernel patch is not applied or active.
./benchmarks/io_uring_real 1 $ITERATIONS > $RESULTS_DIR/io_uring_lowlat.jsonl 2>/dev/null

if [ $? -ne 0 ]; then
    echo "ERROR: Low-Latency mode failed. Is the kernel patch applied?"
    exit 1
fi

echo "Results saved to $RESULTS_DIR/"
echo "Summary:"
# Basic stats if summarize.py exists, or use python one-liner
python3 -c "
import json, sys, numpy as np
def stats(path):
    with open(path) as f:
        data = [json.loads(line)['total_ns'] for line in f]
    return np.mean(data), np.percentile(data, 50), np.percentile(data, 99), np.percentile(data, 99.9)

n_mean, n_p50, n_p99, n_p999 = stats('$RESULTS_DIR/io_uring_normal.jsonl')
l_mean, l_p50, l_p99, l_p999 = stats('$RESULTS_DIR/io_uring_lowlat.jsonl')

print(f'Mode        Mean    p50     p99     p999')
print(f'Normal      {n_mean/1000:7.2f} {n_p50/1000:7.2f} {n_p99/1000:7.2f} {n_p999/1000:7.2f} us')
print(f'LowLat      {l_mean/1000:7.2f} {l_p50/1000:7.2f} {l_p99/1000:7.2f} {l_p999/1000:7.2f} us')
"
