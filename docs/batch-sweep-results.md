# Batch Sweep Optimization Results

## Goal
Quantify the relationship between batch size and host overhead to find the optimal 
operating point for deterministic ~20µs workloads.

## Results Table

| Batch | total p50 (µs) | total p99 (µs) | submit p50 (µs) | submit p99 (µs) | cost/request (ns) |
|-------|----------------|----------------|-----------------|-----------------|-------------------|
| 1     | 20.6           | 56.5           | 0.47            | 7.4             | 471               |
| 2     | 20.8           | 71.9           | 0.67            | 9.0             | 335               |
| 4     | 20.8           | 59.3           | 0.70            | 8.1             | 175               |
| 8     | 21.1           | 54.6           | 0.95            | 8.9             | 119               |
| 16    | 21.5           | 54.2           | 1.39            | 9.8             | 87                |
| 32    | 22.6           | 53.6           | 2.36            | 10.0            | 74                |

## Optimal Batch Size

Based on the data, the **optimal batch size is between 8 and 16**.

### 1. Small Batches (1–4)
- **High Overhead**: The per-request submission tax is >150ns. 
- **Syscall Dominance**: The fixed cost of entering the kernel is paid too frequently.

### 2. Large Batches (32+)
- **Diminishing Returns**: The improvement in `cost/request` slows significantly (only 13ns 
  gain from 16 to 32).
- **Latency Increase**: The total p50 latency begins to climb (from 21.5µs to 22.6µs) as 
  the kernel takes longer to process the larger ring segment and manage the request pool.

### 3. The Sweet Spot (8–16)
- **Saturation**: Submission overhead is successfully driven below 100ns per request.
- **Stability**: p99 total latency remains stable and actually shows slight improvement 
  compared to small batches due to better unit-scheduling in the kernel.

## Interpretation

System design for deterministic inference must balance three factors:
1. **Per-request Latency**: Increases with batch size due to serialization and processing.
2. **Throughput**: Improves as submission overhead is amortized.
3. **Batching Efficiency**: Saturates quickly; pushing beyond batch 16 provides marginal 
   gains at the cost of increased base latency.

**Recommendation**: Use a batch size of **16** for maximum submission efficiency without 
sacrificing microsecond-scale latency targets.
