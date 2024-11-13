// https://syzkaller.appspot.com/bug?id=5579b93c3266d954e7512ddf31cabff02fb1a394
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
  *(uint16_t*)0x20000280 = 0x26;
  memcpy((void*)0x20000282, "hash\000\000\000\000\000\000\000\000\000\000", 14);
  *(uint32_t*)0x20000290 = 0;
  *(uint32_t*)0x20000294 = 0;
  memcpy((void*)0x20000298,
         "digest_null-"
         "generic\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000",
         64);
  syscall(__NR_bind, r[0], 0x20000280ul, 0x58ul);
  res = syscall(__NR_accept, r[0], 0ul, 0ul);
  if (res != -1)
    r[1] = res;
  syscall(__NR_setsockopt, r[0], 0x117, 1, 0ul, 0ul);
  *(uint64_t*)0x20001a40 = 0;
  *(uint32_t*)0x20001a48 = 0;
  *(uint64_t*)0x20001a50 = 0;
  *(uint64_t*)0x20001a58 = 0;
  *(uint64_t*)0x20001a60 = 0;
  *(uint64_t*)0x20001a68 = 0;
  *(uint32_t*)0x20001a70 = 0;
  syscall(__NR_sendmmsg, r[1], 0x20001a40ul, 0x4924924924925f4ul, 0x48000ul);
  syscall(__NR_accept, r[1], 0ul, 0ul);
  return 0;
}
