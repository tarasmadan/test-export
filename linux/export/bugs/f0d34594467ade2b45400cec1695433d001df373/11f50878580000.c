// https://syzkaller.appspot.com/bug?id=f0d34594467ade2b45400cec1695433d001df373
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

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  memcpy((void*)0x400000000000, "hfsplus\000", 8);
  memcpy((void*)0x400000000040, "./file1\000", 8);
  memcpy(
      (void*)0x400000000880,
      "\x78\x9c\xec\xdd\xcd\x6f\x1c\x67\x1d\x07\xf0\xef\xac\xd7\x2f\x9b\x4a\x8e"
      "\xdb\xa6\x69\x40\x95\x30\x8d\x54\x10\x11\x89\x1d\x2b\x85\x70\x49\x40\x08"
      "\x05\xa9\x42\x55\x38\x70\xb6\x1a\xa7\xb1\xe2\xa4\xc1\x71\x51\xda\x03\x71"
      "\x01\x89\x2b\x07\xfe\x80\x72\x08\x17\x38\x81\x10\x12\x12\x52\xa4\x72\x86"
      "\x5b\xc5\xcd\xe2\x54\x09\x89\x4b\x4f\x69\x0f\x0c\x9a\xd9\x59\x7b\xed\xee"
      "\xda\x9b\x57\x3b\xf4\xf3\x89\x66\x9f\xe7\x99\x67\xe6\x99\xdf\xfc\xe6\x65"
      "\x5f\x22\x6b\x02\x7c\x6e\x5d\x38\x91\xf6\xdd\x74\x72\xe1\xc4\x6b\xb7\xaa"
      "\xf6\xc6\x9d\x85\x95\x8d\x3b\x0b\xd7\x7a\xf5\x24\x93\x49\x5a\x49\xbb\x5b"
      "\xa4\xb8\x9e\x14\x1f\x24\xe7\xd3\x9d\xf2\x85\x6a\x66\x33\x5c\x31\x6c\x3b"
      "\xbf\x5e\x3e\x7b\xf1\xc3\x8f\x37\x3e\xea\xb6\xda\xd9\x1a\xaf\x7a\xe9\x0c"
      "\x0f\xb0\x3d\xca\x5e\xac\x37\x53\x66\x93\x8c\x35\xe5\x43\xd8\x36\xde\x1b"
      "\x0f\x36\xde\xe4\x56\xb5\xd8\xcc\x4c\x95\xb0\xe3\xbd\xc4\xc1\x7e\x1b\x4f"
      "\x52\x6e\xf3\xe3\xa3\x5b\x3d\x83\x94\x63\x7d\x8d\xa1\xd7\x3b\xf0\xf4\x28"
      "\xba\xef\x9b\x7d\xba\xd7\xff\x4c\x72\x28\xc9\x54\xef\x0d\x6d\xbd\xdb\xd9"
      "\x7a\xf2\x11\xee\xe9\xbe\xee\x45\xeb\x8f\x2f\x0e\x00\x00\x00\x38\x30\x0e"
      "\xdf\xbb\x9d\xdc\xca\xf4\x7e\xc7\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x4f\x93\xe6\xf9\xff\x45\x33\xb5\x7a\xf5\xd9\x14\xbd\xe7\xff\x4f\xf4\x3d"
      "\x63\x7f\x62\x9f\xc3\x1d\x6e\xf7\xc8\xa6\x7a\x95\xbb\xad\x27\x11\x0c\x00"
      "\x00\x00\x00\x00\x00\x00\x3c\x5e\x5f\xba\x97\xdf\x5d\x2c\xcb\xe9\x5e\xbb"
      "\x2c\xea\xff\xf3\x7f\xb9\x6e\x1c\xa9\x5f\x9f\xc9\xdb\xb9\x99\xa5\xac\xe6"
      "\x64\x6e\x65\x31\x6b\x59\xcb\x6a\xe6\x93\xcc\xf4\x0d\x34\x71\x6b\x71\x6d"
      "\x6d\x75\xbe\xb7\xe6\xa7\x65\x59\x0e\x59\xf3\xf4\xc0\x35\x4f\x8f\x18\x70"
      "\xe7\x51\xec\x35\x00\x00\x00\x00\x00\x00\x00\xfc\xdf\x38\xd7\x94\x3f\xcb"
      "\x85\x4c\xef\x73\x2c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xb0\x4d"
      "\x91\x8c\x75\x8b\x7a\x3a\xd2\xab\xcf\xa4\xd5\x4e\x32\x95\x64\xa2\x5a\x6e"
      "\x3d\xf9\x47\xaf\xfe\x34\xbb\xbb\xdf\x01\x00\x00\x00\xc0\x13\x70\xf8\x5e"
      "\xee\xe5\x56\xa6\x7b\xed\xb2\xa8\xbf\xf3\x1f\xad\xbf\xf7\x4f\xe5\xed\x5c"
      "\xcf\x5a\x96\xb3\x96\x95\x2c\xe5\x52\xfd\x5b\x40\xf7\x5b\x7f\x6b\xe3\xce"
      "\xc2\xca\xc6\x9d\x85\x6b\xd5\xf4\xd9\x71\xbf\xfd\x9f\xad\xfa\x1f\xa7\xf7"
      "\x0c\xa3\x1e\x31\xdd\xdf\x1e\x06\x6f\xf9\x58\xbd\x44\x27\x97\xb3\x5c\xcf"
      "\x39\x99\x37\xf2\x56\x56\x72\x29\xad\x7a\xcd\xca\xb1\x5e\x3c\x83\xe3\x7a"
      "\xaf\x8a\xa9\x38\xd7\x55\x96\xa3\x25\xe8\x52\x53\x56\x7b\xfe\xab\xa6\x3c"
      "\x18\x66\xea\x8c\x8c\x6f\x66\x64\xae\x89\xad\xca\xc6\xb3\xbb\x67\xa2\xff"
      "\xe8\x3c\xc0\x96\xe6\xd3\xda\xfc\xe5\xe7\xc8\x7d\xe4\xfc\xdc\xae\x5b\x29"
      "\xfe\xdb\x3b\x26\x87\x7a\x73\x92\x67\xbe\xbf\x77\xce\xc7\xef\x6b\x67\x1e"
      "\xca\xce\x4c\x9c\xee\x3b\xfb\x8e\xee\x9e\x89\xe4\x2b\x7f\xfa\xfd\x8f\xae"
      "\xac\x5c\xbf\x7a\xa5\x58\x3f\x71\x70\x4e\xa3\xfb\x30\xf9\xaf\xad\xab\x66"
      "\x67\x26\x16\xfa\x32\xf1\xe2\xc8\x99\xb8\x7c\xf3\xe9\xcc\xc4\x4e\xad\xbc"
      "\xb0\x59\xbf\x90\xef\xe5\x87\x39\x91\xd9\xbc\x9e\xd5\x2c\xe7\x27\x59\xcc"
      "\x5a\x96\x32\x9b\xef\xd6\xb5\xc5\xe6\x7c\xae\x5e\x67\x76\xcf\xd4\xf9\x6d"
      "\xad\xd7\xf7\x8a\x62\xa2\x39\x2e\x63\x3b\x62\xfa\xf2\xe1\x6e\xb9\x5b\x4c"
      "\x2f\xd7\xeb\x4e\x67\x39\x3f\xc8\x5b\xb9\x94\xa5\xbc\x5a\xff\x3b\x9d\xf9"
      "\x7c\x23\x67\x72\x26\x67\xfb\x8e\xf0\x0b\x23\x5c\xf5\xad\x01\x57\xfd\x9f"
      "\x87\x07\x7f\xfc\xab\x4d\xa5\x93\xe4\x97\x4d\x79\x30\x54\x79\x7d\xb6\x2f"
      "\xaf\xfd\xf7\xdc\x99\xba\xaf\x7f\x4e\x2b\xe5\x64\x77\xbd\xe7\x1e\xd9\xbd"
      "\x71\x53\xfb\x8b\x4d\xa5\x3a\x12\x3f\x6f\xca\x83\x61\x33\x13\x53\xd9\x7c"
      "\x97\xe8\x45\xf7\x7c\x2f\x03\xe3\x03\x33\xf1\x9b\xfa\xb6\x72\x73\xe5\xfa"
      "\xd5\xd5\x2b\x8b\x37\x76\x8c\x5b\xac\x0f\xde\xde\x2b\xd9\xbe\xfb\x07\xe7"
      "\x46\x52\x9d\x2f\xcf\x55\x07\xab\x6e\x6d\x3f\x3b\xaa\xbe\xe7\x07\xf6\xcd"
      "\xd7\x7d\x47\x36\xfb\x5a\x3b\xfb\x7e\xdb\xd9\xec\xdb\xeb\x4a\x9d\x68\x3e"
      "\xc3\x7d\x76\xa4\xd3\x75\xdf\x8b\x03\xfb\x16\xea\xbe\x63\x7d\x7d\x5b\x9f"
      "\xb7\x3e\x2d\xcb\xb2\xfb\x79\x0b\x80\x03\xef\xd0\xd7\x0e\x4d\x74\xfe\xdd"
      "\xf9\x7b\xe7\xfd\xce\x2f\x3a\x57\x3a\xaf\x4d\x7d\x67\xf2\x9b\x93\x2f\x4d"
      "\x64\xfc\x6f\xe3\xdf\x6a\xcf\x8d\xbd\xd2\x7a\xa9\xf8\x43\xde\xcf\x4f\xb3"
      "\xf7\x37\x74\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x60\x4f\x37\xdf\x79\xf7\xea\xe2\xca\xca"
      "\xd2\xea\x8e\x4a\x59\x96\xb7\x87\x74\x3d\x96\x4a\xda\xc9\xb6\x39\x7f\xfd"
      "\x4b\xdf\x32\x49\xea\x87\x01\x8d\x3e\x60\xb5\xf4\xf9\x56\x52\xcf\x69\xa7"
      "\xa9\xdc\x5f\x60\xb7\x1f\x6c\x77\xde\x7b\xd0\x24\xfc\xb3\x39\x26\x4f\x24"
      "\xe1\x8f\xa4\x32\x35\xf4\xfc\xd9\x59\xf9\xa4\x2c\xcb\x83\x11\xf3\x28\x95"
      "\xb2\x71\x50\xe2\xd9\x8f\xca\xbe\xde\x96\x80\x27\xe0\xd4\xda\xb5\x1b\xa7"
      "\x6e\xbe\xf3\xee\xd7\x97\xaf\x2d\xbe\xb9\xf4\xe6\xd2\xf5\xb3\x67\xce\x9c"
      "\x9d\x3b\x7b\xe6\xd5\x85\x53\x97\x97\x57\x96\xe6\xba\xaf\xfb\x1d\x25\xf0"
      "\x38\xf4\x7d\x02\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x9e\x12\xa3\xfd"
      "\x71\x4e\xf1\x70\x7f\xdb\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0\x10\x2e\x9c\x48\xfb\x6e\x8a"
      "\xcc\xcf\x9d\x9c\xab\xda\x1b\x77\x16\x56\xaa\xa9\x57\xdf\x5a\xf2\x93\x24"
      "\xad\x24\xc5\x6c\x52\x7c\x90\x9c\x4f\x77\xca\x4c\xdf\x70\xc5\xb0\xed\xac"
      "\x27\x17\x3f\xfc\x78\xe3\xa3\x6e\xab\xdd\x4c\xf5\xf2\xad\xdd\xd6\x1b\xcd"
      "\x7a\x33\x65\x36\xc9\x58\x53\x0e\x30\x35\x68\x66\x79\x7b\xd8\x78\x45\x3d"
      "\xce\x8d\xe1\xe3\x8d\xa8\xd8\xdc\xc3\x2a\x61\xc7\x7b\x89\x83\xfd\xf6\xbf"
      "\x00\x00\x00\xff\xff\xed\x6e\x1a\xbb",
      1683);
  syz_mount_image(/*fs=*/0x400000000000, /*dir=*/0x400000000040,
                  /*flags=MS_REC|MS_SYNCHRONOUS|MS_NODIRATIME*/ 0x4810,
                  /*opts=*/0x400000000140, /*chdir=*/0x11, /*size=*/0x693,
                  /*img=*/0x400000000880);
  memcpy((void*)0x400000000040, "./file1\000", 8);
  syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x400000000040ul,
          /*flags=O_SYNC|O_NOATIME|O_CREAT|FASYNC|O_RDWR*/ 0x143042,
          /*mode=S_IXGRP|S_IWGRP|S_IXUSR*/ 0x58);
  memcpy((void*)0x400000000040, ".\000", 2);
  res = syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul,
                /*file=*/0x400000000040ul, /*flags=*/0, /*mode=*/0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x400000000000, ".\002\000", 3);
  memcpy((void*)0x400000000180, ".\002\000", 3);
  syscall(__NR_symlinkat, /*old=*/0x400000000000ul, /*newfd=*/r[0],
          /*new=*/0x400000000180ul);
  memcpy((void*)0x400000000800, ".\002\000", 3);
  memcpy((void*)0x400000000840, "./file1\000", 8);
  syscall(__NR_rename, /*old=*/0x400000000800ul, /*new=*/0x400000000840ul);
  memcpy((void*)0x400000000800, ".\002\000", 3);
  memcpy((void*)0x400000000840, "./file1\000", 8);
  syscall(__NR_rename, /*old=*/0x400000000800ul, /*new=*/0x400000000840ul);
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x3ffffffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x400000000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x400001000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  const char* reason;
  (void)reason;
  loop();
  return 0;
}
