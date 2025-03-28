// https://syzkaller.appspot.com/bug?id=5ba5b3fcbff850f6d8b8d95a3cab3a33d84450bc
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

static long syz_open_procfs(volatile long a0, volatile long a1)
{
  char buf[128];
  memset(buf, 0, sizeof(buf));
  if (a0 == 0) {
    snprintf(buf, sizeof(buf), "/proc/self/%s", (char*)a1);
  } else if (a0 == -1) {
    snprintf(buf, sizeof(buf), "/proc/thread-self/%s", (char*)a1);
  } else {
    snprintf(buf, sizeof(buf), "/proc/self/task/%d/%s", (int)a0, (char*)a1);
  }
  int fd = open(buf, O_RDWR);
  if (fd == -1)
    fd = open(buf, O_RDONLY);
  return fd;
}

#ifndef __NR_io_uring_register
#define __NR_io_uring_register 427
#endif
#ifndef __NR_io_uring_setup
#define __NR_io_uring_setup 425
#endif

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  *(uint32_t*)0x200000c4 = 0;
  *(uint32_t*)0x200000c8 = 0;
  *(uint32_t*)0x200000cc = 0;
  *(uint32_t*)0x200000d0 = 0;
  *(uint32_t*)0x200000d8 = -1;
  *(uint32_t*)0x200000dc = 0;
  *(uint32_t*)0x200000e0 = 0;
  *(uint32_t*)0x200000e4 = 0;
  res = syscall(__NR_io_uring_setup, 0xa30, 0x200000c0ul);
  if (res != -1)
    r[0] = res;
  syscall(__NR_io_uring_register, r[0], 9ul, 0ul, 0ul);
  *(uint64_t*)0x20003140 = 0x20001400;
  *(uint32_t*)0x20003148 = 0x80;
  *(uint64_t*)0x20003150 = 0;
  *(uint64_t*)0x20003158 = 0;
  *(uint64_t*)0x20003160 = 0;
  *(uint64_t*)0x20003168 = 0;
  *(uint32_t*)0x20003170 = 0;
  *(uint32_t*)0x20003178 = 0;
  syscall(__NR_recvmmsg, -1, 0x20003140ul, 1ul, 0ul, 0ul);
  memcpy((void*)0x20000000, "fdinfo/3\000", 9);
  res = -1;
  res = syz_open_procfs(0, 0x20000000);
  if (res != -1)
    r[1] = res;
  syscall(__NR_preadv, r[1], 0x200017c0ul, 0x333ul, 0, 0);
  return 0;
}
