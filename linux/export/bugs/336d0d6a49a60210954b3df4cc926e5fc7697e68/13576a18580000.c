// https://syzkaller.appspot.com/bug?id=336d0d6a49a60210954b3df4cc926e5fc7697e68
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
  memcpy((void*)0x20000000, "hfsplus\000", 8);
  memcpy((void*)0x200000c0, "./file1\000", 8);
  memcpy(
      (void*)0x20001440,
      "\x78\x9c\xec\xdd\xcd\x6f\x1c\x67\x1d\x07\xf0\xef\x6c\xd6\x2f\x1b\x4a\xea"
      "\xa6\x69\x9b\xa2\x4a\xb5\x1a\x09\x10\x11\x89\x1d\x2b\x05\x73\x69\x40\x08"
      "\xe5\x50\xa1\xaa\x1c\x38\x5b\x89\x93\x58\xd9\xa4\xc1\x71\x91\x5b\x21\x6a"
      "\xde\xaf\x95\xc8\x1f\x50\x0e\xbe\x71\x40\x48\xdc\x38\x44\x94\x0b\x17\xb8"
      "\xf5\xea\x63\x25\x04\x97\x5e\x30\xa7\x45\x33\x3b\xde\x6c\x6c\xef\xda\x49"
      "\x88\xd7\x49\x3f\x9f\x6a\xf6\x79\x66\x9e\x79\x9e\xf9\x3d\xbf\x9d\x99\x7d"
      "\x6b\xe4\x00\x9f\x5b\x17\x4f\xa7\x79\x37\x45\x2e\x9e\x7e\x73\xb5\x5c\xdf"
      "\x58\x9f\x6b\x6f\xac\xcf\xdd\xd8\xaa\x27\x99\x48\xd2\x48\x9a\xdd\x22\xc5"
      "\x7f\x3a\x9d\xce\xc7\xc9\x85\x74\x97\xbc\x5c\x6e\xac\x87\x2b\x06\x1d\xe7"
      "\xce\xd2\xfc\xdb\x9f\x7c\xb6\xf1\x69\x77\xad\x59\x2f\xd5\xfe\x8d\x61\xfd"
      "\xf6\x67\xad\x5e\x32\x9d\xe4\x48\x5d\x3e\x9c\xdf\xee\x18\xef\xd2\x5e\xe3"
      "\x4d\xee\x35\x66\xd1\x9b\x61\x99\xb0\x53\x5b\x89\x83\x51\x1b\x4b\xd2\xa9"
      "\xfc\xeb\x4e\x77\xcb\x8f\xfe\xf6\x4c\xaf\xa5\x4f\x6b\xb7\xde\x7b\x9e\xf9"
      "\xc0\x13\xa0\xe8\xbe\x6e\xee\x30\x95\x1c\xed\xbf\x15\xac\x75\x8b\xc6\x41"
      "\xc5\xf5\x28\x86\xdd\x9d\xd6\x0e\x30\x0e\x00\x00\x00\x18\x95\x67\x37\xb3"
      "\x99\xd5\x1c\x1b\x75\x1c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0\x24\xa9"
      "\xff\xfe\x7f\x51\x2f\x8d\xad\xfa\x74\x8a\xa3\xf5\x9f\xd2\x1f\xaf\xb7\xa5"
      "\xae\x1f\x2e\xaf\x3e\xd8\xee\x77\x1f\x57\x1c\x00\x00\x00\x00\x00\x00\x00"
      "\x70\x80\x5e\xdd\xcc\x66\x56\x73\x2c\xc9\x5a\x92\xb1\x7a\xf3\x6b\xd5\xe3"
      "\x89\xea\xf1\x0b\x79\x37\xb7\xb3\x98\xe5\x9c\xc9\x6a\x16\xb2\x92\x95\x2c"
      "\x67\x36\xc9\x54\xdf\x40\xe3\xab\x0b\x2b\x2b\xcb\xb3\xbd\x9e\x5b\xff\x67"
      "\xc0\xce\x9e\xe7\x76\xed\x79\x6e\x8f\x40\x27\xea\xb2\xf5\x7f\x99\x36\x00"
      "\x00\x00\x00\x00\x00\x00\x3c\x6d\x7e\x9e\x8b\xd5\xef\xff\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x70\x68\x14\xc9\x91\x6e\x51\x2d\x27\xb6\xea"
      "\x53\x69\x34\x93\x4c\x26\x19\x2f\xf7\x5b\x4b\xfe\xb1\x55\x7f\x92\xdd\x1d"
      "\x75\x00\x00\x00\x00\xf0\x08\x3a\xfb\xdc\xef\xd9\xcd\x6c\x66\x35\xc7\x7a"
      "\xfd\x8a\xea\x33\xff\x8b\xd5\xe7\xfe\xc9\xbc\x9b\x9b\x59\xc9\x52\x56\xd2"
      "\xce\x62\x2e\x57\xdf\x05\x74\x3f\xf5\x37\x36\xd6\xe7\xda\x1b\xeb\x73\x37"
      "\xca\x65\xe7\xb8\xdf\xfe\xf7\x03\x85\x5b\x8d\x98\xee\x77\x0f\xbb\x1f\xf9"
      "\x64\xb5\x47\x2b\x57\xb2\x54\x6d\x39\x93\x4b\x79\x27\x45\x71\x39\x8d\xaa"
      "\x67\xe9\xe4\x56\x3c\xbb\xc7\xf5\xb3\x32\xa6\xe2\x8d\x89\x37\x4a\x63\xc3"
      "\xc2\xe9\xcb\xde\xe5\xba\x2c\x67\xfe\x61\x5d\xee\xf0\xc1\x03\x4d\x76\x90"
      "\x07\xfc\x32\x65\xaa\xca\xc8\x58\x2f\x23\x33\x75\x6c\x65\x36\x9e\x1b\x9e"
      "\x89\x07\x7c\x76\xb6\x1f\x69\x36\x8d\x5e\xb0\x27\xb6\x1d\x69\xdb\x24\xea"
      "\x9c\xd7\xf6\x79\xbc\xa3\x75\x59\xce\xe7\xd7\x83\x72\x3e\x12\xdb\x33\x71"
      "\xae\xef\xec\x7b\x71\x78\xce\x93\xaf\xfc\xe9\xf7\x3f\xbc\xd6\xbe\x79\xfd"
      "\xda\x95\xdb\xa7\x0f\xcf\x94\xf6\x67\xeb\x4b\xc1\xee\x95\xd1\xda\x99\x89"
      "\xb9\xbe\x4c\xbc\xf4\x34\x67\x62\x87\x99\x2a\x13\x2f\xf4\xd6\x2f\xe6\x7b"
      "\xf9\x41\x4e\x67\x3a\x6f\x65\x39\x4b\xf9\x71\x16\xb2\x92\xc5\x4c\xe7\xbb"
      "\x55\x6d\xa1\x3e\x9f\x8b\xbe\x4b\x7e\x40\xa6\x2e\xdc\xb7\xf6\xd6\x5e\x91"
      "\x8c\xd7\x67\x68\xf7\x2e\x7a\x7f\x4c\xd9\x23\xa6\xd7\xaa\xbe\xc7\xb2\x94"
      "\xef\xe7\x9d\x5c\xce\x62\x5e\xaf\xfe\x3b\x97\xd9\x7c\x23\xe7\x73\x3e\xf3"
      "\xbd\x67\x78\xfa\xcf\xc9\xde\x77\xda\xc6\x80\xab\xbe\xf3\xc5\x5d\x83\x3f"
      "\xf5\xd5\xba\xd2\x4a\xf2\x9b\xba\xac\x5c\x6d\xee\x35\xf1\xc7\xac\xcc\xeb"
      "\x73\x7d\x79\xed\xbf\xe7\x4e\x55\x6d\xfd\x5b\xee\x5d\x07\xc7\xf7\xf5\x7a"
      "\x74\x7f\x96\xfe\x30\x3c\x94\xe6\x97\xea\x4a\x79\x8c\x5f\xd4\xe5\xe1\xb0"
      "\x3d\x13\xb3\x7d\x99\x78\x7e\x78\x26\x7e\x57\xdd\x56\x6e\xb7\x6f\x5e\x5f"
      "\xbe\xb6\x70\x6b\x7f\x87\x3b\xfe\x61\x5d\x29\xaf\xa3\x5f\x3d\xfc\xab\xc4"
      "\xb1\x87\xeb\x36\x4c\x79\xbe\x1c\x2f\x9f\xac\x6a\xed\xfe\xb3\xa3\x6c\x7b"
      "\xbe\x6e\x1b\xab\x96\x7b\xf9\x1a\xaf\x7f\x71\xe9\xf6\x6b\xec\x68\x7b\xa1"
      "\xd7\xd6\xbd\x52\xd7\x06\x5e\xa9\xe3\xf5\x7b\xb8\x9d\x23\x9d\xab\xda\x5e"
      "\xda\xb5\x6d\xae\x6a\x3b\xd9\xd7\xb6\xfd\xfd\x56\xbb\xf7\x7e\xe8\x69\xf8"
      "\xf1\x07\xe0\xa9\x75\xf4\x6b\x47\xc7\x5b\xff\x6c\xfd\xbd\xf5\x51\xeb\x97"
      "\xad\x6b\xad\x37\x27\xbf\x33\xf1\xcd\x89\x57\xc6\x33\xf6\xd7\xb1\x6f\x35"
      "\x67\x8e\x7c\xb9\xf1\x4a\xf1\xc7\x7c\x94\x9f\x3e\x8e\x17\x41\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\xf8\xfc\xb9\xfd\xde\xfb\xd7\x17\xda\xed\xc5\xe5\xbe\x4a\xe3\xfa"
      "\x42\xbb\xd3\xe9\x7c\xb0\x4b\xd3\xe0\x4a\x6b\xc8\x80\x87\xa7\x72\xa4\x8e"
      "\xf2\x00\x0f\xfa\xf2\x33\xc9\xa8\xa6\x3c\x9e\x64\x40\xd3\x64\x0e\x34\x9e"
      "\xff\x76\x3a\x9d\x7a\x4b\x71\x28\xce\x84\xa1\x95\x4e\x69\x22\x9d\x87\xec"
      "\xfe\x97\x24\xfb\xdb\xb9\x99\x64\x67\xd3\x66\xa7\xd3\x19\x79\x12\x46\x78"
      "\x53\x02\x0e\xc4\xd9\x95\x1b\xb7\xce\xde\x7e\xef\xfd\xaf\x2f\xdd\x58\xb8"
      "\xba\x78\x75\xf1\xe6\xfc\xf9\xf3\xf3\x33\xf3\xe7\x5f\x9f\x3b\x7b\x65\xa9"
      "\xbd\x38\xd3\x7d\x1c\x75\x94\xc0\xe3\x70\xef\x45\x7f\xd4\x91\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\xfb\x75\x10\xff\x9c\x60\xf0\xd1\x27\x0f\x72"
      "\xaa\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\xc0\x13\xea\xe2\xe9\x34\xef\xa6\xc8\xec\xcc\x99\x99\x72"
      "\x7d\x63\xfd\x56\x36\xd6\xe7\xda\xdd\x7a\xb7\xec\x6a\x26\x69\x24\x29\x7e"
      "\x92\x14\x1f\x27\x17\xd2\x5d\x32\xd5\x37\x5c\x31\xe8\x38\x77\x96\xe6\xdf"
      "\xfe\xe4\xb3\x8d\x4f\xef\x8d\xd5\xdc\xda\xbf\x31\xac\xdf\xae\x1a\xdb\x37"
      "\xac\xd5\x4b\xa6\x93\x1c\xa9\xcb\x47\x70\xdf\x78\x97\x1e\x79\xbc\xa2\x37"
      "\xc3\x32\x61\xa7\xb6\x12\x07\xa3\xf6\xbf\x00\x00\x00\xff\xff\x51\x11\x04"
      "\x73",
      1693);
  syz_mount_image(/*fs=*/0x20000000, /*dir=*/0x200000c0,
                  /*flags=MS_STRICTATIME|MS_SILENT|MS_NOATIME*/ 0x1008400,
                  /*opts=*/0x20000180, /*chdir=*/0x86, /*size=*/0x69d,
                  /*img=*/0x20001440);
  memcpy((void*)0x20000100, "./file1\000", 8);
  res = syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000100ul,
                /*flags=O_SYNC|O_CREAT|FASYNC|O_RDWR*/ 0x103042, /*mode=*/0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x200002c0 = 0x200011c0;
  memset((void*)0x200011c0, 149, 1);
  *(uint64_t*)0x200002c8 = 1;
  syscall(__NR_pwritev2, /*fd=*/r[0], /*vec=*/0x200002c0ul, /*vlen=*/1ul,
          /*off_low=*/0x47fff, /*off_high=*/0, /*flags=*/0ul);
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
