// https://syzkaller.appspot.com/bug?id=d64b3c48a3e81dca017e61a50d15880bc677cbb5
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
  res = syscall(__NR_socket, 0x10, 3, 0x10);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20023000 = 0x20024000;
  *(uint16_t*)0x20024000 = 0x10;
  *(uint16_t*)0x20024002 = 0;
  *(uint32_t*)0x20024004 = 0;
  *(uint32_t*)0x20024008 = 0;
  *(uint32_t*)0x20023008 = 0xc;
  *(uint64_t*)0x20023010 = 0x20000040;
  *(uint64_t*)0x20000040 = 0x20000240;
  *(uint32_t*)0x20000240 = 0x1c;
  *(uint16_t*)0x20000244 = 0x28;
  *(uint16_t*)0x20000246 = 0xaff;
  *(uint32_t*)0x20000248 = 0;
  *(uint32_t*)0x2000024c = 0;
  *(uint8_t*)0x20000250 = 1;
  *(uint8_t*)0x20000251 = 0;
  *(uint16_t*)0x20000252 = 0;
  *(uint16_t*)0x20000254 = 8;
  *(uint16_t*)0x20000256 = 0;
  *(uint32_t*)0x20000258 = 0x4102;
  *(uint64_t*)0x20000048 = 0x1c;
  *(uint64_t*)0x20023018 = 1;
  *(uint64_t*)0x20023020 = 0;
  *(uint64_t*)0x20023028 = 0;
  *(uint32_t*)0x20023030 = 0;
  syscall(__NR_sendmsg, r[0], 0x20023000, 0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
