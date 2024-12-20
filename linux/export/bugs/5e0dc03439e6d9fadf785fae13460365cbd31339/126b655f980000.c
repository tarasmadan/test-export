// https://syzkaller.appspot.com/bug?id=5e0dc03439e6d9fadf785fae13460365cbd31339
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

#ifndef __NR_io_uring_register
#define __NR_io_uring_register 427
#endif
#ifndef __NR_io_uring_setup
#define __NR_io_uring_setup 425
#endif

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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  const char* reason;
  (void)reason;
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  *(uint32_t*)0x20005104 = 0;
  *(uint32_t*)0x20005108 = 0x82;
  *(uint32_t*)0x2000510c = 0;
  *(uint32_t*)0x20005110 = 0;
  *(uint32_t*)0x20005118 = -1;
  memset((void*)0x2000511c, 0, 12);
  res =
      syscall(__NR_io_uring_setup, /*entries=*/0x29e6, /*params=*/0x20005100ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20005500 = 3;
  *(uint32_t*)0x20005504 = 0;
  *(uint64_t*)0x20005508 = 0;
  *(uint64_t*)0x20005510 = 0x20005480;
  *(uint64_t*)0x20005480 = 0;
  *(uint64_t*)0x20005488 = 0;
  *(uint64_t*)0x20005490 = 0;
  *(uint64_t*)0x20005498 = 0;
  *(uint64_t*)0x200054a0 = 0x20005380;
  *(uint64_t*)0x200054a8 = 0xff;
  *(uint64_t*)0x20005518 = 0;
  syscall(__NR_io_uring_register, /*fd=*/r[0], /*opcode=*/0xful,
          /*arg=*/0x20005500ul, /*size=*/0x20ul);
  memcpy((void*)0x20000000, "fdinfo/3\000", 9);
  res = -1;
  res = syz_open_procfs(/*pid=*/-1, /*file=*/0x20000000);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x20000640 = 0x20000140;
  *(uint64_t*)0x20000648 = 0x86;
  syscall(__NR_preadv, /*fd=*/r[1], /*vec=*/0x20000640ul, /*vlen=*/1ul,
          /*off_low=*/0, /*off_high=*/0);
  return 0;
}
