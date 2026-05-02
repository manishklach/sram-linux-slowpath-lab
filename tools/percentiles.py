import sys
import json
import os

def analyze(filename):
    if not os.path.exists(filename):
        return
    
    stages = ['submit_ns', 'device_ns', 'completion_ns', 'total_ns']
    data = {s: [] for s in stages}
    
    with open(filename, 'r') as f:
        for line in f:
            try:
                row = json.loads(line)
                for s in stages:
                    data[s].append(row[s])
            except:
                continue
                
    if not data['total_ns']:
        return

    # Sort indices based on total_ns to get percentiles of total latency
    total = data['total_ns']
    sorted_indices = sorted(range(len(total)), key=lambda k: total[k])
    n = len(total)
    
    print(f"\n--- {filename} ---")
    for label, p in [('p50', 0.5), ('p99', 0.99), ('p999', 0.999)]:
        idx = sorted_indices[min(n - 1, int(n * p))]
        print(f"{label}:")
        print(f"  total:      {total[idx]/1000:6.1f}µs")
        print(f"  submit:     {data['submit_ns'][idx]/1000:6.1f}µs")
        print(f"  device:     {data['device_ns'][idx]/1000:6.1f}µs")
        print(f"  completion: {data['completion_ns'][idx]/1000:6.1f}µs")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 tools/percentiles.py <jsonl_files>")
    else:
        for arg in sys.argv[1:]:
            analyze(arg)
