// https://syzkaller.appspot.com/bug?id=7e98ae88bcf946363eb3fabca189b3deb3700caf
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
  res = syscall(__NR_socket, 0x10ul, 3ul, 0x10);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x200001c0 = 0;
  *(uint32_t*)0x200001c8 = 0x6fa10300;
  *(uint64_t*)0x200001d0 = 0x20000100;
  *(uint64_t*)0x20000100 = 0x20000580;
  *(uint32_t*)0x20000580 = 0x1c;
  *(uint16_t*)0x20000584 = 0x27;
  *(uint16_t*)0x20000586 = 0x829;
  *(uint32_t*)0x20000588 = 0;
  *(uint32_t*)0x2000058c = 0;
  *(uint8_t*)0x20000590 = 4;
  *(uint8_t*)0x20000591 = 0;
  *(uint16_t*)0x20000592 = 0;
  *(uint16_t*)0x20000594 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x20000596, 0xc, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000597, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000597, 0, 7, 1);
  *(uint32_t*)0x20000598 = 0;
  *(uint64_t*)0x20000108 = 0x1c;
  *(uint64_t*)0x200001d8 = 1;
  *(uint64_t*)0x200001e0 = 0xffffff7f0e000000;
  *(uint64_t*)0x200001e8 = 0;
  *(uint32_t*)0x200001f0 = 0;
  syscall(__NR_sendmsg, r[0], 0x200001c0ul, 0ul);
  return 0;
}
