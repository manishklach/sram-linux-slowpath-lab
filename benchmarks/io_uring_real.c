#define _GNU_SOURCE
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
#include <stdbool.h>
#include <errno.h>
#include <signal.h>
#include <getopt.h>

/*
 * Advanced io_uring benchmark for validating existing fast paths.
 * Supports NOP (raw overhead) and SRAM20 (deterministic inference) tracks.
 */

#ifndef IORING_SETUP_LOW_LATENCY
#define IORING_SETUP_LOW_LATENCY (1U << 20)
#endif

#ifndef IORING_REGISTER_BUFFERS
#define IORING_REGISTER_BUFFERS 0
#endif

#ifndef IORING_REGISTER_FILES
#define IORING_REGISTER_FILES 2
#endif

enum workload_type {
    WORKLOAD_NOP,
    WORKLOAD_SRAM20
};

struct app_sq_ring {
    unsigned *head;
    unsigned *tail;
    unsigned *ring_mask;
    unsigned *ring_entries;
    unsigned *flags;
    unsigned *dropped;
    unsigned *array;
};

struct app_cq_ring {
    unsigned *head;
    unsigned *tail;
    unsigned *ring_mask;
    unsigned *ring_entries;
    struct io_uring_cqe *cqes;
};

struct submitter {
    int ring_fd;
    struct app_sq_ring sq_ring;
    struct io_uring_sqe *sqes;
    struct app_cq_ring cq_ring;
};

static int io_uring_setup(unsigned entries, struct io_uring_params *p) {
    return syscall(__NR_io_uring_setup, entries, p);
}

static int io_uring_enter(int fd, unsigned to_submit, unsigned min_complete, unsigned flags, sigset_t *sig) {
#if defined(__x86_64__)
    return syscall(__NR_io_uring_enter, fd, to_submit, min_complete, flags, sig, _NSIG / 8);
#else
    return syscall(__NR_io_uring_enter, fd, to_submit, min_complete, flags, sig);
#endif
}

static int io_uring_register(int fd, unsigned opcode, void *arg, unsigned nr_args) {
    return syscall(__NR_io_uring_register, fd, opcode, arg, nr_args);
}

static inline uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return ((uint64_t) ts.tv_sec * 1000000000ull) + (uint64_t) ts.tv_nsec;
}

static void busy_wait_ns(uint64_t ns) {
    uint64_t start = now_ns();
    while (now_ns() - start < ns) {
        __builtin_ia32_pause();
    }
}

int setup_context(struct submitter *s, unsigned entries, struct io_uring_params *p) {
    int ret = io_uring_setup(entries, p);
    if (ret < 0) {
        perror("io_uring_setup");
        return 1;
    }
    s->ring_fd = ret;

    void *sq_ptr = mmap(0, p->sq_off.array + p->sq_entries * sizeof(__u32),
                        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE,
                        s->ring_fd, IORING_OFF_SQ_RING);
    
    s->sqes = mmap(0, p->sq_entries * sizeof(struct io_uring_sqe),
                   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE,
                   s->ring_fd, IORING_OFF_SQES);

    void *cq_ptr = mmap(0, p->cq_off.cqes + p->cq_entries * sizeof(struct io_uring_cqe),
                        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE,
                        s->ring_fd, IORING_OFF_CQ_RING);

    s->sq_ring.head = (unsigned *)((char *)sq_ptr + p->sq_off.head);
    s->sq_ring.tail = (unsigned *)((char *)sq_ptr + p->sq_off.tail);
    s->sq_ring.ring_mask = (unsigned *)((char *)sq_ptr + p->sq_off.ring_mask);
    s->sq_ring.ring_entries = (unsigned *)((char *)sq_ptr + p->sq_off.ring_entries);
    s->sq_ring.flags = (unsigned *)((char *)sq_ptr + p->sq_off.flags);
    s->sq_ring.array = (unsigned *)((char *)sq_ptr + p->sq_off.array);

    s->cq_ring.head = (unsigned *)((char *)cq_ptr + p->cq_off.head);
    s->cq_ring.tail = (unsigned *)((char *)cq_ptr + p->cq_off.tail);
    s->cq_ring.ring_mask = (unsigned *)((char *)cq_ptr + p->cq_off.ring_mask);
    s->cq_ring.ring_entries = (unsigned *)((char *)cq_ptr + p->cq_off.ring_entries);
    s->cq_ring.cqes = (struct io_uring_cqe *)((char *)cq_ptr + p->cq_off.cqes);

    return 0;
}

static char *mode_to_str(int mode) {
    switch (mode) {
        case 'A': return "BASELINE";
        case 'B': return "SQPOLL";
        case 'C': return "SQPOLL + REGISTER_BUFFERS";
        case 'E': return "SQPOLL + REGISTER_BUFFERS + FIXED_FILES";
        default: return "UNKNOWN";
    }
}

