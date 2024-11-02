// https://syzkaller.appspot.com/bug?id=e217e779406d3001cbd165d4ec5e4ba35314b379
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

static long syz_open_dev(volatile long a0, volatile long a1, volatile long a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul, /*prot=*/7ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  intptr_t res = 0;
  memcpy((void*)0x200060c0, "/dev/sg#\000", 9);
  res = -1;
  res = syz_open_dev(/*dev=*/0x200060c0, /*id=*/0, /*flags=*/0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000480 = 0x53;
  *(uint32_t*)0x20000484 = 0xfffffffc;
  *(uint8_t*)0x20000488 = 6;
  *(uint8_t*)0x20000489 = 0;
  *(uint16_t*)0x2000048a = 0;
  *(uint32_t*)0x2000048c = 0x7e;
  *(uint64_t*)0x20000490 = 0x20000080;
  *(uint64_t*)0x20000498 = 0x20000340;
  memcpy((void*)0x20000340, "\x7c\x3e\x0b\x15\x2b\x8e", 6);
  *(uint64_t*)0x200004a0 = 0;
  *(uint32_t*)0x200004a8 = 0;
  *(uint32_t*)0x200004ac = 0;
  *(uint32_t*)0x200004b0 = 0;
  *(uint64_t*)0x200004b4 = 0;
  *(uint8_t*)0x200004bc = 0;
  *(uint8_t*)0x200004bd = 0;
  *(uint8_t*)0x200004be = 0;
  *(uint8_t*)0x200004bf = 0;
  *(uint16_t*)0x200004c0 = 0;
  *(uint16_t*)0x200004c2 = 0;
  *(uint32_t*)0x200004c4 = 0;
  *(uint32_t*)0x200004c8 = 0;
  *(uint32_t*)0x200004cc = 0;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x2285, /*arg=*/0x20000480ul);
  return 0;
}