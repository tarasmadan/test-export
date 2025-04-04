// https://syzkaller.appspot.com/bug?id=f86606dfb403cc9435c0ed5f17e1d80c110cddef
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
#ifndef __NR_pwritev2
#define __NR_pwritev2 328
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
  const char* reason;
  (void)reason;
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  memcpy((void*)0x20000600, "hfsplus\000", 8);
  memcpy((void*)0x20000640, "./file2\000", 8);
  memcpy((void*)0x20000200,
         "\x00\x0d\xd2\x48\x2a\xd4\xda\x9d\x0a\xe3\x5a\x82\x15\x57\x36\x5d\x84"
         "\xa6\x8b\xfd\xd5\x5e\x98\xab\xc9\xf4\x64\xa9\xe2\xa8\x4a\xe9\x28\x14"
         "\x1e\xc9\x9e\xdb\x34\xf6\x0d\x11\x83\x7e\x57\x76\x8a\x7c\xda\x39\x3f"
         "\x98\xc2\x73\x36\xf0\x58\x34\x01\x41\x74\x53\x6c\xc7\xc8\x06\xe4\x36"
         "\x9d\x1c\x65\x66\xaf\x57\xc4\x47\xe4\x4c\xa7\x2d\xa5\x17\xb6\x2b\x8c"
         "\xae\x80\x3a\x91\x3c\xe5\x5a\xf6\x74\x1c\x72\x4a\x13\xe5\x93\xc5\xea"
         "\x61\x0a\xa5\xe8\xf4\x5b\xab\xd7\x55\x3d\x6c\xec\x45\xa5\xf9\xda\xf2"
         "\x26\x9b\xe1\xa6\xc3\x5f\x78\x30\x7e\xa3\xc0\x78\x65\x62\x2e\x32\x13"
         "\xce\x36\xb7\x94\x0e\xb3\xc5\x74\xa1\x76\x9b\x1b\x07\x49\xc1\x6c\xbe"
         "\x07\x44\x53\xc6\xfa\x76\x20\xeb\x04\x11\xcc\x9b\x81\x2d\x10\x13\x99"
         "\x7c\x5a\xa3\xed\xa8\xec\x70\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         186);
  memcpy(
      (void*)0x20000c80,
      "\x78\x9c\xec\xdd\x4f\x68\x1c\xd7\x1d\x07\xf0\xef\xac\x65\x59\xeb\x82\xb3"
      "\x49\xec\x24\x2d\x85\x8a\x18\x4c\x1b\x53\x5b\xab\x6d\x6a\x17\x0a\x75\x4b"
      "\x29\x3a\x84\x62\xe8\x25\x57\x61\xaf\x63\xe1\xb5\x12\xa4\x4d\x51\x42\x29"
      "\xea\xff\x6b\xa1\x97\x1c\x72\x48\x0f\xea\x21\xa7\x5e\x53\x7a\x08\x4d\xcf"
      "\x3d\xf5\xae\xbb\xa1\x77\xdd\x5c\x66\x76\x56\x5e\x5b\x8a\xb2\x8a\x15\xed"
      "\x2a\xf9\x7c\xe0\xed\x7b\x6f\xdf\xbc\x37\x6f\x7e\x9e\x79\xda\x99\xc5\x6c"
      "\x80\xaf\xac\xa5\xd7\x73\x7a\x33\x45\x96\x2e\xbf\xb6\x51\xd6\xb7\xb7\x3a"
      "\xbd\xed\xad\xce\xfd\x61\x39\xc9\x99\x24\x8d\x64\x66\x90\xa5\x58\x4d\x8a"
      "\x4f\x92\x1b\x19\xa4\x7c\xbd\x7c\xb3\x1e\xae\xf8\xb4\xfd\xbc\xfa\xe0\xa3"
      "\xf7\x2f\xbd\xf7\x61\xa7\xaa\xfc\xa5\x1c\x6b\x66\xb8\x7d\xe3\xa0\x7e\xe3"
      "\xd9\xac\x53\xe6\x93\x9c\xaa\xf3\xa3\x1a\xef\xd6\x53\x8f\x57\xec\x1e\x61"
      "\x19\xb0\x8b\xc3\xc0\xc1\xa4\x3d\xdc\x63\xf3\x30\xdd\x9f\xf2\xba\x05\xa6"
      "\x41\x31\xf8\xbb\xb9\x47\x2b\x39\x9b\x64\xae\xfe\x1c\x90\x7a\x75\x68\x1c"
      "\xef\xec\x8e\xde\xa1\x56\x39\x00\x00\x00\x38\xa1\x9e\xd9\xc9\x4e\x36\x72"
      "\x6e\xd2\xf3\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x93\xa4\xfe\xfd\xff"
      "\xa2\x4e\x8d\x61\x79\x3e\xc5\xf0\xf7\xff\x67\xeb\xf7\x52\x97\x4f\xb4\x8f"
      "\x27\x3d\x01\x00\x00\x00\x00\x00\x00\x00\x38\x02\xdf\xda\xc9\x4e\x36\x72"
      "\x6e\x58\x7f\x58\x54\xdf\xf9\xbf\x5c\x55\xce\x57\xaf\x5f\xcb\xdb\x59\x4f"
      "\x37\x6b\xb9\x92\x8d\x2c\xa7\x9f\x7e\xd6\xd2\x4e\xd2\x1a\x19\x68\x76\x63"
      "\xb9\xdf\x5f\x6b\x8f\xd1\x73\x71\xdf\x9e\x8b\xc7\x73\xbc\x00\x00\x00\x00"
      "\x00\x00\x00\xf0\x25\xf5\xdb\x2c\x3d\xfa\xfe\x1f\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\xa6\x41\x91\x9c\x1a\x64\x55\x3a\x3f\x2c\xb7\xd2\x98"
      "\x49\x32\x97\x64\xb6\xdc\x6e\x33\xf9\xe7\xb0\x7c\x92\x7d\x3c\xe9\x09\x00"
      "\x00\x00\xc0\x31\x78\x66\x27\x3b\xd9\xc8\xb9\x61\xfd\x61\x51\xdd\xf3\xbf"
      "\x50\xdd\xf7\xcf\xe5\xed\xac\xa6\x9f\x95\xf4\xd3\x4b\x37\xb7\xab\x67\x01"
      "\x83\xbb\xfe\xc6\xf6\x56\xa7\xb7\xbd\xd5\xb9\x5f\xa6\xbd\xe3\xfe\xf8\x7f"
      "\x87\x9a\x46\x35\x62\x06\xcf\x1e\xf6\xdf\xf3\x42\xb5\xc5\x85\xdd\x1e\x4b"
      "\xf9\x59\x7e\x91\xcb\x99\xcf\xcd\xac\x65\x25\xbf\xcc\x72\xfa\xe9\x66\x3e"
      "\x3f\xad\x4a\xcb\x29\xd2\xaa\x9f\x5e\xb4\x86\xf3\xdc\x7f\xbe\x37\x1e\xab"
      "\xdd\xfc\xac\xb9\xbe\x54\xcd\xa4\x99\x3b\x59\xa9\xe6\x76\x25\xb7\xf2\x66"
      "\x7a\xb9\x9d\x46\x75\x0c\xd5\x36\x07\xef\xf1\x37\x65\x74\x8a\x1f\xd5\xc6"
      "\x8c\xd1\xed\x3a\x2f\x8f\xe8\xcf\x75\x3e\x1d\x5a\x55\x44\x4e\xef\x46\x64"
      "\xa1\x8e\x7d\x19\x8d\x67\x0f\x8e\xc4\x21\xcf\x93\x27\xf7\xd4\x4e\x63\xf7"
      "\x19\xd4\xf9\x2f\x20\xe6\x67\xeb\xbc\x8c\xf5\x1f\xa7\x3a\xe6\x8b\x23\x67"
      "\xdf\x0b\x07\x47\x22\xf9\xce\x7f\xff\x75\xf3\x6e\x6f\xf5\xde\xdd\x3b\xeb"
      "\x97\xa7\xe7\x90\x3e\xa7\x27\x23\xd1\x19\x89\xc4\x8b\x5f\xa9\x48\xcc\xd6"
      "\xd1\x18\xac\xa2\x87\x5b\x2d\x5f\xae\xfa\x9e\xcb\x4a\x7e\x9e\x37\x73\x3b"
      "\xdd\x5c\xcb\x42\xda\xb9\x9e\x85\xfc\x20\x8b\xe9\x3c\x76\x86\x5d\x18\xe3"
      "\x5a\x6b\x1c\xee\x5a\xbb\xf8\xed\xba\xd0\x4c\xf2\xa7\x3a\x9f\x0e\x65\x5c"
      "\x9f\x1d\x89\xeb\xe8\x4a\xd7\xaa\xda\x46\xdf\x79\x14\xa5\xe7\x8e\x7e\x45"
      "\x9a\xf9\x46\x5d\x28\xf7\xf1\xbb\x3a\x9f\x0e\x4f\x46\xa2\x3d\x12\x89\xe7"
      "\x0f\x8e\xc4\x5f\x1f\x96\xaf\xeb\xbd\xd5\x7b\x6b\x77\x97\xdf\x1a\x73\x7f"
      "\x97\xea\xbc\xbc\x6c\xff\x30\x55\x6b\x73\x79\xbe\x3c\x57\xfe\x63\x55\xb5"
      "\xc7\xcf\x8e\xb2\xed\xf9\x7d\xdb\xda\x55\xdb\xf9\xdd\xb6\xc6\x9e\xb6\x0b"
      "\xbb\x6d\x9f\x75\xa5\xce\xd6\x9f\xe1\xf6\x8e\x34\x68\x7b\x71\xdf\xb6\x4e"
      "\xd5\xf6\xd2\x48\xdb\x7e\x9f\x72\x00\x98\x7a\x67\x5f\x39\x3b\xdb\x7c\xd0"
      "\xfc\x4f\xf3\x83\xe6\xef\x9b\x77\x9b\xaf\xcd\xfd\xe4\xcc\xf5\x33\xdf\x9c"
      "\xcd\xe9\x7f\xcf\xfc\xe3\xd4\xdf\x1b\x7f\x6b\xfc\xb0\x78\x25\x1f\xe4\xd7"
      "\x8f\xee\xff\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x80\xcf\x6f\xfd\x9d\x77\xef\x2d\xf7\x7a"
      "\xdd\x35\x05\x85\x69\x2c\xcc\xd5\x27\xea\xd8\xbd\xce\xd4\x1d\xa6\x61\xf2"
      "\x27\xb9\x30\xc1\x45\x09\x38\x16\x57\xfb\xf7\xdf\xba\xba\xfe\xce\xbb\xdf"
      "\x5d\xb9\xbf\xfc\x46\xf7\x8d\xee\xea\xe2\xf5\x6b\xd7\xaf\x75\xbe\xdf\xfe"
      "\xde\xd5\x3b\x2b\xbd\xee\xc2\xe0\x75\xd2\xb3\x04\xbe\x08\x8f\xfe\xe8\x4f"
      "\x7a\x26\x00\x00\x00\x00\x00\x00\x00\x00\x00\xc0\xb8\x8e\xe3\xbf\x13\x4c"
      "\xfa\x18\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x80\x93\x6d\xe9\xf5\x9c\xde\x4c\x91\xf6\xc2\x95\x85"
      "\xb2\xbe\xbd\xd5\xe9\x95\x69\x58\x1e\x6e\xb7\x99\x99\x24\x8d\x24\xc5\xaf"
      "\x92\xe2\x93\xe4\x46\x06\x29\xad\x91\xe1\x8a\x4f\xdb\xcf\xab\x0f\x3e\x7a"
      "\xff\xd2\x7b\x1f\x76\x06\xb5\x99\x3a\x55\xdb\x37\x0e\xea\x37\x9e\xcd\x3a"
      "\x65\x3e\xc9\xa9\x3a\x3f\xaa\xf1\x6e\x3d\xf5\x78\xc5\xee\x11\x96\x01\xbb"
      "\x38\x0c\x1c\x4c\xda\xff\x03\x00\x00\xff\xff\x39\x3d\x09\x1e",
      1509);
  syz_mount_image(/*fs=*/0x20000600, /*dir=*/0x20000640,
                  /*flags=MS_STRICTATIME*/ 0x1000000, /*opts=*/0x20000200,
                  /*chdir=*/0xfe, /*size=*/0x5e5, /*img=*/0x20000c80);
  memcpy((void*)0x20000040, "./file2\000", 8);
  res = syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000040ul,
                /*flags=O_SYNC|O_NOATIME|FASYNC|O_RDWR*/ 0x143002, /*mode=*/0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000100 = 0x20000080;
  memset((void*)0x20000080, 255, 1);
  *(uint64_t*)0x20000108 = 0xabfb;
  syscall(__NR_pwritev2, /*fd=*/r[0], /*vec=*/0x20000100ul, /*vlen=*/1ul,
          /*off_low=*/0x5405, /*off_high=*/0, /*flags=*/0ul);
  return 0;
}
