# Quickstart Guide

This guide provides the steps to build and run the validation harness in a synthetic 
environment (like WSL) before moving to native hardware.

## 1. Build the Harness
Compile the benchmarks and simulation tools:
```bash
./scripts/build.sh
```

## 2. Run Validation (WSL / Sanity Check)
Execute the existing fast-path validation script. This runs both the `nop` and `sram20` 
workload tracks across all supported `io_uring` modes.
```bash
./scripts/run_existing_fastpath_validation.sh 1000
```

## 3. Compare Results
Analyze the generated JSONL files to see the latency distribution.
```bash
python3 tools/compare.py results/*.jsonl
```

## What to Expect
- **NOP Track**: Sub-microsecond overhead. This establishes the absolute "floor" of 
  `io_uring` on your host.
- **SRAM20 Track**: ~20µs of deterministic compute plus host-side overhead. You should 
  observe that p99 latency significantly exceeds the 20µs floor.

## Next Steps
Once you have verified the harness on a local machine, proceed to 
[Native Linux Validation](native-linux-validation.md) for high-fidelity attribution.
