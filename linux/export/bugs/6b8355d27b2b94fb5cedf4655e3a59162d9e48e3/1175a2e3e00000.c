// https://syzkaller.appspot.com/bug?id=6b8355d27b2b94fb5cedf4655e3a59162d9e48e3
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

static long syz_open_dev(volatile long a0, volatile long a1, volatile long a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  intptr_t res = 0;
  res = syz_open_dev(0xc, 4, 1);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000080 = 0;
  *(uint32_t*)0x20000084 = 0;
  *(uint32_t*)0x20000088 = 0x10;
  *(uint32_t*)0x2000008c = 6;
  *(uint32_t*)0x20000090 = 0x200;
  *(uint64_t*)0x20000098 = 0x200004c0;
  memcpy(
      (void*)0x200004c0,
      "\x6c\xdf\xaa\xb4\x72\xac\x96\x19\x16\x75\xeb\x1c\xc8\xd3\x00\xc9\x00\x10"
      "\xca\x86\x4b\x58\xcd\x42\xe4\x6a\xdc\x51\xc5\xc7\x1a\x32\x81\xf8\x19\xa4"
      "\xd7\x24\xea\xb2\x48\xbd\x02\x06\xad\xbe\x42\x43\xc6\xc2\x57\xff\xaa\x45"
      "\x77\x80\x24\x31\x8b\x19\x7d\x6b\xe2\x60\xf0\xdf\x26\xe7\x74\xfb\x35\x06"
      "\xc9\xc3\x21\x52\x9f\x23\xf1\x23\xf6\x7f\x27\xcc\x6e\x58\x1a\xa5\x90\x61"
      "\x3b\xdf\x80\x36\x8f\x86\x58\xdc\x6e\x59\x99\xc7\x60\xb9\x67\x54\x4e\x7f"
      "\x06\xca\x4e\x4b\x99\xf5\xb5\xa8\xaa\xe9\x69\xb1\xad\x86\x8f\x14\xcf\x82"
      "\xef\x6f\x26\xe1\xff\x80\x93\xa8\xe9\xd6\x6f\x03\xba\x2d\x28\x28\x95\x99"
      "\xbe\x59\x52\x26\x20\x0d\xd7\x3c\xc6\x06\xcf\x0f\x6b\x1c\xfb\x93\xfc\xa2"
      "\x1a\x48\xd7\x5b\xd8\x67\x34\xce\x02\x9b\xf8\x20\x24\x2f\x3f\x2b\xd7\xa6"
      "\x6a\x63\xee\x4d\xfc\x9d\x6f\x8f\x7b\xef\xb0\x57\x9d\x0f\xf3\xe6\x26\x58"
      "\x32\x2b\x58\xb9\x86\x3f\x32\x07\xbf\x37\xf4\x6a\x4b\x45\x82\xe6\x59\x48"
      "\x28\x54\x34\x53\x35\xb3\xf7\xea\x2d\x76\x24\xd6\x1a\x52\x79\xa3\x0a\xac"
      "\xc8\xd2\x74\x3e\x01\xa3\x5e\x93\xfb\xb7\xf4\xa6\xf7\xee\x40\x1d\x73\x90"
      "\x90\xd6\x69\x55\x97\x36\x64\x04\xe7\x8d\xeb\x85\x51\x8a\xad\xe4\x38\x92"
      "\xc6\x65\xce\x67\x7a\xb9\xfe\x64\xfb\xca\xb9\xb2\x05\xb0\xc5\xac\xbe\x6e"
      "\xc7\xa6\x1d\xcf\x3f\x53\xb6\x2c\xa3\xcf\xed\xeb\x03\xe9\x80\xe5\xb4\xbe"
      "\x68\xf7\x46\x5e\x4a\x4c\x6c\x63\x58\x9e\xf8\x55\xe8\xea\x03\x89\x60\xfb"
      "\x67\xfa\x02\x8c\x30\xcc\xaf\xa3\xae\x2a\x32\x60\x3a\x51\xdf\x11\xcc\x7e"
      "\xaf\xb0\x14\x70\x87\x11\xd0\x27\x4f\x9c\xa4\x0f\xd9\xcc\x0b\xc6\x84\x70"
      "\x70\x35\x48\xf0\x58\x6e\x17\xec\x13\x72\x36\x15\x29\x24\xdc\x47\x16\x2e"
      "\xc0\xa1\xb1\xdc\x36\x1a\x2c\xd5\xa8\x2e\x4b\xb5\x91\x53\xef\x77\x42\x3c"
      "\xeb\x48\x44\x7b\x70\x91\x3a\x13\xa6\x80\x92\xf4\xa5\xcf\x01\x09\x71\xb0"
      "\xf7\x43\xd1\xe3\xa8\x5a\xc8\x9c\xbc\x21\x45\x7a\x66\xdc\xd3\x81\x10\x98"
      "\x1e\x6f\x80\x0c\xeb\x07\xdb\x88\x69\xba\x1d\x44\x91\x80\x89\xe0\xa5\x6e"
      "\x8e\xf5\x91\x1e\x93\x60\x16\xa8\xee\x4f\xcb\xd8\xb9\xf7\x81\xb3\xeb\x47"
      "\x0e\x38\xe0\xf0\xe9\xeb\x44\xb9\xc9\xf0\xf4\x4c\x62\x8f\x09\xad\xbf\x3c"
      "\x27\x56\x14\xd9\xe9\xc4\x7e\xf3\xfb\xd7\xd3\x76\x4f\x71\x15\x5f\x62\xbb"
      "\xa4\xf4\xd7\x7e\xab\x01\xcc\x41\x1f\xda\x96\x49\x21\x25\xf8\x4b\x63\xd9"
      "\x5d\x65\xf6\x41\x66\xa8\x06\x0a\xc2\x99\x65\xd5\xbe\x16\x40\x0c\xa9\x17"
      "\xee\x42\x18\x84\xc9\xdb\xfc\x02\xc6\xbd\x62\x43\x37\x74\x21\xde\xe4\xb9"
      "\x0d\xce\xed\x62\x7b\x76\x2e\xf6\x06\x56\x83\xcb\xb7\x7f\xc7\xd6\xff\x3f"
      "\x78\x1e\xf5\x0a\xce\xc3\xd1\x67\x46\x03\x55\xd5\xd6\xda\x01\x81\x63\x1c"
      "\x6a\x4c\x15\x04\x26\xb6\xfe\x14\x4c\x80\xd2\x36\x3d\x9f\x40\xe4\x06\x52"
      "\xbe\x63\x98\xc5\x26\xff\x8c\x50\xe4\xd0\xf0\x4f\xad\x6f\x96\x25\x25\xd8"
      "\x09\x69\xdb\x4d\x87\x0e\xb8\x72\xa0\x99\xe5\x2d\x16\x0d\x39\x13\xd7\x24"
      "\x05\x18\x35\x6e\x5f\xf4\x1f\xfd\xdc\x84\xd5\x52\x73\xc2\x36\xb6\x85\x7b"
      "\x90\xc0\xa8\xcd\xa8\x35\x5a\x96\x79\xae\x0e\xdc\x87\x5b\xb8\xc7\x21\xa0"
      "\x2d\x63\xd3\xee\xc3\xf8\xaf\x16\xb2\xe3\x42\x4c\x83\x80\x5a\xe6\x13\x63"
      "\x08\x69\x74\x8d\xc9\x1a\xd4\x4b\xc0\x42\xdc\xb7\x39\x24\x0f\xe0\xcc\x22"
      "\xd6\x5b\x17\x39\xdf\xfa\xd0\x40\xe7\xa5\xbc\xbd\xd1\xbb\x47\xe0\xd9\x81"
      "\x33\x7f\xef\xf8\x0f\x4f\xf3\xa4\x19\x6c\xe1\x5e\x89\xb3\xa6\xb5\xb5\x2a"
      "\xba\x97\x78\xee\x5c\xe4\xe9\x55\xe0\x7a\x87\xf7\x7a\xed\xbf\x27\xcc\xee"
      "\xc7\x97\x8a\xa7\x90\xd0\x9f\xcf\x30\x0c\x6d\xf7\x80\x01\xac\x10\x73\xf4"
      "\xe8\xd2\x68\xd9\x55\xa6\x98\x03\xb8\xf9\xf1\xe9\xf2\x97\x01\x17\xd0\xc6"
      "\x4c\xdc\x1f\xad\x79\x0c\xef\x14\x20\x7e\xe9\x20\x05\x8b\x97\x6c\xc8\x57"
      "\x92\xdb\xa3\x3b\x44\x12\x38\x95\xc9\xa8\x99\x11\x02\x4c\x9d\x2b\xbe\xc9"
      "\x79\x5d\xd3\x73\xd5\xea\xb1\xc6\xb1\x12\x85\xb9\x94\xe5\xab\x3f\x2f\x99"
      "\xb7\xc8\x21\x51\xda\x01\x89\x78\x40\xab\xf9\x85\x13\x98\x9c\x0c\x07\xba"
      "\x9e\x04\xb6\xb9\x2d\x51\x77\x2c\xaf\x52\xfd\xa3\x3f\xb7\xf5\xca\x3e\xd6"
      "\x78\xdb\x05\xac\x39\xc8\x39\x8a\xf8\x38\x58\x55\x57\x4d\x25\x5d\x6c\x84"
      "\x60\x44\x87\x5d\xb6\x2e\xc3\x48\xdf\x23\x86\x3c\x80\x3d\x4c\x76\x4f\x66"
      "\x82\x34\x1e\x3e\xea\xdb\x2e\xe3\x87\x3a\xa7\x46\x2c\xbd\x8c\x9b\xcc\x40"
      "\xd8\xf2\xc7\x7c\x00\x78\xfd\x9c\xc4\xbb\xdd\x37\x53\xb3\x82\xcc\x71\xb2"
      "\x55\x2a\x4f\x1b\xdc\xaa\xa2\xf3\x3f\xb0\x93\xd7\x71\x9d\x58\xeb\x00\x45"
      "\xfa\xf9\xf8\x7e\x89\x62\xee\x70\x4e\xbf\xb8\x29\xe1\xa7\x05\x16\x36\xb7"
      "\x72\x13\x94\xda\xff\x82\xed\xb4\x1e\xf5\xae\x8b\x1d\xd8\xa2\x5a",
      1024);
  syscall(__NR_ioctl, r[0], 0x4b72ul, 0x20000080ul);
  res = syz_open_dev(0xc, 4, 1);
  if (res != -1)
    r[1] = res;
  *(uint16_t*)0x20000080 = 0;
  *(uint16_t*)0x20000082 = 0;
  *(uint16_t*)0x20000084 = 0;
  *(uint16_t*)0x20000086 = 0x20;
  *(uint16_t*)0x20000088 = 8;
  *(uint16_t*)0x2000008a = 1;
  syscall(__NR_ioctl, r[1], 0x560aul, 0x20000080ul);
  return 0;
}