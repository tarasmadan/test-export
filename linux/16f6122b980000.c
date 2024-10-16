// https://syzkaller.appspot.com/bug?id=68414d97aff516b2ada6f66a36b4ecca7339a066
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

#ifndef __NR_io_uring_setup
#define __NR_io_uring_setup 425
#endif
#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

static unsigned long long procid;

#define SIZEOF_IO_URING_SQE 64
#define SIZEOF_IO_URING_CQE 16
#define SQ_HEAD_OFFSET 0
#define SQ_TAIL_OFFSET 64
#define SQ_RING_MASK_OFFSET 256
#define SQ_RING_ENTRIES_OFFSET 264
#define SQ_FLAGS_OFFSET 276
#define SQ_DROPPED_OFFSET 272
#define CQ_HEAD_OFFSET 128
#define CQ_TAIL_OFFSET 192
#define CQ_RING_MASK_OFFSET 260
#define CQ_RING_ENTRIES_OFFSET 268
#define CQ_RING_OVERFLOW_OFFSET 284
#define CQ_FLAGS_OFFSET 280
#define CQ_CQES_OFFSET 320

struct io_sqring_offsets {
  uint32_t head;
  uint32_t tail;
  uint32_t ring_mask;
  uint32_t ring_entries;
  uint32_t flags;
  uint32_t dropped;
  uint32_t array;
  uint32_t resv1;
  uint64_t resv2;
};

struct io_cqring_offsets {
  uint32_t head;
  uint32_t tail;
  uint32_t ring_mask;
  uint32_t ring_entries;
  uint32_t overflow;
  uint32_t cqes;
  uint64_t resv[2];
};

struct io_uring_params {
  uint32_t sq_entries;
  uint32_t cq_entries;
  uint32_t flags;
  uint32_t sq_thread_cpu;
  uint32_t sq_thread_idle;
  uint32_t features;
  uint32_t resv[4];
  struct io_sqring_offsets sq_off;
  struct io_cqring_offsets cq_off;
};

#define IORING_OFF_SQ_RING 0
#define IORING_OFF_SQES 0x10000000ULL
#define IORING_SETUP_SQE128 (1U << 10)
#define IORING_SETUP_CQE32 (1U << 11)

