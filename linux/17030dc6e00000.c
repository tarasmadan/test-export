// https://syzkaller.appspot.com/bug?id=2c91a9ebeb17895972ec695b8349426d1904d7b3
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

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0ul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20007300 = 0;
  *(uint32_t*)0x20007308 = 0;
  *(uint64_t*)0x20007310 = 0x200072c0;
  *(uint64_t*)0x200072c0 = 0x20000040;
  memcpy((void*)0x20000040,
         "\x70\x00\x00\x00\x30\x00\x37\x7d\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x5c\x00\x01\x00\x58\x00\x01\x00\x08\x00\x01\x00\x69\x66"
         "\x65\x00\x48\x00\x02\x80\x0a\x00\x03\x00\x00\x65\x7b\x1a\xa6\x81\x00"
         "\x00\x1c\x00\x01\x00\x01\x80\x00\x00\x33\x06\x00\x00\x21\x73\x81\x57"
         "\x05\x00\x00\x00\xff\x00\x00\x00\x00\x00\x00\x00\x1c\x00\x06\x00\x08"
         "\x00\x03\x00\x25\x02\x00\x00\x04\x00\x03\x00\x08\x00\x03\x00\x03\x00"
         "\x00\x00\x04\x00\x01\x00\xe7\xff\x05\x00",
         112);
  *(uint64_t*)0x200072c8 = 0x70;
  *(uint64_t*)0x20007318 = 1;
  *(uint64_t*)0x20007320 = 0;
  *(uint64_t*)0x20007328 = 0;
  *(uint32_t*)0x20007330 = 0;
  syscall(__NR_sendmsg, r[0], 0x20007300ul, 0ul);
  return 0;
}
