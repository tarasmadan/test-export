// https://syzkaller.appspot.com/bug?id=ff834988a0e05a23199aa0230d9afa4b010c69d2
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  memcpy((void*)0x20000c40, "udf\000", 4);
  memcpy((void*)0x20000c80, "./file0\000", 8);
  memcpy(
      (void*)0x20001a40,
      "\x00\x99\x17\x59\x3d\x44\xd6\x85\xcf\x81\x76\x52\x18\x46\xa9\xe9\x02\x05"
      "\xb4\xb8\x9c\x0e\xd4\x9b\x3e\x12\x01\xfa\x4a\x79\xb0\xb9\x65\x13\x16\xa8"
      "\x9d\x7e\x40\x38\xe9\x4e\x54\xfd\xff\xa2\x5c\x52\x9d\x1c\xb4\xe4\x3b\xf7"
      "\xe1\x2b\xd2\xa5\x55\x68\x13\x00\xb8\x5d\x66\x21\x47\x0c\x30\x4d\x6b\xa5"
      "\x73\x11\x61\xf3\xf1\xda\x11\x93\xa8\x55\x25\xe8\xc9\xa5\xa9\x57\x98\x07"
      "\x0c\xa4\x8f\xa7\xed\xcf\x62\xe3\x76\x26\x48\x0f\x67\x31\x41\xbe\xe1\xea"
      "\x25\x22\xf8\xb6\x1a\xac\x12\xf9\x84\xc1\x21\x66\x83\xae\x80\xe6\x14\x61"
      "\x69\xcf\xb7\xaa\x7c\x50\xdd\x4c\x52\x25\x9f\xaa\xee\x2f\xed\xc1\x07\x7b"
      "\xda\x4c\x3e\x65\xd7\x00\x5d\x0a\xb7\x1d\xb6\x56\x17\xab\xeb\x3c\x51\xb0"
      "\x56\xd9\x55\xf1\x28\x5e\xd9\xd2\x6d\x7c\x91\x0b\xf3\x29\x1f\x6b\x34\x9c"
      "\xe7\xee\xe3\x3a\x31\xa4\x84\xc3\x19\x93\xef\xfe\x39\xfc\xfa\x55\xe7\x22"
      "\xa2\x0b\xf9\x0b\x2f\x43\xff\xbf\xd1\x9a\xfa\xeb\x1d\x6e\x96\x83\xce\x09"
      "\xf4\xc8\xeb\x95\x91\xf0\x77\x2a\x12",
      225);
  memcpy(
      (void*)0x20000d00,
      "\x78\x9c\xec\xdd\x5d\x6c\x5c\x67\x5a\x07\xf0\xe7\x9d\x63\x27\x76\xca\xb2"
      "\x53\xda\xa6\x5d\xba\x48\xb3\x14\xb1\x69\x9a\x04\xe7\xa3\xad\x51\x5a\xe4"
      "\x6c\x8c\xb5\x2b\x45\x6d\x54\xc7\x0b\x37\x20\x8f\xe3\x49\x18\xd5\x5f\xb5"
      "\x9d\x55\x5a\xc1\x2a\x48\xc0\x0d\x08\x82\x8a\xb4\x02\x2e\xc8\x0d\x12\x17"
      "\x5c\xe4\x06\x09\xad\x10\x8a\xb8\x59\x24\x40\x8a\x40\x95\x16\x81\x44\xa0"
      "\x69\xb4\x12\x02\x66\x05\x0b\x2b\x2a\x61\x74\x66\xde\xb1\xc7\x6e\xd2\xb8"
      "\xf9\xb2\xd3\xfc\x7e\x6d\xfc\x9f\x39\xf3\x9c\x39\xef\x99\xf6\x39\x3e\x13"
      "\xcd\x7b\x26\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x88"
      "\x2f\xfd\xf4\xb1\xa1\x83\x69\xab\x47\x01\x00\x3c\x48\xaf\x8d\xbf\x31\x74"
      "\xd8\xef\x7f\x00\x78\xa4\x9c\xf2\xfe\x1f\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\xdb\x4b\x51"
      "\xc4\x5b\x91\xe2\xbd\xb1\x56\x9a\x6c\xdf\xef\x18\x38\xd1\x9c\x3b\x77\x7e"
      "\x62\x74\xec\xe6\xab\x0d\xa6\x48\x51\x89\xa2\x5d\x5f\xfe\x19\x38\x78\xe8"
      "\xf0\x91\x17\x5f\x7a\x79\xb8\x9b\x1f\xbf\xfe\xbd\xf6\xb9\x78\x7d\xfc\xd4"
      "\xb1\xda\xf1\xf9\xd9\x85\xc5\xc6\xd2\x52\x63\xba\x36\x31\xd7\x3c\x3d\x3f"
      "\xdd\xd8\xf4\x33\xdc\xed\xfa\x1b\xed\x6d\xbf\x00\xb5\xd9\x37\xcf\x4d\x9f"
      "\x39\xb3\x54\x3b\x74\xe0\xf0\xba\x87\xcf\x57\x6f\xec\x7c\x6c\x77\xf5\xe8"
      "\xf0\xb3\xfb\x9e\xef\xd6\x4e\x8c\x8e\x8d\x8d\xf7\xd4\xf4\xf5\xdf\xf1\xd6"
      "\x3f\x22\xdd\xbb\xa7\xe2\x53\x64\x47\x14\xf1\xe5\x48\xf1\xad\xfd\xdf\x49"
      "\xf5\x88\xa8\xc4\xdd\xf7\xc2\x6d\x8e\x1d\xf7\xdb\x60\xf4\x95\xfd\xd7\xde"
      "\x89\x89\xd1\xb1\xf6\x8e\xcc\x34\xeb\x73\xcb\xe5\x83\xa9\x92\xab\xfa\x22"
      "\xaa\x3d\x2b\x8d\x74\x7b\xe4\x01\xf4\xe2\x5d\x19\x89\xb8\x50\xfe\x77\x2a"
      "\x07\xbc\xb7\xdc\xbd\xf1\x85\xfa\x62\x7d\x6a\xa6\x51\x3b\x59\x5f\x5c\x6e"
      "\x2e\x37\xe7\xe7\x52\xa5\x33\xda\x72\x7f\xaa\x51\x89\xe1\x14\xb1\x10\x11"
      "\xad\x62\xab\x07\xcf\x76\xd3\x1f\x45\xbc\x1a\x29\x6e\x7c\xd8\x4a\x53\x11"
      "\x51\x74\xfb\xe0\x85\xd7\xc6\xdf\x18\x3a\x7c\xeb\x15\xfb\x1e\xe0\x20\x6f"
      "\xb1\xf9\x6a\x11\x71\x35\x1e\x82\x9e\x85\x6d\x6a\x67\x14\xf1\xdb\x91\xe2"
      "\xdd\xc9\xa1\x38\x9d\xfb\xaa\xdd\x36\xd7\x23\xbe\x58\xe6\x2b\x11\x6f\x95"
      "\x79\x25\xc5\xc5\x7c\x3f\x95\x07\x88\xe1\x88\xef\xfa\x7d\x02\x0f\xb5\xbe"
      "\x28\xe2\x6f\x22\xc5\x7c\x6a\xa5\xe9\x6e\xef\xb7\xcf\x2b\x4f\x7c\xb5\xf6"
      "\x95\xb9\x33\xf3\x3d\xb5\xdd\xf3\xca\x87\xfe\xfd\xc1\x83\xe4\xdc\x84\x6d"
      "\x6c\x20\x8a\x98\x6a\x9f\xf1\xb7\xd2\x9d\xff\x65\x17\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\xf0\x60\x14\xf1\xcd\x48\x71\x79\x76\x4f\x5a\x88\xde"
      "\x39\xa5\xcd\xb9\xb3\xb5\x53\xf5\xa9\x99\xce\xa7\x82\xbb\x9f\xfd\xaf\xe5"
      "\xb5\x56\x56\x56\x56\xaa\xa9\x93\xb5\x9c\x43\x39\x47\x72\x9e\xcc\x39\x99"
      "\x73\x21\xe7\x85\x9c\x17\x73\x5e\xca\x79\x39\xe7\x95\x9c\x57\x73\x5e\xcb"
      "\xd9\xca\x19\x95\xbc\xfd\x9c\xb5\x9c\x43\x39\x47\x72\x9e\xcc\x39\x99\x73"
      "\x21\xe7\x85\x9c\x17\x73\x5e\xca\x79\x39\xe7\x95\x9c\x57\x73\x5e\xcb\xd9"
      "\xca\x19\xe6\x3d\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x70\x8f\x0d\x46\x11\xbf\x11\x29\xfe\xfd\xf7\xbf\xd6\xfe\x5e\xe9\x68"
      "\x7f\x2f\xfd\x67\x8f\x0e\x1f\x3f\xf1\x99\xde\xef\x8c\x7f\xe6\x36\xcf\x53"
      "\xd6\x1e\x88\x88\x6f\xc6\xe6\xbe\x93\x77\x47\xfe\xae\xf1\x54\x29\xff\xb9"
      "\xf7\xfb\x05\xdc\xde\x40\x14\xf1\xf5\xfc\xfd\x7f\xbf\xbc\xd5\x83\x01\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xb6\x85"
      "\x4a\x14\xf1\x2b\x91\xe2\x1b\xdf\x6b\xa5\x48\x11\x31\x12\x31\x19\x9d\xbc"
      "\x56\x6c\xf5\xe8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x80\xd2\xce\x54\xc4\xab\x91\xe2\x67\x7f\x77\x64\x75"
      "\x59\x5f\x44\xa4\xf6\xbf\x1d\x7b\xca\x1f\x47\x62\xa4\xc8\xf9\x44\x99\xaf"
      "\xc4\xc8\xc1\x76\x56\x46\x8e\x95\x39\x10\x71\x60\x0b\xc6\x0f\xdc\xb9\xa5"
      "\xb7\xdf\x79\xb3\x3e\x33\xd3\x58\x74\xc3\x0d\x37\xdc\x58\xbd\xb1\xd5\x47"
      "\x26\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x78\x84\xa5\x22"
      "\xfe\x3e\x52\xfc\xe4\xef\xb5\x52\x35\x22\xce\x57\x6f\xec\x7c\x6c\x77\xf5"
      "\xe8\xf0\xb3\xfb\x9e\x2f\xa2\x68\x5f\x04\x20\xf5\xd6\xbf\x3e\x7e\xea\x58"
      "\xed\xf8\xfc\xec\xc2\x62\x63\x69\xa9\x31\x5d\x9b\x98\x6b\x9e\x9e\x9f\x6e"
      "\x6c\x76\x73\x03\x27\x9a\x73\xe7\xce\x4f\x8c\x8e\xdd\x97\x9d\xb9\xad\xc1"
      "\xfb\x3c\xfe\xc1\x81\xe3\xf3\x0b\x6f\x2f\x36\xcf\xfe\xc2\xf2\x4d\x1f\xdf"
      "\x35\x70\x6c\x6a\x69\x79\xb1\x7e\xfa\xe6\x0f\xc7\x60\xf4\x45\x0c\xf5\x2e"
      "\xd9\xdb\x1e\xf0\xc4\xe8\x58\x7b\xd0\x33\xcd\xfa\x5c\x7b\xd5\x54\xb9\xc5"
      "\x00\xfb\x22\x6a\x9b\xdd\x19\x1e\x79\xbb\x52\x11\xff\x1b\x29\xde\xdb\xff"
      "\xed\x78\x3c\x2f\xcb\xd7\xff\xe8\xef\xdc\x5b\xeb\xfe\x3f\xfc\xc5\xb5\x7b"
      "\x3f\xdc\xb7\x3e\x57\xff\x77\x6c\x1f\x3f\x3e\x7b\x74\xf8\xf8\xae\xe7\x36"
      "\x73\x3b\x6d\x76\xa0\x7b\xdb\x8d\x57\x36\xc2\xd8\x78\xcf\xe2\xbe\x3c\xca"
      "\x1f\xea\x59\x56\xcd\xe3\xda\xf4\x73\xc3\x23\xaa\xec\xff\x17\x22\xc5\xcf"
      "\xff\x51\x91\xba\x3d\x94\xfb\xff\x07\x3a\xf7\x8a\xd5\xda\xff\xf9\xfa\x5a"
      "\x4f\x1d\xdd\x90\xab\xb6\xa8\xff\x9f\xe8\x59\x76\x34\x1f\xb5\xfa\xfb\x22"
      "\x06\x96\x67\x17\xfa\x9f\x8e\x18\x58\x7a\xfb\x9d\xfd\xcd\xd9\xfa\xd9\xc6"
      "\xd9\xc6\xdc\x91\x43\x2f\xbf\x34\x7c\xe4\xe5\x17\x8f\xbc\xd4\xbf\x23\x62"
      "\xe0\x4c\x73\xa6\x31\xb4\x76\x6b\xd3\xaf\x1d\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\xdc\x2f\xfd\xa9\x88\x2f\x45\x8a\x5f\xfa\xbb\xbf"
      "\x5c\x9d\x37\x9e\xe7\xff\x7d\xa6\x73\x6f\x6d\xfe\x5f\xef\xfc\xdf\x3d\x1b"
      "\x9e\xa7\xf7\xba\x01\xb7\xba\x7d\xd3\xb9\x7e\xb7\x99\xd7\xd7\xab\xdc\x66"
      "\x4a\x45\x3c\x15\x29\x9e\xfd\xb3\x67\xda\xe3\x4d\xb1\xcb\x9c\x77\xb8\x43"
      "\xbb\x52\x11\xdf\x2f\xfb\x69\xfa\xcb\xe9\x0b\x79\x59\xee\xff\x3c\xb3\xff"
      "\xe6\xfd\x7f\x61\x43\xae\xda\xa2\xf9\xbf\x8f\xf7\x2c\xbb\x90\x8f\x13\xff"
      "\x11\x29\x1e\xff\x83\x67\xe2\x0b\x3d\xc7\x89\x8d\xb3\x7b\xcb\xba\xbf\x88"
      "\x14\x53\x3f\xf2\xf9\x5c\x17\x3b\xca\xba\xee\xf3\x75\xe6\x44\x77\x26\x06"
      "\x97\xb5\x5f\x8b\x14\xef\x9f\x5c\x5f\xdb\x9d\x37\xfd\xc4\x5a\xed\xc1\xcd"
      "\xee\x16\x6c\xa5\xb2\xff\x67\x23\xc5\x3f\xfc\xd6\xdf\xc6\x8f\xe6\x65\xeb"
      "\xaf\xff\x71\xf3\xfe\xdf\xb5\x21\x57\x6d\x51\xff\x3f\xd9\xbb\x4f\x11\xb1"
      "\xf4\xf6\x3b\x6f\xd6\x67\x66\x1a\x8b\x4b\x9b\x7e\x29\xe0\x91\x53\xf6\xff"
      "\xaf\x47\x8a\xbf\xfe\x93\x6f\xc7\x73\x79\xd9\xc7\x5d\xff\xa7\x7b\x9d\x9f"
      "\x3d\xcf\xad\xcf\xc1\x6e\xd1\x16\xf5\xff\x53\x3d\xcb\xaa\x79\x5c\x3f\xf6"
      "\x09\x5f\x0b\x00\x00\x00\x00\x00\x00\x00\x00\x00\x78\x58\xec\x4a\x45\xfc"
      "\x53\xa4\xf8\xf3\x3f\xdd\x97\xf6\xe7\x65\x9b\xf9\xfc\xef\xf4\x86\x5c\xb5"
      "\x45\x9f\xff\x7b\xba\x67\xd9\xf4\xba\xcf\xff\xde\xbf\x1b\x9b\x7e\x91\x01"
      "\x00\x60\x9b\xe8\x4f\x45\xfc\x44\xa4\xf8\xe3\xe9\xeb\xa9\x3b\x37\xf6\x96"
      "\xf3\x7f\x5f\x59\x9b\xff\x33\xba\xf1\xc4\xbd\x7d\x4e\xff\x83\xed\x79\xfe"
      "\x9f\xe8\x5c\xff\x13\xcc\xff\x2f\xb7\x99\x52\x11\xff\x97\xe7\xf5\x0e\xdd"
      "\x66\x5e\xef\x8f\x47\x8a\x5f\xfb\xa9\x7d\xb9\x2e\xed\x2e\xeb\x46\xba\xc3"
      "\x6d\xff\x1c\x78\x6d\x7e\x6e\xff\xb1\x99\x99\xf9\xd3\xf5\xe5\xfa\xd4\x4c"
      "\xa3\x36\xbe\x50\x3f\xdd\x28\xd7\xdd\x1b\x29\xfe\xf5\xdf\x3e\x9f\xd7\xad"
      "\xb4\xe7\xf9\x76\xe7\x47\x77\xe6\x06\xaf\xcd\x09\xfe\x9d\x48\xf1\x73\x1f"
      "\x74\x6b\x3b\x73\x82\xbb\x73\x29\x9f\x5c\xab\x3d\x58\xd6\xee\x8f\x14\xdf"
      "\x7f\x7f\x7d\x6d\x77\xde\xd5\x53\x6b\xb5\x87\xca\xda\xdf\x8c\x14\x63\xff"
      "\x7d\xf3\xda\xdd\x6b\xb5\x87\xcb\xda\x7f\x8c\x14\xff\xf9\x6e\xad\x5b\xbb"
      "\xab\xac\xed\xbe\x9f\x7b\x7a\xad\xf6\xc0\xe9\xf9\x99\x8f\xbc\x65\x03\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x60\xeb\xf5\xa7\x22"
      "\x52\xa4\xb8\xf2\x33\x97\x56\xe7\xc6\xaf\xbf\xfe\x57\xf7\x3a\x00\xeb\xaf"
      "\xff\xb5\xd1\xfd\xfa\xfe\xff\xea\xbd\xd9\x4d\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x78\x28\xa4\x28\xe2\xbf\x22\xc5\x7b"
      "\x63\xad\x74\xad\x28\xef\x77\x0c\x9c\x68\xce\x9d\x3b\x3f\x31\x3a\x76\xf3"
      "\xd5\x06\x53\xa4\xa8\x44\xd1\xae\x2f\xff\x0c\x1c\x3c\x74\xf8\xc8\x8b\x2f"
      "\xbd\x3c\xdc\xcd\x8f\x5f\xff\x5e\xfb\x5c\xbc\x3e\x7e\xea\x58\xed\xf8\xfc"
      "\xec\xc2\x62\x63\x69\xa9\x31\x5d\x9b\x98\x6b\x9e\x9e\x9f\x6e\x6c\xfa\x19"
      "\xee\x76\xfd\x8d\xf6\xb6\x5f\x80\xda\xec\x9b\xe7\xa6\xcf\x9c\x59\xaa\x1d"
      "\x3a\x70\x78\xdd\xc3\xe7\xab\x37\x76\x3e\xb6\xbb\x7a\x74\xf8\xd9\x7d\xcf"
      "\x77\x6b\x27\x46\xc7\xc6\xc6\x7b\x6a\xfa\xfa\xef\x78\xeb\x1f\x91\xee\xdd"
      "\x53\xf1\x29\xb2\x23\x8a\xf8\xab\x48\xf1\xad\xfd\xdf\x49\xff\x5c\x44\x54"
      "\xe2\xee\x7b\xe1\x36\xc7\x8e\xfb\x6d\x30\xfa\xca\xfe\x6b\xef\xc4\xc4\xe8"
      "\x58\x7b\x47\x66\x9a\xf5\xb9\xe5\xf2\xc1\x54\xc9\x55\x7d\x11\xd5\x9e\x95"
      "\x46\xba\x3d\xf2\x00\x7a\xf1\xae\x8c\x44\x5c\x88\x88\x4a\x39\xe0\xbd\xe5"
      "\xee\x8d\x2f\xd4\x17\xeb\x53\x33\x8d\xda\xc9\xfa\xe2\x72\x73\xb9\x39\x3f"
      "\x97\x2a\x9d\xd1\x96\xfb\x53\x8d\x4a\x0c\xa7\x88\x85\x88\x68\x15\x5b\x3d"
      "\x78\xb6\x9b\xfe\x28\xe2\x4a\xa4\xb8\xf1\x61\x2b\xfd\x4b\x11\x51\x74\xfb"
      "\xe0\x85\xd7\xc6\xdf\x18\x3a\x7c\xeb\x15\xfb\x1e\xe0\x20\x6f\xb1\xf9\x6a"
      "\x11\x71\x35\x1e\x82\x9e\x85\x6d\x6a\x67\x14\xf1\x64\xa4\x78\x77\x72\x28"
      "\xde\x2f\x3a\x7d\xd5\x6e\x9b\xeb\x11\x5f\x2c\xf3\x95\x88\xb7\xca\xbc\x92"
      "\xe2\x62\xbe\x9f\xca\x03\xc4\x70\xc4\x77\xfd\x3e\x81\x87\x5a\x5f\x14\x71"
      "\x32\x52\xcc\xa7\x56\xba\x5e\xe4\xde\x6f\x9f\x57\x9e\xf8\x6a\xed\x2b\x73"
      "\x67\xe6\x7b\x6a\xbb\xe7\x95\x0f\xfd\xfb\x83\x07\xc9\xb9\x09\xdb\xd8\x40"
      "\x14\xf1\x41\xfb\x8c\xbf\x95\x3e\xf0\xfb\x1c\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\xb6\xb9\x22\x5e\x8d\x14\x97\x67\xf7\xa4\xf6\xfc\xd0\xd5\x39"
      "\xa5\xcd\xb9\xb3\xb5\x53\xf5\xa9\x99\xce\xc7\xfa\xbb\x9f\xfd\xaf\xe5\xb5"
      "\x56\x56\x56\x56\xaa\xa9\x93\xb5\x9c\x43\x39\x47\x72\x9e\xcc\x39\x99\x73"
      "\x21\xe7\x85\x9c\x17\x73\x5e\xca\x79\x39\xe7\x95\x9c\x57\x73\x5e\xcb\xd9"
      "\xca\x19\x95\xbc\xfd\x9c\xb5\x9c\x43\x39\x47\x72\x9e\xcc\x39\x99\x73\x21"
      "\xe7\x85\x9c\x17\x73\x5e\xca\x79\x39\xe7\x95\x9c\x57\x73\x5e\xcb\xd9\xca"
      "\x19\x3e\x27\x0d\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xc0"
      "\x7d\x52\x89\x22\x7e\x35\x52\x7c\xe3\x7b\xad\xb4\x52\x74\xbe\x5f\x76\x32"
      "\x3a\x79\xcd\x3c\x57\xf8\x54\xfb\xff\x00\x00\x00\xff\xff\x38\xfe\x23"
      "\x84",
      3132);
  syz_mount_image(/*fs=*/0x20000c40, /*dir=*/0x20000c80, /*flags=*/0,
                  /*opts=*/0x20001a40, /*chdir=*/1, /*size=*/0xc3c,
                  /*img=*/0x20000d00);
  memcpy((void*)0x20000100, "./file1\000", 8);
  res = syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000100ul,
                /*flags=O_CREAT|O_RDWR*/ 0x42, /*mode=*/0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000100, "./bus\000", 6);
  syscall(__NR_open, /*file=*/0x20000100ul,
          /*flags=O_SYNC|O_NOCTTY|O_NOATIME|O_CREAT|FASYNC|0x2*/ 0x143142ul,
          /*mode=*/0ul);
  memcpy((void*)0x20000040, "./bus\000", 6);
  res = syscall(__NR_open, /*file=*/0x20000040ul,
                /*flags=O_SYNC|O_NOATIME|O_RDWR|0x3c*/ 0x14103eul,
                /*mode=S_IXOTH*/ 1ul);
  if (res != -1)
    r[1] = res;
  syscall(
      __NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x600000ul,
      /*prot=PROT_SEM|PROT_WRITE|PROT_EXEC|0x7ffff0*/ 0x7ffffeul,
      /*flags=MAP_UNINITIALIZED|MAP_LOCKED|MAP_FIXED|MAP_SHARED*/ 0x4002011ul,
      /*fd=*/r[1], /*offset=*/0ul);
  syscall(__NR_ftruncate, /*fd=*/r[1], /*len=*/0x20cf01ul);
  syscall(__NR_read, /*fd=*/r[1], /*buf=*/0x20000f80ul, /*len=*/0xfffffe58ul);
  *(uint32_t*)0x200000c0 = 0x18;
  *(uint32_t*)0x200000c4 = 0;
  *(uint64_t*)0x200000c8 = 0;
  *(uint32_t*)0x200000d0 = 0;
  *(uint32_t*)0x200000d4 = 0;
  syscall(__NR_write, /*fd=*/r[0], /*arg=*/0x200000c0ul, /*len=*/0xfffffdeful);
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
