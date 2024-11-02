// https://syzkaller.appspot.com/bug?id=1568a614680eeefbb3dfa3030b49c17b57756569
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

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0ul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000300 = 0;
  *(uint32_t*)0x20000308 = 0;
  *(uint64_t*)0x20000310 = 0x200002c0;
  *(uint64_t*)0x200002c0 = 0x20000080;
  *(uint32_t*)0x20000080 = 0x7c;
  *(uint16_t*)0x20000084 = 0x30;
  *(uint16_t*)0x20000086 = 0x727;
  *(uint32_t*)0x20000088 = 0;
  *(uint32_t*)0x2000008c = 0;
  *(uint8_t*)0x20000090 = 0;
  *(uint8_t*)0x20000091 = 0;
  *(uint16_t*)0x20000092 = 0;
  *(uint16_t*)0x20000094 = 0x68;
  *(uint16_t*)0x20000096 = 1;
  *(uint16_t*)0x20000098 = 0x64;
  STORE_BY_BITMASK(uint16_t, , 0x2000009a, 1, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000009b, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000009b, 0, 7, 1);
  *(uint16_t*)0x2000009c = 7;
  *(uint16_t*)0x2000009e = 1;
  memcpy((void*)0x200000a0, "ct\000", 3);
  *(uint16_t*)0x200000a4 = 0x54;
  STORE_BY_BITMASK(uint16_t, , 0x200000a6, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200000a7, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200000a7, 1, 7, 1);
  *(uint16_t*)0x200000a8 = 0x18;
  *(uint16_t*)0x200000aa = 1;
  *(uint32_t*)0x200000ac = 0;
  *(uint32_t*)0x200000b0 = 0;
  *(uint32_t*)0x200000b4 = 0;
  *(uint32_t*)0x200000b8 = 0;
  *(uint32_t*)0x200000bc = 0;
  *(uint16_t*)0x200000c0 = 0x14;
  *(uint16_t*)0x200000c2 = 7;
  memcpy((void*)0x200000c4,
         "\xc9\xb2\x48\x3d\xb8\x58\xef\x59\xe6\xce\x30\xa6\x82\x90\x8e\xa3",
         16);
  *(uint16_t*)0x200000d4 = 6;
  *(uint16_t*)0x200000d6 = 4;
  *(uint16_t*)0x200000d8 = 0x797;
  *(uint16_t*)0x200000dc = 0x14;
  *(uint16_t*)0x200000de = 8;
  memcpy((void*)0x200000e0,
         "\xbf\x66\x14\xdd\x68\x62\x2c\xce\x6d\x98\xfa\xed\x56\x52\x5a\x4b",
         16);
  *(uint16_t*)0x200000f0 = 8;
  *(uint16_t*)0x200000f2 = 5;
  *(uint32_t*)0x200000f4 = 0;
  *(uint16_t*)0x200000f8 = 4;
  *(uint16_t*)0x200000fa = 6;
  *(uint64_t*)0x200002c8 = 0x7c;
  *(uint64_t*)0x20000318 = 1;
  *(uint64_t*)0x20000320 = 0;
  *(uint64_t*)0x20000328 = 0;
  *(uint32_t*)0x20000330 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000300ul, 0ul);
  return 0;
}