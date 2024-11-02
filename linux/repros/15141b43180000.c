// https://syzkaller.appspot.com/bug?id=57e9ad94cc82b757e68c62c3eef6e3d6e4485e59
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
  memcpy((void*)0x20000000, "nilfs2\000", 7);
  memcpy((void*)0x20000240, "./file0\000", 8);
  memcpy(
      (void*)0x20000680,
      "\x78\x9c\xec\xdd\x4d\x8c\x5b\x47\x01\x00\xe0\x79\xde\xf5\x26\x9b\xa4\xc4"
      "\x29\x09\x5d\x92\xd0\x26\xfc\xb4\xe5\xa7\xbb\xcd\x66\x09\x3f\x11\x24\x55"
      "\x73\x21\x6a\x2a\x6e\x95\x2a\x2e\x51\x9a\x96\x88\x34\x20\x52\x09\x5a\x55"
      "\x22\xc9\x89\x1b\xad\xaa\x70\xe5\x47\x9c\xca\xa1\x02\x84\xd4\x5e\x50\xd4"
      "\x13\x97\x4a\x34\x12\x97\x9e\x0a\x07\x0e\x44\x41\xaa\xc4\x01\x0a\x89\xab"
      "\xd8\x33\x5e\x7b\x62\xeb\xd9\xfb\x67\x7b\xfd\x7d\xd2\x78\x3c\x6f\x9e\x3d"
      "\xf3\x9e\x9f\x9f\x9f\xdf\x7b\x33\x13\x80\x89\x55\x69\x3c\x2e\x2d\xcd\x15"
      "\x21\x5c\x7d\xeb\xb5\x13\xff\x7c\xf0\x1f\xb3\x77\xa6\x1c\x6b\xcd\x51\x6b"
      "\x3c\x4e\xb7\xa5\xaa\x21\x84\x22\xa6\xa7\xb3\xf7\x7b\x7f\xaa\x19\xdf\xfa"
      "\xe0\xe5\x33\xdd\xe2\x22\x2c\x36\x1e\x53\x3a\x3c\x79\xb3\xf5\xda\xed\x21"
      "\x84\x4b\xe1\x40\xb8\x16\x6a\x61\xef\xd5\xeb\xaf\xbe\xb3\xf8\xc4\xa9\xcb"
      "\x27\xaf\x1c\x7c\xf7\xf5\xa3\x37\xd6\x67\xe9\x01\x00\x60\xb2\x7c\xfb\xda"
      "\xd1\xa5\x3d\x7f\xfb\xcb\xbe\x5d\x1f\xbe\x71\xff\xf1\xb0\xa5\x35\x3d\x1d"
      "\x9f\xd7\x62\x7a\x47\x3c\xee\x3f\x1e\x0f\xfc\xd3\xf1\x7f\x25\x74\xa6\x8b"
      "\xb6\xd0\x6e\x26\x9b\x6f\x3a\x86\x4a\x36\xdf\x54\x97\xf9\xda\xcb\xa9\x66"
      "\xf3\x4d\xf7\x28\x7f\x26\x7b\xdf\x6a\x2b\x7f\x5f\xc7\x7c\x5b\x4a\xca\x9f"
      "\x6a\x9b\xd6\x6d\xb9\x61\x9c\xa5\xed\xb8\x16\x8a\xca\x7c\x47\xba\x52\x99"
      "\x9f\x6f\xfe\x27\x0f\x8d\xff\xf5\x33\xc5\xfc\x85\x73\xe7\x9f\xbd\x38\xa4"
      "\x8a\x02\x6b\xee\xdf\x0f\x84\x10\x0e\x08\x83\x86\x7a\xbd\xfe\x93\xc6\x0a"
      "\x1c\x81\xba\x08\xc2\x4a\x43\x7d\xe7\xb0\xf7\x40\x00\x4d\xf9\xf5\xc2\xbb"
      "\x5c\xca\xcf\x2c\xac\x4e\xeb\xdd\xa6\xfb\x2b\xff\xe6\x63\x95\xee\xaf\x87"
      "\x35\xb0\xd1\xdb\xbf\xf2\xc7\xab\xfc\xdf\x5c\xb6\xc7\x61\xed\x6c\xd6\xad"
      "\x29\x2d\x57\xfa\x1e\xed\x88\xe9\xfc\x3a\x42\x7e\xff\xd2\xa0\xdf\xff\xf4"
      "\x7e\xf9\xf5\x88\x6a\x9f\xf5\xec\x75\x1d\x61\x5c\xae\x2f\xf4\xaa\xe7\xd4"
      "\x06\xd7\x63\xa5\x7a\xd5\x3f\xdf\x2e\x36\xab\x6f\xc4\x38\xad\x87\x6f\x66"
      "\xf9\xed\xdf\x9f\xfc\x33\x1d\x97\xcf\x18\xe8\xee\x3f\x13\x76\xfe\xff\xc0"
      "\x08\xd4\x61\x53\x87\xea\x08\xd4\x41\xe8\x3b\xd4\x87\xbd\x03\x02\x46\xd6"
      "\xf2\x7d\x73\x4d\xf5\x28\xe5\xe7\xf7\xf5\xe5\xf9\x5b\x4a\xf2\xb7\x96\xe4"
      "\xcf\x96\xe4\x6f\x2b\xc9\xdf\x5e\x92\x0f\x93\xec\x0f\x2f\xfc\x2c\xbc\x52"
      "\x2c\xff\xcf\xcf\xff\xd3\x0f\x7a\x3e\x2c\x9d\x67\xbb\x27\xc6\x1f\x1b\xb0"
      "\x3e\xf9\xf9\xc8\x41\xcb\xcf\xef\xfb\x1d\xd4\x6a\xcb\xcf\xef\x27\x86\x51"
      "\xf6\xe6\xe9\xa7\xce\x7e\xf5\x99\xa7\xaf\x37\xef\xff\x2f\x5a\xdb\xff\xed"
      "\xb8\xbd\x1f\x88\xe9\x5a\xfc\x6e\x5d\x8b\x33\xa4\xf3\x85\xf9\x79\xf5\xd6"
      "\xbd\xff\xb5\xce\x72\x2a\x3d\xe6\xbb\x37\xab\xcf\x3d\x77\xcd\x5f\x6f\x96"
      "\xb8\xbb\x73\xbe\x62\xf7\xf2\xfb\x84\xb6\xfd\xcc\x5d\xf5\x98\xeb\x7c\xdd"
      "\xce\x5e\xf3\xed\xef\x9c\xaf\x96\xcd\x37\x1b\xc3\xd6\xac\xbe\xf9\xf1\xc9"
      "\xb6\xec\x75\xe9\xf8\x23\xed\x57\xd3\xfa\x9a\xce\x96\xb7\x9a\x2d\xc7\x4c"
      "\x56\x8f\xb4\x5f\xd9\x15\xe3\xbc\x1e\xb0\x12\x69\x7b\xec\x75\xff\x7f\xda"
      "\x3e\xe7\x42\xb5\x78\xf6\xdc\xf9\xb3\x8f\xc6\x74\xda\x4e\xff\x3c\x55\xdd"
      "\x72\x67\xfa\xa1\xf6\x37\xfd\xed\xc6\xd4\x1d\x58\x9d\x7e\xdb\xff\xcc\x85"
      "\xce\xf6\x3f\x3b\x5a\xd3\xab\x95\xf6\xfd\xc2\xce\xe5\xe9\x45\xfb\x7e\xa1"
      "\x96\x4d\x5f\x6c\x26\x5b\x97\xc9\xd3\xf4\xc3\x31\x9d\x7e\xe7\xbe\x3b\x35"
      "\xdb\x98\x3e\x7f\xe6\xfb\xe7\x9f\x59\xeb\x85\x87\x09\x77\xf1\xc5\x97\xbe"
      "\x77\xfa\xfc\xf9\xb3\x3f\xf4\x24\x3d\x99\xb5\x5a\xd6\xff\xc9\xb1\xd1\xa8"
      "\x86\x27\x3d\x9f\x0c\x7b\xcf\x04\xac\xb7\x85\x17\x9e\xff\xc1\xc2\xc5\x17"
      "\x5f\x7a\xe4\xdc\xf3\xa7\x9f\x3b\xfb\xdc\xd9\x0b\x87\x8f\x1c\x39\xbc\xb8"
      "\x78\xe4\x6b\x87\x97\x16\x1a\xc7\xf5\x0b\xed\x47\xf7\xc0\x66\xb2\xfc\xa3"
      "\x3f\xec\x9a\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xfd\xfa\xd1\xc9\x13"
      "\xd7\xff\xfa\xf6\x57\xde\x6b\xb6\xff\x5f\x6e\xff\x97\xda\xff\xa7\x3b\x7f"
      "\x53\xfb\xff\x9f\x66\xed\xff\xf3\x76\xf2\xa9\x1d\x7c\x6a\x07\xb8\xab\x4b"
      "\x7e\x63\xdc\xbd\x37\x3b\xeb\x31\x93\xcd\x57\x8d\xe1\xe3\x59\x7d\x77\x67"
      "\xe5\xec\xc9\x5e\xf7\x89\x18\xb7\xc6\xf1\x8b\xed\xff\x53\x7b\xfb\xbc\x5f"
      "\xd7\x54\x9f\xfb\xb2\xe9\x79\xff\xbd\x69\xbe\xac\x3b\x81\xbb\xfa\x4b\x99"
      "\xc9\xfa\x20\xc9\xc7\x0b\xfc\x74\x8c\xaf\xc4\xf8\xd7\x01\x86\xa8\x98\xed"
      "\x3e\x39\xc6\x65\xfd\x5b\xa7\x6d\x3d\xf5\x4f\xa1\x5f\x8a\xf1\x94\x3e\xb7"
      "\xb4\x35\xa4\x7e\x4c\x52\xfb\xef\x5e\xfd\x3a\xa5\xfd\xff\xae\x0d\xa8\x23"
      "\x6b\x6f\x23\x9a\x13\x0e\x7b\x19\x81\xee\xfe\x35\xf2\xfd\x7f\xb7\x1d\x89"
      "\x0f\xbd\x2e\x13\x10\xac\xe7\x89\x0a\xf5\xba\x51\x3c\x80\xd1\x30\xec\xf1"
      "\x3f\xd3\x79\xcf\x14\x5f\xf8\xd3\xb7\xb6\xde\x09\x69\xb6\x9b\x8f\x75\xee"
      "\x2f\xf3\xfe\x4b\x61\x35\x46\x7d\xfc\x49\xe5\x6f\xae\xf1\x3f\x5b\xe3\xdf"
      "\xf5\xb5\xff\xeb\xd2\xbb\x7a\x47\x3f\xcf\xfd\x8f\xae\xf0\xdf\x5f\xdc\x78"
      "\xaf\xad\xd8\xb0\xb7\xdf\xfd\x6f\xbe\xfc\xa9\x1f\xe8\xdd\xe5\x65\xb6\xfb"
      "\x30\x96\x9f\x96\xff\xa1\xd0\x5f\xf9\xf5\x5f\x65\xe5\xe7\x17\x84\xfa\xf4"
      "\xbf\xac\xfc\x6d\x7d\x96\x7f\xb3\x9e\x95\xbf\x7f\x65\xe5\xff\x3f\x96\x9f"
      "\x56\xdb\xc3\x9f\xe9\xb3\xfc\xcb\xcd\x1a\x17\x95\xce\x7a\xe4\xe7\x8d\xd3"
      "\xf5\xbf\xfc\xbc\x71\x72\x2b\x5b\xfe\xd4\xb7\xe7\xc0\x9f\xff\x0a\x07\x6a"
      "\xbc\x1d\xcb\x87\x49\xd6\x7b\x9c\xd9\x7e\x47\xb0\x1d\x4d\xe3\x32\xfe\x6f"
      "\x2f\xf9\x7d\x18\x5f\x8e\xe9\xb4\x23\x4c\xf7\x39\xe4\xbf\xc8\x83\xd6\x3f"
      "\xdd\x5f\x91\x7e\x07\xf6\x64\xef\x5f\x94\xfc\xbe\x8d\xcb\x38\xc5\xbd\x4c"
      "\xfa\xf8\xbf\x5f\x8f\x71\xd9\xf7\x21\x8d\xff\x9b\xb6\xc7\x5a\x97\x74\xa5"
      "\x2d\x5d\xed\xb2\x6e\xc7\x7d\x5b\x81\xcd\xe6\xfd\x91\xbf\xfe\x37\x66\xe1"
      "\xd2\x08\xd4\x41\x18\xd1\x30\x3b\x02\x75\xe8\x0c\xf5\x7a\x7e\x42\x61\x63"
      "\x0d\xb5\x70\x86\xbe\xfe\x87\x7d\xf5\x79\xd8\xe5\x0f\x7b\xfd\x97\xc9\xc7"
      "\xff\xcd\x8f\xe1\xf3\xf1\x7f\x2b\xd9\x1f\x88\x7c\xfc\xdf\xfc\xf5\xf9\xf8"
      "\xbf\x79\x7e\x3e\xbe\x5e\x9e\x9f\x8f\xff\x9b\xaf\xcf\x7c\xfc\xdf\x3c\xff"
      "\xbe\xec\x7d\xf3\x33\xd8\x73\x25\xf9\x9f\x2c\xc9\xdf\x5b\x92\xbf\x6f\x39"
      "\x7f\xb6\x5b\xfe\xfe\x92\xd7\x7f\xaa\x24\xff\x60\x49\xfe\xfd\x25\xf9\x0f"
      "\x94\xe4\xdf\x5b\x92\x3f\x55\x92\xff\xd9\x92\xfc\xcf\x95\xe4\x3f\x58\x92"
      "\xff\x70\x49\xfe\xe7\x4b\xf2\x37\xbb\xd4\x1e\x65\x52\x97\x1f\x26\x59\xde"
      "\x3e\xcf\xf7\x1f\x26\x47\xba\xfe\xd3\xeb\xfb\xbf\xbb\x24\x1f\x18\x5f\x3f"
      "\x7f\xe3\xd0\xe3\x4f\xff\xfe\x3b\xb5\x66\xfb\xff\x99\xd6\xff\xb5\x74\x1d"
      "\xef\x78\x4c\x57\xe3\x7f\xe7\x1f\xc7\x74\x7e\xdd\x3b\xb4\xa5\xef\xe4\xbd"
      "\x1d\xd3\x7f\xcf\xf2\x47\xfd\x7c\x07\x4c\x92\xbc\xff\x8c\xfc\xf7\xfd\xa1"
      "\x92\x7c\x60\x7c\xa5\xfb\xbc\x7c\xbf\x61\x02\x15\xdd\x7b\xec\xe9\xb7\xdf"
      "\xaa\x5e\xc7\xf9\x8c\x97\x2f\xc4\xf8\x8b\x31\xfe\x52\x8c\x1f\x89\xf1\x7c"
      "\x8c\x17\x62\x7c\x28\xc6\x8b\x1b\x54\x3f\xd6\xc7\xe3\xbf\xfb\xe3\xd1\x57"
      "\x8a\xe5\xff\xfb\x3b\xb3\xfc\x7e\xef\x27\xcf\xdb\x03\xe5\xfd\x44\x1d\xee"
      "\xb3\x3e\xf9\xf9\x81\x41\xef\x67\xcf\xfb\xf1\x1b\xd4\x6a\xcb\x5f\x61\x73"
      "\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x80\xa1\xa9\x34\x1e\x97\x96\xe6\x8a\x10\xae\xbe"
      "\xf5\xda\x89\xa7\x4e\x9d\x5b\xb8\x33\xe5\x58\x6b\x8e\x5a\xe3\x71\xba\x2d"
      "\x55\x6d\xbd\x2e\x84\x47\x63\x3c\x15\xe3\x5f\xc6\x27\xb7\x3e\x78\xf9\x4c"
      "\x7b\x7c\x3b\xc6\x45\x58\x0c\x45\x28\x5a\xd3\xc3\x93\x37\x5b\x25\x6d\x0f"
      "\x21\x5c\x0a\x07\xc2\xb5\x50\x0b\x7b\xaf\x5e\x7f\xf5\x9d\xc5\x27\x4e\x5d"
      "\x3e\x79\xe5\xe0\xbb\xaf\x1f\xbd\xb1\x7e\x6b\x00\x00\x00\x00\x36\xbf\x8f"
      "\x02\x00\x00\xff\xff\xf0\xf9\x18\xba",
      2655);
  syz_mount_image(
      /*fs=*/0x20000000, /*dir=*/0x20000240,
      /*flags=MS_LAZYTIME|MS_RELATIME|MS_NODIRATIME|MS_NOATIME*/ 0x2200c00,
      /*opts=*/0x20000280, /*chdir=*/3, /*size=*/0xa5f, /*img=*/0x20000680);
  memcpy((void*)0x20000040, ".\000", 2);
  res = syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000040ul,
                /*flags=*/0ul, /*mode=*/0ul);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000180, "./file1\000", 8);
  memcpy((void*)0x20000640, "./bus\000", 6);
  syscall(__NR_linkat, /*oldfd=*/r[0], /*old=*/0x20000180ul, /*newfd=*/r[0],
          /*new=*/0x20000640ul, /*flags=*/0ul);
  return 0;
}