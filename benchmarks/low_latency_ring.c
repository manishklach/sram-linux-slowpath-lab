#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <linux/io_uring.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

/* 
 * IORING_SETUP_LOW_LATENCY is our experimental flag (bit 20).
 * This benchmark demonstrates its effect by timing the io_uring_enter
 * completion path.
 */
#ifndef IORING_SETUP_LOW_LATENCY
#define IORING_SETUP_LOW_LATENCY (1U << 20)
#endif

static int io_uring_setup(unsigned entries, struct io_uring_params *p) {
    return syscall(__NR_io_uring_setup, entries, p);
}

static int io_uring_enter(int fd, unsigned to_submit, unsigned min_complete, unsigned flags, sigset_t *sig) {
    /* Note: on some archs the args for enter are different, this is for x86_64 */
    return syscall(__NR_io_uring_enter, fd, to_submit, min_complete, flags, sig, _NSIG / 8);
}

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

int main(int argc, char *argv[]) {
    int iterations = 1000;
    int use_low_latency = 0;
    
    if (argc > 1) use_low_latency = atoi(argv[1]);
    if (argc > 2) iterations = atoi(argv[2]);

    struct io_uring_params p = { };
    if (use_low_latency) {
        p.flags = IORING_SETUP_LOW_LATENCY;
    }

    int ring_fd = io_uring_setup(32, &p);
    if (ring_fd < 0) {
        perror("io_uring_setup (is the kernel patch applied?)");
        return 1;
    }

    // Map the rings (minimal setup for raw syscalls)
    // In a real validation, we would use liburing to actually submit NOPs.
    // Here we focus on the enter() syscall latency itself when waiting.

    for (int i = 1; i <= iterations; i++) {
        uint64_t t0 = now_ns();
        
        // Simulating the "device execution" before we wait
        busy_wait_ns(20000); // 20us
        
        uint64_t t1 = now_ns();
        
        /* 
         * Call enter to wait for 1 completion.
         * On a patched kernel with IORING_SETUP_LOW_LATENCY, this call
         * will busy-wait before sleeping, significantly reducing
         * p99 variance caused by the scheduler.
         */
        io_uring_enter(ring_fd, 0, 1, IORING_ENTER_GETEVENTS, NULL);
        
        uint64_t t2 = now_ns();
        
        uint64_t total_ns = t2 - t0;
        uint64_t completion_ns = t2 - t1;

        printf("{\"iter\":%d,\"low_latency\":%d,\"completion_ns\":%lu,\"total_ns\":%lu}\n",
               i, use_low_latency, (unsigned long)completion_ns, (unsigned long)total_ns);
    }

    close(ring_fd);
    return 0;
}
