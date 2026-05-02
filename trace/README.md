# Native Linux Tracing with bpftrace

This directory contains `bpftrace` scripts designed for use on a native Linux installation to capture kernel-side events related to inference request processing.

## Prerequisites

On a native Ubuntu machine, install `bpftrace`:
```bash
sudo apt update
sudo apt install -y bpftrace
```

## How to use

1. **Terminal 1**: Start the tracing script (requires root).
   ```bash
   sudo bpftrace trace/bpf/full_path.bt > trace_output.txt
   ```

2. **Terminal 2**: Run the benchmark.
   ```bash
   taskset -c 0 ./build/baseline 10000
   ```

3. **Terminal 1**: Stop the tracing with `Ctrl-C`.

4. **Analysis**: Use the parser to summarize the events.
   ```bash
   python3 tools/parse_trace.py trace_output.txt
   ```

## Included Scripts

- `full_path.bt`: Comprehensive trace of syscalls, IRQs, softirqs, and scheduler events.
- `sched_latency.bt`: Specifically measures the wakeup-to-schedule delay for the benchmark processes.
- `irq_softirq.bt`: Measures the execution duration of interrupt and softirq handlers.
