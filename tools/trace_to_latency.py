import sys
import re
import os

def parse_trace(filename):
    if not os.path.exists(filename):
        print(f"Error: {filename} not found.")
        return

    # Mapping of pid -> active request info
    active_requests = {}
    completed_requests = []
    
    # Track global state for correlation
    last_irq_ts = 0

    with open(filename, 'r') as f:
        for line in f:
            parts = line.split()
            if not parts or parts[0] == "TIME_NS" or parts[0] == "Tracing":
                continue
                
            try:
                ts = int(parts[0])
                pid = int(parts[1])
                comm = parts[2]
                event = parts[3]
                
                if event == "marker:":
                    action = parts[4]
                    req_id = int(parts[5])
                    
                    if action == "REQ_START":
                        active_requests[pid] = {
                            'id': req_id,
                            'start_ts': ts,
                            'events': {}
                        }
                    elif action == "REQ_END":
                        if pid in active_requests:
                            req = active_requests[pid]
                            req['end_ts'] = ts
                            completed_requests.append(req)
                            del active_requests[pid]
                
                # Correlation logic
                
                # 1. IRQs (likely completion signals if they happen before wakeup)
                if event.startswith("irq_handler_entry"):
                    last_irq_ts = ts
                
                # 2. Wakeups targeting our processes
                if event == "sched_wakeup:":
                    target_pid_part = [p for p in parts if p.startswith("target_pid=")][0]
                    target_pid = int(target_pid_part.split('=')[1])
                    if target_pid in active_requests:
                        active_requests[target_pid]['events']['wakeup'] = ts
                        if last_irq_ts:
                            active_requests[target_pid]['events']['completion_irq'] = last_irq_ts

                # 3. Scheduler switches into our processes
                if event == "sched_switch:":
                    next_pid_part = [p for p in parts if p.startswith("next_pid=")][0]
                    next_pid = int(next_pid_part.split('=')[1])
                    if next_pid in active_requests:
                        active_requests[next_pid]['events']['sched_in'] = ts

                # 4. Syscall entries
                if event == "sys_enter_io_uring_enter":
                    if pid in active_requests:
                        active_requests[pid]['events']['syscall_enter'] = ts

            except Exception:
                continue

    if not completed_requests:
        print("No completed requests correlated. Ensure the benchmark was running and markers were captured.")
        return

    print(f"--- Request Attribution Summary (N={len(completed_requests)}) ---")
    print(f"{'ID':<6} {'Total(µs)':<12} {'IRQ->Wake(µs)':<15} {'Wake->Sched(µs)':<15}")
    print("-" * 60)
    
    wakeup_to_sched_delays = []

    for req in completed_requests[:20]: # Show sample
        e = req['events']
        total_us = (req['end_ts'] - req['start_ts']) / 1000.0
        
        irq_to_wake = (e['wakeup'] - e['completion_irq']) / 1000.0 if ('wakeup' in e and 'completion_irq' in e) else 0
        wake_to_sched = (e['sched_in'] - e['wakeup']) / 1000.0 if ('sched_in' in e and 'wakeup' in e) else 0
        
        if wake_to_sched > 0:
            wakeup_to_sched_delays.append(e['sched_in'] - e['wakeup'])
            
        print(f"{req['id']:<6} {total_us:<12.1f} {irq_to_wake:<15.1f} {wake_to_sched:<15.1f}")

    # Final derived metrics
    if wakeup_to_sched_delays:
        avg_ns = sum(wakeup_to_sched_delays) / len(wakeup_to_sched_delays)
        print(f"\nMetric: wakeup_to_sched_ns (avg): {avg_ns:.0f} ns")
    else:
        print("\nMetric: wakeup_to_sched_ns could not be calculated (missing events).")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 tools/trace_to_latency.py <trace.log>")
    else:
        parse_trace(sys.argv[1])
