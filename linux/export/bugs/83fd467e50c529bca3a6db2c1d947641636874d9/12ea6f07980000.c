// https://syzkaller.appspot.com/bug?id=83fd467e50c529bca3a6db2c1d947641636874d9
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
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

#include <linux/futex.h>
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

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  int i = 0;
  for (; i < 100; i++) {
    if (pthread_create(&th, &attr, fn, arg) == 0) {
      pthread_attr_destroy(&attr);
      return;
    }
    if (errno == EAGAIN) {
      usleep(50);
      continue;
    }
    break;
  }
  exit(1);
}

typedef struct {
  int state;
} event_t;

static void event_init(event_t* ev)
{
  ev->state = 0;
}

static void event_reset(event_t* ev)
{
  ev->state = 0;
}

static void event_set(event_t* ev)
{
  if (ev->state)
    exit(1);
  __atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1000000);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, 0);
}

static int event_isset(event_t* ev)
{
  return __atomic_load_n(&ev->state, __ATOMIC_ACQUIRE);
}

static int event_timedwait(event_t* ev, uint64_t timeout)
{
  uint64_t start = current_time_ms();
  uint64_t now = start;
  for (;;) {
    uint64_t remain = timeout - (now - start);
    struct timespec ts;
    ts.tv_sec = remain / 1000;
    ts.tv_nsec = (remain % 1000) * 1000 * 1000;
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, &ts);
    if (__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
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

struct thread_t {
  int created, call;
  event_t ready, done;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    event_wait(&th->ready);
    event_reset(&th->ready);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    event_set(&th->done);
  }
  return 0;
}

static void execute_one(void)
{
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  int i, call, thread;
  for (call = 0; call < 9; call++) {
    for (thread = 0; thread < (int)(sizeof(threads) / sizeof(threads[0]));
         thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        event_init(&th->ready);
        event_init(&th->done);
        event_set(&th->done);
        thread_start(thr, th);
      }
      if (!event_isset(&th->done))
        continue;
      event_reset(&th->done);
      th->call = call;
      __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
      event_set(&th->ready);
      event_timedwait(&th->done, 50 + (call == 0 ? 4000 : 0));
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    memcpy((void*)0x20000040, "nilfs2\000", 7);
    memcpy((void*)0x20000200, "./file0\000", 8);
    memcpy(
        (void*)0x20000a80,
        "\x78\x9c\xec\xdd\x4d\x6c\x1c\x57\x01\x00\xe0\x37\x6b\xaf\xf3\xdb\x66"
        "\x53\x1c\x6a\xd2\xd0\x26\x14\xda\xf2\x53\xbb\x71\x4c\xf8\x89\x80\x54"
        "\xcd\x85\xa8\xa9\xb8\x55\xaa\xb8\x44\x49\x5a\x22\x9c\x80\x48\x25\x48"
        "\xd5\x43\x92\x13\x37\x5a\x55\xe1\xca\xef\xa9\x97\xf2\xa3\x4a\xe4\x82"
        "\xa2\x9e\xb8\x54\xa2\x91\xb8\xf4\x54\x38\x70\x20\x0a\x52\x25\x0e\xd0"
        "\x90\xb8\xf2\xfa\xbd\xf5\xee\xf3\x6e\x66\xed\xd8\x9e\xac\xf7\xfb\xa4"
        "\xd9\xb7\x6f\xde\xdb\x7d\x6f\x76\x67\x66\x67\x67\xe6\xbd\x17\x80\xa1"
        "\x55\x6b\x3e\xce\xcc\x4c\x14\x21\x5c\xbe\xf2\xc6\x91\x7f\x3d\xf6\xcf"
        "\x2d\x21\x5c\x19\x5f\xcc\xd1\x68\x3e\x8e\xb6\xc5\xea\x21\x84\x22\xc6"
        "\x47\xb3\xf7\xfb\x60\x64\x21\xbc\xf5\xe1\xab\x27\xba\x85\x45\x98\x6e"
        "\x3e\xa6\x78\x78\xee\x46\xeb\xb5\xdb\x42\x08\x17\xc2\xde\x70\x35\x34"
        "\xc2\xee\xcb\xd7\x5e\x7f\x77\xfa\xd9\x63\x17\x8f\x5e\xda\xf7\xde\x9b"
        "\x87\xae\xaf\xcd\xd2\x03\x00\xc0\x70\xf9\xce\xd5\x43\x33\xbb\xfe\xfe"
        "\xd7\x87\x76\x7e\xf4\xd6\xc3\x87\xc3\xa6\xd6\xfc\x74\x7c\xde\x88\xf1"
        "\xed\xf1\xb8\xff\x70\x3c\xf0\x4f\xc7\xff\xb5\xd0\x19\x2f\xda\xa6\x76"
        "\x63\x59\xbe\xd1\x38\xd5\xb2\x7c\x23\x5d\xf2\xb5\x97\x53\xcf\xf2\x8d"
        "\xf6\x28\x7f\x2c\x7b\xdf\x7a\x8f\x7c\x9b\x4a\xca\x1f\x69\x9b\xd7\x6d"
        "\xb9\x61\x90\xa5\xf5\xb8\x11\x8a\xda\x64\x47\xbc\x56\x9b\x9c\x5c\xf8"
        "\x4f\x1e\x9a\xff\xeb\xc7\x8a\xc9\xb3\xa7\x67\x5f\x3c\x57\x51\x45\x81"
        "\x55\xf7\x9f\x47\x42\x08\x7b\x4d\xa6\xa1\x9f\x2e\x36\x37\x88\xea\xeb"
        "\xb1\xae\xd3\xdc\x8e\xaa\xf7\x40\x00\x0b\xf2\xeb\x85\x4b\x5c\xc8\xcf"
        "\x2c\xdc\x9d\xd6\xbb\x8d\xf6\x57\xfe\x8d\xa7\x6b\xdd\x5f\x0f\xab\x60"
        "\xbd\xd7\x7f\xe5\x0f\x56\xf9\xbf\xbd\x68\x8f\xc3\xea\xd9\xa8\x6b\x53"
        "\x5a\xae\xb4\x1d\x6d\x8f\xf1\xb6\xeb\x08\x67\x42\x97\xfb\x97\xf2\xed"
        "\x6f\x2e\x7f\xe3\x6c\xfb\x4f\xef\x97\x5f\x8f\xa8\xf7\x59\xcf\x5e\xd7"
        "\x11\x06\xe5\xfa\x42\xaf\x7a\x8e\xac\x73\x3d\x56\xaa\x57\xfd\xf3\xf5"
        "\x62\xa3\xfa\x46\x0c\xd3\xe7\xf0\xcd\x2c\xbd\x7d\xfb\xc9\xbf\xd3\x41"
        "\xf9\x8e\x81\xee\xfe\xeb\xfc\xbf\xc9\x34\xb4\xd3\x92\xe3\xdb\x3b\xc8"
        "\xef\x95\x01\x36\xb6\xfc\xbe\xb9\xb9\x28\xa5\xe7\xf7\xf5\xe5\xe9\x9b"
        "\x4a\xd2\x37\x97\xa4\x6f\x29\x49\xdf\xda\x91\x6b\x69\xfa\xb6\x92\xd7"
        "\xc3\x30\xfb\xe3\xcb\x3f\x0b\xaf\x15\x8b\xff\xf3\xf3\xff\xf4\xcb\x3d"
        "\x1f\x9e\xce\xb3\xdd\x17\xc3\xfb\x97\x59\x9f\xfc\x7c\xe4\x72\xcb\x1f"
        "\xbb\x43\x6c\x3d\xca\x77\x8c\xc4\x20\xf9\xd3\xf1\xe7\x4f\x7d\xf5\xe4"
        "\x0b\xd7\x16\xee\xff\x2f\x5a\xeb\xff\xed\xb8\xbe\xef\x8d\xf1\x46\xdc"
        "\x9a\xae\xc6\x0c\xe9\x7c\x61\x7e\x5e\xbd\x75\xef\x7f\xa3\xb3\x9c\x5a"
        "\x8f\x7c\x0f\x64\xf5\xb9\xaf\x4b\xfe\xe6\xf3\xf1\xce\x7c\xc5\xf8\xe2"
        "\xfb\x84\xb6\xfd\xcc\x92\x7a\x4c\x74\xbe\x6e\x47\xaf\x7c\x7b\x3a\xf3"
        "\x35\xb2\x7c\x5b\xe2\xb4\x39\xab\x6f\x7e\x7c\xb2\x35\x7b\x5d\x3a\xfe"
        "\x48\xfb\xd5\xf4\x79\x8d\x66\xcb\x5b\xcf\x96\x63\x2c\xab\x47\xda\xaf"
        "\xec\x8c\x61\x5e\x0f\x58\x89\xb4\x3e\xf6\xba\xff\x3f\xad\x9f\x13\xa1"
        "\x5e\xbc\x78\x7a\xf6\xd4\x53\x31\x9e\xd6\xd3\xbf\x8c\xd4\x37\xcd\xcf"
        "\xdf\x5f\x5e\xd4\xef\x56\xbb\xee\xc0\xdd\xe9\xb7\xfd\xcf\x44\xe8\x6c"
        "\xff\xb3\xbd\x35\xbf\x5e\x6b\xdf\x2f\xec\x58\x9c\x5f\xb4\xef\x17\x1a"
        "\xd9\xfc\xe9\x1e\xf3\x0f\xc4\x78\xfa\x9d\xfb\xde\xc8\x96\xe6\xfc\xc9"
        "\x13\x3f\x98\x3d\xb9\xda\x0b\x0f\x43\xee\xdc\xf9\x57\xbe\x7f\x7c\x76"
        "\xf6\xd4\x8f\x3c\xf1\xc4\x13\x4f\x5a\x4f\xaa\xde\x33\x01\x6b\x6d\xea"
        "\xe5\x33\x3f\x9c\x3a\x77\xfe\x95\x27\x4f\x9f\x39\xfe\xd2\xa9\x97\x4e"
        "\x9d\x3d\x70\xf0\xe0\x81\xe9\xe9\x83\x5f\x3b\x30\x33\xd5\x3c\xae\x9f"
        "\x6a\x3f\xba\x07\x36\x92\xc5\x1f\xfd\xaa\x6b\x02\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\xf4\xeb\xc7\x47\x8f\x5c\xfb\xdb\x3b\x5f\x79\x7f\xa1"
        "\xfd\xff\x62\xfb\xbf\xd4\xfe\x3f\xdd\xf9\x9b\xda\xff\xff\xb4\x58\x68"
        "\xeb\x5e\x6b\x9b\x1f\xba\x8c\x03\x98\xda\x01\xee\xec\x92\xde\xcc\x93"
        "\x75\xb0\x3a\x96\xe5\xab\xc7\xe9\x13\x59\x7d\xc7\xb3\x72\x76\x65\xaf"
        "\xfb\x64\x0c\x5b\xe3\xf8\xc5\xf6\xff\xa9\xb8\xbc\x5f\xd7\x54\x9f\x07"
        "\xb3\xf9\x79\xff\xbd\x29\x5f\xd6\x9d\xc0\x92\xfe\x52\xc6\xb2\x5e\x47"
        "\xf2\xf1\x02\x3f\x13\xc3\x4b\x31\xfc\x75\x80\x0a\x15\x5b\xba\xcf\x8e"
        "\x61\x59\xff\xd6\x69\x5d\x4f\xfd\x53\x2c\xa3\x5f\x8a\xee\x05\x53\x89"
        "\xf4\xbd\xa5\x2f\x25\xf5\x63\x92\xda\x7f\xf7\xea\xd7\x29\xed\xff\x77"
        "\xae\x43\x1d\x59\x7d\xeb\xd1\x9c\xb0\xea\x65\x04\xba\xfb\xf7\x10\xf7"
        "\xff\xdd\x3c\x76\x59\xc3\xf7\xff\xcd\xfd\xd5\x2f\xe3\xe6\xde\x69\x9b"
        "\xaa\xae\x9b\xa9\xfa\x69\x6e\xce\x28\x1e\xc0\xbd\xa1\xea\xf1\x3f\xd3"
        "\x79\xcf\x14\x9e\xfd\xf3\xb7\x37\xcf\x4f\x29\xdb\x8d\xa7\x3b\xf7\x97"
        "\x79\xff\xa5\x70\x37\xee\xf5\xf1\x27\x95\xbf\xb1\xc6\xff\x6c\x8d\x7f"
        "\xd7\xd7\xfe\xef\xe6\xd2\xd1\x13\x1a\x2b\x2b\xf7\x7f\xbf\xb8\xfe\x7e"
        "\x5b\xb1\x61\x77\xbf\xfb\xdf\x7c\xf9\x53\x3f\xd0\xe3\x79\x09\x77\x3e"
        "\x13\xfd\xd1\x7c\xf9\x37\x17\x17\xe5\xf1\xd0\x5f\xf9\x73\xbf\xca\xca"
        "\xcf\x2f\x08\xf5\xe9\x66\x5c\xfe\xf4\xf9\x6f\xed\xb3\xfc\x25\xcb\xbf"
        "\xa7\xac\xa4\xf3\x6f\x77\x9b\xfb\xff\x58\x7e\xfa\xd8\x9e\x78\xb4\xdf"
        "\xf2\x17\x6a\x5c\xd4\x3a\xeb\x91\x9f\x37\x4e\xd7\xff\xf2\xf3\xc6\xc9"
        "\xad\x6c\xf9\x4f\xae\x74\xf9\x57\x38\x50\xe3\xed\x58\x3e\x0c\xb3\x41"
        "\x19\x67\x76\xb9\xfa\x18\xff\xb7\xa9\x6c\xfc\xdf\x25\x56\x79\xfc\xdf"
        "\x5e\xf2\xfb\x30\xbe\x1c\xe3\x69\x47\x98\xee\x73\xc8\x47\x38\x59\x66"
        "\xfd\x5b\x91\xf4\x3b\xb0\x2b\x7b\xff\xa2\xe4\xf7\xcd\xf8\xbf\x83\xed"
        "\xeb\x31\x2c\xdb\x1e\xd2\xf8\xbf\x69\x7d\x6c\x74\x89\xd7\xda\xe2\xf5"
        "\x2e\x9f\xed\x46\xdd\xd7\xc0\xa0\xfa\xe0\xde\xbd\xfe\xb7\xf8\x43\x55"
        "\x7d\x5d\x4c\xfd\x7e\x5f\xd5\xd7\xc3\xb4\x8c\x69\x6e\x6e\x6e\x6d\x4f"
        "\x68\x95\xa8\xb4\x70\x2a\xff\xfc\xab\xfe\x9f\x50\x75\xf9\x55\x7f\xfe"
        "\x65\xf2\xf1\x7f\xf3\x63\xf8\x7c\xfc\xdf\x3c\x3d\x1f\xff\x37\x4f\xcf"
        "\xc7\xff\xcd\xd3\xf3\xf1\xf5\xf2\xf4\xad\x59\x7a\xfe\x79\xe6\xe3\xff"
        "\xe6\xe9\x0f\x66\xef\x9b\x8f\x0f\x3c\x91\xfd\xc1\xce\xd3\x3f\x55\xf2"
        "\xfa\xdd\x25\xe9\x0f\x95\xa4\xef\x29\x49\xff\x74\x49\xfa\xbe\x92\xf4"
        "\x87\x4b\xd2\x1f\x29\x49\x7f\xa0\x24\xfd\xd1\x92\xf4\xcf\x96\xa4\x7f"
        "\xae\x24\xfd\xb1\x92\xf4\x27\x4a\xd2\x3f\x5f\x92\xbe\xd1\xa5\xf6\x28"
        "\xc3\xba\xfc\x30\xcc\xf2\xf6\x79\xb6\x7f\x18\x1e\xa9\x7d\x6d\xaf\xed"
        "\x7f\xbc\x24\x1d\x18\x5c\x3f\x7f\x6b\xff\x33\x2f\xfc\xe1\xbb\x8d\x85"
        "\xf6\xff\x63\xad\xf3\x21\xe9\x3a\xde\xe1\x18\xaf\xc7\xff\xce\x3f\x89"
        "\xf1\xfc\xba\x77\x68\x8b\xcf\xa7\xbd\x13\xe3\xff\xc8\xd2\xef\xf5\xf3"
        "\x1d\x30\x4c\xf2\xfe\x33\xf2\xdf\xf7\xc7\x4b\xd2\x81\xc1\x95\xee\xf3"
        "\xb2\x7d\xc3\x10\x2a\xba\xb7\x93\xe8\xb7\xdf\xaa\x5e\xc7\xf9\x0c\x96"
        "\x2f\xc4\xf0\x8b\x31\xfc\x52\x0c\x9f\x8c\xe1\x64\x0c\xa7\x62\xb8\x3f"
        "\x86\xd3\xeb\x54\x3f\xd6\xc6\x33\xbf\x7f\xfb\xd0\x6b\xc5\xe2\xff\xfd"
        "\x1d\x59\x7a\xbf\xf7\x93\xe7\xed\x81\xf2\x7e\xa2\x0e\xf4\x59\x9f\xfc"
        "\xfc\xc0\x72\xef\xc7\xcf\xfb\xf1\x5b\xae\xbb\x2d\x7f\x85\xcd\xc1\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x2a\x53\x6b\x3e\xce\xcc\x4c\x14\x21\x5c"
        "\xbe\xf2\xc6\x91\xe7\x8f\x9d\x9e\x9a\x9f\xf3\xad\x56\x8e\x46\xf3\x71"
        "\xb4\x2d\x56\x6f\xbd\x2e\x84\xa7\x62\x38\x12\xc3\x5f\xc6\x27\xb7\x3e"
        "\x7c\xf5\x44\x7b\x78\x3b\x86\x45\x98\x0e\x45\x28\x5a\xf3\xc3\x73\x37"
        "\x5a\x25\x6d\x0b\x21\x5c\x08\x7b\xc3\xd5\xd0\x08\xbb\x2f\x5f\x7b\xfd"
        "\xdd\xe9\x67\x8f\x5d\x3c\x7a\x69\xdf\x7b\x6f\x1e\xba\xbe\x76\x9f\x00"
        "\x00\x00\x00\x6c\x7c\x1f\x07\x00\x00\xff\xff\x57\xcf\x1c\xd1",
        2650);
    syz_mount_image(/*fs=*/0x20000040, /*dir=*/0x20000200,
                    /*flags=MS_SYNCHRONOUS*/ 0x10, /*opts=*/0x20000480,
                    /*chdir=*/5, /*size=*/0xa5a, /*img=*/0x20000a80);
    break;
  case 1:
    memcpy((void*)0x20000080, "./bus\000", 6);
    res =
        syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000080ul,
                /*flags=O_SYNC|O_DIRECT|O_CREAT|O_RDWR*/ 0x105042, /*mode=*/0);
    if (res != -1)
      r[0] = res;
    break;
  case 2:
    memcpy((void*)0x20000180, "./bus\000", 6);
    res = syscall(
        __NR_open, /*file=*/0x20000180ul,
        /*flags=O_TRUNC|O_NOCTTY|O_LARGEFILE|O_CREAT|FASYNC|0x3e*/ 0xa37eul,
        /*mode=*/0ul);
    if (res != -1)
      r[1] = res;
    break;
  case 3:
    memcpy((void*)0x20000040, "./bus\000", 6);
    res = syscall(
        __NR_open, /*file=*/0x20000040ul,
        /*flags=O_TRUNC|O_NOCTTY|O_NOATIME|O_DIRECT|O_CREAT|0x2002*/ 0x46342ul,
        /*mode=*/0ul);
    if (res != -1)
      r[2] = res;
    break;
  case 4:
    syscall(__NR_ftruncate, /*fd=*/r[2], /*len=*/0x2088002ul);
    break;
  case 5:
    syscall(
        __NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x600000ul,
        /*prot=PROT_GROWSUP|PROT_WRITE|0x8088e3ad102bc190*/
        0x8088e3ad122bc192ul,
        /*flags=MAP_UNINITIALIZED|MAP_LOCKED|MAP_FIXED|MAP_SHARED*/ 0x4002011ul,
        /*fd=*/r[1], /*offset=*/0ul);
    break;
  case 6:
    syscall(__NR_read, /*fd=*/r[0], /*buf=*/0x20000000ul, /*count=*/0x2000ul);
    break;
  case 7:
    syscall(__NR_read, /*fd=*/r[0], /*buf=*/0x20001500ul, /*len=*/0x2020ul);
    break;
  case 8:
    memcpy((void*)0x20000040, "cpu.stat\000", 9);
    syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000040ul,
            /*flags=*/0x275a, /*mode=*/0);
    break;
  }
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
  for (procid = 0; procid < 5; procid++) {
    if (fork() == 0) {
      use_temporary_dir();
      loop();
    }
  }
  sleep(1000000);
  return 0;
}