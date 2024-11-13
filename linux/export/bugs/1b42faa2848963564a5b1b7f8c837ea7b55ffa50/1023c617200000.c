// https://syzkaller.appspot.com/bug?id=1b42faa2848963564a5b1b7f8c837ea7b55ffa50
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

static long syz_open_pts(volatile long a0, volatile long a1)
{
  int ptyno = 0;
  if (ioctl(a0, TIOCGPTN, &ptyno))
    return -1;
  char buf[128];
  sprintf(buf, "/dev/pts/%d", ptyno);
  return open(buf, a1, 0);
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000000, "/dev/ptmx\000", 10);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000000, 0, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x205befdc = 0;
  *(uint32_t*)0x205befe0 = 0;
  *(uint32_t*)0x205befe4 = 0;
  *(uint32_t*)0x205befe8 = 0;
  *(uint8_t*)0x205befec = 0;
  *(uint8_t*)0x205befed = 0;
  *(uint8_t*)0x205befee = 0;
  *(uint8_t*)0x205befef = 0;
  *(uint32_t*)0x205beff0 = 0;
  *(uint32_t*)0x205beff4 = 0;
  *(uint32_t*)0x205beff8 = 0;
  *(uint32_t*)0x205beffc = 0;
  syscall(__NR_ioctl, r[0], 0x40045431, 0x205befdc);
  res = syz_open_pts(r[0], 0);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x200000c0 = 0xf;
  syscall(__NR_ioctl, r[1], 0x5423, 0x200000c0);
  syscall(__NR_ioctl, r[1], 0x400455c8, 0xb);
  return 0;
}
