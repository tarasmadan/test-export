// https://syzkaller.appspot.com/bug?id=d2b0d0fc9db2d47a00d0b2136ac738eaf8e53c9c
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
  memcpy((void*)0x20000000, "hfsplus\000", 8);
  memcpy((void*)0x20000040, "./file1\000", 8);
  memcpy(
      (void*)0x20000a40,
      "\x78\x9c\xec\xdd\x4f\x6c\x1c\x57\x1d\x07\xf0\xef\x6e\x76\xd7\xde\x50\xa5"
      "\x4e\x9b\xa4\x01\x55\x22\x6a\xa4\x82\x88\x48\x9c\x58\x49\x31\x97\x06\x84"
      "\x50\x24\x2a\x54\x95\x03\xe2\x68\x25\x4e\x63\x65\x93\x56\x8e\x8b\x9c\x08"
      "\x41\xca\xdf\x03\x17\x0e\xbd\x53\x24\x72\xe3\x02\x12\xf7\xa0\x72\x06\x4e"
      "\xbd\xfa\x58\x09\x89\x4b\x4f\x01\x24\x16\xcd\xec\xac\xbd\xb6\x13\x67\xd7"
      "\x8e\xbd\xb6\xfa\xf9\x44\xb3\xef\xcd\xbc\x3f\xf3\xde\x6f\x66\x76\x76\x67"
      "\x15\x39\xc0\x67\xd6\x95\x33\x69\x3c\x4c\x2d\x57\xce\xbc\xb1\x5c\xac\xaf"
      "\x3c\x98\xe9\xac\x3c\x98\xb9\xd5\xcf\x27\x99\x48\x52\x4f\x1a\xbd\x24\xb5"
      "\xdb\x49\xed\xa3\xe4\x72\x7a\x4b\x3e\x5f\x6c\xac\xba\xab\x3d\x69\x3f\x1f"
      "\x2c\xcc\xbe\xf5\xf1\xa7\x2b\x9f\xf4\xd6\x1a\xd5\x52\xd6\xaf\x6f\xd5\x6e"
      "\x93\x4b\xf5\xc7\x6c\xbc\x5f\x2d\x39\x95\xe4\x50\x95\xee\xc0\xba\xfe\xae"
      "\x6e\xe8\xaf\x35\x72\x77\xb5\xd5\x19\x16\x01\x3b\xdd\x0f\x1c\x8c\x5b\x33"
      "\x49\x77\x9d\x1f\x9c\x58\x2b\x79\xaa\xe1\xaf\x5b\x60\xdf\xaa\xf5\xee\x9b"
      "\x9b\x2e\xe8\xa9\xe4\x70\x92\xc9\xea\x73\x40\xef\xae\xd8\xbb\x67\xef\x4f"
      "\xdd\x89\xa1\xaa\xdd\xdf\xf5\x81\x00\x00\x00\xc0\xf8\x3d\xff\xeb\xf2\x2b"
      "\xfc\x91\x71\x8f\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0e\x92\xea\xef"
      "\xff\xd7\xaa\xa5\xde\xcf\x9f\x4a\xad\xff\xf7\xff\x5b\xd5\xb6\x54\xf9\x83"
      "\x64\x72\xe3\x86\x87\xe3\x19\x07\x00\x00\x00\x00\x00\x00\x00\xec\xc4\x77"
      "\x9e\xdb\xb0\xe1\x8b\x8f\xf2\x28\xcb\x39\xd2\xfb\xd9\x3f\xe9\xd6\xca\xdf"
      "\xfc\x5f\x29\x57\x8e\x95\xaf\x9f\xcb\x7b\xb9\x93\xf9\x2c\xe6\x6c\x96\x33"
      "\x97\xa5\x2c\x65\x31\xe7\x93\x4c\x95\xe5\xcd\xf2\xb5\xb5\x3c\xb7\xb4\xb4"
      "\x78\x7e\x88\x96\x17\x56\x5b\x66\xa0\xe5\x85\x21\x67\xd0\xde\xd1\xfc\x01"
      "\x00\x00\x00\x00\x00\x00\xe0\x40\x68\x8c\xde\xe4\xa7\xb9\x92\x23\xbb\x31"
      "\x16\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xd8\xae\x5a\x72\xa8\x97"
      "\x94\xcb\xb1\x7e\x7e\x2a\xf5\x46\x92\xc9\x24\xad\xa2\xde\xfd\xe4\xef\xfd"
      "\xfc\xbe\xf4\xdb\xbf\x0c\xae\x75\xff\xdb\x2d\x6d\xaa\xf6\x70\x2f\xc7\x04"
      "\x00\x00\x00\x63\xf2\xfc\xa3\x3c\xca\x72\x8e\xf4\xd7\xbb\xb5\xf2\x3b\xff"
      "\x89\xf2\x7b\xff\x64\xde\xcb\xed\x2c\x65\x21\x4b\xe9\x64\x3e\xd7\xca\x67"
      "\x01\xbd\x6f\xfd\xf5\x95\x07\x33\x9d\x95\x07\x33\xb7\x8a\x65\x73\xbf\xdf"
      "\xf8\xd7\x48\xc3\x28\x7b\x4c\xef\xd9\xc3\xe3\xf7\x7c\xb2\xac\xd1\xce\xf5"
      "\x2c\x94\x5b\xce\xe6\x6a\xde\x49\x27\xd7\x52\x2f\x5b\x16\x4e\xf6\xc7\xf3"
      "\xf8\x71\xbd\x5f\x8c\xa9\xf6\x7a\x65\xc8\x91\x5d\xab\xd2\x62\xe6\xbf\x49"
      "\x73\xa4\x59\x6d\x47\x6d\xe8\x9a\x53\x65\x44\x8a\x11\xf5\x22\x32\x5d\xb5"
      "\x2d\xa2\x71\x74\xeb\x48\x8c\x78\x74\xfa\x7b\xea\xc7\xfe\x7c\xea\xab\x4f"
      "\x7e\x8e\x3d\xcb\x98\x2f\xf7\x92\xd7\x7e\xdf\x4b\x8b\xf9\xfc\x72\xa4\x98"
      "\xec\xb6\x8d\x91\xb8\x30\x70\xf6\x9d\xd8\x3a\x12\xc9\x97\xfe\xfc\x87\xef"
      "\xdf\xe8\xdc\xbe\x39\x71\xfd\xce\x99\xfd\x33\xa5\x11\x4c\x0c\x3c\x41\xdb"
      "\x18\x89\x99\x81\x48\xbc\x34\x6c\x24\x6e\x1c\xd4\x48\x0c\x9a\x2e\x23\x71"
      "\x7c\x75\xfd\x4a\xbe\x9d\xef\xe5\x4c\x4e\xe5\xcd\x2c\x66\x21\x3f\xcc\x5c"
      "\x96\x32\x9f\x53\xf9\x56\xe6\x72\x28\x73\xd5\xf9\x5c\xbc\x4e\x6d\x1d\xa9"
      "\xcb\xeb\xd6\xde\x7c\xda\x48\x5a\xe5\x71\x69\x56\xef\xa2\xc3\x8f\x69\x29"
      "\x73\x79\xa5\x6c\x7b\x24\x0b\xf9\x6e\xde\xc9\xb5\xcc\xe7\x52\xf9\xef\x42"
      "\xce\xe7\xb5\x5c\xcc\xc5\xcc\x0e\x1c\xe1\xe3\x43\x5c\xf5\xf5\xd1\xde\x69"
      "\x4f\x7f\x79\xe0\x61\xf2\xaf\x92\xb4\x87\x6b\xb7\x07\x8a\x81\x1d\x5d\xbd"
      "\x3b\x0d\x9e\xf5\xd3\xe5\x75\x70\x74\xdd\x96\xb5\x28\xbd\xf0\xec\xef\x47"
      "\x8d\x2f\x54\x99\x62\x1f\x3f\xab\xd2\xfd\x61\x63\x24\xce\x0f\x44\xe2\xc5"
      "\xad\x23\xf1\xbb\xf2\x6d\xe5\x4e\xe7\xf6\xcd\xc5\x1b\x73\xef\x0e\xb9\xbf"
      "\x57\xab\xb4\xb8\x8e\x7e\xb1\xaf\xee\x12\xc5\xf9\xf2\x42\x71\xb0\xca\xb5"
      "\xf5\x67\x47\x51\xf6\xe2\xc6\xb2\xc9\x5e\xbc\x5a\xd5\x2f\x2e\xbd\xb2\xf5"
      "\x77\xdc\xa2\xec\xf8\x6a\xd9\xd3\xae\xd4\x56\xf5\x19\x6e\x73\x4f\x17\xca"
      "\xb2\x97\x1e\x5b\x36\x53\x96\x9d\x1c\x28\x5b\xf7\x79\xeb\x72\xef\xf3\x16"
      "\x00\xfb\xde\xe1\xaf\x1c\x6e\xb5\xff\xd9\xfe\x5b\xfb\xc3\xf6\xcf\xdb\x37"
      "\xda\x6f\x4c\x7e\x73\xe2\x6b\x13\x2f\xb7\xd2\xfc\x6b\xf3\xeb\x8d\xe9\x43"
      "\xaf\xd6\x5f\xae\xfd\x29\x1f\xe6\xc7\x6b\xdf\xff\x01\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80"
      "\xed\xbb\x73\xf7\xde\xcd\xb9\x4e\x67\x7e\x71\x43\xa6\xdb\xed\xfe\xe4\x09"
      "\x45\xbb\x98\x69\x27\xe9\x6f\x49\x9e\xd6\xaa\x99\xa7\xd7\xd9\x9d\x4c\x2b"
      "\x49\x99\x69\xf4\x33\xa3\xf5\x33\x31\x54\xe5\xd6\xda\xd1\x79\xfd\x8f\x3b"
      "\x19\x73\x73\xd4\x56\xc9\x33\x09\x54\xa3\x3a\xc9\xee\xde\xbb\xf9\xef\x6e"
      "\xb7\xbb\xb7\x87\xe9\xfd\xe7\x1e\x77\x86\x37\xb7\x38\xe7\xd7\x32\xdd\xca"
      "\xa6\xa2\xee\x50\xcd\xc7\x96\xf9\x4f\xf7\xd9\x75\x38\xce\x77\x25\x60\x2f"
      "\x9c\x5b\xba\xf5\xee\xb9\x3b\x77\xef\x7d\x75\xe1\xd6\xdc\xdb\xf3\x6f\xcf"
      "\xdf\x9e\xbd\x78\x71\x76\x7a\xf6\xe2\xa5\x7f\x9c\xbb\xbe\xd0\x99\x9f\xee"
      "\xbd\x8e\x7b\x94\xc0\x6e\x58\xbb\xe9\x8f\x7b\x24\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\xc0\xb0\xf6\xe2\xbf\x25\x3c\x61\xd7\xff\xdb\xe3\xa9\x02\x00"
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
      "\x00\x00\x07\xd4\x95\x33\x13\x55\xee\xec\x74\xf1\xba\xf2\x60\xa6\x53\x2c"
      "\xfd\xfc\x6a\xc5\xb2\x5a\x3d\x49\xed\x47\x49\xed\xa3\xe4\x72\x7a\x4b\xa6"
      "\x06\xba\xab\x3d\x69\x3f\x1f\x2c\xcc\xbe\xf5\xf1\xa7\x2b\x9f\xf4\xd6\x1a"
      "\xd5\x52\xd6\xaf\xaf\x6b\xd7\xdc\xce\x2c\xee\x57\x4b\x4e\x25\x39\x54\xa5"
      "\x83\x26\x77\xd0\xdf\xd5\x2a\xdd\xd6\xc8\x4a\xb5\xd5\x19\x16\x01\x3b\xdd"
      "\x0f\x1c\x8c\xdb\xff\x03\x00\x00\xff\xff\xba\x74\x0f\xc9",
      1688);
  syz_mount_image(/*fs=*/0x20000000, /*dir=*/0x20000040,
                  /*flags=MS_NOATIME*/ 0x400, /*opts=*/0x20000080, /*chdir=*/1,
                  /*size=*/0x698, /*img=*/0x20000a40);
  memcpy((void*)0x20000080, "./file2\000", 8);
  memcpy((void*)0x200000c0, "./file1\000", 8);
  syscall(__NR_rename, /*old=*/0x20000080ul, /*new=*/0x200000c0ul);
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
