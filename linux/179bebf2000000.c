// https://syzkaller.appspot.com/bug?id=f7964510c9dae24e52cac0c3f67bee9f9d1d533a
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

long r[53];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  memcpy((void*)0x20000000, "\x2f\x64\x65\x76\x2f\x73\x67\x23\x00", 9);
  r[2] = syz_open_dev(0x20000000ul, 0x0ul, 0x2ul);
  *(uint8_t*)0x20012000 = (uint8_t)0x3;
  *(uint8_t*)0x20012001 = (uint8_t)0x80;
  *(uint8_t*)0x20012002 = (uint8_t)0x2;
  *(uint8_t*)0x20012003 = (uint8_t)0x2;
  *(uint64_t*)0x20012008 = (uint64_t)0x0;
  *(uint64_t*)0x20012010 = (uint64_t)0x4;
  *(uint8_t*)0x20012018 = (uint8_t)0xd6;
  *(uint8_t*)0x20012019 = (uint8_t)0x9;
  *(uint8_t*)0x2001201a = (uint8_t)0x4a;
  *(uint8_t*)0x2001201b = (uint8_t)0x1;
  memcpy((void*)0x20012020,
         "\xad\x7f\x6c\x93\x85\x99\x6b\x9a\xdf\x65\x38\x39", 12);
  *(uint8_t*)0x20012030 = (uint8_t)0x0;
  *(uint8_t*)0x20012031 = (uint8_t)0x17;
  *(uint8_t*)0x20012032 = (uint8_t)0x4f39;
  *(uint8_t*)0x20012033 = (uint8_t)0x7;
  *(uint32_t*)0x20012038 = (uint32_t)0xffffffffffffffe0;
  *(uint8_t*)0x20012048 = (uint8_t)0x401;
  *(uint8_t*)0x20012049 = (uint8_t)0x8;
  *(uint8_t*)0x2001204a = (uint8_t)0xfffffffeffffffff;
  *(uint8_t*)0x2001204b = (uint8_t)0x81;
  *(uint8_t*)0x20012050 = (uint8_t)0x1;
  *(uint32_t*)0x20012054 = (uint32_t)0x9;
  *(uint32_t*)0x20012058 = (uint32_t)0x400;
  *(uint8_t*)0x20012060 = (uint8_t)0x1;
  *(uint8_t*)0x20012061 = (uint8_t)0x100000001;
  *(uint8_t*)0x20012062 = (uint8_t)0x201;
  *(uint8_t*)0x20012063 = (uint8_t)0x19c;
  *(uint32_t*)0x20012068 = (uint32_t)0x0;
  *(uint8_t*)0x20012078 = (uint8_t)0x7;
  *(uint8_t*)0x20012079 = (uint8_t)0xfffffffffffffff9;
  *(uint8_t*)0x2001207a = (uint8_t)0xf97;
  *(uint8_t*)0x2001207b = (uint8_t)0x8000;
  *(uint8_t*)0x20012080 = (uint8_t)0x0;
  *(uint8_t*)0x20012081 = (uint8_t)0xfffffffffffffffa;
  *(uint16_t*)0x20012082 = (uint16_t)0x6;
  *(uint64_t*)0x20012084 = (uint64_t)0x20003fd0;
  *(uint8_t*)0x20003fd0 = (uint8_t)0x5;
  *(uint8_t*)0x20003fd1 = (uint8_t)0xd4;
  *(uint8_t*)0x20003fd2 = (uint8_t)0x8;
  *(uint8_t*)0x20003fd3 = (uint8_t)0x5;
  *(uint64_t*)0x20003fd8 = (uint64_t)0x77359400;
  *(uint64_t*)0x20003fe0 = (uint64_t)0x0;
  *(uint8_t*)0x20003fe8 = (uint8_t)0x1;
  *(uint8_t*)0x20003fe9 = (uint8_t)0xffff;
  *(uint8_t*)0x20003fea = (uint8_t)0x4b7c;
  *(uint8_t*)0x20003feb = (uint8_t)0xd4;
  *(uint8_t*)0x20003ff0 = (uint8_t)0x9;
  *(uint32_t*)0x20003ff4 = (uint32_t)0x1;
  *(uint32_t*)0x20003ff8 = (uint32_t)0x1ff;
  r[52] = syscall(__NR_write, r[2], 0x20012000ul, 0x90ul);
}

int main()
{
  loop();
  return 0;
}