// https://syzkaller.appspot.com/bug?id=99cf1fa5f0cb6840aa06557609692676382e85e6
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block",
            (uint8_t)a1, (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

long r[11];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0x860000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  memcpy((void*)0x20000000, "\x2f\x64\x65\x76\x2f\x73\x67\x23\x00", 9);
  r[2] = syz_open_dev(0x20000000ul, 0x0ul, 0x2ul);
  *(uint8_t*)0x20010000 = (uint8_t)0x1000002;
  *(uint8_t*)0x20010001 = (uint8_t)0x0;
  *(uint16_t*)0x20010002 = (uint16_t)0x0;
  *(uint16_t*)0x20010004 = (uint16_t)0x0;
  *(uint16_t*)0x20010006 = (uint16_t)0x0;
  *(uint16_t*)0x20010008 = (uint16_t)0x0;
  memcpy((void*)0x2001000a,
         "\x62\x24\xd8\xc7\x34\xe0\x65\xcd\xf1\x6f\x8d\xb0\x9a\xdc\x78"
         "\x57\x47\x34\x45\x84\xc6\x00\x00\x00\x04\x83\x08\x1f\x6a\x89"
         "\x5f",
         31);
  r[10] = syscall(__NR_write, r[2], 0x20010000ul, 0x2aul);
}

int main()
{
  loop();
  return 0;
}
