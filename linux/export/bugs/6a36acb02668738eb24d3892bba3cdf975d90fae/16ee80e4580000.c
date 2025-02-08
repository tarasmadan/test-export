// https://syzkaller.appspot.com/bug?id=6a36acb02668738eb24d3892bba3cdf975d90fae
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
  syscall(__NR_mmap, /*addr=*/0x1ffffffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200000000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200001000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  const char* reason;
  (void)reason;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  memcpy((void*)0x200000000080, "ext4\000", 5);
  memcpy((void*)0x200000000000, "./file2\000", 8);
  *(uint8_t*)0x2000000002c0 = 0;
  memcpy(
      (void*)0x200000000880,
      "\x78\x9c\xec\xdd\x5f\x6b\x1c\x6b\x19\x00\xf0\x67\x26\xbb\xe7\x34\x6d\x8e"
      "\x9b\xa3\x5e\xe8\x01\xdb\x62\x2b\x49\xd1\x6e\x92\xc6\xb6\xc1\x8b\x56\x41"
      "\xf4\xaa\xa0\xd6\xfb\x1a\x93\x6d\x08\xd9\x64\x4b\xb2\x69\x9b\x50\x34\xc5"
      "\x0f\x20\x88\xa8\xe0\x8d\x5e\x79\x23\xf8\x01\x04\xe9\x47\x10\xa1\x60\xef"
      "\x45\x45\x11\x6d\xf5\x42\xa1\x3a\xb2\xbb\xb3\x31\x4d\x77\x93\x94\x6e\x76"
      "\x31\xfb\xfb\xc1\x9b\xf9\xbb\xf3\x3c\x6f\x96\x79\x67\xde\x99\x61\x27\x80"
      "\xa1\x75\x3e\x22\x26\x23\x22\xcb\xb2\xec\x52\x44\x94\xf2\xf9\x69\x5e\x62"
      "\xa7\x55\x1a\xeb\xbd\x78\xfe\x68\xa1\x51\x92\xc8\xb2\xdb\x7f\x4d\x22\xc9"
      "\xe7\xb5\xb7\xf5\x6e\x3e\x3c\x93\x7f\xec\x54\x44\x7c\xed\xcb\x11\xdf\x4c"
      "\x5e\x8f\xbb\xb1\xb5\xbd\x32\x5f\xad\x56\xd6\xf3\xe9\xa9\xfa\x6a\xf2\x32"
      "\xcb\xb6\x2f\x2f\xaf\xce\x2f\x55\x96\x2a\x6b\xb3\xb3\x33\xd7\xe6\xae\xcf"
      "\x5d\x9d\x9b\xee\x49\x3d\xc7\x23\xe2\xc6\x17\xff\xf8\x83\xef\xfe\xec\x4b"
      "\x37\x7e\xf5\x99\x07\xbf\xbb\xf3\xe7\xc9\x6f\xb5\x2a\xd8\xb2\xb7\x1e\xbd"
      "\xd4\xaa\x7a\xb1\xf9\xbf\x68\x2b\x44\xc4\xfa\x71\x04\x1b\x90\x42\xb3\x86"
      "\x2d\x57\x07\x9c\x0b\x00\x00\x07\x6b\x9c\xef\x7f\x38\x22\x3e\x19\x11\x97"
      "\xa2\x14\x23\xcd\xb3\x39\x00\x00\x00\xe0\x24\xc9\x6e\x8e\xc5\xcb\xa4\x75"
      "\xff\x0f\x00\x00\x00\x38\x99\xd2\x88\x18\x8b\x24\x2d\xe7\xcf\xfb\x8e\x45"
      "\x9a\x96\xcb\xad\x67\x78\x3f\x1a\xa7\xd3\x6a\x6d\xa3\xfe\xe9\xac\xb4\x7b"
      "\xbd\x60\x3c\x8a\xe9\xdd\xe5\x6a\x65\x3a\x7f\x76\x60\x3c\x8a\x49\x63\x7a"
      "\x26\x7f\xc6\xb6\x3d\x7d\x65\xdf\xf4\x6c\x44\xbc\x1f\x11\xdf\x2f\x8d\x36"
      "\xa7\xcb\x0b\xb5\xea\xe2\x40\xaf\x7c\x00\x00\x00\xc0\xf0\x38\xb3\xaf\xff"
      "\xff\x8f\x52\xab\xff\x0f\x00\x00\x00\x9c\x30\xe3\x83\x4e\x00\x00\x00\x00"
      "\x38\x76\xfa\xff\x00\x00\x00\x70\xf2\xe9\xff\x03\x00\x00\xc0\x89\xf6\x95"
      "\x5b\xb7\x1a\x25\x6b\xbf\xff\x7a\xf1\xfe\xd6\xe6\x4a\xed\xfe\xe5\xc5\xca"
      "\xc6\x4a\x79\x75\x73\xa1\xbc\x50\x5b\xbf\x57\x5e\xaa\xd5\x96\x9a\xbf\xd9"
      "\xb7\x7a\xd8\xf6\xaa\xb5\xda\xbd\xcf\xc6\xda\xe6\xc3\xa9\x7a\x65\xa3\x3e"
      "\xb5\xb1\xb5\x7d\x67\xb5\xb6\xb9\x56\xbf\xb3\xfc\xca\x2b\xb0\x01\x00\x00"
      "\x80\x3e\x7a\xff\xdc\x93\x67\x49\x44\xec\x7c\x6e\x34\x8d\x88\x2c\xd9\xb3"
      "\xac\x18\x91\x8d\xec\x5d\xb9\xd0\xff\xfc\x80\xe3\x93\xbe\xc9\xca\x7f\x38"
      "\xbe\x3c\x80\xfe\x1b\x19\x74\x02\xc0\xc0\x38\xa5\x87\xe1\x55\x1c\x74\x02"
      "\xc0\xc0\x1d\xd6\x0e\x74\x7d\x78\xe7\xd7\xbd\xcf\x05\x00\x00\x38\x1e\x13"
      "\x1f\x7f\xf2\x2c\x5a\xf7\xff\x9b\xa5\xe1\x9d\x7c\x59\x32\xc8\xc4\x80\x63"
      "\x97\xdf\xff\x4f\xec\xeb\x30\x7c\xdc\xff\x87\xe1\xe5\xfe\x1f\x0c\xaf\xe2"
      "\x41\x67\x00\x3a\x05\x70\xe2\xa5\x47\xd8\xd5\xdf\xfe\xfe\x7f\x96\xbd\x51"
      "\x52\x00\x00\x40\xcf\x8d\x35\x4b\x92\x96\xf3\x7e\xc0\x58\xa4\x69\xb9\x1c"
      "\xf1\x5e\xf3\xb5\x00\xc5\xe4\xee\x72\xb5\x32\x1d\x11\x1f\x8a\x88\xdf\x96"
      "\x8a\xef\x36\xa6\x67\x9a\x9f\x4c\x5c\x1e\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x23\xca\xb2\x24\xb2\x2e"
      "\x46\x77\xd7\x01\x00\x00\x00\xfe\x9f\x45\xa4\x7f\x4a\xf2\xf7\x7f\x4d\x94"
      "\x2e\x8e\xed\xbf\x3e\xf0\x4e\xf2\xcf\x52\x73\x18\x11\x0f\x7e\x7c\xfb\x87"
      "\x0f\xe7\xeb\xf5\xf5\x99\xc6\xfc\xbf\xed\xce\xaf\xff\x28\x9f\x7f\xa5\xdf"
      "\x57\x2f\x00\x00\x00\x80\x4e\xda\xfd\xf4\x76\x3f\x1e\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x7a\xe9\xc5\xf3"
      "\x47\x0b\xed\xd2\xcf\xb8\x7f\xf9\x42\x44\x8c\x77\x8a\x5f\x88\x53\xcd\xe1"
      "\xa9\x28\x46\xc4\xe9\xbf\x27\x51\xd8\xf3\xb9\x24\x22\x46\x7a\x10\x7f\xe7"
      "\x71\x44\x7c\xac\x53\xfc\xa4\x91\x56\x8c\xe7\x59\xec\x8f\x9f\x46\xc4\xe8"
      "\x80\xe3\x9f\xe9\x41\x7c\x18\x66\x4f\x1a\xed\xcf\xe7\x3b\xed\x7f\x69\x9c"
      "\x6f\x0e\x3b\xef\x7f\x85\xbc\xbc\xad\xee\xed\x5f\xba\xdb\xfe\x8d\x74\x69"
      "\xff\xde\xeb\xb4\xc1\xf4\xf5\x59\x1f\x3c\xfd\xc5\x54\xd7\xf8\x8f\x23\x3e"
      "\x28\x74\x6e\x7f\xda\xf1\x93\x2e\xf1\x2f\x1c\xb1\x8e\xdf\xf8\xfa\xf6\x76"
      "\xb7\x65\xd9\x4f\x23\x26\x3a\x1e\x7f\x92\x57\x62\x4d\x25\x85\x7b\x53\x1b"
      "\x5b\xdb\x97\x97\x57\xe7\x97\x2a\x4b\x95\xb5\xd9\xd9\x99\x6b\x73\xd7\xe7"
      "\xae\xce\x4d\x4f\xdd\x5d\xae\x56\xf2\xbf\x1d\x63\x7c\xef\x13\xbf\xfc\xcf"
      "\x41\xf5\x3f\xdd\x25\xfe\xf8\x21\xf5\xbf\x78\xc4\xfa\xff\xfb\xe9\xc3\xe7"
      "\x1f\x69\x8d\x16\xf7\x2d\x2a\xc6\x4f\xb2\x6c\xf2\x42\xe7\xe3\x6f\xeb\xff"
      "\x1f\x37\xf7\xc7\x6f\x1f\xfb\x3e\x95\x7f\xdd\x8d\xe9\x89\xf6\xf8\x4e\x6b"
      "\x7c\xaf\xb3\x3f\xff\xcd\xd9\x73\x5d\x72\x1b\x7d\x1c\xb1\xd8\xa5\xfe\x87"
      "\x7d\xff\x93\x47\xac\xff\xa5\xaf\x7e\xe7\xf7\x47\x5c\x15\x00\xe8\x83\x8d"
      "\xad\xed\x95\xf9\x6a\xb5\xb2\xde\x71\xa4\x75\x9c\x3f\x78\x1d\x23\x46\x0e"
      "\x1b\x19\x8d\x3e\x06\x9d\x8f\x83\xd6\x69\x9f\xc4\xf6\x21\x9f\x6f\xe7\xa1"
      "\xfa\x58\xf7\x7f\x65\x3d\xdb\xe0\x00\x1b\x25\x00\x00\xe0\x58\xfc\xef\xa4"
      "\x7f\xd0\x99\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\xc0\xf0\x3a\xec\x67\xc0\xa2\x07\x3f\x27\xb6\x3f\xe6\xce\x60\xaa\x0a\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x70\xa0\xff\x06\x00\x00\xff\xff\xc4\xe9\xc9\x36",
      1275);
  syz_mount_image(
      /*fs=*/0x200000000080, /*dir=*/0x200000000000,
      /*flags=MS_LAZYTIME|MS_SYNCHRONOUS|MS_SILENT|MS_RDONLY|MS_NOSUID|0xc*/
      0x200801f, /*opts=*/0x2000000002c0, /*chdir=*/0xfe, /*size=*/0x4fb,
      /*img=*/0x200000000880);
  return 0;
}
