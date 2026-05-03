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

/*
 * Advanced io_uring benchmark for validating existing fast paths.
 * Modes: Baseline, SQPOLL, REGISTER_BUFFERS, IOPOLL, Fixed Files.
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
        case 'D': return "IOPOLL";
        case 'E': return "SQPOLL + REGISTER_BUFFERS + FIXED_FILES";
        default: return "UNKNOWN";
    }
}

int main(int argc, char *argv[]) {
    int iterations = 1000;
    char mode = 'A';
    
    if (argc > 1) mode = argv[1][0];
    if (argc > 2) iterations = atoi(argv[2]);

    struct io_uring_params p = { };
    p.flags |= IORING_SETUP_LOW_LATENCY;

    if (mode == 'B' || mode == 'C' || mode == 'E') {
        p.flags |= IORING_SETUP_SQPOLL;
    }
    if (mode == 'D') {
        p.flags |= IORING_SETUP_IOPOLL;
    }

    struct submitter s;
    if (setup_context(&s, 32, &p)) return 1;

    void *buf = NULL;
    if (mode == 'C' || mode == 'E') {
        buf = aligned_alloc(4096, 4096);
        memset(buf, 0, 4096);
        struct iovec iov = { .iov_base = buf, .iov_len = 4096 };
        if (io_uring_register(s.ring_fd, IORING_REGISTER_BUFFERS, &iov, 1) < 0) {
            perror("io_uring_register(BUFFERS)");
            // Continue anyway, but expect non-fixed path
        }
    }

    if (mode == 'E') {
        int fds[1] = { s.ring_fd };
        if (io_uring_register(s.ring_fd, IORING_REGISTER_FILES, fds, 1) < 0) {
            perror("io_uring_register(FILES)");
        }
    }

    fprintf(stderr, "Starting %d iterations in mode %c (%s)...\n", iterations, mode, mode_to_str(mode));

    for (int i = 1; i <= iterations; i++) {
        unsigned tail = *s.sq_ring.tail;
        unsigned index = tail & *s.sq_ring.ring_mask;
        struct io_uring_sqe *sqe = &s.sqes[index];

        memset(sqe, 0, sizeof(*sqe));
        sqe->opcode = IORING_OP_NOP;
        sqe->user_data = i;

        if (mode == 'C' || mode == 'E') {
            sqe->addr = (uintptr_t)buf;
            sqe->len = 4096;
            sqe->buf_index = 0;
            /* Even for NOP, setting these can trigger the fixed-buffer code paths in the kernel */
        }
        
        if (mode == 'E') {
            sqe->flags |= IOSQE_FIXED_FILE;
            sqe->fd = 0; // index 0 in registered files
        } else {
            sqe->fd = -1;
        }

        s.sq_ring.array[index] = index;
        __atomic_store_n(s.sq_ring.tail, tail + 1, __ATOMIC_RELEASE);

        uint64_t t_start = now_ns();
        
        /* 
         * For SQPOLL, we don't necessarily need to call enter to submit,
         * but we need it to wait for completions if we want to block.
         */
        int enter_flags = IORING_ENTER_GETEVENTS;
        if (!(p.flags & IORING_SETUP_SQPOLL)) {
            // Non-SQPOLL needs explicit submission if we don't use GETEVENTS for it
        }

        int ret = io_uring_enter(s.ring_fd, 1, 1, enter_flags, NULL);
        if (ret < 0 && errno != EBUSY) {
            perror("io_uring_enter");
            break;
        }
        
        uint64_t t_end = now_ns();

        /* Consume completion */
        unsigned head = *s.cq_ring.head;
        unsigned tail_val;
        while (head == (tail_val = __atomic_load_n(s.cq_ring.tail, __ATOMIC_ACQUIRE))) {
            // Busy wait if enter returned early (can happen with SQPOLL/IOPOLL)
            if (now_ns() - t_start > 1000000000ull) {
                fprintf(stderr, "Timeout waiting for completion\n");
                break;
            }
            __builtin_ia32_pause();
        }

        struct io_uring_cqe *cqe = &s.cq_ring.cqes[head & *s.cq_ring.ring_mask];
        __atomic_store_n(s.cq_ring.head, head + 1, __ATOMIC_RELEASE);

        printf("{\"iter\":%d,\"mode\":\"%c\",\"request_id\":%lu,\"total_ns\":%lu}\n",
               i, mode, (unsigned long)cqe->user_data, (unsigned long)(t_end - t_start));
    }

    close(s.ring_fd);
    if (buf) free(buf);
    return 0;
}
