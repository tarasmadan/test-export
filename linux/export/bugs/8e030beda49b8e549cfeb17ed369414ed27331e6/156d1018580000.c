// https://syzkaller.appspot.com/bug?id=8e030beda49b8e549cfeb17ed369414ed27331e6
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

uint64_t r[1] = {0xffffffffffffffff};

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
  memcpy((void*)0x20000040, "/proc/self/maps\000", 16);
  res = syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul, /*file=*/0x20000040ul,
                /*flags=O_TRUNC|O_NOCTTY|O_LARGEFILE*/ 0x8300, /*mode=*/0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_mmap, /*addr=*/0ul, /*len=*/0x2020089ul, /*prot=*/3ul,
          /*flags=*/0xeb1ul, /*fd=*/0xfffffffa, /*off=*/0x8000ul);
  *(uint64_t*)0x20001600 = 0x81;
  *(uint64_t*)0x20001608 = 0x3b;
  *(uint64_t*)0x20001610 = 0x24;
  *(uint64_t*)0x20001618 = 0;
  *(uint64_t*)0x20001620 = 0x715;
  *(uint64_t*)0x20001628 = 0x8001;
  *(uint64_t*)0x20001630 = 0x7d6;
  *(uint64_t*)0x20001638 = 0x9d;
  *(uint64_t*)0x20001640 = 5;
  *(uint32_t*)0x20001648 = 0xbfaf;
  *(uint32_t*)0x2000164c = 2;
  *(uint32_t*)0x20001650 = 8;
  *(uint32_t*)0x20001654 = 0xd97;
  *(uint64_t*)0x20001658 = 2;
  *(uint64_t*)0x20001660 = 5;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0xc0686611, /*arg=*/0x20001600ul);
  return 0;
}
