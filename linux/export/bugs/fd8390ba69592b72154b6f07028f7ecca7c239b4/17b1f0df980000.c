// https://syzkaller.appspot.com/bug?id=fd8390ba69592b72154b6f07028f7ecca7c239b4
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
  memcpy((void*)0x20000140, "./bus\000", 6);
  memcpy(
      (void*)0x20000640,
      "\x78\x9c\xec\xdd\x4f\x68\x1c\xd7\x1d\x07\xf0\xef\xac\x56\xb2\xd7\x05\x45"
      "\x49\xec\x26\x94\x40\x45\x0c\x69\xa9\xa8\xad\x3f\x28\xad\x7a\xa9\x5b\x4a"
      "\xd1\x21\x94\x90\x1e\x7a\x16\xb6\x1c\x0b\xaf\x95\x20\x29\x45\x09\xa5\xa8"
      "\xff\xaf\x3d\xe4\x5a\x48\x0f\xba\xf5\x54\xe8\xdd\xd0\x9e\x9b\x5b\xae\x3a"
      "\x06\x0a\x85\x92\x93\x4e\xdd\x30\xb3\x23\x69\xf5\x37\x92\xec\x68\x57\xf1"
      "\xe7\x63\x66\xdf\x7b\xfb\x66\xde\xfb\xcd\x6f\x67\x66\xff\x08\x33\x01\x9e"
      "\x59\xf3\x13\x69\x3e\x4e\x91\xf9\x89\x37\xd6\xcb\xf6\xd6\xe6\x4c\x7b\x6b"
      "\x73\xe6\x4a\xdd\xdd\x4e\x52\xd6\x1b\x49\xb3\x5b\xa4\x58\x4e\x9a\x7f\x49"
      "\xee\xa4\xbb\x64\xb8\x67\xb8\xe2\xb8\x79\x3e\x5c\x9a\x7b\xeb\x93\xcf\xb6"
      "\x3e\xed\xb6\x9a\xf5\x52\xad\xdf\xd8\x3f\xc4\x79\x6c\xd4\x4b\xc6\x93\x0c"
      "\xd5\x65\xa9\x33\xd4\xbb\xda\xa9\xa7\xd9\x37\xde\xdd\xba\x6c\x9d\x3b\xbc"
      "\x62\x37\x33\x65\xc2\x6e\x96\xe5\xc6\xb9\x07\x83\xa7\xa7\x73\xc8\x99\x8e"
      "\xcc\x63\xcf\x77\xe0\xf2\x28\xba\xef\x9b\x87\x8c\x25\xd7\x92\x5c\xad\x3f"
      "\x07\xa4\xbe\x3a\x34\x2e\x36\xba\xa7\xcf\xfb\x2f\x00\x00\x00\xcf\x82\xe7"
      "\xb6\xb3\x9d\xf5\x8c\xf6\x3b\x0e\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xb8"
      "\x4c\xea\xfb\xff\x17\xf5\xd2\xd8\xa9\x8f\xa7\xd8\xb9\xff\xff\x48\xfd\x5c"
      "\xea\xfa\xa5\xf6\xb8\xdf\x01\x00\x00\x00\x00\x00\x00\x00\xc0\x53\xf0\xcd"
      "\xed\x6c\x67\x3d\xa3\xc9\xff\xab\xbf\xe5\x77\x8a\xea\x6f\xfe\xaf\x56\x9d"
      "\xd7\xab\xc7\xaf\xe5\xbd\xac\x66\x31\x2b\xb9\x95\xf5\x2c\x64\x2d\x6b\x59"
      "\xc9\x54\x92\xb1\x9e\x81\x46\xd6\x17\xd6\xd6\x56\xa6\x4e\xb1\xe5\xf4\x91"
      "\x5b\x4e\x5f\xdc\x3e\x03\x00\x00\x00\x00\x00\x00\xc0\x57\xd0\x6f\x33\x9f"
      "\xd1\x7e\x07\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xbd\x8a\x64"
      "\xa8\x5b\x54\xcb\xf5\x9d\xfa\x58\x1a\xcd\x24\x57\x53\x64\xa4\x5c\x6f\x23"
      "\xf9\x38\xe9\xd6\x2f\xb3\xc7\xfd\x0e\x00\x00\x00\x00\x2e\xc0\x73\xdb\xd9"
      "\xce\x7a\x46\x77\xda\x9d\xa2\xfa\xce\xff\xf5\xea\x7b\xff\xd5\xbc\x97\xe5"
      "\xac\x65\x29\x6b\x69\x67\x31\xf7\xaa\xdf\x02\xba\xdf\xfa\x1b\x5b\x9b\x33"
      "\xed\xad\xcd\x99\x47\xe5\x72\x78\xdc\x1f\xfd\xf7\x4c\x61\x54\x23\xa6\xfb"
      "\xdb\xc3\xd1\x33\x37\xea\x35\xef\x67\xa9\x7a\xe6\x56\xee\xe6\x9d\xb4\xab"
      "\x9e\x9d\xbe\x97\x77\xe2\x39\x3a\xae\xdf\x94\x31\x15\x3f\xac\x9d\x32\xb2"
      "\x7b\x75\x59\xee\xf9\x9f\xeb\x72\x30\x8c\x55\x39\x1b\xde\xcd\xc8\x64\x1d"
      "\x5b\x99\x8d\xe7\x4f\xce\xc4\x19\x5f\x9d\x83\x33\x4d\xa5\xb1\xfb\xcb\xcf"
      "\xf5\x23\x66\xba\xb2\xb7\xe9\xb9\x72\x7e\xad\x2e\xcb\xfd\xf9\xe3\xc9\x39"
      "\xef\x9c\x69\x47\x9e\xd8\xc1\x4c\x4c\xd7\x47\xdf\xc7\xe5\x39\x53\xee\x7d"
      "\xd9\x38\xee\xac\xf8\xd6\x3f\xfe\xf6\x8b\x07\xed\xe5\x87\x0f\xee\xaf\x4e"
      "\x0c\xce\x61\x74\x4e\x07\x33\x31\xd3\x73\x1e\xbe\x74\xf2\xd1\xf7\x15\xcb"
      "\xc4\x64\x95\x89\x1b\xbb\xed\xf9\xfc\x34\x3f\xcf\x44\xc6\xf3\x66\x56\xb2"
      "\x94\x5f\x66\x21\x6b\x59\xcc\x78\x7e\x52\xd5\x16\xea\xe3\xb9\x7c\x1c\x3b"
      "\x39\x53\x77\xf6\xb5\xde\xfc\xa2\x48\x46\xea\xd7\xa5\x7b\x15\x3d\x5b\x4c"
      "\xaf\x56\xdb\x8e\x66\x29\x3f\xcb\x3b\xb9\x97\xc5\xbc\x5e\xfd\x9b\xce\x54"
      "\xbe\x97\xd9\xcc\x66\xae\xe7\x15\xbe\x71\x8a\x2b\x6d\xe3\x6c\x67\xfd\xcd"
      "\x6f\xd7\x95\x56\x92\x3f\xd5\xe5\x60\x28\xf3\xfa\x7c\x4f\x5e\x7b\xaf\xb9"
      "\x63\x55\x5f\xef\x33\x7b\x59\x7a\xe1\xf8\x2c\x15\xe7\xbc\x36\x36\xbf\x51"
      "\x57\xca\x39\x7e\x57\x97\x83\xe1\x60\x26\xa6\x7a\x32\xf1\xe2\xc9\xc7\xcb"
      "\x5f\xab\xcb\xf8\x6a\x7b\xf9\xe1\xca\x83\x85\x77\x4f\x39\xdf\x6b\x75\x59"
      "\xa6\xf2\x0f\x03\xf5\xce\x5c\x1e\x2f\x2f\x94\x2f\x56\xd5\xda\x7f\x74\x94"
      "\x7d\x2f\x1e\xd9\x37\x55\xf5\x5d\xdf\xed\x6b\x1c\xea\xbb\xb1\xdb\xf7\x45"
      "\x67\xea\x48\xfd\x19\xee\xf0\x48\xd3\x55\xdf\x4b\x3d\x7d\xff\xeb\x74\x76"
      "\xaf\xe1\x65\xdf\xcb\x3d\x7d\xad\x23\x3e\x6f\x01\x30\xf0\xae\x7d\xe7\xda"
      "\x48\xeb\x3f\xad\x7f\xb7\x3e\x6a\xfd\xbe\xf5\xa0\xf5\xc6\xd5\x1f\x5f\xf9"
      "\xfe\x95\x57\x46\x32\xfc\xaf\xe1\x1f\x34\x27\x87\x5e\x6b\xbc\x52\xfc\x3d"
      "\x1f\xe5\xd7\x7b\xdf\xff\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\xf3\x5b\x7d\xff\x83\x87"
      "\x0b\xed\xf6\xe2\x4a\x9f\x2b\x45\x7d\x23\x9f\x41\x89\x47\xe5\xf8\xca\x48"
      "\xa7\xd3\xf9\xb2\xa6\x28\xba\x77\x20\xca\x00\xec\x69\x51\xdf\x59\xaa\xff"
      "\x09\xef\x56\xea\xcc\x5c\x54\x3c\xfd\xbb\x26\x01\x17\xe3\xf6\xda\xa3\x77"
      "\x6f\xaf\xbe\xff\xc1\x77\x97\x1e\x2d\xbc\xbd\xf8\xf6\xe2\xf2\xf0\xec\xec"
      "\xdc\xe4\xdc\xec\xeb\x33\xb7\xef\x2f\xb5\x17\x27\xbb\x8f\xfd\x8e\x12\xf8"
      "\x32\xec\xbd\xe9\xf7\x3b\x12\x00\x00\x00\x00\x00\x00\x00\x00\x00\xe0\xb4"
      "\x2e\xe2\xbf\x2e\xf4\x7b\x1f\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\xcb\x6d\x7e\x22\xcd\xc7\x29"
      "\x32\x35\x79\x6b\xb2\x6c\x6f\x6d\xce\xb4\xcb\x65\xa7\xbe\xb7\x66\x33\x49"
      "\x23\x49\xf1\xab\xa4\xf8\x67\x72\x27\xdd\x25\x63\x3d\xc3\x15\xc7\xcd\xf3"
      "\xe1\xd2\xdc\x5b\x9f\x7c\xb6\xf5\x69\xb7\xb5\x51\x8f\x57\xad\xdf\x38\x69"
      "\xbb\x23\x0d\x1f\x5c\x7f\xa3\x5e\x32\x9e\x64\xa8\x2e\x9f\xc0\xbe\xf1\xee"
      "\x3e\xf1\x78\xc5\xee\x1e\x96\x09\xbb\xb9\x93\x38\xe8\xb7\xcf\x03\x00\x00"
      "\xff\xff\x56\xf0\x05\xf6",
      1572);
  syz_mount_image(/*fs=*/0x20000600, /*dir=*/0x20000140, /*flags=*/0,
                  /*opts=*/0x200001c0, /*chdir=*/3, /*size=*/0x624,
                  /*img=*/0x20000640);
  memcpy((void*)0x20000040, ".\000", 2);
  res = syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000040ul,
                /*flags=*/0, /*mode=*/0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000040, ".\000", 2);
  res = syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000040ul,
                /*flags=*/0, /*mode=*/0);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x200000c0, "./file2\000", 8);
  memcpy((void*)0x20000180, "./file1\000", 8);
  syscall(__NR_renameat2, /*oldfd=*/r[1], /*old=*/0x200000c0ul, /*newfd=*/r[0],
          /*new=*/0x20000180ul, /*flags=*/0ul);
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