// https://syzkaller.appspot.com/bug?id=9f4513b41da4f8a2961128dacb661a43f4acc21f
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
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0x14ul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x200031c0 = 0;
  *(uint32_t*)0x200031c8 = 0;
  *(uint64_t*)0x200031d0 = 0x20003180;
  *(uint64_t*)0x20003180 = 0x20003000;
  *(uint32_t*)0x20003000 = 0x38;
  *(uint16_t*)0x20003004 = 0x1403;
  *(uint16_t*)0x20003006 = 1;
  *(uint32_t*)0x20003008 = 0;
  *(uint32_t*)0x2000300c = 0;
  *(uint16_t*)0x20003010 = 9;
  *(uint16_t*)0x20003012 = 2;
  memcpy((void*)0x20003014, "syz1\000", 5);
  *(uint16_t*)0x2000301c = 8;
  *(uint16_t*)0x2000301e = 0x41;
  memcpy((void*)0x20003020, "siw\000", 4);
  *(uint16_t*)0x20003024 = 0x14;
  *(uint16_t*)0x20003026 = 0x33;
  memcpy((void*)0x20003028,
         "lo\000\000\000\000\000\000\000\000\000\000\000\000\000\000", 16);
  *(uint64_t*)0x20003188 = 0x38;
  *(uint64_t*)0x200031d8 = 1;
  *(uint64_t*)0x200031e0 = 0;
  *(uint64_t*)0x200031e8 = 0;
  *(uint32_t*)0x200031f0 = 0;
  syscall(__NR_sendmsg, r[0], 0x200031c0ul, 0ul);
  *(uint64_t*)0x20000500 = 0;
  *(uint32_t*)0x20000508 = 0;
  *(uint64_t*)0x20000510 = 0x200004c0;
  *(uint64_t*)0x200004c0 = 0x20000000;
  memcpy((void*)0x20000000, "\x30\x00\x00\x00\x02\x07\x02\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00"
                            "\x18\x00\x01",
         27);
  *(uint64_t*)0x200004c8 = 1;
  *(uint64_t*)0x20000518 = 1;
  *(uint64_t*)0x20000520 = 0;
  *(uint64_t*)0x20000528 = 0;
  *(uint32_t*)0x20000530 = 0;
  syscall(__NR_sendmsg, -1, 0x20000500ul, 0ul);
  *(uint64_t*)0x20000200 = 0;
  *(uint32_t*)0x20000208 = 0;
  *(uint64_t*)0x20000210 = 0x200001c0;
  *(uint64_t*)0x200001c0 = 0x20000000;
  memcpy((void*)0x20000000, "\x28\x00\x00\x00\x4a\x00\x01", 7);
  *(uint64_t*)0x200001c8 = 1;
  *(uint64_t*)0x20000218 = 1;
  *(uint64_t*)0x20000220 = 0;
  *(uint64_t*)0x20000228 = 0;
  *(uint32_t*)0x20000230 = 0;
  syscall(__NR_sendmsg, -1, 0x20000200ul, 0ul);
  *(uint64_t*)0x20001f80 = 0;
  *(uint32_t*)0x20001f88 = 0;
  *(uint64_t*)0x20001f90 = 0x20001f40;
  *(uint64_t*)0x20001f40 = 0x20000000;
  memcpy((void*)0x20000000, "\x14\x00\x00\x00\x04\x14", 6);
  *(uint64_t*)0x20001f48 = 1;
  *(uint64_t*)0x20001f98 = 1;
  *(uint64_t*)0x20001fa0 = 0;
  *(uint64_t*)0x20001fa8 = 0;
  *(uint32_t*)0x20001fb0 = 0;
  syscall(__NR_sendmsg, -1, 0x20001f80ul, 0ul);
  res = syscall(__NR_socket, 0x10ul, 2ul, 0x14);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000000, "E", 1);
  syscall(__NR_sendto, r[1], 0x20000000ul, 0x10a73ul, 0x8c0ul, 0ul,
          0x4b6ae4f95a5de35bul);
  return 0;
}