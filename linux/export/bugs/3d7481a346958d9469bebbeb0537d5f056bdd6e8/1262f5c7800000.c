// https://syzkaller.appspot.com/bug?id=3d7481a346958d9469bebbeb0537d5f056bdd6e8
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

static uintptr_t syz_open_pts(uintptr_t a0, uintptr_t a1)
{
  int ptyno = 0;
  if (ioctl(a0, TIOCGPTN, &ptyno))
    return -1;
  char buf[128];
  sprintf(buf, "/dev/pts/%d", ptyno);
  return open(buf, a1, 0);
}

static void execute_one();
extern unsigned long long procid;

void loop()
{
  while (1) {
    execute_one();
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void execute_one()
{
  long res = 0;
  memcpy((void*)0x20000000, "/dev/ptmx", 10);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000000, 6, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x203b3fdc = 0;
  *(uint32_t*)0x203b3fe0 = 0;
  *(uint32_t*)0x203b3fe4 = 0;
  *(uint32_t*)0x203b3fe8 = 0;
  *(uint8_t*)0x203b3fec = 0;
  *(uint8_t*)0x203b3fed = 0;
  *(uint8_t*)0x203b3fee = 0;
  *(uint8_t*)0x203b3fef = 0;
  *(uint32_t*)0x203b3ff0 = 0;
  *(uint32_t*)0x203b3ff4 = 0;
  *(uint32_t*)0x203b3ff8 = 0;
  *(uint32_t*)0x203b3ffc = 0;
  syscall(__NR_ioctl, r[0], 0x40045431, 0x203b3fdc);
  memcpy((void*)0x200006c0,
         "\x77\x17\x48\x14\x45\xc0\xc2\x53\x77\x3e\x00\xd4\x9b\xa3\x90\x63\xe2"
         "\x43\x2e\x8d\xe5\x8f\x59\x30\xfd\x07\xbc\x54\xb7\x0c\x0e\xa1\x7b\x47"
         "\x28\xdd\xe5\xf9\xee\xdf\xc8\x11\xff\x1f\x75\x64\x25\x58\x33\x44\x44"
         "\xc9\xfe\x3d\x13\x00\x4d\xcb\x05\xc0\x19\xe5\x47\xd1\xfe\x0e\x54\xa0"
         "\xf9\x9e\xbb\x12\x4b\xbc\xfe\x24\x85\xa8\x33\x95\x7b\x2f\xfe\xd5\x98"
         "\x20\x2a\xfd\xa9\xa2\xaa\x13\x6b\x65\x9d\xdc\x9b\x8b\x3b\x6d\x3f\xaa"
         "\x47\x96\x9b\xa4\x86\x55\x39\xf6\x21\x3e\x4c\x7c\xe5\xec\xf4\x8b\xb3"
         "\x72\x5b\xbc\xc4\xfb\x89\x2a\xb0\x93\x94\x39\x74\x80\x43\x1f\xac\x81"
         "\x32\x44\xfe\x07\x7f\xcc\xda\x59\x13\x86\x8a\x35\x13\xfb\x2f\xfb\x17"
         "\x22\x28\x94\xcc\xc9\xe0\x09\xe6\x59\xc9\xec\x71\x46\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x87\xe4\x95\x7f\xe4\x75\x3d\x90\x85\x6a"
         "\xba\x01\x5e\xd7\x88\x61\xf6\xa9\x08\xfd\x9c\xca\x07\xf7\x5d\x75\xb1"
         "\xfc\x8c\x70\x44\x99\xa3\xa9\x5c\x8a\x70\x72\xa8\xf7\x0c\xdf\x47\x95"
         "\x53\x51\xb9\xac\x14\x93\xe8\xba\x70\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\xd5\xcd\xdb\x88\x98\xeb\x56\x79\x8b\x08\x60\x5b\x94\x1b"
         "\xb0\x59\x25\x09\xad\xb2\xab\xd3\x86\xe1\x9e\x88\x94\x7a\x3d\x55\x15"
         "\xf3\x94\x60\x09\xb8\x69\x33\xe8\x3a\xa8\x6e\xfc\xba\x26\x51\xf3\x32"
         "\x27\xf3\xc8\x3d\xef\xeb\x43\xf3\x85\x00\x00\x00\x00\x00\x00",
         321);
  syscall(__NR_write, r[0], 0x200006c0, 0x141);
  res = syz_open_pts(r[0], 0);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x200000c0 = 0x12;
  *(uint32_t*)0x200000c4 = 0;
  *(uint32_t*)0x200000c8 = 0;
  *(uint32_t*)0x200000cc = 0;
  *(uint8_t*)0x200000d0 = 0;
  *(uint8_t*)0x200000d1 = 0;
  *(uint8_t*)0x200000d2 = 0;
  *(uint8_t*)0x200000d3 = 0;
  *(uint32_t*)0x200000d4 = 0;
  *(uint32_t*)0x200000d8 = 0;
  *(uint32_t*)0x200000dc = 0;
  *(uint32_t*)0x200000e0 = 0;
  syscall(__NR_ioctl, r[1], 0x5412, 0x200000c0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}