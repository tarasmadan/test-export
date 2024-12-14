// https://syzkaller.appspot.com/bug?id=f86606dfb403cc9435c0ed5f17e1d80c110cddef
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

#define FS_IOC_SETFLAGS _IOW('f', 2, long)
static void remove_dir(const char* dir)
{
  int iter = 0;
  DIR* dp = 0;
  const int umount_flags = MNT_FORCE | UMOUNT_NOFOLLOW;

retry:
  while (umount2(dir, umount_flags) == 0) {
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
    while (umount2(filename, umount_flags) == 0) {
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
      if (umount2(filename, umount_flags))
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
        if (umount2(dir, umount_flags))
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
      sleep_ms(10);
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
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
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  memcpy((void*)0x20000080, "hfsplus\000", 8);
  memcpy((void*)0x20000140, "./file1\000", 8);
  memcpy(
      (void*)0x20001bc0,
      "\x78\x9c\xec\xdd\xcd\x6f\x1c\x67\x1d\x07\xf0\xef\xae\x37\xb6\x37\x94\xd4"
      "\x4d\x93\x36\x45\x95\x62\x35\x12\x20\x2c\x12\xbf\xc8\x05\x73\x21\x45\x08"
      "\xf9\x50\xa1\xaa\x1c\x38\x5b\x89\xd3\x58\xd9\xa4\xc5\x76\x91\x5b\x21\xea"
      "\xf2\x7a\xed\x21\x7f\x40\x39\xf8\xc6\x09\x89\x7b\xa4\x72\xe1\x02\xb7\x5e"
      "\x7d\xac\x84\xe0\xd2\x0b\xe6\xb4\x68\x66\x67\xed\xad\xed\xf5\x4b\x88\xb3"
      "\x76\xf9\x7c\xa2\xd9\xe7\x99\x79\x66\x9e\xe7\xf7\xfc\x76\x66\x67\x77\xad"
      "\x68\x03\xfc\xdf\x9a\x9f\x48\xe3\x51\x6a\x99\x9f\x78\x7d\xad\x58\xdf\xdc"
      "\x98\x69\x6d\x6e\xcc\xdc\xef\xd6\x93\x8c\x24\xa9\x27\x8d\x4e\x91\xda\xbf"
      "\xdb\xed\xf6\x27\xc9\xcd\x74\x96\xbc\x54\x6c\xac\xba\xab\xf5\x1b\xe7\xe1"
      "\xd2\xdc\x9b\x9f\x7e\xbe\xf9\x59\x51\x1f\x2a\xfb\x6a\x74\xf7\xaf\x1f\x74"
      "\xdc\xd1\xac\x57\x4b\xc6\xcb\xee\x3b\xe5\x93\xea\xef\xd6\x61\xfd\x8d\x1e"
      "\xd6\x5d\x6d\x7b\x86\x45\xc2\xae\x75\x13\x07\x83\x76\x2e\x49\xbb\xf4\xcf"
      "\x87\x9d\x2d\x3f\xfb\xeb\x33\xdb\x2d\x3d\x9a\xfb\x1d\x7d\xe8\x99\x0f\x9c"
      "\x01\xb5\xce\x7d\x73\x8f\xb1\xe4\x7c\x75\xa1\x17\xef\x03\x3a\x77\xc5\xce"
      "\x3d\xfb\x4c\x5b\x1f\x74\x00\x00\x00\x00\xf0\x14\x3c\xbb\x95\xad\xac\xe5"
      "\xc2\xa0\xe3\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\xb3\xa4\xfa\xfd\xff"
      "\x5a\xb5\xd4\xbb\xf5\xf1\xd4\xba\xbf\xff\x3f\x5c\x6d\x4b\x55\x3f\x5d\xae"
      "\x1e\x6f\xf7\x47\x27\x15\x07\x00\x00\x00\x00\x00\x00\x00\x9c\x9c\x67\x5f"
      "\xdb\xb5\xe1\xea\x56\xb6\xb2\x96\x0b\xdd\xf5\x76\xad\xfc\x9b\xff\x2b\xe5"
      "\xca\xa5\xf2\xf1\x2b\x79\x37\x2b\x59\xcc\x72\xae\x67\x2d\x0b\x59\xcd\x6a"
      "\x96\x33\x95\x64\xac\xa7\xa3\xe1\xb5\x85\xd5\xd5\xe5\xa9\x23\x1c\x39\xbd"
      "\xef\x91\xd3\x87\x44\x3e\x52\x95\xcd\xc7\x9f\x3c\x00\x00\x00\x00\x00\x00"
      "\x00\x7c\x89\xfd\x2a\xf3\x3b\x7f\xff\x07\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x80\xd3\xa0\x96\x0c\x75\x8a\x72\xb9\xd4\xad\x8f\xa5\xde\x48\x32"
      "\x9a\x64\xb8\xd8\x6f\x3d\xf9\x7b\xb7\x7e\x96\x3d\x1a\x74\x00\x00\x00\x00"
      "\xf0\x14\x3c\xbb\x95\xad\xac\xe5\x42\x77\xbd\x5d\x2b\x3f\xf3\xbf\x50\x7e"
      "\xee\x1f\xcd\xbb\x79\x90\xd5\x2c\x65\x35\xad\x2c\xe6\x76\xf9\x5d\x40\xe7"
      "\x53\x7f\x7d\x73\x63\xa6\xb5\xb9\x31\x73\xbf\x58\xf6\xf6\xfb\xda\xbf\x8e"
      "\x15\x46\xd9\x63\x3a\xdf\x3d\xec\x3f\xf2\x95\x72\x8f\x66\xee\x64\xa9\xdc"
      "\x72\x3d\xb7\xf2\x76\x5a\xb9\x9d\x7a\x79\x64\xe1\x4a\x37\x9e\xfd\xe3\xfa"
      "\xb0\x88\xa9\xf6\xfd\xca\x11\x23\xbb\x5d\x95\xc5\xcc\x3f\xaa\xca\x3d\x3e"
      "\x38\xd6\x64\xfb\x39\xe6\x97\x29\x63\x65\x46\xce\x6d\x67\x64\xb2\x8a\xad"
      "\xc8\xc6\x73\x07\x67\xe2\x98\xcf\xce\xee\x91\xa6\x52\xdf\x0e\xf6\xd2\xae"
      "\x91\x76\x4d\xe2\xb1\x72\x7e\xbe\x2a\x8b\xf9\xfc\xae\x5f\xce\x07\x62\x77"
      "\x26\xa6\x7b\xce\xbe\x17\x0e\xce\x79\xf2\x8d\x3f\xff\xf1\xa7\x77\x5b\x0f"
      "\xee\xdd\xbd\xb3\x32\x71\x7a\xa6\x74\xb0\xf5\xaa\x1c\xaa\xca\x76\xf9\xd8"
      "\xdc\x9b\x89\x99\x9e\x4c\xbc\xf8\x65\xcc\x44\x5f\x93\x65\x26\x2e\x6f\xaf"
      "\xcf\xe7\x47\xf9\x49\x26\x32\x9e\x37\xb2\x9c\xa5\xfc\x3c\x0b\x59\xcd\x62"
      "\xc6\xf3\xc3\xb2\xb6\x50\x9d\xcf\xb5\x9e\x4b\xbe\x4f\xa6\x6e\x7e\x61\xed"
      "\x8d\xc3\x22\x19\xae\xce\xd0\xce\x93\x75\xbc\x98\x5e\x29\x8f\xbd\x90\xa5"
      "\xfc\x38\x6f\xe7\x76\x16\xf3\x6a\xf9\x6f\x3a\x53\xf9\x4e\x66\x33\x9b\xb9"
      "\x9e\x67\xf8\xf2\x11\x5e\x69\xeb\x7d\xae\xfa\xf6\x57\xf7\x0d\xfe\xda\x37"
      "\xab\x4a\x33\xc9\xef\xab\xf2\x74\x28\xf2\xfa\x5c\x4f\x5e\x7b\x5f\x73\xc7"
      "\xca\xb6\xde\x2d\x3b\x59\xba\xf8\xe4\xef\x47\x8d\xaf\x55\x95\x62\x8c\x5f"
      "\x57\xe5\xe9\xb0\x3b\x13\x53\x3d\x99\x78\xfe\xe0\x4c\xfc\xa1\x7c\x59\x59"
      "\x69\x3d\xb8\xb7\x7c\x77\xe1\x9d\xa3\x0d\x77\xf1\xa3\xaa\x52\x5c\x47\xbf"
      "\x3d\x55\x77\x89\xe2\x7c\xb9\x58\x3c\x59\xe5\xda\x17\xcf\x8e\xa2\xed\xf9"
      "\x7d\xdb\xa6\xca\xb6\x4b\xdb\x6d\xf5\x3d\x6d\x97\xb7\xdb\x3a\x57\xea\x7a"
      "\xdf\x2b\x75\xb8\x7a\x0f\xb7\xb7\xa7\xe9\xb2\xed\xc5\x7d\xdb\x66\xca\xb6"
      "\x2b\x3d\x6d\xfb\xbd\xdf\x02\xe0\xd4\x3b\xff\xad\xf3\xc3\xcd\x7f\x34\xff"
      "\xd6\xfc\xb8\xf9\x9b\xe6\xdd\xe6\xeb\xa3\x3f\x18\xf9\xee\xc8\xcb\xc3\x39"
      "\xf7\x97\x73\xdf\x6b\x4c\x0e\x7d\xbd\xfe\x72\xed\x4f\xf9\x38\xbf\xdc\xf9"
      "\xfc\x0f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x3c\xbe\x95\xf7\xde\xbf\xb7\xd0\x6a\x2d\x2e"
      "\xef\xaa\xb4\xdb\xed\x0f\xfa\x34\x9d\xe5\x4a\xf7\xe7\xcc\x9e\xe2\xa0\x2f"
      "\x3d\x93\x0c\x6a\xca\xc3\x49\x4e\x47\xe6\xff\xd3\x6e\xb7\xab\x2d\xb5\xd3"
      "\x10\xcf\xc1\x95\x76\x61\x24\xed\x13\x1f\xab\x91\x64\xbf\xa6\xab\xbd\x5b"
      "\x3e\x1c\xc8\xf9\x33\xe0\x17\x26\xe0\xc4\xdd\x58\xbd\xff\xce\x8d\x95\xf7"
      "\xde\xff\xf6\xd2\xfd\x85\xb7\x16\xdf\x5a\x7c\x30\x37\x3b\x3b\x37\x39\x37"
      "\xfb\xea\xcc\x8d\x3b\x4b\xad\xc5\xc9\xce\xe3\xa0\xa3\x04\x4e\xc2\xce\x4d"
      "\x7f\xd0\x91\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x47\xf5\x64\xfe\xcf"
      "\x40\x33\x49\xff\x7d\xfa\x8f\x3e\xfa\x34\xa7\x0a\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x9c\x51\xf3"
      "\x13\x69\x3c\x4a\x2d\x53\x93\xd7\x27\x8b\xf5\xcd\x8d\x99\x56\xb1\x74\xeb"
      "\x3b\x7b\x36\x92\xd4\x93\xd4\x7e\x91\xd4\x3e\x49\x6e\xa6\xb3\x64\xac\xa7"
      "\xbb\x5a\xbf\x71\x1e\x2e\xcd\xbd\xf9\xe9\xe7\x9b\x9f\xed\xf4\xd5\xe8\xee"
      "\x5f\x3f\xe8\xb8\xa3\x59\xaf\x96\x8c\x27\x19\xaa\xca\x27\xd5\xdf\xad\xff"
      "\xb9\xbf\xda\xf6\x0c\x8b\x84\x5d\xeb\x26\x0e\x06\xed\xbf\x01\x00\x00\xff"
      "\xff\xc5\x23\x05\x32",
      1625);
  syz_mount_image(/*fs=*/0x20000080, /*dir=*/0x20000140,
                  /*flags=MS_RELATIME*/ 0x200000, /*opts=*/0x20000200,
                  /*chdir=*/1, /*size=*/0x659, /*img=*/0x20001bc0);
  memcpy((void*)0x20000100, "./bus\000", 6);
  syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000100ul,
          /*flags=O_CREAT|O_RDWR*/ 0x42,
          /*mode=S_IRGRP|S_IXUSR|S_IRUSR*/ 0x160);
  memcpy((void*)0x20000380, "/dev/loop", 9);
  *(uint8_t*)0x20000389 = 0x30;
  *(uint8_t*)0x2000038a = 0;
  memcpy((void*)0x20000140, "./bus\000", 6);
  syscall(__NR_mount, /*src=*/0x20000380ul, /*dst=*/0x20000140ul, /*type=*/0ul,
          /*flags=MS_BIND*/ 0x1000ul, /*data=*/0ul);
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
  use_temporary_dir();
  loop();
  return 0;
}