// https://syzkaller.appspot.com/bug?id=660cabc1b8bdb5a0a68e37cc37ed4c9229199e9e
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

static void execute_one();
extern unsigned long long procid;

void loop()
{
  while (1) {
    execute_one();
  }
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

void execute_one()
{
  *(uint32_t*)0x20346fd4 = 0;
  *(uint32_t*)0x20346fd8 = 0;
  *(uint32_t*)0x20346fdc = 0;
  *(uint32_t*)0x20346fe0 = 0x47;
  *(uint32_t*)0x20346fe4 = 0;
  *(uint32_t*)0x20346fe8 = -1;
  *(uint32_t*)0x20346fec = 0;
  *(uint8_t*)0x20346ff0 = 0;
  *(uint8_t*)0x20346ff1 = 0;
  *(uint8_t*)0x20346ff2 = 0;
  *(uint8_t*)0x20346ff3 = 0;
  *(uint8_t*)0x20346ff4 = 0;
  *(uint8_t*)0x20346ff5 = 0;
  *(uint8_t*)0x20346ff6 = 0;
  *(uint8_t*)0x20346ff7 = 0;
  *(uint8_t*)0x20346ff8 = 0;
  *(uint8_t*)0x20346ff9 = 0;
  *(uint8_t*)0x20346ffa = 0;
  *(uint8_t*)0x20346ffb = 0;
  *(uint8_t*)0x20346ffc = 0;
  *(uint8_t*)0x20346ffd = 0;
  *(uint8_t*)0x20346ffe = 0;
  *(uint8_t*)0x20346fff = 0;
  syscall(__NR_bpf, 0, 0x20346fd4, 0xfffffffffffffe3d);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}