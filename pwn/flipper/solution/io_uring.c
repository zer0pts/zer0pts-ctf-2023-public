/* 
 * References:
 * https://github.com/shuveb/io_uring-by-example/blob/master/02_cat_uring/main.c
 * https://unixism.net/loti/low_level.html
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include "io_uring.h"
#include "util.h"

int io_uring_register(int fd, unsigned int opcode, void *arg,
		      unsigned int nr_args)
{
	return syscall(__NR_io_uring_register, fd, opcode, arg, nr_args);
}

int io_uring_setup(unsigned int entries, struct io_uring_params *p)
{
	return syscall(__NR_io_uring_setup, entries, p);
}

int io_uring_enter(int fd, unsigned int to_submit, unsigned int min_complete,
		   unsigned int flags)
{
	return syscall(__NR_io_uring_enter, fd, to_submit, min_complete,
			flags, NULL, 0);    // no signals
}

int uring_create_raw(size_t n_sqe, size_t n_cqe)
{
    struct io_uring_params p = {
        .cq_entries = n_cqe,
        .flags = IORING_SETUP_CQSIZE
    };

    int res = io_uring_setup(n_sqe, &p);
    if (res < 0)
        fatal("io_uring_setup() failed");
    return res;
}

int app_setup_uring(struct submitter *s, unsigned int entries)
{
    struct app_io_sq_ring *sring = &s->sq_ring;
    struct app_io_cq_ring *cring = &s->cq_ring;
    struct io_uring_params p;
    void *sq_ptr, *cq_ptr;

    /*
     * We need to pass in the io_uring_params structure to the io_uring_setup()
     * call zeroed out. We could set any flags if we need to, but for this
     * example, we don't.
     * */
    memset(&p, 0, sizeof(p));
    //p.flags = IORING_SETUP_SQPOLL;
    //p.sq_thread_idle = 1000;
    p.wq_fd = -1;
    s->ring_fd = io_uring_setup(entries, &p);     // SQ/CQ with at least 1 entry
    if (s->ring_fd < 0) {
        perror("io_uring_setup");
        return 1;
    }

    /*
     * io_uring communication happens via 2 shared kernel-user space ring buffers,
     * which can be jointly mapped with a single mmap() call in recent kernels. 
     * While the completion queue is directly manipulated, the submission queue 
     * has an indirection array in between. We map that in as well.
     * */

    int sring_sz = p.sq_off.array + p.sq_entries * sizeof(unsigned);
    int cring_sz = p.cq_off.cqes + p.cq_entries * sizeof(struct io_uring_cqe);

    /* In kernel version 5.4 and above, it is possible to map the submission and 
     * completion buffers with a single mmap() call. Rather than check for kernel 
     * versions, the recommended way is to just check the features field of the 
     * io_uring_params structure, which is a bit mask. If the 
     * IORING_FEAT_SINGLE_MMAP is set, then we can do away with the second mmap()
     * call to map the completion ring.
     * */
    if (p.features & IORING_FEAT_SINGLE_MMAP) {
        if (cring_sz > sring_sz) {
            sring_sz = cring_sz;
        }
        cring_sz = sring_sz;
    }

    /* Map in the submission and completion queue ring buffers.
     * Older kernels only map in the submission queue, though.
     * */
    sq_ptr = mmap(0, sring_sz, PROT_READ | PROT_WRITE, 
            MAP_SHARED | MAP_POPULATE,
            s->ring_fd, IORING_OFF_SQ_RING);
    if (sq_ptr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    if (p.features & IORING_FEAT_SINGLE_MMAP) {
        cq_ptr = sq_ptr;
    } else {
        /* Map in the completion queue ring buffer in older kernels separately */
        cq_ptr = mmap(0, cring_sz, PROT_READ | PROT_WRITE, 
                MAP_SHARED | MAP_POPULATE,
                s->ring_fd, IORING_OFF_CQ_RING);
        if (cq_ptr == MAP_FAILED) {
            perror("mmap");
            return 1;
        }
    }
    /* Save useful fields in a global app_io_sq_ring struct for later
     * easy reference */
    sring->head = sq_ptr + p.sq_off.head;
    sring->tail = sq_ptr + p.sq_off.tail;
    sring->ring_mask = sq_ptr + p.sq_off.ring_mask;
    sring->ring_entries = sq_ptr + p.sq_off.ring_entries;
    sring->flags = sq_ptr + p.sq_off.flags;
    sring->array = sq_ptr + p.sq_off.array;

    /* Save useful fields in a global app_io_cq_ring struct for later
     * easy reference */
    cring->head = cq_ptr + p.cq_off.head;
    cring->tail = cq_ptr + p.cq_off.tail;
    cring->ring_mask = cq_ptr + p.cq_off.ring_mask;
    cring->ring_entries = cq_ptr + p.cq_off.ring_entries;
    cring->cqes = cq_ptr + p.cq_off.cqes;

    /* Map in the submission queue entries array */
    s->sqes = mmap(0, p.sq_entries * sizeof(struct io_uring_sqe),
            PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE,
            s->ring_fd, IORING_OFF_SQES);
    if (s->sqes == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    return 0;
}

void app_debug_print_uring(struct submitter *s)
{
#define PTR_AND_DEREF(ptr) ptr, *(ptr)
    printf("struct submitter s = {\n");
    printf("    .ring_fd = %d,\n", s->ring_fd);
    printf("    .sq_ring = {\n");
    printf("        .head =         %p -> 0x%x,\n", PTR_AND_DEREF(s->sq_ring.head));
    printf("        .tail =         %p -> 0x%x,\n", PTR_AND_DEREF(s->sq_ring.tail));
    printf("        .ring_mask =    %p -> 0x%x,\n", PTR_AND_DEREF(s->sq_ring.ring_mask));
    printf("        .ring_entries = %p -> 0x%x,\n", PTR_AND_DEREF(s->sq_ring.ring_entries));
    printf("        .flags =        %p -> 0x%x,\n", PTR_AND_DEREF(s->sq_ring.flags));
    printf("        .array =        %p -> 0x%x\n",  PTR_AND_DEREF(s->sq_ring.array));
    printf("    },\n");
    printf("    .cq_ring = {\n");
    printf("        .head =         %p -> 0x%x,\n", PTR_AND_DEREF(s->cq_ring.head));
    printf("        .tail =         %p -> 0x%x,\n", PTR_AND_DEREF(s->cq_ring.tail));
    printf("        .ring_mask =    %p -> 0x%x,\n", PTR_AND_DEREF(s->cq_ring.ring_mask));
    printf("        .ring_entries = %p -> 0x%x,\n", PTR_AND_DEREF(s->cq_ring.ring_entries));
    printf("        .cqes =         %p\n",  s->cq_ring.cqes);
    printf("    },\n");
    printf("    .sqes = %p\n", s->sqes);
    printf("}\n");
#undef PTR_AND_DEREF
}

/*
 * Read from completion queue.
 * In this function, we read completion events from the completion queue, get
 * the data buffer that will have the file data and print it to the console.
 *
 * */
int read_from_cq(struct submitter *s, bool print, int *reaped_success, int *results) {
    struct app_io_cq_ring *cring = &s->cq_ring;
    struct io_uring_cqe *cqe;
    unsigned head, reaped = 0, success = 0;

    head = *cring->head;

    do {
        read_barrier();
        /*
         * Remember, this is a ring buffer. If head == tail, it means that the
         * buffer is empty.
         * */
        if (head == *cring->tail)
            break;

        /* Get the entry */
        cqe = &cring->cqes[head & *s->cq_ring.ring_mask];
        if (print) {
            if (cqe->res < 0) {
                printf("cqe: res = %d (error: %s), user_data = 0x%llx\n", cqe->res, strerror(abs(cqe->res)), cqe->user_data);
            } else {
                printf("cqe: res = %d, user_data = 0x%llx\n", cqe->res, cqe->user_data);
            }
        }
        if (cqe->res >= 0) {
            success++;
            if (results) {
                *results++ = cqe->res;
            }
        }

        head++;
        reaped++;
    } while (1);

    *cring->head = head;
    write_barrier();

    if (reaped_success != NULL) {
        *reaped_success = success;
    }

    return reaped;
}

/*
 * Submit to submission queue.
 * In this function, we submit requests to the submission queue. You can submit
 * many types of requests. Ours is going to be the readv() request, which we
 * specify via IORING_OP_READV.
 *
 * */
int submit_to_sq(struct submitter *s, struct io_uring_sqe *sqes, unsigned int sqe_len, unsigned int min_complete) {
    struct app_io_sq_ring *sring = &s->sq_ring;
    unsigned index, head, tail, next_tail, mask, to_submit;

    /* assume unique submitter, i.e. tail does not change */
    next_tail = tail = *sring->tail;

    /* Add our submission queue entry to the tail of the SQE ring buffer */
    for (to_submit = 0; to_submit < sqe_len; to_submit++) {
        read_barrier();
        head = *sring->head;            // this may change as kernel processes sqe
        mask = *s->sq_ring.ring_mask;   // ...but does this ever change?

        // SQ full (tail wrapped back to head)
        if ((head & mask) == (tail & mask) && head != tail) {
            break;
        }

        next_tail++;
        index = tail & mask;
        struct io_uring_sqe *sqe = &s->sqes[index];
        memcpy(sqe, &sqes[to_submit], sizeof(*sqe));
        sring->array[index] = index;
        tail = next_tail;
    }

    /* Update the tail so the kernel can see it. */
    if(*sring->tail != tail) {
        *sring->tail = tail;
        write_barrier();
    }

    /*
     * Tell the kernel we have submitted events with the io_uring_enter() system
     * call. We also pass in the IOURING_ENTER_GETEVENTS flag which causes the
     * io_uring_enter() call to wait until min_complete events (the 3rd param)
     * complete.
     * */

    int ret = io_uring_enter(s->ring_fd, to_submit, min_complete, IORING_ENTER_GETEVENTS);
    if(ret < 0) {
        perror("io_uring_enter");
        return ret;
    }
    //io_uring_enter(s->ring_fd, 0, 0, IORING_ENTER_SQ_WAKEUP);

    return to_submit;
}
