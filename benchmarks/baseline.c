#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <stdint.h>

static inline uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return ((uint64_t) ts.tv_sec * 1000000000ull) + (uint64_t) ts.tv_nsec;
}

static inline void busy_wait_ns(uint64_t target_ns) {
    uint64_t start = now_ns();
    while ((now_ns() - start) < target_ns) {
#if defined(__x86_64__) || defined(__i386__)
        __asm__ volatile("pause");
#else
        __asm__ volatile("" ::: "memory");
#endif
    }
}

void simulate_sram_execution() {
    // 20 microseconds deterministic execution
    busy_wait_ns(20000);
}

void simulate_memory_setup() {
    // 10us for memory setup
    busy_wait_ns(10000);
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
        uint64_t t0 = now_ns();
        
        simulate_memory_setup();
        uint64_t t1 = now_ns();
        
        simulate_sram_execution();
        uint64_t t2 = now_ns();
        
        // Simulating completion overhead
        long target_delay = get_completion_delay_ns();
        busy_wait_ns(target_delay);
        uint64_t t3 = now_ns();
        
        uint64_t submit_ns = t1 - t0;
        uint64_t device_ns = t2 - t1;
        uint64_t completion_ns = t3 - t2;
        uint64_t total_ns = t3 - t0;
        
        printf("{\"iter\":%d,\"submit_ns\":%lu,\"device_ns\":%lu,\"completion_ns\":%lu,\"total_ns\":%lu}\n",
               i, submit_ns, device_ns, completion_ns, total_ns);
    }
    return 0;
}
