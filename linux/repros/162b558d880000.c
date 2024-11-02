// https://syzkaller.appspot.com/bug?id=3897413eb15aa1bd9cd201cea5e58c4326448b51
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

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);

  *(uint64_t*)0x20000000 = 0x20000040;
  *(uint16_t*)0x20000040 = 0xeb9f;
  *(uint8_t*)0x20000042 = 1;
  *(uint8_t*)0x20000043 = 0;
  *(uint32_t*)0x20000044 = 0x18;
  *(uint32_t*)0x20000048 = 0;
  *(uint32_t*)0x2000004c = 0x40;
  *(uint32_t*)0x20000050 = 0x40;
  *(uint32_t*)0x20000054 = 4;
  *(uint32_t*)0x20000058 = 2;
  *(uint16_t*)0x2000005c = 0;
  *(uint8_t*)0x2000005e = 0;
  *(uint8_t*)0x2000005f = 0x11;
  *(uint32_t*)0x20000060 = 4;
  *(uint32_t*)0x20000064 = -1;
  *(uint32_t*)0x20000068 = 0;
  *(uint16_t*)0x2000006c = 0;
  *(uint8_t*)0x2000006e = 0;
  *(uint8_t*)0x2000006f = 2;
  *(uint32_t*)0x20000070 = 0;
  *(uint32_t*)0x20000074 = 0;
  *(uint16_t*)0x20000078 = 1;
  *(uint8_t*)0x2000007a = 0;
  *(uint8_t*)0x2000007b = 0xd;
  *(uint32_t*)0x2000007c = 0;
  *(uint32_t*)0x20000080 = 2;
  *(uint32_t*)0x20000084 = 1;
  *(uint32_t*)0x20000088 = 2;
  *(uint16_t*)0x2000008c = 0;
  *(uint8_t*)0x2000008e = 0;
  *(uint8_t*)0x2000008f = 0xe;
  *(uint32_t*)0x20000090 = 2;
  *(uint32_t*)0x20000094 = 0;
  *(uint8_t*)0x20000098 = 0;
  *(uint8_t*)0x20000099 = 0;
  *(uint8_t*)0x2000009a = 0x61;
  *(uint8_t*)0x2000009b = 0;
  *(uint64_t*)0x20000008 = 0x200001c0;
  *(uint32_t*)0x20000010 = 0x5c;
  *(uint32_t*)0x20000014 = 0x109;
  *(uint32_t*)0x20000018 = 7;
  syscall(__NR_bpf, 0x12ul, 0x20000000ul, 0x20ul);
  return 0;
}