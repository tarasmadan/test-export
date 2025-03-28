// https://syzkaller.appspot.com/bug?id=59997d828822e7dc674e4715a5bd454d9dc1742f
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

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);

  *(uint64_t*)0x20000100 = 0x20000040;
  *(uint16_t*)0x20000040 = 0xeb9f;
  *(uint8_t*)0x20000042 = 1;
  *(uint8_t*)0x20000043 = 0;
  *(uint32_t*)0x20000044 = 0x18;
  *(uint32_t*)0x20000048 = 0;
  *(uint32_t*)0x2000004c = 0x18;
  *(uint32_t*)0x20000050 = 0x18;
  *(uint32_t*)0x20000054 = 2;
  *(uint32_t*)0x20000058 = 0;
  *(uint16_t*)0x2000005c = 1;
  *(uint8_t*)0x2000005e = 0;
  STORE_BY_BITMASK(uint8_t, , 0x2000005f, 5, 0, 7);
  STORE_BY_BITMASK(uint8_t, , 0x2000005f, 0, 7, 1);
  *(uint32_t*)0x20000060 = 0;
  *(uint32_t*)0x20000064 = 0;
  *(uint32_t*)0x20000068 = 5;
  *(uint32_t*)0x2000006c = 0;
  *(uint8_t*)0x20000070 = 0;
  *(uint8_t*)0x20000071 = 0;
  *(uint64_t*)0x20000108 = 0x20000200;
  *(uint32_t*)0x20000110 = 0x32;
  *(uint32_t*)0x20000114 = 0x9e;
  *(uint32_t*)0x20000118 = 1;
  syscall(__NR_bpf, 0x12, 0x20000100, 0x20);
  return 0;
}
