// https://syzkaller.appspot.com/bug?id=bea1c185923957c5339d5b43bca13e71047453da
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

uint64_t r[1] = {0xffffffffffffffff};
void execute_one()
{
  long res = 0;
  res = syscall(__NR_socket, 0x26, 5, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x2065b000 = 0x26;
  memcpy((void*)0x2065b002,
         "\x68\x61\x73\x68\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 14);
  *(uint32_t*)0x2065b010 = 0;
  *(uint32_t*)0x2065b014 = 0;
  memcpy((void*)0x2065b018,
         "\x68\x6d\x61\x63\x28\x73\x68\x61\x33\x2d\x33\x38\x34\x29\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  syscall(__NR_bind, r[0], 0x2065b000, 0x58);
  syscall(__NR_setsockopt, r[0], 0x117, 1, 0x20000000, 0x1b5);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
