// https://syzkaller.appspot.com/bug?id=2aabc827b8aea09353150377f96630a3ff18f6a3
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
  syscall(__NR_mmap, /*addr=*/0x3ffffffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x400000000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x400001000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  const char* reason;
  (void)reason;
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  res = syscall(__NR_socket, /*domain=*/2ul, /*type=*/1ul, /*proto=*/0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x400000000040 = 1;
  syscall(__NR_setsockopt, /*fd=*/r[0], /*level=*/1,
          /*optname=SO_ZEROCOPY*/ 0x3c, /*optval=*/0x400000000040ul,
          /*optlen=*/0xfff0ul);
  *(uint32_t*)0x4000000000c0 = 1;
  syscall(__NR_setsockopt, /*fd=*/r[0], /*level=*/6, /*optname=*/0x13,
          /*optval=*/0x4000000000c0ul, /*optlen=*/4ul);
  *(uint16_t*)0x400000000080 = 2;
  *(uint16_t*)0x400000000082 = htobe16(0);
  *(uint32_t*)0x400000000084 = htobe32(0x7f000001);
  syscall(__NR_connect, /*fd=*/r[0], /*addr=*/0x400000000080ul,
          /*addrlen=*/0x10ul);
  *(uint32_t*)0x4000000001c0 = -1;
  syscall(__NR_setsockopt, /*fd=*/r[0], /*level=*/6, /*optname=*/0x13,
          /*optval=*/0x4000000001c0ul, /*optlen=*/4ul);
  syscall(__NR_write, /*fd=*/r[0], /*data=*/0x4000000014c0ul, /*len=*/0x46bul);
  *(uint64_t*)0x400000000f40 = 0;
  *(uint32_t*)0x400000000f48 = 0;
  *(uint64_t*)0x400000000f50 = 0x400000000500;
  *(uint64_t*)0x400000000500 = 0x4000000006c0;
  memset((void*)0x4000000006c0, 237, 1);
  *(uint64_t*)0x400000000508 = 0xfffffeb1;
  *(uint64_t*)0x400000000510 = 0x400000000200;
  memset((void*)0x400000000200, 181, 1);
  *(uint64_t*)0x400000000518 = 1;
  *(uint64_t*)0x400000000520 = 0x400000000340;
  memset((void*)0x400000000340, 46, 1);
  *(uint64_t*)0x400000000528 = 1;
  *(uint64_t*)0x400000000530 = 0x400000000140;
  memset((void*)0x400000000140, 85, 1);
  *(uint64_t*)0x400000000538 = 1;
  *(uint64_t*)0x400000000540 = 0x400000000000;
  memset((void*)0x400000000000, 243, 1);
  *(uint64_t*)0x400000000548 = 1;
  *(uint64_t*)0x400000000f58 = 5;
  *(uint64_t*)0x400000000f60 = 0;
  *(uint64_t*)0x400000000f68 = 0;
  *(uint32_t*)0x400000000f70 = 0;
  *(uint32_t*)0x400000000f78 = 0;
  *(uint64_t*)0x400000000f80 = 0;
  *(uint32_t*)0x400000000f88 = 0;
  *(uint64_t*)0x400000000f90 = 0x400000000480;
  *(uint64_t*)0x400000000480 = 0x400000000580;
  memset((void*)0x400000000580, 241, 1);
  *(uint64_t*)0x400000000488 = 1;
  *(uint64_t*)0x400000000490 = 0x400000001080;
  memcpy(
      (void*)0x400000001080,
      "\x61\xbb\xfc\xab\x4e\x45\x7d\x38\xb4\xa7\x9b\xc6\x83\x74\x23\x15\x4f\xf6"
      "\xc4\x80\x3a\xef\xe3\x91\x19\xe9\xc3\x98\x0d\xb8\x41\xd3\x17\xa7\x18\x1b"
      "\xcd\x8e\xfa\x98\x82\x00\xfa\x94\x54\x43\x43\x3d\xa8\x4f\x05\xc2\x7e\x03"
      "\x09\xc2\x0e\x17\xbd\x53\x51\x80\x0c\x10\xdf\x61\xb8\xac\x8e\x78\x22\x20"
      "\xae\x98\x58\x03\xf2\x58\xf4\x7c\x43\xf5\x45\x32\x95\x5e\x5e\x66\xdd\x35"
      "\x08\x52\x1c\x9a\x3b\x1a\x40\x78\x3b\x5d\x81\xa8\x04\xb1\x20\x87\x64\x4e"
      "\x53\x3b\x6d\x15\xac\xf6\x24\x9c\x96\x3b\xca\x40\x3c\xfd\x3a\x79\xe0\x09"
      "\xc5\x1d\xe7\x0c\x4d\xc9\x39\x19\xa8\x32\xd7\xf1\x6b\xaf\x02\x97\x09\xc8"
      "\x83\x9d\x75\x3d\x64\xfd\x85\x7f\x1f\x61\x30\x32\xfd\x8c\x2c\x9f\xff\xad"
      "\x91\xa9\x99\xc4\xe0\x1e\x0b\x9b\xf8\x91\x60\x92\x9d\x05\xf8\xcb\x1a\x2c"
      "\xb0\xfd\x91\x49\xef\x07\x6b\x70\x4b\x14\x46\xbe\xd1\xc1\x63\x52\xe3\x1b"
      "\xc6\x8b\x7b\xc6\x8f\xd9\x0e\x9b\xf8\x54\x25\xfe\x7f\x41\x2e\x66\x4b\x25"
      "\x0a\x89\xeb\xd2\xb9\x2d\xa8\xd0\xfb\xe5\x4a\xab\x9d\x00\x35\x9f\x8a\xde"
      "\x07\xa4\x73\xd0\x27\x6c\x2c\x6c\xf9\xc0\xef\xb5\xf8\xf0\xd9\x7c\x65\x49"
      "\x12\xb2\x3a\x3f\xc2\xed\x7c\xa6\x2c\xcc\xdc\x0e\xfa\x91\x3f\xec\x68\xe2"
      "\xe1\xea\x8d\xf6\x80\x2b\xf8\xf5\x5d\x29\x92\x18\x79\x6e\xd2\x9c\x49\x71"
      "\xc7\x28\xfe\x17\xa4\x93\xe1\xce\x75\xfe\x13\x7e\x8b\xb5\x7f\xb3\x27\x2c"
      "\xbd\x09\x4b\x6d\xe2\xba\x06\x67\x6a\x56\xaa\x02\x05\xf1\x42\xc1\x89\xcb"
      "\xb5\x02\x71\x5a\xaa\x4d\x05\x91\x53\xe8\xe8\xfd\xbc\x5a\x8b\xb5\x3e\x0d"
      "\xf6\x96\x32\xda\xbb\xfe\xde\x7e\xf4\x75\x03\x9b\x14\x33\xed\xb0\xac\xd7"
      "\xc2\x08\x32\x73\x1f\xff\x1e\x9c\xe3\x88\xce\x52\x40\xe6\xf8\xe5\xde\x17"
      "\xd0\x21\xc9\xcc\xc6\x34\x41\x0e\xe5\x01\xab\xac\x97\xf7\xeb\xd8\x09\xa8"
      "\x4f\x21\x9d\xf7\xff\x3d\x57\xcb\x65\x2d\x6c\x8c\x3b\x6a\x29\xe0\x11\x21"
      "\xa7\xed\xa0\x36\x28\x91\xdb\xf7\x90\x61\xe2\xf6\x5d\xe5\x6b\x7c\x35\x98"
      "\xd3\xd6\x73\x83\xcf\xf0\x18\xd3\x46\x63\xf8\xb0\x95\x51\x65\xcc\x7b\x49"
      "\xa3\xe9\xe9\x1d\xec\xb9\x38\x8d\x7f\xd8\x56\x88\x41\xd9\xad\x2f\xb9\x49"
      "\x1f\x7a\x04\x7b\x56\x9f\x63\xca\xd3\x39\xaa\x9b\xc3\x95\x9a\xbd\xee\x4c"
      "\x70\x81\x07\xb5\x1f\xbd\x86\x75\x20\x31\xfb\x5b\x3a\xed\x07\xee\x34\x56"
      "\x28\x66\x1a\x18\x30\x4f\x2a",
      511);
  *(uint64_t*)0x400000000498 = 0x1ff;
  *(uint64_t*)0x4000000004a0 = 0x400000000b40;
  memset((void*)0x400000000b40, 77, 1);
  *(uint64_t*)0x4000000004a8 = 1;
  *(uint64_t*)0x4000000004b0 = 0x4000000005c0;
  memcpy((void*)0x4000000005c0,
         "\x6f\x32\x52\x7f\xf5\x49\x72\x2b\x3a\x37\xd2\x0a\xca\x2d\xd9\xfe\x3b"
         "\x08\x4e\xd5\x0e\x5d\x1d\xc2\x91\xa4\x4b\x97\x3f\x5b\x71\xc4\x37\xce"
         "\x94\xf8\x74\x88\xac\x8d\x45\x60\x83\x26\x39\x8a\xf1\x7d\xa2\x6e\x4c"
         "\x7b\x3c\xcc\x07\xc4\xdd\x40\x8a\x3b\xff\xfc\x10\x70\x3b\xb9\x45\xdb"
         "\xb0\x13\x70\xa5\x79\x45\x78\xea\x26\xb0\x63\x29\xb2\x31\xe6\xc6\xa6"
         "\x9a\x4c\x5c\xe2\x97\x2b\x54\x25\x5c\xe6\xf9\x58\x01\xc5\x74\xd9\xaf"
         "\x52\x21\xc4\x61\x1a\x78\x75\x05\x9d\xc3\x73\xe4\xc8\xcf\xdf\x88\xe2"
         "\xad\x1b\xb1\xb8\x5e\x9b\xd5\x13\x0d\xd5\x86\x49\x55\xbb\x50\x6c\x0a"
         "\x92\x50\x18\x3b\x94\xac\x85\xf3\x2a\x34\xfe\xc8\x31\x32",
         150);
  *(uint64_t*)0x4000000004b8 = 0x96;
  *(uint64_t*)0x4000000004c0 = 0x400000000180;
  memset((void*)0x400000000180, 8, 1);
  *(uint64_t*)0x4000000004c8 = 1;
  *(uint64_t*)0x400000000f98 = 5;
  *(uint64_t*)0x400000000fa0 = 0;
  *(uint64_t*)0x400000000fa8 = 0;
  *(uint32_t*)0x400000000fb0 = 0;
  *(uint32_t*)0x400000000fb8 = 0x70040000;
  *(uint64_t*)0x400000000fc0 = 0;
  *(uint32_t*)0x400000000fc8 = 0;
  *(uint64_t*)0x400000000fd0 = 0x4000000002c0;
  *(uint64_t*)0x4000000002c0 = 0x400000000380;
  memset((void*)0x400000000380, 16, 1);
  *(uint64_t*)0x4000000002c8 = 1;
  *(uint64_t*)0x4000000002d0 = 0x4000000007c0;
  memset((void*)0x4000000007c0, 161, 1);
  *(uint64_t*)0x4000000002d8 = 1;
  *(uint64_t*)0x4000000002e0 = 0x400000000800;
  memset((void*)0x400000000800, 115, 1);
  *(uint64_t*)0x4000000002e8 = 1;
  *(uint64_t*)0x4000000002f0 = 0x4000000009c0;
  memset((void*)0x4000000009c0, 92, 1);
  *(uint64_t*)0x4000000002f8 = 1;
  *(uint64_t*)0x400000000fd8 = 4;
  *(uint64_t*)0x400000000fe0 = 0;
  *(uint64_t*)0x400000000fe8 = 0;
  *(uint32_t*)0x400000000ff0 = 0;
  *(uint32_t*)0x400000000ff8 = 0;
  *(uint64_t*)0x400000001000 = 0;
  *(uint32_t*)0x400000001008 = 0x33;
  *(uint64_t*)0x400000001010 = 0x400000000dc0;
  *(uint64_t*)0x400000001018 = 3;
  *(uint64_t*)0x400000001020 = 0;
  *(uint64_t*)0x400000001028 = 0;
  *(uint32_t*)0x400000001030 = 0;
  *(uint32_t*)0x400000001038 = 0;
  syscall(__NR_sendmmsg, /*fd=*/r[0], /*mmsg=*/0x400000000f40ul, /*vlen=*/4ul,
          /*f=MSG_ZEROCOPY|MSG_BATCH|MSG_OOB|MSG_MORE|MSG_DONTWAIT|MSG_CONFIRM*/
          0x4048841ul);
  return 0;
}
