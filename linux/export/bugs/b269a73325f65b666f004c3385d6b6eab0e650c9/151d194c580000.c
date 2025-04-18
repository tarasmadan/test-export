// https://syzkaller.appspot.com/bug?id=b269a73325f65b666f004c3385d6b6eab0e650c9
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffffffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200000000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200001000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  const char* reason;
  (void)reason;
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  res = syscall(__NR_socket, /*domain=*/0x18ul, /*type=*/1ul, /*proto=*/1);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x200000000080,
         "dummy0\000\000\000\000\000\000\000\000\000\000", 16);
  *(uint32_t*)0x200000000090 = 0;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x8922, /*arg=*/0x200000000080ul);
  res = syscall(__NR_socket, /*domain=*/0x10ul, /*type=*/3ul, /*proto=*/0);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x200000000300 = 0;
  *(uint32_t*)0x200000000308 = 0;
  *(uint64_t*)0x200000000310 = 0x2000000002c0;
  *(uint64_t*)0x2000000002c0 = 0x200000000180;
  *(uint32_t*)0x200000000180 = 0x34;
  *(uint16_t*)0x200000000184 = 0x10;
  *(uint16_t*)0x200000000186 = 1;
  *(uint32_t*)0x200000000188 = 0;
  *(uint32_t*)0x20000000018c = 0;
  *(uint8_t*)0x200000000190 = 0;
  *(uint8_t*)0x200000000191 = 0;
  *(uint16_t*)0x200000000192 = 0;
  *(uint32_t*)0x200000000194 = 0;
  *(uint32_t*)0x200000000198 = 0;
  *(uint32_t*)0x20000000019c = 0;
  *(uint16_t*)0x2000000001a0 = 8;
  *(uint16_t*)0x2000000001a2 = 0x1b;
  *(uint32_t*)0x2000000001a4 = 0;
  *(uint16_t*)0x2000000001a8 = 0xc;
  STORE_BY_BITMASK(uint16_t, , 0x2000000001aa, 0x1a, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000000001ab, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000000001ab, 1, 7, 1);
  *(uint16_t*)0x2000000001ac = 8;
  STORE_BY_BITMASK(uint16_t, , 0x2000000001ae, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000000001af, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000000001af, 1, 7, 1);
  *(uint16_t*)0x2000000001b0 = 4;
  STORE_BY_BITMASK(uint16_t, , 0x2000000001b2, 1, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000000001b3, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000000001b3, 1, 7, 1);
  *(uint64_t*)0x2000000002c8 = 0x34;
  *(uint64_t*)0x200000000318 = 1;
  *(uint64_t*)0x200000000320 = 0;
  *(uint64_t*)0x200000000328 = 0;
  *(uint32_t*)0x200000000330 = 0xc0;
  syscall(__NR_sendmsg, /*fd=*/r[1], /*msg=*/0x200000000300ul, /*f=*/0ul);
  return 0;
}
