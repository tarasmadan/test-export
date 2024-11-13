// https://syzkaller.appspot.com/bug?id=e4be30826c1b7777d69a9e3e20bc7b708ee8f82c
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[5] = {0x0, 0x0, 0x0, 0x0, 0x0};
void loop()
{
  long res = 0;
  memcpy((void*)0x20000300, "keyring", 8);
  *(uint8_t*)0x20000080 = 0x73;
  *(uint8_t*)0x20000081 = 0x79;
  *(uint8_t*)0x20000082 = 0x7a;
  *(uint8_t*)0x20000083 = 0;
  *(uint8_t*)0x20000084 = 0;
  res = syscall(__NR_add_key, 0x20000300, 0x20000080, 0, 0, 0xfffffffe);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x200003c0, "keyring", 8);
  *(uint8_t*)0x20000680 = 0x73;
  *(uint8_t*)0x20000681 = 0x79;
  *(uint8_t*)0x20000682 = 0x7a;
  *(uint8_t*)0x20000683 = 0;
  *(uint8_t*)0x20000684 = 0;
  res = syscall(__NR_add_key, 0x200003c0, 0x20000680, 0, 0, r[0]);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000340, "keyring", 8);
  *(uint8_t*)0x20000240 = 0x73;
  *(uint8_t*)0x20000241 = 0x79;
  *(uint8_t*)0x20000242 = 0x7a;
  *(uint8_t*)0x20000243 = 0;
  *(uint8_t*)0x20000244 = 0;
  res = syscall(__NR_add_key, 0x20000340, 0x20000240, 0, 0, r[1]);
  if (res != -1)
    r[2] = res;
  memcpy((void*)0x200001c0, "user", 5);
  *(uint8_t*)0x20000200 = 0x73;
  *(uint8_t*)0x20000201 = 0x79;
  *(uint8_t*)0x20000202 = 0x7a;
  *(uint8_t*)0x20000203 = 0;
  *(uint8_t*)0x20000204 = 0;
  memcpy((void*)0x20000440, "\x01", 1);
  res = syscall(__NR_add_key, 0x200001c0, 0x20000200, 0x20000440, 1, r[2]);
  if (res != -1)
    r[3] = res;
  memcpy((void*)0x20000000, "user", 5);
  *(uint8_t*)0x200002c0 = 0x73;
  *(uint8_t*)0x200002c1 = 0x79;
  *(uint8_t*)0x200002c2 = 0x7a;
  *(uint8_t*)0x200002c3 = 0;
  *(uint8_t*)0x200002c4 = 0;
  memcpy((void*)0x20000540,
         "\xc1\x47\x54\x9a\xf9\xb7\x94\x6d\x98\x71\x6d\xa0\xab\x93\x76\x10\x72"
         "\x12\x46\x46\x5e\xd3\xeb\x80\x2e\x15\x17\xcf\x4e\x0e\xff\x94\x49\x34"
         "\xe7\x48\xcd\x5a\xcd\xe4\xc2\x17\x58\x84\xbd\x90\x95\xc6\xe3\x46\xec"
         "\x09\xe7\x3f\xff\xca\x1c\x95\xcf\x74\xe9\xe7\x47\x2d\xb2\xb5\xf0\x3e"
         "\x09\xa5\xce\x5e\xc3\x2f\x2c\x30\xd5\xfa\x83\xa0\x29\xd9\x5f\x72\xe8"
         "\x1c\xbb\x01\xde\x8c\x62\xfa\x21\xdf\x74\xa9\xf6\xfb\x65\x5e\x02\xe1"
         "\xbe\x3b\x44\x6b\xbf\xb8\x0e\x25\x7b\x80\x92\x6a\x7a\x66\x00\x5f\x59"
         "\xc6\x2c\xf3\x88\xba\xd1\x33\x55\xe7\xb0\x81\x01\xf4\xa4\x91\x1c\x32"
         "\xc3\x44\x8f\xf8\x3f\x69\x5a\xec\x69\x62\x43\xe5\x0e\x5d\xc0\x44\x30"
         "\x4e\x1a\x07\x23\xf7\x25\xcd\x05\x68\xcc\xd7\x14\x62\x60\x3a\x27\x89"
         "\x78\x9d\xd1\xac\x4b\x28\x4e\x00\x44\x0a\x10\x84\x27\xfc\xb1\x65\xb4"
         "\x28\x60\x5a\x73\x53",
         192);
  res = syscall(__NR_add_key, 0x20000000, 0x200002c0, 0x20000540, 0xc0, r[0]);
  if (res != -1)
    r[4] = res;
  *(uint32_t*)0x20000100 = r[4];
  *(uint32_t*)0x20000104 = r[4];
  *(uint32_t*)0x20000108 = r[3];
  *(uint64_t*)0x20000140 = 0x20000040;
  memcpy((void*)0x20000040,
         "\x73\x68\x61\x33\x38\x34\x2d\x67\x65\x6e\x65\x72\x69\x63\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  *(uint64_t*)0x20000148 = 0;
  *(uint32_t*)0x20000150 = 0;
  *(uint32_t*)0x20000154 = 0;
  *(uint32_t*)0x20000158 = 0;
  *(uint32_t*)0x2000015c = 0;
  *(uint32_t*)0x20000160 = 0;
  *(uint32_t*)0x20000164 = 0;
  *(uint32_t*)0x20000168 = 0;
  *(uint32_t*)0x2000016c = 0;
  *(uint32_t*)0x20000170 = 0;
  syscall(__NR_keyctl, 0x17, 0x20000100, 0x20a53ffb, 0x18, 0x20000140);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
