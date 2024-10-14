// https://syzkaller.appspot.com/bug?id=66320cf620df291adfe9953b0f944ca117dad37b
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
  res = syscall(__NR_socket, 0x10, 3, 4);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0x20000140;
  *(uint32_t*)0x20000140 = 0x200002c0;
  *(uint32_t*)0x200002c0 = 0x54;
  *(uint16_t*)0x200002c4 = 0x12;
  *(uint16_t*)0x200002c6 = 0x105;
  *(uint32_t*)0x200002c8 = 0;
  *(uint32_t*)0x200002cc = 0;
  *(uint16_t*)0x200002d0 = 0x14;
  STORE_BY_BITMASK(uint16_t, , 0x200002d2, 0, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200002d3, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200002d3, 0, 7, 1);
  *(uint64_t*)0x200002d4 = htobe64(0);
  *(uint64_t*)0x200002dc = htobe64(1);
  *(uint16_t*)0x200002e4 = 0x25;
  STORE_BY_BITMASK(uint16_t, , 0x200002e6, 0, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200002e7, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200002e7, 1, 7, 1);
  *(uint16_t*)0x200002e8 = 4;
  STORE_BY_BITMASK(uint16_t, , 0x200002ea, 0, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200002eb, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200002eb, 0, 7, 1);
  memcpy((void*)0x200002ec, "\xab\x02\xf8\x1c\x58\x02\xab\x60\x02\xd1\xf7\xa3"
                            "\x04\x00\xd6\xa2\x38\xc0\xeb\x48\x9e\xde\x2c\x52"
                            "\x90\xff\x52\x6e\x53",
         29);
  *(uint16_t*)0x2000030c = 8;
  STORE_BY_BITMASK(uint16_t, , 0x2000030e, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000030f, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000030f, 1, 7, 1);
  *(uint16_t*)0x20000310 = 4;
  STORE_BY_BITMASK(uint16_t, , 0x20000312, 1, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000313, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000313, 0, 7, 1);
  *(uint32_t*)0x20000144 = 0x54;
  *(uint32_t*)0x2000000c = 1;
  *(uint32_t*)0x20000010 = 0;
  *(uint32_t*)0x20000014 = 0;
  *(uint32_t*)0x20000018 = 0;
  syscall(__NR_sendmsg, (intptr_t)r[0], 0x20000000, 0);
  return 0;
}
