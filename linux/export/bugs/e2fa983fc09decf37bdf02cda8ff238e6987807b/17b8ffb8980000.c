// https://syzkaller.appspot.com/bug?id=e2fa983fc09decf37bdf02cda8ff238e6987807b
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

  memcpy((void*)0x20000040, "hfsplus\000", 8);
  memcpy((void*)0x20000080, "./bus\000", 6);
  memcpy(
      (void*)0x20000840,
      "\x78\x9c\xec\xdd\xcd\x6f\x1c\x67\x1d\x07\xf0\xef\x6c\xd6\x8e\x37\x95\x1c"
      "\xb7\x4d\xd3\x80\x2a\x35\x6a\xa4\x82\x88\x48\xec\x58\x29\x98\x4b\x03\x42"
      "\x28\x48\x15\x94\x70\xe0\x6c\x35\x4e\x63\x65\x93\x06\xdb\x45\x49\x85\x48"
      "\x78\x13\x57\x0e\xfd\x03\xca\x21\x5c\xe0\x44\xc5\x05\x09\x29\x52\x39\xc3"
      "\xad\x57\x9f\x50\x25\x24\x2e\xe5\x12\x7a\x60\xd0\xcc\xce\x6e\xd6\x89\x5f"
      "\x36\x75\x1c\xdb\xed\xe7\x13\xcd\x3e\xcf\x33\xcf\xcc\x33\xbf\xf9\xcd\xcb"
      "\xce\x6e\x64\x6d\x80\xcf\xad\xf3\x27\xd3\xbe\x9b\x22\xe7\x4f\xbe\x76\xa3"
      "\x6a\xaf\xde\x99\xed\xae\xde\x99\xbd\xda\xd4\x5f\x4c\x72\x30\x49\x2b\x69"
      "\xf7\x8a\x14\xd7\x92\xe2\x83\xe4\x5c\x7a\x53\xbe\x50\xcd\x6c\x86\x2b\x36"
      "\xda\xce\xbb\x8b\x73\x17\x3e\xfc\x78\xf5\xa3\x5e\xab\xdd\x4c\xf5\xf2\xad"
      "\xa4\xb3\xcd\xbd\xb8\xdd\x4c\x39\x9e\xe4\x40\x53\x3e\xae\xf1\xde\xd8\xf6"
      "\x78\xc5\x20\x33\x55\xc2\x4e\xf4\x13\x07\xbb\x6d\x2c\x49\xb9\xc6\x8f\x8f"
      "\xde\xef\x59\x4f\x79\x60\xa8\xb1\xe1\xf5\x0e\xec\x1f\x45\xef\x7d\xf3\x21"
      "\x53\xc9\xa1\x24\x13\xcd\x73\x40\xef\x5d\xb1\xf7\x20\xb0\xaf\xdd\xde\xed"
      "\x00\x00\x00\x00\xe0\x09\x38\x7c\xef\x56\x72\x23\x93\xbb\x1d\x07\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\xec\x27\xcd\xef\xff\x17\xcd\xd4\xea\xd7\x8f"
      "\xa7\xe8\xff\xfe\xff\x78\x33\x2f\x4d\x7d\x6f\x1a\x31\xb2\xbb\xad\x9d\x0e"
      "\x04\x00\x00\x00\x00\x00\x00\x00\x76\xde\x8b\xf7\xf2\x87\x0b\x65\x39\xd9"
      "\x6b\x15\x29\x8b\xfa\xff\xfc\x5f\xaa\x9b\x47\xea\xd7\xa7\xf2\x76\x96\xb3"
      "\x90\xa5\x9c\xca\x8d\xcc\x67\x25\x2b\x59\xca\x4c\x92\xa9\xa1\x81\xc6\x6f"
      "\xcc\xaf\xac\x2c\xcd\xf4\xd7\xfc\xa4\x2c\xcb\x0d\xd6\x3c\xb3\xee\x9a\x67"
      "\x46\x0c\xb8\xf3\x78\xf6\x1b\x00\x00\x00\x00\x00\x00\x00\x3e\x23\x5e\x6d"
      "\xca\x5f\xe4\x7c\x26\x77\x39\x16\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x58\xa3\x48\x0e\xf4\x8a\x7a\x3a\xd2\xaf\x4f\xa5\xd5\x4e\x32\x91\x64"
      "\xbc\x5a\xee\x76\xf2\x8f\x7e\x7d\x3f\xbb\xbb\xdb\x01\x00\x00\x00\xc0\x13"
      "\x70\xf8\x5e\xee\xe5\x46\x26\xfb\xed\xb2\xa8\x3f\xf3\x1f\xad\x3f\xf7\x4f"
      "\xe4\xed\x5c\xcb\x4a\x16\xb3\x92\x6e\x16\x72\xb1\xfe\x2e\xa0\xf7\xa9\xbf"
      "\xb5\x7a\x67\xb6\xbb\x7a\x67\xf6\x6a\x35\x3d\x3c\xee\x37\xff\x7d\xbf\xfe"
      "\xfe\xe4\x96\x61\xd4\x23\xa6\xf7\xdd\xc3\xfa\x5b\x3e\x56\x2f\xd1\xc9\xa5"
      "\x2c\xd6\x73\x4e\xe5\x8d\xbc\x95\x6e\x2e\xa6\x55\xaf\x59\x39\xd6\x8f\x67"
      "\xfd\xb8\x7e\x5e\xc5\x54\xbc\xda\x18\x31\x41\x17\x9b\xb2\xda\xf3\xdf\x36"
      "\xe5\xde\x30\x55\x67\x64\x6c\x90\x91\xe9\x26\xb6\x2a\x1b\x4f\x6f\x9e\x89"
      "\xe1\xa3\xf3\x29\xb6\x34\x93\xd6\xe0\x9b\x9f\x23\xdb\xcc\xf9\xad\x41\xad"
      "\xf8\x5f\x59\xf6\x6a\x87\xfa\x73\x92\xa7\xbe\xbb\x6e\xce\xcb\xc3\x43\x8d"
      "\xb1\x47\xda\x99\x6d\x79\x30\x13\x67\x86\xce\xbe\xa3\x9b\x67\x22\xf9\xd2"
      "\x9f\xff\xf8\xa3\xcb\xdd\x6b\x57\x2e\x5f\x5a\x3e\xb9\x77\x4e\xa3\x47\x70"
      "\xb0\xec\x1f\xa1\x87\x33\x31\x3b\x94\x89\xe7\x3f\xf3\x99\x18\x36\x9d\xb1"
      "\xb4\xf2\xdc\xa0\x7d\x3e\xdf\xc9\x0f\x73\x32\xc7\xf3\x7a\x96\xb2\x98\x9f"
      "\x64\x3e\x2b\x59\xc8\xf1\x7c\xbb\xae\xcd\x37\xe7\x73\xf5\x3a\xd5\x3f\x77"
      "\xd7\xcf\xd4\xb9\x35\xad\xd7\xb7\x8a\x64\xbc\x39\x2e\xbd\xbb\xe8\x08\x31"
      "\xfd\x33\x83\x98\x5e\xaa\xd7\x9d\xcc\x62\xbe\x97\xb7\x72\x31\x0b\x79\xa5"
      "\xfe\x77\x26\x33\xf9\x5a\xce\xe6\x6c\xe6\x86\x8e\xf0\x73\x23\x5c\xf5\xad"
      "\x47\xbb\xd3\x9e\xf8\x72\x53\xe9\x24\xf9\x4d\x53\xee\x0d\x55\x5e\x9f\x1e"
      "\xca\xeb\xf0\x3d\x77\xaa\xee\x1b\x9e\xd3\x4a\x79\xb0\xb7\xde\x33\x8f\xed"
      "\xfd\xe8\xfd\xef\xff\xe7\x07\x75\xa5\xfd\xc5\x66\x4e\x75\x24\x7e\xd9\x94"
      "\x7b\xc3\x83\x99\x98\x19\x3a\x5f\x9e\xdd\x3c\x13\xbf\xab\x6f\x2b\xcb\xdd"
      "\x6b\x57\x96\x2e\xcf\x5f\x1f\x71\x7b\x2f\x37\x65\x75\x1d\xfd\x7a\x4f\xbd"
      "\x33\x57\xe7\xcb\x33\xd5\xc1\xaa\x5b\x6b\xcf\x8e\xaa\xef\xd9\x75\xfb\x66"
      "\xea\xbe\x23\x83\xbe\xd6\x83\x7d\xbf\xef\x0c\xfa\xb6\xba\x52\xc7\x9b\x67"
      "\xb8\x87\x47\x3a\x53\xf7\x3d\xbf\x6e\xdf\x6c\xdd\x77\x6c\xa8\xaf\x7e\xde"
      "\xaa\x1b\x9f\x94\x65\xd9\x7b\xde\x02\x60\xcf\x3b\xf4\x95\x43\xe3\x9d\x7f"
      "\x75\xfe\xde\x79\xaf\xf3\xab\xce\xe5\xce\x6b\x13\xdf\x3a\xf8\xf5\x83\x2f"
      "\x8c\x67\xec\x6f\x63\xdf\x68\x4f\x1f\x78\xb9\xf5\x42\xf1\xa7\xbc\x97\x9f"
      "\x65\xeb\x4f\xe8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\xc0\x96\x96\x6f\xb6\x92\x74\x17\x96"
      "\x96\x6f\xbe\x73\x65\xbe\x3b\xa8\x94\x65\x79\x6b\xed\x9c\x9d\xad\xa4\x9d"
      "\xac\x99\xf3\xd7\xbf\x0c\x2d\x93\xe4\x76\x15\xed\xe8\x03\x56\x4b\x9f\x6b"
      "\x25\x4f\x24\xf8\xcf\x6d\x65\x22\xa3\x2e\xfc\xdf\xb2\x2c\xf7\x46\xcc\xa3"
      "\x54\xca\xc6\x5e\x89\x67\xf3\x4a\x59\x3c\x70\xed\x3c\x96\xca\x6e\xdf\x99"
      "\x80\x9d\x76\x7a\xe5\xea\xf5\xd3\xcb\x37\xdf\xf9\xea\xe2\xd5\xf9\x37\x17"
      "\xde\x5c\xb8\x36\x77\xf6\xec\xdc\xf4\xdc\xd9\x57\x66\x4f\x5f\x5a\xec\x2e"
      "\x4c\xf7\x5e\x77\x3b\x4a\x60\x27\x0c\x3d\x81\x03\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\xfb\xc4\x68\x7f\x9c\x53\x6c\xef\x6f\x7b\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xb6"
      "\xe1\xfc\xc9\xb4\xef\xa6\xc8\xcc\xf4\xa9\xe9\xaa\xbd\x7a\x67\xb6\x5b\x4d"
      "\xfd\xfa\xfd\x25\xdb\x49\x5a\x49\x8a\x9f\x26\xc5\x07\xc9\xb9\xf4\xa6\x4c"
      "\x0d\x0d\x57\x6c\xb4\x9d\x77\x17\xe7\x2e\x7c\xf8\xf1\xea\x47\xf7\xc7\x6a"
      "\xf7\x97\x6f\x6d\xb6\xde\x68\x6e\x37\x53\x8e\x27\x39\xd0\x94\xeb\x98\x78"
      "\xb4\xf1\x8a\x7a\x9c\xeb\x1b\x8f\x37\xa2\x62\xb0\x87\x55\xc2\x4e\xf4\x13"
      "\x07\xbb\xed\xff\x01\x00\x00\xff\xff\x7d\x4b\x1a\x32",
      1651);
  syz_mount_image(
      /*fs=*/0x20000040, /*dir=*/0x20000080,
      /*flags=MS_LAZYTIME|MS_SYNCHRONOUS|MS_RELATIME|MS_NODIRATIME|MS_MANDLOCK*/
      0x2200850, /*opts=*/0x200003c0, /*chdir=*/1, /*size=*/0x673,
      /*img=*/0x20000840);
  memcpy((void*)0x20000000, "./file1\000", 8);
  syscall(__NR_unlink, /*path=*/0x20000000ul);
  return 0;
}