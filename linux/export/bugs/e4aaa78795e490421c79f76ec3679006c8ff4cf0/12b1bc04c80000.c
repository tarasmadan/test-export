// https://syzkaller.appspot.com/bug?id=e4aaa78795e490421c79f76ec3679006c8ff4cf0
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  memcpy((void*)0x200000c0, "./bus\000", 6);
  res = syscall(__NR_open, 0x200000c0ul, 0x14da42ul, 0ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000040 = 0x4000;
  syscall(__NR_ioctl, r[0], 0x40086602, 0x20000040ul);
  memcpy((void*)0x200001c0, "cgroup.controllers\000", 19);
  res = syscall(__NR_openat, 0xffffff9c, 0x200001c0ul, 0x275aul, 0ul);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20000000 = 2;
  syscall(__NR_ioctl, r[1], 0x4004662b, 0x20000000ul);
  return 0;
}