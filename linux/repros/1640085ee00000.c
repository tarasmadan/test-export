// https://syzkaller.appspot.com/bug?id=6fbb32225787f789f5ce49000ac86713a6c24588
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
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  intptr_t res = 0;
  memcpy((void*)0x200017c0, "/selinux/enforce\000", 17);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x200017c0ul, 2ul, 0ul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000000 = 0x20000100;
  memcpy((void*)0x20000100, " 8", 2);
  *(uint64_t*)0x20000008 = 2;
  *(uint64_t*)0x20000010 = 0;
  *(uint64_t*)0x20000018 = 0;
  syscall(__NR_writev, r[0], 0x20000000ul, 2ul);
  memcpy((void*)0x20000000, "/sys/kernel/debug/bluetooth/6lowpan_enable\000",
         43);
  syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000000ul, 2ul, 0ul);
  return 0;
}