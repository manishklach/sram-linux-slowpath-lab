import sys
import os

def parse_trace(filename):
    if not os.path.exists(filename):
        print(f"Error: {filename} not found.")
        return

    event_counts = {}
    timestamps = []
    
    with open(filename, 'r') as f:
        # Skip header if present
        for line in f:
            parts = line.split()
            if not parts or parts[0] == "TIME_NS" or parts[0] == "Tracing":
                continue
                
            try:
                # Time is parts[0]
                ts = int(parts[0])
                timestamps.append(ts)
                
                # Event name is parts[3]
                event = parts[3]
                # Clean up if it ends with ':'
                if event.endswith(':'):
                    event = event[:-1]
                    
                event_counts[event] = event_counts.get(event, 0) + 1
            except:
                continue

    if not timestamps:
        print("No valid events found in trace.")
        return

    print(f"--- Trace Summary: {filename} ---")
    print(f"Total events: {len(timestamps)}")
    print(f"Duration:     {(max(timestamps) - min(timestamps))/1e9:.2f} seconds")
    print("\nEvent Counts:")
    for event, count in sorted(event_counts.items(), key=lambda x: x[1], reverse=True):
        print(f"  {event:<30}: {count}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 tools/parse_trace.py <trace_output_file>")
    else:
        parse_trace(sys.argv[1])
