// https://syzkaller.appspot.com/bug?id=cb04749ba5c9d6a347380b5b79de8fc5cfe47c24
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

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  *(uint32_t*)0x20000000 = 1;
  *(uint32_t*)0x20000004 = 8;
  *(uint64_t*)0x20000008 = 0x20001000;
  memcpy((void*)0x20001000,
         "\x7a\x0a\xf8\xff\x75\x25\x70\x25\xbf\xa1\x00\x00\x00\x00\x00\x00\x07"
         "\x01\x00\x00\xf8\xff\xff\xff\xb7\x02\x00\x00\x05\x00\x00\x00\xbf\x13"
         "\x00\x00\x00\x00\x00\x00\x85\x00\x00\x00\x06\x00\x00\x00\xb7\x00\x00"
         "\x00\x00\x00\x00\x00\x95\x00\x00\xff\x00\x00\x00\x00",
         64);
  *(uint64_t*)0x20000010 = 0x20000100;
  memcpy((void*)0x20000100, "GPL", 4);
  *(uint32_t*)0x20000018 = 0;
  *(uint32_t*)0x2000001c = 0;
  *(uint64_t*)0x20000020 = 0;
  *(uint32_t*)0x20000028 = 0;
  *(uint32_t*)0x2000002c = 0;
  *(uint8_t*)0x20000030 = 0;
  *(uint8_t*)0x20000031 = 0;
  *(uint8_t*)0x20000032 = 0;
  *(uint8_t*)0x20000033 = 0;
  *(uint8_t*)0x20000034 = 0;
  *(uint8_t*)0x20000035 = 0;
  *(uint8_t*)0x20000036 = 0;
  *(uint8_t*)0x20000037 = 0;
  *(uint8_t*)0x20000038 = 0;
  *(uint8_t*)0x20000039 = 0;
  *(uint8_t*)0x2000003a = 0;
  *(uint8_t*)0x2000003b = 0;
  *(uint8_t*)0x2000003c = 0;
  *(uint8_t*)0x2000003d = 0;
  *(uint8_t*)0x2000003e = 0;
  *(uint8_t*)0x2000003f = 0;
  *(uint32_t*)0x20000040 = 0;
  *(uint32_t*)0x20000044 = 0;
  res = syscall(__NR_bpf, 5, 0x20000000, 0x48);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000080 = r[0];
  *(uint32_t*)0x20000084 = 0x2000000;
  *(uint32_t*)0x20000088 = 0xe;
  *(uint32_t*)0x2000008c = 0x55;
  *(uint64_t*)0x20000090 = 0x20000140;
  memcpy((void*)0x20000140,
         "\xa0\x6a\xd8\x76\xd5\x6a\x00\x64\xd0\x82\x77\x8c\x39\x38", 14);
  *(uint64_t*)0x20000098 = 0x20000380;
  *(uint32_t*)0x200000a0 = 0x700;
  *(uint32_t*)0x200000a4 = 0x4000000;
  syscall(__NR_bpf, 0xa, 0x20000080, 0x28);
  return 0;
}