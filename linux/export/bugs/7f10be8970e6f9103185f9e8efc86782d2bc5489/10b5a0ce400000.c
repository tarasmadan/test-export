// https://syzkaller.appspot.com/bug?id=7f10be8970e6f9103185f9e8efc86782d2bc5489
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
  memcpy((void*)0x20000240, "/proc/thread-self/attr/exec", 28);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000240, 2, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000280, "stack ", 6);
  memcpy((void*)0x20000286, "&&", 3);
  syscall(__NR_write, r[0], 0x20000280, 9);
  return 0;
}