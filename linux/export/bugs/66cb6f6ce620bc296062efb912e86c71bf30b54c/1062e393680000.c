// https://syzkaller.appspot.com/bug?id=66cb6f6ce620bc296062efb912e86c71bf30b54c
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
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul, /*prot=*/7ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  intptr_t res = 0;
  memcpy((void*)0x20000000, "/dev/iommu\000", 11);
  res = syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul, /*file=*/0x20000000ul,
                /*flags=*/0ul, /*mode=*/0ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000140 = 0x6a60;
  *(uint32_t*)0x20000144 = 0xd;
  *(uint32_t*)0x20000148 = 0;
  *(uint32_t*)0x2000014c = 0;
  *(uint32_t*)0x20000154 = 0;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x3ba0, /*arg=*/0x20000140ul);
  return 0;
}
