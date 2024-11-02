// https://syzkaller.appspot.com/bug?id=62655fc84ccd812e528e0cdc45386baea96593cb
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

#ifndef __NR_seccomp
#define __NR_seccomp 317
#endif

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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul, /*prot=*/7ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  intptr_t res = 0;
  *(uint16_t*)0x20000100 = 6;
  *(uint64_t*)0x20000108 = 0x20000000;
  *(uint16_t*)0x20000000 = 0;
  *(uint8_t*)0x20000002 = 0;
  *(uint8_t*)0x20000003 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint16_t*)0x20000008 = 0;
  *(uint8_t*)0x2000000a = 0;
  *(uint8_t*)0x2000000b = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint16_t*)0x20000010 = 0;
  *(uint8_t*)0x20000012 = 0;
  *(uint8_t*)0x20000013 = 0;
  *(uint32_t*)0x20000014 = 0;
  *(uint16_t*)0x20000018 = 0x7ffc;
  *(uint8_t*)0x2000001a = 0;
  *(uint8_t*)0x2000001b = 0;
  *(uint32_t*)0x2000001c = 0;
  *(uint16_t*)0x20000020 = 0;
  *(uint8_t*)0x20000022 = 0;
  *(uint8_t*)0x20000023 = 0;
  *(uint32_t*)0x20000024 = 0;
  *(uint16_t*)0x20000028 = 1;
  *(uint8_t*)0x2000002a = 0;
  *(uint8_t*)0x2000002b = 0;
  *(uint32_t*)0x2000002c = 0;
  syscall(__NR_seccomp, /*op=*/1ul, /*flags=*/0ul, /*arg=*/0x20000100ul);
  memcpy((void*)0x200060c0, "/dev/sg#\000", 9);
  res = -1;
  res = syz_open_dev(/*dev=*/0x200060c0, /*id=*/0, /*flags=*/0x8002);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_fcntl, /*fd=*/r[0], /*cmd=*/0ul, /*arg=*/r[0]);
  if (res != -1)
    r[1] = res;
  syscall(__NR_ioctl, /*fd=*/r[1], /*cmd=*/0x5393, /*arg=*/0x20000000ul);
  return 0;
}