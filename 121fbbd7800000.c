// https://syzkaller.appspot.com/bug?id=98de7581d334faa54310825a332cafcddfcb5bb7
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
  res = syscall(__NR_socket, 2, 2, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000480, "\x66\x69\x6c\x74\x65\x72\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x200004a0 = 0xe;
  *(uint32_t*)0x200004a4 = 2;
  *(uint32_t*)0x200004a8 = 0x288;
  *(uint64_t*)0x200004b0 = 0;
  *(uint64_t*)0x200004b8 = 0x20000140;
  *(uint64_t*)0x200004c0 = 0x20000170;
  *(uint64_t*)0x200004c8 = 0x200002f8;
  *(uint64_t*)0x200004d0 = 0;
  *(uint64_t*)0x200004d8 = 0;
  *(uint32_t*)0x200004e0 = 0;
  *(uint64_t*)0x200004e8 = 0x20000080;
  *(uint64_t*)0x200004f0 = 0x20000140;
  memcpy(
      (void*)0x20000140,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xfe\xff"
      "\xff\xff\x01\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x69\x70"
      "\x36\x67\x72\x65\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x74\x65\x61\x6d"
      "\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x76\x65\x74\x68\x30\x5f"
      "\x74\x6f\x5f\x62\x6f\x6e\x64\x00\x00\x00\x73\x79\x7a\x5f\x74\x75\x6e\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00"
      "\x00\x00\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x28\x01"
      "\x00\x00\x28\x01\x00\x00\x58\x01\x00\x00\x6c\x69\x6d\x69\x74\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x20\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x73\x74\x70\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x48\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\xaa\xaa\xaa\xaa\xaa\xaa\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff"
      "\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00\x41\x55\x44\x49"
      "\x54\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\xfe\xff\xff\xff\x01\x00"
      "\x00\x00\x11\x00\x00\x00\x00\x00\x00\x00\x00\x00\x76\x6c\x61\x6e\x30\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x6c\x6f\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x72\x6f\x73\x65\x30\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x62\x72\x69\x64\x67\x65\x30\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x63\x31"
      "\xf0\x03\xb7\x01\x00\x00\x00\x00\x00\x00\x00\x00\x70\x00\x00\x00\x70\x00"
      "\x00\x00\xa0\x00\x00\x00\x41\x55\x44\x49\x54\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf8\xff"
      "\xff\xff\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00",
      648);
  syscall(__NR_setsockopt, r[0], 0, 0x80, 0x20000480, 0x300);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
