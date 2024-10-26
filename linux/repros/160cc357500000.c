// https://syzkaller.appspot.com/bug?id=e3fbba8e171fb9fa9f96ae167ba56e437bc2dfb6
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
  res = syscall(__NR_socket, 0x10ul, 0x803ul, 0);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x200001c0 = 0;
  *(uint32_t*)0x200001c8 = 0;
  *(uint64_t*)0x200001d0 = 0x20000180;
  *(uint64_t*)0x20000180 = 0;
  *(uint64_t*)0x20000188 = 0;
  *(uint64_t*)0x200001d8 = 1;
  *(uint64_t*)0x200001e0 = 0;
  *(uint64_t*)0x200001e8 = 0;
  *(uint32_t*)0x200001f0 = 0;
  syscall(__NR_sendmsg, r[1], 0x200001c0ul, 0ul);
  *(uint32_t*)0x20000200 = 0x2ba;
  res = syscall(__NR_getsockname, r[1], 0x20000100ul, 0x20000200ul);
  if (res != -1)
    r[2] = *(uint32_t*)0x20000104;
  *(uint64_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c8 = 0;
  *(uint64_t*)0x200000d0 = 0x20000200;
  *(uint64_t*)0x20000200 = 0x20000240;
  *(uint32_t*)0x20000240 = 0x40;
  *(uint16_t*)0x20000244 = 0x10;
  *(uint16_t*)0x20000246 = 0xff1f;
  *(uint32_t*)0x20000248 = 0;
  *(uint32_t*)0x2000024c = 0;
  *(uint8_t*)0x20000250 = 0;
  *(uint8_t*)0x20000251 = 0;
  *(uint16_t*)0x20000252 = 0;
  *(uint32_t*)0x20000254 = 0;
  *(uint32_t*)0x20000258 = 0;
  *(uint32_t*)0x2000025c = 0;
  *(uint16_t*)0x20000260 = 0x20;
  STORE_BY_BITMASK(uint16_t, , 0x20000262, 0x12, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000263, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000263, 1, 7, 1);
  *(uint16_t*)0x20000264 = 8;
  *(uint16_t*)0x20000266 = 1;
  memcpy((void*)0x20000268, "sit\000", 4);
  *(uint16_t*)0x2000026c = 0x14;
  STORE_BY_BITMASK(uint16_t, , 0x2000026e, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000026f, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000026f, 1, 7, 1);
  *(uint16_t*)0x20000270 = 6;
  *(uint16_t*)0x20000272 = 0xd;
  *(uint16_t*)0x20000274 = 0xb3;
  *(uint16_t*)0x20000278 = 8;
  *(uint16_t*)0x2000027a = 1;
  *(uint32_t*)0x2000027c = r[2];
  *(uint64_t*)0x20000208 = 0x40;
  *(uint64_t*)0x200000d8 = 1;
  *(uint64_t*)0x200000e0 = 0;
  *(uint64_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000f0 = 0;
  syscall(__NR_sendmsg, r[0], 0x200000c0ul, 0ul);
  return 0;
}
