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
 * Real io_uring benchmark for kernel patch validation.
 * Performs IORING_OP_NOP operations and measures completion latency.
 */

#ifndef IORING_SETUP_LOW_LATENCY
#define IORING_SETUP_LOW_LATENCY (1U << 20)
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
    /* Fallback for other architectures might need adjustment */
    return syscall(__NR_io_uring_enter, fd, to_submit, min_complete, flags, sig);
#endif
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

int main(int argc, char *argv[]) {
    int iterations = 1000;
    int use_low_latency = 0;
    if (argc > 1) use_low_latency = atoi(argv[1]);
    if (argc > 2) iterations = atoi(argv[2]);

    struct io_uring_params p = { };
    if (use_low_latency) p.flags |= IORING_SETUP_LOW_LATENCY;

    struct submitter s;
    if (setup_context(&s, 32, &p)) return 1;

    fprintf(stderr, "Starting %d iterations in %s mode...\n", iterations, use_low_latency ? "LOW_LATENCY" : "NORMAL");

    for (int i = 1; i <= iterations; i++) {
        unsigned tail = *s.sq_ring.tail;
        unsigned index = tail & *s.sq_ring.ring_mask;
        struct io_uring_sqe *sqe = &s.sqes[index];

        memset(sqe, 0, sizeof(*sqe));
        sqe->opcode = IORING_OP_NOP;
        sqe->user_data = i;

        s.sq_ring.array[index] = index;
        __atomic_store_n(s.sq_ring.tail, tail + 1, __ATOMIC_RELEASE);

        uint64_t t_start = now_ns();
        
        /* Submit and wait for 1 completion */
        int ret = io_uring_enter(s.ring_fd, 1, 1, IORING_ENTER_GETEVENTS, NULL);
        if (ret < 0) {
            perror("io_uring_enter");
            break;
        }
        
        uint64_t t_end = now_ns();

        unsigned head = *s.cq_ring.head;
        struct io_uring_cqe *cqe = &s.cq_ring.cqes[head & *s.cq_ring.ring_mask];
        __atomic_store_n(s.cq_ring.head, head + 1, __ATOMIC_RELEASE);

        printf("{\"iter\":%d,\"request_id\":%lu,\"total_ns\":%lu}\n",
               i, (unsigned long)cqe->user_data, (unsigned long)(t_end - t_start));
    }

    close(s.ring_fd);
    return 0;
}
