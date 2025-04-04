// https://syzkaller.appspot.com/bug?id=e6f8fe57cea144e72896950c7f7fd0c5cbd5afd5
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

static void reset_loop_device(const char* loopname)
{
  int loopfd = open(loopname, O_RDWR);
  if (loopfd == -1) {
    return;
  }
  if (ioctl(loopfd, LOOP_CLR_FD, 0)) {
  }
  close(loopfd);
}

static long syz_mount_image(volatile long fsarg, volatile long dir,
                            volatile long flags, volatile long optsarg,
                            volatile long change_dir,
                            volatile unsigned long size, volatile long image)
{
  unsigned char* data = (unsigned char*)image;
  int res = -1, err = 0, need_loop_device = !!size;
  char* mount_opts = (char*)optsarg;
  char* target = (char*)dir;
  char* fs = (char*)fsarg;
  char* source = NULL;
  char loopname[64];
  if (need_loop_device) {
    int loopfd;
    memset(loopname, 0, sizeof(loopname));
    snprintf(loopname, sizeof(loopname), "/dev/loop%llu", procid);
    if (setup_loop_device(data, size, loopname, &loopfd) == -1)
      return -1;
    close(loopfd);
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
  if (need_loop_device)
    reset_loop_device(loopname);
  errno = err;
  return res;
}

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  intptr_t res = 0;
  memcpy((void*)0x20000240, "bfs\000", 4);
  memcpy((void*)0x20000100, "./file0\000", 8);
  sprintf((char*)0x20000000, "%020llu", (long long)-1);
  *(uint64_t*)0x20000014 = -1;
  sprintf((char*)0x2000001c, "0x%016llx", (long long)0);
  *(uint16_t*)0x2000002e = -1;
  *(uint16_t*)0x20000030 = -1;
  memcpy((void*)0x20000032, "\x00\x10\x00\x00\x00\x00\x00\x00\x00\x03\x03\x00",
         12);
  *(uint16_t*)0x2000003e = 0;
  memcpy(
      (void*)0x20000140,
      "\x78\x9c\xec\xce\x31\x4e\x84\x40\x18\x05\xe0\x07\x1a\xd1\x06\x0f\x60\xe1"
      "\x0d\xb8\x83\x67\xb1\xb4\xa4\xc2\x58\x70\x1b\x2b\x7b\x4f\xe1\x11\x4c\x3c"
      "\x80\x85\xad\x0d\x9b\x85\x85\x50\x6c\xb5\xc5\x92\x6c\xbe\x2f\x99\xcc\xfc"
      "\x33\xf3\x92\xf7\xf5\xff\xf1\x90\x3a\x19\xee\x33\x1a\x56\xda\xae\xe8\x93"
      "\xb6\x7b\x7d\xf9\xcc\xe2\x26\x5c\x90\x32\x49\x95\xe4\x76\x3f\xd4\xd3\xdd"
      "\xef\xd3\xb4\x17\x87\xf7\xef\xbf\xb7\xe7\x79\xad\xa2\xd7\x5b\xf4\x05\x00"
      "\x00\x4e\x57\xa6\x49\x72\x37\x8f\x7d\x99\xa6\x79\x1f\x8f\x3f\xcb\x9f\xc7"
      "\xe4\xea\x68\xb8\x3a\x47\x43\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x6d"
      "\xed\x02\x00\x00\xff\xff\x8d\x43\x1f\xc3",
      172);
  syz_mount_image(/*fs=*/0x20000240, /*dir=*/0x20000100, /*flags=*/0,
                  /*opts=*/0x20000000, /*chdir=*/4, /*size=*/0xac,
                  /*img=*/0x20000140);
  memcpy((void*)0x20000400, "./file1\000", 8);
  syscall(__NR_mkdir, /*path=*/0x20000400ul, /*mode=*/0ul);
  memcpy((void*)0x20000300, "./bus\000", 6);
  syscall(__NR_mkdir, /*path=*/0x20000300ul, /*mode=*/0ul);
  memcpy((void*)0x20000400, "./file1/file0\000", 14);
  syscall(__NR_mkdir, /*path=*/0x20000400ul, /*mode=*/0ul);
  memcpy((void*)0x20000200, "./file0\000", 8);
  memcpy((void*)0x20000080, "overlay\000", 8);
  memcpy((void*)0x20002200, "workdir", 7);
  *(uint8_t*)0x20002207 = 0x3d;
  memcpy((void*)0x20002208, "./bus", 5);
  *(uint8_t*)0x2000220d = 0x2c;
  memcpy((void*)0x2000220e, "lowerdir", 8);
  *(uint8_t*)0x20002216 = 0x3d;
  memcpy((void*)0x20002217, "./file0", 7);
  *(uint8_t*)0x2000221e = 0x2c;
  memcpy((void*)0x2000221f, "upperdir", 8);
  *(uint8_t*)0x20002227 = 0x3d;
  memcpy((void*)0x20002228, "./file1/file0", 13);
  *(uint8_t*)0x20002235 = 0x2c;
  *(uint8_t*)0x20002236 = 0;
  syscall(__NR_mount, /*src=*/0ul, /*dst=*/0x20000200ul, /*type=*/0x20000080ul,
          /*flags=*/0ul, /*opts=*/0x20002200ul);
  memcpy((void*)0x20000040, "fuse\000", 5);
  memcpy((void*)0x200000c0, "./file0\000", 8);
  memcpy((void*)0x20000440, "fd", 2);
  *(uint8_t*)0x20000442 = 0x3d;
  sprintf((char*)0x20000443, "0x%016llx", (long long)-1);
  *(uint8_t*)0x20000455 = 0x2c;
  memcpy((void*)0x20000456, "rootmode", 8);
  *(uint8_t*)0x2000045e = 0x3d;
  sprintf((char*)0x2000045f, "%023llo", (long long)0x2000);
  *(uint8_t*)0x20000476 = 0x2c;
  memcpy((void*)0x20000477, "user_id", 7);
  *(uint8_t*)0x2000047e = 0x3d;
  sprintf((char*)0x2000047f, "%020llu", (long long)-1);
  *(uint8_t*)0x20000493 = 0x2c;
  memcpy((void*)0x20000494, "group_id", 8);
  *(uint8_t*)0x2000049c = 0x3d;
  sprintf((char*)0x2000049d, "%020llu", (long long)0xee01);
  *(uint8_t*)0x200004b1 = 0x2c;
  memcpy((void*)0x200004b2, "max_read", 8);
  *(uint8_t*)0x200004ba = 0x3d;
  sprintf((char*)0x200004bb, "0x%016llx", (long long)4);
  *(uint8_t*)0x200004cd = 0x2c;
  memcpy((void*)0x200004ce, "allow_other", 11);
  *(uint8_t*)0x200004d9 = 0x2c;
  memcpy((void*)0x200004da, "max_read", 8);
  *(uint8_t*)0x200004e2 = 0x3d;
  sprintf((char*)0x200004e3, "0x%016llx", (long long)0x10000);
  *(uint8_t*)0x200004f5 = 0x2c;
  memcpy((void*)0x200004f6, "subj_role", 9);
  *(uint8_t*)0x200004ff = 0x3d;
  memcpy((void*)0x20000500, ".,+:", 4);
  *(uint8_t*)0x20000504 = 0x2c;
  memcpy((void*)0x20000505, "smackfsroot", 11);
  *(uint8_t*)0x20000510 = 0x3d;
  memcpy((void*)0x20000511, "overlay\000", 8);
  *(uint8_t*)0x20000519 = 0x2c;
  memcpy((void*)0x2000051a, "seclabel", 8);
  *(uint8_t*)0x20000522 = 0x2c;
  memcpy((void*)0x20000523, "smackfsroot", 11);
  *(uint8_t*)0x2000052e = 0x3d;
  memcpy((void*)0x2000052f, "lowerdir", 8);
  *(uint8_t*)0x20000537 = 0x2c;
  memcpy((void*)0x20000538, "rootcontext", 11);
  *(uint8_t*)0x20000543 = 0x3d;
  memcpy((void*)0x20000544, "unconfined_u", 12);
  *(uint8_t*)0x20000550 = 0x2c;
  memcpy((void*)0x20000551, "subj_role", 9);
  *(uint8_t*)0x2000055a = 0x3d;
  *(uint8_t*)0x2000055b = 0x2c;
  memcpy((void*)0x2000055c, "smackfshat", 10);
  *(uint8_t*)0x20000566 = 0x3d;
  *(uint8_t*)0x20000567 = 0x2c;
  memcpy((void*)0x20000568, "obj_role", 8);
  *(uint8_t*)0x20000570 = 0x3d;
  memcpy((void*)0x20000571, "^/*", 3);
  *(uint8_t*)0x20000574 = 0x2c;
  *(uint8_t*)0x20000575 = 0;
  res = -1;
  res = syz_mount_image(
      /*fs=*/0x20000040, /*dir=*/0x200000c0,
      /*flags=MS_REC|MS_SYNCHRONOUS|MS_SILENT|MS_REMOUNT|0x82*/ 0xc0b2,
      /*opts=*/0x20000440, /*chdir=*/0, /*size=*/0, /*img=*/0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000340, "./file0\000", 8);
  syscall(__NR_openat, /*fd=*/r[0], /*file=*/0x20000340ul,
          /*flags=O_TRUNC|0x1000*/ 0x1200ul, /*mode=S_IXOTH*/ 1ul);
  return 0;
}
