// https://syzkaller.appspot.com/bug?id=a2bd5831b881d8b3ef8a81c54d8fcbcb40855597
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
  long res = 0;
  res = syscall(__NR_socket, 0x26, 5, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000080 = 0x26;
  memcpy((void*)0x20000082,
         "\x61\x65\x61\x64\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 14);
  *(uint32_t*)0x20000090 = 0;
  *(uint32_t*)0x20000094 = 0;
  memcpy((void*)0x20000098,
         "\x67\x65\x6e\x65\x72\x69\x63\x2d\x67\x63\x6d\x2d\x61\x65\x73\x6e\x69"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  syscall(__NR_bind, r[0], 0x20000080, 0x58);
  res = syscall(__NR_accept, r[0], 0, 0);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x203bcfd0,
         "\xd3\xab\x27\x19\x1a\x01\x00\x23\x56\xba\x60\x2d\xff\x05\x00\x0b",
         16);
  syscall(__NR_setsockopt, r[0], 0x117, 1, 0x203bcfd0, 0x10);
  *(uint64_t*)0x20000000 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint64_t*)0x20000010 = 0x200001c0;
  *(uint64_t*)0x20000018 = 0;
  *(uint64_t*)0x20000020 = 0x20000200;
  *(uint64_t*)0x20000200 = 0x18;
  *(uint32_t*)0x20000208 = 0x117;
  *(uint32_t*)0x2000020c = 3;
  *(uint32_t*)0x20000210 = 1;
  *(uint64_t*)0x20000028 = 0x18;
  *(uint32_t*)0x20000030 = 0;
  syscall(__NR_sendmmsg, r[1], 0x20000000, 1, 0);
  *(uint64_t*)0x20000040 = 0x20000240;
  *(uint32_t*)0x20000048 = 0x80;
  *(uint64_t*)0x20000050 = 0x200001c0;
  *(uint64_t*)0x200001c0 = 0x200002c0;
  *(uint64_t*)0x200001c8 = 0x5c;
  *(uint64_t*)0x200001d0 = 0x20001700;
  *(uint64_t*)0x200001d8 = 0x1000;
  *(uint64_t*)0x20000058 = 0x32f;
  *(uint64_t*)0x20000060 = 0x20000340;
  *(uint64_t*)0x20000068 = 0xfffffffffffffea8;
  *(uint32_t*)0x20000070 = 0;
  syscall(__NR_recvmsg, r[1], 0x20000040, 0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
