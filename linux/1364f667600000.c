// https://syzkaller.appspot.com/bug?id=6c6ad1a51e95256ea0f9da3eaa795e0a36d31926
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
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10, 3, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0x11, 0x800000003, 0);
  if (res != -1)
    r[1] = res;
  *(uint16_t*)0x20000080 = 0x11;
  memcpy((void*)0x20000082,
         "\x00\x00\x01\x00\x00\x00\x00\x00\x08\x00\x44\x94\x4e\xeb\xa7\x1a\x49"
         "\x76\xe2\x52\x92\x2c\xb1\x8f\x6e\x2e\x2a\xba\x00\x00\x00\x01\x2e\x0b"
         "\x38\x36\x00\x54\x04\xb0\xe0\x30\x1a\x4c\xe8\x75\xf2\xe3\xff\x5f\x16"
         "\x3e\xe3\x40\xb7\x67\x95\x00\x80\x00\x00\x00\x00\x00\x00\x01\x01\x01"
         "\x3c\x58\x11\x03\x9e\x15\x77\x50\x27\xec\xce\x66\xfd\x79\x2b\xbf\x0e"
         "\x5b\xf5\xff\x1b\x08\x16\xf3\xf6\xdb\x1c\x00\x01\x00\x00\x00\x00\x00"
         "\x00\x00\x49\x74\x00\x00\x00\x00\x00\x00\x00\x06\xad\x8e\x5e\xcc\x32"
         "\x6d\x3a\x09\xff\xc2\xc6\x54",
         126);
  syscall(__NR_bind, r[1], 0x20000080, 0x80);
  *(uint32_t*)0x20000140 = 0x14;
  res = syscall(__NR_getsockname, r[1], 0x20000040, 0x20000140);
  if (res != -1)
    r[2] = *(uint32_t*)0x20000044;
  *(uint64_t*)0x20000280 = 0;
  *(uint32_t*)0x20000288 = 0;
  *(uint64_t*)0x20000290 = 0x20000080;
  *(uint64_t*)0x20000080 = 0x200000c0;
  *(uint32_t*)0x200000c0 = 0x3c;
  *(uint16_t*)0x200000c4 = 0x24;
  *(uint16_t*)0x200000c6 = 0xf01;
  *(uint32_t*)0x200000c8 = 0xfffffffe;
  *(uint32_t*)0x200000cc = 0;
  *(uint8_t*)0x200000d0 = 0;
  *(uint32_t*)0x200000d4 = r[2];
  *(uint16_t*)0x200000d8 = 0;
  *(uint16_t*)0x200000da = 0;
  *(uint16_t*)0x200000dc = -1;
  *(uint16_t*)0x200000de = -1;
  *(uint16_t*)0x200000e0 = 0;
  *(uint16_t*)0x200000e2 = 0;
  *(uint16_t*)0x200000e4 = 0xb;
  *(uint16_t*)0x200000e6 = 1;
  memcpy((void*)0x200000e8, "dsmark\000", 7);
  *(uint16_t*)0x200000f0 = 0xc;
  *(uint16_t*)0x200000f2 = 2;
  *(uint16_t*)0x200000f4 = 8;
  *(uint16_t*)0x200000f6 = 2;
  *(uint16_t*)0x200000f8 = 2;
  *(uint64_t*)0x20000088 = 0x3c;
  *(uint64_t*)0x20000298 = 1;
  *(uint64_t*)0x200002a0 = 0;
  *(uint64_t*)0x200002a8 = 0;
  *(uint32_t*)0x200002b0 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000280, 0);
  return 0;
}