static long syz_io_uring_setup(volatile long a0, volatile long a1,
                               volatile long a2, volatile long a3)
{
  uint32_t entries = (uint32_t)a0;
  struct io_uring_params* setup_params = (struct io_uring_params*)a1;
  void** ring_ptr_out = (void**)a2;
  void** sqes_ptr_out = (void**)a3;
  setup_params->flags &= ~(IORING_SETUP_CQE32 | IORING_SETUP_SQE128);
  uint32_t fd_io_uring = syscall(__NR_io_uring_setup, entries, setup_params);
  uint32_t sq_ring_sz =
      setup_params->sq_off.array + setup_params->sq_entries * sizeof(uint32_t);
  uint32_t cq_ring_sz = setup_params->cq_off.cqes +
                        setup_params->cq_entries * SIZEOF_IO_URING_CQE;
  uint32_t ring_sz = sq_ring_sz > cq_ring_sz ? sq_ring_sz : cq_ring_sz;
  *ring_ptr_out =
      mmap(0, ring_sz, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE,
           fd_io_uring, IORING_OFF_SQ_RING);
  uint32_t sqes_sz = setup_params->sq_entries * SIZEOF_IO_URING_SQE;
  *sqes_ptr_out = mmap(0, sqes_sz, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_POPULATE, fd_io_uring, IORING_OFF_SQES);
  uint32_t* array =
      (uint32_t*)((uintptr_t)*ring_ptr_out + setup_params->sq_off.array);
  for (uint32_t index = 0; index < entries; index++)
    array[index] = index;
  return fd_io_uring;
}

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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

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
  memcpy((void*)0x20000040, "ext4\000", 5);
  memcpy((void*)0x20000140, "./file1\000", 8);
  memcpy((void*)0x200005c0, "noblock_validity", 16);
  *(uint8_t*)0x200005d0 = 0x2c;
  memcpy((void*)0x200005d1, "bsddf", 5);
  *(uint8_t*)0x200005d6 = 0x2c;
  memcpy((void*)0x200005d7, "sysvgroups", 10);
  *(uint8_t*)0x200005e1 = 0x2c;
  memcpy((void*)0x200005e2, "norecovery", 10);
  *(uint8_t*)0x200005ec = 0x2c;
  memcpy((void*)0x200005ed, "debug_want_extra_isize", 22);
  *(uint8_t*)0x20000603 = 0x3d;
  sprintf((char*)0x20000604, "0x%016llx", (long long)0x80);
  *(uint8_t*)0x20000616 = 0x2c;
  memcpy((void*)0x20000617, "orlov", 5);
  *(uint8_t*)0x2000061c = 0x2c;
  memcpy((void*)0x2000061d, "debug", 5);
  *(uint8_t*)0x20000622 = 0x2c;
  memcpy((void*)0x20000623, "noauto_da_alloc", 15);
  *(uint8_t*)0x20000632 = 0x2c;
  memcpy((void*)0x20000633, "nomblk_io_submit", 16);
  *(uint8_t*)0x20000643 = 0x2c;
  *(uint8_t*)0x20000644 = 0;
  memcpy(
      (void*)0x20000680,
      "\x78\x9c\xec\xdd\x5d\x6b\x1c\x55\x18\x00\xe0\x77\x36\x49\xbf\xb5\x29\x94"
      "\xa2\x22\x12\xe8\x85\x95\xda\x4d\x93\xf8\x51\x41\xb0\x5e\x8a\x16\x0b\x7a"
      "\x5f\x97\x64\x1a\x6a\x36\xdd\x92\xdd\x94\x26\x16\xda\x5e\xd8\x1b\x6f\xa4"
      "\x08\x22\x16\xc4\x1f\xe0\xbd\x97\xc5\x3f\xe0\xaf\x28\x68\xa1\x48\x09\x7a"
      "\xe1\x4d\x64\x36\xb3\xed\x36\xd9\xcd\xe7\x6a\xb6\xce\xf3\xc0\xb4\xe7\xcc"
      "\xcc\xe6\xcc\xd9\x33\xef\xd9\x77\x76\x76\xd9\x00\x0a\x6b\x24\xfb\xa7\x14"
      "\xf1\x62\xdc\x8c\xaf\x93\x88\xc3\x6d\xdb\x06\x23\xdf\x38\xb2\xb2\xdf\xd2"
      "\xa3\xeb\x93\xd9\x92\xc4\xf2\xf2\x27\x7f\x24\x91\xe4\xeb\x5a\xfb\x27\xf9"
      "\xff\x07\xf3\xca\x0b\x11\xf1\xcb\x97\x11\x27\x4b\x6b\xdb\xad\x2f\x2c\xce"
      "\x54\xaa\xd5\x74\x2e\xaf\x8f\x36\x66\xaf\x8c\xd6\x17\x16\x4f\x5d\x9a\xad"
      "\x4c\xa7\xd3\xe9\xe5\xf1\x89\x89\x33\x6f\x4e\x8c\xbf\xf3\xf6\x5b\x3d\xeb"
      "\xeb\x6b\xe7\xff\xfa\xee\xe3\x7b\x1f\x9c\xf9\xea\xf8\xd2\xb7\x3f\x3d\x38"
      "\x72\x27\x89\xb3\x71\x28\xdf\xd6\xde\x8f\x1d\xb8\xd9\x5e\x19\x89\x91\xfc"
      "\x39\x19\x8a\xb3\xab\x76\x1c\xeb\x41\x63\xfd\x24\xd9\xed\x03\x60\x5b\x06"
      "\xf2\x38\x1f\x8a\x6c\x0e\x38\x1c\x03\x79\xd4\x03\xff\x7f\x37\x22\x62\x19"
      "\x28\xa8\x44\xfc\x43\x41\xb5\xf2\x80\xd6\xb5\x7d\x8f\xae\x83\x9f\x19\x0f"
      "\xdf\x5f\xb9\x00\x5a\xdb\xff\xc1\x95\xf7\x46\x62\x5f\xf3\xda\xe8\xc0\x52"
      "\xf2\xd4\x95\x51\x76\xbd\x3b\xdc\x83\xf6\xb3\x36\x7e\xfe\xfd\xee\x9d\x6c"
      "\x89\x0d\xde\x87\xb8\xd1\x83\xf6\x00\x5a\x6e\xde\x8a\x88\xd3\x83\x83\x6b"
      "\xe7\xbf\x24\x9f\xff\xb6\xef\x74\xf3\xcd\xe3\xf5\xad\x6e\xa3\x68\xaf\x3f"
      "\xb0\x9b\xee\x65\xf9\xcf\xeb\x9d\xf2\x9f\xd2\xe3\xfc\x27\x3a\xe4\x3f\x07"
      "\x3b\xc4\xee\x76\x6c\x1c\xff\xa5\x07\x3d\x68\xa6\xab\x2c\xff\x7b\xb7\x63"
      "\xfe\xfb\x78\xea\x1a\x1e\xc8\x6b\xcf\x35\x73\xbe\xa1\xe4\xe2\xa5\x6a\x7a"
      "\x3a\x22\x9e\x8f\x88\x13\x31\xb4\x37\xab\xaf\x77\x3f\xe7\xcc\xd2\xfd\xe5"
      "\x6e\xdb\xda\xf3\xbf\x6c\xc9\xda\x6f\xe5\x82\xf9\x71\x3c\x18\xdc\xfb\xf4"
      "\x63\xa6\x2a\x8d\xca\x4e\xfa\xdc\xee\xe1\xad\x88\x97\x9e\xe4\xbf\x49\xac"
      "\x99\xff\xf7\x35\x73\xdd\xd5\xe3\x9f\x3d\x1f\xe7\x37\xd9\xc6\xb1\xf4\xee"
      "\x2b\xdd\xb6\x6d\xdc\xff\x76\xbd\xcf\x80\x97\x7f\x8c\x78\xb5\xe3\xf8\x3f"
      "\xb9\xa3\x95\xac\x7f\x7f\x72\xb4\x79\x3e\x8c\xb6\xce\x8a\xb5\xfe\xbc\x7d"
      "\xec\xd7\x6e\xed\x6f\xad\xff\xbd\x97\x8d\xff\x81\xf5\xfb\x3f\x9c\xb4\xdf"
      "\xaf\xad\x6f\xbd\x8d\x1f\xf6\xfd\x9d\x76\xdb\xb6\xdd\xf3\x7f\x4f\xf2\x69"
      "\xb3\xbc\x27\x5f\x77\xad\xd2\x68\xcc\x8d\x45\xec\x49\x3e\x5a\xbb\x7e\xfc"
      "\xc9\x63\x5b\xf5\xd6\xfe\x59\xff\x4f\x1c\x5f\x7f\xfe\xeb\x74\xfe\xef\x8f"
      "\x88\xcf\x36\xd9\xff\xdb\x47\x6f\x77\xdd\xb5\x1f\xc6\x7f\x6a\x4b\xe3\xbf"
      "\xf5\xc2\xfd\x0f\xbf\xf8\xbe\x5b\xfb\x9b\x1b\xff\x37\x9a\xa5\x13\xf9\x9a"
      "\xcd\xcc\x7f\x9b\x3d\xc0\x9d\x3c\x77\x00\x00\x00\x00\x00\x00\xd0\x6f\x4a"
      "\x11\x71\x28\x92\x52\xf9\x71\xb9\x54\x2a\x97\x57\x3e\xdf\x71\x34\x0e\x94"
      "\xaa\xb5\x7a\xe3\xe4\xc5\xda\xfc\xe5\xa9\x68\x7e\x57\x76\x38\x86\x4a\xad"
      "\x3b\xdd\x87\xdb\x3e\x0f\x31\x96\x7f\x1e\xb6\x55\x1f\x5f\x55\x9f\x88\x88"
      "\x23\x11\xf1\xcd\xc0\xfe\x66\xbd\x3c\x59\xab\x4e\xed\x76\xe7\x01\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xa0\x4f\x1c\xec\xf2\xfd"
      "\xff\xcc\x6f\x03\xbb\x7d\x74\xc0\xbf\xce\x4f\x7e\x43\x71\x6d\x18\xff\xbd"
      "\xf8\xa5\x27\xa0\x2f\x79\xfd\x87\xe2\xea\x12\xff\xa6\x05\x28\x00\x81\x0e"
      "\xc5\x25\xfe\xa1\xb8\xc4\x3f\x14\x97\xf8\x87\xe2\x12\xff\x50\x5c\xe2\x1f"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x7a\xea\xfc\xb9\x73\xd9\xb2\xbc\xf4\xe8\xfa\x64\x56\x9f\xba"
      "\xba\x30\x3f\x53\xbb\x7a\x6a\x2a\xad\xcf\x94\x67\xe7\x27\xcb\x93\xb5\xb9"
      "\x2b\xe5\xe9\x5a\x6d\xba\x9a\x96\x27\x6b\xb3\x1b\xfd\xbd\x6a\xad\x76\x65"
      "\x6c\x3c\xe6\xaf\x8d\x36\xd2\x7a\x63\xb4\xbe\xb0\x78\x61\xb6\x36\x7f\xb9"
      "\x71\xe1\xd2\x6c\x65\x3a\xbd\x90\x0e\xfd\x27\xbd\x02\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x67\x4b\x7d\x61\x71\xa6\x52\xad"
      "\xa6\x73\x0a\x5d\x0b\xef\xc5\x6e\x1f\xc6\xe7\x2f\xef\xe4\xe1\x49\xe7\x51"
      "\x4e\xda\x3a\xb8\x62\x5b\x4d\x0c\xf6\xcb\x30\x29\xf4\xb4\xb0\xcb\x13\x13"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xb4\xf9\x27\x00\x00"
      "\xff\xff\x36\xd7\x33\x82",
      1392);
  syz_mount_image(/*fs=*/0x20000040, /*dir=*/0x20000140,
                  /*flags=MS_RELATIME*/ 0x200000, /*opts=*/0x200005c0,
                  /*chdir=*/3, /*size=*/0x570, /*img=*/0x20000680);
  memcpy((void*)0x20000180, "./bus\000", 6);
  syscall(__NR_open, /*file=*/0x20000180ul,
          /*flags=O_TRUNC|O_SYNC|O_NOCTTY|O_NOATIME|O_CREAT|O_WRONLY|0x3c*/
          0x14137dul, /*mode=*/0ul);
  memcpy((void*)0x20000080, "./file1\000", 8);
  res = syscall(__NR_creat, /*file=*/0x20000080ul, /*mode=*/0ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000600 = 0x78;
  *(uint32_t*)0x20000604 = 0;
  *(uint64_t*)0x20000608 = 0;
  *(uint64_t*)0x20000610 = 0;
  *(uint32_t*)0x20000618 = 0;
  *(uint32_t*)0x2000061c = 0;
  *(uint64_t*)0x20000620 = 0;
  *(uint64_t*)0x20000628 = 0;
  *(uint64_t*)0x20000630 = 0;
  *(uint64_t*)0x20000638 = 0;
  *(uint64_t*)0x20000640 = 0;
  *(uint64_t*)0x20000648 = 0;
  *(uint32_t*)0x20000650 = 0;
  *(uint32_t*)0x20000654 = 0;
  *(uint32_t*)0x20000658 = 0;
  *(uint32_t*)0x2000065c = 0;
  *(uint32_t*)0x20000660 = 0;
  *(uint32_t*)0x20000664 = 0;
  *(uint32_t*)0x20000668 = 0;
  *(uint32_t*)0x2000066c = 0;
  *(uint32_t*)0x20000670 = 0;
  *(uint32_t*)0x20000674 = 0;
  syscall(__NR_write, /*fd=*/r[0], /*arg=*/0x20000600ul, /*len=*/0x78ul);
  memcpy((void*)0x20000280, "./file1\000", 8);
  syscall(__NR_unlink, /*path=*/0x20000280ul);
  memcpy((void*)0x20000380, "/dev/loop", 9);
  *(uint8_t*)0x20000389 = 0x30;
  *(uint8_t*)0x2000038a = 0;
  memcpy((void*)0x20000340, "./bus\000", 6);
  syscall(__NR_mount, /*src=*/0x20000380ul, /*dst=*/0x20000340ul, /*type=*/0ul,
          /*flags=MS_BIND*/ 0x1000ul, /*data=*/0ul);
  memcpy((void*)0x200005c0, "./bus\000", 6);
  res = syscall(__NR_open, /*file=*/0x200005c0ul, /*flags=*/0ul, /*mode=*/0ul);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20000104 = 0;
  *(uint32_t*)0x20000108 = 0;
  *(uint32_t*)0x2000010c = 3;
  *(uint32_t*)0x20000110 = 0;
  *(uint32_t*)0x20000118 = -1;
  memset((void*)0x2000011c, 0, 12);
  syz_io_uring_setup(/*entries=*/0x1526, /*params=*/0x20000100, /*ring_ptr=*/0,
                     /*sqes_ptr=*/0);
  *(uint32_t*)0x20000140 = 0;
  syscall(__NR_ioctl, /*fd=*/r[1], /*cmd=*/0x4c02, /*arg=*/0x20000140ul);
  return 0;
}