int main(int argc, char *argv[]) {
    int iterations = 1000;
    int batch_size = 1;
    int adaptive = 0;
    char mode = 'A';
    enum workload_type workload = WORKLOAD_NOP;

    static struct option long_options[] = {
        {"mode", required_argument, 0, 'm'},
        {"iters", required_argument, 0, 'i'},
        {"workload", required_argument, 0, 'w'},
        {"batch", required_argument, 0, 'b'},
        {"adaptive", no_argument, 0, 'a'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "m:i:w:b:a", long_options, NULL)) != -1) {
        switch (opt) {
            case 'm': mode = optarg[0]; break;
            case 'i': iterations = atoi(optarg); break;
            case 'b': batch_size = atoi(optarg); break;
            case 'a': adaptive = 1; break;
            case 'w':
                if (strcmp(optarg, "nop") == 0) workload = WORKLOAD_NOP;
                else if (strcmp(optarg, "sram20") == 0) workload = WORKLOAD_SRAM20;
                break;
        }
    }

    struct io_uring_params p = { };
    p.flags |= IORING_SETUP_LOW_LATENCY;

    if (mode == 'B' || mode == 'C' || mode == 'E') {
        p.flags |= IORING_SETUP_SQPOLL;
    }

    struct submitter s;
    if (setup_context(&s, 32, &p)) {
        if (errno == EINVAL && (p.flags & IORING_SETUP_LOW_LATENCY)) {
            fprintf(stderr, "IORING_SETUP_LOW_LATENCY not supported, falling back to NORMAL...\n");
            p.flags &= ~IORING_SETUP_LOW_LATENCY;
            if (setup_context(&s, 32, &p)) return 1;
        } else {
            return 1;
        }
    }

    void *buf = NULL;
    if (mode == 'C' || mode == 'E') {
        buf = aligned_alloc(4096, 4096);
        memset(buf, 0, 4096);
        struct iovec iov = { .iov_base = buf, .iov_len = 4096 };
        if (io_uring_register(s.ring_fd, IORING_REGISTER_BUFFERS, &iov, 1) < 0) {
            perror("io_uring_register(BUFFERS)");
        }
    }

    if (mode == 'E') {
        int fds[1] = { s.ring_fd };
        if (io_uring_register(s.ring_fd, IORING_REGISTER_FILES, fds, 1) < 0) {
            perror("io_uring_register(FILES)");
        }
    }

    fprintf(stderr, "Starting %d iterations | Mode: %s | Workload: %s\n", 
            iterations, mode_to_str(mode), workload == WORKLOAD_NOP ? "nop" : "sram20");

    int current_batch = batch_size;
    for (int i = 1; i <= iterations; i++) {
        uint64_t t_start = now_ns();
        
        for (int b = 0; b < current_batch; b++) {
            unsigned tail = *s.sq_ring.tail;
            unsigned index = tail & *s.sq_ring.ring_mask;
            struct io_uring_sqe *sqe = &s.sqes[index];

            memset(sqe, 0, sizeof(*sqe));
            sqe->opcode = IORING_OP_NOP;
            sqe->user_data = i * 100 + b;

            if (mode == 'C' || mode == 'E') {
                sqe->addr = (uintptr_t)buf;
                sqe->len = 4096;
                sqe->buf_index = 0;
            }
            
            if (mode == 'E') {
                sqe->flags |= IOSQE_FIXED_FILE;
                sqe->fd = 0;
            } else {
                sqe->fd = -1;
            }

            s.sq_ring.array[index] = index;
            __atomic_store_n(s.sq_ring.tail, tail + 1, __ATOMIC_RELEASE);
        }

        int enter_flags = IORING_ENTER_GETEVENTS;
        if (p.flags & IORING_SETUP_SQPOLL) {
            if (*s.sq_ring.flags & IORING_SQ_NEED_WAKEUP)
                enter_flags |= IORING_ENTER_SQ_WAKEUP;
        }

        int ret = io_uring_enter(s.ring_fd, current_batch, current_batch, enter_flags, NULL);
        if (ret < 0 && errno != EBUSY) {
            perror("io_uring_enter");
            break;
        }
        
        uint64_t t_after_submit = now_ns();

        /* SRAM20 deterministic compute model */
        if (workload == WORKLOAD_SRAM20) {
            busy_wait_ns(20000); // 20us
        }
        uint64_t t_after_device = now_ns();

        /* Consume all completions in batch */
        for (int b = 0; b < current_batch; b++) {
            unsigned head = *s.cq_ring.head;
            unsigned tail_val;
            while (head == (tail_val = __atomic_load_n(s.cq_ring.tail, __ATOMIC_ACQUIRE))) {
                if (now_ns() - t_start > 1000000000ull) {
                    fprintf(stderr, "Timeout waiting for completion\n");
                    break;
                }
                __builtin_ia32_pause();
            }
            __atomic_store_n(s.cq_ring.head, head + 1, __ATOMIC_RELEASE);
        }
        
        uint64_t t_end = now_ns();
        uint64_t total_ns = t_end - t_start;

        if (workload == WORKLOAD_NOP) {
            printf("{\"iter\":%d,\"mode\":\"%c\",\"workload\":\"nop\",\"batch\":%d,\"total_ns\":%lu}\n",
                   i, mode, current_batch, (unsigned long)total_ns);
        } else {
            printf("{\"iter\":%d,\"mode\":\"%c\",\"workload\":\"sram20\",\"batch\":%d,\"submit_ns\":%lu,\"device_ns\":%lu,\"complete_ns\":%lu,\"total_ns\":%lu}\n",
                   i, mode, current_batch,
                   (unsigned long)(t_after_submit - t_start), 
                   (unsigned long)(t_after_device - t_after_submit),
                   (unsigned long)(t_end - t_after_device),
                   (unsigned long)total_ns);
        }
        fflush(stdout);

        if (adaptive) {
            // Target total latency: ~22-25us
            if (total_ns > 25000) {
                if (current_batch > 1) current_batch--;
            } else if (total_ns < 22000) {
                if (current_batch < 32) current_batch++;
            }
        }
    }

    close(s.ring_fd);
    if (buf) free(buf);
    return 0;
}
