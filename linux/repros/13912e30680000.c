// https://syzkaller.appspot.com/bug?id=b34adda9bfcc56c7de4107cba2b92ae57417c6ae
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef __NR_io_uring_enter
#define __NR_io_uring_enter 426
#endif
#ifndef __NR_io_uring_setup
#define __NR_io_uring_setup 425
#endif

#define SIZEOF_IO_URING_SQE 64
#define SIZEOF_IO_URING_CQE 16
#define SQ_HEAD_OFFSET 0
#define SQ_TAIL_OFFSET 64
#define SQ_RING_MASK_OFFSET 256
#define SQ_RING_ENTRIES_OFFSET 264
#define SQ_FLAGS_OFFSET 276
#define SQ_DROPPED_OFFSET 272
#define CQ_HEAD_OFFSET 128
#define CQ_TAIL_OFFSET 192
#define CQ_RING_MASK_OFFSET 260
#define CQ_RING_ENTRIES_OFFSET 268
#define CQ_RING_OVERFLOW_OFFSET 284
#define CQ_FLAGS_OFFSET 280
#define CQ_CQES_OFFSET 320

struct io_sqring_offsets {
  uint32_t head;
  uint32_t tail;
  uint32_t ring_mask;
  uint32_t ring_entries;
  uint32_t flags;
  uint32_t dropped;
  uint32_t array;
  uint32_t resv1;
  uint64_t resv2;
};

struct io_cqring_offsets {
  uint32_t head;
  uint32_t tail;
  uint32_t ring_mask;
  uint32_t ring_entries;
  uint32_t overflow;
  uint32_t cqes;
  uint64_t resv[2];
};

struct io_uring_params {
  uint32_t sq_entries;
  uint32_t cq_entries;
  uint32_t flags;
  uint32_t sq_thread_cpu;
  uint32_t sq_thread_idle;
  uint32_t features;
  uint32_t resv[4];
  struct io_sqring_offsets sq_off;
  struct io_cqring_offsets cq_off;
};

#define IORING_OFF_SQ_RING 0
#define IORING_OFF_SQES 0x10000000ULL

static long syz_io_uring_setup(volatile long a0, volatile long a1,
                               volatile long a2, volatile long a3)
{
  uint32_t entries = (uint32_t)a0;
  struct io_uring_params* setup_params = (struct io_uring_params*)a1;
  void** ring_ptr_out = (void**)a2;
  void** sqes_ptr_out = (void**)a3;
  uint32_t fd_io_uring = syscall(__NR_io_uring_setup, entries, setup_params);
  uint32_t sq_ring_sz =
      setup_params->sq_off.array + setup_params->sq_entries * sizeof(uint32_t);
  uint32_t cq_ring_sz = setup_params->cq_off.cqes +
                        setup_params->cq_entries * SIZEOF_IO_URING_CQE;
  uint32_t ring_sz = sq_ring_sz > cq_ring_sz ? sq_ring_sz : cq_ring_sz;
  *ring_ptr_out =
      mmap(0, ring_sz, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE,
           fd_io_uring, IORING_OFF_SQ_RING);
  uint32_t sqes_sz = setup_params->sq_entries * SIZEOF_IO_URING_SQE;
  *sqes_ptr_out = mmap(0, sqes_sz, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_POPULATE, fd_io_uring, IORING_OFF_SQES);
  uint32_t* array =
      (uint32_t*)((uintptr_t)*ring_ptr_out + setup_params->sq_off.array);
  for (uint32_t index = 0; index < entries; index++)
    array[index] = index;
  return fd_io_uring;
}

static long syz_io_uring_submit(volatile long a0, volatile long a1,
                                volatile long a2)
{
  char* ring_ptr = (char*)a0;
  char* sqes_ptr = (char*)a1;
  char* sqe = (char*)a2;
  uint32_t sq_ring_mask = *(uint32_t*)(ring_ptr + SQ_RING_MASK_OFFSET);
  uint32_t* sq_tail_ptr = (uint32_t*)(ring_ptr + SQ_TAIL_OFFSET);
  uint32_t sq_tail = *sq_tail_ptr & sq_ring_mask;
  char* sqe_dest = sqes_ptr + sq_tail * SIZEOF_IO_URING_SQE;
  memcpy(sqe_dest, sqe, SIZEOF_IO_URING_SQE);
  uint32_t sq_tail_next = *sq_tail_ptr + 1;
  __atomic_store_n(sq_tail_ptr, sq_tail_next, __ATOMIC_RELEASE);
  return 0;
}

uint64_t r[4] = {0xffffffffffffffff, 0x0, 0x0, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul, /*prot=*/7ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  intptr_t res = 0;
  *(uint32_t*)0x20000084 = 0xd14e;
  *(uint32_t*)0x20000088 = 0x10100;
  *(uint32_t*)0x2000008c = 0;
  *(uint32_t*)0x20000090 = 0x31b;
  *(uint32_t*)0x20000098 = -1;
  memset((void*)0x2000009c, 0, 12);
  res = -1;
  res = syz_io_uring_setup(/*entries=*/0x1592, /*params=*/0x20000080,
                           /*ring_ptr=*/0x20000540, /*sqes_ptr=*/0x20000100);
  if (res != -1) {
    r[0] = res;
    r[1] = *(uint64_t*)0x20000540;
    r[2] = *(uint64_t*)0x20000100;
  }
  res = syscall(__NR_socket, /*domain=*/0x26ul, /*type=*/5ul, /*proto=*/0);
  if (res != -1)
    r[3] = res;
  *(uint8_t*)0x20000140 = 0xa;
  *(uint8_t*)0x20000141 = 0x20;
  *(uint16_t*)0x20000142 = 1;
  *(uint32_t*)0x20000144 = r[3];
  *(uint64_t*)0x20000148 = 0;
  *(uint64_t*)0x20000150 = 0x20000040;
  *(uint64_t*)0x20000040 = 0;
  *(uint32_t*)0x20000048 = 0;
  *(uint64_t*)0x20000050 = 0;
  *(uint64_t*)0x20000058 = 0;
  *(uint64_t*)0x20000060 = 0;
  *(uint64_t*)0x20000068 = 0;
  *(uint32_t*)0x20000070 = 0;
  *(uint32_t*)0x20000158 = 0;
  *(uint32_t*)0x2000015c = 0;
  *(uint64_t*)0x20000160 = 1;
  *(uint16_t*)0x20000168 = 0;
  *(uint16_t*)0x2000016a = 0;
  memset((void*)0x2000016c, 0, 20);
  syz_io_uring_submit(/*ring_ptr=*/r[1], /*sqes_ptr=*/r[2], /*sqe=*/0x20000140);
  syscall(__NR_io_uring_enter, /*fd=*/r[0], /*to_submit=*/0x7689,
          /*min_complete=*/0, /*flags=*/0ul, /*sigmask=*/0ul, /*size=*/0ul);
  return 0;
}
