// https://syzkaller.appspot.com/bug?id=704e8f8042049fb353885d97e1cc7f88fa82b0ce
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
  memcpy((void*)0x20000100, "/dev/infiniband/rdma_cm", 24);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000100, 2, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000c40 = 0;
  *(uint16_t*)0x20000c44 = 0x18;
  *(uint16_t*)0x20000c46 = 0xfa00;
  *(uint64_t*)0x20000c48 = 0;
  *(uint64_t*)0x20000c50 = 0x20000c00;
  *(uint16_t*)0x20000c58 = 0x13f;
  *(uint8_t*)0x20000c5a = 0;
  *(uint8_t*)0x20000c5b = 0;
  *(uint8_t*)0x20000c5c = 0;
  *(uint8_t*)0x20000c5d = 0;
  *(uint8_t*)0x20000c5e = 0;
  *(uint8_t*)0x20000c5f = 0;
  res = syscall(__NR_write, r[0], 0x20000c40, 0x20);
  if (res != -1)
    r[1] = *(uint32_t*)0x20000c00;
  memcpy((void*)0x20000c80, "\x0e\x00\x00\x00\x18\x00\x00\xfa", 8);
  *(uint64_t*)0x20000c88 = 0x20000000;
  *(uint32_t*)0x20000c90 = r[1];
  memcpy((void*)0x20000c94, "\x00\x8f\x49\x1c\x00\xc5\xb0\x7d\x3e\x93\xde\xff",
         12);
  syscall(__NR_write, r[0], 0x20000c80, 0x20);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
