// https://syzkaller.appspot.com/bug?id=bbe0c977e92315a0462e047c65d280b82fa8838c
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0x40000000015, 5, 0);
  *(uint16_t*)0x20fc4000 = 2;
  *(uint16_t*)0x20fc4002 = htobe16(0x4e20);
  *(uint32_t*)0x20fc4004 = htobe32(0x7f000001);
  *(uint8_t*)0x20fc4008 = 0;
  *(uint8_t*)0x20fc4009 = 0;
  *(uint8_t*)0x20fc400a = 0;
  *(uint8_t*)0x20fc400b = 0;
  *(uint8_t*)0x20fc400c = 0;
  *(uint8_t*)0x20fc400d = 0;
  *(uint8_t*)0x20fc400e = 0;
  *(uint8_t*)0x20fc400f = 0;
  syscall(__NR_bind, r[0], 0x20fc4000, 0x10);
  *(uint64_t*)0x2099ffc4 = 0x20db5ff0;
  *(uint32_t*)0x2099ffcc = 0x10;
  *(uint64_t*)0x2099ffd4 = 0x20542000;
  *(uint64_t*)0x2099ffdc = 0;
  *(uint64_t*)0x2099ffe4 = 0x20a50000;
  *(uint64_t*)0x2099ffec = 0x48;
  *(uint32_t*)0x2099fff4 = 0;
  *(uint32_t*)0x2099fffc = 0;
  *(uint16_t*)0x20db5ff0 = 2;
  *(uint16_t*)0x20db5ff2 = htobe16(0x4e20);
  *(uint8_t*)0x20db5ff4 = 0xac;
  *(uint8_t*)0x20db5ff5 = 0x14;
  *(uint8_t*)0x20db5ff6 = 0;
  *(uint8_t*)0x20db5ff7 = 0xaa;
  *(uint8_t*)0x20db5ff8 = 0;
  *(uint8_t*)0x20db5ff9 = 0;
  *(uint8_t*)0x20db5ffa = 0;
  *(uint8_t*)0x20db5ffb = 0;
  *(uint8_t*)0x20db5ffc = 0;
  *(uint8_t*)0x20db5ffd = 0;
  *(uint8_t*)0x20db5ffe = 0;
  *(uint8_t*)0x20db5fff = 0;
  *(uint64_t*)0x20a50000 = 0x48;
  *(uint32_t*)0x20a50008 = 0x114;
  *(uint32_t*)0x20a5000c = 1;
  memcpy((void*)0x20a50010,
         "\x73\xb5\xe6\x9c\x3f\xcd\xfe\xd2\x42\xc8\x50\xe6\x72\xce\x2d\xe0\x00"
         "\x00\x10\x00\x00\x00\x00\x00\xa6\x02\x5c\xdb\xfc\xca\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x06\x1b\xcf\x00\x00\x03\xba\xa1",
         49);
  syscall(__NR_sendmmsg, r[0], 0x2099ffc4, 1, 0);
}

int main()
{
  loop();
  return 0;
}
