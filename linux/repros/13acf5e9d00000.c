// https://syzkaller.appspot.com/bug?id=2484728c9a1d0482d5e27e2c4bb0e184b6dd9ec3
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
  memcpy((void*)0x20003300, "/sys/fs/smackfs/cipso2\000", 23);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20003300ul, 2ul, 0ul);
  if (res != -1)
    r[0] = res;
  syscall(__NR_write, r[0], 0ul, 0xf0ff7ful);
  return 0;
}
