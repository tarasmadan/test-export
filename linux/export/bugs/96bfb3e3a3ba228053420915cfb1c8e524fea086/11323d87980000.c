// https://syzkaller.appspot.com/bug?id=96bfb3e3a3ba228053420915cfb1c8e524fea086
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
  memcpy((void*)0x20000080, "hfsplus\000", 8);
  memcpy((void*)0x20000140, "./file1\000", 8);
  memcpy(
      (void*)0x200009c0,
      "\x78\x9c\xec\xdd\xcd\x6f\x1c\x67\x1d\x07\xf0\xef\x6c\x36\x8e\x37\x14\xd7"
      "\x4d\x93\x36\x45\x95\x12\x35\x12\x20\x22\x12\x3b\x26\x05\x73\x21\x20\x84"
      "\x72\xa8\x50\x55\x0e\x9c\xad\xc4\x69\xac\x6c\xd2\xe2\xb8\xc8\xad\x10\x75"
      "\xcb\xdb\xb5\x87\xfc\x01\xe5\xe0\x1b\x27\x24\xee\x91\xca\x85\x0b\xdc\x7a"
      "\xf5\xb1\x12\x82\x4b\x2e\x98\xd3\xa2\x99\x9d\xb5\xb7\xf6\xae\x5f\x4a\xec"
      "\xb5\xe1\xf3\xb1\x66\x9f\x67\xe6\x99\x79\x9e\xdf\xfc\xe6\x6d\x77\x25\x6b"
      "\x03\xfc\xdf\xba\x79\x39\xcd\xc7\x29\x72\xf3\xf2\x6b\xcb\xe5\xfc\xda\xea"
      "\x4c\x7b\x6d\x75\xe6\x7e\xaf\x9e\xe4\x54\x92\x46\xd2\xec\x16\x29\xfe\xd5"
      "\xe9\x74\x3e\x49\x6e\xa4\x3b\xe5\xa5\x72\x61\xdd\x5d\x31\x6c\x9c\x47\x0b"
      "\xb3\x6f\x7c\xfa\x64\xed\xb3\xee\x5c\xb3\x9e\xaa\xf5\x1b\x3b\x6d\xb7\x37"
      "\x2b\xf5\x94\x8b\x49\x4e\xd4\xe5\xd3\xea\xef\xd6\x6e\xfd\x8d\xef\xd6\x5d"
      "\xb1\xb1\x87\x65\xc2\x2e\xf5\x12\x07\xa3\x76\x32\x49\xa7\xf2\x8f\x47\xdd"
      "\x25\x3f\xfb\xcb\x33\x1b\x2d\x7d\x5a\x83\xb6\xde\xf5\xcc\x07\x8e\x81\xa2"
      "\xfb\xdc\xdc\x66\x32\x39\x5d\x5f\xe8\xe5\xfb\x80\xee\x53\xb1\xfb\xcc\x3e"
      "\xd6\x56\x46\x1d\x00\x00\x00\x00\x1c\x82\x67\xd7\xb3\x9e\xe5\x4c\x8c\x3a"
      "\x0e\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x38\x4e\xea\xdf\xff\x2f\xea\xe9"
      "\x5b\xbd\xc5\x17\x53\xf4\x7e\xff\x7f\xac\x6e\x4b\x5d\x3f\x5a\x2e\xec\x6f"
      "\xf5\xc7\x07\x15\x07\x00\x00\x00\x00\x00\x00\x00\x1c\xa2\x0b\xeb\x59\xcf"
      "\x72\x26\x7a\xf3\x9d\x22\x8d\x24\xaf\x54\x33\x67\xab\xd7\x2f\xe5\x9d\x3c"
      "\xcc\x7c\x16\x73\x25\xcb\x99\xcb\x52\x96\xb2\x98\xe9\x24\x93\x7d\x1d\x8d"
      "\x2d\xcf\x2d\x2d\x2d\x4e\xef\x61\xcb\x6b\x03\xb7\xbc\xb6\x4b\xa0\xa7\xea"
      "\xb2\xf5\x74\xf6\x1b\x00\x00\x00\x00\x00\x00\x00\x8e\x91\x27\x13\x5b\x97"
      "\x14\xcd\xad\x4b\x3e\xcc\xcd\x6c\x5b\x0d\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x46\xa9\x48\x4e\x74\x8b\x6a\x3a\xdb\xab\x4f\xa6\xd1\x4c\x32"
      "\x9e\x64\xac\x5c\x6f\x25\xf9\x5b\xaf\x7e\x9c\x3d\x1e\x75\x00\x00\x00\x00"
      "\x70\x08\x9e\x5d\xcf\x7a\x96\x33\xd1\x9b\xef\x14\xd5\x67\xfe\x17\xaa\xcf"
      "\xfd\xe3\x79\x27\x0f\xb2\x94\x85\x2c\xa5\x9d\xf9\xdc\xae\xbe\x0b\xe8\x7e"
      "\xea\x6f\xac\xad\xce\xb4\xd7\x56\x67\xee\x97\xd3\xf6\x7e\xbf\xff\xcf\x7d"
      "\x85\x51\xf5\x98\xee\x77\x0f\x83\x47\x3e\x5f\xad\xd1\xca\x9d\x2c\x54\x4b"
      "\xae\xe4\x56\xde\x4a\x3b\xb7\xd3\xa8\xb6\x2c\x9d\xaf\xe2\xb9\x3d\x91\x0c"
      "\x8c\xeb\x83\x32\xa6\xe2\x7b\xb5\x3d\x46\x76\xbb\x2e\xcb\x3d\xff\xa8\x2e"
      "\xb7\x79\x7f\x5f\x3b\x3b\xcc\x3e\xbf\x4c\x99\xac\x32\x72\x72\x23\x23\x53"
      "\x75\x6c\x65\x36\x9e\xeb\x1d\x99\xc1\x47\x68\x9f\x47\x67\xeb\x48\xd3\x69"
      "\x6c\x04\x7b\x76\xcb\x48\x5b\x76\xe2\x0b\xe5\xfc\x74\x5d\x96\xfb\xf3\xdb"
      "\x8d\x9c\x37\xf6\x15\xf3\xc1\xd8\x9a\x89\x6b\x7d\x67\xdf\x0b\x3b\xe7\x3c"
      "\xf9\xda\x9f\xfe\xf0\xd3\xbb\xed\x07\xf7\xee\xde\x79\x78\x79\xe0\x69\x74"
      "\x04\xad\xd4\xe5\x89\xba\xec\x54\xaf\xad\xed\x99\x98\xe9\xcb\xc4\x8b\xff"
      "\x8b\x99\x18\x6a\xaa\xca\xc4\x87\x1b\xf3\x37\xf3\xa3\xfc\x24\x97\x73\x31"
      "\xaf\x67\x31\x0b\xf9\x79\xe6\xb2\x94\xf9\x5c\xcc\x0f\xab\xda\x5c\x7d\x3e"
      "\x17\x7d\x97\xfc\x90\x4c\xdd\xf8\xdc\xdc\xeb\xbb\x45\x32\x56\x9f\xa1\xdd"
      "\x83\xd5\xc8\xb9\x7d\xc4\xf4\x4a\xb5\xed\x44\x16\xf2\xe3\xbc\x95\xdb\x99"
      "\xcf\xab\xd5\xdf\xb5\x4c\xe7\xdb\xb9\x9e\xeb\x99\xed\x3b\xc2\xe7\x76\x3e"
      "\xc2\xd5\x55\xdf\x18\x72\xd5\x77\xbe\x3c\x30\xf8\x4b\x5f\xaf\x2b\xad\x24"
      "\xbf\xab\xcb\xa3\xa1\xcc\xeb\x73\x7d\x79\xed\xbf\xe7\x4e\x56\x6d\xfd\x4b"
      "\x36\xb3\x74\x66\x0f\x59\xda\xe7\xbd\xb1\xf9\x95\xba\x52\x8e\xf1\xab\x23"
      "\x72\x57\xec\xda\x9a\x89\xe9\xbe\x4c\x3c\xbf\xb6\x3a\x9e\xe1\x99\xf8\x7d"
      "\x75\x5b\x79\xd8\x7e\x70\x6f\xf1\xee\xdc\xdb\x7b\x1b\xee\xcc\x47\x75\xa5"
      "\xbc\x8e\x7e\xb3\xf5\xc9\x3c\xd2\xbb\x4a\x79\xbe\x9c\x29\x0f\x56\x35\xf7"
      "\xf9\xb3\xa3\x6c\x7b\x7e\x60\xdb\x74\xd5\x76\x76\xa3\xad\xb1\xad\xed\xdc"
      "\x46\x5b\xf7\x4a\x5d\x19\x7a\xa5\x8e\xd5\xef\xe1\xfa\x7b\x6a\xf6\x45\xf7"
      "\xe2\xc0\x51\x66\xaa\xb6\xf3\x7d\x6d\x83\xde\x6f\x01\x70\xe4\x9d\xfe\xc6"
      "\xe9\xb1\xd6\xdf\x5b\x7f\x6d\x7d\xdc\xfa\x75\xeb\x6e\xeb\xb5\xf1\x1f\x9c"
      "\xfa\xce\xa9\x97\xc7\x72\xf2\xcf\x27\xbf\xdb\x9c\x3a\xf1\xd5\xc6\xcb\xc5"
      "\x1f\xf3\x71\x7e\xb9\xf9\xf9\x1f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf8\xe2\x1e\xbe\xfb"
      "\xde\xbd\xb9\x76\x7b\x7e\x71\x4b\xa5\xd3\xe9\xbc\x3f\xa4\xe9\x38\x57\x7a"
      "\x3f\x67\x76\x88\x83\xbe\xf4\x4c\x32\xaa\x5d\x1e\x4b\x72\x34\x32\xff\xef"
      "\x4e\xa7\x53\x2f\x29\x8e\x42\x3c\x3b\x57\x3a\xa5\x53\xe9\x1c\xf8\x58\xcd"
      "\x24\x83\x9a\x2e\xf4\x2f\xf9\x60\x24\xe7\xcf\x88\x6f\x4c\xc0\x81\xbb\xba"
      "\x74\xff\xed\xab\x0f\xdf\x7d\xef\x9b\x0b\xf7\xe7\xde\x9c\x7f\x73\xfe\xc1"
      "\xec\xf5\xeb\xb3\x53\xb3\xd7\x5f\x9d\xb9\x7a\x67\xa1\x3d\x3f\xd5\x7d\x1d"
      "\x75\x94\xc0\x41\xd8\x7c\xe8\x8f\x3a\x12\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x60\xaf\x9e\xce\xff\x0c\xb4\x92\x0c\x5f\x67\xf8\xe8\xe3\x87\xb9\xab"
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
      "\x00\x00\x00\xc0\x31\x75\xf3\x72\x9a\x8f\x53\x64\x7a\xea\xca\x54\x39\xbf"
      "\xb6\x3a\xd3\x2e\xa7\x5e\x7d\x73\xcd\x66\x92\x46\x92\xe2\x17\x49\xf1\x49"
      "\x72\x23\xdd\x29\x93\x7d\xdd\x15\xc3\xc6\x79\xb4\x30\xfb\xc6\xa7\x4f\xd6"
      "\x3e\xdb\xec\xab\xd9\x5b\xbf\xb1\xd3\x76\x7b\xb3\x52\x4f\xb9\x98\xe4\x44"
      "\x5d\x3e\xad\xfe\x6e\xfd\xd7\xfd\x95\x7b\x57\x8c\xa7\x4e\xd8\xa5\x5e\xe2"
      "\x60\xd4\xfe\x13\x00\x00\xff\xff\x53\x97\x02\xdb",
      1650);
  syz_mount_image(
      /*fs=*/0x20000080, /*dir=*/0x20000140,
      /*flags=MS_LAZYTIME|MS_STRICTATIME|MS_NODIRATIME|MS_NOATIME*/ 0x3000c00,
      /*opts=*/0x20000200, /*chdir=*/1, /*size=*/0x672, /*img=*/0x200009c0);
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
