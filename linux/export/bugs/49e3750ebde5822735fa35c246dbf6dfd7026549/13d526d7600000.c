// https://syzkaller.appspot.com/bug?id=49e3750ebde5822735fa35c246dbf6dfd7026549
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
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10, 0x803, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000000, "\x26\x00\x00\x00\x22\x00\x47\x01\x05\x00\x07\x00"
                            "\x89\x80\xe8\xff\x06\x00\x6d\x20\x00\x2b\x1f\x00"
                            "\xc0\xe9\xff\x09\x4a\x51\xf1\x01\x01\xc7\x03\x35"
                            "\x00\xb0",
         38);
  syscall(__NR_write, r[0], 0x20000000, 0x26);
  *(uint32_t*)0x20d9bffc = 0;
  syscall(__NR_setsockopt, r[0], 1, 8, 0x20d9bffc, 4);
  *(uint64_t*)0x20001480 = 0;
  *(uint32_t*)0x20001488 = 0;
  *(uint64_t*)0x20001490 = 0x20001440;
  *(uint64_t*)0x20001440 = 0x200000c0;
  *(uint32_t*)0x200000c0 = 0x184;
  *(uint16_t*)0x200000c4 = 0x30;
  *(uint16_t*)0x200000c6 = 0x10dd;
  *(uint32_t*)0x200000c8 = 0x70bd2c;
  *(uint32_t*)0x200000cc = 0x25dfdbfb;
  *(uint8_t*)0x200000d0 = 0;
  *(uint8_t*)0x200000d1 = 0;
  *(uint16_t*)0x200000d2 = 0;
  *(uint16_t*)0x200000d4 = 0x7c;
  *(uint16_t*)0x200000d6 = 1;
  *(uint16_t*)0x200000d8 = 0x78;
  *(uint32_t*)0x200000da = 0x1b;
  *(uint16_t*)0x200000de = 0x10;
  *(uint16_t*)0x200000e0 = 1;
  memcpy((void*)0x200000e2, "tunnel_key\000", 11);
  *(uint16_t*)0x200000ee = 0x5c;
  *(uint16_t*)0x200000f0 = 2;
  *(uint16_t*)0x200000f2 = 0x14;
  *(uint16_t*)0x200000f4 = 5;
  *(uint8_t*)0x200000f6 = 0xfe;
  *(uint8_t*)0x200000f7 = 0x88;
  *(uint8_t*)0x200000f8 = 0;
  *(uint8_t*)0x200000f9 = 0;
  *(uint8_t*)0x200000fa = 0;
  *(uint8_t*)0x200000fb = 0;
  *(uint8_t*)0x200000fc = 0;
  *(uint8_t*)0x200000fd = 0;
  *(uint8_t*)0x200000fe = 0;
  *(uint8_t*)0x200000ff = 0;
  *(uint8_t*)0x20000100 = 0;
  *(uint8_t*)0x20000101 = 0;
  *(uint8_t*)0x20000102 = 0;
  *(uint8_t*)0x20000103 = 0;
  *(uint8_t*)0x20000104 = 0;
  *(uint8_t*)0x20000105 = 1;
  *(uint16_t*)0x20000106 = 8;
  *(uint16_t*)0x20000108 = 7;
  *(uint32_t*)0x2000010a = 0x3a;
  *(uint16_t*)0x2000010e = 8;
  *(uint16_t*)0x20000110 = 3;
  *(uint8_t*)0x20000112 = 0xac;
  *(uint8_t*)0x20000113 = 0x14;
  *(uint8_t*)0x20000114 = 0x14;
  *(uint8_t*)0x20000115 = 0xbb;
  *(uint16_t*)0x20000116 = 8;
  *(uint16_t*)0x20000118 = 4;
  *(uint32_t*)0x2000011a = htobe32(0);
  *(uint16_t*)0x2000011e = 8;
  *(uint16_t*)0x20000120 = 0xa;
  *(uint8_t*)0x20000122 = 0;
  *(uint16_t*)0x20000126 = 8;
  *(uint16_t*)0x20000128 = 4;
  *(uint8_t*)0x2000012a = 0xac;
  *(uint8_t*)0x2000012b = 0x14;
  *(uint8_t*)0x2000012c = 0x14;
  *(uint8_t*)0x2000012d = 0xbb;
  *(uint16_t*)0x2000012e = 0x14;
  *(uint16_t*)0x20000130 = 6;
  memcpy((void*)0x20000132,
         "\x06\x1b\x6a\x2d\x24\x36\x8b\xb1\x64\x35\xe8\x9f\x9f\x33\xcb\xfa",
         16);
  *(uint16_t*)0x20000142 = 8;
  *(uint16_t*)0x20000144 = 3;
  *(uint8_t*)0x20000146 = 0xac;
  *(uint8_t*)0x20000147 = 0x14;
  *(uint8_t*)0x20000148 = 0x14;
  *(uint8_t*)0x20000149 = 0xaa;
  *(uint16_t*)0x2000014a = 4;
  *(uint16_t*)0x2000014c = 6;
  *(uint16_t*)0x20000150 = 0x48;
  *(uint16_t*)0x20000152 = 1;
  *(uint16_t*)0x20000154 = 0x44;
  *(uint32_t*)0x20000156 = 0x15;
  *(uint16_t*)0x2000015a = 0xc;
  *(uint16_t*)0x2000015c = 1;
  memcpy((void*)0x2000015e, "simple\000", 7);
  *(uint16_t*)0x20000166 = 0x2c;
  *(uint16_t*)0x20000168 = 2;
  *(uint16_t*)0x2000016a = 0x1c;
  *(uint16_t*)0x2000016c = 3;
  memcpy((void*)0x2000016e, "vboxnet0eth1(eth1GPL\000", 21);
  *(uint16_t*)0x20000186 = 0xc;
  *(uint16_t*)0x20000188 = 3;
  memcpy((void*)0x2000018a, ")ppp0\000", 6);
  *(uint16_t*)0x20000192 = 4;
  *(uint16_t*)0x20000194 = 6;
  *(uint16_t*)0x20000198 = 0x78;
  *(uint16_t*)0x2000019a = 1;
  *(uint16_t*)0x2000019c = 0x74;
  *(uint32_t*)0x2000019e = 8;
  *(uint16_t*)0x200001a2 = 0xc;
  *(uint16_t*)0x200001a4 = 1;
  memcpy((void*)0x200001a6, "simple\000", 7);
  *(uint16_t*)0x200001ae = 0x5c;
  *(uint16_t*)0x200001b0 = 2;
  *(uint16_t*)0x200001b2 = 8;
  *(uint16_t*)0x200001b4 = 3;
  memcpy((void*)0x200001b6, "\000", 1);
  *(uint16_t*)0x200001ba = 0xc;
  *(uint16_t*)0x200001bc = 3;
  memcpy((void*)0x200001be, "#@\276&\000", 5);
  *(uint16_t*)0x200001c6 = 0x18;
  *(uint16_t*)0x200001c8 = 3;
  memcpy((void*)0x200001ca, "bdevloem0trusted\000", 17);
  *(uint16_t*)0x200001de = 0x18;
  *(uint16_t*)0x200001e0 = 2;
  *(uint32_t*)0x200001e2 = 5;
  *(uint32_t*)0x200001e6 = 3;
  *(uint32_t*)0x200001ea = 5;
  *(uint32_t*)0x200001ee = 7;
  *(uint32_t*)0x200001f2 = 0x401;
  *(uint16_t*)0x200001f6 = 0x14;
  *(uint16_t*)0x200001f8 = 3;
  memcpy((void*)0x200001fa, "+system/\'system\000", 16);
  *(uint16_t*)0x2000020a = 4;
  *(uint16_t*)0x2000020c = 6;
  *(uint16_t*)0x20000210 = 0x34;
  *(uint16_t*)0x20000212 = 1;
  *(uint16_t*)0x20000214 = 0x30;
  *(uint32_t*)0x20000216 = 2;
  *(uint16_t*)0x2000021a = 8;
  *(uint16_t*)0x2000021c = 1;
  memcpy((void*)0x2000021e, "ipt\000", 4);
  *(uint16_t*)0x20000222 = 0x1c;
  *(uint16_t*)0x20000224 = 2;
  *(uint16_t*)0x20000226 = 8;
  *(uint16_t*)0x20000228 = 3;
  *(uint32_t*)0x2000022a = 0x100;
  *(uint16_t*)0x2000022e = 8;
  *(uint16_t*)0x20000230 = 2;
  *(uint32_t*)0x20000232 = 4;
  *(uint16_t*)0x20000236 = 8;
  *(uint16_t*)0x20000238 = 2;
  *(uint32_t*)0x2000023a = 3;
  *(uint16_t*)0x2000023e = 4;
  *(uint16_t*)0x20000240 = 6;
  *(uint64_t*)0x20001448 = 0x184;
  *(uint64_t*)0x20001498 = 1;
  *(uint64_t*)0x200014a0 = 0;
  *(uint64_t*)0x200014a8 = 0;
  *(uint32_t*)0x200014b0 = 0x10;
  syscall(__NR_sendmsg, r[0], 0x20001480, 0x20000010);
  return 0;
}
