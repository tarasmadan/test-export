// https://syzkaller.appspot.com/bug?id=0e77b741e3a1b835114f0f7f0f3e53e12c9a73bc
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
  const char* reason;
  (void)reason;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  memcpy((void*)0x20000100, "hfsplus\000", 8);
  memcpy((void*)0x20002900,
         "./"
         "file0aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\000",
         250);
  memcpy(
      (void*)0x20003480,
      "\x78\x9c\xec\xdd\xc1\x6f\x1c\x57\x1d\x07\xf0\xef\xac\xd7\x8e\x37\x54\xa9"
      "\xd3\x26\x34\x42\x45\x58\x89\x54\x90\x22\x12\x27\x56\x0a\xe1\x82\x41\x08"
      "\xe5\x50\xa1\xaa\x1c\x7a\xb6\x12\xa7\xb1\xba\x49\xaa\xc4\x45\x69\x85\xc0"
      "\x05\x04\x27\x24\x0e\xfd\x03\x0a\x92\x6f\x1c\x10\x12\xf7\xa0\x70\xe1\x52"
      "\x6e\xbd\xfa\x58\x09\x89\x4b\xc4\x21\xea\x65\xd1\xcc\xce\xda\xbb\xf6\xda"
      "\xde\x24\xf6\x3a\xa1\x9f\x4f\x34\x9e\xf7\xe6\xcd\xbc\xf9\xcd\x6f\xde\xcc"
      "\x78\xd7\x59\x6d\x80\x2f\xad\x2b\x67\xd3\xbc\x9f\x22\x57\xce\xbe\x71\xaf"
      "\xac\xaf\xaf\xcd\xb7\xd7\xd7\xe6\x8f\xd4\xcd\xed\x24\x65\xb9\x91\x34\xbb"
      "\xb3\x14\xb7\x92\xe2\x41\xb2\x50\xb6\x17\x7d\x53\xfa\xe6\xdb\x7c\xbc\x7c"
      "\xf9\xad\xcf\x1e\xae\x7f\xde\xad\x35\xeb\xa9\x5a\x7f\x62\x63\xbb\xe9\x91"
      "\x42\x1e\xb2\x8f\xd5\x7a\xca\x6c\xdd\xdf\xec\xd0\x2d\x27\x47\xea\xbf\xdb"
      "\x57\x15\x5e\x5e\x48\x72\xb5\x9e\x0f\x9a\x1a\xb5\xaf\x81\x15\xcb\xa4\x9d"
      "\xa9\xe7\x70\xe8\x3a\xdb\xac\x6e\x36\x1e\xd9\x73\xf3\x1d\xaf\x77\xe0\xd9"
      "\xd7\x7b\x3a\x15\xdd\xe7\xe6\x36\x33\xc9\xd1\xfa\xc9\x5c\xdd\x0d\xea\xbb"
      "\x43\x63\x7c\x11\x1e\x8c\xd5\xc3\x0e\x00\x00\x00\x00\xc6\xe0\xd3\xdb\x87"
      "\x1d\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x3c\x7f\xea\xef\xff\x2f\xea"
      "\xa9\x51\xcf\x33\x9b\xa2\xf7\xfd\xff\x53\xbd\x65\x75\xf9\x19\xb4\x30\xf2"
      "\x9a\xf7\x0f\x34\x0e\x00\x00\x00\x00\x00\x00\x00\x18\x8f\x6f\x3c\xca\xa3"
      "\xdc\xcb\xb1\x5e\xbd\x53\x54\x7f\xf3\x3f\x5d\x55\x4e\xe4\x8b\x4e\xf2\x95"
      "\xbc\x9f\xbb\x59\xca\x9d\x9c\xcb\xbd\x2c\x66\x25\x2b\xb9\x93\x0b\x49\x66"
      "\xfa\x3a\x9a\xba\xb7\xb8\xb2\x72\xe7\xc2\xc6\x96\xa5\xe1\x5b\x5e\x1c\xba"
      "\xe5\xc5\x71\x1d\x31\x00\x00\x00\x00\x00\x00\x00\xfc\x5f\xfa\x55\x5a\x9b"
      "\x7f\xff\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x67\x41\x91\x4c"
      "\x74\x67\xd5\x74\xa2\x9e\x67\x26\x8d\x66\x36\xdb\xb2\x9a\xfc\x2b\xc9\xd4"
      "\x61\xc7\xfb\x18\x8a\x61\x0b\xef\x8f\x3f\x0e\x00\x00\x00\x78\x2a\xd3\x4f"
      "\xb0\xcd\x8b\x8f\x5a\xc9\xbd\x1c\xeb\xd5\x3b\x45\xf5\x9a\xff\xab\xd5\xeb"
      "\xe5\xe9\xbc\x9f\x5b\x59\xc9\x72\x56\xd2\xce\x52\xae\xd5\xaf\xa1\xcb\x57"
      "\xfd\x8d\xf5\xb5\xf9\xf6\xfa\xda\xfc\xcd\x72\x2a\xeb\x83\xfd\xfe\xe0\x3f"
      "\x8f\x15\xc6\x54\xdd\xc3\x44\x55\x1b\xb6\xe7\x53\xd5\x1a\xad\x5c\xcf\x72"
      "\xb5\xe4\x5c\xae\x56\xc1\x5c\x4b\xa3\xbb\xef\x33\xc9\xa9\x5e\x3c\x7d\x71"
      "\xf5\xf9\xa8\x8c\xa9\xf8\x7e\x6d\xc4\xc8\x9a\x75\x5a\xcb\x9d\xfd\x61\xa7"
      "\x77\x11\xf6\xc5\xe0\x5b\x11\x8d\x5d\xd6\x6c\x6d\x06\x97\x6c\x64\x64\xae"
      "\x8e\xad\xdc\xf2\x78\x37\x03\x45\xf5\x46\x4d\xb2\x35\x13\x7b\x9e\x9d\xe6"
      "\x40\x6d\xa6\xea\x75\x72\x63\x4f\x17\xd2\xd8\x78\xe7\xe7\xc4\x01\xe4\xfc"
      "\x68\x3d\x2f\x8f\xe7\xb7\x07\x9a\xf3\xc7\xb5\x91\x89\x46\xaa\x4c\x5c\xec"
      "\x8d\xbe\xf2\x9a\xd9\x3d\x13\xc9\x37\xff\xf6\xe7\xb7\x6f\xb4\x6f\xbd\x7b"
      "\xe3\xfa\xdd\xb3\xcf\xce\x21\xed\x61\x62\x87\xe5\x5b\xc7\xc4\x7c\x5f\x26"
      "\x5e\x79\xae\x33\xd1\x7c\xcc\xf5\xe7\xaa\x4c\x9c\xdc\xa8\x5f\xc9\x8f\xf3"
      "\xd3\x9c\xcd\x6c\xde\xcc\x9d\x2c\xe7\x67\x59\xcc\x4a\x96\xd2\xa9\xdb\x17"
      "\xeb\xf1\x5c\xfe\x9c\xd9\x3d\x53\x0b\x03\xb5\x37\xf7\x8a\x64\xaa\x3e\x2f"
      "\xdd\x73\x36\x4a\x4c\xb3\xf9\x51\x55\x5a\xcc\xe9\x6a\xdb\x63\x59\x4e\x91"
      "\xdb\xb9\x96\xa5\xbc\x5e\xfd\xbb\x98\x0b\xf9\x4e\x2e\xe5\x52\x2e\xf7\x9d"
      "\xe1\x93\x3b\xc6\x5d\x1d\x5b\x75\xd5\x37\xb6\x5e\xf5\xbd\x33\xfd\xf7\xa1"
      "\xc1\x9f\xf9\x56\x5d\x28\xef\x6e\xbf\xdb\xbc\xcb\x2d\xec\x76\xc4\x3b\x8d"
      "\xce\xfd\xd2\xbd\xf7\x97\x79\x3d\xde\x97\xd7\xee\xa8\x7f\xb8\xb1\xd6\xf1"
      "\xbe\xeb\x60\xae\x2f\x4b\x2f\xf5\xb2\x33\x39\xb4\xf3\x27\xb9\x37\x36\xbf"
      "\x56\x17\xca\x7d\xfc\x7a\x8f\xe7\xc4\x78\xcd\xd4\x99\x28\x2f\xa0\xde\x53"
      "\xa2\x17\xdd\xcb\xdd\x4c\x34\xab\x67\xd1\xf6\x71\xfe\xc7\x4e\xb9\x5d\xda"
      "\xb7\x3a\x9d\x1b\x8b\xef\xed\xd0\xff\xea\x96\xfa\x6b\xf5\xbc\x1c\x56\x6b"
      "\x5f\xdf\x6b\xed\x9e\xe1\xa7\x62\x7f\x95\xe3\xe5\xa5\x4c\xd7\x77\x92\xc1"
      "\xd1\x51\xb6\xbd\xbc\x71\x97\xe9\x6b\xeb\x6c\x8e\xe5\x6e\xdb\xe0\x13\xb7"
      "\xdc\xee\x64\xd5\x56\x14\xbd\x2b\xf5\x27\xb9\x5d\x0d\x80\xed\x57\xea\x54"
      "\xfd\x3b\xdc\xf6\x9e\x2e\x56\x6d\xaf\x0c\x6d\x9b\xaf\xda\x4e\xf5\xb5\x0d"
      "\xfc\xbe\x95\xdb\x69\xe7\xda\x18\xf2\x07\xc0\x93\xf8\xe7\xdb\x1b\xc5\x99"
      "\x1c\x9d\x6a\xfd\xbb\xf5\x69\xeb\x93\xd6\x6f\x5a\x37\x5a\x6f\x4c\xff\xf0"
      "\xc8\x77\x8f\xbc\x3a\x95\xc9\x7f\x4c\x7e\xaf\x39\x37\xf1\x5a\xe3\xd5\xe2"
      "\xaf\xf9\x24\xbf\xd8\x7c\xfd\x0f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x3c\xb9\xbb\x1f\x7c"
      "\xf8\xee\x62\xbb\xbd\x74\x67\x78\xa1\xb1\x73\xd3\x40\xa1\x95\xad\x4b\x76"
      "\xea\xf9\xc8\xf0\x7e\x8a\xfa\x0b\x7d\x46\xd8\xd7\x73\x51\x98\x4e\x32\xb0"
      "\xa4\xfa\x9e\xa3\xb1\x87\xd1\xda\x1a\xc6\xb6\x42\xe7\x97\xc9\xd8\xf3\xd3"
      "\xfb\x12\xc1\xe1\xeb\xfc\xbe\x2c\x34\xb7\x8d\xa8\x61\x85\x85\x81\x25\x7f"
      "\xd9\xde\xe1\x47\x8f\x19\x61\x31\xda\x75\x71\x80\x85\x46\xc6\xbb\xd3\x89"
      "\x0c\x1f\x00\x7b\xde\x3a\x3a\x2f\x1e\xe8\x9d\x09\x38\x68\xe7\x57\x6e\xbe"
      "\x77\xfe\xee\x07\x1f\x7e\x7b\xf9\xe6\xe2\x3b\x4b\xef\x2c\xdd\x9a\xbc\x74"
      "\xe9\xf2\xdc\xe5\x4b\xaf\xcf\x9f\xbf\xbe\xdc\x5e\x9a\xeb\xfe\x3c\xec\x28"
      "\x81\x83\xb0\xf9\xd0\x3f\xec\x48\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80"
      "\x51\x0d\xfb\x60\xc0\xe9\x17\x76\xfd\xd0\xc8\xc4\x88\x9f\xf1\xf0\x3f\x0b"
      "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x80\x7d\x71\xe5\x6c\x9a\xf7\x53\xe4\xc2\xdc\xb9\xb9\xb2\xbe"
      "\xbe\x36\xdf\x2e\xa7\x5e\x79\x73\xcd\x66\x92\x46\x23\x29\x7e\x9e\x14\x0f"
      "\x92\x85\x74\xa7\xcc\xf4\x75\x57\xe4\x4f\x0f\xd2\xe9\x5b\x30\x51\xcf\x3f"
      "\x5e\xbe\xfc\xd6\x67\x0f\xd7\x3f\xdf\xec\xab\xd9\x5d\x3f\x69\xd4\xf3\x9d"
      "\xed\xde\x9a\x64\xb5\x9e\x32\x5b\xef\x72\x76\xe4\x04\xec\xda\x5f\xb5\xe3"
      "\xab\x4f\xdd\x5f\xf1\xdf\xde\x31\x94\x09\xfb\xa2\xd3\xe9\x2c\x3c\x5d\x7c"
      "\xb0\x3f\xfe\x17\x00\x00\xff\xff\x72\x9b\xf4\x55",
      1758);
  syz_mount_image(/*fs=*/0x20000100, /*dir=*/0x20002900,
                  /*flags=MS_LAZYTIME|MS_SYNCHRONOUS*/ 0x2000010,
                  /*opts=*/0x200022c0, /*chdir=*/1, /*size=*/0x6de,
                  /*img=*/0x20003480);
  return 0;
}
