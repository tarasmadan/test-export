// https://syzkaller.appspot.com/bug?id=c3b019c2a6c68f21ef5acc451bd4de534675564d
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

static void use_temporary_dir(void)
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    exit(1);
  if (chmod(tmpdir, 0777))
    exit(1);
  if (chdir(tmpdir))
    exit(1);
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
  return munmap(dest, destlen);
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

static long syz_mount_image(volatile long fsarg, volatile long dir,
                            volatile long flags, volatile long optsarg,
                            volatile long change_dir,
                            volatile unsigned long size, volatile long image)
{
  unsigned char* data = (unsigned char*)image;
  int res = -1, err = 0, loopfd = -1, need_loop_device = !!size;
  char* mount_opts = (char*)optsarg;
  char* target = (char*)dir;
  char* fs = (char*)fsarg;
  char* source = NULL;
  char loopname[64];
  if (need_loop_device) {
    memset(loopname, 0, sizeof(loopname));
    snprintf(loopname, sizeof(loopname), "/dev/loop%llu", procid);
    if (setup_loop_device(data, size, loopname, &loopfd) == -1)
      return -1;
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
  if (need_loop_device) {
    ioctl(loopfd, LOOP_CLR_FD, 0);
    close(loopfd);
  }
  errno = err;
  return res;
}

#define FS_IOC_SETFLAGS _IOW('f', 2, long)
static void remove_dir(const char* dir)
{
  int iter = 0;
  DIR* dp = 0;
retry:
  while (umount2(dir, MNT_DETACH | UMOUNT_NOFOLLOW) == 0) {
  }
  dp = opendir(dir);
  if (dp == NULL) {
    if (errno == EMFILE) {
      exit(1);
    }
    exit(1);
  }
  struct dirent* ep = 0;
  while ((ep = readdir(dp))) {
    if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
      continue;
    char filename[FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", dir, ep->d_name);
    while (umount2(filename, MNT_DETACH | UMOUNT_NOFOLLOW) == 0) {
    }
    struct stat st;
    if (lstat(filename, &st))
      exit(1);
    if (S_ISDIR(st.st_mode)) {
      remove_dir(filename);
      continue;
    }
    int i;
    for (i = 0;; i++) {
      if (unlink(filename) == 0)
        break;
      if (errno == EPERM) {
        int fd = open(filename, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0) {
          }
          close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno != EBUSY || i > 100)
        exit(1);
      if (umount2(filename, MNT_DETACH | UMOUNT_NOFOLLOW))
        exit(1);
    }
  }
  closedir(dp);
  for (int i = 0;; i++) {
    if (rmdir(dir) == 0)
      break;
    if (i < 100) {
      if (errno == EPERM) {
        int fd = open(dir, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0) {
          }
          close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno == EBUSY) {
        if (umount2(dir, MNT_DETACH | UMOUNT_NOFOLLOW))
          exit(1);
        continue;
      }
      if (errno == ENOTEMPTY) {
        if (iter < 100) {
          iter++;
          goto retry;
        }
      }
    }
    exit(1);
  }
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
  if (symlink("/dev/binderfs", "./binderfs")) {
  }
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter = 0;
  for (;; iter++) {
    char cwdbuf[32];
    sprintf(cwdbuf, "./%d", iter);
    if (mkdir(cwdbuf, 0777))
      exit(1);
    reset_loop();
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      if (chdir(cwdbuf))
        exit(1);
      setup_test();
      execute_one();
      exit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      sleep_ms(1);
      if (current_time_ms() - start < 5000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
    remove_dir(cwdbuf);
  }
}

void execute_one(void)
{
  memcpy((void*)0x200000c0, "hfs\000", 4);
  memcpy((void*)0x20000100, "./bus\000", 6);
  *(uint64_t*)0x20000640 = 0;
  sprintf((char*)0x20000648, "0x%016llx", (long long)-1);
  *(uint32_t*)0x2000065a = -1;
  memcpy(
      (void*)0x2000065e,
      "\xa2\x4e\xd3\x17\xcb\x4e\x0a\xd8\x44\x54\x2b\x7e\x77\xc8\xd2\x6d\x39\x38"
      "\x0e\x1e\x59\xe6\x93\x54\xae\x34\xcb\x24\xb8\xd4\xcb\x6e\xf4\xbb\xa4\x1c"
      "\xb1\x39\xa6\x72\x93\x4c\x16\x4b\x8a\x72\xf1\xc0\xc5\x59\x2a\x76\xd7\x8c"
      "\x96\x55\xaa\xa2\x56\xcd\xd0\xf4\x9e\x4d\x7d\x9b\x0c\x7d\xc9\x8f\x15\x6f"
      "\x96\xf0\xfb\x7f\x53\x89\xec\x78\x58\x01\xbc\x23\x20\xc6\xa6\x2c\x24\x82"
      "\x78\xe1\xaa\x06\x5a\x90\x5c\x2c\x19\x67\x97\xdf\x61\xd2\x50\x2c\x2d\x34"
      "\xa7\xbb\x1c\x00\x3b\xe2\x93\x83\xaa\x04\x3a\xd2\xc2\x7c\x38\x5a\x78\xfe"
      "\x0b\x62\x79\x2f\x72\xa4\xc1\x3e\x7b\xbd\x94\x05\xb2\xb9\xf0\x19\xa2\x87"
      "\x1a\x45\x43\x92\x39\xfa\x99\x93\x6a\x8c\x09\x73\x52\xd9\x6d\xe2\x88\xe1"
      "\x56\xae\x16\xba\xde\x83\xbd\x9d\xa8\xd2\x4c\x2f\x4a\xcb\xbc\x60\x11\xf4"
      "\xde\x81\x96\x29\xd7\x27\x49\x69\xb4\xae\x2e\xf8\xc6\x2f\x47\x51\xbe\x89"
      "\x4c\x83\x65\xc2\x36\xd7\x84\x9e\x00\x10\xcc\xcb\x86\xfe\x81\xbb\x12\x1b"
      "\xb4\x85\x81\xe9\x95\x6b\x0d\x21\xe5\x00\x5c\xc2\x0f\x43\x1a\xd3\x42\x4b"
      "\x1d\xfa\xfb\xdd\x25\x93\x39\xd4\x75\xc4\x21\xde\xa7\xb7\x48\xaf\xf3\x95"
      "\xd1\x71\xac\x01\x01\x93\x76\xc0\xab\x53\x3a\xed\xa5\x89\x51\x0c\xe6\x46"
      "\x46\x94\x14\x69\x8f\xcb\x6f\xa0\x38\xb3\xf6\xc7\x25\x00\xdd\x7d\x48\x63"
      "\x77\xcc\xe2\x22\x17\x66\x5b\x7e\x06\x5a\xff\x16\xaf\x36\x7d\x2e\xbf\x80"
      "\xf7\x2c\x7a\x94\xcb\xfb\x29\xc1\x33\xe5\xa2\xb5\xac\xca\xed\x04\xde\xce"
      "\xd8\xce\x5e\x78\x98\x07\xb8\x57\x03\x4d\xe9\xb1\xbf\x29\xc3\x06\x8f\x1e"
      "\x09\x89\x83\xa7\x81\x2d",
      348);
  sprintf((char*)0x200007ba, "%023llo", (long long)-1);
  sprintf((char*)0x200007d1, "0x%016llx", (long long)-1);
  sprintf((char*)0x200007e3, "%023llo", (long long)-1);
  sprintf((char*)0x200007fa, "%023llo", (long long)-1);
  memcpy(
      (void*)0x20000811,
      "\xb5\x48\x6c\xd9\x08\xd9\x47\xa6\xa9\xe5\x39\xac\xf6\xc5\x3a\x49\xe4\xd1"
      "\xe1\x34\x87\xd1\x08\x61\x13\x33\x40\xd3\xf2\xb2\x8f\x8d\x7b\x7a\x3e\x55"
      "\xf1\xd5\x78\x0f\x06\xd2\xa1\xa1\x44\xbf\x68\xbf\xa1\x56\xc3\xad\xac\x7a"
      "\x2f\x6a\x8e\xb3\x77\x00\xe6\x0c\xdd\xca\x0b\xc3\x6e\x08\x97\x1c\x11\x9c"
      "\xe0\x8d\x8b\x78\xaa\x29\x58\xdf\x16\xec\x04\x9b\xb7\x19\xfb\x02\xb0\xcf"
      "\xef\x09\x98\x24\x14\xaa\xe0\xae\x91\xab\xcd\x80\x87\x0e\x51\x8f\xff\xc0"
      "\xf3\x53\xc6\x9b\x67\x67\x1e\x2c\x7b\x2a\xf2\x31\x4c\x32\x21\x13\xa8\x43"
      "\x28\x94\xc8\xba\xce\x66\x66\x07\x8a\xe1\x66\xf5\xfd\xee\xda\x09\x06\x22"
      "\x83\x0a\xa2\xf6\x2e\xfe\x40\x5f\xdc\xe2\x53\x13\x00\x9e\x15\x1a\x25\x17"
      "\xa1\xd5\x37\x68\x96\x97\x53\xfc\x3e\x23\x6c\x49\x31\x49\xcf\x0c\x92\x0b"
      "\x71\xec\xbd\xb3\x40\x4e\xb5\xd4\xbe\xc7\x45\x2f\x15\x5a\x23\xdf\xbc\x20"
      "\x8c\xbd\x14\x2e\xef\x1f\xb5\x42\x45\x3b\xe7\xe6\x7f\x17\xa0\xcc\xb2\x18"
      "\xca\x68\xe8\x98\xed\xed\x7a\x4d\xdd\x78\xf0\xba\x73\xc3\xc9\xec\x11\x39"
      "\x93\x3a\x28\x7f\x01\x27\x8d\x2a\x76\x48\xd3\x46\x83\x5b\x6f\xb4\xec\xd4"
      "\x0c\xf7\x80\x8a\xfc\x0d\xc7\x61\xbb\x43\x19\x03\x71\x0b\x2d\xc9\x70\x7a"
      "\xde\xc9\x12\x1e\x4e\xaf\x9d\x8a\x6f\x41\x40\x45\x09\xb1\x10\xe9\x4a\x6d"
      "\xe9\x7d\xb5\xc7\x26\xcf\x62\xb3\x79\xc8\xb2\xa6\x2c\x81\x58\xca\xc2\x16"
      "\xbe\xc8\xe1\x9b\x47\xb5\x96\x67\xbd\x90\x4f\xdc\x95\x4a\xdb\xa8\x5c\x64"
      "\x26\xe4\x1a\x79\xa0\xce\x63\x6c\xb8\x3b\xa6\x09\x6d\x49\xea\xb4\x6a\x4e"
      "\xf4\x04\xb3\xf1\x1c\x2b\x35\xd3\x56\xf5\xf1\xf2\x50\x56\x7a\xab\x8b\x05"
      "\xfd\x30\x19\x59\xba\x42\x1e\xf1\x2e\xe1\x94\x85\x93\x14\x4c\x2f\x09\x3d"
      "\x86\x41\x66\x34\x8d\x25\x35\x22\x7b\xb0\xfe\xdf\xa6\xd3\x77\x39\x1f\x2f"
      "\xf8\x62\x2d\x9b\x81\xf0\xbc\x2b\x34\xac\x8d\x79\x0f\xd8\x71\x86\x7d\x33"
      "\x70\x63\xa4\x16\x60\x4f\x38\x65\x90\x10\xa1\x98\x2e\xef\x99\x30\x84\xb4"
      "\xa7\x59\xaf\x60\x9d\xba\xec\x25\xa1\x20\x21\x61\x28\x9d\x0b\x98\xc5\x65"
      "\x52\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
      466);
  sprintf((char*)0x200009e3, "0x%016llx", (long long)-1);
  memcpy(
      (void*)0x200003c0,
      "\x78\x9c\xec\xdd\xc1\x6a\x13\x41\x1c\xc7\xf1\xdf\x6c\x62\x9b\x6a\xa9\xab"
      "\xad\x08\xe2\xa9\x5a\xf0\x24\x6d\xbd\x88\x17\x45\xf2\x10\x9e\x44\x6d\x22"
      "\x94\x86\x0a\xb6\x82\xed\xc5\xe8\x59\x7c\x00\xef\xbe\x82\x0f\xe1\x49\x7c"
      "\x81\x7a\xf2\xe4\x03\xe4\xb6\x32\xb3\x93\x64\x93\xee\x66\x43\x48\x33\x6d"
      "\xfc\x7e\x20\x61\x4d\xe6\x3f\xf3\x1f\x67\x37\x3b\xff\x40\x89\x00\xfc\xb7"
      "\x9e\xd6\x4f\xbe\x3d\xf8\x63\x1f\x46\xaa\xa8\x22\xe9\x91\x14\x49\xaa\x49"
      "\x55\x49\x37\x74\xb3\xf6\x6e\xff\x70\xf7\xb0\xd5\x6c\x8c\xea\xa8\xe2\x22"
      "\xec\xc3\x28\x8d\x34\xa7\xda\xec\xec\x37\xf3\x42\x6d\x9c\x8b\xb0\x9e\x48"
      "\xb1\xfd\x57\x55\xcb\xbd\xd7\x70\x66\x6a\x49\x92\xfc\x0e\x9d\x04\x82\x73"
      "\x57\x7f\x4f\x92\xfa\xe0\x3e\x09\x16\xfd\xd5\xe9\xde\xaf\x05\xcc\x71\x9a"
      "\xda\xa1\x13\x08\xcc\x74\xd4\xd1\x7b\xad\x84\xce\x03\x00\x10\x96\xbf\xff"
      "\x47\xfe\x3e\xbf\xec\xf7\xef\x51\x24\x6d\xf8\xdb\xfe\x5c\xdd\xff\x3b\xa1"
      "\x13\x08\x2c\x73\xff\xaf\x4a\x27\x4a\x8c\x5d\xdf\xab\xee\xad\x7e\xbd\xe7"
      "\x4a\x38\xbb\xf7\x8b\xba\x55\xe2\x24\x63\x2d\x28\x3d\xb3\x2a\x03\x09\x94"
      "\x55\x95\x2e\x97\x68\xe9\xf5\x6e\xab\x79\x7f\xe7\x4d\xab\x11\xe9\x93\x1e"
      "\x7b\x99\x66\x6b\xee\xb9\x91\x9e\xba\x5d\xd9\x6c\x3f\x9e\xee\x7a\x3d\xa7"
      "\x36\x1d\x61\xf2\xb9\x5f\x71\x73\xb8\x64\xe7\xb0\x5d\x90\xff\xea\x74\x47"
      "\x2c\x67\x7e\x98\x9f\xe6\xb9\x89\xf5\x55\x8d\xde\xfe\xaf\x9a\x18\xbb\x4c"
      "\x6e\xa5\xe2\xa1\x95\x4a\xf3\xdf\x2c\xee\xd1\xcd\x32\x4e\x5b\x15\xcc\xf2"
      "\x9a\x1b\xe4\x96\x1f\xc1\x2b\x99\x65\x6d\x30\x8d\xac\x05\xdf\xe7\xc0\x17"
      "\x04\x71\x59\x9e\x2e\xea\xfa\x50\x54\x3a\xbb\xad\x92\xa8\xd5\xdc\xa8\xed"
      "\x92\xa8\xb5\xe1\xa8\xfe\xd9\x5c\x1c\x79\xd6\xcc\x17\xf3\xcc\xac\xeb\xaf"
      "\xbe\xab\x9e\xd9\xff\x47\xf6\x7f\x7b\x43\xe3\x5c\x99\xb6\x8d\x6b\xe9\xcf"
      "\x8c\x91\xf3\xa9\xba\x96\x71\xf6\xa5\xf6\xed\xdc\x96\xd9\x0b\x58\x4b\x63"
      "\xcf\x07\x13\xf9\xac\x57\x7a\xa8\x95\x83\xa3\xe3\xbd\x97\xad\x56\xf3\xed"
      "\xc1\xd1\x71\xbb\x7b\xb0\xc7\xc1\x7c\x1f\x74\x4f\x82\xf3\x92\x0f\x07\xa5"
      "\x07\x8b\x76\xb9\x4a\x1b\xdb\xfd\xf9\x94\x06\x0d\xfb\xf1\x84\xd9\xe8\x2f"
      "\x7a\xe8\x4c\x10\x88\xdd\x77\x99\xc4\xd8\x83\x4c\xbd\xb2\xe9\x4a\x24\xfb"
      "\x14\x8f\xd8\xa7\x27\x65\x9d\x67\x7a\xdc\xca\xab\x0d\x12\xb7\x23\x97\x74"
      "\xb9\xb8\x82\xcb\xed\xb6\xb0\x82\x1b\xb7\xe6\xba\x73\x4f\xba\x3b\xfe\x88"
      "\xb1\xcf\x73\x4e\x98\xba\x7e\xe9\x05\xdf\xff\x03\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x5c\x34\xb3\xf8\xd3\x85\xd0\x73\x04\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xe0\xa2\x33"
      "\x3a\x67\xbf\xff\x2b\x7e\xff\x17\x98\x95\x7f\x01\x00\x00\xff\xff\x4c\x04"
      "\x84\xa3",
      614);
  syz_mount_image(/*fs=*/0x200000c0, /*dir=*/0x20000100, /*flags=*/0x4080,
                  /*opts=*/0x20000640, /*chdir=*/2, /*size=*/0x266,
                  /*img=*/0x200003c0);
  memcpy((void*)0x20000380, "./file1\000", 8);
  syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000380ul,
          /*flags=*/0x141842ul, /*mode=*/0ul);
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul, /*prot=*/7ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  use_temporary_dir();
  loop();
  return 0;
}
