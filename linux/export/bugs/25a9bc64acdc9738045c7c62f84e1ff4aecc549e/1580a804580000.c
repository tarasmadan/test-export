// https://syzkaller.appspot.com/bug?id=25a9bc64acdc9738045c7c62f84e1ff4aecc549e
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
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

static __thread int clone_ongoing;
static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  if (__atomic_load_n(&clone_ongoing, __ATOMIC_RELAXED) != 0) {
    exit(sig);
  }
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  int skip = __atomic_load_n(&skip_segv, __ATOMIC_RELAXED) != 0;
  int valid = addr < prog_start || addr > prog_end;
  if (skip && valid) {
    _longjmp(segv_env, 1);
  }
  exit(sig);
}

static void install_segv_handler(void)
{
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  syscall(SYS_rt_sigaction, 0x20, &sa, NULL, 8);
  syscall(SYS_rt_sigaction, 0x21, &sa, NULL, 8);
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_NODEFER | SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
}

#define NONFAILING(...)                                                        \
  ({                                                                           \
    int ok = 1;                                                                \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    } else                                                                     \
      ok = 0;                                                                  \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    ok;                                                                        \
  })

static void use_temporary_dir(void)
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    exit(1);
  if (chmod(tmpdir, 0777))
    exit(1);
  if (chdir(tmpdir))
    exit(1);
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

