// https://syzkaller.appspot.com/bug?id=9f54c9ad3dec036caa23bc578d99434c6e33e016
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
#define __NR_memfd_create 279
#endif
#ifndef __NR_mmap
#define __NR_mmap 222
#endif
#ifndef __NR_mount
#define __NR_mount 40
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
  memcpy((void*)0x20000a40, "udf\000", 4);
  memcpy((void*)0x200001c0,
         "./"
         "file1aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\000",
         108);
  memcpy((void*)0x20000640,
         "\x73\x68\x6f\x72\x74\x61\x64\x00\x00\x00\x00\x6d\x65\x42\x77\xe7\x3d"
         "\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30"
         "\xb0\x30\x34\x2c\x6e\x6f\x61\x64\x69\x6e\x69\x63\xca\x2c\xee\x2f\x9b"
         "\x00\x57\x19\x21\x65\xf8\xa2\xc4\x81\x13\x77\x59\x9e\x66\x4d\x94\x4a"
         "\xb0\x2f\xb2\x4b\x22\xfb\x9f\x0c\xe4\x16\x4c\x95\x46\xc6\x1a\xf6\x3b"
         "\x57\xcf\x4a\x6d\xff\x0d\x33\x58\x6c\x5d\x3d\xcf\x5c\x3c\xbf\x02\x6e"
         "\x8b\xed\x5e\x56\x82\x7b\xb3\xe5\xa8\xb1\x5c\x9f\x9e\x45\x1d\xd9\xac"
         "\x3f\x4a\xc4\x00\x00\x00\x00\x00\x00",
         128);
  sprintf((char*)0x200006c0, "%020llu", (long long)0);
  memcpy((void*)0x200006d4,
         "\x30\x76\x3f\x9f\xf9\x4b\xf8\xd6\xea\x49\x38\x46\x3d\x31\x38\x34\x34"
         "\x36\x37\x34\x34\x80\x00\x0f\xe7",
         25);
  memcpy(
      (void*)0x20001540,
      "\x78\x9c\xec\xdb\x4f\x6c\x9b\xe7\x7d\x07\xf0\xdf\xc3\x57\xb2\x69\xa7\x6b"
      "\x15\xb7\x75\x93\x36\xcb\x58\xb4\x08\x3c\xa5\x0d\xe4\xff\x4a\xbc\x01\xf6"
      "\xac\x0a\x6d\xe6\x26\x46\x65\x65\xf3\x65\x30\x65\xc9\x0e\x11\xfd\xab\x24"
      "\x17\x4e\x37\xb4\x1e\x36\xa0\x08\xd0\x83\x51\x60\x3d\x6c\xc0\x90\xcb\x0e"
      "\x03\x76\xf0\x0e\xbb\xec\x14\xec\x30\x0c\x18\x36\x18\x3b\x0c\xc5\x8a\x76"
      "\x5a\xba\x66\xe9\x8d\xc1\x06\xe4\xb4\x69\x78\x5f\x3e\x94\x28\x59\x8e\xd5"
      "\x38\xb6\x64\xfb\xf3\x31\xec\x2f\xf9\xf2\xf7\x92\xcf\x1f\x9a\x7c\xc9\x87"
      "\x6f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x11\xbf\xf5"
      "\x95\x53\x43\x07\xd3\x76\xb7\x02\x00\xb8\x9f\x5e\x1a\xfb\xc6\xd0\x61\xef"
      "\xff\x00\xf0\x48\x39\xe7\xf3\x3f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\xec\x74\x29\x8a\xf8\xb3\x48\xf1\xea\x4f\xda\xe9\x42\x75\xbd\xa3\x7e"
      "\xa6\x35\x7b\xe5\xea\xf8\xc8\xe8\xe6\xbb\xed\x49\x91\xa2\x16\x45\x55\x5f"
      "\xfe\xad\x1f\x3c\x74\xf8\xc8\xd1\x63\xc7\x87\xbb\xf9\xc1\xfb\x7f\xd4\x9e"
      "\x8c\x97\xc7\xce\x9d\x6a\x9c\x9e\x9b\x99\x5f\x98\x5a\x5c\x9c\x9a\x6c\x8c"
      "\xcf\xb6\x2e\xce\x4d\x4e\x6d\xf9\x1e\xee\x76\xff\x8d\x06\xab\x01\x68\xcc"
      "\xbc\x76\x65\xf2\xd2\xa5\xc5\xc6\xa1\xe7\x0e\xaf\xbb\xf9\xea\xc0\x3b\xbb"
      "\x1f\xdb\x3f\x70\xe2\xf8\x8b\xe7\xf7\x75\x6b\xc7\x47\x46\x47\xc7\x7a\x6a"
      "\xfa\xfa\x3f\xf4\xa3\xdf\xe2\x76\x67\x78\xec\x8a\x22\x7e\x16\x29\xea\xdf"
      "\x7b\x37\x35\x23\xa2\x16\x77\x3f\x16\x77\x78\xee\xdc\x6b\x7b\xaa\x4e\x0c"
      "\x56\x9d\x18\x1f\x19\xad\x3a\x32\xdd\x6a\xce\x2e\x95\x37\xa6\x5a\xae\xaa"
      "\x45\x0c\xf4\xec\x74\xb2\x3b\x46\xf7\x61\x2e\xee\x4a\x23\xe2\x5a\xd9\xfc"
      "\xb2\xc1\x83\x65\xf7\xc6\xe6\x9b\x0b\xcd\x89\xe9\xa9\xc6\xd9\xe6\xc2\x52"
      "\x6b\xa9\x35\x37\x9b\x6a\x9d\xd6\x96\xfd\x19\x88\x5a\x0c\xa7\x88\xf9\x88"
      "\x68\x17\xb7\xde\x5d\x7f\x14\xf1\xef\x91\xe2\xfb\xef\xb7\xd3\x44\x44\x14"
      "\xdd\x71\x78\xb6\x3a\x31\xf8\xce\xed\xa9\xdd\x83\x3e\x6e\x41\x5f\xd9\xb7"
      "\x22\xe2\x66\x3c\x00\x73\xb6\x83\xed\x8e\x22\xde\x88\x14\x3f\x38\x3f\x14"
      "\x17\xf3\xb8\x56\xc3\xf6\x4c\xc4\xd7\xcb\x7c\x3a\xe2\x9b\x65\x2e\x47\x5c"
      "\xcf\xd7\x53\xf9\x04\x79\x2a\xe2\xbd\x4d\x9e\x4f\x3c\x58\xfa\xa2\x88\x7f"
      "\x8a\x14\x73\xa9\x9d\x26\xbb\x73\x5f\xbd\xae\x9c\x79\xa5\xf1\xb5\xd9\x4b"
      "\x73\x3d\xb5\xdd\xd7\x95\x07\xfe\xfd\xe1\x7e\xda\xe1\xaf\x4d\xf5\x28\x62"
      "\xa2\x7a\xc5\x6f\xa7\x0f\x7f\xb0\x03\x00\x00\x00\x00\xc0\xce\x53\xc4\xdf"
      "\x46\x8a\x1b\x33\x07\xd2\x7c\xf4\xae\x29\xb6\x66\x2f\x37\xce\x35\x27\xa6"
      "\x3b\xdf\x0a\x77\xbf\xfb\x6f\xe4\xbd\x56\x56\x56\x56\x06\x52\x27\x1b\x39"
      "\x87\x72\x9e\xcc\x79\x36\xe7\x85\x9c\xf3\x39\xaf\xe5\xbc\x9e\xf3\xcd\x9c"
      "\x37\x72\xbe\x95\xf3\x66\xce\xe5\x9c\xed\x9c\x51\xcb\x8f\x9f\xb3\x91\x73"
      "\x28\xe7\xc9\x9c\x67\x73\x5e\xc8\x39\x9f\xf3\x5a\xce\xeb\x39\xdf\xcc\x79"
      "\x23\xe7\x5b\x39\x6f\xe6\x5c\xce\xd9\xce\x19\xd6\xbd\x00\x00\x00\x00\x00"
      "\x00\x00\xd8\x61\xf6\x44\x11\x3f\x8e\x14\x5f\xf8\x9b\x6f\x55\xe7\x15\x47"
      "\x75\x5e\xfa\x27\x4e\x0c\x1f\xf8\xea\x17\x7b\xcf\x19\xff\xcc\x1d\xee\xa7"
      "\xac\x7d\x2e\x22\x6e\xc4\xd6\xce\xc9\xed\xcf\xa7\x0e\xa7\x5a\xf9\xe7\xa3"
      "\xef\x17\x5b\x53\x8f\x22\xbe\x93\xcf\xff\xfb\xc3\xed\x6e\x0c\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\xb0\xad\x6a\x51\xc4\x67\x22\xc5\x0f\xdf\x68\xa7\x48"
      "\x11\xd1\x88\xb8\x10\x9d\x5c\x2e\xb6\xbb\x75\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\xc0\x87\x51\x4f\x45\x9c\x8e\x14\xbf\xf8\x4a\xbd\xba\x7e\x33\x22"
      "\x3e\x1b\x11\xff\xb7\x52\xfe\x89\x88\xe5\x95\x0d\xb6\xbb\xc5\x00\x00\x00"
      "\x00\x00\x00\x00\xc0\x2d\x52\x11\x43\x91\xe2\xf1\x27\xdb\x69\x20\x22\xae"
      "\x0e\xbc\xb3\xfb\xb1\xfd\x03\x27\x8e\xbf\x78\x7e\x5f\x11\x45\xa4\xb2\xa4"
      "\xb7\xfe\xe5\xb1\x73\xa7\x1a\xa7\xe7\x66\xe6\x17\xa6\x16\x17\xa7\x26\x1b"
      "\xe3\xb3\xad\x8b\x73\x93\x53\x5b\x7d\xb8\xfa\x99\xd6\xec\x95\xab\xe3\x23"
      "\xa3\xf7\xa4\x33\x77\xb4\xe7\x1e\xb7\x7f\x4f\xfd\xf4\xdc\xfc\xeb\x0b\xad"
      "\xcb\xaf\x2e\x6d\x7a\xfb\xde\xfa\xa9\x89\xc5\xa5\x85\xe6\xc5\xcd\x6f\x8e"
      "\x3d\x51\x8b\x18\xea\xdd\x32\x58\x35\x78\x7c\x64\xb4\x6a\xf4\x74\xab\x39"
      "\x5b\xed\x9a\x6a\xb7\x69\x60\x2d\xa2\xb1\xd5\xce\x00\x00\x00\x00\x00\x00"
      "\x00\xf0\xd0\xd8\x9b\x8a\x38\x1a\x29\x5e\x6d\x1d\x49\xdd\x75\xe3\xbe\xce"
      "\x9a\xff\xaf\x74\xae\x15\xab\xb5\x7f\xf1\x07\x6b\xbf\x05\x98\xde\x90\x5d"
      "\xbd\xbf\x1f\xd8\xca\xe5\xb4\xd5\x86\x0e\x56\x0b\xef\x8d\xf1\x91\xd1\xd1"
      "\xb1\x9e\xcd\x7d\xfd\xb7\x96\x96\x6d\x4a\xa9\x88\xbf\x8e\x14\x9f\xfb\xdd"
      "\x27\xaa\xf5\xf0\x14\x7b\x37\x5d\x1b\x2f\xeb\x76\x45\x8a\x63\xdf\x3a\x92"
      "\xeb\x06\x3e\x57\xd6\x9d\x5c\x57\x55\x1f\x1c\x1f\x19\x6d\xbc\x34\x37\xfb"
      "\xe5\x53\xd3\xd3\x73\x17\x9b\x4b\xcd\x89\xe9\xa9\xc6\xd8\x7c\xf3\xe2\x96"
      "\x7f\x38\x00\x00\x00\x00\x00\x00\x00\x00\xf7\xd0\xde\x54\xc4\x9f\x47\x8a"
      "\xdf\x1b\xba\x99\xba\xe7\x9d\xe7\xf5\xff\xbe\xce\xb5\x9e\xf5\xff\xdf\xa8"
      "\x96\xd0\x2b\xf5\xb4\x3e\x57\x55\x6b\xfb\x1f\xaf\xd6\xf6\x3b\x97\x3f\x71"
      "\x62\xb8\x31\xfa\x6b\xb7\xdb\x7e\x2f\xd6\xff\xcb\x36\xa5\x54\xc4\xbf\x45"
      "\x8a\xc7\x7f\xff\x89\xea\x7c\xfa\xee\xfa\xff\xd0\x86\xda\xb2\xee\xbf\x23"
      "\xc5\xbf\xfe\xe3\x53\xb9\xae\xb6\xab\xac\x3b\xd8\xed\x4e\xe7\x1e\x2f\xb5"
      "\xa6\xa7\x86\x52\x1e\xab\xcf\x3f\xdb\xad\x8d\xaa\xf6\x78\xae\xfd\xe4\x5a"
      "\xed\xc1\xb2\xf6\xf3\x91\xe2\x2f\x9f\x59\x5f\x3b\x9c\x6b\x3f\xb5\x56\x7b"
      "\xa8\xac\xfd\xe3\x48\xf1\xbf\x47\x37\xaf\xfd\xf4\x5a\xed\xe1\xb2\xf6\x8f"
      "\x22\xc5\x6f\xbf\xdd\xe8\xd6\xee\x2d\x6b\xcf\xe4\xda\xfd\x6b\xb5\xcf\x5d"
      "\x9c\x9b\x9e\xbc\xd3\xb0\x96\xf3\xff\x77\x91\xe2\xec\x2f\xbe\x9a\xba\x7d"
      "\xbe\xed\xfc\xf7\xfc\xfe\xe3\xda\x86\x5c\x75\xcb\x9c\x7f\xf0\xe5\x8f\x6a"
      "\xfe\x07\x7a\xb6\x5d\xcb\xf3\xfa\xe3\x3c\xff\x07\xef\x30\xff\x7f\x1f\x29"
      "\xfe\xe4\xa7\x4f\xe5\xba\xce\xd8\x1f\xca\xb7\x3f\x5e\xfd\xbb\x36\xff\xbf"
      "\x13\x29\xfe\xeb\x57\xd7\xd7\x1e\xcb\xb5\xfb\xd6\x6a\x0f\x6e\xb5\x5b\xdb"
      "\xad\x9c\xff\x2f\x45\x8a\x13\x3f\xfa\xd1\x6a\x9f\xf3\xfc\xe7\x91\x5d\x9b"
      "\xa1\xde\xf9\xff\x6c\xdf\xfa\x5c\x7d\x96\x6c\xd3\xfc\x3f\xde\xb3\x6d\x20"
      "\xb7\xeb\xf0\x2f\x39\x16\x8f\xa2\xc5\xd7\xbf\xfd\x5a\x73\x7a\x7a\x6a\xc1"
      "\x05\x17\x5c\x70\x61\xf5\xc2\x76\xbf\x32\x71\x3f\x94\xef\xff\xff\x1c\x29"
      "\x5e\x38\x53\x4b\xdd\xe3\x98\xfc\xfe\xff\xb1\xce\xb5\xb5\xe3\xbf\xf7\xbf"
      "\xb3\xf6\xfe\xff\xc2\x86\x5c\xb5\x4d\xef\xff\xfb\x7a\xb6\xbd\x90\x8f\x5a"
      "\xfa\xfb\x22\xea\x4b\x33\xf3\xfd\xfb\x23\xea\x8b\xaf\x7f\xfb\xcb\xad\x99"
      "\xe6\xe5\xa9\xcb\x53\xb3\xc3\xc7\x8e\x1e\x79\x7e\xf8\xd8\xb1\xe7\xfb\x77"
      "\x75\x8f\xed\xd6\x2e\x6d\x79\xe8\x1e\x0a\xe5\xfc\x9f\x89\x14\xaf\xfc\xf4"
      "\x5f\x56\x3f\xc7\xac\x3f\xfe\xdb\xfc\xf8\x7f\xef\x86\x5c\xb5\x4d\xf3\xff"
      "\xc9\xde\x3e\xad\x3b\xae\xd9\xf2\x50\x3c\x92\xca\xf9\xbf\x1e\x29\xbe\xfb"
      "\xf6\xbb\xab\x9f\x37\x3f\xe8\xf8\xbf\xfb\xf9\xff\xc0\x17\xd6\xe7\xea\xff"
      "\xbf\x6d\x9a\xff\x4f\xf5\x6c\xab\x7e\xe3\xff\xf1\x88\xe7\x7b\xb6\x1d\xf8"
      "\x74\xc4\xa9\xad\x3e\x16\x00\x00\x00\x3c\x64\xf6\xe6\x75\xf2\x3f\xfd\xf5"
      "\x7f\x58\x3d\xe7\x7d\xfd\xe7\xff\xf8\x62\xb7\xb6\xf7\xfb\x9f\xdb\xd9\x09"
      "\xe7\xff\x03\x00\x00\x00\x00\xc0\xa3\x6e\x6f\x2a\xe2\xaf\x22\xc5\xff\x0c"
      "\x7d\x29\x75\xcf\x21\xdb\xca\xef\x3f\x27\x37\xe4\xaa\x6d\xfa\xfd\xdf\xfe"
      "\x9e\x6d\x93\xf7\xe9\xbc\x96\x2d\x0f\x32\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\xc0\x0e\x94\xa2\x88\xa7\x23\xc5\xab\x3f\x69\xa7\xe5\xa2\xbc\xde"
      "\x51\x3f\xd3\x9a\xbd\x72\x75\x7c\x64\x74\xf3\xdd\xf6\xa4\x48\x51\x8b\xa2"
      "\xaa\x2f\xff\xd6\x0f\x1e\x3a\x7c\xe4\xe8\xb1\xe3\xc3\xdd\xfc\xe0\xfd\x3f"
      "\x6a\x4f\xc6\xcb\x63\xe7\x4e\x35\x4e\xcf\xcd\xcc\x2f\x4c\x2d\x2e\x4e\x4d"
      "\x36\xc6\x67\x5b\x17\xe7\x26\xa7\xb6\x7c\x0f\x77\xbb\xff\x46\x83\xd5\x00"
      "\x34\x66\x5e\xbb\x32\x79\xe9\xd2\x62\xe3\xd0\x73\x87\xd7\xdd\x7c\x75\xe0"
      "\x9d\xdd\x8f\xed\x1f\x38\x71\xfc\xc5\xf3\xfb\xba\xb5\xe3\x23\xa3\xa3\x63"
      "\x3d\x35\x7d\xfd\x1f\xfa\xd1\x6f\x91\x6e\xb3\x7d\x57\x14\x71\x29\x52\xd4"
      "\xbf\xf7\x6e\xfa\x8f\x22\xa2\x16\x77\x3f\x16\x77\x78\xee\xdc\x6b\x7b\xaa"
      "\x4e\x0c\x56\x9d\x18\x1f\x19\xad\x3a\x32\xdd\x6a\xce\x2e\x95\x37\xa6\x5a"
      "\xae\xaa\x45\x0c\xf4\xec\x74\xb2\x3b\x46\xf7\x61\x2e\xee\x4a\x23\xe2\x5a"
      "\xd9\xfc\xb2\xc1\x83\x65\xf7\xc6\xe6\x9b\x0b\xcd\x89\xe9\xa9\xc6\xd9\xe6"
      "\xc2\x52\x6b\xa9\x35\x37\x9b\x6a\x9d\xd6\x96\xfd\x19\x88\x5a\x0c\xa7\x88"
      "\xf9\x88\x68\x17\xb7\xde\x5d\x7f\x14\x31\x11\x29\xbe\xff\x7e\x3b\xbd\x5d"
      "\x44\x14\xdd\x71\x78\xf6\xa5\xb1\x6f\x0c\x1d\xbe\x73\x7b\x6a\xf7\xa0\x8f"
      "\xbd\x56\xbe\xbb\xe9\xe6\xbe\xb2\x6f\x45\xc4\xcd\x78\x00\xe6\x6c\x07\xdb"
      "\x1d\x45\x7c\x2c\x52\xfc\xe0\xfc\x50\xfc\xac\xe8\x8c\x6b\x35\x6c\xcf\x44"
      "\x7c\xbd\xcc\xa7\x23\xbe\x59\xe6\x72\xc4\xf5\x7c\x3d\x95\x4f\x90\xa7\x22"
      "\xde\xdb\xe4\xf9\xc4\x83\xa5\x2f\x8a\x38\x1b\x29\xe6\x52\x3b\xfd\x67\x91"
      "\xe7\xbe\x7a\x5d\x39\xf3\x4a\xe3\x6b\xb3\x97\xe6\x7a\x6a\xbb\xaf\x2b\x0f"
      "\xfc\xfb\xc3\xfd\xb4\xc3\x5f\x9b\xea\x51\xc4\xcf\xab\x57\xfc\x76\xfa\xb9"
      "\xff\xcf\x00\x00\x00\x00\x00\x0f\x91\x22\x7e\x33\x52\xdc\x98\x39\x90\xaa"
      "\xf5\xc1\xd5\x35\xc5\xd6\xec\xe5\xc6\xb9\xe6\xc4\x74\xe7\x6b\xfd\xee\x77"
      "\xff\x8d\xbc\xd7\xca\xca\xca\xca\x40\xea\x64\x23\xe7\x50\xce\x93\x39\xcf"
      "\xe6\xbc\x90\x73\x3e\xe7\xb5\x9c\xd7\x73\xbe\x99\xf3\x46\xce\xb7\x72\xde"
      "\xcc\xb9\x9c\xb3\x9d\x33\x6a\xf9\xf1\x73\x36\x72\x0e\xe5\x3c\x99\xf3\x6c"
      "\xce\x0b\x39\xe7\x73\x5e\xcb\x79\x3d\xe7\x9b\x39\x6f\xe4\x7c\x2b\xe7\xcd"
      "\x9c\xcb\x39\xdb\x39\xc3\xf7\xe4\x00\x00\x00\x00\x00\x00\xc0\x0e\x54\x8b"
      "\x22\x9e\x88\x14\x3f\x7c\xa3\x9d\x56\x8a\xce\x02\xef\x85\xe8\xe4\xb2\x75"
      "\xce\x87\xde\xff\x07\x00\x00\xff\xff\x56\xcc\x3f\x2e",
      2587);
  syz_mount_image(
      /*fs=*/0x20000a40, /*dir=*/0x200001c0,
      /*flags=MS_REC|MS_STRICTATIME|MS_NOATIME|MS_DIRSYNC*/ 0x1004480,
      /*opts=*/0x20000640, /*chdir=*/1, /*size=*/0xa1b, /*img=*/0x20001540);
  memcpy((void*)0x20000540, "./file1\000", 8);
  memcpy((void*)0x20000580, "./file1\000", 8);
  memcpy((void*)0x200005c0, "erofs\000", 6);
  syscall(__NR_mount, /*src=*/0x20000540ul, /*dst=*/0x20000580ul,
          /*type=*/0x200005c0ul, /*flags=MS_RELATIME*/ 0x200000ul,
          /*data=*/0ul);
  return 0;
}
