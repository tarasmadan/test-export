// https://syzkaller.appspot.com/bug?id=30dffac9601663950bb6b107ad73fad40952d73b
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
  res = syscall(__NR_socket, 0xa, 5, 0x84);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000080 = 0x20000000;
  *(uint16_t*)0x20000000 = 0x10;
  *(uint16_t*)0x20000002 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x20000088 = 0xc;
  *(uint64_t*)0x20000090 = 0x200013c0;
  *(uint64_t*)0x200013c0 = 0x20000040;
  *(uint32_t*)0x20000040 = 0x24;
  *(uint16_t*)0x20000044 = 0x10;
  *(uint16_t*)0x20000046 = 0;
  *(uint32_t*)0x20000048 = 0;
  *(uint32_t*)0x2000004c = 0;
  *(uint8_t*)0x20000050 = 0;
  *(uint8_t*)0x20000051 = 0;
  *(uint16_t*)0x20000052 = 0;
  *(uint32_t*)0x20000054 = 0;
  *(uint32_t*)0x20000058 = 0;
  *(uint32_t*)0x2000005c = 0;
  *(uint16_t*)0x20000060 = 4;
  *(uint16_t*)0x20000062 = 0x1a;
  *(uint64_t*)0x200013c8 = 0xffdd;
  *(uint64_t*)0x20000098 = 1;
  *(uint64_t*)0x200000a0 = 0;
  *(uint64_t*)0x200000a8 = 0;
  *(uint32_t*)0x200000b0 = 0;
  syscall(__NR_sendmsg, -1, 0x20000080, 0);
  memcpy((void*)0x20000000,
         "\x69\x70\x36\x5f\x76\x74\x69\x30\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint64_t*)0x20000010 = 0x20000040;
  memcpy((void*)0x20000040, "\xd5\x48\x57\x4b\x2f\x8c\x03\x3d\x46\xcd\x9b\x6c"
                            "\xe2\x88\x1e\x43\x41\x32\x83\x3d\x29\x37\x48\xda"
                            "\x73\xa5\xf7\x4b\x2e\x23\xca\xae",
         32);
  syscall(__NR_ioctl, r[0], 0x8000000000089f1, 0x20000000);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
