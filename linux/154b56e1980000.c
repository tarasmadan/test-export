// https://syzkaller.appspot.com/bug?id=badad2dede6b94b2b829498e485cd76573e37d4f
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

void execute_one(void)
{
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  memcpy((void*)0x20000600, "hfsplus\000", 8);
  memcpy((void*)0x20000640, "./file0\000", 8);
  memset((void*)0x20000680, 0, 1);
  memcpy(
      (void*)0x20001a00,
      "\x78\x9c\xec\xdd\x4f\x6b\x1c\xe7\x1d\x07\xf0\xef\xac\x64\xd9\x72\xc1\x51"
      "\x12\x3b\x49\x4b\x4b\x85\x7d\x68\x89\xa9\xad\xd5\x26\x8e\x0e\x05\xbb\xa5"
      "\x14\x1d\x42\x09\xf4\x92\x4b\x0e\xc2\x5e\xc7\xc2\x6b\x25\x48\x9b\xa2\x84"
      "\x52\xe4\xfe\xbd\xf6\x1d\x24\x3d\xc8\xe7\x9e\x7a\x28\x3d\x18\xd2\x73\xdf"
      "\x82\xa0\x87\x1c\x0a\xbd\xeb\xe6\x32\xb3\xb3\xab\x8d\x25\xcb\x2b\xc7\xd1"
      "\xae\x9a\xcf\x07\x9e\x7d\x9e\xd9\x67\xe6\xf9\xf3\xf3\xcc\xa3\x9d\x59\xcc"
      "\x06\xf8\xc6\x5a\x7e\x37\xa7\x1e\xa6\xc8\xf2\xe5\xb7\x37\xcb\xed\x9d\xed"
      "\x56\x67\x67\xbb\x75\xaf\x5f\x4e\x72\x3a\x49\x23\x99\xee\x65\x29\xd6\x92"
      "\xe2\xf3\xe4\x46\x7a\x29\xdf\x2e\xdf\xac\x9b\x2b\x9e\xd4\xcf\x7b\x9f\xbe"
      "\xb5\xf4\x45\xf3\xc1\xfd\xde\xd6\x74\x9d\xaa\xfd\x1b\x87\x1d\x37\x9a\xad"
      "\x3a\x65\x3e\xc9\x54\x9d\x1f\xd1\xf4\x93\xda\xbb\xf9\x6c\xed\x0d\x29\x06"
      "\x33\x2c\x03\x76\xa9\x1f\x38\x18\xb7\x47\xfb\x6c\x1d\xe5\xf0\xaf\x78\xdd"
      "\x02\x93\xa0\xe8\xfd\xdd\xdc\x67\x2e\x39\x9b\xe4\x4c\xfd\x39\x20\xf5\xea"
      "\xd0\x38\xde\xd1\x3d\x7f\x47\x5a\xe5\x00\x00\x00\xe0\x84\x7a\x61\x37\xbb"
      "\xd9\xcc\xb9\x71\x8f\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x4e\x92\xfa"
      "\xf7\xff\x8b\x3a\x35\xfa\xe5\xf9\x14\xfd\xdf\xff\x9f\xa9\xdf\x4b\x5d\x3e"
      "\xd1\x1e\x8e\x7b\x00\x00\x00\x00\x00\x00\x00\x00\xf0\x1c\x7c\xbf\xd8\xda"
      "\xcd\x66\xce\xf5\xb7\x1f\x15\xd5\x77\xfe\x17\xab\x8d\xf3\xd5\xeb\xb7\xf2"
      "\x51\x36\xd2\xce\x7a\xae\x64\x33\x2b\xe9\xa6\x9b\xf5\x34\x93\xcc\x0d\x35"
      "\x34\xb3\xb9\xd2\xed\xae\x37\x47\x38\x72\xf1\xc0\x23\x17\x8f\x67\xbe\x00"
      "\x00\x00\x00\x00\x00\x00\xf0\x7f\xea\xb7\x59\xde\xfb\xfe\x1f\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x26\x41\x91\x4c\xf5\xb2\x2a\x9d\xef\x97"
      "\xe7\xd2\x98\x4e\x72\x26\xc9\x4c\xb9\xdf\x56\xf2\x8f\x7e\xf9\x24\x7b\x38"
      "\xee\x01\x00\x00\x00\xc0\x31\x78\x61\x37\xbb\xd9\xcc\xb9\xfe\xf6\xa3\xa2"
      "\xba\xe7\x7f\xa5\xba\xef\x3f\x93\x8f\xb2\x96\x6e\x56\xd3\x4d\x27\xed\xdc"
      "\xaa\x9e\x05\xf4\xee\xfa\x1b\x3b\xdb\xad\xce\xce\x76\xeb\x5e\x99\xf6\xb7"
      "\xfb\x93\xff\x1e\x69\x18\x55\x8b\xe9\x3d\x7b\x38\xb8\xe7\x85\x6a\x8f\x62"
      "\x70\xc4\x72\x7e\x9e\x5f\xe6\x72\xe6\xf3\x4e\xd6\xb3\x9a\x5f\x65\x25\xdd"
      "\xb4\x33\x9f\x9f\x55\xa5\x95\x14\x99\xab\x9f\x5e\xcc\xf5\xc7\x79\xf0\x78"
      "\x6f\x7c\x69\xeb\x9d\xa7\x8d\xf5\xb5\x6a\x24\xb3\xb9\x9d\xd5\x6a\x6c\x57"
      "\x72\x33\x1f\xa4\x93\x5b\x69\x54\x73\xa8\xf6\x39\xbc\xc7\xfb\x65\x74\x8a"
      "\xa9\x5c\xaf\x8c\x18\xa3\x5b\x75\x5e\xce\xe8\xcf\x75\x3e\x19\xe6\xaa\x88"
      "\x9c\x1a\x44\x64\xa1\x8e\x7d\x19\x8d\x17\x0f\x8f\xc4\x11\xcf\x93\xc7\x7b"
      "\x6a\xa6\x31\x78\x06\x75\x7e\x94\x98\x5f\xbf\x7e\xa4\x98\x9f\xad\xf3\x32"
      "\xd6\x7f\x9c\xe8\x98\x2f\x0e\x9d\x7d\xaf\x1c\x1e\x89\xe4\xe2\xbf\xbf\xf7"
      "\xb7\x3b\x9d\xb5\xbb\x77\x6e\x6f\x5c\x9e\x9c\x29\x3d\xa3\xc7\x23\xd1\x1a"
      "\x8a\xc4\xab\xdf\xa8\x48\xcc\xd4\xd1\xe8\xad\xa2\x8d\x5c\x18\xd4\x3c\x7d"
      "\xb5\xbc\x58\x1d\x7b\x2e\xab\xf9\x45\x3e\xc8\xad\xb4\xf3\x66\x96\xf2\x66"
      "\x16\xf3\x46\xde\xc8\x42\x96\x72\x6d\x28\xae\x17\x46\xb8\xd6\x1a\x47\xbb"
      "\xd6\x2e\xfd\xb0\x2e\xcc\x26\xf9\x53\x9d\x4f\x86\x32\xae\x2f\x0e\xc5\x75"
      "\x78\xa5\x9b\xab\xea\x86\xdf\xd9\x8b\xd2\x4b\xcf\x7f\x45\x9a\xfe\x4e\x5d"
      "\x28\xfb\xf8\x5d\x9d\x4f\x86\xc7\x23\xd1\x1c\x8a\xc4\xcb\x87\x47\xe2\x2f"
      "\x8f\xca\xd7\x8d\xce\xda\xdd\xf5\x3b\x2b\x1f\x8e\xd8\xdf\x0f\xea\xbc\xbc"
      "\x6c\xff\x30\x51\x6b\x73\x79\xbe\xbc\x54\xfe\x63\x55\x5b\x5f\x3e\x3b\xca"
      "\xba\x97\x0f\xac\x6b\x56\x75\xe7\x07\x75\x8d\x7d\x75\x17\x06\x75\x4f\xbb"
      "\x52\x67\xea\xcf\x70\xfb\x5b\x5a\xac\xea\x5e\x3d\xb0\xae\x55\xd5\xbd\x36"
      "\x54\x77\xd0\xa7\x1c\x00\x26\xde\xd9\xd7\xcf\xce\xcc\xfe\x67\xf6\x5f\xb3"
      "\x9f\xcd\xfe\x7e\xf6\xce\xec\xdb\x67\x7e\x7a\x7a\xe9\xf4\x77\x67\x72\xea"
      "\x9f\xd3\x7f\x9f\xfa\x6b\xe3\x41\xe3\xc7\xc5\xeb\xf9\x2c\xbf\xd9\xbb\xff"
      "\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x9e\xdd\xc6\xc7\x9f\xdc\x5d\xe9\x74\xda\xeb\x0a"
      "\x0a\x0a\x0a\x83\xc2\xb8\x57\x26\xe0\xeb\x76\xb5\x7b\xef\xc3\xab\x1b\x1f"
      "\x7f\xf2\xa3\xd5\x7b\x2b\xef\xb7\xdf\x6f\xaf\xb5\x5a\xcd\x6b\x8b\x4b\xd7"
      "\x96\x16\xaf\x5d\xbd\xbd\xda\x69\x2f\xf4\x5e\xc7\x3d\x4c\xe0\x6b\xb0\xf7"
      "\x47\x7f\xdc\x23\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x46\x75\x1c\xff"
      "\x9d\x60\xdc\x73\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x4e\xb6\xe5\x77\x73\xea\x61\x8a\x34\x17"
      "\xae\x2c\xa4\x48\xb2\xdd\xea\xec\x6c\xb7\x3a\x65\x5d\x3f\xef\x99\x4e\xd2"
      "\x48\x52\xfc\x3a\x29\x3e\x4f\x6e\xa4\x97\x32\x37\xd4\x5c\xf1\xa4\x7e\xde"
      "\xfb\xf4\xad\xa5\x2f\x9a\x0f\xee\xef\xb5\x35\xdd\xdf\xbf\x71\xd8\x71\xa3"
      "\xd9\xaa\x53\xe6\x93\x4c\xd5\xf9\xf3\x6a\xef\xe6\x57\x6e\xaf\x18\xcc\xb0"
      "\x0c\xd8\xa5\x7e\xe0\x60\xdc\xfe\x17\x00\x00\xff\xff\xd2\x4f\x0c\x81",
      1511);
  syz_mount_image(/*fs=*/0x20000600, /*dir=*/0x20000640, /*flags=MS_RDONLY*/ 1,
                  /*opts=*/0x20000680, /*chdir=*/0, /*size=*/0x5e7,
                  /*img=*/0x20001a00);
  memcpy((void*)0x20000080, "./file0\000", 8);
  syscall(__NR_mount, /*src=*/0ul, /*dst=*/0x20000080ul, /*type=*/0ul,
          /*flags=MS_I_VERSION|MS_UNBINDABLE|MS_REMOUNT|MS_NOEXEC*/ 0x820028ul,
          /*opts=*/0ul);
  memcpy((void*)0x20000540, "./file0\000", 8);
  syscall(__NR_chdir, /*dir=*/0x20000540ul);
  memcpy((void*)0x20000040, "./bus\000", 6);
  syscall(__NR_creat, /*file=*/0x20000040ul, /*mode=*/0ul);
  memcpy((void*)0x20000180, "./file1\000", 8);
  memcpy((void*)0x200001c0, "./bus\000", 6);
  syscall(__NR_rename, /*old=*/0x20000180ul, /*new=*/0x200001c0ul);
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