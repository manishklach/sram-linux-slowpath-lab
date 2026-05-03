import sys
import os
import json
import numpy as np

def parse_trace(filename):
    if not os.path.exists(filename):
        print(f"Error: {filename} not found.")
        return

    # Data structures for correlation
    reqs = {} # req_ptr -> data
    ctxs = {} # ctx_ptr -> data
    tasks = {} # pid -> data
    
    processed = []
    
    with open(filename, 'r') as f:
        for line in f:
            parts = line.split()
            if not parts or parts[0] == "TIME" or not parts[0].isdigit():
                continue
                
            try:
                ts = int(parts[0])
                pid = int(parts[1])
                event = parts[2]
                target = parts[3]
                
                if event == "ISSUE":
                    req_ptr = target
                    lat_us = int(parts[4])
                    fixed = int(parts[6])
                    reqs[req_ptr] = {
                        'submit_to_issue_us': lat_us,
                        'issue_ts': ts,
                        'fixed_buffers': fixed,
                        'pid': pid
                    }
                elif event == "COMPLETE":
                    req_ptr = target
                    total_lat_us = int(parts[4])
                    low_latency = int(parts[5])
                    fixed = int(parts[6])
                    
                    if req_ptr in reqs:
                        r = reqs[req_ptr]
                        r['complete_ts'] = ts
                        r['total_lat_us'] = total_lat_us
                        r['low_latency'] = low_latency
                        r['issue_to_complete_us'] = (ts - r['issue_ts']) / 1000.0
                        
                        # Store in processed but keep for wakeup correlation
                        # (req_ptr is unique for active request)
                        processed.append(r)
                        # We don't delete yet because we need to link ctx to this request
                elif event == "CQ_WAKEUP":
                    ctx_ptr = target
                    wakeup_lat_us = int(parts[4])
                    # Associate with last completed requests on this ctx
                    # In our benchmark, it's 1-at-a-time, so we can find the most recent
                    # completed request that doesn't have a wakeup yet.
                    # For simplicity, just update the last one.
                    if processed:
                        processed[-1]['complete_to_wakeup_us'] = wakeup_lat_us
                        processed[-1]['wakeup_ts'] = ts
                elif event == "SCHED_IN":
                    target_pid = int(target)
                    sched_lat_us = int(parts[4])
                    if processed and processed[-1]['pid'] == target_pid:
                         processed[-1]['wakeup_to_sched_us'] = sched_lat_us
            except Exception as e:
                # print(f"Error parsing line: {line.strip()} -> {e}")
                continue

    if not processed:
        print("No correlated requests found in trace.")
        return

    def print_stats(name, data):
        if not data: return
        
        metrics = {
            "Submit -> Total": [r.get('total_lat_us') for r in data if 'total_lat_us' in r],
            "Submit -> Issue": [r.get('submit_to_issue_us') for r in data if 'submit_to_issue_us' in r],
            "Issue -> Complete": [r.get('issue_to_complete_us') for r in data if 'issue_to_complete_us' in r],
            "Complete -> Wakeup": [r.get('complete_to_wakeup_us') for r in data if 'complete_to_wakeup_us' in r],
            "Wakeup -> Sched": [r.get('wakeup_to_sched_us') for r in data if 'wakeup_to_sched_us' in r],
        }
        
        print(f"\n--- {name} Mode Results (N={len(data)}) ---")
        print(f"{'Metric':<20} {'Mean':<10} {'p50':<10} {'p99':<10} {'p99.9':<10}")
        print("-" * 65)
        
        for label, vals in metrics.items():
            vals = [v for v in vals if v is not None]
            if not vals: continue
            print(f"{label:<20} {np.mean(vals):<10.1f} {np.percentile(vals, 50):<10.1f} {np.percentile(vals, 99):<10.1f} {np.percentile(vals, 99.9):<10.1f}")

    print_stats("OVERALL", processed)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 tools/io_uring_trace_to_latency.py <trace.log>")
    else:
        parse_trace(sys.argv[1])
