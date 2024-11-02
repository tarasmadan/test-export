// https://syzkaller.appspot.com/bug?id=bef2b6ede833b761163d45b5b8544dd6291b3f0b
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

struct csum_inet {
  uint32_t acc;
};

static void csum_inet_init(struct csum_inet* csum)
{
  csum->acc = 0;
}

static void csum_inet_update(struct csum_inet* csum, const uint8_t* data,
                             size_t length)
{
  if (length == 0)
    return;

  size_t i;
  for (i = 0; i < length - 1; i += 2)
    csum->acc += *(uint16_t*)&data[i];

  if (length & 1)
    csum->acc += (uint16_t)data[length - 1];

  while (csum->acc > 0xffff)
    csum->acc = (csum->acc & 0xffff) + (csum->acc >> 16);
}

static uint16_t csum_inet_digest(struct csum_inet* csum)
{
  return ~csum->acc;
}

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_socket, 2, 1, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x2039c000 = 1;
  syscall(__NR_setsockopt, r[0], 6, 0x10000000013, 0x2039c000, 4);
  *(uint32_t*)0x200b2000 = -1;
  syscall(__NR_setsockopt, r[0], 6, 0x14, 0x200b2000, 4);
  *(uint16_t*)0x20b55000 = 2;
  *(uint16_t*)0x20b55002 = htobe16(0x4e22);
  *(uint32_t*)0x20b55004 = htobe32(0);
  *(uint8_t*)0x20b55008 = 0;
  *(uint8_t*)0x20b55009 = 0;
  *(uint8_t*)0x20b5500a = 0;
  *(uint8_t*)0x20b5500b = 0;
  *(uint8_t*)0x20b5500c = 0;
  *(uint8_t*)0x20b5500d = 0;
  *(uint8_t*)0x20b5500e = 0;
  *(uint8_t*)0x20b5500f = 0;
  syscall(__NR_bind, r[0], 0x20b55000, 0x10);
  *(uint16_t*)0x20000040 = 2;
  *(uint16_t*)0x20000042 = htobe16(0x4e22);
  *(uint32_t*)0x20000044 = htobe32(0x7f000001);
  *(uint8_t*)0x20000048 = 0;
  *(uint8_t*)0x20000049 = 0;
  *(uint8_t*)0x2000004a = 0;
  *(uint8_t*)0x2000004b = 0;
  *(uint8_t*)0x2000004c = 0;
  *(uint8_t*)0x2000004d = 0;
  *(uint8_t*)0x2000004e = 0;
  *(uint8_t*)0x2000004f = 0;
  syscall(__NR_sendto, r[0], 0x205c9000, 0x4da, 0x800000020000000, 0x20000040,
          0x10);
  *(uint32_t*)0x20965fec = 0x40000004;
  *(uint32_t*)0x20965ff0 = 0x852b;
  *(uint32_t*)0x20965ff4 = 0xffff;
  *(uint32_t*)0x20965ff8 = 0x7fffffff;
  *(uint32_t*)0x20965ffc = 0;
  syscall(__NR_setsockopt, r[0], 6, 0x1d, 0x20965fec, 0x14);
  *(uint64_t*)0x20000440 = 0x20000080;
  memcpy((void*)0x20000080, "\xb8", 1);
  *(uint64_t*)0x20000448 = 1;
  syscall(__NR_writev, r[0], 0x20000440, 1);
  *(uint32_t*)0x20000000 = 8;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0x7c;
  *(uint32_t*)0x20000010 = 0;
  *(uint32_t*)0x20000014 = 0;
  *(uint32_t*)0x20000018 = 0;
  *(uint32_t*)0x2000001c = 0;
  *(uint32_t*)0x20000020 = 0;
  *(uint32_t*)0x20000024 = 0;
  *(uint32_t*)0x20000028 = 0;
  *(uint32_t*)0x2000002c = 0;
  *(uint32_t*)0x20000030 = 4;
  *(uint32_t*)0x20000034 = 0;
  *(uint32_t*)0x20000038 = 0;
  *(uint32_t*)0x2000003c = 0;
  syscall(__NR_setsockopt, r[0], 6, 0x16, 0x20000000, 0x182);
  *(uint64_t*)0x200006c0 = 0x20001ac0;
  memcpy(
      (void*)0x20001ac0,
      "\x88\x76\x0d\x5b\x96\xd0\x74\x99\x71\x77\x51\x84\xb4\x6c\xac\x09\x01\x10"
      "\xa5\x8c\x70\xe8\x48\x93\xac\x3b\x5c\x1f\xe5\xc7\x70\x24\xfc\x8a\x94\x4a"
      "\x2f\x4c\xdd\x82\x95\x57\x9b\x34\x6a\x69\x1d\x00\x4b\x56\x53\xf5\x73\x38"
      "\xed\x4d\x2e\x04\x0b\x8d\x7f\x24\xcd\xb8\xcc\x50\x83\x2d\x57\x3c\x03\x72"
      "\x46\xac\xf3\xdb\xb5\x3c\x2d\x72\x0f\xdd\x3c\x50\x38\xbf\x5a\x5b\x71\xbc"
      "\x9a\x56\x5b\xd0\x9a\x20\x40\xce\x93\x09\x8b\xd5\x85\xba\x05\x13\x83\x85"
      "\xbd\xf9\x6e\x70\x82\x72\x97\x14\x30\xd4\xcb\x0e\xeb\xec\x17\xfe\x5b\x24"
      "\xbf\xd8\x9b\xb5\xa9\x57\x2a\x33\x37\xf9\x12\xe1\x90\x37\x1e\x5f\x05\x98"
      "\x10\xdf\xe6\x61\x09\xce\x3a\x12\x84\xb1\x53\xef\xe6\xf7\x26\x9c\xc6\x97"
      "\x5c\xb4\xdd\xad\x48\xec\x3c\xf2\x66\xd6\x7a\x1d\x1e\xaa\xf5\x60\x19\xf3"
      "\x02\x95\xc6\x80\x37\x32\x2d\x9a\xa7\x99\x0c\x56\xba\x8e\x69\x9d\x22\xde"
      "\x15\xdd\x17\x6f\x54\xc5\x6e\xd8\x51\x8d\xcc\xfe\xf1\x60\x56\x28\x97\x6b"
      "\xd5\x98\xaa\xb6\x0d\x5f\x8c\x02\x9c\x37\xa6\xbd\x00\x3f\xae\x94\xca\x3a"
      "\x0b\x25\x98\x0c\x3e\xa1\xdf\x75\x44\x3f\x4f\xb5\xbc\x01\x52\x59\x27\x08"
      "\x80\x40\xb5\xca\xcf\xc9\x5f\x3e\x1f\xc6\x5f\x7f\xc4\x94\x81\x81\x9a\x15"
      "\x06\xdd\x3c\x02\x86\xef\x50\x9d\x75\x94\x2b\x6d\x9a\xf4\xa6\xcc\x25\x2f"
      "\xf6\x7b\x81\x8d\xa4\x0b\x80\x20\xe9\x23\x95\x15\xbc\x6e\x3c\xc7\x11\xbc"
      "\x8d\xf7\xbd\xae\x34\xa5\x97\x34\x15\x50\x51\xc9\xd2\x85\x7d\xa8\xd5\x46"
      "\x5b\x13\x52\x7d\x9f\xb8\x5b\x18\x65\x4b\x64\x2d\xa3\x8d\xe3\x34\xe1\xac"
      "\xe3\xc1\xb2\x95\x12\x3b\xb3\xcf\x1b\xad\x55\xd0\x76\xec\xb8\xba\x53\x43"
      "\x98\xdc\x30\x20\x43\x34\x1d\x41\xd1\x89\x4a\x56\xae\x1f\x82\x30\x1b\xe8"
      "\xe9\xcb\x89\x9c\xb3\x1f\xfa\x91\xbe\x43\xee\x18\x2d\x35\x13\xfe\x9d\xde"
      "\xd6\xba\x77\xa4\x64\x9a\xe2\x8b\x37\x9e\x95\xa0\xda\xf0\xb0\x25\x7b\xe4"
      "\xad\x89\x75\x87\x1f\x09\x62\xfd\x77\x37\x16\x55\xe0\xb5\x1d\x94\xfa\xad"
      "\x0d\x1c\xa4\x1b\xa0\x78\x0b\x00\x3c\xd8\x59\x58\xfd\x14\xef\x67\x70\xa1"
      "\x94\x27\xc0\x3f\x07\xbd\xa5\x90\x05\x76\x1f\xfc\x83\xe7\x60\xd3\x46\x6e"
      "\xff\x53\x58\x7b\xfe\xb4\xd3\xd2\x52\x70\x9b\xcc\xe8\x75\x81\x04\x7e\x5c"
      "\xf6\xdc\x8b\xdb\x69\x90\xfd\x68\xb1\x66\x2a\x78\x01\xa7\x0b\x79\xb8\x4e"
      "\x6f\x88\x55\xb4\xd0\xac\xd4\xf3\xcd\x50\x52\x84\xc3\xcb\x07\x24\x11\x00"
      "\x7d\x28\x70\x3d\x95\xf2\x06\x87\x2f\x8e\x62\x70\xce\xf3\x02\xac\x33\x91"
      "\xfa\x72\x3c\xe1\x89\x4c\xd9\x99\x82\x16\x08\xdd\xd3\x64\xe9\xcd\xb2\x63"
      "\xd0\xd4\x39\x0a\x74\x79\x99\xb2\xfe\x6e\xa2\x99\x4f\x99\xb4\xe8\xba\x6a"
      "\x94\xd7\x90\x96\xcc\x0b\xac\x90\x45\xae\x83\x64\x19\x2f\x5c\xcc\x0c\x4e"
      "\x7e\x47\x51\xe8\x64\xf3\x39\x56\x83\x85\x75\x7a\x5a\xed\xa3\xb0\xe8\x79"
      "\x04\x56\x46\x54\x39\x04\x19\xc7\xa7\x5f\xd8\x1c\xbe\xfa\x5d\xe6\xd4\x00"
      "\x4e\x00\x7e\xf5\xfc\x15\xe8\xb4\x58\x19\xcc\x65\xd4\x79\xa2\x8c\xa2\x18"
      "\x87\xb6\x1c\x53\x35\xa7\x9c\xff\x75\x4d\x86\x31\x7c\xdb\x66\x5c\x3a\x1e"
      "\x8c\x1c\x03\xa3\x72\x44\xf4\xe7\xc7\x39\xc7\x3a\x10\xdc\x4d\xce\xcb\x66"
      "\x62\xa9\x56\xbc\x66\xb1\xd8\x57\xc9\xbc\x59\xe8\xf3\xb1\xac\x3c\x85\xb7"
      "\xb4\x3a\x7b\xa0\x26\x2f\xf0\x25\x32\x10\x13\xbf\x5d\x16\x52\x6b\x85\x72"
      "\xda\x07\xb6\xc4\xfc\x8c\x4e\xf3\xa9\xbd\x8f\x61\x9a\xdf\xd5\x1e\x9d\x09"
      "\x2a\x71\xa1\x36\x4c\x6a\x5d\x8e\xec\xf2\x32\x58\x08\xac\xac\xce\xd7\x8a"
      "\xd2\xb8\x6e\x7a\x8d\xde\xcb\x50\x87\x24\x05\x78\xfd\x86\x2d\xde\x54\x54"
      "\x65\xf5\x40\xd6\x5f\xd0\xd1\xef\x56\xca\x54\xe7\x44\x54\x1a\x89\x4b\x3e"
      "\x57\x3f\x9b\x25\xc1\x7c\x94\x4f\x81\x6f\x24\x52\x6c\x77\x4d\xe6\x63\x57"
      "\x72\xa2\x6a\x4c\x00\x08\xdf\x7b\xac\x2f\xc1\x19\x82\xde\x76\x7d\x1a\xf6"
      "\x2d\x3f\x65\x6c\x0c\xe0\xc7\x74\xb9\x48\x46\xc8\xa8\xae\xbe\xe7\xa6\xf2"
      "\x8a\x9a\xe0\xd8\x16\xe0\x4b\x89\x0d\xfe\xea\x6c\xb6\x03\xa5\xce\xfa\x2b"
      "\x95\x6b\x8d\x9d\x0e\xb0\x67\x28\x88\xba\xd7\x1e\x9b\x7b\x58\x96\xf0\x29"
      "\xa0\x39\x16\x7a\xea\x72\xc2\xd3\xf8\xc0\x93\xcd\x8f\xce\x98\x40\x88\x01"
      "\xf1\x55\x21\x37\x24\x14\x0a\xb9\x83\x24\x7c\x6a\xd6\x59\x5c\x92\x28\xd6"
      "\x32\x39\x4c\x9d\x06\x6e\x1d\x08\x2d\x1a\xa7\x4b\x94\xe2\x45\xcf\x58\x05"
      "\x15\x95\x37\xf2\x68\x68\xd0\x16\xd1\xd7\x1c\xa9\xe6\xaa\xfc\xc3\x71\x5f"
      "\x0a\x7f\x35\x7d\x57\xac\x8d\xdd\x9e\x90\x12\x1c\x00\xf2\xc5\x23\xd1\xef"
      "\x70\xa2\xf8\x85\xc4\xc6\xf7\xd5\x4f\x73\x55\x4e\xf9\xfd\x8b\xe7\xb2\x05"
      "\x8b\x7f\x04\x99\x4b\xa4\xdc\x2a\xc2\x61\xf2\x46\x9b\x65\x1b\x12\x83\x39"
      "\xb9\x7f\x94\x62\x0f\x37\xd7\x27\x5b\x0e\xb3\x67\xd6\xdf\xe1\x7d\x33\xcb"
      "\xd8\x3a\x88\x4a\xf0\x85\xd0\x6d\x93\xe8\xe4\x82\xd1\x12\x94\xa2\xa2\xcd"
      "\x56\x60\x54\xa2\xce\x9a\x2e\x5c\xe5\xc0\x56\xc7\x6c\xe8\x0e\x05\x94\x22"
      "\x47\xf1\xcf\x5c\xb6\xba\x22\x44\x64\xf4\x23\xc8\xde\x23\xa5\xc4\xf3\x4b"
      "\x9a\x36\x29\xc5\x12\x05\x19\xe8\x7e\x61\x7f\x2a\x8c\xb7\xf5\x0e\xb9\xe0"
      "\x0e\x17\x5f\xde\xa5\x90\xe5\x0d\x87\xd3\xee\x72\x96\xb6\x51\xfe\x62\x8e"
      "\x93\x7a\xe6\x58\xeb\x32\x5d\xd9\xab\x52\xae\x1f\xff\x67\x95\x27\x40\x3a"
      "\xc1\x01\xb2\xc5\xf9\xf0\x0e\xdd\xa4\x6a\xd5\xbc\x4c\xd2\x90\xd7\x5d\xe1"
      "\xe2\xc9\xc3\x22\x50\x1f\x58\xbd\xd6\xab\xa1\xaa\x24\x63\xa1\xc8\xd8\x0d"
      "\xba\x21\xb3\xf0\xc3\xc3\xd1\x73\xae\xbe\xa2\x3e\x8f\x10\x41\x66\xef\x29"
      "\x9d\xd4\x67\x77\xe7\xcf\x01\x20\x9d\x7a\xd4\x39\x68\x26\x93\xe5\xcb\xa9"
      "\xc2\x92\x5c\x99\x1b\xac\x6c\xc2\x55\x74\xc6\xb9\x51\x21\xff\x2d\xff\x35"
      "\xc7\x8b\x27\xb1\xdf\x0e\x9a\x15\xa6\xea\xc4\x73\x91\x60\x1d\x7f\xe7\xc1"
      "\x39\xba\xa4\xc9\xbb\xc4\xeb\x99\x40\x9a\x61\xf1\x8b\xdc\x04\x16\x50\xc1"
      "\x8f\x4f\x04\x01\xcd\x8b\xca\xc7\x80\x43\x86\xfc\x80\x7e\x63\x9f\x09\x8e"
      "\x56\x78\x21\x81\xb9\x5c\x9c\x20\x13\x24\xf5\x27\x74\x15\x11\x2d\x9c\x48"
      "\x2f\x0c\x23\xa1\xf6\xe1\x32\xe3\x36\x7c\xa9\x19\x9f\x98\xbd\x4d\xb3\xbb"
      "\xd1\x92\xf2\x89\x70\x68\x61\x2b\xb0\x80\xc7\x6a\x33\x31\xcf\x22\xac\x0a"
      "\x91\xe2\x15\xf0\xae\x5f\xbd\x51\x99\xf7\x63\x01\xfd\x4e\x39\x7f\x59\xf2"
      "\x2f\x51\xd9\x87\x29\x5c\x1d\x1b\xb1\xc5\xb7\x55\xeb\x52\xe3\xfa\x32\x82"
      "\xed\x85\x30\x96\x3b\x64\x56\xa2\x17\x12\xeb\xdc\x27\x54\x68\xd6\x8d\xb0"
      "\xcf\x33\x7d\xea\x45\x3a\x3f\x2f\x87\x77\x63\xf7\x91\x05\x5f\x9a\x8c\x56"
      "\xa3\xd4\x8a\x43\x70\xd7\x17\x66\x67\x5b\xc3\x07\xc1\x47\x5e\xb6\x1f\x91"
      "\xe0\x3d\x5d\x8f\xf9\x84\x3c\xd2\xfe\xcf\x3b\xf0\xa2\xce\xe9\x89\xd0\xad"
      "\xa9\xa0\x82\x73\x69\xfa\xb9\x6c\x72\x8f\x5b\x54\xb1\x2c\xd9\x6f\xe9\x57"
      "\x46\x3a\xb2\xab\xd3\x51\xe4\x9e\xd2\xa7\xef\x98\xa5\x69\xa5\xf6\xc4\xa2"
      "\x96\x33\xd1\x4a\x4d\x34\xb7\xb6\x28\xc5\x58\x45\x9e\x67\xf3\x68\x9b\xe9"
      "\x9f\xc1\x09\xc0\x4b\xba\x7f\x6e\xdc\x33\x56\x2f\xeb\xb6\xc8\xb2\x98\x41"
      "\x93\xf1\x7b\xbc\xde\x20\x33\x92\x1d\x48\x2b\x68\x71\x93\x2a\xd7\x7a\x0b"
      "\x84\x6e\xc9\xbc\x69\xac\x4c\x61\x6e\x10\x96\xd9\x48\x9c\x92\xb7\x70\xcb"
      "\x07\xd9\xa4\x27\x50\x3d\x2a\xa0\x07\x99\xe4\x84\x84\xff\xa6\xaa\x93\x68"
      "\xd9\x29\x26\xcd\x9c\x35\xbe\xf7\xd3\xf7\x07\x08\x6c\x1a\xbb\x98\x1e\x81"
      "\x13\xab\x90\x21\x06\x47\xd7\x69\x65\xed\x13\x63\x86\x52\xe9\x37\xca\x1a"
      "\xc6\x89\x34\x53\x44\x07\x37\x4a\xc4\xd9\x63\x1d\x30\x1d\x81\x92\xb0\x49"
      "\xb6\x30\x19\x8a\x5f\x8d\xef\x00\x3b\xff\x98\xe5\x91\x8f\x8f\x74\x9c\xd3"
      "\x80\x03\xf0\x80\x6d\x15\x18\x0d\x77\xeb\x44\xb9\xdf\x42\x3c\x7b\xf6\x36"
      "\x70\xbb\x0c\xb4\xf3\xd4\xb1\xb2\x22\x4d\x34\x1c\x99\xaa\xe9\x61\x62\x15"
      "\x54\xae\x9b\x07\xc8\xad\x0b\x4e\xfa\xbd\x2b\x5b\xa7\x5e\x3e\xeb\x0c\x71"
      "\xbc\x38\x79\x5e\x9e\xdc\xb9\x85\x3c\x32\xe0\xb7\xaf\x49\x08\x01\xad\x8e"
      "\x1a\xb5\xbb\x30\x74\x71\xe5\xff\xb1\x0a\x6b\xd4\x1d\xef\xcf\x97\x84\xc4"
      "\xa2\x46\x30\xf2\x8d\x60\xca\xd2\xb8\x79\xc6\x13\xe1\xb1\x99\x58\x2e\x9f"
      "\xe6\xd2\x7f\x3a\x9d\xa6\x07\x9a\x06\x48\x8d\x86\xa6\x3b\x0f\x61\x8a\x43"
      "\x88\xf3\xa7\x76\x6d\xab\x6a\x6f\x1e\x21\x37\xb5\x40\x11\x50\x9f\xa6\x8b"
      "\x76\x69\xb5\x41\xaf\x1d\xfa\x6e\x31\x74\xf8\x0d\x71\xc9\x3d\xd8\x0b\x12"
      "\xa7\x7c\x2b\x22\xe8\x9d\xf6\xd2\xd2\xe0\x47\x39\xb9\x59\x6c\xa4\x35\xad"
      "\x64\xdd\x8e\x5d\x4e\x6b\xe1\xa1\x5e\x29\xcd\x06\xcf\xab\x0f\xab\xb1\x85"
      "\xb8\x2d\x7e\x80\x14\xeb\x40\x2e\x96\xdb\x77\xba\xb9\x40\xc2\x11\xe4\x77"
      "\x5c\x09\x1b\x54\x45\xf2\x12\x91\x24\xdc\x7f\xb6\xae\xa2\xb3\x8b\x53\xb7"
      "\xc3\x95\xe1\x47\xcc\xbb\x50\xc4\x86\x9b\xea\xcd\x67\xba\x95\x51\x9e\x3b"
      "\xef\x4d\x6a\xca\x55\x0e\xb1\xd4\xa3\x00\x2d\xd7\x48\xc7\xf1\xb5\x31\x86"
      "\x56\xcf\xb7\xb6\x7f\x9a\x9a\x0d\xad\xd5\x5a\x2b\xa6\x16\x4b\xd2\xcc\xac"
      "\xe4\x58\x61\x21\x3c\xd3\x1f\x27\x05\x80\xd4\xce\x58\xc6\xfe\xaf\x9b\x97"
      "\xc0\x1f\x66\xee\xa6\xfe\x09\xc6\xfb\x45\xce\x54\xaf\x5a\xac\x81\xe5\x67"
      "\x47\x2a\x8c\x6d\xfd\x30\xb3\x8a\x2f\x9d\x80\x6f\xb5\xcd\x3c\xe0\xdf\x44"
      "\x66\x03\x31\x31\xc8\xa7\x0c\x9b\xc5\x4e\xd6\x3e\xba\x3a\x07\x9b\xd9\x15"
      "\xd1\xc4\xf3\x2a\xbe\x94\xe9\x1f\x65\x29\x6c\x45\xd0\x69\xe7\x6b\xf4\x0a"
      "\x2f\x46\x06\x4e\x2d\x42\x42\x5e\xee\x66\x51\x25\xd9\x6d\xc3\x04\xca\x08"
      "\xe5\xc6\x15\x86\xc2\xe2\x66\x0f\x7a\x54\xf9\xee\xcf\xe0\x56\xc3\x84\xba"
      "\x99\x5f\xf8\xe8\xf8\x11\x96\x61\x4a\xd3\xdd\x1b\xcb\x3f\x48\x23\x74\x10"
      "\x2a\xef\xb1\x7a\x23\x1b\xba\x8e\x07\x9b\xc8\x96\x82\xde\x6c\x8a\xa3\xb9"
      "\xd8\xec\x63\x8d\xa2\xe6\xa2\xec\xd2\xd4\x0a\xfa\x13\x6c\xc7\xd9\x6c\x5b"
      "\x66\x30\xf4\x94\x60\xd4\xb0\xff\xa1\x08\xe0\x2e\x1d\x0a\xab\x34\xfc\xa1"
      "\x31\x17\x09\x7a\x51\x46\xbd\xb7\x86\x3d\x69\x9e\xe8\x04\x24\x77\x5e\x1c"
      "\x37\xa0\x19\x31\xa5\x78\x20\x15\x58\xe2\x7c\xf6\x67\xf1\xa7\x0b\xe1\xe5"
      "\x8f\xc8\xdb\xf6\xe6\x4e\x4e\xda\x0d\x58\x7e\x37\x68\x07\x01\x6e\x95\x8e"
      "\xeb\xf0\x7e\x29\xc6\xbe\xbf\xec\xb5\x56\xa5\xcc\x57\x6e\xd6\x37\x38\x51"
      "\x19\xa3\x16\x49\x46\x75\x85\xc3\x26\x09\x5f\xcf\x9a\x9b\x58\x8d\x1a\x43"
      "\xb6\xd3\xe2\x84\xf9\xca\x0b\xd6\x1b\x8a\x59\xab\x8e\x85\x85\xc8\xe4\x0c"
      "\x0e\xd4\x68\x82\x29\x25\x52\x8f\xc0\x43\xff\xcc\x70\x00\x72\xa5\x4e\xaa"
      "\xf5\xf8\x51\xdd\xef\x75\xea\xa7\xe9\x48\x17\xbc\x84\x47\xfa\xc6\xef\xfb"
      "\x6f\x8a\xc7\xf4\x12\x28\x46\x57\x3e\xd2\x80\xc7\x3e\xc1\xf4\x6b\x71\xb0"
      "\x18\x00\x59\x2a\x03\x31\xa9\xd5\xa0\x81\x4a\xfb\xd7\x0d\x5e\x52\xcb\x62"
      "\xe0\x83\x8e\x22\xf8\xf6\x3e\x72\x2c\x6f\x11\x27\xe3\x13\xa4\x8a\x10\xca"
      "\x34\x6a\xba\x4b\x6a\x63\xbc\x16\x00\x65\xf8\xcf\x97\x7f\x62\xfc\x31\xd5"
      "\x83\x3e\xe3\xe3\x69\x78\x8d\x04\xcc\x54\xe6\x35\x3b\xae\x68\x01\x19\x0a"
      "\x31\xab\x54\xdf\x34\xf0\x13\x49\x22\xb2\x4d\x5c\x17\x93\xed\xba\xe3\x34"
      "\x84\xd6\xca\x85\x52\x2b\x4a\xa1\xe5\x00\x9f\x47\x94\x56\x98\xd3\x4c\x5b"
      "\x52\xf7\x0f\x63\x8c\x02\x88\xc3\x0f\xfb\x8f\x91\x50\x11\x1a\x14\x26\x0d"
      "\x15\x54\xee\x09\x80\x60\x6a\x5e\xd1\x8c\xba\x8d\x8f\xe1\xd4\xf5\x4f\xf5"
      "\x76\xc3\x87\xee\xf1\xa8\xfc\xb5\xfa\x5e\x85\xbf\xde\x36\x64\xe7\x88\x15"
      "\x59\xd5\xbc\x81\xcb\xf5\xe4\x80\xfe\x81\xfd\x99\xcd\x02\xd6\x72\xc2\x1c"
      "\x2a\x4a\xe3\x3e\x80\x96\x00\xf4\x17\x38\x4e\x7a\xc0\x22\x64\x7c\xe2\x24"
      "\x49\x6a\x07\x3c\x6e\xcc\x52\x60\x84\xa1\x28\xba\xc6\x48\xf8\xf8\x4f\xda"
      "\xd7\x96\xe0\xb2\xd4\x95\x5a\x71\xec\x43\x44\xf1\xca\x55\x9b\x28\xb7\xf1"
      "\x3f\xe8\xb7\xa3\xcd\xab\x08\x95\x8c\x36\x79\xb5\xf5\xfb\x15\xe6\x69\x37"
      "\x9a\x0e\x22\x74\x0b\x9b\xc3\x6a\xee\x45\x06\xd9\x6a\x40\xe7\xa5\xb0\x9a"
      "\x6a\x7a\x77\x14\x45\x27\x7e\xb3\x8b\x56\x56\x85\xe9\x91\x25\x64\x7a\x5c"
      "\x82\x7f\xed\x10\x21\x0e\x91\xae\xc3\x7b\x52\x0c\xb3\x77\x2b\x14\x45\x6c"
      "\x31\x1d\x66\xfa\xb8\x90\x89\x24\x9e\xc3\xb1\xd9\x7a\xca\x93\x8b\xaa\x20"
      "\x90\x1b\xf5\xe9\x67\x50\x28\x39\xe1\x75\x6f\xc1\x5f\x16\x7c\x41\xfc\x96"
      "\x43\x5b\x69\x1f\x11\xa5\x05\x0d\x5e\xd6\x4d\xec\xba\x33\x23\x2c\x19\xb0"
      "\x9d\x4a\x44\xfe\x2b\xc3\x74\x21\x0b\xa0\x65\x6a\xbb\x38\xa7\x8c\x3e\x90"
      "\xe4\xe3\xbc\x9c\xcb\xed\x7f\x58\x41\xd0\xe6\x84\x0b\xb9\x81\x02\x8b\x3d"
      "\x2f\xe1\x9b\x1c\xf0\xb7\xa0\x03\x3e\xd7\xe9\x75\x46\x92\x7a\xf3\x6e\x76"
      "\xdc\xf2\xe1\x49\x97\x92\xe8\x05\x15\xc4\x8f\x66\xb9\xec\xef\xd2\xca\xc5"
      "\xbb\x38\x15\xfb\xdd\x9c\xb0\xe9\x00\x90\xb4\xfb\x23\x2f\xb0\xd7\x7d\x28"
      "\xb8\x7f\xd3\xb0\x88\x4d\xdd\x68\x9a\x24\x6c\x76\x55\x85\x1a\xd9\x53\x0a"
      "\xf4\x03\x3a\xa1\x4d\x84\x33\xd5\xbe\xf9\x6b\x66\x86\xb8\x82\xa1\x3e\xdf"
      "\x47\x9e\xb4\xe5\xef\x5e\xe9\x3d\x9d\x5c\xe0\x82\xd2\x2f\xe1\x48\x7e\x62"
      "\xe6\x76\xdd\xf5\x3e\x5d\x6f\x24\x19\xdf\xcd\xc4\x88\x93\x59\x0c\x45\x5b"
      "\x5b\xee\xfa\xc0\x17\x81\x44\xa5\x37\x1b\xf8\x09\x88\xbd\xac\x4d\xc5\xe0"
      "\x70\xbc\x71\x86\x05\xd0\xbf\xd4\xac\x03\x7c\x70\x63\x65\x5a\x51\x14\x4e"
      "\x89\xcd\xeb\x42\xc3\xec\xdc\x92\x0e\x03\x53\x12\x25\x47\x04\x04\x55\x1a"
      "\x8a\x2f\x14\x1b\x66\x7d\xb6\x3d\xd7\x84\xeb\x8d\xbe\x20\x63\x41\xbf\xa5"
      "\xb3\xf0\x77\xcf\xed\x75\xb1\xa6\x11\x1d\xa5\x52\x26\x88\xc1\xb8\x40\x7f"
      "\x72\xa6\x6b\xe4\xdd\xb3\x1a\xe8\xb2\x63\x60\x2a\x39\x2e\x70\x48\x56\x1c"
      "\x23\x58\x22\xca\xb9\x55\x26\x0e\xd7\x18\xac\x1b\x6b\x99\xd5\x75\x84\x11"
      "\xf0\xfa\xa6\xe4\x48\xb5\x6b\x63\x6d\xe1\x54\x72\xa5\x6a\xc2\xef\x9c\xeb"
      "\x7a\x19\x42\xfb\x36\x82\x52\x2b\x65\x09\x04\x57\xe1\x1b\x20\x9a\x72\xae"
      "\xe4\x81\xbf\xfe\xa4\x2f\xa6\xae\x25\xd5\xc0\xf8\xf8\x13\x3c\x84\x71\x42"
      "\xfe\x15\x67\xa1\x39\xb4\x42\x72\xf2\x48\xbc\x4c\xec\x06\xf3\x33\x92\x3c"
      "\xd9\x05\xeb\xc8\x7b\x63\x42\xd9\x5e\xf7\x7c\xc1\x31\x81\xcb\x44\x8f\x9b"
      "\x3a\xbe\x68\xc5\x8f\xf4\x2e\xdc\x65\x07\xca\x0d\xee\xe2\x29\x0e\xa6\x74"
      "\x4a\x3a\x95\x41\xfd\x3a\xa4\xcd\x94\x4e\x24\x25\x71\x3a\x57\x8e\xb1\xad"
      "\x3d\x63\xe2\xe5\xf7\xda\x8e\x0c\x5f\x45\x97\xdc\xfb\xf7\x0f\x6b\xd2\xd9"
      "\xa2\x45\x4f\x72\x27\x49\x3a\xd8\xa4\xa9\x21\x19\x40\x41\x73\x8f\xfa\x1b"
      "\x19\x18\x41\xf5\x5d\x9a\x40\xcb\x18\xb8\xcd\xfc\xdd\x53\x07\x43\x17\x2f"
      "\xe7\x6f\xc0\x76\x11\x30\x34\x8d\x41\x77\xba\xc5\xf5\x93\xfb\xdc\xea\xd1"
      "\xb0\x5e\xb0\x16\x65\x8d\xd1\x5b\x79\xcb\x23\x40",
      3144);
  *(uint64_t*)0x200006c8 = 0xc48;
  syscall(__NR_writev, r[0], 0x200006c0, 1);
  *(uint8_t*)0x20001440 = 0;
  *(uint8_t*)0x20001441 = 0;
  *(uint8_t*)0x20001442 = 0;
  *(uint8_t*)0x20001443 = 0;
  *(uint8_t*)0x20001444 = 0;
  *(uint8_t*)0x20001445 = 0;
  *(uint8_t*)0x20001446 = 0;
  *(uint8_t*)0x20001447 = 0;
  *(uint8_t*)0x20001448 = 0;
  *(uint8_t*)0x20001449 = 0;
  *(uint8_t*)0x2000144a = 0;
  *(uint8_t*)0x2000144b = 0;
  *(uint16_t*)0x2000144c = htobe16(0x8847);
  STORE_BY_BITMASK(uint32_t, 0x2000144e, htobe32(0x10001), 0, 8);
  STORE_BY_BITMASK(uint32_t, 0x2000144e, htobe32(3), 8, 1);
  STORE_BY_BITMASK(uint32_t, 0x2000144e, htobe32(0x88), 9, 3);
  STORE_BY_BITMASK(uint32_t, 0x2000144e, htobe32(0), 12, 20);
  STORE_BY_BITMASK(uint8_t, 0x20001452, 0x17, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20001452, 4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x20001453, 1, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x20001453, 2, 2, 6);
  *(uint16_t*)0x20001454 = htobe16(0xa8);
  *(uint16_t*)0x20001456 = htobe16(0x65);
  *(uint16_t*)0x20001458 = htobe16(0x1e);
  *(uint8_t*)0x2000145a = 8;
  *(uint8_t*)0x2000145b = 0;
  *(uint16_t*)0x2000145c = 0;
  *(uint8_t*)0x2000145e = 0xac;
  *(uint8_t*)0x2000145f = 0x14;
  *(uint8_t*)0x20001460 = 0x14;
  *(uint8_t*)0x20001461 = 0xaa;
  *(uint8_t*)0x20001462 = 0xac;
  *(uint8_t*)0x20001463 = 0x14;
  *(uint8_t*)0x20001464 = 0x14;
  *(uint8_t*)0x20001465 = 0xaa;
  *(uint8_t*)0x20001466 = 7;
  *(uint8_t*)0x20001467 = 0x1f;
  *(uint8_t*)0x20001468 = 8;
  *(uint32_t*)0x20001469 = htobe32(0xe0000001);
  *(uint32_t*)0x2000146d = htobe32(0xe0000002);
  *(uint32_t*)0x20001471 = htobe32(0xe0000002);
  *(uint32_t*)0x20001475 = htobe32(0xe0000002);
  *(uint32_t*)0x20001479 = htobe32(0xe0000002);
  *(uint32_t*)0x2000147d = htobe32(0xe0000001);
  *(uint8_t*)0x20001481 = 0xac;
  *(uint8_t*)0x20001482 = 0x14;
  *(uint8_t*)0x20001483 = 0x14;
  *(uint8_t*)0x20001484 = 0xbb;
  *(uint8_t*)0x20001485 = 0x94;
  *(uint8_t*)0x20001486 = 6;
  *(uint32_t*)0x20001487 = htobe32(5);
  *(uint8_t*)0x2000148b = 0x83;
  *(uint8_t*)0x2000148c = 0x23;
  *(uint8_t*)0x2000148d = 1;
  *(uint32_t*)0x2000148e = htobe32(0xe0000002);
  *(uint32_t*)0x20001492 = htobe32(4);
  *(uint32_t*)0x20001496 = htobe32(0xffffff01);
  *(uint8_t*)0x2000149a = 0xac;
  *(uint8_t*)0x2000149b = 0x14;
  *(uint8_t*)0x2000149c = 0x14;
  *(uint8_t*)0x2000149d = 0xbb;
  *(uint32_t*)0x2000149e = htobe32(0xe0000001);
  *(uint8_t*)0x200014a2 = 0xac;
  *(uint8_t*)0x200014a3 = 0x14;
  *(uint8_t*)0x200014a4 = 0x14;
  *(uint8_t*)0x200014a5 = 0x1f;
  *(uint8_t*)0x200014a6 = 0xac;
  *(uint8_t*)0x200014a7 = 0x14;
  *(uint8_t*)0x200014a8 = 0x14;
  *(uint8_t*)0x200014a9 = 0xbb;
  *(uint8_t*)0x200014aa = 0xac;
  *(uint8_t*)0x200014ab = 0x14;
  *(uint8_t*)0x200014ac = 0x14;
  *(uint8_t*)0x200014ad = 0xbb;
  STORE_BY_BITMASK(uint16_t, 0x200014ae, 0, 0, 1);
  STORE_BY_BITMASK(uint16_t, 0x200014ae, 0, 1, 1);
  STORE_BY_BITMASK(uint16_t, 0x200014ae, 1, 2, 1);
  STORE_BY_BITMASK(uint16_t, 0x200014ae, 5, 3, 1);
  STORE_BY_BITMASK(uint16_t, 0x200014ae, 0, 4, 4);
  STORE_BY_BITMASK(uint16_t, 0x200014ae, 0xfff8, 8, 1);
  STORE_BY_BITMASK(uint16_t, 0x200014ae, 0, 9, 4);
  STORE_BY_BITMASK(uint16_t, 0x200014ae, 1, 13, 3);
  *(uint16_t*)0x200014b0 = htobe16(0x880b);
  *(uint16_t*)0x200014b2 = htobe16(0);
  *(uint16_t*)0x200014b4 = htobe16(0);
  *(uint16_t*)0x200014b6 = htobe16(0x40);
  *(uint16_t*)0x200014b8 = htobe16(5);
  STORE_BY_BITMASK(uint16_t, 0x200014ba, -1, 0, 1);
  STORE_BY_BITMASK(uint16_t, 0x200014ba, 0, 1, 1);
  STORE_BY_BITMASK(uint16_t, 0x200014ba, 0, 2, 1);
  STORE_BY_BITMASK(uint16_t, 0x200014ba, 0x80, 3, 1);
  STORE_BY_BITMASK(uint16_t, 0x200014ba, 0, 4, 9);
  STORE_BY_BITMASK(uint16_t, 0x200014ba, 0, 13, 3);
  *(uint16_t*)0x200014bc = htobe16(0x800);
  *(uint16_t*)0x200014be = htobe16(0xfffd);
  STORE_BY_BITMASK(uint16_t, 0x200014c0, 0xd38, 0, 1);
  STORE_BY_BITMASK(uint16_t, 0x200014c0, 0, 1, 1);
  STORE_BY_BITMASK(uint16_t, 0x200014c0, 6, 2, 1);
  STORE_BY_BITMASK(uint16_t, 0x200014c0, 2, 3, 1);
  STORE_BY_BITMASK(uint16_t, 0x200014c0, 0, 4, 9);
  STORE_BY_BITMASK(uint16_t, 0x200014c0, 0, 13, 3);
  *(uint16_t*)0x200014c2 = htobe16(0x86dd);
  *(uint16_t*)0x200014c4 = htobe16(3);
  *(uint16_t*)0x200014c6 = 8;
  *(uint16_t*)0x200014c8 = htobe16(0x88be);
  *(uint32_t*)0x200014ca = htobe32(1);
  STORE_BY_BITMASK(uint8_t, 0x200014ce, 0xdb, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x200014ce, 1, 4, 4);
  *(uint8_t*)0x200014cf = 5;
  STORE_BY_BITMASK(uint8_t, 0x200014d0, 0x8c, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x200014d0, 0, 2, 1);
  STORE_BY_BITMASK(uint8_t, 0x200014d0, 0xc4, 3, 2);
  STORE_BY_BITMASK(uint8_t, 0x200014d0, 2, 5, 3);
  *(uint8_t*)0x200014d1 = 3;
  *(uint32_t*)0x200014d2 = 1;
  *(uint32_t*)0x200014d6 = htobe32(5);
  *(uint16_t*)0x200014da = 8;
  *(uint16_t*)0x200014dc = htobe16(0x22eb);
  *(uint32_t*)0x200014de = htobe32(4);
  STORE_BY_BITMASK(uint8_t, 0x200014e2, 0xef, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x200014e2, 2, 4, 4);
  *(uint8_t*)0x200014e3 = 0;
  STORE_BY_BITMASK(uint8_t, 0x200014e4, 1, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x200014e4, 0, 2, 1);
  STORE_BY_BITMASK(uint8_t, 0x200014e4, 9, 3, 2);
  STORE_BY_BITMASK(uint8_t, 0x200014e4, 0, 5, 3);
  *(uint8_t*)0x200014e5 = 0;
  *(uint32_t*)0x200014e6 = 2;
  *(uint32_t*)0x200014ea = htobe32(0x20);
  *(uint16_t*)0x200014ee = htobe16(1);
  STORE_BY_BITMASK(uint8_t, 0x200014f0, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x200014f0, 7, 2, 5);
  STORE_BY_BITMASK(uint8_t, 0x200014f0, 0x3f, 7, 1);
  STORE_BY_BITMASK(uint8_t, 0x200014f1, -1, 0, 1);
  STORE_BY_BITMASK(uint8_t, 0x200014f1, 0x80, 1, 2);
  STORE_BY_BITMASK(uint8_t, 0x200014f1, 7, 3, 1);
  STORE_BY_BITMASK(uint8_t, 0x200014f1, 0xf9, 4, 1);
  *(uint16_t*)0x200014f2 = 8;
  *(uint16_t*)0x200014f4 = htobe16(0x6558);
  *(uint32_t*)0x200014f6 = htobe32(2);
  *(uint32_t*)0x20000140 = 1;
  *(uint32_t*)0x20000144 = 4;
  *(uint32_t*)0x20000148 = 0x20076e;
  *(uint32_t*)0x2000014c = 0;
  *(uint32_t*)0x20000150 = 0xb1e;
  *(uint32_t*)0x20000154 = 0xa3e;
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x20001452, 92);
  *(uint16_t*)0x2000145c = csum_inet_digest(&csum_1);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}