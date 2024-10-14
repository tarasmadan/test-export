// https://syzkaller.appspot.com/bug?id=e5a6d245423583fb049fc0fe985237ea62f3b9b8
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
  }

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xf7d000, 3, 0x32, -1, 0);
  *(uint64_t*)0x20346fc8 = 0x20baefec;
  *(uint32_t*)0x20346fd0 = 0x14;
  *(uint64_t*)0x20346fd8 = 0x20ca3000;
  *(uint64_t*)0x20346fe0 = 4;
  *(uint64_t*)0x20346fe8 = 0x20a01f13;
  *(uint64_t*)0x20346ff0 = 0xed;
  *(uint32_t*)0x20346ff8 = 0;
  *(uint64_t*)0x20ca3000 = 0x20f6a000;
  *(uint64_t*)0x20ca3008 = 0xf;
  *(uint64_t*)0x20ca3010 = 0x20f6afa9;
  *(uint64_t*)0x20ca3018 = 0x57;
  *(uint64_t*)0x20ca3020 = 0x208e3f35;
  *(uint64_t*)0x20ca3028 = 0xcb;
  *(uint64_t*)0x20ca3030 = 0x20cd5ffe;
  *(uint64_t*)0x20ca3038 = 2;
  syscall(__NR_recvmsg, -1, 0x20346fc8, 0);
  *(uint32_t*)0x200ba000 = 6;
  *(uint32_t*)0x200ba004 = 5;
  *(uint64_t*)0x200ba008 = 0x20346fc8;
  *(uint64_t*)0x200ba010 = 0x20f74000;
  *(uint32_t*)0x200ba018 = -1;
  *(uint32_t*)0x200ba01c = 0x36f;
  *(uint64_t*)0x200ba020 = 0x201a7f05;
  *(uint32_t*)0x200ba028 = 0;
  *(uint32_t*)0x200ba02c = 0;
  *(uint8_t*)0x200ba030 = 0;
  *(uint8_t*)0x200ba031 = 0;
  *(uint8_t*)0x200ba032 = 0;
  *(uint8_t*)0x200ba033 = 0;
  *(uint8_t*)0x200ba034 = 0;
  *(uint8_t*)0x200ba035 = 0;
  *(uint8_t*)0x200ba036 = 0;
  *(uint8_t*)0x200ba037 = 0;
  *(uint8_t*)0x200ba038 = 0;
  *(uint8_t*)0x200ba039 = 0;
  *(uint8_t*)0x200ba03a = 0;
  *(uint8_t*)0x200ba03b = 0;
  *(uint8_t*)0x200ba03c = 0;
  *(uint8_t*)0x200ba03d = 0;
  *(uint8_t*)0x200ba03e = 0;
  *(uint8_t*)0x200ba03f = 0;
  *(uint32_t*)0x200ba040 = 0;
  *(uint8_t*)0x20346fc8 = 0x18;
  STORE_BY_BITMASK(uint8_t, 0x20346fc9, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20346fc9, 0, 4, 4);
  *(uint16_t*)0x20346fca = 0;
  *(uint32_t*)0x20346fcc = 0;
  *(uint8_t*)0x20346fd0 = 0;
  *(uint8_t*)0x20346fd1 = 0;
  *(uint16_t*)0x20346fd2 = 0;
  *(uint32_t*)0x20346fd4 = 0;
  STORE_BY_BITMASK(uint8_t, 0x20346fd8, 0x19, 0, 3);
  STORE_BY_BITMASK(uint8_t, 0x20346fd8, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, 0x20346fd8, 6, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x20346fd9, 5, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20346fd9, 1, 4, 4);
  *(uint16_t*)0x20346fda = 0xc;
  *(uint32_t*)0x20346fdc = 0;
  *(uint8_t*)0x20346fe8 = 0x95;
  *(uint8_t*)0x20346fe9 = 0;
  *(uint16_t*)0x20346fea = 0;
  *(uint32_t*)0x20346fec = 0;
  memcpy((void*)0x20f74000, "\x02\x00\x00\x00\x00\x00\x00\x00\x00", 9);
  r[0] = syscall(__NR_bpf, 5, 0x200ba000, 0x1e5);
  *(uint32_t*)0x20f7cfd8 = r[0];
  *(uint32_t*)0x20f7cfdc = 0;
  *(uint32_t*)0x20f7cfe0 = 0xbd;
  *(uint32_t*)0x20f7cfe4 = 0x6b;
  *(uint64_t*)0x20f7cfe8 = 0x2072b000;
  *(uint64_t*)0x20f7cff0 = 0x20909000;
  *(uint32_t*)0x20f7cff8 = 1;
  *(uint32_t*)0x20f7cffc = 2;
  memcpy((void*)0x2072b000,
         "\xd8\x4b\x5d\xc9\xcf\xa6\xff\xfc\x84\x03\xa1\xa7\x34\x5f\x82\xaa\x75"
         "\xdc\x5f\x5a\x6b\xf9\xda\xe5\x6e\xe3\xbc\x9e\x18\x77\x30\x20\xbe\xa1"
         "\x1c\xd2\x21\x1d\x89\x39\x4f\x2c\x53\x2b\xe8\x3e\x07\x94\x44\xc8\x0c"
         "\x47\x1b\xb6\x34\xec\x89\xa1\xff\x4f\x13\xb0\xfe\x5e\x18\x5a\x64\x1c"
         "\x98\x87\x70\x5c\xa3\x6d\x5d\x77\x24\x1d\x23\xfd\x3b\x11\x98\xb1\xf4"
         "\xb2\xbe\xf6\xaf\xe0\xc5\x09\x25\xa4\xad\xa5\xdc\xfd\xcb\xbf\x68\x2f"
         "\x0f\x46\xa5\x96\xc9\x72\x88\x50\xf4\x42\x1a\x95\xa0\xe3\x7b\x65\x13"
         "\x2f\x5b\xe8\xeb\xfb\xcc\xaf\xb6\x3e\xd7\x68\xcc\xce\x1d\x38\xe0\x72"
         "\x8c\x78\x14\x54\x77\xda\x81\x2c\xad\xa2\x57\xd0\x6f\x44\x89\x4e\x30"
         "\xca\xaf\xfa\x5f\x5e\x26\x83\xc9\xfe\x15\xca\x24\x52\x38\xd5\x2e\xdd"
         "\x19\x56\xb2\x80\x09\x3e\xf3\x41\x48\x9d\x60\x07\xff\xa4\xfa\xac\x08"
         "\x3f\x2a",
         189);
  syscall(__NR_bpf, 0xa, 0x20f7cfd8, 0x28);
}

int main()
{
  loop();
  return 0;
}
