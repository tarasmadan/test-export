// https://syzkaller.appspot.com/bug?id=2291081711acc3b1b8b84eee6ba648aacf99addf
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socketpair, 1ul, 3ul, 0, 0x20000080ul);
  if (res != -1)
    r[1] = *(uint32_t*)0x20000084;
  memcpy((void*)0x200003c0,
         "lo\000\000\000\000\000\000\000\000\000\000\000\000\000\000", 16);
  res = syscall(__NR_ioctl, r[1], 0x8933, 0x200003c0ul);
  if (res != -1)
    r[2] = *(uint32_t*)0x200003d0;
  *(uint64_t*)0x20000040 = 0;
  *(uint32_t*)0x20000048 = 0;
  *(uint64_t*)0x20000050 = 0x20000780;
  *(uint64_t*)0x20000780 = 0x200000c0;
  *(uint32_t*)0x200000c0 = 0x34;
  *(uint16_t*)0x200000c4 = 0x24;
  *(uint16_t*)0x200000c6 = 0xf0b;
  *(uint32_t*)0x200000c8 = 0;
  *(uint32_t*)0x200000cc = 0;
  *(uint8_t*)0x200000d0 = 0x60;
  *(uint8_t*)0x200000d1 = 0;
  *(uint16_t*)0x200000d2 = 0;
  *(uint32_t*)0x200000d4 = r[2];
  *(uint16_t*)0x200000d8 = 0;
  *(uint16_t*)0x200000da = 0;
  *(uint16_t*)0x200000dc = -1;
  *(uint16_t*)0x200000de = -1;
  *(uint16_t*)0x200000e0 = 0;
  *(uint16_t*)0x200000e2 = 0;
  *(uint16_t*)0x200000e4 = 9;
  *(uint16_t*)0x200000e6 = 1;
  memcpy((void*)0x200000e8, "cake\000", 5);
  *(uint16_t*)0x200000f0 = 4;
  *(uint16_t*)0x200000f2 = 8;
  *(uint64_t*)0x20000788 = 0x34;
  *(uint64_t*)0x20000058 = 1;
  *(uint64_t*)0x20000060 = 0;
  *(uint64_t*)0x20000068 = 0;
  *(uint32_t*)0x20000070 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000040ul, 0ul);
  return 0;
}
