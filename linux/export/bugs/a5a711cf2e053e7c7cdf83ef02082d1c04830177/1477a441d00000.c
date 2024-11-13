// https://syzkaller.appspot.com/bug?id=a5a711cf2e053e7c7cdf83ef02082d1c04830177
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
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0xc);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c8 = 0;
  *(uint64_t*)0x200000d0 = 0x20000000;
  *(uint64_t*)0x20000000 = 0x200001c0;
  *(uint32_t*)0x200001c0 = 0x14;
  *(uint16_t*)0x200001c4 = 0x10;
  *(uint16_t*)0x200001c6 = 1;
  *(uint32_t*)0x200001c8 = 0;
  *(uint32_t*)0x200001cc = 0;
  *(uint8_t*)0x200001d0 = 0;
  *(uint8_t*)0x200001d1 = 0;
  *(uint16_t*)0x200001d2 = htobe16(0xa);
  *(uint32_t*)0x200001d4 = 0x20;
  *(uint8_t*)0x200001d8 = 0;
  *(uint8_t*)0x200001d9 = 0xa;
  *(uint16_t*)0x200001da = 3;
  *(uint32_t*)0x200001dc = 0;
  *(uint32_t*)0x200001e0 = 0;
  *(uint8_t*)0x200001e4 = 0;
  *(uint8_t*)0x200001e5 = 0;
  *(uint16_t*)0x200001e6 = htobe16(0);
  *(uint16_t*)0x200001e8 = 9;
  *(uint16_t*)0x200001ea = 1;
  memcpy((void*)0x200001ec, "syz0\000", 5);
  *(uint32_t*)0x200001f4 = 0x70;
  *(uint8_t*)0x200001f8 = 9;
  *(uint8_t*)0x200001f9 = 0xa;
  *(uint16_t*)0x200001fa = 0x401;
  *(uint32_t*)0x200001fc = 0;
  *(uint32_t*)0x20000200 = 0;
  *(uint8_t*)0x20000204 = 0;
  *(uint8_t*)0x20000205 = 0;
  *(uint16_t*)0x20000206 = htobe16(0);
  *(uint16_t*)0x20000208 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x2000020a, 0xa, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000020b, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000020b, 0, 7, 1);
  *(uint32_t*)0x2000020c = htobe32(0);
  *(uint16_t*)0x20000210 = 9;
  *(uint16_t*)0x20000212 = 2;
  memcpy((void*)0x20000214, "syz1\000", 5);
  *(uint16_t*)0x2000021c = 9;
  *(uint16_t*)0x2000021e = 1;
  memcpy((void*)0x20000220, "syz0\000", 5);
  *(uint16_t*)0x20000228 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x2000022a, 5, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000022b, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000022b, 0, 7, 1);
  *(uint32_t*)0x2000022c = htobe32(0x2e);
  *(uint16_t*)0x20000230 = 0x34;
  STORE_BY_BITMASK(uint16_t, , 0x20000232, 0x11, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000233, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000233, 1, 7, 1);
  *(uint16_t*)0x20000234 = 0xa;
  *(uint16_t*)0x20000236 = 1;
  memcpy((void*)0x20000238, "limit\000", 6);
  *(uint16_t*)0x20000240 = 0x24;
  STORE_BY_BITMASK(uint16_t, , 0x20000242, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000243, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000243, 1, 7, 1);
  *(uint16_t*)0x20000244 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x20000246, 4, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000247, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000247, 0, 7, 1);
  *(uint32_t*)0x20000248 = htobe32(1);
  *(uint16_t*)0x2000024c = 0xc;
  STORE_BY_BITMASK(uint16_t, , 0x2000024e, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000024f, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000024f, 0, 7, 1);
  *(uint64_t*)0x20000250 = htobe64(0);
  *(uint16_t*)0x20000258 = 0xc;
  STORE_BY_BITMASK(uint16_t, , 0x2000025a, 1, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000025b, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000025b, 0, 7, 1);
  *(uint64_t*)0x2000025c = htobe64(0x200000000000);
  *(uint32_t*)0x20000264 = 0x14;
  *(uint16_t*)0x20000268 = 0x10;
  *(uint16_t*)0x2000026a = 1;
  *(uint32_t*)0x2000026c = 0;
  *(uint32_t*)0x20000270 = 0;
  *(uint8_t*)0x20000274 = 0;
  *(uint8_t*)0x20000275 = 0;
  *(uint16_t*)0x20000276 = htobe16(0xa);
  *(uint64_t*)0x20000008 = 0xb8;
  *(uint64_t*)0x200000d8 = 1;
  *(uint64_t*)0x200000e0 = 0;
  *(uint64_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000f0 = 0;
  syscall(__NR_sendmsg, r[0], 0x200000c0ul, 0ul);
  return 0;
}