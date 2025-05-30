// https://syzkaller.appspot.com/bug?id=29047010c8fc6106059964efd51ff4f88f3b46ba
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
  memcpy((void*)0x20000f00, "udf\000", 4);
  memcpy((void*)0x200000c0, "./file1\000", 8);
  memset((void*)0x20001080, 0, 4);
  sprintf((char*)0x20001084, "%023llo", (long long)-1);
  sprintf((char*)0x2000109b, "%023llo", (long long)-1);
  sprintf((char*)0x200010b2, "%020llu", (long long)0);
  memcpy(
      (void*)0x200010c6,
      "\x2c\x75\x6d\x61\x73\x6b\x3d\x30\x30\x00\x00\x00\x00\x30\x30\x30\x30\x30"
      "\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x2c\x67\x69\x64\x3d\x69"
      "\x67\x67\x10\xb0\x1c\x1f\x9d\x6e\x6f\x72\x65\x2c\x6e\x6f\x61\x64\x69\x6e"
      "\x69\x63\x62\x2c\x75\x6e\x64\x65\x6c\x65\x74\x65\x2c\x6e\x6f\x76\x72\x73"
      "\x2c\x6c\x61\x73\x74\x62\x6c\x6f\x63\x6b\x3d\x30\x30\x30\x30\x30\x30\x30"
      "\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x31\x2c\x75\x69\x64\x3d"
      "\x69\x67\x6e\x6f\x72\x65\x2c\x6e\x6f\x73\x74\x72\x69\x63\x74\x2c\x61\x6e"
      "\x63\x68\x6f\x72\x3d\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30"
      "\x30\x30\x30\x30\x30\x30\x35\x2c\x76\x6f\x6c\x75\x6d\x65\x3d\x30\x30\x30"
      "\x30\x30\x30\x38\x6e\xe5\xee\xf6\x21\x88\xe3\x30\x30\x30\x30\x30\x30\x30"
      "\x30\x30\x30\x89\x30\x30\x2c\x75\x6e\x12\x02\x68\x2e\x87\xdc\x70\xc6\xe7"
      "\xea\x5d\xda\xe3\x18\x73\x88\xc6\x71\xa9\x3c\x7e",
      210);
  memcpy(
      (void*)0x20000200,
      "\x78\x9c\xec\xdd\x4f\x6c\x1c\xd7\x7d\x07\xf0\xdf\x1b\x2e\xc5\xa5\xdd\x56"
      "\x4c\xec\x28\x4e\x1a\x17\x9b\xb6\x48\x65\xc5\x72\xf5\x2f\xa6\x62\x15\xee"
      "\xaa\xa6\xd9\x06\x90\x65\x21\x14\x73\x0b\xc0\x95\x48\xa9\x0b\x53\x24\x41"
      "\x52\x8d\x6c\xa4\x05\xd3\x4b\x0f\x3d\x04\x28\x8a\x1e\x72\x22\xd0\x1a\x05"
      "\x52\x34\x30\x9a\x22\xe8\x91\x69\x5d\x20\xb9\xf8\x50\xe4\xd4\x13\xd1\xc2"
      "\x46\x50\xf4\xc0\x16\x01\x72\x0a\x18\xcc\xec\x5b\x71\x45\x91\x16\x2d\x8a"
      "\x14\x25\x7f\x3e\x36\xf5\xdd\x9d\x7d\x6f\xf6\xbd\x79\xab\x19\x8a\xe0\x9b"
      "\x17\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x40\xc4\x1f\xbc"
      "\x7a\xfe\xc4\xc9\xb4\x83\x82\xb5\x7d\x68\x0c\x00\xb0\x2f\x2e\x8e\x7d\xf5"
      "\xc4\xa9\x9d\x5c\xff\x01\x80\xc7\xc6\xe5\x9d\xfe\xfb\x1f\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x78\x58\x52\x14\xf1\x54\xa4\x98\xbb\xb8\x96"
      "\x26\xaa\xe7\x1d\xf5\x0b\xed\xfe\x9b\xb7\xc6\x47\x46\xb7\xae\x36\x98\xaa"
      "\x9a\x7d\x55\xf9\xf2\xab\x7e\xf2\xd4\xe9\x33\x5f\x7a\x71\xf8\x6c\x37\x2f"
      "\xb4\x67\x3e\xa4\xfe\x83\xf6\xd9\x78\x7d\xec\xf2\xf9\xc6\x2b\xb3\x37\xe6"
      "\xe6\xa7\x16\x16\xa6\x26\x1b\xe3\x33\xed\xab\xb3\x93\x53\x3b\xde\xc3\x6e"
      "\xeb\x6f\x76\xac\x3a\x00\x8d\x1b\x6f\xdc\x9c\xbc\x76\x6d\xa1\x71\xea\x85"
      "\xd3\x77\xbc\x7c\x6b\xe8\x83\x81\x27\x8f\x0c\x9d\x1b\x7e\xee\xf8\xb3\xdd"
      "\xb2\xe3\x23\xa3\xa3\x63\x1b\x45\xea\xbd\xe5\x77\x7b\xf7\x85\xed\x66\x78"
      "\x1c\x8a\x22\x8e\x47\x8a\xe7\xbf\xf7\xd3\xd4\x8a\x88\x22\x76\x7f\x2c\xea"
      "\xfb\x3b\xf6\x9b\x0d\x56\x9d\x38\x56\x75\x62\x7c\x64\xb4\xea\xc8\x74\xbb"
      "\x35\xb3\x58\xbe\x78\xa9\x7b\x20\x8a\x88\x46\x4f\xa5\x66\xf7\x18\x6d\x3d"
      "\x16\x51\xeb\xdf\xd7\x3e\x6c\xaf\x19\xb1\x54\x36\xbf\x6c\xf0\xb1\xb2\x7b"
      "\x63\x73\xad\xf9\xd6\x95\xe9\xa9\xc6\xa5\xd6\xfc\x62\x7b\xb1\x3d\x3b\x73"
      "\x29\x75\x5a\x5b\xf6\xa7\x11\x45\x9c\x4d\x11\xcb\x11\xb1\x3a\x70\xf7\xee"
      "\xfa\xa3\x88\x5a\xa4\xf8\xce\xe1\xb5\x74\x25\x22\xfa\xba\xc7\xe1\x8b\xd5"
      "\xc4\xe0\xed\xdb\x51\xec\x61\x1f\x77\xa0\x6c\x67\xa3\x3f\x62\xb9\x78\x04"
      "\xc6\xec\x00\x1b\x88\x22\x5e\x8b\x14\x3f\x7b\xf7\x68\x5c\xcd\xe7\x99\xea"
      "\x5c\xf3\x85\x88\xd7\xca\xfc\x41\xc4\xdb\x65\xbe\x1c\x91\xca\x0f\xc6\x99"
      "\x88\xf7\xb7\xf8\x1c\xf1\x68\xaa\x45\x11\x7f\x59\x8e\xff\xb9\xb5\x34\x59"
      "\x9d\x0f\xba\xe7\x95\x0b\x5f\x6b\x7c\x65\xe6\xda\x6c\x4f\xd9\xee\x79\xe5"
      "\x23\x5e\x1f\xee\x3a\x53\x3c\xa4\xeb\xc3\xe0\xa6\xdc\x1f\x07\xfc\xdc\x54"
      "\x8f\x22\x5a\xd5\x19\x7f\x2d\xdd\xff\x37\x3b\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x3c\x68\x83\x51\xc4\x67\x22\xc5\xab\xff\xf1\x27"
      "\xd5\xbc\xe2\xa8\xe6\xa5\x1f\x3e\x37\xfc\x87\x43\xbf\xda\x3b\x67\xfc\x99"
      "\x7b\xec\xa7\x2c\xfb\x42\x44\x2c\x15\x3b\x9b\x93\x7b\x28\x4f\x0c\xbc\x94"
      "\x2e\xa5\xf4\x90\xe7\x12\x7f\x9c\xd5\xa3\x88\x3f\xcd\xf3\xff\xbe\xf5\xb0"
      "\x1b\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0\xb1"
      "\x56\xc4\x4f\x22\xc5\x4b\xef\x1d\x4d\xcb\xd1\xbb\xa6\x78\x7b\xe6\x7a\x63"
      "\x30\x22\x3a\xab\xc2\x76\xd7\xfe\xed\xae\x99\xbe\xbe\xbe\xbe\xde\x48\x9d"
      "\x6c\xe6\x9c\xc8\xb9\x94\x73\x39\xe7\x4a\xce\xd5\x9c\x51\xe4\xfa\x39\x9b"
      "\x39\x27\x72\x2e\xe5\x5c\xce\xb9\x92\x73\x35\x67\xf4\xe5\xfa\x39\x9b\x39"
      "\x27\x72\x2e\xe5\x5c\xce\xb9\x92\x73\x35\x67\xd4\x72\xfd\x9c\xcd\x9c\x13"
      "\x39\x97\x72\x2e\xe7\x5c\xc9\xb9\x9a\x33\x0e\xc8\xda\xbd\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x8f\x93\x22\x8a\xf8\x45\xa4\xf8\xf6\x37\xd6"
      "\x52\xa4\x88\x68\x46\x4c\x44\x27\x57\x06\x1e\x76\xeb\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\xd2\x40\x2a"
      "\xe2\xfb\x91\xa2\xf1\x47\xcd\xdb\xdb\x6a\x11\x91\xaa\xff\x3b\x8e\x96\x7f"
      "\x9c\x89\xe6\xa1\x32\x3f\x19\xcd\xe1\x32\x5f\x8e\xe6\xf9\x9c\xad\x2a\x6b"
      "\xcd\x6f\x3d\x84\xf6\xb3\x3b\xfd\xa9\x88\x1f\x47\x8a\x81\xfa\x3b\xb7\x07"
      "\x3c\x8f\x7f\x7f\xe7\xd9\xed\x8f\x41\xbc\xfd\xcd\x8d\x67\x9f\xad\x75\xb2"
      "\xaf\xfb\xe2\xd0\x07\x03\x4f\x1e\x39\x7c\x6e\x78\xf4\x37\x9e\xd9\xee\x71"
      "\xda\xaa\x01\xc7\x2e\xb4\x67\x6e\xde\x6a\x8c\x8f\x8c\x8e\x8e\xf5\x6c\xae"
      "\xe5\x77\xff\x64\xcf\xb6\xa1\xfc\xbe\xc5\x83\xe9\x3a\x11\xb1\xf0\xe6\x5b"
      "\x6f\xb4\xa6\xa7\xa7\xe6\xef\xff\x41\xf9\x11\xd8\x45\xf5\x47\xe8\x41\xaa"
      "\x7d\x5c\x7a\xea\x41\xf5\x20\x6a\x07\xa2\x19\x0f\xa7\xef\xf7\x70\xb8\x6f"
      "\x7f\xce\x50\xec\xa5\xf2\xfa\xff\x7e\xa4\xf8\xdd\xf7\xfe\xb3\x7b\xc1\xef"
      "\x5c\xff\xeb\xf1\x2b\x9d\x67\x1b\xc3\xfc\xf3\x3f\xdb\xb8\xfe\xbf\xb4\x79"
      "\x47\x3b\xbc\xfe\xd7\x36\xd7\xcb\xd7\xff\xf2\x9a\xbe\xd5\xf5\xff\xa9\x9e"
      "\x6d\x2f\xe5\xef\x46\xfa\x6b\x11\xf5\xc5\x1b\x73\xfd\x47\x22\xea\x0b\x6f"
      "\xbe\x75\xbc\x7d\xa3\x75\x7d\xea\xfa\xd4\xcc\x99\x13\x27\xbe\x3c\x3c\xfc"
      "\xe5\xd3\x27\xfa\x0f\x45\xd4\xaf\xb5\xa7\xa7\x7a\x1e\x3d\x90\xc3\x05\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xb0\x7f\x52\x11\xbf\x1f\x29"
      "\x5a\x3f\x5e\x4b\x8d\x88\xb8\x55\xcd\xd7\x1a\x3a\x37\xfc\xdc\xf1\x67\xfb"
      "\xa2\xaf\x9a\x6f\x75\xc7\xbc\xed\xd7\xc7\x2e\x9f\x6f\xbc\x32\x7b\x63\x6e"
      "\x7e\x6a\x61\x61\x6a\xb2\x31\x3e\xd3\xbe\x3a\x3b\x39\xb5\xd3\xb7\xab\x57"
      "\xd3\xbd\xc6\x47\x46\xf7\xa4\x33\xf7\x34\xb8\xc7\xed\x1f\xac\xbf\x32\x3b"
      "\xf7\xe6\x7c\xfb\xfa\x1f\x2f\x6e\xf9\xfa\x13\xf5\xf3\x57\x16\x16\xe7\x5b"
      "\x57\xb7\x7e\x39\x06\xa3\x88\x68\xf6\x6e\x39\x56\x35\x78\x7c\x64\xb4\x6a"
      "\xf4\x74\xbb\x35\x53\x55\xbd\xb4\xe5\x64\xfa\x8f\xae\x3f\x15\xf1\x5f\x91"
      "\xe2\xea\x99\x46\xfa\x7c\xde\x96\xe7\xff\x6f\x9e\xe1\x7f\xc7\xfc\xff\xa5"
      "\xcd\x3b\xda\xa3\xf9\xff\x9f\xe8\xd9\x56\xbe\x67\x4a\x45\xfc\x3c\x52\xfc"
      "\xce\x5f\x3d\x13\x9f\xaf\xda\xf9\x44\xdc\x75\xcc\x72\xb9\xbf\x8b\x14\xc7"
      "\xce\x7e\x2e\x97\x8b\x43\x65\xb9\x6e\x1b\x3a\xf7\x15\xe8\xcc\x0c\x2c\xcb"
      "\xfe\x5f\xa4\xf8\xa7\x5f\xdc\x59\xb6\x3b\x1f\xf2\xa9\x8d\xb2\x27\x77\x7c"
      "\x60\x1f\x11\xe5\xf8\x1f\x8e\x14\xdf\xff\x8b\xef\xc6\x6f\xe6\x6d\x77\xde"
      "\xff\x61\xeb\xf1\x7f\x62\xf3\x8e\xee\x77\xfc\xd7\x3f\x7c\xfc\x9f\xee\xd9"
      "\xf6\xc4\x1d\xf7\x2b\xd8\x75\xd7\xc9\xe3\x7f\x3c\x52\xbc\xfc\xd4\x3b\xf1"
      "\x5b\x79\xdb\x87\xdd\xff\xa3\x7b\xef\x8d\xa3\xb9\xf0\xed\xfb\x73\xec\xd1"
      "\xdf\xff\x4f\xf5\x6c\x1b\xca\xef\xfb\xdb\x0f\xa6\xeb\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x8f\xb4\xfe\x54\xc4\xdf\x47\x8a\x1f\x8e\xd6\xd2\x8b\x79"
      "\xdb\x4e\x7e\xff\x6f\x72\xf3\x8e\xf6\xe8\xf7\xbf\x3e\xdd\xb3\x6d\xf2\xc1"
      "\xac\x57\x74\xcf\x07\xbb\x3e\xa8\x00\x00\x00\x00\x70\x40\xf4\xa7\x22\x7e"
      "\x12\x29\xae\x2f\xbe\x73\x7b\x0e\xf5\x9d\xf3\xbf\x7b\xe6\x7f\xfe\xde\xc6"
      "\xfc\xcf\x91\xb4\xe9\xd5\xea\xe7\x7c\xbf\x56\xdd\x37\xe0\x41\xfe\xfc\xaf"
      "\xd7\x50\x7e\xdf\x89\xdd\x77\x1b\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x0e\x94\x94\x8a\x78\x31\xaf\xa7\x3e\x51\xcd\xe7\x9f"
      "\xdc\x76\x3d\xf5\x95\x48\xf1\xea\xff\x3c\x9f\xcb\xa5\x23\x65\xb9\xee\x3a"
      "\xf0\x43\xd5\x9f\xf5\x8b\xb3\x33\xc7\xcf\x4f\x4f\xcf\x5e\x6d\x2d\xb6\xae"
      "\x4c\x4f\x35\xc6\xe6\x5a\x57\xa7\xca\xba\x4f\x47\x8a\xb5\xbf\xfd\x5c\xae"
      "\x5b\x54\xeb\xab\x77\xd7\x9b\xef\xac\xf1\xbe\xb1\x16\xfb\x7c\xa4\x18\xfd"
      "\x87\x6e\xd9\xce\x5a\xec\xdd\xb5\xc9\x9f\xde\x28\x7b\xb2\x2c\xfb\x89\x48"
      "\xf1\xdf\xff\x78\x67\xd9\xee\x3a\xd6\x9f\xda\x28\x7b\xaa\x2c\xfb\x37\x91"
      "\xe2\xeb\xff\xb2\x75\xd9\x23\x1b\x65\x4f\x97\x65\xbf\x1b\x29\x7e\xf4\xf5"
      "\x46\xb7\xec\x13\x65\xd9\xee\xfd\x51\x3f\xbd\x51\xf6\x85\xab\xb3\xc5\x1e"
      "\x8c\x0a\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1f"
      "\x37\xfd\xa9\x88\x3f\x8f\x14\xff\x7b\x63\xf9\xf6\x5c\xfe\xbc\xfe\x7f\x7f"
      "\xcf\xd3\xca\xdb\xdf\xec\x59\xef\x7f\x93\x5b\xd5\x3a\xff\x43\xd5\xfa\xff"
      "\xdb\x3d\xbe\x9f\xf5\xff\xab\xfb\x0a\x2c\x6d\xf7\xae\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0\x78\x4a\x51\xc4\x5b"
      "\x91\x62\xee\xe2\x5a\x5a\x19\x28\x9f\x77\xd4\x2f\xb4\x67\x6e\xde\x1a\x1f"
      "\x19\xdd\xba\xda\x60\xaa\x6a\xf6\x55\xe5\xcb\xaf\xfa\xc9\x53\xa7\xcf\x7c"
      "\xe9\xc5\xe1\xb3\xdd\xfc\xf0\xfa\x0f\xda\x67\xe2\xf5\xb1\xcb\xe7\x1b\xaf"
      "\xcc\xde\x98\x9b\x9f\x5a\x58\x98\x9a\x6c\x8c\xcf\xb4\xaf\xce\x4e\x4e\xed"
      "\x78\x0f\xbb\xad\xbf\xd9\xb1\xea\x00\x34\x6e\xbc\x71\x73\xf2\xda\xb5\x85"
      "\xc6\xa9\x17\x4e\xdf\xf1\xf2\xad\xa1\x0f\x06\x9e\x3c\x32\x74\x6e\xf8\xb9"
      "\xe3\xcf\x76\xcb\x8e\x8f\x8c\x8e\x8e\xf5\x94\xa9\xf5\xdf\xf7\xbb\xdf\x25"
      "\x6d\xb3\xfd\x50\x14\xf1\xd7\x91\xe2\xf9\xef\xfd\x34\xfd\x70\x20\xa2\x88"
      "\xdd\x1f\x8b\x7b\x7c\x76\xf6\xda\x60\xd5\x89\x63\x55\x27\xc6\x47\x46\xab"
      "\x8e\x4c\xb7\x5b\x33\x8b\xe5\x8b\x97\xba\x07\xa2\x88\x68\xf4\x54\x6a\x76"
      "\x8f\xd1\x3e\x8c\xc5\xae\x34\x23\x96\xca\xe6\x97\x0d\x3e\x56\x76\x6f\x6c"
      "\xae\x35\xdf\xba\x32\x3d\xd5\xb8\xd4\x9a\x5f\x6c\x2f\xb6\x67\x67\x2e\xa5"
      "\x4e\x6b\xcb\xfe\x34\xa2\x88\xb3\x29\x62\x39\x22\x56\x07\xee\xde\x5d\x7f"
      "\x14\xf1\x46\xa4\xf8\xce\xe1\xb5\xf4\xaf\x03\x11\x7d\xdd\xe3\xf0\xc5\x8b"
      "\x63\x5f\x3d\x71\x6a\xfb\x76\x14\x7b\xd8\xc7\x1d\x28\xdb\xd9\xe8\x8f\x58"
      "\x2e\x3e\xea\x98\xad\xaf\xaf\xaf\xef\x5f\x3b\x0f\xba\x81\x28\xe2\x9f\x23"
      "\xc5\xcf\xde\x3d\x1a\xff\x36\x10\x51\x8b\xce\x57\x7c\x21\xe2\xb5\x32\x7f"
      "\x10\xf1\x76\x74\xc6\x3b\x95\x1f\x8c\x33\x11\xef\x6f\xf1\x39\xe2\xd1\x54"
      "\x8b\x22\xfe\xbf\x1c\xff\x73\x6b\xe9\xdd\x81\xf2\x7c\xd0\x3d\xaf\x5c\xf8"
      "\x5a\xe3\x2b\x33\xd7\x66\x7b\xca\x76\xcf\x2b\x8f\xfc\xf5\x61\x3f\x1d\xf0"
      "\xeb\x49\x3d\x8a\xf8\x51\x75\xc6\x5f\x4b\xff\xee\xef\x35\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\xc0\x01\x52\xc4\xaf\x47\x8a\x97\xde\x3b"
      "\x9a\xaa\xf9\xc1\xb7\xe7\x14\xb7\x67\xae\x37\x2e\xb7\xae\x4c\x77\xa6\xf5"
      "\x75\xe7\xfe\x75\xe7\x4c\xaf\xaf\xaf\xaf\x37\x52\x27\x9b\x39\x27\x72\x2e"
      "\xe5\x5c\xce\xb9\x92\x73\x35\x67\x14\xb9\x7e\xce\x66\x99\xf5\xf5\xf5\x89"
      "\xfc\x7c\x29\xe7\x72\xce\x95\x9c\xab\x39\xa3\x2f\xd7\xcf\xd9\xcc\x39\x91"
      "\x73\x29\xe7\x72\xce\x95\x9c\xab\x39\xa3\x96\xeb\xe7\x6c\xe6\x9c\xc8\xb9"
      "\x94\x73\x39\xe7\x4a\xce\xd5\x9c\x71\x40\xe6\xee\x01\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x8f\x97\xa2\xfa\x2f\xc5\xb7\xbf\xb1"
      "\x96\xd6\x07\x3a\xeb\x4b\x4f\x44\x27\x57\xac\x07\xfa\xd8\xfb\x65\x00\x00"
      "\x00\xff\xff\xe5\xd8\xfc\x18",
      3139);
  syz_mount_image(/*fs=*/0x20000f00, /*dir=*/0x200000c0,
                  /*flags=MS_LAZYTIME|MS_SYNCHRONOUS*/ 0x2000010,
                  /*opts=*/0x20001080, /*chdir=*/1, /*size=*/0xc43,
                  /*img=*/0x20000200);
  *(uint64_t*)0x20000100 = -1;
  *(uint64_t*)0x20000108 = -1;
  syscall(__NR_setrlimit, /*res=RLIMIT_FSIZE*/ 1ul, /*rlim=*/0x20000100ul);
  memcpy((void*)0x200000c0, "./bus\000", 6);
  res = syscall(
      __NR_open, /*file=*/0x200000c0ul,
      /*flags=O_NONBLOCK|O_NOCTTY|O_NOATIME|O_LARGEFILE|O_CREAT|0x82002*/
      0xca942ul, /*mode=*/0ul);
  if (res != -1)
    r[0] = res;
  syscall(__NR_ftruncate, /*fd=*/r[0], /*len=*/0x8002007ffbul);
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
