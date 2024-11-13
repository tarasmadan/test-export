// https://syzkaller.appspot.com/bug?id=dfc9dc4719d1bdec7389b626eec3efb3429cc0dd
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

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

long r[14];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0x281000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  *(uint32_t*)0x20275000 = (uint32_t)0x20000;
  *(uint32_t*)0x20275004 = (uint32_t)0x0;
  *(uint32_t*)0x20275008 = (uint32_t)0x40;
  *(uint32_t*)0x2027500c = (uint32_t)0x5;
  *(uint32_t*)0x20275010 = (uint32_t)0x8;
  *(uint64_t*)0x20275018 = (uint64_t)0x0;
  r[7] = syscall(__NR_ioctl, 0xfffffffffffffffful, 0xc0106418ul,
                 0x20275000ul);
  memcpy((void*)0x2026f000,
         "\x2f\x64\x65\x76\x2f\x62\x75\x73\x2f\x75\x73\x62\x2f\x30\x30"
         "\x23\x2f\x30\x30\x23\x00",
         21);
  r[9] = syz_open_dev(0x2026f000ul, 0xfffffffffffffffbul, 0x602ul);
  *(uint16_t*)0x20274ffa = (uint16_t)0x2;
  *(uint16_t*)0x20274ffc = (uint16_t)0x0;
  *(uint16_t*)0x20274ffe = (uint16_t)0x0;
  r[13] = syscall(__NR_ioctl, r[9], 0x802c550aul, 0x20274ffaul);
}

int main()
{
  loop();
  return 0;
}