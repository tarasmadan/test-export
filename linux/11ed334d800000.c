// https://syzkaller.appspot.com/bug?id=5e694bb8245458c02eea43ac605cb5b7c6c9de11
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0xa, 2, 0);
  memcpy((void*)0x20f2b000, "\x6e\x61\x74\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20f2b020 = 0x1b;
  *(uint32_t*)0x20f2b024 = 5;
  *(uint32_t*)0x20f2b028 = 0x628;
  *(uint32_t*)0x20f2b02c = 0x300;
  *(uint32_t*)0x20f2b030 = 0x440;
  *(uint32_t*)0x20f2b034 = -1;
  *(uint32_t*)0x20f2b038 = 0x300;
  *(uint32_t*)0x20f2b03c = 0;
  *(uint32_t*)0x20f2b040 = 0x558;
  *(uint32_t*)0x20f2b044 = 0x558;
  *(uint32_t*)0x20f2b048 = -1;
  *(uint32_t*)0x20f2b04c = 0x558;
  *(uint32_t*)0x20f2b050 = 0x558;
  *(uint32_t*)0x20f2b054 = 5;
  *(uint64_t*)0x20f2b058 = 0x20368fb0;
  *(uint8_t*)0x20f2b060 = 0;
  *(uint8_t*)0x20f2b061 = 0;
  *(uint8_t*)0x20f2b062 = 0;
  *(uint8_t*)0x20f2b063 = 0;
  *(uint8_t*)0x20f2b064 = 0;
  *(uint8_t*)0x20f2b065 = 0;
  *(uint8_t*)0x20f2b066 = 0;
  *(uint8_t*)0x20f2b067 = 0;
  *(uint8_t*)0x20f2b068 = 0;
  *(uint8_t*)0x20f2b069 = 0;
  *(uint8_t*)0x20f2b06a = 0;
  *(uint8_t*)0x20f2b06b = 0;
  *(uint8_t*)0x20f2b06c = 0;
  *(uint8_t*)0x20f2b06d = 0;
  *(uint8_t*)0x20f2b06e = 0;
  *(uint8_t*)0x20f2b06f = 0;
  *(uint8_t*)0x20f2b070 = 0;
  *(uint8_t*)0x20f2b071 = 0;
  *(uint8_t*)0x20f2b072 = 0;
  *(uint8_t*)0x20f2b073 = 0;
  *(uint8_t*)0x20f2b074 = 0;
  *(uint8_t*)0x20f2b075 = 0;
  *(uint8_t*)0x20f2b076 = 0;
  *(uint8_t*)0x20f2b077 = 0;
  *(uint8_t*)0x20f2b078 = 0;
  *(uint8_t*)0x20f2b079 = 0;
  *(uint8_t*)0x20f2b07a = 0;
  *(uint8_t*)0x20f2b07b = 0;
  *(uint8_t*)0x20f2b07c = 0;
  *(uint8_t*)0x20f2b07d = 0;
  *(uint8_t*)0x20f2b07e = 0;
  *(uint8_t*)0x20f2b07f = 0;
  *(uint8_t*)0x20f2b080 = 0;
  *(uint8_t*)0x20f2b081 = 0;
  *(uint8_t*)0x20f2b082 = 0;
  *(uint8_t*)0x20f2b083 = 0;
  *(uint8_t*)0x20f2b084 = 0;
  *(uint8_t*)0x20f2b085 = 0;
  *(uint8_t*)0x20f2b086 = 0;
  *(uint8_t*)0x20f2b087 = 0;
  *(uint8_t*)0x20f2b088 = 0;
  *(uint8_t*)0x20f2b089 = 0;
  *(uint8_t*)0x20f2b08a = 0;
  *(uint8_t*)0x20f2b08b = 0;
  *(uint8_t*)0x20f2b08c = 0;
  *(uint8_t*)0x20f2b08d = 0;
  *(uint8_t*)0x20f2b08e = 0;
  *(uint8_t*)0x20f2b08f = 0;
  *(uint8_t*)0x20f2b090 = 0;
  *(uint8_t*)0x20f2b091 = 0;
  *(uint8_t*)0x20f2b092 = 0;
  *(uint8_t*)0x20f2b093 = 0;
  *(uint8_t*)0x20f2b094 = 0;
  *(uint8_t*)0x20f2b095 = 0;
  *(uint8_t*)0x20f2b096 = 0;
  *(uint8_t*)0x20f2b097 = 0;
  *(uint8_t*)0x20f2b098 = 0;
  *(uint8_t*)0x20f2b099 = 0;
  *(uint8_t*)0x20f2b09a = 0;
  *(uint8_t*)0x20f2b09b = 0;
  *(uint8_t*)0x20f2b09c = 0;
  *(uint8_t*)0x20f2b09d = 0;
  *(uint8_t*)0x20f2b09e = 0;
  *(uint8_t*)0x20f2b09f = 0;
  *(uint8_t*)0x20f2b0a0 = 0;
  *(uint8_t*)0x20f2b0a1 = 0;
  *(uint8_t*)0x20f2b0a2 = 0;
  *(uint8_t*)0x20f2b0a3 = 0;
  *(uint8_t*)0x20f2b0a4 = 0;
  *(uint8_t*)0x20f2b0a5 = 0;
  *(uint8_t*)0x20f2b0a6 = 0;
  *(uint8_t*)0x20f2b0a7 = 0;
  *(uint8_t*)0x20f2b0a8 = 0;
  *(uint8_t*)0x20f2b0a9 = 0;
  *(uint8_t*)0x20f2b0aa = 0;
  *(uint8_t*)0x20f2b0ab = 0;
  *(uint8_t*)0x20f2b0ac = 0;
  *(uint8_t*)0x20f2b0ad = 0;
  *(uint8_t*)0x20f2b0ae = 0;
  *(uint8_t*)0x20f2b0af = 0;
  *(uint8_t*)0x20f2b0b0 = 0;
  *(uint8_t*)0x20f2b0b1 = 0;
  *(uint8_t*)0x20f2b0b2 = 0;
  *(uint8_t*)0x20f2b0b3 = 0;
  *(uint8_t*)0x20f2b0b4 = 0;
  *(uint8_t*)0x20f2b0b5 = 0;
  *(uint8_t*)0x20f2b0b6 = 0;
  *(uint8_t*)0x20f2b0b7 = 0;
  *(uint8_t*)0x20f2b0b8 = 0;
  *(uint8_t*)0x20f2b0b9 = 0;
  *(uint8_t*)0x20f2b0ba = 0;
  *(uint8_t*)0x20f2b0bb = 0;
  *(uint8_t*)0x20f2b0bc = 0;
  *(uint8_t*)0x20f2b0bd = 0;
  *(uint8_t*)0x20f2b0be = 0;
  *(uint8_t*)0x20f2b0bf = 0;
  *(uint8_t*)0x20f2b0c0 = 0;
  *(uint8_t*)0x20f2b0c1 = 0;
  *(uint8_t*)0x20f2b0c2 = 0;
  *(uint8_t*)0x20f2b0c3 = 0;
  *(uint8_t*)0x20f2b0c4 = 0;
  *(uint8_t*)0x20f2b0c5 = 0;
  *(uint8_t*)0x20f2b0c6 = 0;
  *(uint8_t*)0x20f2b0c7 = 0;
  *(uint8_t*)0x20f2b0c8 = 0;
  *(uint8_t*)0x20f2b0c9 = 0;
  *(uint8_t*)0x20f2b0ca = 0;
  *(uint8_t*)0x20f2b0cb = 0;
  *(uint8_t*)0x20f2b0cc = 0;
  *(uint8_t*)0x20f2b0cd = 0;
  *(uint8_t*)0x20f2b0ce = 0;
  *(uint8_t*)0x20f2b0cf = 0;
  *(uint8_t*)0x20f2b0d0 = 0;
  *(uint8_t*)0x20f2b0d1 = 0;
  *(uint8_t*)0x20f2b0d2 = 0;
  *(uint8_t*)0x20f2b0d3 = 0;
  *(uint8_t*)0x20f2b0d4 = 0;
  *(uint8_t*)0x20f2b0d5 = 0;
  *(uint8_t*)0x20f2b0d6 = 0;
  *(uint8_t*)0x20f2b0d7 = 0;
  *(uint8_t*)0x20f2b0d8 = 0;
  *(uint8_t*)0x20f2b0d9 = 0;
  *(uint8_t*)0x20f2b0da = 0;
  *(uint8_t*)0x20f2b0db = 0;
  *(uint8_t*)0x20f2b0dc = 0;
  *(uint8_t*)0x20f2b0dd = 0;
  *(uint8_t*)0x20f2b0de = 0;
  *(uint8_t*)0x20f2b0df = 0;
  *(uint8_t*)0x20f2b0e0 = 0;
  *(uint8_t*)0x20f2b0e1 = 0;
  *(uint8_t*)0x20f2b0e2 = 0;
  *(uint8_t*)0x20f2b0e3 = 0;
  *(uint8_t*)0x20f2b0e4 = 0;
  *(uint8_t*)0x20f2b0e5 = 0;
  *(uint8_t*)0x20f2b0e6 = 0;
  *(uint8_t*)0x20f2b0e7 = 0;
  *(uint32_t*)0x20f2b0e8 = 0;
  *(uint16_t*)0x20f2b0ec = 0x190;
  *(uint16_t*)0x20f2b0ee = 0x1b8;
  *(uint32_t*)0x20f2b0f0 = 0;
  *(uint64_t*)0x20f2b0f8 = 0;
  *(uint64_t*)0x20f2b100 = 0;
  *(uint16_t*)0x20f2b108 = 0xc0;
  memcpy((void*)0x20f2b10a, "\x63\x6f\x6e\x6e\x74\x72\x61\x63\x6b\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20f2b127 = 2;
  *(uint8_t*)0x20f2b128 = 0xfe;
  *(uint8_t*)0x20f2b129 = 0x80;
  *(uint8_t*)0x20f2b12a = 0;
  *(uint8_t*)0x20f2b12b = 0;
  *(uint8_t*)0x20f2b12c = 0;
  *(uint8_t*)0x20f2b12d = 0;
  *(uint8_t*)0x20f2b12e = 0;
  *(uint8_t*)0x20f2b12f = 0;
  *(uint8_t*)0x20f2b130 = 0;
  *(uint8_t*)0x20f2b131 = 0;
  *(uint8_t*)0x20f2b132 = 0;
  *(uint8_t*)0x20f2b133 = 0;
  *(uint8_t*)0x20f2b134 = 0;
  *(uint8_t*)0x20f2b135 = 0;
  *(uint8_t*)0x20f2b136 = 0;
  *(uint8_t*)0x20f2b137 = 0;
  *(uint32_t*)0x20f2b138 = htobe32(0);
  *(uint32_t*)0x20f2b13c = htobe32(0);
  *(uint32_t*)0x20f2b140 = htobe32(0);
  *(uint32_t*)0x20f2b144 = htobe32(0);
  *(uint32_t*)0x20f2b148 = htobe32(0xe0000002);
  *(uint32_t*)0x20f2b158 = htobe32(0);
  *(uint32_t*)0x20f2b15c = htobe32(0);
  *(uint32_t*)0x20f2b160 = htobe32(0);
  *(uint32_t*)0x20f2b164 = htobe32(0);
  *(uint32_t*)0x20f2b168 = htobe32(0);
  *(uint32_t*)0x20f2b178 = htobe32(0);
  *(uint32_t*)0x20f2b17c = htobe32(0);
  *(uint32_t*)0x20f2b180 = htobe32(0);
  *(uint32_t*)0x20f2b184 = htobe32(0);
  *(uint8_t*)0x20f2b188 = -1;
  *(uint8_t*)0x20f2b189 = 1;
  *(uint8_t*)0x20f2b18a = 0;
  *(uint8_t*)0x20f2b18b = 0;
  *(uint8_t*)0x20f2b18c = 0;
  *(uint8_t*)0x20f2b18d = 0;
  *(uint8_t*)0x20f2b18e = 0;
  *(uint8_t*)0x20f2b18f = 0;
  *(uint8_t*)0x20f2b190 = 0;
  *(uint8_t*)0x20f2b191 = 0;
  *(uint8_t*)0x20f2b192 = 0;
  *(uint8_t*)0x20f2b193 = 0;
  *(uint8_t*)0x20f2b194 = 0;
  *(uint8_t*)0x20f2b195 = 0;
  *(uint8_t*)0x20f2b196 = 0;
  *(uint8_t*)0x20f2b197 = 1;
  *(uint32_t*)0x20f2b198 = htobe32(0);
  *(uint32_t*)0x20f2b19c = htobe32(0);
  *(uint32_t*)0x20f2b1a0 = htobe32(0);
  *(uint32_t*)0x20f2b1a4 = htobe32(0);
  *(uint32_t*)0x20f2b1a8 = 0;
  *(uint32_t*)0x20f2b1ac = 0;
  *(uint16_t*)0x20f2b1b0 = 0;
  *(uint16_t*)0x20f2b1b2 = 0;
  *(uint16_t*)0x20f2b1b4 = 0;
  *(uint16_t*)0x20f2b1b6 = 0;
  *(uint16_t*)0x20f2b1b8 = 0;
  *(uint16_t*)0x20f2b1ba = 0;
  *(uint16_t*)0x20f2b1bc = 0;
  *(uint16_t*)0x20f2b1c0 = 0;
  *(uint16_t*)0x20f2b1c2 = 0;
  *(uint16_t*)0x20f2b1c8 = 0x28;
  memcpy((void*)0x20f2b1ca, "\x69\x70\x76\x36\x68\x65\x61\x64\x65\x72\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20f2b1e7 = 0;
  *(uint8_t*)0x20f2b1e8 = 0;
  *(uint8_t*)0x20f2b1e9 = 0;
  *(uint8_t*)0x20f2b1ea = 0;
  *(uint16_t*)0x20f2b1f0 = 0x28;
  memcpy((void*)0x20f2b1f2, "\x43\x4c\x41\x53\x53\x49\x46\x59\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20f2b20f = 0;
  *(uint32_t*)0x20f2b210 = 0;
  *(uint8_t*)0x20f2b218 = 0;
  *(uint8_t*)0x20f2b219 = 0;
  *(uint8_t*)0x20f2b21a = 0;
  *(uint8_t*)0x20f2b21b = 0;
  *(uint8_t*)0x20f2b21c = 0;
  *(uint8_t*)0x20f2b21d = 0;
  *(uint8_t*)0x20f2b21e = 0;
  *(uint8_t*)0x20f2b21f = 0;
  *(uint8_t*)0x20f2b220 = 0;
  *(uint8_t*)0x20f2b221 = 0;
  *(uint8_t*)0x20f2b222 = 0;
  *(uint8_t*)0x20f2b223 = 0;
  *(uint8_t*)0x20f2b224 = 0;
  *(uint8_t*)0x20f2b225 = 0;
  *(uint8_t*)0x20f2b226 = 0;
  *(uint8_t*)0x20f2b227 = 0;
  *(uint8_t*)0x20f2b228 = 0;
  *(uint8_t*)0x20f2b229 = 0;
  *(uint8_t*)0x20f2b22a = 0;
  *(uint8_t*)0x20f2b22b = 0;
  *(uint8_t*)0x20f2b22c = 0;
  *(uint8_t*)0x20f2b22d = 0;
  *(uint8_t*)0x20f2b22e = 0;
  *(uint8_t*)0x20f2b22f = 0;
  *(uint8_t*)0x20f2b230 = 0;
  *(uint8_t*)0x20f2b231 = 0;
  *(uint8_t*)0x20f2b232 = -1;
  *(uint8_t*)0x20f2b233 = -1;
  *(uint32_t*)0x20f2b234 = htobe32(0);
  *(uint32_t*)0x20f2b238 = htobe32(0);
  *(uint32_t*)0x20f2b23c = htobe32(0);
  *(uint32_t*)0x20f2b240 = htobe32(0);
  *(uint32_t*)0x20f2b244 = htobe32(0);
  *(uint32_t*)0x20f2b248 = htobe32(0);
  *(uint32_t*)0x20f2b24c = htobe32(0);
  *(uint32_t*)0x20f2b250 = htobe32(0);
  *(uint32_t*)0x20f2b254 = htobe32(0);
  memcpy((void*)0x20f2b258,
         "\xd0\x20\xfe\x68\xde\x5b\x75\x58\x43\xbd\xfa\xe5\x3b\xe8\x5d\xcc",
         16);
  memcpy((void*)0x20f2b268,
         "\x2c\xf1\x11\xda\x60\x9c\x57\xe0\x45\x4d\x62\xeb\x97\xa6\x6b\x6f",
         16);
  *(uint8_t*)0x20f2b278 = 0;
  *(uint8_t*)0x20f2b279 = 0;
  *(uint8_t*)0x20f2b27a = 0;
  *(uint8_t*)0x20f2b27b = 0;
  *(uint8_t*)0x20f2b27c = 0;
  *(uint8_t*)0x20f2b27d = 0;
  *(uint8_t*)0x20f2b27e = 0;
  *(uint8_t*)0x20f2b27f = 0;
  *(uint8_t*)0x20f2b280 = 0;
  *(uint8_t*)0x20f2b281 = 0;
  *(uint8_t*)0x20f2b282 = 0;
  *(uint8_t*)0x20f2b283 = 0;
  *(uint8_t*)0x20f2b284 = 0;
  *(uint8_t*)0x20f2b285 = 0;
  *(uint8_t*)0x20f2b286 = 0;
  *(uint8_t*)0x20f2b287 = 0;
  *(uint8_t*)0x20f2b288 = 0;
  *(uint8_t*)0x20f2b289 = 0;
  *(uint8_t*)0x20f2b28a = 0;
  *(uint8_t*)0x20f2b28b = 0;
  *(uint8_t*)0x20f2b28c = 0;
  *(uint8_t*)0x20f2b28d = 0;
  *(uint8_t*)0x20f2b28e = 0;
  *(uint8_t*)0x20f2b28f = 0;
  *(uint8_t*)0x20f2b290 = 0;
  *(uint8_t*)0x20f2b291 = 0;
  *(uint8_t*)0x20f2b292 = 0;
  *(uint8_t*)0x20f2b293 = 0;
  *(uint8_t*)0x20f2b294 = 0;
  *(uint8_t*)0x20f2b295 = 0;
  *(uint8_t*)0x20f2b296 = 0;
  *(uint8_t*)0x20f2b297 = 0;
  *(uint16_t*)0x20f2b298 = 0;
  *(uint8_t*)0x20f2b29a = 0;
  *(uint8_t*)0x20f2b29b = 0;
  *(uint8_t*)0x20f2b29c = 0;
  *(uint32_t*)0x20f2b2a0 = 0;
  *(uint16_t*)0x20f2b2a4 = 0x100;
  *(uint16_t*)0x20f2b2a6 = 0x148;
  *(uint32_t*)0x20f2b2a8 = 0;
  *(uint64_t*)0x20f2b2b0 = 0;
  *(uint64_t*)0x20f2b2b8 = 0;
  *(uint16_t*)0x20f2b2c0 = 0x58;
  memcpy((void*)0x20f2b2c2, "\x68\x61\x73\x68\x6c\x69\x6d\x69\x74\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20f2b2df = 1;
  memcpy((void*)0x20f2b2e0,
         "\x73\x69\x74\x2f\x80\xff\xff\xcc\xe4\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint32_t*)0x20f2b2f0 = 0;
  *(uint32_t*)0x20f2b2f4 = 0;
  *(uint32_t*)0x20f2b2f8 = 6;
  *(uint32_t*)0x20f2b2fc = 0;
  *(uint32_t*)0x20f2b300 = 0;
  *(uint32_t*)0x20f2b304 = 3;
  *(uint32_t*)0x20f2b308 = 5;
  *(uint8_t*)0x20f2b30c = 0;
  *(uint8_t*)0x20f2b30d = 0;
  *(uint64_t*)0x20f2b310 = 0;
  *(uint16_t*)0x20f2b318 = 0x48;
  memcpy((void*)0x20f2b31a, "\x52\x45\x44\x49\x52\x45\x43\x54\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20f2b337 = 0;
  *(uint32_t*)0x20f2b338 = 0;
  *(uint8_t*)0x20f2b33c = 0xfe;
  *(uint8_t*)0x20f2b33d = 0x80;
  *(uint8_t*)0x20f2b33e = 0;
  *(uint8_t*)0x20f2b33f = 0;
  *(uint8_t*)0x20f2b340 = 0;
  *(uint8_t*)0x20f2b341 = 0;
  *(uint8_t*)0x20f2b342 = 0;
  *(uint8_t*)0x20f2b343 = 0;
  *(uint8_t*)0x20f2b344 = 0;
  *(uint8_t*)0x20f2b345 = 0;
  *(uint8_t*)0x20f2b346 = 0;
  *(uint8_t*)0x20f2b347 = 0;
  *(uint8_t*)0x20f2b348 = 0;
  *(uint8_t*)0x20f2b349 = 0;
  *(uint8_t*)0x20f2b34a = 0;
  *(uint8_t*)0x20f2b34b = 0xbb;
  *(uint8_t*)0x20f2b34c = 0xfe;
  *(uint8_t*)0x20f2b34d = 0x80;
  *(uint8_t*)0x20f2b34e = 0;
  *(uint8_t*)0x20f2b34f = 0;
  *(uint8_t*)0x20f2b350 = 0;
  *(uint8_t*)0x20f2b351 = 0;
  *(uint8_t*)0x20f2b352 = 0;
  *(uint8_t*)0x20f2b353 = 0;
  *(uint8_t*)0x20f2b354 = 0;
  *(uint8_t*)0x20f2b355 = 0;
  *(uint8_t*)0x20f2b356 = 0;
  *(uint8_t*)0x20f2b357 = 0;
  *(uint8_t*)0x20f2b358 = 0;
  *(uint8_t*)0x20f2b359 = 0;
  *(uint8_t*)0x20f2b35a = 0;
  *(uint8_t*)0x20f2b35b = 0xaa;
  *(uint16_t*)0x20f2b35c = 0;
  *(uint16_t*)0x20f2b35e = 0;
  *(uint8_t*)0x20f2b360 = 0;
  *(uint8_t*)0x20f2b361 = 0;
  *(uint8_t*)0x20f2b362 = 0;
  *(uint8_t*)0x20f2b363 = 0;
  *(uint8_t*)0x20f2b364 = 0;
  *(uint8_t*)0x20f2b365 = 0;
  *(uint8_t*)0x20f2b366 = 0;
  *(uint8_t*)0x20f2b367 = 0;
  *(uint8_t*)0x20f2b368 = 0;
  *(uint8_t*)0x20f2b369 = 0;
  *(uint8_t*)0x20f2b36a = 0;
  *(uint8_t*)0x20f2b36b = 0;
  *(uint8_t*)0x20f2b36c = 0;
  *(uint8_t*)0x20f2b36d = 0;
  *(uint8_t*)0x20f2b36e = 0;
  *(uint8_t*)0x20f2b36f = 0;
  *(uint8_t*)0x20f2b370 = 0;
  *(uint8_t*)0x20f2b371 = 0;
  *(uint8_t*)0x20f2b372 = 0;
  *(uint8_t*)0x20f2b373 = 0;
  *(uint8_t*)0x20f2b374 = 0;
  *(uint8_t*)0x20f2b375 = 0;
  *(uint8_t*)0x20f2b376 = 0;
  *(uint8_t*)0x20f2b377 = 0;
  *(uint8_t*)0x20f2b378 = 0;
  *(uint8_t*)0x20f2b379 = 0;
  *(uint8_t*)0x20f2b37a = 0;
  *(uint8_t*)0x20f2b37b = 0;
  *(uint8_t*)0x20f2b37c = 0;
  *(uint8_t*)0x20f2b37d = 0;
  *(uint8_t*)0x20f2b37e = 0;
  *(uint8_t*)0x20f2b37f = 0;
  *(uint8_t*)0x20f2b380 = 0;
  *(uint8_t*)0x20f2b381 = 0;
  *(uint8_t*)0x20f2b382 = 0;
  *(uint8_t*)0x20f2b383 = 0;
  *(uint8_t*)0x20f2b384 = 0;
  *(uint8_t*)0x20f2b385 = 0;
  *(uint8_t*)0x20f2b386 = 0;
  *(uint8_t*)0x20f2b387 = 0;
  *(uint8_t*)0x20f2b388 = 0;
  *(uint8_t*)0x20f2b389 = 0;
  *(uint8_t*)0x20f2b38a = 0;
  *(uint8_t*)0x20f2b38b = 0;
  *(uint8_t*)0x20f2b38c = 0;
  *(uint8_t*)0x20f2b38d = 0;
  *(uint8_t*)0x20f2b38e = 0;
  *(uint8_t*)0x20f2b38f = 0;
  *(uint8_t*)0x20f2b390 = 0;
  *(uint8_t*)0x20f2b391 = 0;
  *(uint8_t*)0x20f2b392 = 0;
  *(uint8_t*)0x20f2b393 = 0;
  *(uint8_t*)0x20f2b394 = 0;
  *(uint8_t*)0x20f2b395 = 0;
  *(uint8_t*)0x20f2b396 = 0;
  *(uint8_t*)0x20f2b397 = 0;
  *(uint8_t*)0x20f2b398 = 0;
  *(uint8_t*)0x20f2b399 = 0;
  *(uint8_t*)0x20f2b39a = 0;
  *(uint8_t*)0x20f2b39b = 0;
  *(uint8_t*)0x20f2b39c = 0;
  *(uint8_t*)0x20f2b39d = 0;
  *(uint8_t*)0x20f2b39e = 0;
  *(uint8_t*)0x20f2b39f = 0;
  *(uint8_t*)0x20f2b3a0 = 0;
  *(uint8_t*)0x20f2b3a1 = 0;
  *(uint8_t*)0x20f2b3a2 = 0;
  *(uint8_t*)0x20f2b3a3 = 0;
  *(uint8_t*)0x20f2b3a4 = 0;
  *(uint8_t*)0x20f2b3a5 = 0;
  *(uint8_t*)0x20f2b3a6 = 0;
  *(uint8_t*)0x20f2b3a7 = 0;
  *(uint8_t*)0x20f2b3a8 = 0;
  *(uint8_t*)0x20f2b3a9 = 0;
  *(uint8_t*)0x20f2b3aa = 0;
  *(uint8_t*)0x20f2b3ab = 0;
  *(uint8_t*)0x20f2b3ac = 0;
  *(uint8_t*)0x20f2b3ad = 0;
  *(uint8_t*)0x20f2b3ae = 0;
  *(uint8_t*)0x20f2b3af = 0;
  *(uint8_t*)0x20f2b3b0 = 0;
  *(uint8_t*)0x20f2b3b1 = 0;
  *(uint8_t*)0x20f2b3b2 = 0;
  *(uint8_t*)0x20f2b3b3 = 0;
  *(uint8_t*)0x20f2b3b4 = 0;
  *(uint8_t*)0x20f2b3b5 = 0;
  *(uint8_t*)0x20f2b3b6 = 0;
  *(uint8_t*)0x20f2b3b7 = 0;
  *(uint8_t*)0x20f2b3b8 = 0;
  *(uint8_t*)0x20f2b3b9 = 0;
  *(uint8_t*)0x20f2b3ba = 0;
  *(uint8_t*)0x20f2b3bb = 0;
  *(uint8_t*)0x20f2b3bc = 0;
  *(uint8_t*)0x20f2b3bd = 0;
  *(uint8_t*)0x20f2b3be = 0;
  *(uint8_t*)0x20f2b3bf = 0;
  *(uint8_t*)0x20f2b3c0 = 0;
  *(uint8_t*)0x20f2b3c1 = 0;
  *(uint8_t*)0x20f2b3c2 = 0;
  *(uint8_t*)0x20f2b3c3 = 0;
  *(uint8_t*)0x20f2b3c4 = 0;
  *(uint8_t*)0x20f2b3c5 = 0;
  *(uint8_t*)0x20f2b3c6 = 0;
  *(uint8_t*)0x20f2b3c7 = 0;
  *(uint8_t*)0x20f2b3c8 = 0;
  *(uint8_t*)0x20f2b3c9 = 0;
  *(uint8_t*)0x20f2b3ca = 0;
  *(uint8_t*)0x20f2b3cb = 0;
  *(uint8_t*)0x20f2b3cc = 0;
  *(uint8_t*)0x20f2b3cd = 0;
  *(uint8_t*)0x20f2b3ce = 0;
  *(uint8_t*)0x20f2b3cf = 0;
  *(uint8_t*)0x20f2b3d0 = 0;
  *(uint8_t*)0x20f2b3d1 = 0;
  *(uint8_t*)0x20f2b3d2 = 0;
  *(uint8_t*)0x20f2b3d3 = 0;
  *(uint8_t*)0x20f2b3d4 = 0;
  *(uint8_t*)0x20f2b3d5 = 0;
  *(uint8_t*)0x20f2b3d6 = 0;
  *(uint8_t*)0x20f2b3d7 = 0;
  *(uint8_t*)0x20f2b3d8 = 0;
  *(uint8_t*)0x20f2b3d9 = 0;
  *(uint8_t*)0x20f2b3da = 0;
  *(uint8_t*)0x20f2b3db = 0;
  *(uint8_t*)0x20f2b3dc = 0;
  *(uint8_t*)0x20f2b3dd = 0;
  *(uint8_t*)0x20f2b3de = 0;
  *(uint8_t*)0x20f2b3df = 0;
  *(uint8_t*)0x20f2b3e0 = 0;
  *(uint8_t*)0x20f2b3e1 = 0;
  *(uint8_t*)0x20f2b3e2 = 0;
  *(uint8_t*)0x20f2b3e3 = 0;
  *(uint8_t*)0x20f2b3e4 = 0;
  *(uint8_t*)0x20f2b3e5 = 0;
  *(uint8_t*)0x20f2b3e6 = 0;
  *(uint8_t*)0x20f2b3e7 = 0;
  *(uint32_t*)0x20f2b3e8 = 0;
  *(uint16_t*)0x20f2b3ec = 0x118;
  *(uint16_t*)0x20f2b3ee = 0x140;
  *(uint32_t*)0x20f2b3f0 = 0;
  *(uint64_t*)0x20f2b3f8 = 0;
  *(uint64_t*)0x20f2b400 = 0;
  *(uint16_t*)0x20f2b408 = 0x48;
  memcpy((void*)0x20f2b40a, "\x64\x73\x74\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20f2b427 = 0;
  *(uint32_t*)0x20f2b428 = 0;
  *(uint8_t*)0x20f2b42c = 0;
  *(uint8_t*)0x20f2b42d = 0;
  *(uint16_t*)0x20f2b42e = 0;
  *(uint16_t*)0x20f2b430 = 0;
  *(uint16_t*)0x20f2b432 = 0;
  *(uint16_t*)0x20f2b434 = 0;
  *(uint16_t*)0x20f2b436 = 0;
  *(uint16_t*)0x20f2b438 = 0;
  *(uint16_t*)0x20f2b43a = 0;
  *(uint16_t*)0x20f2b43c = 0;
  *(uint16_t*)0x20f2b43e = 0;
  *(uint16_t*)0x20f2b440 = 0;
  *(uint16_t*)0x20f2b442 = 0;
  *(uint16_t*)0x20f2b444 = 0;
  *(uint16_t*)0x20f2b446 = 0;
  *(uint16_t*)0x20f2b448 = 0;
  *(uint16_t*)0x20f2b44a = 0;
  *(uint16_t*)0x20f2b44c = 0;
  *(uint8_t*)0x20f2b44e = 0;
  *(uint16_t*)0x20f2b450 = 0x28;
  memcpy((void*)0x20f2b452, "\x69\x70\x76\x36\x68\x65\x61\x64\x65\x72\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20f2b46f = 0;
  *(uint8_t*)0x20f2b470 = 0;
  *(uint8_t*)0x20f2b471 = 0;
  *(uint8_t*)0x20f2b472 = 0;
  *(uint16_t*)0x20f2b478 = 0x28;
  memcpy((void*)0x20f2b47a, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20f2b497 = 0;
  *(uint32_t*)0x20f2b498 = -1;
  *(uint8_t*)0x20f2b4a0 = 0;
  *(uint8_t*)0x20f2b4a1 = 0;
  *(uint8_t*)0x20f2b4a2 = 0;
  *(uint8_t*)0x20f2b4a3 = 0;
  *(uint8_t*)0x20f2b4a4 = 0;
  *(uint8_t*)0x20f2b4a5 = 0;
  *(uint8_t*)0x20f2b4a6 = 0;
  *(uint8_t*)0x20f2b4a7 = 0;
  *(uint8_t*)0x20f2b4a8 = 0;
  *(uint8_t*)0x20f2b4a9 = 0;
  *(uint8_t*)0x20f2b4aa = 0;
  *(uint8_t*)0x20f2b4ab = 0;
  *(uint8_t*)0x20f2b4ac = 0;
  *(uint8_t*)0x20f2b4ad = 0;
  *(uint8_t*)0x20f2b4ae = 0;
  *(uint8_t*)0x20f2b4af = 0;
  *(uint8_t*)0x20f2b4b0 = 0;
  *(uint8_t*)0x20f2b4b1 = 0;
  *(uint8_t*)0x20f2b4b2 = 0;
  *(uint8_t*)0x20f2b4b3 = 0;
  *(uint8_t*)0x20f2b4b4 = 0;
  *(uint8_t*)0x20f2b4b5 = 0;
  *(uint8_t*)0x20f2b4b6 = 0;
  *(uint8_t*)0x20f2b4b7 = 0;
  *(uint8_t*)0x20f2b4b8 = 0;
  *(uint8_t*)0x20f2b4b9 = 0;
  *(uint8_t*)0x20f2b4ba = 0;
  *(uint8_t*)0x20f2b4bb = 0;
  *(uint8_t*)0x20f2b4bc = 0;
  *(uint8_t*)0x20f2b4bd = 0;
  *(uint8_t*)0x20f2b4be = 0;
  *(uint8_t*)0x20f2b4bf = 0;
  *(uint8_t*)0x20f2b4c0 = 0;
  *(uint8_t*)0x20f2b4c1 = 0;
  *(uint8_t*)0x20f2b4c2 = 0;
  *(uint8_t*)0x20f2b4c3 = 0;
  *(uint8_t*)0x20f2b4c4 = 0;
  *(uint8_t*)0x20f2b4c5 = 0;
  *(uint8_t*)0x20f2b4c6 = 0;
  *(uint8_t*)0x20f2b4c7 = 0;
  *(uint8_t*)0x20f2b4c8 = 0;
  *(uint8_t*)0x20f2b4c9 = 0;
  *(uint8_t*)0x20f2b4ca = 0;
  *(uint8_t*)0x20f2b4cb = 0;
  *(uint8_t*)0x20f2b4cc = 0;
  *(uint8_t*)0x20f2b4cd = 0;
  *(uint8_t*)0x20f2b4ce = 0;
  *(uint8_t*)0x20f2b4cf = 0;
  *(uint8_t*)0x20f2b4d0 = 0;
  *(uint8_t*)0x20f2b4d1 = 0;
  *(uint8_t*)0x20f2b4d2 = 0;
  *(uint8_t*)0x20f2b4d3 = 0;
  *(uint8_t*)0x20f2b4d4 = 0;
  *(uint8_t*)0x20f2b4d5 = 0;
  *(uint8_t*)0x20f2b4d6 = 0;
  *(uint8_t*)0x20f2b4d7 = 0;
  *(uint8_t*)0x20f2b4d8 = 0;
  *(uint8_t*)0x20f2b4d9 = 0;
  *(uint8_t*)0x20f2b4da = 0;
  *(uint8_t*)0x20f2b4db = 0;
  *(uint8_t*)0x20f2b4dc = 0;
  *(uint8_t*)0x20f2b4dd = 0;
  *(uint8_t*)0x20f2b4de = 0;
  *(uint8_t*)0x20f2b4df = 0;
  *(uint8_t*)0x20f2b4e0 = 0;
  *(uint8_t*)0x20f2b4e1 = 0;
  *(uint8_t*)0x20f2b4e2 = 0;
  *(uint8_t*)0x20f2b4e3 = 0;
  *(uint8_t*)0x20f2b4e4 = 0;
  *(uint8_t*)0x20f2b4e5 = 0;
  *(uint8_t*)0x20f2b4e6 = 0;
  *(uint8_t*)0x20f2b4e7 = 0;
  *(uint8_t*)0x20f2b4e8 = 0;
  *(uint8_t*)0x20f2b4e9 = 0;
  *(uint8_t*)0x20f2b4ea = 0;
  *(uint8_t*)0x20f2b4eb = 0;
  *(uint8_t*)0x20f2b4ec = 0;
  *(uint8_t*)0x20f2b4ed = 0;
  *(uint8_t*)0x20f2b4ee = 0;
  *(uint8_t*)0x20f2b4ef = 0;
  *(uint8_t*)0x20f2b4f0 = 0;
  *(uint8_t*)0x20f2b4f1 = 0;
  *(uint8_t*)0x20f2b4f2 = 0;
  *(uint8_t*)0x20f2b4f3 = 0;
  *(uint8_t*)0x20f2b4f4 = 0;
  *(uint8_t*)0x20f2b4f5 = 0;
  *(uint8_t*)0x20f2b4f6 = 0;
  *(uint8_t*)0x20f2b4f7 = 0;
  *(uint8_t*)0x20f2b4f8 = 0;
  *(uint8_t*)0x20f2b4f9 = 0;
  *(uint8_t*)0x20f2b4fa = 0;
  *(uint8_t*)0x20f2b4fb = 0;
  *(uint8_t*)0x20f2b4fc = 0;
  *(uint8_t*)0x20f2b4fd = 0;
  *(uint8_t*)0x20f2b4fe = 0;
  *(uint8_t*)0x20f2b4ff = 0;
  *(uint8_t*)0x20f2b500 = 0;
  *(uint8_t*)0x20f2b501 = 0;
  *(uint8_t*)0x20f2b502 = 0;
  *(uint8_t*)0x20f2b503 = 0;
  *(uint8_t*)0x20f2b504 = 0;
  *(uint8_t*)0x20f2b505 = 0;
  *(uint8_t*)0x20f2b506 = 0;
  *(uint8_t*)0x20f2b507 = 0;
  *(uint8_t*)0x20f2b508 = 0;
  *(uint8_t*)0x20f2b509 = 0;
  *(uint8_t*)0x20f2b50a = 0;
  *(uint8_t*)0x20f2b50b = 0;
  *(uint8_t*)0x20f2b50c = 0;
  *(uint8_t*)0x20f2b50d = 0;
  *(uint8_t*)0x20f2b50e = 0;
  *(uint8_t*)0x20f2b50f = 0;
  *(uint8_t*)0x20f2b510 = 0;
  *(uint8_t*)0x20f2b511 = 0;
  *(uint8_t*)0x20f2b512 = 0;
  *(uint8_t*)0x20f2b513 = 0;
  *(uint8_t*)0x20f2b514 = 0;
  *(uint8_t*)0x20f2b515 = 0;
  *(uint8_t*)0x20f2b516 = 0;
  *(uint8_t*)0x20f2b517 = 0;
  *(uint8_t*)0x20f2b518 = 0;
  *(uint8_t*)0x20f2b519 = 0;
  *(uint8_t*)0x20f2b51a = 0;
  *(uint8_t*)0x20f2b51b = 0;
  *(uint8_t*)0x20f2b51c = 0;
  *(uint8_t*)0x20f2b51d = 0;
  *(uint8_t*)0x20f2b51e = 0;
  *(uint8_t*)0x20f2b51f = 0;
  *(uint8_t*)0x20f2b520 = 0;
  *(uint8_t*)0x20f2b521 = 0;
  *(uint8_t*)0x20f2b522 = 0;
  *(uint8_t*)0x20f2b523 = 0;
  *(uint8_t*)0x20f2b524 = 0;
  *(uint8_t*)0x20f2b525 = 0;
  *(uint8_t*)0x20f2b526 = 0;
  *(uint8_t*)0x20f2b527 = 0;
  *(uint32_t*)0x20f2b528 = 0;
  *(uint16_t*)0x20f2b52c = 0xd0;
  *(uint16_t*)0x20f2b52e = 0x118;
  *(uint32_t*)0x20f2b530 = 0;
  *(uint64_t*)0x20f2b538 = 0;
  *(uint64_t*)0x20f2b540 = 0;
  *(uint16_t*)0x20f2b548 = 0x28;
  memcpy((void*)0x20f2b54a, "\x6d\x68\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20f2b567 = 0;
  *(uint8_t*)0x20f2b568 = 0;
  *(uint8_t*)0x20f2b569 = 0;
  *(uint8_t*)0x20f2b56a = 0;
  *(uint16_t*)0x20f2b570 = 0x48;
  memcpy((void*)0x20f2b572, "\x4e\x45\x54\x4d\x41\x50\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20f2b58f = 0;
  *(uint32_t*)0x20f2b590 = 0;
  *(uint32_t*)0x20f2b594 = htobe32(0);
  *(uint8_t*)0x20f2b5a4 = 0xac;
  *(uint8_t*)0x20f2b5a5 = 0x14;
  *(uint8_t*)0x20f2b5a6 = 0;
  *(uint8_t*)0x20f2b5a7 = 0;
  *(uint16_t*)0x20f2b5b4 = 0;
  *(uint16_t*)0x20f2b5b6 = 0;
  *(uint8_t*)0x20f2b5b8 = 0;
  *(uint8_t*)0x20f2b5b9 = 0;
  *(uint8_t*)0x20f2b5ba = 0;
  *(uint8_t*)0x20f2b5bb = 0;
  *(uint8_t*)0x20f2b5bc = 0;
  *(uint8_t*)0x20f2b5bd = 0;
  *(uint8_t*)0x20f2b5be = 0;
  *(uint8_t*)0x20f2b5bf = 0;
  *(uint8_t*)0x20f2b5c0 = 0;
  *(uint8_t*)0x20f2b5c1 = 0;
  *(uint8_t*)0x20f2b5c2 = 0;
  *(uint8_t*)0x20f2b5c3 = 0;
  *(uint8_t*)0x20f2b5c4 = 0;
  *(uint8_t*)0x20f2b5c5 = 0;
  *(uint8_t*)0x20f2b5c6 = 0;
  *(uint8_t*)0x20f2b5c7 = 0;
  *(uint8_t*)0x20f2b5c8 = 0;
  *(uint8_t*)0x20f2b5c9 = 0;
  *(uint8_t*)0x20f2b5ca = 0;
  *(uint8_t*)0x20f2b5cb = 0;
  *(uint8_t*)0x20f2b5cc = 0;
  *(uint8_t*)0x20f2b5cd = 0;
  *(uint8_t*)0x20f2b5ce = 0;
  *(uint8_t*)0x20f2b5cf = 0;
  *(uint8_t*)0x20f2b5d0 = 0;
  *(uint8_t*)0x20f2b5d1 = 0;
  *(uint8_t*)0x20f2b5d2 = 0;
  *(uint8_t*)0x20f2b5d3 = 0;
  *(uint8_t*)0x20f2b5d4 = 0;
  *(uint8_t*)0x20f2b5d5 = 0;
  *(uint8_t*)0x20f2b5d6 = 0;
  *(uint8_t*)0x20f2b5d7 = 0;
  *(uint8_t*)0x20f2b5d8 = 0;
  *(uint8_t*)0x20f2b5d9 = 0;
  *(uint8_t*)0x20f2b5da = 0;
  *(uint8_t*)0x20f2b5db = 0;
  *(uint8_t*)0x20f2b5dc = 0;
  *(uint8_t*)0x20f2b5dd = 0;
  *(uint8_t*)0x20f2b5de = 0;
  *(uint8_t*)0x20f2b5df = 0;
  *(uint8_t*)0x20f2b5e0 = 0;
  *(uint8_t*)0x20f2b5e1 = 0;
  *(uint8_t*)0x20f2b5e2 = 0;
  *(uint8_t*)0x20f2b5e3 = 0;
  *(uint8_t*)0x20f2b5e4 = 0;
  *(uint8_t*)0x20f2b5e5 = 0;
  *(uint8_t*)0x20f2b5e6 = 0;
  *(uint8_t*)0x20f2b5e7 = 0;
  *(uint8_t*)0x20f2b5e8 = 0;
  *(uint8_t*)0x20f2b5e9 = 0;
  *(uint8_t*)0x20f2b5ea = 0;
  *(uint8_t*)0x20f2b5eb = 0;
  *(uint8_t*)0x20f2b5ec = 0;
  *(uint8_t*)0x20f2b5ed = 0;
  *(uint8_t*)0x20f2b5ee = 0;
  *(uint8_t*)0x20f2b5ef = 0;
  *(uint8_t*)0x20f2b5f0 = 0;
  *(uint8_t*)0x20f2b5f1 = 0;
  *(uint8_t*)0x20f2b5f2 = 0;
  *(uint8_t*)0x20f2b5f3 = 0;
  *(uint8_t*)0x20f2b5f4 = 0;
  *(uint8_t*)0x20f2b5f5 = 0;
  *(uint8_t*)0x20f2b5f6 = 0;
  *(uint8_t*)0x20f2b5f7 = 0;
  *(uint8_t*)0x20f2b5f8 = 0;
  *(uint8_t*)0x20f2b5f9 = 0;
  *(uint8_t*)0x20f2b5fa = 0;
  *(uint8_t*)0x20f2b5fb = 0;
  *(uint8_t*)0x20f2b5fc = 0;
  *(uint8_t*)0x20f2b5fd = 0;
  *(uint8_t*)0x20f2b5fe = 0;
  *(uint8_t*)0x20f2b5ff = 0;
  *(uint8_t*)0x20f2b600 = 0;
  *(uint8_t*)0x20f2b601 = 0;
  *(uint8_t*)0x20f2b602 = 0;
  *(uint8_t*)0x20f2b603 = 0;
  *(uint8_t*)0x20f2b604 = 0;
  *(uint8_t*)0x20f2b605 = 0;
  *(uint8_t*)0x20f2b606 = 0;
  *(uint8_t*)0x20f2b607 = 0;
  *(uint8_t*)0x20f2b608 = 0;
  *(uint8_t*)0x20f2b609 = 0;
  *(uint8_t*)0x20f2b60a = 0;
  *(uint8_t*)0x20f2b60b = 0;
  *(uint8_t*)0x20f2b60c = 0;
  *(uint8_t*)0x20f2b60d = 0;
  *(uint8_t*)0x20f2b60e = 0;
  *(uint8_t*)0x20f2b60f = 0;
  *(uint8_t*)0x20f2b610 = 0;
  *(uint8_t*)0x20f2b611 = 0;
  *(uint8_t*)0x20f2b612 = 0;
  *(uint8_t*)0x20f2b613 = 0;
  *(uint8_t*)0x20f2b614 = 0;
  *(uint8_t*)0x20f2b615 = 0;
  *(uint8_t*)0x20f2b616 = 0;
  *(uint8_t*)0x20f2b617 = 0;
  *(uint8_t*)0x20f2b618 = 0;
  *(uint8_t*)0x20f2b619 = 0;
  *(uint8_t*)0x20f2b61a = 0;
  *(uint8_t*)0x20f2b61b = 0;
  *(uint8_t*)0x20f2b61c = 0;
  *(uint8_t*)0x20f2b61d = 0;
  *(uint8_t*)0x20f2b61e = 0;
  *(uint8_t*)0x20f2b61f = 0;
  *(uint8_t*)0x20f2b620 = 0;
  *(uint8_t*)0x20f2b621 = 0;
  *(uint8_t*)0x20f2b622 = 0;
  *(uint8_t*)0x20f2b623 = 0;
  *(uint8_t*)0x20f2b624 = 0;
  *(uint8_t*)0x20f2b625 = 0;
  *(uint8_t*)0x20f2b626 = 0;
  *(uint8_t*)0x20f2b627 = 0;
  *(uint8_t*)0x20f2b628 = 0;
  *(uint8_t*)0x20f2b629 = 0;
  *(uint8_t*)0x20f2b62a = 0;
  *(uint8_t*)0x20f2b62b = 0;
  *(uint8_t*)0x20f2b62c = 0;
  *(uint8_t*)0x20f2b62d = 0;
  *(uint8_t*)0x20f2b62e = 0;
  *(uint8_t*)0x20f2b62f = 0;
  *(uint8_t*)0x20f2b630 = 0;
  *(uint8_t*)0x20f2b631 = 0;
  *(uint8_t*)0x20f2b632 = 0;
  *(uint8_t*)0x20f2b633 = 0;
  *(uint8_t*)0x20f2b634 = 0;
  *(uint8_t*)0x20f2b635 = 0;
  *(uint8_t*)0x20f2b636 = 0;
  *(uint8_t*)0x20f2b637 = 0;
  *(uint8_t*)0x20f2b638 = 0;
  *(uint8_t*)0x20f2b639 = 0;
  *(uint8_t*)0x20f2b63a = 0;
  *(uint8_t*)0x20f2b63b = 0;
  *(uint8_t*)0x20f2b63c = 0;
  *(uint8_t*)0x20f2b63d = 0;
  *(uint8_t*)0x20f2b63e = 0;
  *(uint8_t*)0x20f2b63f = 0;
  *(uint32_t*)0x20f2b640 = 0;
  *(uint16_t*)0x20f2b644 = 0xa8;
  *(uint16_t*)0x20f2b646 = 0xd0;
  *(uint32_t*)0x20f2b648 = 0;
  *(uint64_t*)0x20f2b650 = 0;
  *(uint64_t*)0x20f2b658 = 0;
  *(uint16_t*)0x20f2b660 = 0x28;
  memcpy((void*)0x20f2b662, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20f2b67f = 0;
  *(uint32_t*)0x20f2b680 = 0xfffffffe;
  syscall(__NR_setsockopt, r[0], 0x29, 0x40, 0x20f2b000, 0x688);
}

int main()
{
  loop();
  return 0;
}