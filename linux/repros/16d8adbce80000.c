// https://syzkaller.appspot.com/bug?id=9a53810ae0c5ff8fd34b57bb1ccac1e1bc29940d
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/loop.h>

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

static unsigned long long procid;

//% This code is derived from puff.{c,h}, found in the zlib development. The
//% original files come with the following copyright notice:

//% Copyright (C) 2002-2013 Mark Adler, all rights reserved
//% version 2.3, 21 Jan 2013
//% This software is provided 'as-is', without any express or implied
//% warranty.  In no event will the author be held liable for any damages
//% arising from the use of this software.
//% Permission is granted to anyone to use this software for any purpose,
//% including commercial applications, and to alter it and redistribute it
//% freely, subject to the following restrictions:
//% 1. The origin of this software must not be misrepresented; you must not
//%    claim that you wrote the original software. If you use this software
//%    in a product, an acknowledgment in the product documentation would be
//%    appreciated but is not required.
//% 2. Altered source versions must be plainly marked as such, and must not be
//%    misrepresented as being the original software.
//% 3. This notice may not be removed or altered from any source distribution.
//% Mark Adler    madler@alumni.caltech.edu

//% BEGIN CODE DERIVED FROM puff.{c,h}

#define MAXBITS 15
#define MAXLCODES 286
#define MAXDCODES 30
#define MAXCODES (MAXLCODES + MAXDCODES)
#define FIXLCODES 288

