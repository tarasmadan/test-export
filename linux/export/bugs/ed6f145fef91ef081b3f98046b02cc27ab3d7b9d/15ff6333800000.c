// https://syzkaller.appspot.com/bug?id=ed6f145fef91ef081b3f98046b02cc27ab3d7b9d
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void loop()
{
  long res;
  memcpy((void*)0x202ac000, "/dev/vhost-net", 15);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x202ac000, 2, 0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000040 = 0;
  syscall(__NR_ioctl, r[0], 0x40000000af01, 0x20000040);
  memcpy((void*)0x20000180, "/dev/audio", 11);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000180, 0, 0);
  if (res != -1)
    r[1] = res;
  syscall(__NR_close, r[1]);
  syscall(__NR_socket, 0x11, 3, 0x300);
  *(uint32_t*)0x20000500 = 0;
  *(uint32_t*)0x20000504 = 1;
  *(uint64_t*)0x20000508 = 0x20000740;
  *(uint64_t*)0x20000510 = 0x200003c0;
  *(uint64_t*)0x20000518 = 0x20000140;
  *(uint64_t*)0x20000520 = 0xfffffffffffffffc;
  syscall(__NR_ioctl, r[0], 0x4028af11, 0x20000500);
  *(uint64_t*)0x20000640 = 0x200000000;
  syscall(__NR_ioctl, r[0], 0x4008af00, 0x20000640);
  *(uint32_t*)0x20000580 = 1;
  *(uint64_t*)0x20000588 = 0x200001c0;
  *(uint64_t*)0x20000590 = 0x34c;
  *(uint64_t*)0x20000598 = 0x20000480;
  *(uint8_t*)0x200005a0 = -1;
  *(uint8_t*)0x200005a1 = 2;
  *(uint64_t*)0x200005a8 = 0;
  *(uint64_t*)0x200005b0 = 0;
  *(uint64_t*)0x200005b8 = 0;
  *(uint64_t*)0x200005c0 = 0;
  *(uint64_t*)0x200005c8 = 0;
  *(uint64_t*)0x200005d0 = 0;
  *(uint64_t*)0x200005d8 = 0;
  *(uint64_t*)0x200005e0 = 0;
  syscall(__NR_write, r[0], 0x20000580, 0x68);
  *(uint32_t*)0x20d7c000 = 0;
  *(uint32_t*)0x20d7c004 = r[1];
  syscall(__NR_ioctl, r[0], 0x4008af30, 0x20d7c000);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
