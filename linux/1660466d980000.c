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
#ifndef __NR_pwritev2
#define __NR_pwritev2 328
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

static long syz_open_procfs(volatile long a0, volatile long a1)
{
  char buf[128];
  memset(buf, 0, sizeof(buf));
  if (a0 == 0) {
    snprintf(buf, sizeof(buf), "/proc/self/%s", (char*)a1);
  } else if (a0 == -1) {
    snprintf(buf, sizeof(buf), "/proc/thread-self/%s", (char*)a1);
  } else {
    snprintf(buf, sizeof(buf), "/proc/self/task/%d/%s", (int)a0, (char*)a1);
  }
  int fd = open(buf, O_RDWR);
  if (fd == -1)
    fd = open(buf, O_RDONLY);
  return fd;
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  memcpy((void*)0x20000080, "udf\000", 4);
  memcpy((void*)0x20000180, "./file1\000", 8);
  memcpy((void*)0x200011c0, "lastblock", 9);
  *(uint8_t*)0x200011c9 = 0x3d;
  sprintf((char*)0x200011ca, "%020llu", (long long)0);
  *(uint8_t*)0x200011de = 0x2c;
  memcpy((void*)0x200011df, "umask", 5);
  *(uint8_t*)0x200011e4 = 0x3d;
  sprintf((char*)0x200011e5, "%023llo", (long long)2);
  *(uint8_t*)0x200011fc = 0x2c;
  memcpy((void*)0x200011fd, "dmode", 5);
  *(uint8_t*)0x20001202 = 0x3d;
  sprintf((char*)0x20001203, "%023llo", (long long)0x7fff);
  *(uint8_t*)0x2000121a = 0x2c;
  memcpy((void*)0x2000121b, "novrs", 5);
  *(uint8_t*)0x20001220 = 0x2c;
  memcpy((void*)0x20001221, "shortad", 7);
  *(uint8_t*)0x20001228 = 0x2c;
  memcpy((void*)0x20001229, "shortad", 7);
  *(uint8_t*)0x20001230 = 0x2c;
  memcpy((void*)0x20001231, "undelete", 8);
  *(uint8_t*)0x20001239 = 0x2c;
  memcpy((void*)0x2000123a, "iocharset", 9);
  *(uint8_t*)0x20001243 = 0x3d;
  memcpy((void*)0x20001244, "cp437", 5);
  *(uint8_t*)0x20001249 = 0x2c;
  memcpy((void*)0x2000124a, "shortad", 7);
  *(uint8_t*)0x20001251 = 0x2c;
  memcpy((void*)0x20001252, "umask", 5);
  *(uint8_t*)0x20001257 = 0x3d;
  sprintf((char*)0x20001258, "%023llo", (long long)6);
  *(uint8_t*)0x2000126f = 0x2c;
  memcpy((void*)0x20001270, "dmode", 5);
  *(uint8_t*)0x20001275 = 0x3d;
  sprintf((char*)0x20001276, "%023llo", (long long)9);
  *(uint8_t*)0x2000128d = 0x2c;
  memcpy((void*)0x2000128e, "fileset", 7);
  *(uint8_t*)0x20001295 = 0x3d;
  sprintf((char*)0x20001296, "%020llu", (long long)0xb);
  *(uint8_t*)0x200012aa = 0x2c;
  memcpy((void*)0x200012ab, "uid", 3);
  *(uint8_t*)0x200012ae = 0x3d;
  sprintf((char*)0x200012af, "%020llu", (long long)0);
  *(uint8_t*)0x200012c3 = 0x2c;
  memcpy((void*)0x200012c4, "session", 7);
  *(uint8_t*)0x200012cb = 0x3d;
  sprintf((char*)0x200012cc, "%020llu", (long long)5);
  *(uint8_t*)0x200012e0 = 0x2c;
  memcpy((void*)0x200012e1, "measure", 7);
  *(uint8_t*)0x200012e8 = 0x2c;
  *(uint8_t*)0x200012e9 = 0;
  memcpy(
      (void*)0x20001300,
      "\x78\x9c\xec\xdd\x4f\x6c\x1c\xd7\x7d\x07\xf0\xdf\x1b\x2d\xc5\x95\xdc\x56"
      "\x4c\xec\x28\x4e\x1a\x17\x9b\xb6\x48\x65\xc5\x72\xf5\x2f\xa6\x62\x15\xee"
      "\xaa\xa6\xd9\x06\x90\x65\x22\x14\x73\x0b\xc0\x15\x49\xa9\x0b\x53\x24\x41"
      "\x52\x8d\x6c\xa4\x05\xd3\x4b\x0f\x3d\x04\x28\x8a\x1e\x72\x22\xd0\x1a\x05"
      "\x52\x34\x30\x9a\x22\xe8\x91\x69\x5d\x20\xb9\xf8\x50\xe4\xd4\x13\xd1\xc2"
      "\x46\x50\xf4\xc0\x16\x01\x72\x0a\x18\xcc\xec\x5b\x71\x49\x91\x36\x2d\x8a"
      "\x12\x65\x7d\x3e\x36\xf5\x9d\x9d\x7d\x6f\xe6\xbd\x99\xd5\x0c\x45\xf0\xcd"
      "\x0b\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x20\xe2\x0f\x5e"
      "\xb9\x74\xfa\x4c\x7a\xd8\xad\x00\x00\x1e\xa4\x2b\xa3\x5f\x3d\x7d\xd6\xfd"
      "\x1f\x00\x1e\x2b\x57\xfd\xfb\x1f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x0e\xba\x14\x45\x3c\x19\x29\xe6\xae\xac\xa5\xf1\xea\x75\x47\xfd\x72"
      "\xbb\xef\xd6\xed\xb1\xa1\xe1\xed\xab\x1d\x49\x55\xcd\x43\x55\xf9\xf2\xab"
      "\x7e\xe6\xec\xb9\xf3\x5f\x7a\x61\xf0\x42\x37\x2f\xb7\x67\x3e\xa0\xfe\xfd"
      "\xf6\xd9\x78\x6d\xf4\xea\xa5\xc6\xcb\xb3\x37\xe7\xe6\xa7\x16\x16\xa6\x26"
      "\x1b\x63\x33\xed\x89\xd9\xc9\xa9\x5d\x6f\x61\xaf\xf5\xb7\x3a\x59\x1d\x80"
      "\xc6\xcd\xd7\x6f\x4d\x5e\xbf\xbe\xd0\x38\xfb\xfc\xb9\x4d\x6f\xdf\x1e\x78"
      "\xbf\xff\x89\xe3\x03\x17\x07\x9f\x3d\xf5\x4c\xb7\xec\xd8\xd0\xf0\xf0\xe8"
      "\x46\x91\x7a\x6f\xf9\xda\x3d\x37\xa4\x63\xa7\x11\x1e\x87\xa3\x88\x53\x91"
      "\xe2\xb9\xef\xfd\x34\xb5\x22\xa2\x88\xbd\x1f\x8b\xfa\x83\x3d\xf7\x5b\x1d"
      "\xa9\x3a\x71\xb2\xea\xc4\xd8\xd0\x70\xd5\x91\xe9\x76\x6b\x66\xb1\x7c\x73"
      "\xa4\x7b\x20\x8a\x88\x46\x4f\xa5\x66\xf7\x18\x6d\x7f\x2e\xa2\xd6\xf7\x40"
      "\xfb\xb0\xb3\x66\xc4\x52\xd9\xfc\xb2\xc1\x27\xcb\xee\x8d\xce\xb5\xe6\x5b"
      "\xd7\xa6\xa7\x1a\x23\xad\xf9\xc5\xf6\x62\x7b\x76\x66\x24\x75\x5a\x5b\xf6"
      "\xa7\x11\x45\x5c\x48\x11\xcb\x11\xb1\xda\x7f\xf7\xe6\xfa\xa2\x88\x5a\xa4"
      "\xf8\xce\xb1\xb5\x74\x2d\x22\x0e\x75\x8f\xc3\x17\xab\x81\xc1\x3b\xb7\xa3"
      "\xd8\xc7\x3e\xee\x42\xd9\xce\x46\x5f\xc4\x72\xf1\x08\x9c\xb3\x03\xac\x3f"
      "\x8a\x78\x35\x52\xfc\xec\x9d\x13\x31\x91\xaf\x33\xd5\xb5\xe6\x0b\x11\xaf"
      "\x96\xf9\x83\x88\xb7\xca\x7c\x29\x22\x95\x1f\x8c\xf3\x11\xef\x6d\xf3\x39"
      "\xe2\xd1\x54\x8b\x22\xfe\xb2\x3c\xff\x17\xd7\xd2\x64\x75\x3d\xe8\x5e\x57"
      "\x2e\x7f\xad\xf1\x95\x99\xeb\xb3\x3d\x65\xbb\xd7\x95\x8f\x78\x7f\xb8\xeb"
      "\x4a\xf1\x90\xee\x0f\x47\xb6\xe4\x83\x71\xc0\xaf\x4d\xf5\x28\xa2\x55\x5d"
      "\xf1\xd7\xd2\xbd\x7f\xb3\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\xc0\xfd\x76\x24\x8a\xf8\x4c\xa4\x78\xe5\x3f\xfe\xa4\x1a\x57\x1c\xd5"
      "\xb8\xf4\x63\x17\x07\xff\x70\xe0\x57\x7b\xc7\x8c\x3f\xfd\x21\xdb\x29\xcb"
      "\x3e\x1f\x11\x4b\xc5\xee\xc6\xe4\x1e\xce\x03\x03\x47\xd2\x48\x4a\x0f\x79"
      "\x2c\xf1\xe3\xac\x1e\x45\xfc\x69\x1e\xff\xf7\xad\x87\xdd\x18\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\xc7\x5a\x11\x3f\x89\x14"
      "\x2f\xbe\x7b\x22\x2d\x47\xef\x9c\xe2\xed\x99\x1b\x8d\xab\xad\x6b\xd3\x9d"
      "\x59\x61\xbb\x73\xff\x76\xe7\x4c\x5f\x5f\x5f\x5f\x6f\xa4\x4e\x36\xab\x4c"
      "\x31\x9e\x5f\x2f\xe5\x5c\xce\xb9\x92\x73\x35\x67\x14\xb9\x7e\xce\x66\xce"
      "\xf1\x9c\x4b\x39\x97\x73\xae\xe4\x5c\xcd\x19\x87\x72\xfd\x9c\xcd\x9c\xe3"
      "\x39\x97\x72\x2e\xe7\x5c\xc9\xb9\x9a\x33\x6a\xb9\x7e\xce\x66\xce\xf1\x9c"
      "\x4b\x39\x97\x73\xae\xe4\x5c\xcd\x19\x07\x64\xee\x5e\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x80\x8f\x93\x22\x8a\xf8\x45\xa4\xf8\xf6\x37\xd6\x52"
      "\xa4\x88\x68\x46\x8c\x47\x27\x57\xfa\x1f\x76\xeb\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x52\x7f\x2a\xe2"
      "\xfb\x91\xa2\xf1\x47\xcd\x3b\xeb\x6a\x11\x91\xaa\xff\x3b\x4e\x94\x7f\x9c"
      "\x8f\xe6\xe1\x32\x3f\x19\xcd\xc1\x32\x5f\x8a\xe6\xa5\x9c\xad\x2a\x6b\xcd"
      "\x6f\x3d\x84\xf6\xb3\x37\x7d\xa9\x88\x1f\x47\x8a\xfe\xfa\xdb\x77\x4e\x78"
      "\x3e\xff\x7d\x9d\x57\x77\x3e\x06\xf1\xd6\x37\x37\x5e\x7d\xb6\xd6\xc9\x43"
      "\xdd\x37\x07\xde\xef\x7f\xe2\xf8\xb1\x8b\x83\xc3\xbf\xf1\xf4\x4e\xcb\x69"
      "\xbb\x06\x9c\xbc\xdc\x9e\xb9\x75\xbb\x31\x36\x34\x3c\x3c\xda\xb3\xba\x96"
      "\xf7\xfe\xc9\x9e\x75\x03\x79\xbf\xc5\xfd\xe9\x3a\x11\xb1\xf0\xc6\x9b\xaf"
      "\xb7\xa6\xa7\xa7\xe6\xef\x7d\xa1\xfc\x08\xec\xa1\xfa\x23\xb4\x90\x6a\x8f"
      "\x4b\x4f\x2d\x54\x0b\x51\x7b\xc0\x3b\xed\x8f\x87\xdd\xe5\x8d\xbe\xf3\x18"
      "\x28\xef\xff\xef\x45\x8a\xdf\x7d\xf7\x3f\xbb\x37\xfc\xce\xfd\xbf\x1e\xbf"
      "\xd2\x79\x75\xe7\x0e\x1f\x3f\xff\xb3\x8d\xfb\xff\x8b\x5b\x37\xb4\xcb\xfb"
      "\x7f\x6d\x6b\xbd\x7c\xff\x2f\xef\xe9\xdb\xdd\xff\x9f\xec\x59\xf7\x62\xfe"
      "\x6e\xa4\xaf\x16\x51\x5f\xbc\x39\xd7\x77\x3c\xa2\xbe\xf0\xc6\x9b\xa7\xda"
      "\x37\x5b\x37\xa6\x6e\x4c\xcd\x9c\x3f\x7d\xfa\xcb\x83\x83\x5f\x3e\x77\xba"
      "\xef\x70\x44\xfd\x7a\x7b\x7a\xaa\x67\xe9\xbe\x1c\x2e\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x80\x07\x27\x15\xf1\xfb\x91\xa2\xf5\xe3\xb5"
      "\xd4\x88\x88\xdb\xd5\x78\xad\x81\x8b\x83\xcf\x9e\x7a\xe6\x50\x1c\xaa\xc6"
      "\x5b\x6d\x1a\xb7\xfd\xda\xe8\xd5\x4b\x8d\x97\x67\x6f\xce\xcd\x4f\x2d\x2c"
      "\x4c\x4d\x36\xc6\x66\xda\x13\xb3\x93\x53\xbb\xdd\x5d\xbd\x1a\xee\x35\x36"
      "\x34\xbc\x2f\x9d\xf9\x50\x47\xf6\xb9\xfd\x47\xea\x2f\xcf\xce\xbd\x31\xdf"
      "\xbe\xf1\xc7\x8b\xdb\xbe\x7f\xb4\x7e\xe9\xda\xc2\xe2\x7c\x6b\x62\xfb\xb7"
      "\xe3\x48\x14\x11\xcd\xde\x35\x27\xab\x06\x8f\x0d\x0d\x57\x8d\x9e\x6e\xb7"
      "\x66\xaa\xaa\x23\xdb\x0e\xa6\xff\xe8\xfa\x52\x11\xff\x15\x29\x26\xce\x37"
      "\xd2\xe7\xf3\xba\x3c\xfe\x7f\xeb\x08\xff\x4d\xe3\xff\x97\xb6\x6e\x68\x9f"
      "\xc6\xff\x7f\xa2\x67\x5d\xb9\xcf\x94\x8a\xf8\x79\xa4\xf8\x9d\xbf\x7a\x3a"
      "\x3e\x5f\xb5\xf3\x68\xdc\x75\xcc\x72\xb9\xbf\x8b\x14\x27\x2f\x7c\x2e\x97"
      "\x8b\xc3\x65\xb9\x6e\x1b\x3a\xcf\x15\xe8\x8c\x0c\x2c\xcb\xfe\x5f\xa4\xf8"
      "\xa7\x5f\x6c\x2e\xdb\x1d\x0f\xf9\xe4\x46\xd9\x33\xbb\x3e\xb0\x8f\x88\xf2"
      "\xfc\x1f\x8b\x14\xdf\xff\x8b\xef\xc6\x6f\xe6\x75\x9b\x9f\xff\xb0\xfd\xf9"
      "\x3f\xba\x75\x43\xfb\x74\xfe\x9f\xea\x59\x77\x74\xd3\xf3\x0a\xf6\xdc\x75"
      "\xf2\xf9\x3f\x15\x29\x5e\x7a\xf2\xed\xf8\xad\xbc\xee\x83\x9e\xff\xd1\x7d"
      "\xf6\xc6\x89\x5c\xf8\xce\xf3\x39\xf6\xe9\xfc\x7f\xaa\x67\xdd\x40\xde\xef"
      "\x6f\xdf\x9f\xae\x03\x00\x00\x00\x00\x00\x00\x00\x00\x3c\xd2\xfa\x52\x11"
      "\x7f\x1f\x29\x7e\x38\x5c\x4b\x2f\xe4\x75\xbb\xf9\xfd\xbf\xc9\xad\x1b\xda"
      "\xa7\xdf\xff\xfa\x74\xcf\xba\xc9\xfb\x33\x5f\xd1\x87\x2e\xec\xf9\xa0\x02"
      "\x00\x00\x00\xc0\x01\xd1\x97\x8a\xf8\x49\xa4\xb8\xb1\xf8\xf6\x9d\x31\xd4"
      "\x9b\xc7\x7f\xf7\x8c\xff\xfc\xbd\x8d\xf1\x9f\x43\x69\xcb\xbb\xd5\xcf\xf9"
      "\x7e\xad\x7a\x6e\xc0\xfd\xfc\xf9\x5f\xaf\x81\xbc\xdf\xf1\xbd\x77\x1b\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0e\x94\x94\x8a"
      "\x78\x21\xcf\xa7\x3e\x5e\x8d\xe7\x9f\xdc\x71\x3e\xf5\x95\x48\xf1\xca\xff"
      "\x3c\x97\xcb\xa5\xe3\x65\xb9\xee\x3c\xf0\x03\xd5\x9f\xf5\x2b\xb3\x33\xa7"
      "\x2e\x4d\x4f\xcf\x4e\xb4\x16\x5b\xd7\xa6\xa7\x1a\xa3\x73\xad\x89\xa9\xb2"
      "\xee\x53\x91\x62\xed\x6f\x3f\x97\xeb\x16\xd5\xfc\xea\xdd\xf9\xe6\x3b\x73"
      "\xbc\x6f\xcc\xc5\x3e\x1f\x29\x86\xff\xa1\x5b\xb6\x33\x17\x7b\x77\x6e\xf2"
      "\xa7\x36\xca\x9e\x29\xcb\x7e\x22\x52\xfc\xf7\x3f\x6e\x2e\xdb\x9d\xc7\xfa"
      "\x53\x1b\x65\xcf\x96\x65\xff\x26\x52\x7c\xfd\x5f\xb6\x2f\x7b\x7c\xa3\xec"
      "\xb9\xb2\xec\x77\x23\xc5\x8f\xbe\xde\xe8\x96\x3d\x5a\x96\xed\x3e\x1f\xf5"
      "\xd3\x1b\x65\x9f\x9f\x98\x2d\xf6\xe1\xac\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\xf0\xb8\xe9\x4b\x45\xfc\x79\xa4\xf8\xdf\x9b"
      "\xcb\x77\xc6\xf2\xe7\xf9\xff\xfb\x7a\x5e\x56\xde\xfa\x66\xcf\x7c\xff\x5b"
      "\xdc\xae\xe6\xf9\x1f\xa8\xe6\xff\xdf\x69\xf9\x5e\xe6\xff\xaf\x9e\x2b\xb0"
      "\xb4\xd3\x5e\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\xe0\xe3\x29\x45\x11\x6f\x46\x8a\xb9\x2b\x6b\x69\xa5\xbf\x7c\xdd"
      "\x51\xbf\xdc\x9e\xb9\x75\x7b\x6c\x68\x78\xfb\x6a\x47\x52\x55\xf3\x50\x55"
      "\xbe\xfc\xaa\x9f\x39\x7b\xee\xfc\x97\x5e\x18\xbc\xd0\xcd\x0f\xae\x7f\xbf"
      "\x7d\x26\x5e\x1b\xbd\x7a\xa9\xf1\xf2\xec\xcd\xb9\xf9\xa9\x85\x85\xa9\xc9"
      "\xc6\xd8\x4c\x7b\x62\x76\x72\x6a\xd7\x5b\xd8\x6b\xfd\xad\x4e\x56\x07\xa0"
      "\x71\xf3\xf5\x5b\x93\xd7\xaf\x2f\x34\xce\x3e\x7f\x6e\xd3\xdb\xb7\x07\xde"
      "\xef\x7f\xe2\xf8\xc0\xc5\xc1\x67\x4f\x3d\xd3\x2d\x3b\x36\x34\x3c\x3c\xda"
      "\x53\xa6\xd6\x77\xcf\x7b\xbf\x4b\xda\x61\xfd\xe1\x28\xe2\xaf\x23\xc5\x73"
      "\xdf\xfb\x69\xfa\x61\x7f\x44\x11\x7b\x3f\x16\x1f\xf2\xd9\xd9\x6f\x47\xaa"
      "\x4e\x9c\xac\x3a\x31\x36\x34\x5c\x75\x64\xba\xdd\x9a\x59\x2c\xdf\x1c\xe9"
      "\x1e\x88\x22\xa2\xd1\x53\xa9\xd9\x3d\x46\x0f\xe0\x5c\xec\x49\x33\x62\xa9"
      "\x6c\x7e\xd9\xe0\x93\x65\xf7\x46\xe7\x5a\xf3\xad\x6b\xd3\x53\x8d\x91\xd6"
      "\xfc\x62\x7b\xb1\x3d\x3b\x33\x92\x3a\xad\x2d\xfb\xd3\x88\x22\x2e\xa4\x88"
      "\xe5\x88\x58\xed\xbf\x7b\x73\x7d\x51\xc4\xeb\x91\xe2\x3b\xc7\xd6\xd2\xbf"
      "\xf6\x47\x1c\xea\x1e\x87\x2f\x5e\x19\xfd\xea\xe9\xb3\x3b\xb7\xa3\xd8\xc7"
      "\x3e\xee\x42\xd9\xce\x46\x5f\xc4\x72\xf1\x08\x9c\xb3\x03\xac\x3f\x8a\xf8"
      "\xe7\x48\xf1\xb3\x77\x4e\xc4\xbf\xf5\x47\xd4\xa2\xf3\x15\x5f\x88\x78\xb5"
      "\xcc\x1f\x44\xbc\x15\xb1\x5e\x2e\xa6\xf2\x83\x71\x3e\xe2\xbd\x6d\x3e\x47"
      "\x3c\x9a\x6a\x51\xc4\xff\x97\xe7\xff\xe2\x5a\x7a\x67\x7d\x3d\xaa\xbf\x32"
      "\xd5\x75\xe5\xf2\xd7\x1a\x5f\x99\xb9\x3e\xdb\x53\xb6\x7b\x5d\xe9\xbd\x3f"
      "\x94\xd7\xa2\x47\xee\xfe\xf0\x20\x1d\xf0\x6b\x53\x3d\x8a\xf8\x51\x75\xc5"
      "\x5f\x4b\xff\xee\xef\x35\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\xc0\x01\x52\xc4\xaf\x47\x8a\x17\xdf\x3d\x91\xaa\xf1\xc1\x77\xc6\x14\xb7"
      "\x67\x6e\x34\xae\xb6\xae\x4d\x77\x86\xf5\x75\xc7\xfe\x75\xc7\x4c\xaf\xaf"
      "\xaf\xaf\x37\x52\x27\x9b\x39\xc7\x73\x2e\xe5\x5c\xce\xb9\x92\x73\x35\x67"
      "\x14\xb9\x7e\xce\x66\x99\xf5\xf5\xf5\xf1\xfc\x7a\x29\xe7\x72\xce\x95\x9c"
      "\xab\x39\xe3\x50\xae\x9f\xb3\x99\x73\x3c\xe7\x52\xce\xe5\x9c\x2b\x39\x57"
      "\x73\x46\x2d\xd7\xcf\xd9\xcc\x39\x9e\x73\x29\xe7\x72\xce\x95\x9c\xab\x39"
      "\xe3\x80\x8c\xdd\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x3e\x5e\x8a\xea\xbf\x14\xdf\xfe\xc6\x5a\x5a\xef\xef\xcc\x2f\x3d\x1e"
      "\x9d\x5c\x31\x1f\xe8\xc7\xde\x2f\x03\x00\x00\xff\xff\x6d\x2a\xf7\x95",
      3131);
  syz_mount_image(/*fs=*/0x20000080, /*dir=*/0x20000180,
                  /*flags=MS_REC*/ 0x4000, /*opts=*/0x200011c0, /*chdir=*/2,
                  /*size=*/0xc3b, /*img=*/0x20001300);
  memcpy((void*)0x200003c0, "net/softnet_stat\000", 17);
  res = -1;
  res = syz_open_procfs(/*pid=*/0, /*file=*/0x200003c0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000200, "./bus\000", 6);
  res = syscall(
      __NR_open, /*file=*/0x20000200ul,
      /*flags=O_TRUNC|O_SYNC|O_NONBLOCK|O_NOATIME|O_CREAT|O_RDWR*/ 0x141a42ul,
      /*mode=*/0ul);
  if (res != -1)
    r[1] = res;
  syscall(__NR_sendfile, /*fdout=*/r[1], /*fdin=*/r[0], /*off=*/0ul,
          /*count=*/0x100800001ul);
  memcpy((void*)0x20001780, "./bus\000", 6);
  res = syscall(
      __NR_open, /*file=*/0x20001780ul,
      /*flags=O_NONBLOCK|O_NOFOLLOW|O_NOATIME|O_DIRECT|O_CREAT|0x2*/ 0x64842ul,
      /*mode=*/0ul);
  if (res != -1)
    r[2] = res;
  *(uint64_t*)0x20000240 = 0x20000000;
  memset((void*)0x20000000, 133, 1);
  *(uint64_t*)0x20000248 = 1;
  syscall(__NR_pwritev2, /*fd=*/r[2], /*vec=*/0x20000240ul, /*vlen=*/1ul,
          /*off_low=*/0x1ffffff, /*off_high=*/0, /*flags=*/0ul);
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
