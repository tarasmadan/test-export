// https://syzkaller.appspot.com/bug?id=0ae5678333f364bf73512b974e03cd0b6be6e497
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
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 6);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000740 = 0;
  *(uint32_t*)0x20000748 = 0;
  *(uint64_t*)0x20000750 = 0x20000380;
  *(uint64_t*)0x20000380 = 0x20000400;
  *(uint32_t*)0x20000400 = 0xf8;
  *(uint16_t*)0x20000404 = 0x26;
  *(uint16_t*)0x20000406 = 1;
  *(uint32_t*)0x20000408 = 0;
  *(uint32_t*)0x2000040c = 0;
  *(uint8_t*)0x20000410 = -1;
  *(uint8_t*)0x20000411 = 2;
  memset((void*)0x20000412, 0, 13);
  *(uint8_t*)0x2000041f = 1;
  *(uint8_t*)0x20000420 = 0xfe;
  *(uint8_t*)0x20000421 = 0x80;
  memset((void*)0x20000422, 0, 13);
  *(uint8_t*)0x2000042f = 0xbb;
  *(uint16_t*)0x20000430 = htobe16(0);
  *(uint16_t*)0x20000432 = htobe16(0);
  *(uint16_t*)0x20000434 = htobe16(0);
  *(uint16_t*)0x20000436 = htobe16(0);
  *(uint16_t*)0x20000438 = 0;
  *(uint8_t*)0x2000043a = 0;
  *(uint8_t*)0x2000043b = 0;
  *(uint8_t*)0x2000043c = 0;
  *(uint32_t*)0x20000440 = 0;
  *(uint32_t*)0x20000444 = -1;
  *(uint32_t*)0x20000448 = htobe32(0xe0000001);
  *(uint32_t*)0x20000458 = htobe32(0);
  *(uint8_t*)0x2000045c = 0;
  *(uint64_t*)0x20000460 = htobe64(0);
  *(uint64_t*)0x20000468 = htobe64(1);
  *(uint64_t*)0x20000470 = 0;
  *(uint64_t*)0x20000478 = 0;
  *(uint64_t*)0x20000480 = 0;
  *(uint64_t*)0x20000488 = 0;
  *(uint64_t*)0x20000490 = 0;
  *(uint64_t*)0x20000498 = 0;
  *(uint64_t*)0x200004a0 = 0;
  *(uint64_t*)0x200004a8 = 0;
  *(uint64_t*)0x200004b0 = 0;
  *(uint64_t*)0x200004b8 = 0;
  *(uint64_t*)0x200004c0 = 0;
  *(uint64_t*)0x200004c8 = 0;
  *(uint32_t*)0x200004d0 = 0;
  *(uint32_t*)0x200004d4 = 0;
  *(uint32_t*)0x200004d8 = 0;
  *(uint32_t*)0x200004dc = 0;
  *(uint32_t*)0x200004e0 = 0;
  *(uint16_t*)0x200004e4 = 0;
  *(uint8_t*)0x200004e6 = 0;
  *(uint8_t*)0x200004e7 = 0;
  *(uint8_t*)0x200004e8 = 0;
  *(uint32_t*)0x200004f0 = 0;
  *(uint32_t*)0x200004f4 = 0;
  *(uint64_t*)0x20000388 = 0xf8;
  *(uint64_t*)0x20000758 = 1;
  *(uint64_t*)0x20000760 = 0;
  *(uint64_t*)0x20000768 = 0;
  *(uint32_t*)0x20000770 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000740ul, 0ul);
  return 0;
}