uint64_t r[1] = {0xffffffffffffffff};

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
  install_segv_handler();
  use_temporary_dir();
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  NONFAILING(memcpy((void*)0x200000000080, "ext3\000", 5));
  NONFAILING(memcpy((void*)0x200000000480, "./file0\000", 8));
  NONFAILING(memcpy((void*)0x200000000140, "quota", 5));
  NONFAILING(*(uint8_t*)0x200000000145 = 0x2c);
  NONFAILING(memcpy((void*)0x200000000146, "resgid", 6));
  NONFAILING(*(uint8_t*)0x20000000014c = 0x3d);
  NONFAILING(sprintf((char*)0x20000000014d, "0x%016llx", (long long)0xee00));
  NONFAILING(*(uint8_t*)0x20000000015f = 0x2c);
  NONFAILING(memcpy((void*)0x200000000160, "bh", 2));
  NONFAILING(*(uint8_t*)0x200000000162 = 0x2c);
  NONFAILING(memcpy((void*)0x200000000163, "noload", 6));
  NONFAILING(*(uint8_t*)0x200000000169 = 0x2c);
  NONFAILING(memcpy((void*)0x20000000016a, "data_err=ignore", 15));
  NONFAILING(*(uint8_t*)0x200000000179 = 0x2c);
  NONFAILING(memcpy((void*)0x20000000017a, "noload", 6));
  NONFAILING(*(uint8_t*)0x200000000180 = 0x2c);
  NONFAILING(*(uint8_t*)0x200000000181 = 0);
  NONFAILING(memcpy(
      (void*)0x200000000980,
      "\x78\x9c\xec\xdc\xcd\x6f\x14\xe5\x1f\x00\xf0\xef\xcc\x6e\xcb\xfb\xaf\xfc"
      "\x10\x5f\x40\xd0\x2a\x1a\x89\x2f\x2d\x2d\x2f\x72\xf0\x82\xd1\xc4\x83\x26"
      "\x26\x7a\xc0\x78\xaa\x6d\x21\xc8\x42\x0d\xad\x89\x10\xa2\xe8\x01\x8f\x86"
      "\xc4\xbb\xf1\x68\xe2\x5f\xe0\x49\x2f\x46\x3d\x99\x78\xd5\xbb\x21\x21\x86"
      "\x0b\xe8\x69\xcd\xec\xce\x94\xa5\xdd\x6d\xbb\xed\x96\xad\xcc\xe7\x93\x0c"
      "\x3c\xcf\xce\xb3\x79\x9e\xef\xcc\x3c\xbb\xcf\x3c\xcf\x4e\x03\x28\xad\xe1"
      "\xec\x9f\x24\x62\x7b\x44\xfc\x1e\x11\x43\xcd\xec\xdd\x05\x86\x9b\xff\xdd"
      "\xbe\x79\x79\xf2\xef\x9b\x97\x27\x93\xa8\xd7\xdf\xfa\x2b\x69\x94\xbb\x75"
      "\xf3\xf2\x64\x51\xb4\x78\xdf\xb6\x66\xa6\x5e\xcf\xf3\x9b\xda\xd4\x7b\xf5"
      "\xdd\x88\x89\x5a\x6d\xfa\x42\x9e\x1f\x9d\x3b\xf7\xc1\xe8\xec\xc5\x4b\x2f"
      "\x9c\x39\x37\x71\x7a\xfa\xf4\xf4\xf9\xf1\xe3\xc7\x8f\x1c\xde\x3f\x78\x6c"
      "\xfc\x68\x4f\xe2\xcc\xe2\xba\xb5\xf7\xe3\x99\x7d\x7b\x5e\x7b\xe7\xda\x1b"
      "\x93\x27\xaf\xbd\xf7\xf3\xb7\x59\x7b\xb7\xe7\xfb\x5b\xe3\x58\x91\x74\xf9"
      "\x22\xc3\xcd\xa3\xdb\xd6\xd3\x5d\x55\xb6\xf1\xed\x68\x49\x27\xd5\x3e\x36"
      "\x84\xae\x54\x22\x22\x3b\x5d\x03\x8d\xfe\x3f\x14\x95\xd8\x32\xbf\x6f\x28"
      "\x5e\xfd\xac\xaf\x8d\x03\xd6\x55\xbd\x5e\xaf\xb7\xfb\x7e\xce\x5d\xa9\x03"
      "\xf7\xb1\x24\xfa\xdd\x02\xa0\x3f\x8a\x2f\xfa\xec\xfe\xb7\xd8\xee\xd1\xd0"
      "\x63\x43\xb8\x71\xa2\x79\x03\x94\xc5\x7d\x3b\xdf\x9a\x7b\xaa\xf3\xb7\xf8"
      "\x03\x0b\xee\x6f\x7b\x69\x38\x22\x4e\x5e\xf9\xe7\xab\x6c\x8b\xd5\xcc\x43"
      "\x00\x00\x74\xe9\xfb\x6c\xfc\xf3\x7c\xbb\xf1\x5f\x1a\x0f\xb5\x94\xfb\x5f"
      "\xbe\x86\xb2\x33\x22\xfe\x1f\x11\xbb\x22\xe2\x81\x88\xd8\x1d\x11\x0f\x46"
      "\x34\xca\x3e\x1c\x11\x8f\x74\x59\xff\xc2\x15\x92\xc5\xe3\x9f\xf4\xfa\xaa"
      "\x02\x5b\xa1\x6c\xfc\xf7\x52\xbe\xb6\x75\xf7\xf8\x2f\x6d\x8c\xfb\xb2\x90"
      "\x2b\xf9\x72\xcf\x8e\x46\xfc\x03\xc9\xa9\x33\xb5\xe9\x43\xf9\x31\x39\x18"
      "\x03\x9b\xb2\xfc\xd8\x12\x75\xfc\xf0\xca\x6f\x5f\x74\xda\xd7\x3a\xfe\xcb"
      "\xb6\xac\xfe\x62\x2c\x98\xb7\xe3\x7a\x75\xc1\x04\xdd\xd4\xc4\xdc\xc4\xda"
      "\xa2\xbe\xe3\xc6\xa7\x11\x7b\xab\xed\xe2\x4f\xa2\x58\xc6\x49\x22\x62\x4f"
      "\x44\xec\x5d\x65\x1d\x67\x9e\xfd\x66\x5f\xa7\x7d\xcb\xc7\xbf\x84\x1e\xac"
      "\x33\xd5\xbf\x8e\x78\xa6\x79\xfe\xaf\xc4\x82\xf8\x0b\x49\xc7\xf5\xc9\xb1"
      "\x17\x8f\x8d\x1f\x1d\xdd\x1c\xb5\xe9\x43\xa3\xc5\x55\xb1\xd8\x2f\xbf\x5e"
      "\x7d\xb3\x53\xfd\x6b\x8a\xbf\x07\xb2\xf3\xbf\xb5\xed\xf5\x3f\x1f\xff\xce"
      "\x64\x73\xc4\xec\xc5\x4b\x67\x1b\xeb\xb5\xb3\xdd\xd7\x71\xf5\x8f\xcf\x3b"
      "\xde\xd3\xac\xf6\xfa\x1f\x4c\xde\x6e\xa4\x07\xf3\xd7\x3e\x9a\x98\x9b\xbb"
      "\x30\x16\x31\x98\xbc\xbe\xf8\xf5\xf1\x3b\xef\x2d\xf2\x45\xf9\x2c\xfe\x83"
      "\x07\xda\xf7\xff\x5d\xd5\x3b\x47\xe2\xd1\x88\xc8\x2e\xe2\xfd\x11\xf1\x58"
      "\x44\x3c\x9e\xb7\xfd\x89\x88\x78\x32\x22\x0e\x2c\x11\xff\x4f\x2f\x3f\xf5"
      "\x7e\xf7\xf1\x2f\x31\x2b\xdf\x43\x59\xfc\x53\xcb\x9d\xff\x68\x3d\xff\xdd"
      "\x27\x2a\x67\x7f\xfc\xae\xfb\xf8\x0b\xd9\xf9\x3f\xd2\x48\x1d\xcc\x5f\x59"
      "\xc9\xe7\xdf\x4a\x1b\xb8\x96\x63\x07\x00\x00\x00\xff\x15\x69\xe3\x37\xf0"
      "\x49\x3a\x32\x9f\x4e\xd3\x91\x91\xe6\x6f\xf8\x77\xc7\xd6\xb4\x36\x33\x3b"
      "\xf7\xdc\xa9\x99\x0f\xcf\x4f\x35\x7f\x2b\xbf\x33\x06\xd2\x62\xa6\x6b\xa8"
      "\x65\x3e\x74\x2c\x9f\x1b\x2e\xf2\xe3\x0b\xf2\x87\xf3\x79\xe3\x2f\x2b\x5b"
      "\x1a\xf9\x91\xc9\x99\xda\x54\xbf\x83\x87\x92\xdb\xd6\xa1\xff\x67\xfe\xac"
      "\xf4\xbb\x75\xc0\xba\xf3\xbc\x16\x94\x97\xfe\x0f\xe5\xa5\xff\x43\x79\xe9"
      "\xff\x50\x5e\xfa\x3f\x94\x57\xbb\xfe\xff\x49\x1f\xda\x01\xdc\x7b\xbe\xff"
      "\xa1\xbc\xf4\x7f\x28\x2f\xfd\x1f\xca\x4b\xff\x87\x52\xea\xf8\x6c\x7c\xba"
      "\xa6\x47\xfe\xd7\x3d\x51\x7c\x64\x6d\x94\xf6\xdc\x0f\x89\x13\x5d\xbd\x2b"
      "\xd2\x8d\xd0\xe6\x12\x24\xaa\x2b\xfe\x63\x16\xab\x4c\x6c\x6a\xbb\xab\xcf"
      "\x1f\x4c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x3d\xf2\x6f\x00"
      "\x00\x00\xff\xff\x41\x68\xe0\xe7",
      1106));
  NONFAILING(
      syz_mount_image(/*fs=*/0x200000000080, /*dir=*/0x200000000480,
                      /*flags=MS_LAZYTIME|MS_STRICTATIME|MS_SILENT*/ 0x3008000,
                      /*opts=*/0x200000000140, /*chdir=*/0xfe, /*size=*/0x452,
                      /*img=*/0x200000000980));
  NONFAILING(memcpy((void*)0x2000000001c0, "./file0\000", 8));
  NONFAILING(memcpy((void*)0x2000000002c0, "afs\000", 4));
  NONFAILING(memcpy((void*)0x200000000400, "dyn", 3));
  NONFAILING(*(uint8_t*)0x200000000403 = 0x2c);
  NONFAILING(*(uint8_t*)0x200000000404 = 0);
  syscall(__NR_mount, /*src=*/0ul, /*dst=*/0x2000000001c0ul,
          /*type=*/0x2000000002c0ul, /*flags=*/0ul, /*opts=*/0x200000000400ul);
  NONFAILING(memcpy((void*)0x2000000000c0, "./file0\000", 8));
  syscall(__NR_chdir, /*dir=*/0x2000000000c0ul);
  NONFAILING(memcpy((void*)0x200000000240, "./file1\000", 8));
  syscall(__NR_lstat, /*file=*/0x200000000240ul, /*statbuf=*/0ul);
  NONFAILING(memcpy((void*)0x2000000000c0, ".\000", 2));
  res = syscall(__NR_open, /*file=*/0x2000000000c0ul, /*flags=*/0ul,
                /*mode=*/0ul);
  if (res != -1)
    r[0] = res;
  syscall(__NR_getdents, /*fd=*/r[0], /*ent=*/0x200000001fc0ul,
          /*count=*/0xb8ul);
  return 0;
}