struct puff_state {
  unsigned char* out;
  unsigned long outlen;
  unsigned long outcnt;
  const unsigned char* in;
  unsigned long inlen;
  unsigned long incnt;
  int bitbuf;
  int bitcnt;
  jmp_buf env;
};
static int puff_bits(struct puff_state* s, int need)
{
  long val = s->bitbuf;
  while (s->bitcnt < need) {
    if (s->incnt == s->inlen)
      longjmp(s->env, 1);
    val |= (long)(s->in[s->incnt++]) << s->bitcnt;
    s->bitcnt += 8;
  }
  s->bitbuf = (int)(val >> need);
  s->bitcnt -= need;
  return (int)(val & ((1L << need) - 1));
}
static int puff_stored(struct puff_state* s)
{
  s->bitbuf = 0;
  s->bitcnt = 0;
  if (s->incnt + 4 > s->inlen)
    return 2;
  unsigned len = s->in[s->incnt++];
  len |= s->in[s->incnt++] << 8;
  if (s->in[s->incnt++] != (~len & 0xff) ||
      s->in[s->incnt++] != ((~len >> 8) & 0xff))
    return -2;
  if (s->incnt + len > s->inlen)
    return 2;
  if (s->outcnt + len > s->outlen)
    return 1;
  for (; len--; s->outcnt++, s->incnt++) {
    if (s->in[s->incnt])
      s->out[s->outcnt] = s->in[s->incnt];
  }
  return 0;
}
struct puff_huffman {
  short* count;
  short* symbol;
};
static int puff_decode(struct puff_state* s, const struct puff_huffman* h)
{
  int first = 0;
  int index = 0;
  int bitbuf = s->bitbuf;
  int left = s->bitcnt;
  int code = first = index = 0;
  int len = 1;
  short* next = h->count + 1;
  while (1) {
    while (left--) {
      code |= bitbuf & 1;
      bitbuf >>= 1;
      int count = *next++;
      if (code - count < first) {
        s->bitbuf = bitbuf;
        s->bitcnt = (s->bitcnt - len) & 7;
        return h->symbol[index + (code - first)];
      }
      index += count;
      first += count;
      first <<= 1;
      code <<= 1;
      len++;
    }
    left = (MAXBITS + 1) - len;
    if (left == 0)
      break;
    if (s->incnt == s->inlen)
      longjmp(s->env, 1);
    bitbuf = s->in[s->incnt++];
    if (left > 8)
      left = 8;
  }
  return -10;
}
static int puff_construct(struct puff_huffman* h, const short* length, int n)
{
  int len;
  for (len = 0; len <= MAXBITS; len++)
    h->count[len] = 0;
  int symbol;
  for (symbol = 0; symbol < n; symbol++)
    (h->count[length[symbol]])++;
  if (h->count[0] == n)
    return 0;
  int left = 1;
  for (len = 1; len <= MAXBITS; len++) {
    left <<= 1;
    left -= h->count[len];
    if (left < 0)
      return left;
  }
  short offs[MAXBITS + 1];
  offs[1] = 0;
  for (len = 1; len < MAXBITS; len++)
    offs[len + 1] = offs[len] + h->count[len];
  for (symbol = 0; symbol < n; symbol++)
    if (length[symbol] != 0)
      h->symbol[offs[length[symbol]]++] = symbol;
  return left;
}
static int puff_codes(struct puff_state* s, const struct puff_huffman* lencode,
                      const struct puff_huffman* distcode)
{
  static const short lens[29] = {3,  4,  5,  6,   7,   8,   9,   10,  11, 13,
                                 15, 17, 19, 23,  27,  31,  35,  43,  51, 59,
                                 67, 83, 99, 115, 131, 163, 195, 227, 258};
  static const short lext[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2,
                                 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
  static const short dists[30] = {
      1,    2,    3,    4,    5,    7,    9,    13,    17,    25,
      33,   49,   65,   97,   129,  193,  257,  385,   513,   769,
      1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
  static const short dext[30] = {0, 0, 0,  0,  1,  1,  2,  2,  3,  3,
                                 4, 4, 5,  5,  6,  6,  7,  7,  8,  8,
                                 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};
  int symbol;
  do {
    symbol = puff_decode(s, lencode);
    if (symbol < 0)
      return symbol;
    if (symbol < 256) {
      if (s->outcnt == s->outlen)
        return 1;
      if (symbol)
        s->out[s->outcnt] = symbol;
      s->outcnt++;
    } else if (symbol > 256) {
      symbol -= 257;
      if (symbol >= 29)
        return -10;
      int len = lens[symbol] + puff_bits(s, lext[symbol]);
      symbol = puff_decode(s, distcode);
      if (symbol < 0)
        return symbol;
      unsigned dist = dists[symbol] + puff_bits(s, dext[symbol]);
      if (dist > s->outcnt)
        return -11;
      if (s->outcnt + len > s->outlen)
        return 1;
      while (len--) {
        if (dist <= s->outcnt && s->out[s->outcnt - dist])
          s->out[s->outcnt] = s->out[s->outcnt - dist];
        s->outcnt++;
      }
    }
  } while (symbol != 256);
  return 0;
}
static int puff_fixed(struct puff_state* s)
{
  static int virgin = 1;
  static short lencnt[MAXBITS + 1], lensym[FIXLCODES];
  static short distcnt[MAXBITS + 1], distsym[MAXDCODES];
  static struct puff_huffman lencode, distcode;
  if (virgin) {
    lencode.count = lencnt;
    lencode.symbol = lensym;
    distcode.count = distcnt;
    distcode.symbol = distsym;
    short lengths[FIXLCODES];
    int symbol;
    for (symbol = 0; symbol < 144; symbol++)
      lengths[symbol] = 8;
    for (; symbol < 256; symbol++)
      lengths[symbol] = 9;
    for (; symbol < 280; symbol++)
      lengths[symbol] = 7;
    for (; symbol < FIXLCODES; symbol++)
      lengths[symbol] = 8;
    puff_construct(&lencode, lengths, FIXLCODES);
    for (symbol = 0; symbol < MAXDCODES; symbol++)
      lengths[symbol] = 5;
    puff_construct(&distcode, lengths, MAXDCODES);
    virgin = 0;
  }
  return puff_codes(s, &lencode, &distcode);
}
static int puff_dynamic(struct puff_state* s)
{
  static const short order[19] = {16, 17, 18, 0, 8,  7, 9,  6, 10, 5,
                                  11, 4,  12, 3, 13, 2, 14, 1, 15};
  int nlen = puff_bits(s, 5) + 257;
  int ndist = puff_bits(s, 5) + 1;
  int ncode = puff_bits(s, 4) + 4;
  if (nlen > MAXLCODES || ndist > MAXDCODES)
    return -3;
  short lengths[MAXCODES];
  int index;
  for (index = 0; index < ncode; index++)
    lengths[order[index]] = puff_bits(s, 3);
  for (; index < 19; index++)
    lengths[order[index]] = 0;
  short lencnt[MAXBITS + 1], lensym[MAXLCODES];
  struct puff_huffman lencode = {lencnt, lensym};
  int err = puff_construct(&lencode, lengths, 19);
  if (err != 0)
    return -4;
  index = 0;
  while (index < nlen + ndist) {
    int symbol;
    int len;
    symbol = puff_decode(s, &lencode);
    if (symbol < 0)
      return symbol;
    if (symbol < 16)
      lengths[index++] = symbol;
    else {
      len = 0;
      if (symbol == 16) {
        if (index == 0)
          return -5;
        len = lengths[index - 1];
        symbol = 3 + puff_bits(s, 2);
      } else if (symbol == 17)
        symbol = 3 + puff_bits(s, 3);
      else
        symbol = 11 + puff_bits(s, 7);
      if (index + symbol > nlen + ndist)
        return -6;
      while (symbol--)
        lengths[index++] = len;
    }
  }
  if (lengths[256] == 0)
    return -9;
  err = puff_construct(&lencode, lengths, nlen);
  if (err && (err < 0 || nlen != lencode.count[0] + lencode.count[1]))
    return -7;
  short distcnt[MAXBITS + 1], distsym[MAXDCODES];
  struct puff_huffman distcode = {distcnt, distsym};
  err = puff_construct(&distcode, lengths + nlen, ndist);
  if (err && (err < 0 || ndist != distcode.count[0] + distcode.count[1]))
    return -8;
  return puff_codes(s, &lencode, &distcode);
}
static int puff(unsigned char* dest, unsigned long* destlen,
                const unsigned char* source, unsigned long sourcelen)
{
  struct puff_state s = {
      .out = dest,
      .outlen = *destlen,
      .outcnt = 0,
      .in = source,
      .inlen = sourcelen,
      .incnt = 0,
      .bitbuf = 0,
      .bitcnt = 0,
  };
  int err;
  if (setjmp(s.env) != 0)
    err = 2;
  else {
    int last;
    do {
      last = puff_bits(&s, 1);
      int type = puff_bits(&s, 2);
      err = type == 0 ? puff_stored(&s)
                      : (type == 1 ? puff_fixed(&s)
                                   : (type == 2 ? puff_dynamic(&s) : -1));
      if (err != 0)
        break;
    } while (!last);
  }
  *destlen = s.outcnt;
  return err;
}

//% END CODE DERIVED FROM puff.{c,h}

#define ZLIB_HEADER_WIDTH 2

static int puff_zlib_to_file(const unsigned char* source,
                             unsigned long sourcelen, int dest_fd)
{
  if (sourcelen < ZLIB_HEADER_WIDTH)
    return 0;
  source += ZLIB_HEADER_WIDTH;
  sourcelen -= ZLIB_HEADER_WIDTH;
  const unsigned long max_destlen = 132 << 20;
  void* ret = mmap(0, max_destlen, PROT_WRITE | PROT_READ,
                   MAP_PRIVATE | MAP_ANON, -1, 0);
  if (ret == MAP_FAILED)
    return -1;
  unsigned char* dest = (unsigned char*)ret;
  unsigned long destlen = max_destlen;
  int err = puff(dest, &destlen, source, sourcelen);
  if (err) {
    munmap(dest, max_destlen);
    errno = -err;
    return -1;
  }
  if (write(dest_fd, dest, destlen) != (ssize_t)destlen) {
    munmap(dest, max_destlen);
    return -1;
  }
  return munmap(dest, max_destlen);
}

static int setup_loop_device(unsigned char* data, unsigned long size,
                             const char* loopname, int* loopfd_p)
{
  int err = 0, loopfd = -1;
  int memfd = syscall(__NR_memfd_create, "syzkaller", 0);
  if (memfd == -1) {
    err = errno;
    goto error;
  }
  if (puff_zlib_to_file(data, size, memfd)) {
    err = errno;
    goto error_close_memfd;
  }
  loopfd = open(loopname, O_RDWR);
  if (loopfd == -1) {
    err = errno;
    goto error_close_memfd;
  }
  if (ioctl(loopfd, LOOP_SET_FD, memfd)) {
    if (errno != EBUSY) {
      err = errno;
      goto error_close_loop;
    }
    ioctl(loopfd, LOOP_CLR_FD, 0);
    usleep(1000);
    if (ioctl(loopfd, LOOP_SET_FD, memfd)) {
      err = errno;
      goto error_close_loop;
    }
  }
  close(memfd);
  *loopfd_p = loopfd;
  return 0;

error_close_loop:
  close(loopfd);
error_close_memfd:
  close(memfd);
error:
  errno = err;
  return -1;
}

static long syz_mount_image(volatile long fsarg, volatile long dir,
                            volatile long flags, volatile long optsarg,
                            volatile long change_dir,
                            volatile unsigned long size, volatile long image)
{
  unsigned char* data = (unsigned char*)image;
  int res = -1, err = 0, loopfd = -1, need_loop_device = !!size;
  char* mount_opts = (char*)optsarg;
  char* target = (char*)dir;
  char* fs = (char*)fsarg;
  char* source = NULL;
  char loopname[64];
  if (need_loop_device) {
    memset(loopname, 0, sizeof(loopname));
    snprintf(loopname, sizeof(loopname), "/dev/loop%llu", procid);
    if (setup_loop_device(data, size, loopname, &loopfd) == -1)
      return -1;
    source = loopname;
  }
  mkdir(target, 0777);
  char opts[256];
  memset(opts, 0, sizeof(opts));
  if (strlen(mount_opts) > (sizeof(opts) - 32)) {
  }
  strncpy(opts, mount_opts, sizeof(opts) - 32);
  if (strcmp(fs, "iso9660") == 0) {
    flags |= MS_RDONLY;
  } else if (strncmp(fs, "ext", 3) == 0) {
    bool has_remount_ro = false;
    char* remount_ro_start = strstr(opts, "errors=remount-ro");
    if (remount_ro_start != NULL) {
      char after = *(remount_ro_start + strlen("errors=remount-ro"));
      char before = remount_ro_start == opts ? '\0' : *(remount_ro_start - 1);
      has_remount_ro = ((before == '\0' || before == ',') &&
                        (after == '\0' || after == ','));
    }
    if (strstr(opts, "errors=panic") || !has_remount_ro)
      strcat(opts, ",errors=continue");
  } else if (strcmp(fs, "xfs") == 0) {
    strcat(opts, ",nouuid");
  }
  res = mount(source, target, fs, flags, opts);
  if (res == -1) {
    err = errno;
    goto error_clear_loop;
  }
  res = open(target, O_RDONLY | O_DIRECTORY);
  if (res == -1) {
    err = errno;
    goto error_clear_loop;
  }
  if (change_dir) {
    res = chdir(target);
    if (res == -1) {
      err = errno;
    }
  }

error_clear_loop:
  if (need_loop_device) {
    ioctl(loopfd, LOOP_CLR_FD, 0);
    close(loopfd);
  }
  errno = err;
  return res;
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul, /*prot=*/7ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  intptr_t res = 0;
  memcpy((void*)0x20000080, "ext3\000", 5);
  memcpy((void*)0x20000480, "./file0\000", 8);
  memcpy((void*)0x20000140, "jqfmt=vfsold", 12);
  *(uint8_t*)0x2000014c = 0x2c;
  memcpy((void*)0x2000014d, "resgid", 6);
  *(uint8_t*)0x20000153 = 0x3d;
  sprintf((char*)0x20000154, "0x%016llx", (long long)0xee00);
  *(uint8_t*)0x20000166 = 0x2c;
  memcpy((void*)0x20000167, "bh", 2);
  *(uint8_t*)0x20000169 = 0x2c;
  memcpy((void*)0x2000016a, "noload", 6);
  *(uint8_t*)0x20000170 = 0x2c;
  memcpy((void*)0x20000171, "data_err=ignore", 15);
  *(uint8_t*)0x20000180 = 0x2c;
  memcpy((void*)0x20000181, "usrjquota=", 10);
  *(uint8_t*)0x2000018b = 0x2c;
  *(uint8_t*)0x2000018c = 0;
  memcpy(
      (void*)0x200004c0,
      "\x78\x9c\xec\xdc\xcb\x6f\x1b\x45\x18\x00\xf0\x6f\xed\x24\x7d\x93\x50\xca"
      "\xa3\xa5\x85\x40\x41\x44\x3c\x92\x26\x7d\xd0\x03\x17\x10\x48\x1c\x40\x42"
      "\x82\x43\x11\xa7\x90\xa4\x55\xa8\xdb\xa0\x26\x48\xb4\x8a\x20\x70\x08\x47"
      "\x54\x89\x3b\xe2\x88\xc4\x5f\xc0\x09\x2e\x08\x38\x21\x71\x85\x3b\xaa\x54"
      "\xa1\x5c\x5a\x38\x19\xad\xbd\x9b\xba\x89\x9d\xc6\x89\x53\x97\xec\xef\x27"
      "\x6d\x3b\xe3\x1d\x6b\xe6\xdb\xdd\xb1\x67\x67\xbc\x09\xa0\xb0\x06\xd3\x7f"
      "\x92\x88\xbd\x11\xf1\x47\x44\xf4\xd7\xb3\xb7\x17\x18\xac\xff\x77\x73\x69"
      "\x7e\xe2\x9f\xa5\xf9\x89\x24\xaa\xd5\xb7\xff\x4e\x6a\xe5\x6e\x2c\xcd\x4f"
      "\xe4\x45\xf3\xf7\xed\xa9\x67\xaa\xd5\x2c\xbf\xa3\x49\xbd\x8b\xef\x45\x8c"
      "\x57\x2a\x53\x97\xb2\xfc\xc8\xdc\x85\x0f\x47\x66\x2f\x5f\x79\x61\xfa\xc2"
      "\xf8\xb9\xa9\x73\x53\x17\xc7\x4e\x9f\x3e\x71\xfc\x48\xdf\xa9\xb1\x93\x1d"
      "\x89\x33\x8d\xeb\xc6\xa1\x4f\x66\x0e\x1f\x7c\xfd\xdd\xab\x6f\x4e\x9c\xb9"
      "\xfa\xfe\x2f\xdf\xa5\xed\xdd\x9b\xed\x6f\x8c\xa3\x53\x06\xeb\x47\xb7\xa9"
      "\xa7\x3b\x5d\x59\x97\xed\x6b\x48\x27\x3d\x5d\x6c\x08\x6d\x29\x47\x44\x7a"
      "\xba\x7a\x6b\xfd\xbf\x3f\xca\xb1\x6b\x79\x5f\x7f\xbc\xf6\x79\x57\x1b\x07"
      "\x6c\xa9\x6a\xb5\x5a\x6d\xf6\xfd\x9c\x59\xa8\x02\xdb\x58\x12\xdd\x6e\x01"
      "\xd0\x1d\xf9\x17\x7d\x7a\xff\x9b\x6f\x77\x69\xe8\x71\x4f\xb8\xfe\x72\xfd"
      "\x06\x28\x8d\xfb\x66\xb6\xd5\xf7\xf4\x44\x29\x2b\xd3\xbb\xe2\xfe\xb6\x93"
      "\x06\x23\xe2\xcc\xc2\xbf\x5f\xa7\x5b\x6c\xd1\x3c\x04\x00\x40\xa3\x1f\xd2"
      "\xf1\xcf\xf3\xcd\xc6\x7f\xa5\x78\xa8\xa1\xdc\x7d\xd9\x1a\xca\x40\x44\xdc"
      "\x1f\x11\xfb\x23\xe2\x81\x88\x38\x10\x11\x0f\x46\xd4\xca\x3e\x1c\x11\x8f"
      "\xb4\x59\xff\xca\x15\x92\xd5\xe3\x9f\xd2\xb5\x0d\x05\xb6\x4e\xe9\xf8\xef"
      "\xa5\x6c\x6d\xeb\xf6\xf1\x5f\x3e\xfa\x8b\x81\x72\x96\xdb\x57\x8b\xbf\x37"
      "\x39\x3b\x5d\x99\x3a\x96\x1d\x93\xa1\xe8\xdd\x91\xe6\x47\xd7\xa8\xe3\xc7"
      "\x57\x7f\xff\xb2\xd5\xbe\xc6\xf1\x5f\xba\xa5\xf5\xe7\x63\xc1\xac\x1d\xd7"
      "\x7a\x56\x4c\xd0\x4d\x8e\xcf\x8d\x6f\x26\xe6\x46\xd7\x3f\x8b\x38\xd4\xd3"
      "\x2c\xfe\x24\xf2\x65\x9c\x24\x22\x0e\x46\xc4\xa1\x0d\xd6\x31\xfd\xec\xb7"
      "\x87\x5b\xed\xbb\x73\xfc\x6b\xe8\xc0\x3a\x53\xf5\x9b\x88\x67\xea\xe7\x7f"
      "\x21\x56\xc4\x9f\x4b\x5a\xae\x4f\x8e\xbe\x78\x6a\xec\xe4\xc8\xce\xa8\x4c"
      "\x1d\x1b\xc9\xaf\x8a\xd5\x7e\xfd\x6d\xf1\xad\x56\xf5\x6f\x2a\xfe\x0e\x48"
      "\xcf\xff\xee\xa6\xd7\xff\x72\xfc\x03\xc9\xce\x88\xd9\xcb\x57\xce\xd7\xd6"
      "\x6b\x67\xdb\xaf\x63\xf1\xcf\x2f\x5a\xde\xd3\x6c\xf4\xfa\xef\x4b\xde\xa9"
      "\xa5\xfb\xb2\xd7\x3e\x1e\x9f\x9b\xbb\x34\x1a\xd1\x97\xbc\xb1\xfa\xf5\xb1"
      "\x5b\xef\xcd\xf3\x79\xf9\x34\xfe\xa1\xa3\xcd\xfb\xff\xfe\xb8\x75\x24\x1e"
      "\x8d\x88\xf4\x22\x3e\x12\x11\x8f\x45\xc4\xe3\x59\xdb\x9f\x88\x88\x27\x23"
      "\xe2\xe8\x1a\xf1\xff\xfc\xca\x53\x1f\xb4\x1f\xff\x1a\xb3\xf2\x1d\x94\xc6"
      "\x3f\x79\xa7\xf3\x1f\x8d\xe7\xbf\xfd\x44\xf9\xfc\x4f\xdf\xb7\x1f\x7f\x2e"
      "\x3d\xff\x27\x6a\xa9\xa1\xec\x95\xf5\x7c\xfe\xad\xb7\x81\x9b\x39\x76\x00"
      "\x00\x00\xf0\x7f\x51\xaa\xfd\x06\x3e\x29\x0d\x2f\xa7\x4b\xa5\xe1\xe1\xfa"
      "\x6f\xf8\x0f\xc4\xee\x52\x65\x66\x76\xee\xb9\xb3\x33\x1f\x5d\x9c\xac\xff"
      "\x56\x7e\x20\x7a\x4b\xf9\x4c\x57\x7f\xc3\x7c\xe8\x68\x36\x37\x9c\xe7\xc7"
      "\x56\xe4\x8f\x67\xf3\xc6\x5f\x95\x77\xd5\xf2\xc3\x13\x33\x95\xc9\x6e\x07"
      "\x0f\x05\xb7\xa7\x45\xff\x4f\xfd\x55\xee\x76\xeb\x80\x2d\xe7\x79\x2d\x28"
      "\x2e\xfd\x1f\x8a\x4b\xff\x87\xe2\xd2\xff\xa1\xb8\xf4\x7f\x28\xae\x66\xfd"
      "\xff\xd3\x2e\xb4\x03\xb8\xfb\x7c\xff\x43\x71\xe9\xff\x50\x5c\xfa\x3f\x14"
      "\x97\xfe\x0f\x85\xd4\xf2\xd9\xf8\xd2\xa6\x1e\xf9\x97\xd8\xf6\x89\x28\xdd"
      "\x13\xcd\xd8\xfe\x89\x9e\x75\xff\x31\x8b\x0d\x26\x76\x34\xdd\xd5\xed\x4f"
      "\x26\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\xce\xf8\x2f\x00\x00"
      "\xff\xff\x70\x88\xe4\x87",
      1086);
  syz_mount_image(/*fs=*/0x20000080, /*dir=*/0x20000480, /*flags=*/0xc0ed0006,
                  /*opts=*/0x20000140, /*chdir=*/0xfe, /*size=*/0x43e,
                  /*img=*/0x200004c0);
  memcpy((void*)0x20000340, "./bus\000", 6);
  res = syscall(__NR_creat, /*file=*/0x20000340ul, /*mode=*/0ul);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20001280, "/dev/loop", 9);
  *(uint8_t*)0x20001289 = 0x30;
  *(uint8_t*)0x2000128a = 0;
  memcpy((void*)0x20000000, "./bus\000", 6);
  syscall(__NR_mount, /*src=*/0x20001280ul, /*dst=*/0x20000000ul, /*type=*/0ul,
          /*flags=*/0x1000ul, /*data=*/0ul);
  memcpy((void*)0x20000080, "./bus\000", 6);
  res = syscall(__NR_open, /*file=*/0x20000080ul, /*flags=*/0x185102ul,
                /*mode=*/0ul);
  if (res != -1)
    r[1] = res;
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0xb36000ul, /*prot=*/2ul,
          /*flags=*/0x28011ul, /*fd=*/r[1], /*offset=*/0ul);
  *(uint64_t*)0x20000900 = 0x200001c0;
  *(uint64_t*)0x200001c0 = 0;
  *(uint32_t*)0x200001c8 = 0;
  *(uint32_t*)0x200001cc = 0;
  *(uint16_t*)0x200001d0 = 0;
  *(uint16_t*)0x200001d2 = 0;
  *(uint32_t*)0x200001d4 = -1;
  *(uint64_t*)0x200001d8 = 0x200012c0;
  memcpy(
      (void*)0x200012c0,
      "\xe4\xe3\x1b\x1b\x51\x29\xdd\xae\x0b\xef\x73\xd1\x99\x36\x95\xf3\x21\xc5"
      "\x8e\x75\x7c\x45\xec\x11\xef\x95\x1b\xc6\x24\x06\xc2\xe9\xc9\x2d\xac\x5a"
      "\x3d\xb3\x3e\xf0\x16\x3b\xcd\x24\xf2\x92\xe3\xee\xaa\x51\x68\xa2\x54\x3f"
      "\x08\x68\x01\x91\x4b\xc0\xc6\x8e\x74\x87\xb0\xa5\x25\xd0\x3b\x25\xbf\xeb"
      "\xdd\x6d\xf5\x1c\xc0\x14\x7f\x2a\xb0\xcf\x6b\xfa\x4c\x41\x06\x53\x49\x10"
      "\xa0\xce\x0d\xaa\x53\xfb\xc1\x94\xe4\xc2\xc9\x8e\xe1\x13\xdf\x84\x99\x95"
      "\x3d\x53\xd8\x47\x42\x5c\x06\x4e\xe2\x7c\xb0\xb9\xb6\xa3\x5c\x92\x4b\xae"
      "\x75\x37\x3c\xd2\xed\xb6\x3c\x1f\x41\xcd\x1e\x19\x34\x0f\xab\x19\xe0\xeb"
      "\xb8\x8a\x0a\x0a\xc6\x4c\x28\x54\xd0\xff\xf4\xd6\x77\xb8\x6e\x5b\x0a\x38"
      "\x26\x8c\x41\x87\x7c\xc7\xef\x25\xcc\x31\x98\x98\xef\x10\x6c\x12\x54\xdd"
      "\x34\xda\x07\x7a\x58\xd4\xab\x71\x75\xf8\x53\xb5\x4c\xee\xfe\x42\xe4\x07"
      "\xa5\xe4\x9a\xe9\x2c\x29\x43\xa3\xb0\x30\x97\xa8\xbc\x26\x0c\x5a\x6a\x8c"
      "\x86\xc6\xe3\xc6\x89\xbb\xe0\xe0\x9a\x4e\x6a\x70\x59\x70\xb9\xba\x03\x55"
      "\x83\x64\xf8\x4e\xad\x50\x8e\x90\xf1\xc8\x2d\xd4\x39\x53\xb9\xf7\xb8\x65"
      "\x79\x60\xf6\xc5\xb0\x6f\x3b\xd1\xaf\xe5\x3f\xb5\xa1\x28\xe1\x13\x9a\x09"
      "\xee\x78\xb1\x0b\x6d\x60\x0d\xdf\x1e\xd4\x64\x33\x06\x88\xc7\x63\xf4\x4b"
      "\x30\xce\x90\x38\x5e\x6f\x56\x35\xcf\x28\x01\x3b\x67\x23\xa4\x0d\xf8\x4d"
      "\xd8\x4e\x13\xa0\x50\x0f\x72\x50\x5a\xc0\x2c\x19\x2f\x26\x7d\xd7\x4a\x4a"
      "\x63\x40\x6a\xbc\x3c\x0b\x0f\xe6\x1c\x32\xee\xc0\x41\x02\xcb\x3b\x2c\xd9"
      "\x51\x45\xa1\xc1\xd7\x7a\x99\x3d\x48\xd5\xb8\x71\x57\x8f\x72\x78\xb3\xaf"
      "\x42\x72\xc1\x5c\x55\x01\x4d\x99\xd5\xb2\xf4\x73\xb9\xb5\x1b\xc8\x9c\xb9"
      "\xd2\x9b\xea\x24\x2e\xda\xb0\x80\x84\x9a\x95\xed\xdf\x81\x4d\xba\x1c\xc6"
      "\x3f\x1e\xd2\xd9\x5d\x52\xb8\x8c\x65\x2d\xf5\xaa\xfd\x2e\x6d\x52\xe3\x34"
      "\x63\x2f\xdd\xd5\x0d\xa4\x1f\x76\x66\xff\xed\xde\x3b\xd3\x5d\x41\x5d\x08"
      "\xd3\xd5\xd3\x0e\x16\x04\xd7\xf6\x65\xac\x5f\x6d\x34\x76\x20\xa8\xe1\xae"
      "\xe1\x66\x24\x90\x50\xa0\x91\x2f\x93\x28\x38\xa1\x6a\x42\x54\x9b\x98\x27"
      "\xf4\xa5\x68\xeb\x26\x47\x16\xe0\x94\x55\xde\x73\x1c\x04\x77\xe7\x17\x1c"
      "\xe2\xb5\x83\x5d\x9b\x82\x70\x57\x82\x07\x17\xb1\x97\x04\x66\x2e\xad\x7a"
      "\x1a\xc1\x16\xef\x1d\xe5\x99\x18\x5b\x92\x49\xdf\x5b\x59\x01\x12\xdd\x4b"
      "\xfd\xd1\xaa\xe5\x0f\x4e\x6e\x2e\x5d\x7d\x6a\x68\x79\x47\x40\xc3\x19\xfb"
      "\xe9\x27\x6f\x73\x42\xf6\xca\xa4\xf4\xdf\x05\x5a\xf8\xc9\x22\x9e\x76\xef"
      "\x68\xe1\x2c\x9e\x24\xf7\x87\xcb\x1a\x05\xfc\x1b\xd7\x2c\xc6\xcb\x8f\x44"
      "\x6f\xdd\x9b\x2e\x0b\x5c\x02\x09\x04\x7c\x1d\xdb\x2e\x28\x15\x42\xb4\x60"
      "\x52\xd9\xce\xe9\x7b\xbf\x9f\x0e\x08\x4f\x14\x20\x90\xd3\xa3\x5c\x1e\x1c"
      "\x65\x0a\x87\x5d\x35\x15\x92\x70\xb0\x2e\x30\x6a\x9b\x47\xe6\xd3\xd4\x5e"
      "\x8a\xa2\x45\x5e\x38\xe6\x62\x17\xea\x16\xc6\x81\x0d\x32\x26\xf8\xa5\x31"
      "\x03\x69\xdd\xc4\x2e\xcd\x13\xd8\x2c\x18\x23\xd9\xd9\x9e\x55\xe1\x12\x99"
      "\x7b\xe2\x55\xb3\xa6\x82\x7f\x83\x23\xf4\xb1\x25\x19\xd2\x17\xf3\x4b\x2e"
      "\x67\x62\x5f\x13\xff\x48\xd0\x5a\xe6\x25\xc9\xb0\x0f\x8e\xac\x49\x7d\xb0"
      "\x3c\x1e\x9d\xb8\x98\x9c\x9d\xf8\x17\xb7\x7a\xfe\x41\x9c\xc0\x0d\xbd\x63"
      "\x04\xe8\xfc\xc4\xe2\xd5\x49\x06\x43\xee\xc2\xde\xd7\xad\xd8\xa7\x14\x42"
      "\x52\xd5\x46\x35\x72\x87\x04\x5b\x56\xc2\xfa\x74\x41\x96\xfe\x27\x0e\x3e"
      "\x34\x3f\x70\xb3\xb4\x78\x33\x12\x3b\xdf\xb3\x8c\x3f\x3d\x3e\xd9\x8c\xe2"
      "\xf5\x9d\xa0\xe7\xf2\xae\x88\x0d\xf6\x05\x2a\xaf\x6f\x7d\xba\x63\xd3\x5a"
      "\xe6\x89\xbc\x72\xc9\xfe\x9a\x5c\x3d\x6d\x70\x42\x84\x10\x4d\x13\x30\x6c"
      "\xed\x5d\x9f\x99\x58\x06\x1c\xfe\x1d\xd7\xff\x5f\x6b\x8b\x20\xdb\x90\xab"
      "\x59\x56\xcf\x02\x2f\x97\x0e\x7d\x68\x40\x85\x1e\x5b\x7f\x7f\xd0\x7b\x73"
      "\x1b\x1e\xd4\x61\xa5\xc9\x03\x44\xaf\xbb\x00\xbc\xe1\x96\x3c\x24\xee\xc6"
      "\xb2\xf0\x32\x02\xbf\x0e\x9f\x1c\xe8\x81\x21\xa2\x58\x02\x99\x42\xde\x11"
      "\xbc\x2c\xfb\x84\xde\xeb\x90\x3b\x9d\x5a\xd4\xa5\x7a\x61\x38\x2c\x7f\x44"
      "\x42\x2b\x94\x59\xcb\x46\x99\xa0\x09\x07\x24\xe3\x64\x9f\xcf\x20\xd2\x76"
      "\x08\x17\x82\x5e\x2a\x0a\xde\x5d\xf3\x90\xaf\x76\x63\x86\x19\x44\x5a\x91"
      "\xed\x21\x80\x98\x15\x43\x74\xe7\x85\x4a\xbf\x50\xcd\x03\x6a\xba\x93\xee"
      "\x6d\xe5\x6c\xaf\x38\x17\x84\x56\xd7\x43\xd2\x9a\xd5\x0c\x22\xd1\xea\x01"
      "\x4b\xca\x4a\x12\xd1\x92\x2b\x51\x52\xb8\xc0\xb4\x70\x4d\x9a\x2c\xc1\xee"
      "\xe8\x6e\xe4\x89\x83\x92\xb2\xc1\x56\x97\xc1\x5f\xb5\x01\xfd\xaa\x87\x36"
      "\xfa\xcd\xce\xb7\x02\x3c\xdf\x70\xb5\x72\xad\x39\xf7\x5d\xa4\x1e\x6c\x0b"
      "\xcc\x2d\xb5\x9a\x3e\x5e\x92\xf0\x4b\x90\x54\x98\xe7\x86\x61\x12\x4c\x54"
      "\xde\x63\xd9\x5b\x4a\xff\x54\x27\x24\xa9\xb5\xd7\xb3\x0c\x16\x94\x0d\x0b"
      "\x03\x64\x8a\x7a\xb1\xf4\x14\x34\x40\xee\xf8\xd5\x94\x64\xe8\xed\xfb\x74"
      "\x5b\x47\x4d\x82\xb2\x7d\x0c\x75\x0b\x93\xe6\x34\xf3\xe1\x14\xef\xe8\xbb"
      "\x5d\x01\x03\xdb\x5e\xe7\xb4\xc7\x9c\x74\xac\x16\xbb\xa0\xdf\x71\x6e\x9d"
      "\xb5\x14\x8c\x77\xb0\x1e\x41\x01\x22\x04\x56\x8d\x07\xfe\x8e\x8c\x7c\x4c"
      "\x73\x5c\xf5\x0f\x6e\x9c\x6a\x38\x55\xdf\xe0\x02\x7d\x0a\xca\xc8\x75\xdb"
      "\x38\xa8\xb6\xb1\x0b\x00\xde\xd7\xb4\xb2\xd9\x13\x49\x3c\x86\xa7\xff\x5c"
      "\x24\x07\x0f\x5d\x30\x90\x75\xa3\x73\x5f\x6c\x99\x4b\x94\xc1\x00\xcd\xdb"
      "\x7c\xd5\x80\x0c\x93\x9f\x39\x58\x32\xf4\x9f\xa8\xf1\x02\x50\x32\x70\x95"
      "\x04\xfa\x9f\xf5\x71\x94\xb8\xa5\x17\x18\x43\xa4\x67\x24\x5b\x54\x60\x9f"
      "\xd0\x26\x5f\x1e\x13\x9d\x1a\xd3\x5e\x6b\x2a\xac\xa2\x5c\x3d\x1a\x6a\x5b"
      "\xb8\x4d\x19\x36\xe1\x10\x93\xde\xc4\x51\x4a\xf7\xea\xc3\x1e\x4e\x12\xc8"
      "\xf9\xd4\xc7\x14\x87\x08\x62\x5d\x30\x07\xa3\x68\x30\x3c\x42\x0a\xb4\xa3"
      "\x2c\x06\xba\x1e\xc1\x80\xdd\x1c\x0e\xf7\x64\x01\xa1\x14\x9b\x56\x71\xa3"
      "\x66\x3d\x66\xb3\x52\x7f\x4a\x4e\xec\x3e\x39\xc8\x40\x90\xb7\x85\x3e\xfa"
      "\xaf\xea\x4c\xf5\x1b\x54\x59\x8b\x9c\x15\x34\xe2\x0d\xba\xc0\xb0\xd1\xf9"
      "\x6a\x57\x11\x47\x53\x4c\x58\x89\xa2\xed\xdc\x54\x30\x4a",
      1346);
  *(uint64_t*)0x200001e0 = 0x542;
  *(uint64_t*)0x200001e8 = 0;
  *(uint64_t*)0x200001f0 = 0;
  *(uint32_t*)0x200001f8 = 0;
  *(uint32_t*)0x200001fc = -1;
  syscall(__NR_io_submit, /*ctx=*/0ul, /*nr=*/1ul, /*iocbpp=*/0x20000900ul);
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x6611, /*arg=*/0ul);
  return 0;
}