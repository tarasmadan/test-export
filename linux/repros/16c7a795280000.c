// https://syzkaller.appspot.com/bug?id=0aa7e5a1bad26ae11a64288058adf307f824772f
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x26ul, 5ul, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000000 = 0x26;
  memcpy((void*)0x20000002, "hash\000\000\000\000\000\000\000\000\000\000", 14);
  *(uint32_t*)0x20000010 = 0;
  *(uint32_t*)0x20000014 = 0;
  memcpy((void*)0x20000018,
         "cryptd(crct10dif-generic)"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000",
         64);
  syscall(__NR_bind, r[0], 0x20000000ul, 0x58ul);
  res = syscall(__NR_accept4, r[0], 0ul, 0ul, 0ul);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x20004e80 = 0;
  *(uint32_t*)0x20004e88 = 0;
  *(uint64_t*)0x20004e90 = 0;
  *(uint64_t*)0x20004e98 = 0;
  *(uint64_t*)0x20004ea0 = 0;
  *(uint64_t*)0x20004ea8 = 0;
  *(uint32_t*)0x20004eb0 = 0x40;
  syscall(__NR_sendmmsg, r[1], 0x20004e80ul, 1ul, 0x8000ul);
  syscall(__NR_accept4, r[1], 0ul, 0ul, 0ul);
  return 0;
}