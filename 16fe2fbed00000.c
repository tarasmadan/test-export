// https://syzkaller.appspot.com/bug?id=d68e1ee659d53a5d3dcf81aeb9414a18e9924593
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

const int kInitNetNsFd = 239;

static long syz_init_net_socket(volatile long domain, volatile long type,
                                volatile long proto)
{
  return syscall(__NR_socket, domain, type, proto);
}

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = -1;
  res = syz_init_net_socket(0x1a, 1, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000180 = 0x1a;
  *(uint16_t*)0x20000182 = 0;
  *(uint8_t*)0x20000184 = 0;
  *(uint8_t*)0x20000185 = 8;
  *(uint8_t*)0x20000186 = 0;
  *(uint8_t*)0x20000187 = 0;
  *(uint8_t*)0x20000188 = 0xaa;
  *(uint8_t*)0x20000189 = 0xaa;
  *(uint8_t*)0x2000018a = 0xaa;
  *(uint8_t*)0x2000018b = 0xaa;
  *(uint8_t*)0x2000018c = 0xaa;
  *(uint8_t*)0x2000018d = 0;
  syscall(__NR_connect, r[0], 0x20000180ul, 0x10ul);
  *(uint64_t*)0x20005bc0 = 0;
  *(uint32_t*)0x20005bc8 = 0;
  *(uint64_t*)0x20005bd0 = 0x20000180;
  *(uint64_t*)0x20000180 = 0x20000080;
  memcpy((void*)0x20000080,
         "\x78\x4f\xeb\x70\x59\x08\xac\x54\x09\x51\x05\xfa\xf9\x3d\xf6\xee\x49"
         "\xb7\x09\xde\x8c\xfe\xe0\xd6\x78\x37\x60\x77\xe7\x4e\xf3\x3d\xa7\x79"
         "\x15\x68\x53\x28\xb8\xa1\x36\x9e\xfc\x4d\x5c\xde\x25\xb2\x5e\x31\xeb"
         "\xa3\xc3\xe6\x1f\x62\x2f\x46\x2a\xd6\x8d\x20\xac\xba\x94\x66\xc5\xc0"
         "\xe4\x15\x35\x66\x01\x49\x33\xc9\xc2\xa7\x60\xea\x54\x1e\xf1\xe9\xea"
         "\x4a\x55\xb9\xa4\xed\x7f\xc1\xbf\x1a\xc7\xfb\x97\x62\x7a\xc5\xba\xc1"
         "\xad\xfa\x61\x76\x83\xb4\x61\x9d\xb3\x77\xec\xed\x4c\xa2\xee\xb2\x45"
         "\xe4\x2d\x2b\x2d\x63\xb5\x2d\xde\xe8\x92\x8b\x70\x38\x69\x82\x69\xf7"
         "\x2f\x82\x28\x60\x53\x69\x48\x4a\x28\x1d\xe9\x7f\x11\x6f\x61\x18\xb9"
         "\x73\x3f\xae\xe4\xdd\x7e\x71\x4b\xab\x13\x0f\x54\x67\x54\x24\x48\x6f"
         "\x5f\x34\xcd\xf4\x75\x10\x3e\x14\x9e\x15\x70\x0a\x2c\x56\x16\xa2\xd3"
         "\x22\x59\x9a\xb4\x4e\xc3\xf3\xa4\xd8\x7c\x8e\x87\x8b\xbe\xca\xfc\x42",
         204);
  *(uint64_t*)0x20000188 = 0xcc;
  *(uint64_t*)0x20000190 = 0x20000240;
  memcpy(
      (void*)0x20000240,
      "\x1b\x9e\xc1\x4c\xe6\x66\xfd\x64\x66\x9c\xa0\x51\x6e\x15\x61\x95\x1e\xfb"
      "\x97\x6e\x77\xc0\x5c\xd8\x04\xe6\x3c\x35\xab\x44\x90\xcd\xb9\x23\x9d\x63"
      "\x6c\x0a\x0b\x24\x3a\x3a\x5b\xef\xee\xd2\xc1\x62\xf9\x1f\x7c\xc6\x60\xe7"
      "\xf1\xf1\xaf\x85\x88\x1b\x13\x46\x9e\xf6\x19\x8a\x08\x68\x24\xda\x9f\x1a"
      "\xe0\x95\x68\xf3\xbd\xb9\x0b\x0d\xb3\x5c\x83\x43\x5e\xe3\xef\x9c\x1f\x89"
      "\x87\x06\xb4\xbc\x49\x5a\x81\xc6\xc3\x7d\xf5\x1d\xc7\x76\xc0\x2d\x1c\x66"
      "\x80\x2f\x81\x77\x2c\xe8\x24\x58\x08\x04\x5d\x90\xd8\x27\xf0\xc3\x8b\xf4"
      "\x72\x9f\x4b\x52\xa2\x31\x68\x63\x31\x16\x3f\x04\x9a\x2b\x98\xa2\x4c\xe0"
      "\x5e\x15\x23\x37\x1e\xf8\x40\x3e\x47\xf6\x92\x90\xd4\x17\xf0\x4f\x2f\x41"
      "\x0b\xf9\x51\xf7\x99\x49\x32\xa6\x0c\x23\x92\x3e\x10\xe4\xbc\x28\x26\x3d"
      "\xf0\xfc\xc2\x24\x04\x35\x8d\xe8\x17\x80\x38\x08\xea\x06\xf0\x7e\x41\x86"
      "\x46\x3e\xf5\x21\x39\x6f\xb1\x74\xdf\x06\x59\xb8\x8e\xd2\x2a\xca\x13\x3c"
      "\xc7\x21\xa7\x50\xac\xd9\xfd\x19\x51\xf9\xa1\x9b\x72\x7c\x4b\x49\x29\xf5"
      "\x83\x98\x77\x08\x15\xc6\x7f\x3b\x3b\x22\x2c\xf0",
      246);
  *(uint64_t*)0x20000198 = 0xf6;
  *(uint64_t*)0x200001a0 = 0x20000340;
  memcpy((void*)0x20000340,
         "\x86\x22\xd1\x3e\x2b\xd4\x38\xf8\x3a\x43\xa8\x04\x4c\xbe\x89\x40\x62"
         "\x4c\xc9\x71\x50\x5f\xe4\x04\x01\xc0\xef\xcb\x32\xe8\x8a\x13\x1f\x1b"
         "\xde\x0a\xc7\x03\x22\xf9\x52\x3d\xbe\x4a\x29\x0f\x74\x5a\xa0\x80\xb4"
         "\xee\xda\xe3\xd4\x56\xaf\x3e\x60\x26\x90\x61\x1d\xeb\xb8\x66\x55\x45"
         "\x3f\x4d\xa6\x21\x96\xe3\x5a\x2f\x75\x33\x48\xfd\xf4\x43\x9c\x1c\xa3"
         "\xd3\x3d\x8d\xc3\x9e\xb6\x66\x15\x8e\x6a\x83\xfc\x80\x65\x1e\x93\x99"
         "\xd8\x91\x96\x03\xf3\xac\x27\x2c\xf5\x20\x69\xb9\xfe\x87\xda\x75\xb7"
         "\xf2\x24\x41\xce\x33\xb8\x08\x51\x03\x0f\x81\xa7\x36\xf8\x2b\x59\xa5"
         "\x7e\x57\xd8\x88\x50\x74\x69\xcf\x3b\xa0\xbd\x6e\x3a\xbc\x3f\x18\x9a"
         "\x3d\x13\x8a\x71\x0f\x86\x20\x91\x09\x94\x25\xd5\x98\x3a\x2a\x15\x1e"
         "\x02\xc4\x1c\xd8\x16\xb7\x7f\x56\x0a\x77\x22\x0b\x2e\xf7\xa9\x56\x73"
         "\x4d\x79\xaf\x3b\x80\xd1\xa2\x5a\x80\x70\x64\xc2\x43\xed\x14\xc9\x01"
         "\xd6\x1b\x25\x54\x4b\x60\x63\x76\x38\x2c\xf3\x9c\x51\xcc\xb0\x63\x57"
         "\xbc\xac\x40\x3e\x54\x02\x32\x58\xd5\xc9\x5b\x77\xa9\x77\xbd",
         236);
  *(uint64_t*)0x200001a8 = 0xec;
  *(uint64_t*)0x20005bd8 = 3;
  *(uint64_t*)0x20005be0 = 0;
  *(uint64_t*)0x20005be8 = 0;
  *(uint32_t*)0x20005bf0 = 0;
  *(uint32_t*)0x20005bf8 = 0;
  *(uint64_t*)0x20005c00 = 0;
  *(uint32_t*)0x20005c08 = 0;
  *(uint64_t*)0x20005c10 = 0;
  *(uint64_t*)0x20005c18 = 0;
  *(uint64_t*)0x20005c20 = 0;
  *(uint64_t*)0x20005c28 = 0;
  *(uint32_t*)0x20005c30 = 0;
  *(uint32_t*)0x20005c38 = 0;
  *(uint64_t*)0x20005c40 = 0;
  *(uint32_t*)0x20005c48 = 0;
  *(uint64_t*)0x20005c50 = 0;
  *(uint64_t*)0x20005c58 = 0;
  *(uint64_t*)0x20005c60 = 0;
  *(uint64_t*)0x20005c68 = 0;
  *(uint32_t*)0x20005c70 = 0;
  *(uint32_t*)0x20005c78 = 0;
  *(uint64_t*)0x20005c80 = 0;
  *(uint32_t*)0x20005c88 = 0;
  *(uint64_t*)0x20005c90 = 0;
  *(uint64_t*)0x20005c98 = 0;
  *(uint64_t*)0x20005ca0 = 0;
  *(uint64_t*)0x20005ca8 = 0;
  *(uint32_t*)0x20005cb0 = 0;
  *(uint32_t*)0x20005cb8 = 0;
  *(uint64_t*)0x20005cc0 = 0;
  *(uint32_t*)0x20005cc8 = 0;
  *(uint64_t*)0x20005cd0 = 0;
  *(uint64_t*)0x20005cd8 = 0;
  *(uint64_t*)0x20005ce0 = 0;
  *(uint64_t*)0x20005ce8 = 0;
  *(uint32_t*)0x20005cf0 = 0;
  *(uint32_t*)0x20005cf8 = 0;
  *(uint64_t*)0x20005d00 = 0;
  *(uint32_t*)0x20005d08 = 0;
  *(uint64_t*)0x20005d10 = 0;
  *(uint64_t*)0x20005d18 = 0;
  *(uint64_t*)0x20005d20 = 0;
  *(uint64_t*)0x20005d28 = 0;
  *(uint32_t*)0x20005d30 = 0;
  *(uint32_t*)0x20005d38 = 0;
  syscall(__NR_sendmmsg, r[0], 0x20005bc0ul, 6ul, 0x4000000ul);
  return 0;
}
