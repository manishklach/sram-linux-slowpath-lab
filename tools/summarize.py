import sys
import json

def process_data(file_obj):
    records = []
    for line in file_obj:
        line = line.strip()
        if not line:
            continue
        try:
            records.append(json.loads(line))
        except json.JSONDecodeError:
            pass

    if not records:
        print("No valid JSON records found.")
        return

    keys = ["submit_ns", "device_ns", "complete_ns", "total_ns"]
    
    print(f"{'Metric':<15} | {'Min':<8} | {'p50':<8} | {'p95':<8} | {'p99':<8} | {'p999':<8} | {'Max':<8} | {'Avg':<8}")
    print("-" * 85)
    
    for key in keys:
        if key not in records[0]:
            continue
            
        values = sorted([r[key] for r in records])
        n = len(values)
        
        v_min = values[0]
        v_max = values[-1]
        v_avg = sum(values) / n
        v_p50 = values[int(n * 0.50)]
        v_p95 = values[int(n * 0.95)]
        v_p99 = values[int(n * 0.99)]
        v_p999 = values[int(n * 0.999)]
        
        print(f"{key:<15} | {v_min:<8} | {v_p50:<8} | {v_p95:<8} | {v_p99:<8} | {v_p999:<8} | {v_max:<8} | {v_avg:<8.2f}")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        with open(sys.argv[1], 'r') as f:
            process_data(f)
    else:
        process_data(sys.stdin)
