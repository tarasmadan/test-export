// https://syzkaller.appspot.com/bug?id=7ee0d0c658bbe261aa2393f2930bbf6763304f04
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_socket, 0xa, 1, 0x84);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000000 = 0xa;
  *(uint16_t*)0x20000002 = htobe16(0);
  *(uint32_t*)0x20000004 = 0;
  *(uint8_t*)0x20000008 = 0;
  *(uint8_t*)0x20000009 = 0;
  *(uint8_t*)0x2000000a = 0;
  *(uint8_t*)0x2000000b = 0;
  *(uint8_t*)0x2000000c = 0;
  *(uint8_t*)0x2000000d = 0;
  *(uint8_t*)0x2000000e = 0;
  *(uint8_t*)0x2000000f = 0;
  *(uint8_t*)0x20000010 = 0;
  *(uint8_t*)0x20000011 = 0;
  *(uint8_t*)0x20000012 = 0;
  *(uint8_t*)0x20000013 = 0;
  *(uint8_t*)0x20000014 = 0;
  *(uint8_t*)0x20000015 = 0;
  *(uint8_t*)0x20000016 = 0;
  *(uint8_t*)0x20000017 = 0;
  *(uint32_t*)0x20000018 = 8;
  syscall(__NR_bind, r[0], 0x20000000, 0x10);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}