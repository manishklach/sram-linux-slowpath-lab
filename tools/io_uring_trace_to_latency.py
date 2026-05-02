import sys
import os
import json

def parse_trace(filename):
    if not os.path.exists(filename):
        print(f"Error: {filename} not found.")
        return

    # Tracking req -> events
    # We use a dict to store data for each request pointer re-used over time
    reqs = {}
    processed = []
    
    with open(filename, 'r') as f:
        for line in f:
            parts = line.split()
            if not parts or parts[0] == "TIME" or parts[0] == "Tracing":
                continue
                
            try:
                ts = int(parts[0])
                pid = int(parts[1])
                event = parts[2]
                req_ptr = parts[3]
                
                if event == "ISSUE":
                    lat_us = int(parts[4])
                    reqs[req_ptr] = {
                        'issue_ts': ts,
                        'issue_lat_us': lat_us,
                        'low_latency': 0
                    }
                elif event == "COMPLETE":
                    total_lat_us = int(parts[4])
                    low_latency = int(parts[5])
                    
                    if req_ptr in reqs:
                        r = reqs[req_ptr]
                        r['complete_ts'] = ts
                        r['total_lat_us'] = total_lat_us
                        r['low_latency'] = low_latency
                        # Calculate issue->complete if possible
                        if 'issue_ts' in r:
                            r['processing_lat_us'] = (ts - r['issue_ts']) / 1000.0
                        
                        processed.append(r)
                        del reqs[req_ptr]
            except:
                continue

    if not processed:
        print("No correlated requests found in trace.")
        return

    lowlat = [r for r in processed if r['low_latency'] == 1]
    normal = [r for r in processed if r['low_latency'] == 0]

    def print_stats(name, data):
        if not data:
            print(f"\n{name}: No data found.")
            return
            
        import numpy as np
        
        totals = [r['total_lat_us'] for r in data]
        sub_to_issue = [r['issue_lat_us'] for r in data if 'issue_lat_us' in r]
        issue_to_comp = [r['processing_lat_us'] for r in data if 'processing_lat_us' in r]
        
        print(f"\n--- {name} Mode Results (N={len(data)}) ---")
        print(f"{'Metric':<20} {'Mean':<10} {'p50':<10} {'p99':<10} {'p99.9':<10}")
        print("-" * 60)
        
        def row(label, vals):
            if not vals: return
            print(f"{label:<20} {np.mean(vals):<10.1f} {np.percentile(vals, 50):<10.1f} {np.percentile(vals, 99):<10.1f} {np.percentile(vals, 99.9):<10.1f}")

        row("Submit -> Total", totals)
        row("Submit -> Issue", sub_to_issue)
        row("Issue -> Complete", issue_to_comp)

    print_stats("NORMAL", normal)
    print_stats("LOW_LATENCY", lowlat)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 tools/io_uring_trace_to_latency.py <trace.log>")
    else:
        parse_trace(sys.argv[1])
