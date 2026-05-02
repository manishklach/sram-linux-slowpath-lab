#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

long get_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return ts.tv_sec * 1000000000L + ts.tv_nsec;
}

void simulate_sram_execution() {
    // 20 microseconds deterministic execution
    long start = get_ns();
    while (get_ns() - start < 20000) {
        // busy wait
    }
}

void simulate_memory_setup() {
    long start = get_ns();
    while (get_ns() - start < 5000) {
        // busy wait 5us for memory setup
    }
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
    }

    for (int i = 1; i <= iterations; i++) {
        long t0 = get_ns();
        
        simulate_memory_setup();
        long t1 = get_ns();
        
        simulate_sram_execution();
        long t2 = get_ns();
        
        // Simulating completion overhead
        long start_completion = get_ns();
        while (get_ns() - start_completion < 3000) {} // 3us completion
        long t3 = get_ns();
        
        long submit_ns = t1 - t0;
        long device_ns = t2 - t1;
        long completion_ns = t3 - t2;
        long total_ns = t3 - t0;
        
        printf("{\"iter\":%d,\"submit_ns\":%ld,\"device_ns\":%ld,\"completion_ns\":%ld,\"total_ns\":%ld}\n",
               i, submit_ns, device_ns, completion_ns, total_ns);
    }
    return 0;
}
