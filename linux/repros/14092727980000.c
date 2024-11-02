// https://syzkaller.appspot.com/bug?id=f86606dfb403cc9435c0ed5f17e1d80c110cddef
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/loop.h>

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif
#ifndef __NR_open_tree
#define __NR_open_tree 428
#endif
#ifndef __NR_renameat2
#define __NR_renameat2 316
#endif

static unsigned long long procid;

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

static uint64_t current_time_ms(void)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static bool write_file(const char* file, const char* what, ...)
{
  char buf[1024];
  va_list args;
  va_start(args, what);
  vsnprintf(buf, sizeof(buf), what, args);
  va_end(args);
  buf[sizeof(buf) - 1] = 0;
  int len = strlen(buf);
  int fd = open(file, O_WRONLY | O_CLOEXEC);
  if (fd == -1)
    return false;
  if (write(fd, buf, len) != len) {
    int err = errno;
    close(fd);
    errno = err;
    return false;
  }
  close(fd);
  return true;
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

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  for (int i = 0; i < 100; i++) {
    if (waitpid(-1, status, WNOHANG | __WALL) == pid)
      return;
    usleep(1000);
  }
  DIR* dir = opendir("/sys/fs/fuse/connections");
  if (dir) {
    for (;;) {
      struct dirent* ent = readdir(dir);
      if (!ent)
        break;
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        continue;
      char abort[300];
      snprintf(abort, sizeof(abort), "/sys/fs/fuse/connections/%s/abort",
               ent->d_name);
      int fd = open(abort, O_WRONLY);
      if (fd == -1) {
        continue;
      }
      if (write(fd, abort, 1) < 0) {
      }
      close(fd);
    }
    closedir(dir);
  } else {
  }
  while (waitpid(-1, status, __WALL) != pid) {
  }
}

