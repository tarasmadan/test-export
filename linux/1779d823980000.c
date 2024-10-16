// https://syzkaller.appspot.com/bug?id=b523b8b4018a2d7e2021bd7912074bd409b477b7
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
  memcpy((void*)0x20000180, "hfsplus\000", 8);
  memcpy((void*)0x20000000, "\351\037q\211Y\036\2223aK\000", 11);
  memcpy(
      (void*)0x20000fc0,
      "\x78\x9c\xec\xdd\xcd\x6f\x1c\x67\x1d\x07\xf0\xef\xac\xd7\x2f\x9b\x4a\x8e"
      "\xdb\xa6\x69\x40\x95\x6a\x1a\xa9\x20\x22\x92\x38\x56\x0a\xe1\x92\x80\x10"
      "\x0a\x52\x85\xaa\x20\xc1\xd9\x6a\x9c\xc6\xca\x26\x0d\x8e\x8b\xd2\x1e\x88"
      "\x0b\x48\x5c\x39\xf0\x07\x94\x43\xb8\xc0\x09\x84\x90\x90\x90\x22\x95\x33"
      "\xdc\x2a\x6e\x16\xa7\x4a\x48\x5c\x7a\x4a\x7b\x60\xd0\xce\xce\x6e\xd6\xee"
      "\xae\xbd\x49\x9a\xac\x43\x3f\x1f\x6b\x76\x7e\xcf\x3c\xb3\xcf\xfc\xe6\x37"
      "\x2f\xfb\x22\x59\x1b\xe0\x73\xeb\xfc\xb1\x34\xef\xa4\x95\xf3\xc7\x5e\xbd"
      "\xd9\x69\x6f\xdd\x5e\x6e\x6f\xdd\x5e\xbe\xda\x8d\x1b\xed\x24\xb3\x49\x1a"
      "\x49\xb3\x3b\x4b\x71\x2d\x29\xde\x4f\xce\xa5\x3b\xe5\x0b\x9d\x85\xf5\x70"
      "\xc5\xa8\xed\xfc\x7a\xed\xcc\x85\x0f\x3e\xda\xfa\xb0\xdb\x6a\xe6\xde\x78"
      "\x9d\x87\xd6\xe8\x04\x9b\xe3\xec\xc5\x66\x3d\x65\x31\xc9\x54\x3d\x7f\x08"
      "\xdb\xc6\x7b\xfd\xc1\xc6\x9b\xbd\x17\x16\xfd\xca\x74\x0a\x76\xb4\x57\x38"
      "\x98\xb4\xe9\x24\xe5\x36\x3f\x3e\x7c\xaf\x67\x98\x72\x6a\xa0\x31\xf2\x7a"
      "\x07\x9e\x1c\x45\xf7\x75\x73\x40\xf7\xfa\x5f\x48\x0e\x24\x99\xeb\xbd\xa0"
      "\x6d\x76\x3b\x1b\x8f\x3f\xc3\x3d\xdd\xd7\xbd\x68\xf3\xd1\xe5\x01\x00\x00"
      "\x00\xfb\xc6\xc1\xbb\xb7\x92\x9b\x99\x9f\x74\x1e\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\xf0\x24\xa9\x7f\xff\xbf\xa8\xa7\x46\x2f\x5e\x4c\xd1\xfb\xfd"
      "\xff\x99\x81\xdf\xd8\x9f\x99\x70\xba\xa3\xed\x9e\xd9\x5c\x2f\xb8\xd3\x78"
      "\x1c\xc9\x00\x00\x00\x00\x00\x00\x00\xc0\xa3\xf5\xe2\xdd\xfc\xee\x42\x59"
      "\xce\xf7\xda\x65\x91\xc6\x0f\xa7\xea\xc6\xa1\xea\xf1\xa9\xbc\x95\x1b\x59"
      "\xcd\x7a\x8e\xe7\x66\x56\xb2\x91\x8d\xac\x67\x29\xc9\xc2\xc0\x40\x33\x37"
      "\x57\x36\x36\xd6\x97\xf2\x52\xf7\x99\x9f\x94\x65\x39\xe2\x99\xa7\x86\x3e"
      "\xf3\xd4\x98\x09\xb7\x1e\x7e\x9f\x01\x00\x00\x00\x00\x00\x00\xe0\xff\xc8"
      "\xd9\x7a\xfe\xb3\x9c\xcf\xfc\x84\x73\x01\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x80\x6d\x8a\x64\xaa\x3b\xab\xa6\x43\xbd\x78\x21\x8d\x66\x92\xb9"
      "\x24\x33\x9d\xf5\x36\x93\x7f\xf4\xe2\x27\xd9\x9d\x49\x27\x00\x00\x00\x00"
      "\x8f\xc1\xc1\xbb\xb9\x9b\x9b\x99\xef\xb5\xcb\xa2\xfa\xcc\x7f\xb8\xfa\xdc"
      "\x3f\x97\xb7\x72\x2d\x1b\x59\xcb\x46\xda\x59\xcd\xc5\xea\xbb\x80\xee\xa7"
      "\xfe\xc6\xd6\xed\xe5\xf6\xd6\xed\xe5\xab\x9d\xe9\xd3\xe3\x7e\xeb\x3f\xf7"
      "\xe2\x3f\xce\xef\x99\x46\x35\x62\xba\xdf\x3d\x0c\xdf\xf2\x91\x6a\x8d\x56"
      "\x2e\x65\xad\x5a\x72\x3c\xaf\xe7\xcd\xb4\x73\x31\x8d\xea\x99\x1d\x47\x7a"
      "\xf9\x0c\xcf\xeb\xdd\x4e\x4e\xc5\xd9\xae\xb2\xcc\xec\x38\x05\xba\x58\xcf"
      "\x3b\x7b\xfe\xab\x7a\xbe\x3f\x2c\x54\x15\x99\xee\x57\xe4\x64\x27\xb7\xa2"
      "\x5b\xc7\xa7\x77\xaf\xc4\xe0\xd1\x79\x80\x2d\x2d\xa5\xd1\xff\xe6\xe7\xd0"
      "\x7d\xd4\xfc\xec\xae\x5b\x29\xfe\x5b\x96\xdd\xe8\x40\x6f\x49\xf2\xd4\xf7"
      "\xf6\xae\xf9\xf4\x7d\xed\xcc\x43\xd9\x59\x89\x53\x03\x67\xdf\xe1\xdd\x2b"
      "\x91\x7c\xf9\x4f\xbf\xff\xd1\xe5\xf6\xb5\x2b\x97\x8b\xcd\x63\xfb\xe7\x34"
      "\x1a\xe6\xc5\xe1\x8b\x67\xff\xd5\x3b\x42\xbd\x4a\x74\x6d\x66\x35\xcb\x03"
      "\x95\x78\x7e\xec\x4a\x5c\xba\xb1\xcf\x2b\x31\x52\x73\x5b\xab\x91\xe7\xfa"
      "\xf1\xf9\x7c\x37\x3f\xc8\xb1\x2c\xe6\xb5\xac\x67\x2d\x3f\xc9\x4a\x36\xb2"
      "\x9a\xc5\x7c\xa7\x8a\x56\xea\xf3\xb9\xf3\xb8\xb0\x7b\xa5\xce\x6d\x6b\xbd"
      "\xb6\x57\x4e\x33\xf5\x71\x99\xda\x91\xd3\x97\x0e\x76\xe7\xbb\xe5\xf4\x52"
      "\xf5\xdc\xf9\xac\xe5\xfb\x79\x33\x17\xb3\x9a\x57\xaa\xbf\x53\x59\xca\xd7"
      "\x73\x3a\xa7\x73\x66\xe0\x08\x3f\x37\xc6\x55\xdf\x18\x72\xd5\xff\x79\x74"
      "\xf2\x47\xbf\x52\x07\xad\x24\xbf\xac\xe7\xfb\x43\xa7\xae\x4f\x0f\xd4\x75"
      "\xf0\x9e\xbb\x50\xf5\x0d\x2e\x69\xa4\xac\x5f\x59\x9e\xf9\xcc\xee\x8d\x7d"
      "\xcd\x2f\xd6\x41\xe7\x48\xfc\x7c\xe0\x1a\x9c\xbc\x7e\x25\xe6\xd2\x7f\x95"
      "\xe8\x65\xf7\x6c\xaf\x02\xd3\x43\x2b\xf1\x9b\xea\xb6\x72\xa3\x7d\xed\xca"
      "\xfa\xe5\x95\xeb\x3b\xc6\x2d\x36\x87\x6f\xef\xe5\x6c\xdf\xfd\xfd\x73\x23"
      "\xe9\x9c\x2f\xcf\xf4\xef\x11\xdb\xcf\x8e\x4e\xdf\xb3\x43\xfb\x96\xaa\xbe"
      "\x43\xfd\xbe\xc6\xce\xbe\xdf\xb6\xfa\x7d\x7b\x5d\xa9\x33\xf5\x7b\xb8\x4f"
      "\x8f\x74\xaa\xea\x7b\x7e\x68\xdf\x72\xd5\x77\x64\xa0\xaf\xf3\x7e\x6b\x2e"
      "\xc9\x6a\x3e\x29\xcb\xb2\xfb\x7e\x0b\x80\x7d\xef\xc0\x57\x0f\xcc\xb4\xfe"
      "\xdd\xfa\x7b\xeb\xbd\xd6\x2f\x5a\x97\x5b\xaf\xce\x7d\x7b\xf6\x1b\xb3\x2f"
      "\xcc\x64\xfa\x6f\xd3\xdf\x6c\x9e\x9c\x7a\xb9\xf1\x42\xf1\x87\xbc\x97\x9f"
      "\x66\xef\x4f\xe8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\xc0\x9e\x6e\xbc\xfd\xce\x95\x95\x76"
      "\x7b\x75\x7d\x47\x50\x96\xe5\xad\x11\x5d\x8f\x24\x48\x33\xd9\xb6\xe4\xaf"
      "\x7f\x19\x58\xa7\xfa\xad\xb1\x24\xe3\x0f\xd8\x59\xfb\x5c\x23\xa9\x96\x34"
      "\x53\x07\xf7\x97\xd8\xad\x07\xdb\x9d\x77\x1f\xb4\x08\xff\xac\x8f\xc9\x63"
      "\x29\xf8\x67\x12\xcc\x8d\x3c\x7f\x76\x06\x1f\x97\x65\xb9\x3f\x72\x1e\x27"
      "\x28\x6b\xfb\x25\x9f\x49\x04\x13\xbd\x2d\x01\x8f\xc1\x89\x8d\xab\xd7\x4f"
      "\xdc\x78\xfb\x9d\xaf\xad\x5d\x5d\x79\x63\xf5\x8d\xd5\x6b\x67\x4e\x9f\x3e"
      "\x73\xf2\xcc\xe9\x57\x96\x4f\x5c\x5a\x6b\xcf\x4d\x3a\x3d\xe0\x11\xaa\x5e"
      "\xeb\xab\xf7\x39\x93\xce\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x18\xd7"
      "\x78\xff\x9c\x53\xf4\x97\x34\x93\xdc\xf7\xff\xf6\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x3c\x84"
      "\xf3\xc7\xd2\xbc\x93\x22\x4b\x27\x8f\x9f\xec\xb4\xb7\x6e\x2f\xb7\x3b\x53"
      "\x2f\xbe\xb7\xe6\xc7\x49\x1a\x49\x8a\xc5\xa4\x78\x3f\x39\x97\xee\x94\x85"
      "\x81\xe1\x8a\x51\xdb\xd9\x4c\x2e\x7c\xf0\xd1\xd6\x87\xdd\x56\xb3\x9e\xaa"
      "\xf5\x1b\x0f\xbf\x17\x9b\xf5\x94\xc5\x24\x53\xf5\x7c\x88\xb9\x61\x0b\xcb"
      "\x5b\xa3\xc6\x2b\xaa\x71\xae\x8f\x1e\x6f\x4c\x45\xbf\x32\x9d\x82\x1d\xed"
      "\x15\x0e\x26\xed\x7f\x01\x00\x00\xff\xff\x2a\xb4\x15\xc5",
      1706);
  syz_mount_image(
      /*fs=*/0x20000180, /*dir=*/0x20000000,
      /*flags=MS_REC|MS_SYNCHRONOUS|MS_NOSUID|MS_NODIRATIME*/ 0x4812,
      /*opts=*/0x20002140, /*chdir=*/0x11, /*size=*/0x6aa, /*img=*/0x20000fc0);
  memcpy((void*)0x20000140, "./file0/../file0\000", 17);
  syscall(__NR_listxattr, /*path=*/0x20000140ul, /*list=*/0ul, /*size=*/0xf9ul);
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
