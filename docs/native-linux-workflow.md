# Native Linux Workflow: End-to-End Attribution

This workflow demonstrates how to correlate userspace requests with kernel events using `trace_marker` and `bpftrace`.

## Steps

### 1. Terminal 1: Start the Trace
Start `bpftrace` to capture all relevant kernel events and userspace markers.
```bash
sudo bpftrace trace/bpf/full_path.bt > results/native_trace.log
```

### 2. Terminal 2: Run the Benchmark
Run the benchmark with a fixed number of iterations.
```bash
taskset -c 0 ./build/baseline 10000
```

### 3. Stop Tracing
Return to Terminal 1 and hit `Ctrl-C` to stop the trace.

### 4. Analyze results
Run the attribution tool to correlate the request IDs with kernel events.
```bash
python3 tools/trace_to_latency.py results/native_trace.log
```

## Expected Output
The tool will produce a table showing the lifecycle of each request, including the critical `wakeup_to_sched_ns` metric.

```text
--- Request Attribution Summary (N=10000) ---
ID     Total(µs)    IRQ->Wake(µs)   Wake->Sched(µs)
------------------------------------------------------------
1      40.5         5.2             2.1
2      41.2         4.8             3.5
...

Metric: wakeup_to_sched_ns (avg): 2850 ns
```
