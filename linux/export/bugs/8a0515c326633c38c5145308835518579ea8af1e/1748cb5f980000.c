// https://syzkaller.appspot.com/bug?id=8a0515c326633c38c5145308835518579ea8af1e
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
  memcpy((void*)0x20000040, "hfsplus\000", 8);
  memcpy((void*)0x20000640, "./file0\000", 8);
  memcpy((void*)0x20000540,
         "\x6e\x6f\x64\x65\x63\x6f\x6d\x70\x6f\x73\x65\x00\x80\x00\x00\x00\x00"
         "\x00\x00\x42\x00\x0f\xc3\x2c\x6e\x6f\x63\x61\x72\x72\x69\x65\x72\x2c"
         "\x6e\x6f\x64\x65\x63\x6e\x6d\x70\x6f\x73\x65\x00\xa5\x80\x26\x3e\x00"
         "\x5a\x80\xc9\x76\x89\x81\x13\xcb\x9f\x46\x80\x64\x2c\xe8\x6d\xd3\xb5"
         "\xd3\xba\x4a\x73\x9e\x66\x54\xe4\xa1\x27\x7e\xf2\xdd\x52\xea\x98\xc5"
         "\xea\x63\x0d\xa0\xc2\x9c\xd7\x3f\x8b\x9f\x4f\xd0\x77\xe9\x7f\x3e\xc1"
         "\xac\xc1\xac\x91\xb9\x3e\xec\x36\x92\x17\x14\x2e\x58\x97",
         116);
  memcpy(
      (void*)0x20000c80,
      "\x78\x9c\xec\xdd\x4f\x6b\x1c\xe7\x1d\x07\xf0\xef\xac\xac\xb5\xa4\x82\xbd"
      "\x76\xec\x24\x2e\x85\x8a\x14\x42\x89\xa9\xad\x3f\x4e\xaa\x42\xa1\x69\x9a"
      "\x16\x51\x42\x09\xf4\x12\x08\x39\x88\x5a\x8a\x85\xd7\x4e\x90\x36\x45\xc9"
      "\xa1\xa8\xa5\xaf\xa0\xaf\x20\xa5\xa8\x87\x9c\x7a\xe8\xa9\xb4\x90\x43\xcf"
      "\x7d\x0b\x2a\x39\x16\x7a\xf2\x45\x37\x85\x99\x9d\xd5\xae\xe5\xb5\x2c\x5b"
      "\x8e\x76\x9d\x7c\x3e\x30\x3b\xcf\xb3\xcf\x3c\xcf\xfc\x9e\x9f\x67\x86\x99"
      "\x15\x66\x02\x7c\x63\x2d\xbf\x93\xc9\xed\x14\x59\xbe\xfa\xd6\x56\x59\xdf"
      "\xdd\x59\x6c\xef\xee\x2c\xde\xe9\x95\x93\x9c\x4d\xd2\x48\xa6\x92\x14\xe5"
      "\xd7\x7f\x4f\xf2\x45\xb2\x9d\xee\x92\x2b\xbd\x86\x81\xf5\x03\xde\x9b\x69"
      "\xde\xfb\xec\xdd\x2b\x6b\xdd\xda\x54\xbd\x54\xdb\x17\x47\xf5\x3b\x9e\xed"
      "\x9c\x9f\x38\xa8\x34\x92\xb4\x4e\x3a\x5e\x3d\xb7\x72\x9c\x85\x13\x8f\xd7"
      "\x9f\xe1\x6c\x92\x8b\xf5\x1a\x46\x6e\xbf\xe7\xbf\x43\x9b\x4f\x78\x5e\x02"
      "\x00\xe3\xac\x48\x26\x86\x7d\xdf\x4a\x66\xea\x9b\xf5\xf2\x39\xa0\x7b\x57"
      "\xdc\xbd\xc7\x7e\xa6\x6d\x8f\x3a\x00\x00\x00\x00\x38\x05\xe7\xf7\xb2\x97"
      "\xad\x9c\x1b\x75\x1c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf0\x2c\xa9\xdf"
      "\xff\x5f\xd4\x4b\xa3\x57\x9e\x4d\xd1\x7b\xff\x7f\xb3\xfe\x2e\x75\xf9\x99"
      "\xf6\xf9\xa8\x03\x00\x00\x00\x00\x00\x00\x00\x80\xa7\xe0\xbb\x7b\xd9\xcb"
      "\x56\xce\xf5\xea\xfb\x45\xf5\x37\xff\x97\xaa\xca\xa5\xea\xf3\x5b\xf9\x28"
      "\x9b\x59\xcd\x46\xae\x65\x2b\x2b\xe9\xa4\x93\x8d\xcc\x27\x69\x0d\x0c\xd4"
      "\xdc\x5a\xe9\x74\x36\xe6\x8f\xd1\x73\x61\x68\xcf\x85\xd3\x99\x2f\x00\x00"
      "\x00\x00\x00\x00\x00\x7c\x4d\xfd\x21\xcb\xfd\xbf\xff\x03\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\xc0\x38\x28\x92\x89\xee\xaa\x5a\x2e\xf5\xca\xad"
      "\x34\xce\x24\x99\x4a\xd2\x2c\xb7\xdb\x4e\xfe\xd9\x2b\x3f\xcb\x3e\x1f\x75"
      "\x00\x00\x00\x00\x70\x0a\xce\xef\x65\x2f\x5b\x39\xd7\xab\xef\x17\xd5\x33"
      "\xff\xf3\xd5\x73\xff\x54\x3e\xca\xdd\x74\xb2\x9e\x4e\xda\x59\xcd\xcd\xea"
      "\xb7\x80\xee\x53\x7f\x63\x77\x67\xb1\xbd\xbb\xb3\x78\xa7\x5c\x1e\x1c\xf7"
      "\xa7\xff\x7f\xac\x30\xaa\x11\xd3\xfd\xed\x61\xf8\x9e\xe7\xca\x2d\xf6\x27"
      "\x0e\x7a\x2c\xe7\x17\xf9\x75\xae\x66\x36\x6f\x67\x23\xeb\xf9\x6d\x56\xd2"
      "\xc9\x6a\x66\xf3\x66\x55\x5a\x49\x91\x56\xfd\xeb\x45\xab\x17\xe7\xf0\x78"
      "\x5f\xbf\xaf\xf6\xf6\xa3\x62\x7d\xb1\x8a\x75\x3a\x6b\x59\xaf\x62\xbb\x96"
      "\xdf\xe4\x83\xb4\x73\x33\x8d\x6a\x0e\xd5\x36\x47\xef\xf1\xf7\x65\x76\x8a"
      "\x9f\xd4\x8e\x99\xa3\x9b\xf5\xba\x9c\xd1\x2f\xeb\xf5\x53\x75\xf9\x49\x3b"
      "\xb6\xaa\x8c\x4c\x1e\x64\x64\xae\xce\x7d\x99\x8d\x0b\x47\x67\xe2\x21\xc7"
      "\xc9\xc3\xe6\x76\x78\x4f\xf3\x69\x1c\xfc\x06\x75\xe9\x2b\xc8\xf9\xcc\x40"
      "\x3c\x6f\x7e\x15\x39\x7f\x62\x87\x33\xb1\x30\x70\xf4\x3d\x7f\x74\x26\x92"
      "\x97\xff\xf2\xa7\x7b\xb7\xda\x77\x6f\xdf\x5a\xdb\xbc\x3a\x3e\x53\x7a\x42"
      "\x87\x33\xb1\x38\x90\x89\x17\xbe\x51\x99\x68\xd6\xd9\xe8\x5e\x25\x1b\x03"
      "\x27\xf4\xa3\xaf\x96\x2f\x55\x7d\xcf\x65\x3d\xbf\xca\x07\xb9\x99\xd5\xbc"
      "\x96\x1b\x59\xca\x42\x5e\xcd\x8d\xcc\xe7\x47\x79\x75\x20\xaf\x97\x8f\x71"
      "\xae\x35\x1e\xef\x5c\xfb\xde\xf7\xeb\xc2\x64\x92\x9f\xd7\xeb\xf1\x50\xe6"
      "\xf5\xc2\x40\x5e\x07\xaf\x74\xad\xaa\x6d\xf0\x9b\x7e\x96\x2e\x3e\xfd\x2b"
      "\xd2\x99\x6f\xd7\x85\xf2\x60\x7d\x63\xec\xae\x48\x17\x0e\x5d\x9b\x7b\x99"
      "\x78\xee\xe8\x4c\xfc\x79\xbf\xfc\xdc\x6c\xdf\xbd\xbd\x71\x6b\xe5\xc3\x63"
      "\xee\xef\xe5\x7a\x5d\x66\xe0\x67\x63\x95\x89\xf2\x78\xb9\x58\xfe\x63\x55"
      "\xb5\xfb\x8f\x8e\xb2\xed\xb9\xa1\x6d\xf3\x55\xdb\xa5\x83\xb6\xc6\x03\x6d"
      "\x97\x0f\xda\x1e\x75\xa6\x36\xeb\x7b\xb8\x07\x46\x9a\x5e\xa8\xda\x5e\x18"
      "\xba\x97\xc5\xaa\xed\xc5\x81\xb6\x61\x77\x39\x00\x8c\xbd\x99\x57\x66\x9a"
      "\xd3\xff\x9b\xfe\xcf\xf4\xa7\xd3\x7f\x9c\xbe\x35\xfd\xd6\xd4\x1b\x67\x97"
      "\xce\x7e\xa7\x99\xc9\x7f\x9f\xf9\xc7\xc4\xdf\x1a\x7f\x6d\xfc\xb8\x78\x25"
      "\x9f\xe6\x77\xfd\xe7\x7f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xe0\xc9\x6d\x7e\xfc\xc9\xed"
      "\x95\x76\x7b\x75\x43\xe1\x34\x0b\xaf\x8f\x47\x18\x0a\x63\x5e\xe8\xbd\x47"
      "\xeb\x71\x7a\xf5\xde\x87\x75\xc2\xbd\x8f\xf0\xa2\x04\x9c\x8a\xeb\x9d\x3b"
      "\x1f\x5e\xdf\xfc\xf8\x93\x1f\xac\xdf\x59\x79\x7f\xf5\xfd\xd5\xbb\x37\x96"
      "\x7e\xb8\xf8\xda\x8d\xf9\xa5\xa5\xeb\x6b\xeb\xed\xd5\xb9\xee\xe7\xa8\xa3"
      "\x04\x00\x9e\xa6\xfe\x4d\xff\xa8\x23\x01\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x1e\xe6\x34\xfe\x33\xf3\xa8\xe7\x08\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x7c\xbd\x2d\xbf\x93\xc9\xed"
      "\x14\x99\x9f\xbb\x36\x57\xd6\x77\x77\x16\xdb\xe5\xd2\x2b\xf7\xb7\x9c\x4a"
      "\x52\x94\x85\x7f\x25\xf9\x22\xd9\x4e\x77\x49\x6b\x60\xb8\xe2\x61\xfb\x79"
      "\x6f\xa6\x79\xef\xb3\x77\xaf\xac\xf5\xc7\x9a\xea\x6d\x5f\x1c\xd5\xef\x78"
      "\xee\x8b\xa5\x71\x28\xa6\x93\x8e\xb7\x70\xe2\xf1\xfa\x33\x9c\x4d\x72\xb1"
      "\x5e\xc3\xc8\x7d\x19\x00\x00\xff\xff\x5d\xb2\xfc\xef",
      1525);
  syz_mount_image(/*fs=*/0x20000040, /*dir=*/0x20000640,
                  /*flags=MS_RELATIME|MS_RDONLY|MS_NOSUID|MS_NODEV*/ 0x200007,
                  /*opts=*/0x20000540, /*chdir=*/1, /*size=*/0x5f5,
                  /*img=*/0x20000c80);
  memcpy((void*)0x20000000, ".\000", 2);
  memcpy((void*)0x20000200, "./file0/../file0\000", 17);
  syscall(__NR_mount, /*src=*/0x20000000ul, /*dst=*/0x20000200ul, /*type=*/0ul,
          /*flags=MS_SHARED|MS_SYNCHRONOUS|MS_RDONLY|MS_DIRSYNC|MS_BIND*/
          0x101091ul, /*data=*/0ul);
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
