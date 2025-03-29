// https://syzkaller.appspot.com/bug?id=0e77b741e3a1b835114f0f7f0f3e53e12c9a73bc
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
  memcpy((void*)0x200000000080, "hfsplus\000", 8);
  memcpy((void*)0x200000000140, "./file1\000", 8);
  memcpy(
      (void*)0x200000000b00,
      "\x78\x9c\xec\xdd\xcd\x6f\x1c\x67\x1d\x07\xf0\xef\x6c\x36\xb6\x37\x94\xd4"
      "\x4d\x93\x36\x45\x95\x62\x35\x12\x20\x2c\x12\xbf\xc8\x05\x73\x21\x20\x84"
      "\x7c\xa8\x50\x55\x0e\x9c\xad\xc4\x69\xac\x6c\xd2\x62\xbb\xc8\xad\x10\x0d"
      "\xef\xd7\x1e\xf2\x07\x94\x83\x6f\x3d\x21\x71\x8f\x54\x2e\x5c\xe0\xd6\xab"
      "\x8f\x95\x10\x5c\x7a\xc1\x9c\x16\xcd\xec\xac\xbd\xf5\xbb\xd3\xc4\x6b\x97"
      "\xcf\x27\x7a\xf6\x79\x9e\x79\x66\x9e\x97\xdf\xce\xec\xec\xae\x15\x6d\x80"
      "\xff\x5b\x73\xe3\x69\x3e\x4a\x91\xb9\xf1\xd7\x56\xcb\xfa\xfa\xda\x74\x7b"
      "\x7d\x6d\xfa\x5e\xaf\x9c\x64\x38\x49\x23\x69\x76\xb3\x14\xff\xe9\x74\x3a"
      "\x1f\x27\x37\xd2\x4d\x79\xa9\xdc\x58\x77\x57\xec\x35\xce\xc3\xc5\xd9\x37"
      "\x3e\xf9\x6c\xfd\xd3\x6e\xad\x59\xa7\x6a\xff\xc6\x7e\xc7\x1d\xce\x83\x3a"
      "\x65\x2c\xc9\x99\x3a\x7f\x52\xfd\xdd\x3c\xa8\xbf\x91\x83\xba\x2b\x36\x57"
      "\x58\x06\xec\x6a\x2f\x70\x30\x68\x67\x93\x74\x2a\xff\x7a\xd8\xdd\xf2\xf3"
      "\xbf\x3d\xb3\xd9\xd2\xa7\xb5\xdb\xd1\x07\x9e\xf9\xc0\x29\x50\x74\xef\x9b"
      "\x3b\x8c\x26\xe7\xea\x0b\xbd\x7c\x1f\xd0\xbd\x2b\x76\xef\xd9\x00\x00\x00"
      "\xc0\x09\xf7\xec\x46\x36\xb2\x9a\xf3\x83\x9e\x07\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x9c\x26\xf5\xef\xff\x17\x75\x6a\xf4\xca\x63\x29\x7a\xbf\xff"
      "\x3f\x54\x6f\x4b\x5d\x3e\x59\xae\x1c\x6d\xf7\x47\x4f\x6b\x1e\x00\x00\x00"
      "\x00\x00\x00\x00\x70\x8c\xae\x6c\x64\x23\xab\x39\xdf\xab\x77\x8a\xea\x6f"
      "\xfe\xaf\x54\x95\x8b\xd5\xe3\x57\xf2\x4e\x96\xb3\x90\xa5\x5c\xcb\x6a\xe6"
      "\xb3\x92\x95\x2c\x65\x32\xc9\x68\x5f\x47\x43\xab\xf3\x2b\x2b\x4b\x93\x87"
      "\x38\x72\x6a\xd7\x23\xa7\x0e\x98\xe8\x70\x9d\xb7\x9e\xcc\xba\x01\x00\x00"
      "\x00\x00\x00\x00\xe0\x4b\xe6\x37\x99\xdb\xfa\xfb\x3f\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x9c\x04\x45\x72\xa6\x9b\x55\xe9\x62\xaf\x3c\x9a"
      "\x46\x33\xc9\x48\x92\xa1\x72\xbf\x07\xc9\x3f\x7a\xe5\xd3\xec\xd1\xa0\x27"
      "\x00\x00\x00\x00\xc7\xe0\xd9\x8d\x6c\x64\x35\xe7\x7b\xf5\x4e\x51\x7d\xe6"
      "\x7f\xa1\xfa\xdc\x3f\x92\x77\x72\x3f\x2b\x59\xcc\x4a\xda\x59\xc8\xad\xea"
      "\xbb\x80\xee\xa7\xfe\xc6\xfa\xda\x74\x7b\x7d\x6d\xfa\x5e\x99\x76\xf6\xfb"
      "\x83\x7f\x1f\x69\x1a\x55\x8f\xe9\x7e\xf7\xb0\xfb\xc8\x97\xab\x3d\x5a\xb9"
      "\x9d\xc5\x6a\xcb\xb5\xdc\xcc\x5b\x69\xe7\x56\x1a\xd5\x91\xa5\xcb\xbd\xf9"
      "\xec\x3e\xaf\x5f\x97\x73\x2a\xbe\x5f\x3b\xe4\xcc\x6e\xd5\x79\xb9\xf2\x0f"
      "\xea\x7c\x87\xf7\x8f\xb4\xd8\xbd\x1c\xf1\xcb\x94\xd1\x2a\x22\x67\x37\x23"
      "\x32\x51\xcf\xad\x8c\xc6\x73\xfb\x47\xe2\x88\xcf\xce\xf6\x91\x26\xd3\xd8"
      "\x9c\xec\xc5\x6d\x23\x6d\x5b\xc4\x63\xc5\xfc\x5c\x9d\x97\xeb\xf9\xc3\x5e"
      "\x31\x1f\x88\xed\x91\x98\xea\x3b\xfb\x5e\xd8\x3f\xe6\xc9\x37\xfe\xf2\xd1"
      "\xcf\xee\xb4\xef\xdf\xbd\x73\x7b\x79\xfc\xe4\x2c\xe9\x70\xce\xd4\x79\xa7"
      "\x7a\x6c\xed\x8c\xc4\x74\x5f\x24\x5e\xfc\x32\x47\x62\x87\x89\x2a\x12\x97"
      "\x36\xeb\x73\xf9\x71\x7e\x9a\xf1\x8c\xe5\xf5\x2c\x65\x31\xbf\xc8\x7c\x56"
      "\xb2\x90\xb1\xfc\xa8\x2a\xcd\xd7\xe7\x73\xd1\x77\xc9\xef\x11\xa9\x1b\x9f"
      "\xab\xbd\x7e\xd0\x4c\x86\xea\x33\xb4\xfb\x64\x1d\x6d\x4e\xaf\x54\xc7\x9e"
      "\xcf\x62\x7e\x92\xb7\x72\x2b\x0b\x79\xb5\xfa\x37\x95\xc9\x7c\x27\x33\x99"
      "\xc9\x6c\xdf\x33\x7c\xe9\x10\xaf\xb4\x8d\x3d\xae\xfa\xce\x57\x77\x9d\xfc"
      "\xd5\x6f\xd6\x85\x56\x92\x3f\xd6\xf9\xc9\x50\xc6\xf5\xb9\xbe\xb8\xf6\xbf"
      "\xe6\x8e\x56\x6d\xfd\x5b\xb6\xa2\x74\xe1\xc9\xdf\x8f\x9a\x5f\xab\x0b\xe5"
      "\x18\xbf\xad\xf3\x93\x61\x7b\x24\x26\xfb\x22\xf1\xfc\xfe\x91\xf8\x53\xf5"
      "\xb2\xb2\xdc\xbe\x7f\x77\xe9\xce\xfc\xdb\x87\x1b\xee\xc2\x07\x75\xa1\xbc"
      "\x8e\x7e\xbf\xdf\x5d\xa2\x68\x3e\xe6\x8a\x1e\x57\x79\xbe\x5c\x28\x9f\xac"
      "\xaa\xf6\xf9\xb3\xa3\x6c\x7b\x7e\xd7\xb6\xc9\xaa\xed\xe2\x66\x5b\x63\x47"
      "\xdb\xa5\xcd\xb6\xee\x95\xfa\x60\xcf\x2b\x75\xa8\x7e\x0f\xb7\xb3\xa7\xa9"
      "\xaa\xed\xc5\x5d\xdb\xa6\xab\xb6\xcb\x7d\x6d\xbb\xbd\xdf\x02\xe0\xc4\x3b"
      "\xf7\xad\x73\x43\xad\x7f\xb6\xfe\xde\xfa\xb0\xf5\xbb\xd6\x9d\xd6\x6b\x23"
      "\x3f\x1c\xfe\xee\xf0\xcb\x43\x39\xfb\xd7\xb3\xdf\x6b\x4e\x9c\xf9\x7a\xe3"
      "\xe5\xe2\xcf\xf9\x30\xbf\xda\xfa\xfc\x0f\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x3c\xbe\xe5"
      "\x77\xdf\xbb\x3b\xdf\x6e\x2f\x2c\x6d\x2b\x74\x3a\x9d\xf7\xf7\x68\x3a\xcd"
      "\x85\xde\xcf\x99\x1d\xe3\xa0\x2f\x3d\x93\x0c\x6a\xc9\x43\x49\x4e\x46\xe4"
      "\xff\xdb\xe9\x74\xea\x2d\xc5\x49\x98\xcf\xfe\x85\x4e\x69\x38\x9d\xa7\x3e"
      "\x56\x33\xc9\x6e\x4d\x57\x06\x1f\x84\x01\xbf\x30\x01\x4f\xdd\xf5\x95\x7b"
      "\x6f\x5f\x5f\x7e\xf7\xbd\x6f\x2f\xde\x9b\x7f\x73\xe1\xcd\x85\xfb\xb3\x33"
      "\x33\xb3\x13\xb3\x33\xaf\x4e\x5f\xbf\xbd\xd8\x5e\x98\xe8\x3e\x0e\x7a\x96"
      "\xc0\xd3\xb0\x75\xd3\x1f\xf4\x4c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80"
      "\xc3\x3a\x8e\xff\x4e\xb0\xf7\xe8\x23\xc7\xb9\x54\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xe0\x94\x9a"
      "\x1b\x4f\xf3\x51\x8a\x4c\x4e\x5c\x9b\x28\xeb\xeb\x6b\xd3\xed\x32\xf5\xca"
      "\x5b\x7b\x36\x93\x34\x92\x14\xbf\x4c\x8a\x8f\x93\x1b\xe9\xa6\x8c\xf6\x75"
      "\x57\xec\x35\xce\xc3\xc5\xd9\x37\x3e\xf9\x6c\xfd\xd3\xad\xbe\x9a\xbd\xfd"
      "\x1b\xfb\x1d\x77\x38\x0f\xea\x94\xb1\x24\x67\xea\xfc\x0b\x38\xfb\x51\x5f"
      "\x7f\x37\xbf\x70\x7f\xc5\xe6\x0a\xcb\x80\x5d\xed\x05\x0e\x06\xed\x7f\x01"
      "\x00\x00\xff\xff\x41\xe0\x06\x26",
      1610);
  syz_mount_image(/*fs=*/0x200000000080, /*dir=*/0x200000000140,
                  /*flags=MS_STRICTATIME|MS_SILENT|MS_NOATIME*/ 0x1008400,
                  /*opts=*/0x200000000a40, /*chdir=*/1, /*size=*/0x64a,
                  /*img=*/0x200000000b00);
  memcpy((void*)0x2000000001c0, "cgroup.controllers\000", 19);
  res = syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x2000000001c0ul,
                /*flags=*/0x275a, /*mode=*/0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x200000000040, "#! ", 3);
  *(uint8_t*)0x200000000043 = 0xa;
  syscall(__NR_write, /*fd=*/r[0], /*data=*/0x200000000040ul,
          /*len=*/0x208e24bul);
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffffffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200000000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200001000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  const char* reason;
  (void)reason;
  loop();
  return 0;
}
