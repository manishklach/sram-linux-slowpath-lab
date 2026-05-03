#!/bin/bash
for b in 1 2 4 8 16 32; do
  echo "Running batch size: $b"
  taskset -c 0 ./benchmarks/io_uring_real --workload=sram20 --mode=A --batch=$b --iters=5000 > results/batch_$b.jsonl
done
