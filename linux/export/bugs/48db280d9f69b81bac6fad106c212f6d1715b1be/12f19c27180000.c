// https://syzkaller.appspot.com/bug?id=48db280d9f69b81bac6fad106c212f6d1715b1be
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

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);

  *(uint32_t*)0x20000600 = 0x12;
  *(uint32_t*)0x20000604 = 6;
  *(uint64_t*)0x20000608 = 0x20000040;
  *(uint8_t*)0x20000040 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000041, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000041, 0, 4, 4);
  *(uint16_t*)0x20000042 = 0;
  *(uint32_t*)0x20000044 = 0;
  *(uint8_t*)0x20000048 = 0;
  *(uint8_t*)0x20000049 = 0;
  *(uint16_t*)0x2000004a = 0;
  *(uint32_t*)0x2000004c = 0;
  *(uint8_t*)0x20000050 = 0x85;
  STORE_BY_BITMASK(uint8_t, , 0x20000051, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000051, 1, 4, 4);
  *(uint16_t*)0x20000052 = 0;
  *(uint32_t*)0x20000054 = 1;
  *(uint8_t*)0x20000058 = 0x95;
  *(uint8_t*)0x20000059 = 0;
  *(uint16_t*)0x2000005a = 0;
  *(uint32_t*)0x2000005c = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000060, 4, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000060, 1, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000060, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000061, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000061, 0xa, 4, 4);
  *(uint16_t*)0x20000062 = 0;
  *(uint32_t*)0x20000064 = 0;
  *(uint8_t*)0x20000068 = 0x95;
  *(uint8_t*)0x20000069 = 0;
  *(uint16_t*)0x2000006a = 0;
  *(uint32_t*)0x2000006c = 0;
  *(uint64_t*)0x20000610 = 0x20000000;
  memcpy((void*)0x20000000, "GPL\000", 4);
  *(uint32_t*)0x20000618 = 0;
  *(uint32_t*)0x2000061c = 0;
  *(uint64_t*)0x20000620 = 0;
  *(uint32_t*)0x20000628 = 0;
  *(uint32_t*)0x2000062c = 0;
  memset((void*)0x20000630, 0, 16);
  *(uint32_t*)0x20000640 = 0;
  *(uint32_t*)0x20000644 = 0x13;
  *(uint32_t*)0x20000648 = -1;
  *(uint32_t*)0x2000064c = 8;
  *(uint64_t*)0x20000650 = 0;
  *(uint32_t*)0x20000658 = 0;
  *(uint32_t*)0x2000065c = 0x10;
  *(uint64_t*)0x20000660 = 0;
  *(uint32_t*)0x20000668 = 0;
  *(uint32_t*)0x2000066c = 0;
  *(uint32_t*)0x20000670 = 0;
  *(uint32_t*)0x20000674 = 0;
  *(uint64_t*)0x20000678 = 0;
  *(uint64_t*)0x20000680 = 0;
  *(uint32_t*)0x20000688 = 0x10;
  *(uint32_t*)0x2000068c = 0;
  syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x20000600ul, /*size=*/0x90ul);
  return 0;
}
