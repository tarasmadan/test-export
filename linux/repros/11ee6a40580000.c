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
  memcpy((void*)0x20000000, "hfsplus\000", 8);
  memcpy((void*)0x20000080, "./file0\000", 8);
  memcpy(
      (void*)0x20000a80,
      "\x70\x61\x72\x74\x3d\x30\x78\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30"
      "\x30\x30\x30\x30\x36\x2c\x6e\x6c\x73\x3d\x69\x73\x6f\x38\x38\x35\x39\x2d"
      "\x31\x34\x2c\x73\x65\x73\x73\x69\x6f\x6e\x3d\x30\x78\x66\x66\x66\x66\x66"
      "\x66\x66\x66\x66\x66\x66\x66\x66\x66\x37\x66\x2c\x62\x61\x72\x72\x69\x65"
      "\x72\x2c\x63\x72\x65\x61\x74\x6f\x72\x3d\xdd\xf2\xbd\x6c\x2c\x6e\x6f\x64"
      "\x65\x63\x6f\x6d\x70\x6f\x73\x65\x2c\x75\x69\x64\x3d",
      103);
  sprintf((char*)0x20000ae7, "0x%016llx", (long long)0);
  memcpy(
      (void*)0x20000af9,
      "\x2c\x74\x79\x70\x65\x3d\x63\x9e\xc4\x8c\x2c\x70\x61\x72\x74\x3d\x30\x78"
      "\x30\x00\x80\x00\x00\x00\x00\x00\x00\x30\x30\x30\x30\x66\x66\x66\x2c\x62"
      "\x61\x72\x72\x69\x65\x72\x2c\x63\x72\x65\x61\x74\x6f\x72\x3d\x65\xfe\x04"
      "\xc2\x2c\x62\x61\x72\x72\x69\x65\x72\x2c\x64\x65\x63\x6f\x6d\x70\x6f\x73"
      "\x65\x2c\x62\x61\x72\x72\x69\x65\x72\x2c\x67\x69\x64\x3d",
      86);
  sprintf((char*)0x20000b4f, "0x%016llx", (long long)0);
  memcpy((void*)0x20000b61, ",nls=cp737,barrier,uid=", 23);
  sprintf((char*)0x20000b78, "0x%016llx", (long long)0);
  memcpy((void*)0x20000b8a, ",\000", 2);
  memcpy(
      (void*)0x20000bc0,
      "\x78\x9c\xec\xdd\x4b\x68\x1c\xe7\x1d\x00\xf0\xff\xec\xae\x56\xbb\x2a\x38"
      "\x72\xe2\x47\x5a\x02\x59\x62\x48\x4b\x45\x6d\xc9\x42\x69\xd5\x4b\xdd\x52"
      "\x8a\x0e\xa1\x84\xf4\xd0\xf3\x62\xcb\xb1\xf0\x5a\x0e\x92\x52\x64\xd3\x36"
      "\x4a\x1f\xf7\x1e\x72\xea\x29\x3d\xe8\x16\x7a\x28\xe9\xdd\xd0\x9e\x1b\x02"
      "\x25\x57\x1d\x03\x85\x5e\x72\xd2\x4d\x65\x66\x67\x56\x2b\xed\x53\xb2\x2c"
      "\xc9\xee\xef\x67\x66\xe6\x9b\xf9\x9e\xf3\x9f\x9d\x99\x9d\x15\x66\x02\xf8"
      "\xbf\xb5\x34\x13\x95\x27\x91\xc4\xd2\xcc\xdb\x9b\xe9\xfa\xce\xf6\x7c\xab"
      "\xbc\x3d\x3f\x99\x67\xb7\x22\xa2\x1a\x11\xa5\x88\x4a\x7b\x11\xc9\x6a\x64"
      "\xb9\xb7\xf2\x29\xbe\x99\x6e\xcc\xcb\x27\x83\xfa\xf9\x78\x65\xf1\xdd\x2f"
      "\xbf\xde\xf9\xaa\xbd\x56\xc9\xa7\xac\x7c\x69\x58\xbd\x3e\xaa\xbd\x9b\xb6"
      "\xf2\x29\x1a\x11\x51\xce\x97\xbd\x26\x06\xb4\xf8\xd9\xe1\xee\x0f\xb4\x77"
      "\x7b\x60\x7b\xe3\x4a\x3a\x7b\x98\x06\xec\x5a\x11\xb8\xf8\xf3\x53\xb5\x0a"
      "\x4f\x6d\xaf\xc7\x56\x27\xef\xd3\x7f\x67\xf3\x61\xd5\x8f\x72\xde\x02\xe7"
      "\x54\xd2\xbe\x6f\xf6\x98\x8e\x98\x8a\x88\x5a\x44\xfb\xae\x9f\x5f\x1d\x4a"
      "\xa7\x3b\xba\x93\xb7\x75\xd6\x03\x00\x00\x00\x80\xa3\xaa\x1f\xbd\xca\x4b"
      "\xbb\xb1\x1b\x9b\x71\xe1\x59\x0c\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x5e\x54\xf9\xfb\xff\x93\x7c\x2a\x15\xe9\x46\x24\xc5\xfb\xff\xab\xf9\xb6"
      "\xc8\xd3\xe7\xd0\xe8\x17\x21\x7e\x31\xd9\x5e\x3e\x79\xf6\x83\x01\x00\x00"
      "\x00\x00\x00\x00\x80\x67\xee\xf5\xdd\xd8\x8d\xcd\xb8\x50\xac\xef\x25\x51"
      "\x8a\xdf\x1c\x28\xf2\x8d\xf8\x20\xd6\x63\x39\xd6\xe2\x7a\x6c\x46\x33\x36"
      "\x62\x23\xd6\x62\x2e\x22\xa6\xbb\x4a\x55\x37\x9b\x1b\x1b\x6b\x73\xf1\x46"
      "\xb6\x76\x69\x48\xcd\x9b\xf1\x79\x9f\x9a\x37\x07\x8f\xf1\xd6\x89\xed\x2d"
      "\x00\x00\x00\x00\x00\x00\x00\x3c\x17\x6a\x23\xf2\xef\x4f\xf4\x6e\xfb\x5d"
      "\x2c\xed\xff\xfd\x1f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xce\x83"
      "\x24\xa2\xdc\x5e\x64\xd3\xa5\x22\x3d\x1d\xa5\x4a\x44\xd4\x22\xa2\x9a\x96"
      "\xdb\x8a\xf8\xbc\x48\x3f\x27\x92\x7e\x1b\x9f\x9c\xfe\x38\x00\x00\x00\xe0"
      "\xa9\xd4\x0e\xae\x26\xb5\x31\xea\xbc\xf4\x61\xec\xc6\x66\x5c\x28\xd6\xf7"
      "\x92\xec\x99\xff\x4a\xf6\xbc\x5c\x8b\x0f\x62\x35\x36\x62\x25\x36\xa2\x15"
      "\xcb\x71\x27\x7f\x86\x4e\x9f\xfa\x4b\x3b\xdb\xf3\xad\x9d\xed\xf9\x07\xe9"
      "\xd4\xdb\xee\x8f\xff\x7b\xa4\xa1\x67\x2d\x46\xfb\xb7\x87\xfe\x3d\xbf\x9a"
      "\x95\xa8\xc7\xdd\x58\xc9\xb6\x5c\x8f\xdb\x91\xc4\x5e\xa6\x94\xb7\xf2\xea"
      "\xce\xf6\x7c\xba\x7c\xd0\x7f\x5c\x1f\xa5\x63\x4a\x7e\x94\x1b\x32\x9a\x72"
      "\x57\xfa\x4e\x3a\xbb\xfa\x59\x96\xfe\xd3\xc1\x5f\x11\x2a\x47\xda\xc5\x63"
      "\x2a\x0d\xcc\x99\xce\x72\x27\x3a\x11\x99\xcd\xc7\x96\xd6\xb8\x58\x44\xa0"
      "\x7f\x24\x46\x1e\x9d\xca\xd0\x9e\xe6\xa2\xd4\xf9\xe5\xe7\xd2\xf0\x9e\xfa"
      "\xc7\xfc\xa3\xe1\xbd\x4f\x1d\x2a\xd5\xf7\x97\x9b\x33\x71\x38\x12\x37\xa3"
      "\xd4\x39\x42\x57\x86\x47\x22\xe2\xdb\x7f\xff\xf4\x97\xf7\x5a\xab\xf7\xef"
      "\xdd\x5d\x9f\x39\x3f\xbb\xd4\xd7\x87\x23\x4b\x1c\x8e\xc4\x7c\x57\x24\xae"
      "\xbe\x40\x91\x18\x6d\x36\x8b\xc4\xe5\xce\xfa\x52\xfc\x2c\x7e\x11\x33\x31"
      "\x99\xaf\xff\x2a\x9a\xb1\x11\xcb\x8d\x22\xbf\x99\x7f\x9e\xd3\xf9\xf4\xf0"
      "\x48\x7d\x31\xd5\xbd\xf6\xce\xa8\x91\xa4\xe7\x64\xa3\x73\xfd\xea\x37\xa6"
      "\x46\xbc\x13\x6b\xb1\x52\x8c\x29\x1a\xf1\xd3\x2c\xd5\x8c\x37\xb2\x63\x7a"
      "\x21\x56\x22\x89\x87\x11\xb1\x1c\x6f\x65\xff\x6e\xc6\x5c\xe7\x6a\xb0\x7f"
      "\x84\x2f\x8f\x71\xd6\x97\xc6\xb8\xd2\x76\xb9\xf6\x9d\x6c\xd1\x09\x53\xd4"
      "\x07\x97\xfd\xeb\x78\x4d\x9e\x94\x34\xae\x17\xbb\xe2\xda\x7d\xcd\x9d\xce"
      "\xf2\xba\xb7\xec\x47\xe9\xe5\xbe\x51\x2a\xee\x75\xe3\xdf\x8f\xba\x54\xbe"
      "\x95\x27\xd2\x16\x7e\x3f\xf4\xfe\x70\xda\x0e\x47\x62\xae\x2b\x12\xaf\x0c"
      "\xfa\xbc\xb4\x43\xfa\x97\xbd\x74\xbe\xde\x5a\xbd\xbf\x76\xaf\xf9\xfe\x98"
      "\xfd\xbd\x99\x2f\xd3\xf3\xe8\x8f\xc7\xbf\x4b\x4c\x1e\xaf\xda\x30\xe9\x11"
      "\x7e\x39\x6a\xf9\xce\x5d\xcc\xe6\x49\x76\x4e\xcd\x66\x79\xaf\x74\xee\xb0"
      "\x07\xe3\x55\xcd\xff\xe2\xd2\x56\xea\xc9\xbb\xdc\xa9\xd7\x3e\x53\x7f\x1e"
      "\x0f\xe3\xce\x81\x33\xf5\xfb\xb1\x10\x0b\xb1\x98\x95\xbe\x92\x95\x9e\xe8"
      "\xb9\x63\xa5\x79\x57\x3b\x2d\x1d\xbc\x86\xa7\x79\xe9\x37\xad\x4a\xe7\x0f"
      "\x3b\xdd\xdf\xb7\x1e\x46\xab\xfd\x7d\x08\x80\xf3\x6d\xea\xbb\x53\xd5\xfa"
      "\x7f\xea\xff\xaa\x7f\x52\xff\x43\xfd\x5e\xfd\xed\xda\x4f\x26\x7f\x30\xf9"
      "\x5a\x35\x26\xfe\x39\xf1\xc3\xca\x6c\xf9\xcd\xd2\x6b\xc9\xdf\xe2\x93\xf8"
      "\xed\xfe\xf3\x3f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x70\x7c\xeb\x8f\x1e\xdf\x6f\xb6\x5a"
      "\xcb\x6b\xfd\x13\xa5\xfe\x59\xc9\xf0\x5a\xcd\xd6\x5e\xf1\x22\xb1\x21\x65"
      "\x0e\x24\x92\xfc\x55\x39\x63\x14\x4e\xd6\x1f\x3d\xde\x1b\xd9\xe0\xf0\x44"
      "\xf1\xf2\x9f\x63\x56\x3f\xc9\x44\xf1\xb6\xc6\xd1\x85\x1b\x27\xdc\x7b\xb9"
      "\xeb\xe8\x24\x5b\x87\x8f\x57\x6d\xf4\xb1\x28\xde\xf2\x34\x46\x5f\x49\x4f"
      "\xc0\xd3\xca\xc7\x1e\x7c\xd1\xf3\xfe\x96\x89\x73\x70\x28\x0f\x27\x1a\x27"
      "\xd7\x60\xf1\x81\xed\xca\x1a\xeb\xd3\x5b\x8e\xae\x2d\xf5\x7e\xc7\xab\x1c"
      "\x11\xfd\xaa\x8f\xb8\x70\x94\x4f\xe2\xea\x03\x9c\xa5\x1b\x1b\x0f\xde\xbf"
      "\xb1\xfe\xe8\xf1\xf7\x56\x1e\x34\xdf\x5b\x7e\x6f\x79\x75\x62\x61\x61\x71"
      "\x76\x71\xe1\xad\xf9\x1b\x77\x57\x5a\xcb\xb3\xed\x79\x57\x85\x53\x79\xf9"
      "\x2d\x70\x1a\xba\xbf\x4e\x74\x54\x23\xe2\xf5\xd1\x75\x87\xbc\xa8\x15\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x78\x86\x4e\xe3\xff\x42\x9c\xf5\x3e\x02"
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
      "\x00\x00\x00\xcf\xb7\xa5\x99\xa8\x3c\x89\x24\xe6\x66\xaf\xcf\xa6\xeb\x3b"
      "\xdb\xf3\xad\x74\x2a\xd2\xfb\x25\x2b\x11\x51\x8a\x88\xe4\xd7\x11\xc9\x3f"
      "\x22\x6e\x45\x7b\x8a\xe9\xae\xe6\x92\x41\xfd\x7c\xbc\xb2\xf8\xee\x97\x5f"
      "\xef\x7c\x95\xa6\x27\xb3\xb6\x2a\x45\xf9\x52\xc4\xd6\xc0\x7a\xe3\xd9\xca"
      "\xa7\x68\x44\x44\x39\x5f\x9e\x54\x7b\xb7\x47\xb7\x57\xdd\x4f\x4e\xf6\xc9"
      "\x4e\x3a\x91\x49\x03\x76\xad\x08\x1c\x9c\xb5\xff\x05\x00\x00\xff\xff\x75"
      "\x20\xe9\x39",
      1767);
  syz_mount_image(/*fs=*/0x20000000, /*dir=*/0x20000080,
                  /*flags=MS_I_VERSION|MS_NOEXEC|MS_NODEV*/ 0x80000c,
                  /*opts=*/0x20000a80, /*chdir=*/1, /*size=*/0x6e7,
                  /*img=*/0x20000bc0);
  memcpy((void*)0x20000100, "./file0\000", 8);
  syscall(__NR_listxattr, /*path=*/0x20000100ul, /*list=*/0ul, /*size=*/0ul);
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
