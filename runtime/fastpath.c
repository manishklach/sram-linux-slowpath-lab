#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

/* 
 * Fast Path Prototype
 * 
 * This program models a kernel-optimized inference path.
 * 
 * It simulates:
 * - Persistent DMA mapping (pre-allocated buffers, no GUP/dma_map in hot path)
 * - Polling completion (no interrupt signaling or context switch delay)
 * - Dedicated inference thread (models a thread pinned to a core with no interference)
 * - No syscall overhead (simulating userspace-driven doorbell or io_uring SQPOLL)
 */

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

int main(int argc, char *argv[]) {
    int iterations = 10000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
    }

    // setup(): pre-allocate all resources
    // In this prototype, that means no setup occurs inside the loop.
    // In a real system, this would happen during initial buffer registration.

    for (int i = 1; i <= iterations; i++) {
        uint64_t t0 = now_ns();
        
        // submission: no per-request setup or syscall
        // only a fast register write/doorbell (simulated by minimal delta)
        uint64_t t1 = now_ns(); 
        
        // device execution: deterministic compute
        simulate_sram_execution();
        uint64_t t2 = now_ns();
        
        // completion: polling-based, very fast
        // simulating a tight poll loop over a completion queue
        busy_wait_ns(500); // 500ns for polling/queue management overhead
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
