import sys
import json
import os
import matplotlib.pyplot as plt
import numpy as np

def plot_file(filename):
    stages = ['submit_ns', 'device_ns', 'completion_ns', 'total_ns']
    data = {s: [] for s in stages}
    
    with open(filename, 'r') as f:
        for line in f:
            try:
                row = json.loads(line)
                for s in stages:
                    data[s].append(row[s] / 1000.0) # convert to us
            except:
                continue

    if not data['total_ns']:
        return

    os.makedirs('results/plots', exist_ok=True)
    basename = os.path.basename(filename).replace('.jsonl', '')
    
    # 1. Histograms
    plt.figure(figsize=(12, 8))
    for i, s in enumerate(stages):
        plt.subplot(2, 2, i+1)
        plt.hist(data[s], bins=100, alpha=0.7, color='steelblue', edgecolor='black')
        plt.title(f"{s} (µs)")
        plt.xlabel('Latency (µs)')
        plt.ylabel('Frequency')
        plt.grid(True, alpha=0.3)
    
    plt.suptitle(f'Latency Distribution - {basename}')
    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    plt.savefig(f'results/plots/{basename}_hists.png')
    plt.close()

    # 2. Stacked latency breakdown
    p_levels = [50, 95, 99, 99.9]
    total = np.array(data['total_ns'])
    
    submit_p = []
    device_p = []
    completion_p = []
    
    sorted_idx = np.argsort(total)
    
    for p in p_levels:
        idx = sorted_idx[min(len(total) - 1, int(len(total) * p / 100))]
        submit_p.append(data['submit_ns'][idx])
        device_p.append(data['device_ns'][idx])
        completion_p.append(data['completion_ns'][idx])

    labels = [f"p{p}" for p in p_levels]
    width = 0.5
    
    fig, ax = plt.subplots(figsize=(10, 6))
    
    # Stacked bars
    ax.bar(labels, submit_p, width, label='Submit (Host Prep)', color='#2ecc71')
    ax.bar(labels, device_p, width, bottom=submit_p, label='Device (SRAM Compute)', color='#3498db')
    ax.bar(labels, completion_p, width, bottom=np.array(submit_p)+np.array(device_p), label='Completion (Host Wakeup)', color='#e74c3c')
    
    ax.set_ylabel('Latency (µs)')
    ax.set_title(f'Latency Attribution Breakdown - {basename}')
    ax.legend(loc='upper left')
    plt.grid(axis='y', alpha=0.3)
    
    # Add labels on top of segments
    for i in range(len(labels)):
        ax.text(i, submit_p[i]/2, f'{submit_p[i]:.1f}', ha='center', va='center', color='white', fontweight='bold')
        ax.text(i, submit_p[i] + device_p[i]/2, f'{device_p[i]:.1f}', ha='center', va='center', color='white', fontweight='bold')
        ax.text(i, submit_p[i] + device_p[i] + completion_p[i]/2, f'{completion_p[i]:.1f}', ha='center', va='center', color='white', fontweight='bold')

    plt.savefig(f'results/plots/{basename}_breakdown.png')
    plt.close()
    print(f"Generated plots for {basename} in results/plots/")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 tools/plot_latency.py <jsonl_files>")
    else:
        for arg in sys.argv[1:]:
            plot_file(arg)
