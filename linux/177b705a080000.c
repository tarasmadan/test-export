// https://syzkaller.appspot.com/bug?id=67c06f27e8efe15830eb8ff6ee742380e18c3d9b
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

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  memcpy((void*)0x20000200, "/dev/udmabuf\000", 13);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000200ul, 2ul, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000000,
         "y\0205%\243\325\372\327\372\027\351\231\242\211\216\315\375", 17);
  res = syscall(__NR_memfd_create, 0x20000000ul, 2ul);
  if (res != -1)
    r[1] = res;
  memset((void*)0x200000c0, 160, 1);
  syscall(__NR_pwrite64, r[1], 0x200000c0ul, 1ul, 0x5b63ul);
  syscall(__NR_fcntl, r[1], 0x409ul, 7ul);
  res = syscall(__NR_dup, r[0]);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x20000080 = r[1];
  *(uint32_t*)0x20000084 = 0;
  *(uint64_t*)0x20000088 = 0;
  *(uint64_t*)0x20000090 = 0x2000;
  res = syscall(__NR_ioctl, r[2], 0x40187542, 0x20000080ul);
  if (res != -1)
    r[3] = res;
  *(uint64_t*)0x20000100 = 2;
  syscall(__NR_ioctl, r[3], 0x40086200, 0x20000100ul);
  return 0;
}