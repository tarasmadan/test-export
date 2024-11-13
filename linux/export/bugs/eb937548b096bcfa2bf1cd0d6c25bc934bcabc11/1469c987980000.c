// https://syzkaller.appspot.com/bug?id=eb937548b096bcfa2bf1cd0d6c25bc934bcabc11
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
  memcpy((void*)0x20000100, "hfsplus\000", 8);
  memcpy((void*)0x20000080, "./file1\000", 8);
  memcpy((void*)0x20000140, "nobarrier", 9);
  *(uint8_t*)0x20000149 = 0x2c;
  memcpy((void*)0x2000014a, "gid", 3);
  *(uint8_t*)0x2000014d = 0x3d;
  sprintf((char*)0x2000014e, "0x%016llx", (long long)0);
  *(uint8_t*)0x20000160 = 0x2c;
  memcpy((void*)0x20000161, "decompose", 9);
  *(uint8_t*)0x2000016a = 0x2c;
  memcpy((void*)0x2000016b, "uid", 3);
  *(uint8_t*)0x2000016e = 0x3d;
  sprintf((char*)0x2000016f, "0x%016llx", (long long)0);
  *(uint8_t*)0x20000181 = 0x2c;
  memcpy((void*)0x20000182, "force", 5);
  *(uint8_t*)0x20000187 = 0x2c;
  memcpy((void*)0x20000188, "umask", 5);
  *(uint8_t*)0x2000018d = 0x3d;
  sprintf((char*)0x2000018e, "%023llo", (long long)8);
  *(uint8_t*)0x200001a5 = 0x2c;
  memcpy((void*)0x200001a6, "nls", 3);
  *(uint8_t*)0x200001a9 = 0x3d;
  memcpy((void*)0x200001aa, "default", 7);
  *(uint8_t*)0x200001b1 = 0x2c;
  *(uint8_t*)0x200001b2 = 0;
  memcpy(
      (void*)0x20000cc0,
      "\x78\x9c\xec\xdd\x4f\x6c\x1b\x59\x19\x00\xf0\x6f\x6c\xc7\x8d\xbb\x52\xea"
      "\xdd\x6d\x77\x17\x84\xd4\x88\x8a\x0a\xb6\xd0\x26\x31\x4b\x0b\x42\xa2\x20"
      "\x84\x72\x58\xa1\x4a\x5c\xf6\x1a\xda\x74\x1b\xc5\xc9\x56\x49\x16\xa5\x15"
      "\xa2\x5e\x60\xe1\x08\x27\xd4\x03\x87\x45\x28\x1c\xf6\x84\xf6\x80\x04\xe2"
      "\x80\x58\xce\x48\x48\x5c\x51\xef\x95\xb8\x57\x1c\x30\x9a\xf1\x8c\x63\x3b"
      "\x89\x63\xb7\xa9\x53\xba\xbf\x9f\x34\x9e\x37\x33\xef\xdf\x7c\x7d\xf3\x3c"
      "\x1e\x37\x72\x00\x9f\x58\x8b\x6f\xc5\x54\x2b\x92\x58\xbc\xf0\xe6\x76\xba"
      "\xfd\x60\xa7\xd1\x7c\xb0\xd3\x58\x2b\xd2\x11\x71\x22\x22\x4a\x11\x95\xce"
      "\x2a\x92\xf5\x88\xe4\xe3\x88\xab\xd1\x59\xe2\x53\xe9\xce\xbc\xba\xe4\xa0"
      "\x76\xde\x78\xf8\xc7\x5f\x9f\xbf\xff\x61\xa3\xb3\x55\xc9\x97\x2c\x7f\x69"
      "\x58\xb9\x5d\xed\x21\x2d\xb4\xf2\x25\x66\x23\xa2\x9c\xaf\xc7\x54\x39\xa8"
      "\xbe\xeb\xb5\xbd\x99\xef\x8d\x55\x75\xd2\xed\x77\x1a\xb0\x73\x45\xe0\xe0"
      "\xb8\xb5\xf7\x68\x8d\x53\x7c\x84\xeb\x16\x78\xd6\xdd\x8b\x28\x4f\xed\xb3"
      "\xbf\x1e\x71\x32\x22\xa6\xf3\xfb\x80\xc8\x67\x87\xd2\x84\xbb\x77\xe4\xc6"
      "\x9a\xe5\x00\x00\x00\xe0\xd9\x54\x3e\x2c\xc3\xa9\x47\xf1\x28\xb6\x63\x66"
      "\x32\xdd\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\xe7\x43\xd2\xf9\xcd\xc0"
      "\x24\x5f\x4a\x45\x7a\x36\x92\xe2\xf7\xff\xab\xf9\xbe\x54\xb5\x7a\xcc\xfd"
      "\x1d\xee\x8b\x83\x3b\xa6\xfb\x37\xdf\xbf\x39\xc1\xce\x00\x00\x00\x00\x00"
      "\x00\x00\xc0\x91\xfb\x28\xff\xe2\xfe\xec\xa3\x78\x14\xdb\x31\x53\xec\x6f"
      "\x27\xd9\x77\xfe\x9f\xcd\x36\x4e\x67\xaf\x2f\xc4\xbb\xb1\x19\xcb\xb1\x11"
      "\x17\x63\x3b\x96\x62\x2b\xb6\x62\x23\xe6\x23\xa6\x66\x7a\x2a\xac\x6e\x2f"
      "\x6d\x6d\x6d\xcc\xef\x2d\xf9\xab\x48\x4b\xb6\xdb\xed\x7b\x79\xc9\x85\x88"
      "\xa8\xef\x29\xb9\x30\x81\x93\x06\x00\x00\x00\x00\x00\x00\x80\xe7\xd7\x8f"
      "\x63\x31\x66\x8e\xbb\x13\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xd0"
      "\x2b\x89\x28\x77\x56\xd9\x72\xba\x48\xd7\xa3\x54\x89\x88\xe9\x88\xa8\xa6"
      "\xf9\x5a\x11\x7f\x2e\xd2\xcf\xba\xda\x90\x63\x7f\x99\x60\x3f\x00\x00\x00"
      "\xe0\x98\x14\x1f\x8d\x67\x92\xff\x76\x12\xed\x24\xfb\xcc\xff\x4a\xf6\xb9"
      "\x7f\x3a\xde\x8d\xf5\xd8\x8a\x95\xd8\x8a\x66\x2c\xc7\x8d\xec\x59\x40\xe7"
      "\x53\x7f\xe9\x1f\xad\x46\xf3\xc1\x4e\x63\x2d\x5d\xf6\x56\xfc\xcd\x7f\x8f"
      "\xd5\x8f\xac\xc6\x88\x28\xc7\x7b\x07\xb4\x3c\x97\xe5\x38\xd3\x2d\xb1\x18"
      "\xdf\x89\xef\xc5\x85\x98\x8d\x6b\xb1\x11\x2b\xf1\x83\x58\x8a\xad\x58\x8e"
      "\xd9\xa8\xa5\x27\x11\x4b\x91\x44\xbd\xd6\x79\x7a\x51\x2f\xfa\xb9\x7f\x7f"
      "\xaf\xf6\x6d\x5d\x1b\xec\xdb\xd9\x81\xed\xd7\xb2\x9e\xd4\xe2\x66\xac\x64"
      "\x7d\xbb\x18\xd7\xab\xd1\x79\x6c\x92\x9d\x43\xda\xe6\x6b\x3d\xad\xfd\xa1"
      "\x1a\x31\xd0\xe2\x7b\x69\x74\x92\x6f\xe4\x46\x8c\xd1\x8d\x9e\x7f\xaf\x5f"
      "\xe6\xcf\x65\x72\xed\x53\x23\xd6\xf1\x74\xd4\xb3\x33\x9f\xea\x46\x64\x2e"
      "\x8d\x7d\x1e\x8d\x17\x87\xc7\x7e\xcc\x71\x32\xd8\xd2\x7c\x94\xba\xcf\xa0"
      "\x4e\xef\xb6\x92\x6e\xee\xac\x45\x7c\x2d\x0d\x52\xab\x73\xb8\x88\xf9\xf7"
      "\xc7\x89\xf9\xc9\x7c\x9d\xc6\xfa\x67\xfd\x31\x3f\x6a\x63\x3e\x4a\x1b\x8c"
      "\xc4\x42\x94\xf2\xd1\x17\xf1\x4a\x7f\xcc\x6f\x7f\xee\xfe\x4b\xfd\x85\xbf"
      "\xf0\xcf\xbf\x5e\xbb\x55\x5a\x5f\xbd\x75\x73\xf3\xc2\x53\x3c\xa5\x27\x31"
      "\x7b\x58\x86\xa9\x22\x31\x18\x89\x46\x4f\x24\x5e\x1d\x3e\xfa\xf2\x48\x34"
      "\xd3\x48\xb4\x46\x8f\xc4\xd4\xe0\x8e\xe9\x51\x4b\x3e\x5d\xd5\x3c\x1a\xd9"
      "\x54\x34\xe2\x6c\xf9\xed\xf4\xf0\xa9\xdd\xf2\xa9\x77\xe2\x46\x2c\xc7\xe5"
      "\x98\x8b\xf9\xb8\x12\x73\xf1\xd5\x58\x88\x46\x77\x84\xa5\xcb\x99\xbe\xb8"
      "\x56\x1a\x6b\xfd\x31\xc9\xae\xb5\xd2\xde\xf9\x6d\xd8\x93\xd8\x73\x9f\xef"
      "\xc9\xf4\xf3\x43\x32\x4f\x56\x1a\x97\x17\x7b\xe2\xda\x3b\xd3\xd5\xb3\x63"
      "\xf9\x9e\xab\xbf\x88\xb9\x9e\xd1\xf7\xd2\xf0\xd1\x37\xf6\xbb\x40\xda\xfe"
      "\xa7\xf3\x74\xda\xc6\x4f\xba\xef\x38\xcf\x82\xbe\x48\xe4\x73\x73\xd1\xbb"
      "\x97\x87\x47\xe2\x37\xed\xf4\x75\xb3\xb9\xbe\xba\x71\x6b\xe9\xf6\x88\xed"
      "\x9d\xcf\xd7\xe9\x65\xfb\x7e\xff\xdc\xfc\xdb\xd1\x7b\x3d\xf8\xee\x7e\x14"
      "\xd2\xf1\x92\xce\xb8\x95\x6c\x2b\x8b\x49\xad\x18\x2f\xe9\xb1\x97\xbb\xbd"
      "\xed\x8f\x57\x35\xff\xc6\xa5\x53\xae\xb4\xe7\xd8\x99\xee\xb1\x7a\xcc\xc4"
      "\x4a\x7c\xb7\xb8\x52\xbb\x83\xa0\xb8\x52\xab\xf9\x3d\xdc\xde\x9a\x3a\xc7"
      "\x5e\xed\x3d\xf6\xaf\xdd\x99\xb3\x9a\xdf\xdf\x14\xc7\xfa\xee\x72\xe2\x9d"
      "\x68\x66\x77\x21\x03\x0e\x9d\xaa\x01\x98\xb0\x93\xaf\x9f\xac\xd6\x1e\xd6"
      "\xfe\x5e\xfb\xa0\xf6\xd3\xda\xad\xda\x9b\xd3\xdf\x3a\x71\xe5\xc4\x67\xaa"
      "\x31\xf5\xb7\xca\x9f\xca\xbf\x2f\xfd\xae\xf4\xf5\xe4\xf5\xf8\x20\x7e\x14"
      "\x33\xc7\xdd\x53\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x78\x1e\x6c\xde\xb9\xbb\xba\xd4\x6c"
      "\x2e\x6f\x74\x13\x31\x3d\xb8\xe7\x49\x13\xd5\x03\xdb\x1a\x9e\x88\x52\x33"
      "\x2d\x3a\x2c\xcf\xce\x0b\xa3\x55\x18\xf5\xe1\xf5\xdc\x8d\x24\x4f\x54\x8f"
      "\xf4\xdc\x8b\x5f\xc7\x3a\xba\x60\x4e\x20\x51\x8b\x81\x3d\xc9\x11\x9d\xc5"
      "\x47\x11\x31\x24\x4f\xf5\x89\x3b\x9f\x8c\x3c\xc6\xca\x8f\x7b\x3a\x69\x1c"
      "\x8e\x24\xce\xc5\xd0\xc8\xf6\xb4\xcb\x63\x14\xaf\x14\xa5\xf6\xcf\x53\x89"
      "\xcd\xe9\x58\x5d\x4a\x2a\xfb\x5c\x71\x27\x76\xaf\x82\xa8\xaf\x2e\x35\xff"
      "\xd3\xee\x2b\x5e\x8b\x9e\x4b\x06\x78\xce\x5d\xda\x5a\xbb\x7d\x69\xf3\xce"
      "\xdd\x2f\xad\xac\x2d\xbd\xbd\xfc\xf6\xf2\xfa\xc2\x95\xcb\x57\x2e\x37\xbe"
      "\x32\xff\xe5\x4b\x37\x57\x9a\xcb\x73\x9d\xd7\xe3\xee\x25\xf0\x34\x6c\xde"
      "\xb9\x5b\x3e\xee\x3e\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xe3\xc9\xff"
      "\xf7\xff\xd6\x63\xff\x31\x43\xe5\x90\x3c\xd5\x8d\xcd\xfd\x5b\x3e\x3b\xe9"
      "\x53\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\xfe\x4f\x2d\xbe\x15\x53\xad\x48\x62\x7e\xee\xe2\x5c"
      "\xba\xfd\x60\xa7\xd1\x4c\x97\x22\xbd\x9b\xb3\x12\x11\xa5\x88\x48\x7e\x18"
      "\x91\x7c\x1c\x71\x35\x3a\x4b\xd4\x7b\xaa\x4b\x0e\x6a\xe7\x8d\x87\x51\x3a"
      "\x7f\xff\xc3\xc6\x6e\x5d\x95\x22\x7f\x69\x58\xb9\xd1\xb4\xf2\x25\x66\x23"
      "\xa2\x9c\xaf\x0f\x77\x62\x9f\x6a\xf6\xd6\x77\xbd\xa7\xbe\xd6\x63\x75\x2f"
      "\xe9\x9e\x61\x1a\xb0\x73\x45\xe0\xe0\xb8\xfd\x2f\x00\x00\xff\xff\xa2\x22"
      "\xeb\xc7",
      1802);
  syz_mount_image(/*fs=*/0x20000100, /*dir=*/0x20000080,
                  /*flags=MS_DIRSYNC*/ 0x80, /*opts=*/0x20000140,
                  /*chdir=*/0x44, /*size=*/0x70a, /*img=*/0x20000cc0);
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
