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
    long start = get_ns();
    while (get_ns() - start < 20000) {
        // busy wait
    }
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    int polling_mode = 0; // 0 for interrupt, 1 for polling
    if (argc > 1) {
        iterations = atoi(argv[1]);
    }
    if (argc > 2) {
        polling_mode = atoi(argv[2]);
    }

    // Setup outside loop
    long start_mem = get_ns();
    while(get_ns() - start_mem < 5000) {}

    srand(time(NULL));

    for (int i = 1; i <= iterations; i++) {
        long t0 = get_ns();
        long t1 = get_ns(); // fixed buffer simulation
        
        simulate_sram_execution();
        long t2 = get_ns();
        
        long t3;
        if (polling_mode) {
            // Polling: very fast completion, deterministic
            long start_completion = get_ns();
            while (get_ns() - start_completion < 1000) {} // 1us polling overhead
            t3 = get_ns();
        } else {
            // Interrupt: variable delay (context switch, softirq overhead)
            long start_completion = get_ns();
            long delay = 3000 + (rand() % 5000); // 3us to 8us variable delay
            while (get_ns() - start_completion < delay) {}
            t3 = get_ns();
        }
        
        long submit_ns = t1 - t0;
        long device_ns = t2 - t1;
        long completion_ns = t3 - t2;
        long total_ns = t3 - t0;
        
        printf("{\"iter\":%d,\"submit_ns\":%ld,\"device_ns\":%ld,\"completion_ns\":%ld,\"total_ns\":%ld}\n",
               i, submit_ns, device_ns, completion_ns, total_ns);
    }
    return 0;
}
