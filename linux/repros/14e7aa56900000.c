// https://syzkaller.appspot.com/bug?id=32e20c07949c6d6006f26466022469e33ae69108
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
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0x10);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000000 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint64_t*)0x20000010 = 0x20000100;
  *(uint64_t*)0x20000100 = 0x20000040;
  memcpy((void*)0x20000040, "\x1c\x00\x00\x00\x10\x00\x01\x07\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x0a\x00\x00\x00\x06\x00\x01\x00"
                            "\x20",
         25);
  *(uint64_t*)0x20000108 = 0x1c;
  *(uint64_t*)0x20000018 = 1;
  *(uint64_t*)0x20000020 = 0;
  *(uint64_t*)0x20000028 = 0;
  *(uint32_t*)0x20000030 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000000ul, 0ul);
  return 0;
}
