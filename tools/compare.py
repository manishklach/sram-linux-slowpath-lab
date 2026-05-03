import sys
import json
import os

def get_stats(filename):
    latencies = []
    if not os.path.exists(filename):
        return None
    with open(filename, 'r') as f:
        for line in f:
            try:
                data = json.loads(line)
                latencies.append(data['total_ns'])
            except:
                continue
    if not latencies:
        return None
    latencies.sort()
    n = len(latencies)
    return {
        'p50': latencies[int(n * 0.5)],
        'p95': latencies[int(n * 0.95)],
        'p99': latencies[int(n * 0.99)],
        'p999': latencies[int(n * 0.999)],
        'max': latencies[-1],
        'avg': sum(latencies) / n
    }

def format_us(ns):
    if ns >= 1000000:
        return f"{ns / 1000000:.2f}ms"
    return f"{ns / 1000:.1f}µs"

def print_table(title, files):
    results = {}
    for name, path in files.items():
        stats = get_stats(path)
        if stats:
            results[name] = stats
            
    if not results:
        return

    print(f"\n### {title}")
    print("| Mode      | p50     | p95     | p99     | p999    | Max     | Avg     | vs Baseline |")
    print("|-----------|---------|---------|---------|---------|---------|---------|-------------|")
    
    baseline_stats = results.get('Baseline')
    baseline_p50 = baseline_stats['p50'] if baseline_stats else None
    
    for name in ['Baseline', 'SQPOLL', 'SQPOLL_Buf', 'SQPOLL_Fixed']:
        if name not in results:
            continue
        s = results[name]
        
        improvement = ""
        if baseline_p50 and name != 'Baseline':
            imp = (baseline_p50 - s['p50']) / baseline_p50 * 100
            improvement = f"{imp:+.1f}%"
            
        print(f"| {name:<9} | {format_us(s['p50']):<7} | {format_us(s['p95']):<7} | {format_us(s['p99']):<7} | {format_us(s['max']):<7} | {format_us(s['avg']):<7} | {improvement:<11} |")

def main():
    tracks = {
        "NOP Track (Raw Overhead)": {
            'Baseline': 'results/nop_baseline.jsonl',
            'SQPOLL': 'results/nop_sqpoll.jsonl',
            'SQPOLL_Buf': 'results/nop_sqpoll_buf.jsonl',
            'SQPOLL_Fixed': 'results/nop_sqpoll_buf_file.jsonl'
        },
        "SRAM20 Track (Deterministic Inference)": {
            'Baseline': 'results/sram20_baseline.jsonl',
            'SQPOLL': 'results/sram20_sqpoll.jsonl',
            'SQPOLL_Buf': 'results/sram20_sqpoll_buf.jsonl',
            'SQPOLL_Fixed': 'results/sram20_sqpoll_buf_file.jsonl'
        }
    }
    
    for title, files in tracks.items():
        print_table(title, files)

if __name__ == "__main__":
    main()
