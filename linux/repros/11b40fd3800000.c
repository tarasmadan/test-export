// https://syzkaller.appspot.com/bug?id=93f3b217d56a3e0e5ae1e606733c5aba66ccf628
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#ifndef __NR_setsockopt
#define __NR_setsockopt 366
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 2, 1, 0);
  memcpy((void*)0x20159fb0, "\x62\x72\x6f\x75\x74\x65\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20159fd0 = 0x20;
  *(uint32_t*)0x20159fd4 = 3;
  *(uint32_t*)0x20159fd8 = 0x3c8;
  *(uint32_t*)0x20159fdc = 0;
  *(uint32_t*)0x20159fe0 = 0;
  *(uint32_t*)0x20159fe4 = 0;
  *(uint32_t*)0x20159fe8 = 0;
  *(uint32_t*)0x20159fec = 0;
  *(uint32_t*)0x20159ff0 = 0x20d19000;
  *(uint32_t*)0x20159ff4 = 0;
  *(uint32_t*)0x20159ff8 = 0x20438ff0;
  *(uint32_t*)0x20159ffc = 0x20d19000;
  *(uint32_t*)0x20d19000 = 0;
  memcpy((void*)0x20d19004, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20d19024 = 0;
  *(uint32_t*)0x20d19028 = 0;
  *(uint32_t*)0x20d1902c = 0;
  *(uint32_t*)0x20d19030 = 0;
  memcpy((void*)0x20d19034, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20d19054 = 0;
  *(uint32_t*)0x20d19058 = 0;
  *(uint32_t*)0x20d1905c = 2;
  *(uint32_t*)0x20d19060 = 0x11;
  *(uint32_t*)0x20d19064 = 0;
  *(uint16_t*)0x20d19068 = htobe16(0);
  *(uint8_t*)0x20d1906a = 0x73;
  *(uint8_t*)0x20d1906b = 0x79;
  *(uint8_t*)0x20d1906c = 0x7a;
  *(uint8_t*)0x20d1906d = 0;
  *(uint8_t*)0x20d1906e = 0;
  memcpy((void*)0x20d1907a,
         "\xf1\x2f\xa9\xab\xc8\xe6\xee\xf8\x06\x1d\xfc\x8c\x3e\x7f\xc8\x8a",
         16);
  memcpy((void*)0x20d1908a,
         "\x72\x2c\xad\x42\x29\x86\x9e\x7b\x8c\xdf\xb7\x76\xea\x88\x41\xae",
         16);
  memcpy((void*)0x20d1909a,
         "\x24\x85\x97\x91\x43\x30\x1c\x55\xaf\x7b\x31\x52\x49\xf7\xe5\x56",
         16);
  *(uint8_t*)0x20d190aa = 0xaa;
  *(uint8_t*)0x20d190ab = 0xaa;
  *(uint8_t*)0x20d190ac = 0xaa;
  *(uint8_t*)0x20d190ad = 0xaa;
  *(uint8_t*)0x20d190ae = 0;
  *(uint8_t*)0x20d190af = 0xaa;
  *(uint8_t*)0x20d190b0 = 0;
  *(uint8_t*)0x20d190b1 = 0;
  *(uint8_t*)0x20d190b2 = 0;
  *(uint8_t*)0x20d190b3 = 0;
  *(uint8_t*)0x20d190b4 = 0;
  *(uint8_t*)0x20d190b5 = 0;
  *(uint8_t*)0x20d190b6 = 0xaa;
  *(uint8_t*)0x20d190b7 = 0xaa;
  *(uint8_t*)0x20d190b8 = 0xaa;
  *(uint8_t*)0x20d190b9 = 0xaa;
  *(uint8_t*)0x20d190ba = 0;
  *(uint8_t*)0x20d190bb = 0xbb;
  *(uint8_t*)0x20d190bc = 0;
  *(uint8_t*)0x20d190bd = 0;
  *(uint8_t*)0x20d190be = 0;
  *(uint8_t*)0x20d190bf = 0;
  *(uint8_t*)0x20d190c0 = 0;
  *(uint8_t*)0x20d190c1 = 0;
  *(uint32_t*)0x20d190c4 = 0xf4;
  *(uint32_t*)0x20d190c8 = 0x124;
  *(uint32_t*)0x20d190cc = 0x14c;
  memcpy((void*)0x20d190d0, "\x68\x65\x6c\x70\x65\x72\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20d190f0 = 0x24;
  *(uint32_t*)0x20d190f4 = 0;
  memcpy((void*)0x20d190f8, "\x52\x41\x53\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00",
         30);
  memcpy((void*)0x20d19118, "\x71\x75\x6f\x74\x61\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20d19138 = 0x18;
  *(uint32_t*)0x20d1913c = 0;
  *(uint32_t*)0x20d19140 = 0;
  *(uint64_t*)0x20d19144 = 0;
  *(uint32_t*)0x20d1914c = 0;
  memcpy((void*)0x20d19154, "\x64\x6e\x61\x74\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20d19174 = 0xc;
  memcpy((void*)0x20d19178, "\x0f\x00\x54\x42\xa7\xec", 6);
  *(uint32_t*)0x20d19180 = 0;
  memcpy((void*)0x20d19184, "\x72\x65\x64\x69\x72\x65\x63\x74\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20d191a4 = 4;
  *(uint32_t*)0x20d191a8 = 0;
  *(uint32_t*)0x20d191ac = 0xb;
  *(uint32_t*)0x20d191b0 = 0;
  *(uint16_t*)0x20d191b4 = htobe16(0);
  memcpy((void*)0x20d191b6,
         "\xa5\x15\xc9\x54\xdf\xa7\xd2\xa8\x27\xf3\x94\x93\xcd\xe1\x00\xae",
         16);
  memcpy((void*)0x20d191c6,
         "\xb7\xa8\xf4\x6b\x5e\xe6\x5e\x16\x31\x44\x51\x83\xb3\x8c\xb0\x10",
         16);
  *(uint8_t*)0x20d191d6 = 0x73;
  *(uint8_t*)0x20d191d7 = 0x79;
  *(uint8_t*)0x20d191d8 = 0x7a;
  *(uint8_t*)0x20d191d9 = 0;
  *(uint8_t*)0x20d191da = 0;
  memcpy((void*)0x20d191e6,
         "\xec\x34\xcc\xfa\x2f\x80\x7b\x9d\x2e\xcd\xf6\x2a\xfe\xf9\x98\x54",
         16);
  *(uint8_t*)0x20d191f6 = 0xaa;
  *(uint8_t*)0x20d191f7 = 0xaa;
  *(uint8_t*)0x20d191f8 = 0xaa;
  *(uint8_t*)0x20d191f9 = 0xaa;
  *(uint8_t*)0x20d191fa = 0;
  *(uint8_t*)0x20d191fb = 0;
  *(uint8_t*)0x20d191fc = 0;
  *(uint8_t*)0x20d191fd = 0;
  *(uint8_t*)0x20d191fe = 0;
  *(uint8_t*)0x20d191ff = 0;
  *(uint8_t*)0x20d19200 = 0;
  *(uint8_t*)0x20d19201 = 0;
  *(uint8_t*)0x20d19202 = -1;
  *(uint8_t*)0x20d19203 = -1;
  *(uint8_t*)0x20d19204 = -1;
  *(uint8_t*)0x20d19205 = -1;
  *(uint8_t*)0x20d19206 = -1;
  *(uint8_t*)0x20d19207 = -1;
  *(uint8_t*)0x20d19208 = 0;
  *(uint8_t*)0x20d19209 = 0;
  *(uint8_t*)0x20d1920a = 0;
  *(uint8_t*)0x20d1920b = 0;
  *(uint8_t*)0x20d1920c = 0;
  *(uint8_t*)0x20d1920d = 0;
  *(uint32_t*)0x20d19210 = 0xc8;
  *(uint32_t*)0x20d19214 = 0xf0;
  *(uint32_t*)0x20d19218 = 0xffffff90;
  memcpy((void*)0x20d1921c, "\x73\x74\x61\x74\x65\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20d1923c = 4;
  *(uint32_t*)0x20d19240 = 0;
  memcpy((void*)0x20d19244, "\x6d\x61\x63\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20d19264 = 0xc;
  *(uint8_t*)0x20d19268 = 0xaa;
  *(uint8_t*)0x20d19269 = 0xaa;
  *(uint8_t*)0x20d1926a = 0xaa;
  *(uint8_t*)0x20d1926b = 0xaa;
  *(uint8_t*)0x20d1926c = 0;
  *(uint8_t*)0x20d1926d = 0xaa;
  *(uint32_t*)0x20d19270 = 0;
  memcpy((void*)0x20d19274, "\x43\x4c\x41\x53\x53\x49\x46\x59\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20d19294 = 4;
  *(uint32_t*)0x20d19298 = 0;
  memcpy((void*)0x20d1929c, "\x43\x4c\x41\x53\x53\x49\x46\x59\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20d192bc = 4;
  *(uint32_t*)0x20d192c0 = 0;
  *(uint32_t*)0x20d192c4 = 0;
  memcpy((void*)0x20d192c8, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20d192e8 = 2;
  *(uint32_t*)0x20d192ec = 0;
  *(uint32_t*)0x20d192f0 = 1;
  *(uint32_t*)0x20d192f4 = 0x11;
  *(uint32_t*)0x20d192f8 = 0;
  *(uint16_t*)0x20d192fc = htobe16(0);
  *(uint8_t*)0x20d192fe = 0x73;
  *(uint8_t*)0x20d192ff = 0x79;
  *(uint8_t*)0x20d19300 = 0x7a;
  *(uint8_t*)0x20d19301 = 0;
  *(uint8_t*)0x20d19302 = 0;
  *(uint8_t*)0x20d1930e = 0x73;
  *(uint8_t*)0x20d1930f = 0x79;
  *(uint8_t*)0x20d19310 = 0x7a;
  *(uint8_t*)0x20d19311 = 0;
  *(uint8_t*)0x20d19312 = 0;
  memcpy((void*)0x20d1931e,
         "\x67\x72\x65\x74\x61\x70\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  memcpy((void*)0x20d1932e,
         "\x2d\x4d\xeb\x7a\x3a\xee\x1a\xbd\x1e\xa3\xf6\x55\xce\x04\xf8\x7b",
         16);
  memcpy((void*)0x20d1933e, "\x22\xbe\x45\x69\x97\x11", 6);
  *(uint8_t*)0x20d19344 = 0;
  *(uint8_t*)0x20d19345 = 0;
  *(uint8_t*)0x20d19346 = 0;
  *(uint8_t*)0x20d19347 = 0;
  *(uint8_t*)0x20d19348 = 0;
  *(uint8_t*)0x20d19349 = 0;
  *(uint8_t*)0x20d1934a = 0xaa;
  *(uint8_t*)0x20d1934b = 0xaa;
  *(uint8_t*)0x20d1934c = 0xaa;
  *(uint8_t*)0x20d1934d = 0xaa;
  *(uint8_t*)0x20d1934e = 0;
  *(uint8_t*)0x20d1934f = 0xbb;
  *(uint8_t*)0x20d19350 = 0;
  *(uint8_t*)0x20d19351 = 0;
  *(uint8_t*)0x20d19352 = 0;
  *(uint8_t*)0x20d19353 = 0;
  *(uint8_t*)0x20d19354 = 0;
  *(uint8_t*)0x20d19355 = 0;
  *(uint32_t*)0x20d19358 = 0x70;
  *(uint32_t*)0x20d1935c = 0xac;
  *(uint32_t*)0x20d19360 = 0xd4;
  memcpy((void*)0x20d19364, "\x52\x41\x54\x45\x45\x53\x54\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20d19384 = 0x18;
  memcpy((void*)0x20d19388,
         "\x73\x79\x7a\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint8_t*)0x20d19398 = 0;
  *(uint8_t*)0x20d19399 = 0;
  *(uint32_t*)0x20d1939c = 0;
  memcpy((void*)0x20d193a0, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20d193c0 = 4;
  *(uint32_t*)0x20d193c4 = 0;
  syscall(__NR_setsockopt, r[0], 0, 0x80, 0x20159fb0, 0x418);
}

int main()
{
  loop();
  return 0;
}