// https://syzkaller.appspot.com/bug?id=2347b676d397a17b7c7c6c039f27de8486e520fb
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  res = syscall(__NR_socket, 0x10, 0x4000000002, 0x10);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c8 = 0;
  *(uint64_t*)0x200000d0 = 0x20000100;
  *(uint64_t*)0x20000100 = 0x20000280;
  memcpy((void*)0x20000280,
         "\x2e\x00\x00\x00\x2b\x00\x81\x2d\xe4\x5a\xe0\x87\x18\x50\x82\xcf\x01"
         "\x24\xb0\xd0\x57\xe7\x44\x00\x09\x41\x00\x00\x00\x00\x00\x18\x83\xb2"
         "\xe6\xdc\x02\xe7\xdc\x8e\x5c\x8e\xf1\x0b\x80\xa6\x68\x19\xc6\x41\xba"
         "\xa1\x47\x0e\xc6\x13\x89\xe5\xf9\x28\x99\xda\x18\x9f\x62\x63\xa5\xfe"
         "\x37\xd0\x0e\x8b\x64\x7d\x4a\xb1\xd1\x98\xd9\xfa\x9e\x75\x93\xd2\x43"
         "\x3b\x5d\x75\x0e\x0c\x76\x84\xcd\xe7\x87\x7a\x79\x93\xaf\x70\x90\xe7"
         "\xa8\x73",
         104);
  *(uint64_t*)0x20000108 = 0x68;
  *(uint64_t*)0x200000d8 = 1;
  *(uint64_t*)0x200000e0 = 0;
  *(uint64_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000f0 = 0;
  syscall(__NR_sendmsg, r[0], 0x200000c0, 0);
  return 0;
}
