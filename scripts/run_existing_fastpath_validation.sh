#!/bin/bash

# Validation script for existing io_uring fast paths
# Evaluates Baseline, SQPOLL, and Fixed Resources.

ITERATIONS=${1:-1000}
OUTPUT_DIR="results"
mkdir -p $OUTPUT_DIR
mkdir -p trace_logs

# Modes:
# A: baseline
# B: SQPOLL
# C: SQPOLL + registered buffers
# E: SQPOLL + registered buffers + fixed files

MODES=("A" "B" "C" "E")
DESCS=("baseline" "sqpoll" "sqpoll_buf" "sqpoll_buf_file")

echo "Starting existing fast-path validation (N=$ITERATIONS)..."

for i in "${!MODES[@]}"; do
    MODE=${MODES[$i]}
    DESC=${DESCS[$i]}
    
    echo "[*] Running mode $MODE ($DESC)..."
    
    # Start tracing in background
    sudo bpftrace trace/bpf/io_uring_latency.bt > "trace_logs/trace_$DESC.log" 2>/dev/null &
    BPF_PID=$!
    sleep 2
    
    # Run benchmark
    ./benchmarks/io_uring_real "$MODE" "$ITERATIONS" > "$OUTPUT_DIR/$DESC.jsonl"
    
    # Stop tracing
    sleep 1
    sudo kill $BPF_PID
    wait $BPF_PID 2>/dev/null
    
    echo "[+] Done. Analyzing trace..."
    python3 tools/io_uring_trace_to_latency.py "trace_logs/trace_$DESC.log" > "$OUTPUT_DIR/analysis_$DESC.txt"
done

echo "Validation complete. Results are in $OUTPUT_DIR/"
