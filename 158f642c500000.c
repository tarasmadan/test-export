// https://syzkaller.appspot.com/bug?id=aaa35b314220404bbc2b1c66067b0f9a623baa89
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

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 370
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000, 0x1000, 0, 0x32, -1, 0);
  syscall(__NR_mmap, 0x20000000, 0x1000000, 7, 0x32, -1, 0);
  syscall(__NR_mmap, 0x21000000, 0x1000, 0, 0x32, -1, 0);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10, 3, 6);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000080 = 0;
  *(uint32_t*)0x20000084 = 0;
  *(uint32_t*)0x20000088 = 0x20000040;
  *(uint32_t*)0x20000040 = 0x20000280;
  *(uint32_t*)0x20000280 = 0xf8;
  *(uint16_t*)0x20000284 = 0x21;
  *(uint16_t*)0x20000286 = 1;
  *(uint32_t*)0x20000288 = 0;
  *(uint32_t*)0x2000028c = 0;
  *(uint8_t*)0x20000290 = 0xac;
  *(uint8_t*)0x20000291 = 0x14;
  *(uint8_t*)0x20000292 = 0x14;
  *(uint8_t*)0x20000293 = 0xbb;
  *(uint8_t*)0x200002a0 = 0xac;
  *(uint8_t*)0x200002a1 = 0x14;
  *(uint8_t*)0x200002a2 = 0x14;
  *(uint8_t*)0x200002a3 = 0xbb;
  *(uint16_t*)0x200002b0 = htobe16(0);
  *(uint16_t*)0x200002b2 = htobe16(0);
  *(uint16_t*)0x200002b4 = htobe16(0);
  *(uint16_t*)0x200002b6 = htobe16(0);
  *(uint16_t*)0x200002b8 = 0;
  *(uint8_t*)0x200002ba = 0;
  *(uint8_t*)0x200002bb = 0;
  *(uint8_t*)0x200002bc = 0;
  *(uint32_t*)0x200002c0 = 0;
  *(uint32_t*)0x200002c4 = 0xee01;
  *(uint32_t*)0x200002c8 = 0;
  *(uint8_t*)0x200002cc = 0;
  *(uint16_t*)0x200002d0 = 0xa8;
  *(uint16_t*)0x200002d2 = 7;
  *(uint32_t*)0x200002d4 = htobe32(0);
  *(uint8_t*)0x200002e4 = 0xfc;
  *(uint8_t*)0x200002e5 = 0;
  *(uint8_t*)0x200002e6 = 0;
  *(uint8_t*)0x200002e7 = 0;
  *(uint8_t*)0x200002e8 = 0;
  *(uint8_t*)0x200002e9 = 0;
  *(uint8_t*)0x200002ea = 0;
  *(uint8_t*)0x200002eb = 0;
  *(uint8_t*)0x200002ec = 0;
  *(uint8_t*)0x200002ed = 0;
  *(uint8_t*)0x200002ee = 0;
  *(uint8_t*)0x200002ef = 0;
  *(uint8_t*)0x200002f0 = 0;
  *(uint8_t*)0x200002f1 = 0;
  *(uint8_t*)0x200002f2 = 0;
  *(uint8_t*)0x200002f3 = 0;
  *(uint16_t*)0x200002f4 = htobe16(0);
  *(uint16_t*)0x200002f6 = htobe16(0);
  *(uint16_t*)0x200002f8 = htobe16(0);
  *(uint16_t*)0x200002fa = htobe16(0);
  *(uint16_t*)0x200002fc = 0;
  *(uint8_t*)0x200002fe = 0;
  *(uint8_t*)0x200002ff = 0;
  *(uint8_t*)0x20000300 = 0;
  *(uint32_t*)0x20000304 = 0;
  *(uint32_t*)0x20000308 = 0;
  *(uint64_t*)0x2000030c = 0;
  *(uint64_t*)0x20000314 = 0;
  *(uint64_t*)0x2000031c = 0;
  *(uint64_t*)0x20000324 = 0;
  *(uint64_t*)0x2000032c = 0;
  *(uint64_t*)0x20000334 = 0;
  *(uint64_t*)0x2000033c = 0;
  *(uint64_t*)0x20000344 = 0;
  *(uint64_t*)0x2000034c = 0;
  *(uint64_t*)0x20000354 = 0;
  *(uint64_t*)0x2000035c = 0;
  *(uint64_t*)0x20000364 = 0;
  *(uint32_t*)0x2000036c = 0;
  *(uint32_t*)0x20000370 = 0;
  *(uint8_t*)0x20000374 = 0;
  *(uint8_t*)0x20000375 = 0;
  *(uint8_t*)0x20000376 = 0;
  *(uint8_t*)0x20000377 = 0;
  *(uint32_t*)0x20000044 = 0xf8;
  *(uint32_t*)0x2000008c = 1;
  *(uint32_t*)0x20000090 = 0;
  *(uint32_t*)0x20000094 = 0;
  *(uint32_t*)0x20000098 = 0;
  syscall(__NR_sendmsg, (intptr_t)r[0], 0x20000080, 0);
  return 0;
}
