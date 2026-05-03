import json
import os
import numpy as np
import matplotlib.pyplot as plt

def get_metrics(filename):
    total_ns = []
    submit_ns = []
    batch_size = 1
    
    with open(filename, 'r') as f:
        for line in f:
            data = json.loads(line)
            total_ns.append(data['total_ns'])
            submit_ns.append(data['submit_ns'])
            batch_size = data.get('batch', 1)
            
    return {
        'batch': batch_size,
        'total_p50': np.percentile(total_ns, 50) / 1000.0,
        'total_p99': np.percentile(total_ns, 99) / 1000.0,
        'submit_p50': np.percentile(submit_ns, 50),
        'cost_per_req': np.percentile(submit_ns, 50) / batch_size
    }

batches = [1, 2, 4, 8, 16, 32]
results = []

for b in batches:
    fname = f'results/batch_{b}.jsonl'
    if os.path.exists(fname):
        results.append(get_metrics(fname))

x = [r['batch'] for r in results]
total_p50 = [r['total_p50'] for r in results]
total_p99 = [r['total_p99'] for r in results]
cost_per_req = [r['cost_per_req'] for r in results]
spread = [r['total_p99'] - r['total_p50'] for r in results]

os.makedirs('docs/assets', exist_ok=True)

# Plot 1: Latency vs Batch
plt.figure(figsize=(10, 6))
plt.plot(x, total_p50, marker='o', label='p50')
plt.plot(x, total_p99, marker='s', label='p99')
plt.xlabel('Batch Size')
plt.ylabel('Latency (µs)')
plt.title('Batch Size vs End-to-End Latency')
plt.grid(True, alpha=0.3)
plt.legend()
plt.savefig('docs/assets/batch_latency.png')
plt.close()

# Plot 2: Submission Cost
plt.figure(figsize=(10, 6))
plt.plot(x, cost_per_req, marker='o')
plt.xlabel('Batch Size')
plt.ylabel('Cost per Request (ns)')
plt.title('Batch Size vs Submission Cost per Request')
plt.grid(True, alpha=0.3)
plt.savefig('docs/assets/submission_cost.png')
plt.close()

# Plot 3: Tail Spread
plt.figure(figsize=(10, 6))
plt.plot(x, spread, marker='o')
plt.xlabel('Batch Size')
plt.ylabel('p99 - p50 Spread (µs)')
plt.title('Batch Size vs Tail Latency Spread')
plt.grid(True, alpha=0.3)
plt.savefig('docs/assets/tail_spread.png')
plt.close()

print("Graphs generated in docs/assets/")
