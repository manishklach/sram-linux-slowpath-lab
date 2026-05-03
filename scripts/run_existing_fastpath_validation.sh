#!/bin/bash

# Validation script for existing io_uring fast paths
# Evaluates Baseline, SQPOLL, and Fixed Resources across NOP and SRAM20 workloads.

ITERATIONS=${1:-1000}
OUTPUT_DIR="results"
mkdir -p $OUTPUT_DIR
mkdir -p trace_logs

MODES=("A" "B" "C" "E")
DESCS=("baseline" "sqpoll" "sqpoll_buf" "sqpoll_buf_file")
WORKLOADS=("nop" "sram20")

echo "Starting existing fast-path validation (N=$ITERATIONS)..."

for WORKLOAD in "${WORKLOADS[@]}"; do
    echo "--- Track: $WORKLOAD ---"
    for i in "${!MODES[@]}"; do
        MODE=${MODES[$i]}
        DESC=${DESCS[$i]}
        
        echo "[*] Running mode $MODE ($DESC) | Workload: $WORKLOAD..."
        
        # Start tracing in background (commented out for WSL/non-sudo compatibility)
        # sudo bpftrace trace/bpf/io_uring_latency.bt > "trace_logs/trace_${WORKLOAD}_${DESC}.log" 2>/dev/null &
        # BPF_PID=$!
        # sleep 2
        
        # Run benchmark with new CLI options
        ./benchmarks/io_uring_real --mode="$MODE" --iters="$ITERATIONS" --workload="$WORKLOAD" > "$OUTPUT_DIR/${WORKLOAD}_${DESC}.jsonl"
        
        # Stop tracing
        # sleep 1
        # sudo kill $BPF_PID
        # wait $BPF_PID 2>/dev/null
        
        # echo "[+] Done. Analyzing trace..."
        # if [ -f "trace_logs/trace_${WORKLOAD}_${DESC}.log" ]; then
        #     python3 tools/io_uring_trace_to_latency.py "trace_logs/trace_${WORKLOAD}_${DESC}.log" > "$OUTPUT_DIR/analysis_${WORKLOAD}_${DESC}.txt"
        # fi
    done
done

echo "Validation complete. Results are in $OUTPUT_DIR/"
