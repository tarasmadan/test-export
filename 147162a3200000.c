// https://syzkaller.appspot.com/bug?id=4db14afc80049c484903a7cf4d36d9cb1618469f
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
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  res = syscall(__NR_socket, 0x10, 0x802, 6);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000300 = 0;
  *(uint32_t*)0x20000308 = 0;
  *(uint64_t*)0x20000310 = 0x20000080;
  *(uint64_t*)0x20000080 = 0x20000380;
  memcpy((void*)0x20000380, "\x24\x00\x00\x00\x25\x00\x07\x03\x1d\xff\xfd\x94"
                            "\x6f\xa2\x83\x00\x20\x20\x0a\x00\x09\x00\x00\x00"
                            "\x00\x1d\x85\x68\x0c\x1b\xa3\xa2\x04\x00\xff\x7e",
         36);
  *(uint64_t*)0x20000088 = 0x24;
  *(uint64_t*)0x20000318 = 1;
  *(uint64_t*)0x20000320 = 0;
  *(uint64_t*)0x20000328 = 0;
  *(uint32_t*)0x20000330 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000300, 0);
  return 0;
}