static void reset_loop()
{
  char buf[64];
  snprintf(buf, sizeof(buf), "/dev/loop%llu", procid);
  int loopfd = open(buf, O_RDWR);
  if (loopfd != -1) {
    ioctl(loopfd, LOOP_CLR_FD, 0);
    close(loopfd);
  }
}

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  write_file("/proc/self/oom_score_adj", "1000");
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter = 0;
  for (;; iter++) {
    reset_loop();
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      execute_one();
      exit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      sleep_ms(10);
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      if (current_time_ms() - start < 5000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  memcpy((void*)0x20000600, "hfsplus\000", 8);
  memcpy((void*)0x20000040, "./file1\000", 8);
  memcpy((void*)0x20000080, "part", 4);
  *(uint8_t*)0x20000084 = 0x3d;
  sprintf((char*)0x20000085, "0x%016llx", (long long)0x40);
  *(uint8_t*)0x20000097 = 0x2c;
  memcpy((void*)0x20000098, "nodecompose", 11);
  *(uint8_t*)0x200000a3 = 0x2c;
  memcpy((void*)0x200000a4, "part", 4);
  *(uint8_t*)0x200000a8 = 0x3d;
  sprintf((char*)0x200000a9, "0x%016llx", (long long)7);
  *(uint8_t*)0x200000bb = 0x2c;
  memcpy((void*)0x200000bc, "part", 4);
  *(uint8_t*)0x200000c0 = 0x3d;
  sprintf((char*)0x200000c1, "0x%016llx", (long long)0xc);
  *(uint8_t*)0x200000d3 = 0x2c;
  memcpy((void*)0x200000d4, "uid", 3);
  *(uint8_t*)0x200000d7 = 0x3d;
  sprintf((char*)0x200000d8, "0x%016llx", (long long)0);
  *(uint8_t*)0x200000ea = 0x2c;
  memcpy((void*)0x200000eb, "barrier", 7);
  *(uint8_t*)0x200000f2 = 0x2c;
  memcpy((void*)0x200000f3, "nls", 3);
  *(uint8_t*)0x200000f6 = 0x3d;
  memcpy((void*)0x200000f7, "macinuit", 8);
  *(uint8_t*)0x200000ff = 0x2c;
  memcpy((void*)0x20000100, "gid", 3);
  *(uint8_t*)0x20000103 = 0x3d;
  sprintf((char*)0x20000104, "0x%016llx", (long long)0xee00);
  *(uint8_t*)0x20000116 = 0x2c;
  *(uint8_t*)0x20000117 = 0;
  memcpy(
      (void*)0x20000640,
      "\x78\x9c\xec\xdd\xcf\x6b\x1c\xe7\x19\x07\xf0\xef\xac\xd6\xb2\xe5\x82\xb3"
      "\x49\xec\x24\x2d\x2d\x15\xf6\xa1\x25\xa6\xb6\x56\x9b\x38\x3a\x14\xea\x96"
      "\x52\x74\x08\x25\xd0\x4b\x2e\x39\x08\x7b\x1d\x0b\xaf\x95\x20\x6d\x8a\x12"
      "\x4a\x91\xfb\xf3\xda\xff\x20\x29\x45\x3e\xf7\xd4\x43\xe9\xc1\x90\x9e\x7b"
      "\xed\x51\xd0\x43\x0e\x85\xde\x75\x73\x99\xd9\x59\x69\x6d\x2b\xb2\x14\x2b"
      "\xda\x55\xf2\xf9\xc0\xbb\xef\x3b\xfb\xce\xbc\xf3\xcc\xe3\x99\x57\x3b\xb3"
      "\x98\x0d\xf0\xb5\xb5\xf8\x76\x4e\x3d\x48\x91\xc5\xcb\x6f\xae\x97\xcb\x5b"
      "\x9b\x9d\xde\xd6\x66\xe7\xee\xb0\x9d\xe4\x74\x92\x46\xd2\x1c\x54\x29\x56"
      "\x92\xe2\xd3\xe4\x7a\x06\x25\xdf\x2c\xdf\xac\x87\x2b\x3e\x6f\x3f\xef\x7c"
      "\xfc\xc6\xc2\x67\xed\xfb\xf7\x92\xa2\x39\x18\xab\x39\x5c\xbf\xb1\xdf\x76"
      "\x07\xb3\x51\x97\xcc\x26\x99\xaa\xeb\xa3\x1a\xef\xc6\x33\x8f\x57\xec\x1c"
      "\x61\x99\xb0\x4b\xc3\xc4\xc1\xb8\x3d\x7c\xc2\xc6\x61\x36\x7f\xc6\xeb\x16"
      "\x98\x64\xad\xe4\x6c\x92\x33\xf5\xe7\x80\xd4\xb3\x43\x63\xcc\x61\x3d\xb3"
      "\x43\xcd\x72\x00\x00\x00\x70\x42\x3d\xb7\x9d\xed\xac\xe7\xdc\xb8\xe3\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x80\x93\xa4\x48\xa6\x06\x55\x55\x1a\xc3"
      "\xf6\x6c\x8a\xe1\xef\xff\x4f\xd7\xef\xa5\x6e\x9f\x68\x0f\xc6\x1d\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x1c\x81\xef\x6e\x67\x3b\xeb\x39\x37\x5c\x7e\x58"
      "\x54\xdf\xf9\x5f\xac\x16\xce\x57\xaf\xdf\xc8\x07\x59\x4b\x37\xab\xb9\x92"
      "\xf5\x2c\xa5\x9f\x7e\x56\xd3\x4e\xd2\x1a\x19\x68\x7a\x7d\xa9\xdf\x5f\x6d"
      "\x3f\x75\xcb\x22\xd9\x78\x34\x84\xc1\x96\xf3\xc7\x70\xb0\x00\x00\x00\x00"
      "\x00\x00\x00\xf0\xd5\xf5\x9b\x2c\xee\x7e\xff\x0f\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x93\xa0\x48\xa6\x06\x55\x55\xce\x0f\xdb\xad\x34\x9a"
      "\x49\xce\x24\x99\x2e\xd7\xdb\x48\xfe\x31\x6c\x9f\x64\x0f\xc6\x1d\x00\x00"
      "\x00\x00\x1c\x83\xe7\xb6\xb3\x9d\xf5\x9c\x1b\x2e\x3f\x2c\xaa\x7b\xfe\x97"
      "\xaa\xfb\xfe\x33\xf9\x20\x2b\xe9\x67\x39\xfd\xf4\xd2\xcd\xcd\xea\x59\xc0"
      "\xe0\xae\xbf\xb1\xb5\xd9\xe9\x6d\x6d\x76\xee\x96\xe5\xc9\x71\x7f\xfc\xbf"
      "\x43\x85\x51\x8d\x98\xc1\xb3\x87\xbd\xf7\x3c\x57\xad\x71\x61\x67\x8b\xc5"
      "\xfc\x2c\xbf\xc8\xe5\xcc\xe6\xad\xac\x66\x39\xbf\xcc\x52\xfa\xe9\x66\x36"
      "\x3f\xad\x5a\x4b\x29\xd2\xaa\x9f\x5e\xb4\x86\x71\xee\x1d\xef\xf5\x47\x96"
      "\xde\x7a\x5a\xac\xaf\x54\x91\xcc\xe4\x56\x96\xab\xd8\xae\xe4\x46\xde\x4b"
      "\x2f\x37\xd3\xa8\x8e\xa1\x5a\x67\xff\x3d\xde\x2b\xb3\x53\xfc\xa8\x76\xc0"
      "\x1c\xdd\xac\xeb\xf2\x88\xfe\x54\xd7\x93\xa1\x55\x65\xe4\xd4\x4e\x46\xe6"
      "\xea\xdc\x97\xd9\x78\x7e\xff\x4c\x1c\xf2\x3c\x79\x7c\x4f\xed\x34\x76\x9e"
      "\x41\x9d\xff\xf7\xd1\xe7\xfc\x6c\x5d\x97\xb9\xfe\xc3\x44\xe7\x7c\x7e\xe4"
      "\xec\x7b\x69\xff\x9c\x27\x17\xff\xf3\x9d\xbf\xdd\xee\xad\xdc\xb9\x7d\x6b"
      "\xed\xf2\xe4\x1c\xd2\x17\xf4\x78\x26\x3a\x23\x99\x78\xf9\x6b\x95\x89\xe9"
      "\x3a\x1b\x83\x59\xf4\x70\xb3\xe5\xc5\x6a\xdb\x73\x59\xce\xcf\xf3\x5e\x6e"
      "\xa6\x9b\xd7\xb3\x90\xd7\x33\x9f\xd7\xf2\x5a\xe6\xb2\x90\x6b\x23\x79\xbd"
      "\x70\x80\xf9\xad\x71\xb8\x6b\xed\xd2\xf7\xeb\xc6\x4c\x92\x3f\xd6\xf5\x64"
      "\x28\xf3\xfa\xfc\x48\x5e\x47\x67\xba\x56\xd5\x37\xfa\xce\x20\x4b\xe5\xc9"
      "\xf4\xc2\xd1\xff\x15\x68\x7e\xab\x6e\x94\xfb\xf8\x6d\x5d\x4f\x86\xc7\x33"
      "\xd1\x1e\x39\x5f\x5e\xdc\x3f\x13\x7f\x7e\x58\xbe\xae\xf5\x56\xee\xac\xde"
      "\x5e\x7a\xff\x80\xfb\xfb\x5e\x5d\x97\x99\xfe\xfd\x44\xcd\xcd\xe5\xf9\xf2"
      "\x42\xf9\x8f\x55\x2d\x3d\x7a\x76\x94\x7d\x2f\xee\xd9\xd7\xae\xfa\xce\xef"
      "\xf4\x35\x9e\xe8\xbb\xb0\xd3\xf7\xb4\x2b\x75\xba\xfe\x0c\xf7\xe4\x48\xf3"
      "\x55\xdf\xcb\x7b\xf6\x75\xaa\xbe\x57\x46\xfa\xf6\xfa\x94\x03\xc0\x84\xda"
      "\xfd\x4e\xfb\xec\xab\x67\xa7\x67\xfe\x3b\xf3\xaf\x99\x4f\x66\x7e\x37\x73"
      "\x7b\xe6\xcd\x33\x3f\x39\xbd\x70\xfa\xdb\xd3\x39\xf5\xcf\xe6\xdf\xa7\xfe"
      "\xda\xb8\xdf\xf8\x61\xf1\x6a\x3e\xc9\xaf\x77\xef\xff\x01\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x80\x2f\x6e\xed\xc3\x8f\xee\x2c\xf5\x86\x3f\x03\xd0\xeb\x75\x57\x07\xef"
      "\x7c\xc5\x1a\x7f\xc9\x44\x84\xa1\x71\xd2\x1a\xcd\xfa\xca\x98\x94\x78\x8e"
      "\xaf\x31\xc6\x49\x09\x38\x16\x57\xfb\x77\xdf\xbf\xba\xf6\xe1\x47\x3f\x58"
      "\xbe\xbb\xf4\x6e\xf7\xdd\xee\x4a\xa7\xd3\xbe\x36\xbf\x70\x6d\x61\xfe\xda"
      "\xd5\x5b\xcb\xbd\xee\xdc\xe0\x75\xdc\x61\x02\x5f\x82\xdd\x3f\xfa\xe3\x8e"
      "\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x38\xa8\xe3\xf8\xef\x04\xe3\x3e"
      "\x46\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\xe0\x64\x5b\x7c\x3b\xa7\x1e\xa4\x48\x7b\xee\xca\x5c\xb9"
      "\xbc\xb5\xd9\xe9\x95\x65\xd8\xde\x5d\xb3\x99\xa4\x91\xa4\xf8\x55\x52\x7c"
      "\x9a\x5c\xcf\xa0\xa4\x35\x32\x5c\xf1\x79\xfb\x79\xe7\xe3\x37\x16\x3e\x6b"
      "\xdf\xbf\xb7\x3b\x56\x73\xb8\x7e\x63\xbf\xed\x0e\x66\xa3\x2e\x99\x4d\x32"
      "\x55\xd7\x47\x35\xde\x8d\x67\x1e\xaf\xd8\x39\xc2\x32\x61\x97\x86\x89\x83"
      "\x71\xfb\x7f\x00\x00\x00\xff\xff\x52\x87\x07\xdb",
      1524);
  syz_mount_image(/*fs=*/0x20000600, /*dir=*/0x20000040, /*flags=*/0,
                  /*opts=*/0x20000080, /*chdir=*/3, /*size=*/0x5f4,
                  /*img=*/0x20000640);
  memcpy((void*)0x20000280, "cgroup.controllers\000", 19);
  syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000280ul,
          /*flags=*/0x275a, /*mode=*/0);
  memcpy((void*)0x20000040, ".\000", 2);
  res = syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul, /*file=*/0x20000040ul,
                /*flags=*/0, /*mode=*/0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000340,
         "./"
         "file0aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\000",
         253);
  syscall(__NR_mknodat, /*dirfd=*/r[0], /*file=*/0x20000340ul,
          /*mode=S_IFCHR*/ 0x2000ul, /*dev=*/0x103);
  memset((void*)0x20000640, 0, 1);
  res = syscall(__NR_open_tree, /*dfd=*/r[0], /*filename=*/0x20000640ul,
                /*flags=OPEN_TREE_CLOEXEC|AT_EMPTY_PATH*/ 0x81000ul);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000080, "./file1\000", 8);
  memcpy((void*)0x20000980,
         "./"
         "file0aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\000",
         253);
  syscall(__NR_renameat2, /*oldfd=*/0xffffff9c, /*old=*/0x20000080ul,
          /*newfd=*/r[1], /*new=*/0x20000980ul, /*flags=*/0ul);
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
  loop();
  return 0;
}