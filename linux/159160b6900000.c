// https://syzkaller.appspot.com/bug?id=e2275337450bbf2ed5218b2aec6747f3022078be
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
  memcpy((void*)0x200002c0, "team0\000\000\000\000\000\000\000\000\000\000\000",
         16);
  *(uint32_t*)0x200002d0 = 0;
  syscall(__NR_ioctl, -1, 0x8933, 0x200002c0ul);
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000000 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint64_t*)0x20000010 = 0x20000040;
  *(uint64_t*)0x20000040 = 0x20000280;
  memcpy((void*)0x20000280, "\x44\x00\x00\x00\x10\x00\x01\x04\x00\x00\x00\xdd"
                            "\xff\xff\xff\x00\x00\x00\x00\x00",
         20);
  *(uint32_t*)0x20000294 = -1;
  memcpy((void*)0x20000298, "\x00\x00\x29\xc0\x00\x00\x00\x00\x24\x00\x12\x00"
                            "\x0c\x00\x01\x00\x62\x72\x69\x64\x67\x65\x00\x00"
                            "\x0c\x00\x02\x00\x08\x00\x05\x00\x01\x00\x00\x00"
                            "\x08\x00\x01",
         39);
  *(uint64_t*)0x20000048 = 0x44;
  *(uint64_t*)0x20000018 = 1;
  *(uint64_t*)0x20000020 = 0;
  *(uint64_t*)0x20000028 = 0;
  *(uint32_t*)0x20000030 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000000ul, 0ul);
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x20000240 = 0;
  *(uint32_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0x20000780;
  *(uint64_t*)0x20000780 = 0x200004c0;
  memcpy((void*)0x200004c0, "\x50\x01\x00\x00\x24\x00\x0b\x0d\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         20);
  *(uint32_t*)0x200004d4 = -1;
  memcpy((void*)0x200004d8, "\x00\x00\x00\x02\xff\xff\xff\xff\x00\x00\x40\x00"
                            "\x08\x00\x01\x00\x72\x65\x64\x00\x24\x01\x02\x00"
                            "\x08\x00\x06\x00\x00\x00\x00\x00\x14\x00\x01\x00"
                            "\x00\x00\x1e\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x03\x00\x00\x04\x01\x02",
         55);
  *(uint64_t*)0x20000788 = 0x150;
  *(uint64_t*)0x20000258 = 1;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint32_t*)0x20000270 = 0;
  syscall(__NR_sendmsg, r[1], 0x20000240ul, 0ul);
  return 0;
}
