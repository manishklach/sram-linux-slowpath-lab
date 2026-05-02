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
    while (get_ns() - start < 10000) {
        // busy wait 10us for memory setup
    }
}

long get_completion_delay_ns() {
    int r = rand() % 10000;
    long delay = 5000; // reduced base

    if (r < 5000) {
        delay += 4000; // ~9k
    } else if (r < 9500) {
        delay += 4000 + (r - 5000) * 11000 / 4500; // up to 15k
    } else if (r < 9900) {
        delay += 15000 + (r - 9500) * 10000 / 400; // up to 25k
    } else if (r < 9990) {
        delay += 25000 + (r - 9900) * 10000 / 90; // up to 35k
    } else {
        delay += 35000 + (rand() % 10000);
    }
    return delay;
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
    }

    srand(time(NULL));

    for (int i = 1; i <= iterations; i++) {
        long t0 = get_ns();
        
        simulate_memory_setup();
        long t1 = get_ns();
        
        simulate_sram_execution();
        long t2 = get_ns();
        
        // Simulating completion overhead
        long start_completion = get_ns();
        long target_delay = get_completion_delay_ns();
        while (get_ns() - start_completion < target_delay) {}
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
