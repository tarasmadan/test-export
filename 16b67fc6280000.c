// https://syzkaller.appspot.com/bug?id=983b1f54c1223acdd606d7cb06e88a877f7f50ba
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
  int i, call, thread;
  for (call = 0; call < 5; call++) {
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
      if (call == 0)
        break;
      event_timedwait(&th->done,
                      50 + (call == 0 ? 4000 : 0) + (call == 1 ? 4000 : 0));
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

void execute_call(int call)
{
  switch (call) {
  case 0:
    memcpy((void*)0x200004c0, "ext4\000", 5);
    memcpy((void*)0x20000040, "./file0\000", 8);
    memcpy((void*)0x20000280,
           "\x71\x75\x6f\x74\x61\x2c\x6a\x71\x66\x6d\x5a\xfe\x7f\x21\x63\x31"
           "\x32\x9e\x6e\x8f\x75\x69\x64\x33\x32\x2c\x00\xee\x6a\xe2\x55\x57"
           "\xc4\x0e\x9e\x2d\xb5\x5d\x44\xd7\x24\xa9\x29\xbf\x36\xf6\x0b\x3b"
           "\xef\x2e\x2d\x7f\x0a\x78\xac\x95\x41\x17\x3c\x0e\x43\xbe\x28",
           63);
    memcpy(
        (void*)0x20000540,
        "\x78\x9c\xec\xdd\xc1\x4f\x5c\x5b\x19\x00\xf0\xef\x5e\xa0\x50\x1e\xef"
        "\xc1\xd3\xb7\x50\xa3\xb6\xd6\xd6\x6a\x9a\xce\xc0\xb4\x25\x4d\x57\x75"
        "\xa3\x31\x4d\x13\x63\xe3\xca\x45\x8b\x30\x25\x84\x19\x86\x30\x43\x2d"
        "\xd8\x05\xfd\x1f\x4c\x6c\xe2\x4a\xff\x04\x17\x26\x2e\x4c\xba\x72\xef"
        "\x4e\x77\x6e\xea\xc2\xa4\x6a\xa3\x29\x26\x2e\xc6\xdc\x3b\x03\x05\xca"
        "\x00\x7d\x2d\x4c\xc2\xfd\xfd\x92\x93\x7b\xcf\x3d\x03\xdf\x39\x4c\xee"
        "\x39\xc3\x37\x30\x27\x80\xc2\x3a\x1f\x11\x97\x22\xe2\x4c\x44\x3c\x88"
        "\x88\xf1\xee\xf5\xa4\x5b\xe2\x76\xa7\x64\x8f\x7b\xfd\xea\xc9\xec\xe6"
        "\xab\x27\xb3\x49\xb4\xdb\xf7\xfe\x99\xe4\xed\xd9\xb5\xd8\xf1\x35\x99"
        "\x8f\x22\x62\x23\x22\x46\x22\xe2\x47\xdf\x8f\xf8\x69\xf2\x76\xdc\xe6"
        "\xda\xfa\xe2\x4c\xad\x56\x5d\xe9\xd6\xcb\xad\xfa\x72\xb9\xb9\xb6\x7e"
        "\x75\xa1\x3e\x33\x5f\x9d\xaf\x2e\x55\x2a\xd3\x53\xd3\x93\x37\xaf\xdd"
        "\xa8\x7c\xb0\xb1\x9e\xab\xff\xf6\xe5\xf7\x16\xee\xfc\xf8\x0f\xbf\xff"
        "\xda\x8b\x3f\x6d\x7c\xe7\xe7\x59\xb7\xc6\xba\x6d\x3b\xc7\xf1\x21\x75"
        "\x86\x3e\xb4\x1d\x27\x33\x18\x11\x77\x8e\x23\x58\x1f\x0c\x74\xc7\x73"
        "\xa6\xdf\x1d\xe1\x73\x49\x23\xe2\x0b\x11\x71\x21\xbf\xff\xc7\x63\x20"
        "\x7f\x36\x01\x80\xd3\xac\xdd\x1e\x8f\xf6\xf8\xce\x3a\x00\x70\xda\xa5"
        "\x79\x0e\x2c\x49\x4b\xdd\x5c\xc0\x58\xa4\x69\xa9\xd4\xc9\xe1\x7d\x16"
        "\xa3\x69\xad\xd1\x6c\x5d\x79\xd8\x58\x5d\x9a\xeb\xe4\xca\x26\x62\x28"
        "\x7d\xb8\x50\xab\x4e\x76\x73\x85\x13\x31\x94\x64\xf5\xa9\xfc\xfc\x4d"
        "\xbd\xb2\xa7\x7e\x2d\x22\x3e\x8d\x88\x5f\x0c\x9f\xcd\xeb\xa5\xd9\x46"
        "\x6d\xae\x9f\x2f\x7c\x00\xa0\xc0\x3e\xda\xb3\xfe\xff\x67\xb8\xb3\xfe"
        "\x03\x00\xa7\xdc\x48\xbf\x3b\x00\x00\x9c\x38\xeb\x3f\x00\x14\x8f\xf5"
        "\x1f\x00\x8a\xc7\xfa\x0f\x00\xc5\x63\xfd\x07\x80\xe2\xb1\xfe\x03\x40"
        "\xf1\x58\xff\x01\xa0\x50\x7e\x78\xf7\x6e\x56\xda\x9b\xdd\xcf\xbf\x9e"
        "\x7b\xb4\xb6\xba\xd8\x78\x74\x75\xae\xda\x5c\x2c\xd5\x57\x67\x4b\xb3"
        "\x8d\x95\xe5\xd2\x7c\xa3\x31\x9f\x7f\x66\x4f\xfd\xb0\xef\x57\x6b\x34"
        "\x96\xa7\xae\xc7\xea\xe3\x72\xab\xda\x6c\x95\x9b\x6b\xeb\xf7\xeb\x8d"
        "\xd5\xa5\xd6\xfd\xfc\x73\xbd\xef\x57\x87\x4e\x64\x54\x00\xc0\x41\x3e"
        "\x3d\xf7\xfc\x2f\x49\x44\x6c\xdc\x3a\x9b\x97\xd8\xb1\x97\x83\xb5\x1a"
        "\x4e\xb7\xb4\xdf\x1d\x00\xfa\x66\xa0\xdf\x1d\x00\xfa\xc6\x6e\x5f\x50"
        "\x5c\x7e\xc7\x07\xf6\xd9\xa2\x77\x97\x1d\x7f\x22\x74\x76\x57\xc3\xb3"
        "\xe3\xe9\x0f\x70\xfc\x2e\x7f\x59\xfe\x1f\x8a\x4a\xfe\x1f\x8a\x4b\xfe"
        "\x1f\x8a\x4b\xfe\x1f\x8a\xab\xdd\x4e\xec\xf9\x0f\x00\x05\x23\xc7\x0f"
        "\xbc\xc3\xfb\xff\xbb\x79\xff\x1f\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x02\x1b"
        "\xcb\x4b\x92\x96\xba\x7b\x81\x8f\x45\x9a\x96\x4a\x11\x1f\x47\xc4\x44"
        "\x0c\x25\x0f\x17\x6a\xd5\xc9\x88\xf8\x24\x22\xfe\x3c\x3c\x34\x9c\xd5"
        "\xa7\xfa\xdd\x69\x00\xe0\x3d\xa5\x7f\x4f\xba\xfb\x7f\x5d\x1e\xbf\x38"
        "\xb6\xb7\xf5\x4c\xf2\xdf\xe1\xfc\x18\x11\x3f\xfb\xd5\xbd\x5f\x3e\x9e"
        "\x69\xb5\x56\xa6\xb2\xeb\xff\xda\xbe\xde\x7a\xd6\xbd\x5e\xe9\x47\xff"
        "\x01\x80\xc3\x6c\xad\xd3\x5b\xeb\xf8\x96\xd7\xaf\x9e\xcc\x6e\x95\x93"
        "\xec\xcf\xcb\xef\x76\x36\x17\xcd\xe2\x6e\x76\x4b\xa7\x65\x30\x06\xf3"
        "\xe3\x48\x0c\x45\xc4\xe8\xbf\x93\x6e\xbd\x23\x7b\xbd\x32\xf0\x01\xe2"
        "\x6f\x3c\x8d\x88\x2f\xed\x37\xfe\x24\xcf\x8d\x4c\x74\x77\x3e\xdd\x1b"
        "\x3f\x8b\xfd\xf1\x89\xc6\x4f\x77\xc5\x4f\xf3\xb6\xce\x31\xfb\x59\x7c"
        "\xf1\xdd\x43\xf7\xdc\xd2\x15\x8a\xe2\x79\x36\xff\xdc\xde\xef\xfe\x4b"
        "\xe3\x7c\x7e\xdc\xff\xfe\x1f\xc9\x67\xa8\xf7\xb7\x35\xff\x6d\xbe\x35"
        "\xff\xa5\xdb\xf3\xdf\x40\x8f\xf9\xef\xfc\x51\x63\x5c\xff\xe3\x0f\x7a"
        "\xb6\x3d\x8d\xf8\xca\xe0\x7e\xf1\x93\xed\xf8\x49\x8f\xf8\x17\x8f\x18"
        "\xff\xaf\x5f\xfd\xfa\x85\x5e\x6d\xed\x5f\x47\x5c\x8e\xfd\xe3\xef\x8c"
        "\x55\x6e\xd5\x97\xcb\xcd\xb5\xf5\xab\x0b\xf5\x99\xf9\xea\x7c\x75\xa9"
        "\x52\x99\x9e\x9a\x9e\xbc\x79\xed\x46\xa5\x9c\xe7\xa8\xcb\x5b\x99\xea"
        "\xb7\xfd\xe3\xd6\x95\x4f\x0e\x1a\xff\x68\x8f\xf8\x23\x87\x8c\xff\xd2"
        "\x11\xc7\xff\x9b\xff\x3d\xf8\xc9\x37\x0e\x88\xff\xed\x6f\xee\xff\xfc"
        "\x7f\x76\x40\xfc\x6c\x4d\xfc\xd6\x11\xe3\xcf\x8c\xfe\xae\xe7\x5c\x9f"
        "\xc5\x9f\xeb\x31\xfe\xc3\x9e\xff\x2b\x47\x8c\xff\xe2\x6f\xeb\x73\x47"
        "\x7c\x28\x00\x70\x02\x9a\x6b\xeb\x8b\x33\xb5\x5a\x75\xc5\x89\x13\x27"
        "\x4e\xb6\x4f\xfa\x3d\x33\x01\xc7\xed\xcd\x4d\xdf\xef\x9e\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\xbd\x9c\xc4\xbf\x13\xf5\x7b\x8c"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x9c\x5e"
        "\xff\x0f\x00\x00\xff\xff\x9a\x4c\xd8\xf7",
        1200);
    syz_mount_image(0x200004c0, 0x20000040, 0, 0x20000280, 1, 0x4ae,
                    0x20000540);
    break;
  case 1:
    memcpy((void*)0x200004c0, "ext4\000", 5);
    memcpy((void*)0x20000040, "./file0\000", 8);
    memcpy((void*)0x20000000, "quota", 5);
    *(uint8_t*)0x20000005 = 0x2c;
    memcpy((void*)0x20000006, "jqfmt=vfsv0", 11);
    *(uint8_t*)0x20000011 = 0x2c;
    memcpy((void*)0x20000012, "nouid32", 7);
    *(uint8_t*)0x20000019 = 0x2c;
    *(uint8_t*)0x2000001a = 0;
    memcpy(
        (void*)0x20000540,
        "\x78\x9c\xec\xdd\xc1\x4f\x5c\x5b\x19\x00\xf0\xef\x5e\xa0\x50\x1e\xef"
        "\x41\x63\x17\x6a\xd4\xd6\x5a\xad\xa6\xe9\x0c\x4c\x5b\xd2\x74\x55\x37"
        "\x1a\xd3\x34\x31\x36\xae\x5c\xb4\x08\x53\x42\x98\x61\x08\x33\xd4\x82"
        "\x5d\xd0\xff\xc1\xc4\x26\xae\xf4\x4f\x70\x61\xe2\xc2\xa4\x2b\xf7\xee"
        "\x74\xe7\xa6\x2e\x4c\xaa\x36\x9a\x62\xe2\x62\xcc\xbd\x33\x50\xa0\x0c"
        "\xd0\xd7\xc2\x24\xdc\xdf\x2f\x39\xb9\xf7\xdc\x33\x9d\xef\x9c\x4e\xee"
        "\x39\xc3\x37\x30\x27\x80\xc2\xba\x18\x11\x1b\x11\x71\x26\x22\x1e\x46"
        "\xc4\x78\xf7\x7a\xd2\x2d\x71\xa7\x53\xb2\xc7\xbd\x79\xfd\x74\x76\xf3"
        "\xf5\xd3\xd9\x24\xda\xed\xfb\xff\x4c\xf2\xf6\xec\x5a\xec\xf8\x37\x99"
        "\x4f\xba\xcf\x39\x12\x11\x3f\xfa\x7e\xc4\x4f\x93\x77\xe3\x36\xd7\xd6"
        "\x17\x67\x6a\xb5\xea\x4a\xb7\x5e\x6e\xd5\x97\xcb\xcd\xb5\xf5\x6b\x0b"
        "\xf5\x99\xf9\xea\x7c\x75\xa9\x52\x99\x9e\x9a\x9e\xbc\x75\xfd\x66\xe5"
        "\xa3\x8d\xf5\x42\xfd\xb7\xaf\xbe\xb7\x70\xf7\xc7\x7f\xf8\xfd\x57\x5f"
        "\xfe\x69\xe3\x3b\x3f\xcf\xba\x35\xd6\x6d\xdb\x39\x8e\x8f\xa9\x33\xf4"
        "\xa1\xed\x38\x99\xc1\x88\xb8\x7b\x1c\xc1\xfa\x60\xa0\x3b\x9e\x33\xfd"
        "\xee\x08\x9f\x4b\x1a\x11\xa5\x88\xb8\x94\xdf\xff\xe3\x31\x90\xbf\x9a"
        "\x00\xc0\x69\xd6\x6e\x8f\x47\x7b\x7c\x67\x1d\x00\x38\xed\xd2\x3c\x07"
        "\x96\xa4\xa5\x6e\x2e\x60\x2c\xd2\xb4\x54\xea\xe4\xf0\xce\xc7\x68\x5a"
        "\x6b\x34\x5b\x57\x1f\x35\x56\x97\xe6\x3a\xb9\xb2\x89\x18\x4a\x1f\x2d"
        "\xd4\xaa\x93\xdd\x5c\xe1\x44\x0c\x25\x59\x7d\x2a\x3f\x7f\x5b\xaf\xec"
        "\xa9\x5f\x8f\x88\x73\x11\xf1\x8b\xe1\xb3\x79\xbd\x34\xdb\xa8\xcd\xf5"
        "\xf3\x8d\x0f\x00\x14\xd8\x27\x7b\xd6\xff\xff\x0c\x77\xd6\x7f\x00\xe0"
        "\x94\x1b\xe9\x77\x07\x00\x80\x13\x67\xfd\x07\x80\xe2\xb1\xfe\x03\x40"
        "\xf1\x58\xff\x01\xa0\x78\xac\xff\x00\x50\x3c\xd6\x7f\x00\x28\x1e\xeb"
        "\x3f\x00\x14\xca\x0f\xef\xdd\xcb\x4a\x7b\xb3\xfb\xfd\xd7\x73\x8f\xd7"
        "\x56\x17\x1b\x8f\xaf\xcd\x55\x9b\x8b\xa5\xfa\xea\x6c\x69\xb6\xb1\xb2"
        "\x5c\x9a\x6f\x34\xe6\xf3\xef\xec\xa9\x1f\xf6\x7c\xb5\x46\x63\x79\xea"
        "\x46\xac\x3e\x29\xb7\xaa\xcd\x56\xb9\xb9\xb6\xfe\xa0\xde\x58\x5d\x6a"
        "\x3d\xc8\xbf\xd7\xfb\x41\x75\xe8\x44\x46\x05\x00\x1c\xe4\xdc\x85\x17"
        "\x7f\x49\x22\x62\xe3\xf6\xd9\xbc\xc4\x8e\xbd\x1c\xac\xd5\x70\xba\xa5"
        "\xfd\xee\x00\xd0\x37\x03\xfd\xee\x00\xd0\x37\x76\xfb\x82\xe2\xf2\x33"
        "\x3e\xb0\xcf\x16\xbd\xbb\xec\xf8\x15\xa1\xb3\xbb\x1a\x9e\x1f\x4f\x7f"
        "\x80\xe3\x77\xe5\x4b\xf2\xff\x50\x54\xf2\xff\x50\x5c\xf2\xff\x50\x5c"
        "\xf2\xff\x50\x5c\xed\x76\x62\xcf\x7f\x00\x28\x18\x39\x7e\xe0\x3d\x3e"
        "\xff\xdf\xcd\xe7\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x14\xd8\x58\x5e\x92"
        "\xb4\xd4\xdd\x0b\x7c\x2c\xd2\xb4\x54\x8a\xf8\x34\x22\x26\x62\x28\x79"
        "\xb4\x50\xab\x4e\x46\xc4\x67\x11\xf1\xe7\xe1\xa1\xe1\xac\x3e\xd5\xef"
        "\x4e\x03\x00\x1f\x28\xfd\x7b\xd2\xdd\xff\xeb\xca\xf8\xe5\xb1\xbd\xad"
        "\x67\x92\xff\x0e\xe7\xc7\x88\xf8\xd9\xaf\xee\xff\xf2\xc9\x4c\xab\xb5"
        "\x32\x95\x5d\xff\xd7\xf6\xf5\xd6\xf3\xee\xf5\x4a\x3f\xfa\x0f\x00\x1c"
        "\x66\x6b\x9d\xde\x5a\xc7\xb7\xbc\x79\xfd\x74\x76\xab\x9c\x64\x7f\x5e"
        "\x7d\xb7\xb3\xb9\x68\x16\x77\xb3\x5b\x3a\x2d\x83\x31\x98\x1f\x47\x62"
        "\x28\x22\x46\xff\x9d\x74\xeb\x1d\xd9\xfb\x95\x81\x8f\x10\x7f\xe3\x59"
        "\x44\x7c\x71\xbf\xf1\x27\x79\x6e\x64\xa2\xbb\xf3\xe9\xde\xf8\x59\xec"
        "\x4f\x4f\x34\x7e\xba\x2b\x7e\x9a\xb7\x75\x8e\xd9\xff\xc5\x17\xde\x3f"
        "\x74\xcf\x2d\x5d\xa1\x28\x5e\x64\xf3\xcf\x9d\xfd\xee\xbf\x34\x2e\xe6"
        "\xc7\xfd\xef\xff\x91\x7c\x86\xfa\x70\x5b\xf3\xdf\xe6\x3b\xf3\x5f\xba"
        "\x3d\xff\x0d\xf4\x98\xff\x2e\x1e\x35\xc6\x8d\x3f\xfe\xa0\x67\xdb\xb3"
        "\x88\x2f\x0f\xee\x17\x3f\xd9\x8e\x9f\xf4\x88\x7f\xf9\x88\xf1\xff\xfa"
        "\x95\xaf\x5d\xea\xd5\xd6\xfe\x75\xc4\x95\xd8\x3f\xfe\xce\x58\xe5\x56"
        "\x7d\xb9\xdc\x5c\x5b\xbf\xb6\x50\x9f\x99\xaf\xce\x57\x97\x2a\x95\xe9"
        "\xa9\xe9\xc9\x5b\xd7\x6f\x56\xca\x79\x8e\xba\xbc\x95\xa9\x7e\xd7\x3f"
        "\x6e\x5f\xfd\xec\xa0\xf1\x8f\xf6\x88\x3f\x72\xc8\xf8\xbf\x79\xc4\xf1"
        "\xff\xe6\x7f\x0f\x7f\xf2\xf5\x03\xe2\x7f\xfb\x1b\xfb\xbf\xfe\xe7\x0f"
        "\x88\x9f\xad\x89\xdf\x3a\x62\xfc\x99\xd1\xdf\xf5\x9c\xeb\xb3\xf8\x73"
        "\x3d\xc6\x7f\xd8\xeb\x7f\xf5\x88\xf1\x5f\xfe\x6d\x7d\xee\x88\x0f\x05"
        "\x00\x4e\x40\x73\x6d\x7d\x71\xa6\x56\xab\xae\x38\x71\xe2\xc4\xc9\xf6"
        "\x49\xbf\x67\x26\xe0\xb8\xbd\xbd\xe9\xfb\xdd\x13\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\xa0\x97\x93\xf8\x73\xa2\x7e\x8f\x11\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\xd3\xeb\xff\x01"
        "\x00\x00\xff\xff\x68\x36\xd9\x6e",
        1198);
    syz_mount_image(0x200004c0, 0x20000040, 0, 0x20000000, 1, 0x4ae,
                    0x20000540);
    break;
  case 2:
    memcpy((void*)0x20000080, "./file0\000", 8);
    memcpy((void*)0x20000180, "trusted.overlay.upper\000", 22);
    syscall(__NR_setxattr, 0x20000080ul, 0x20000180ul, 0x200005c0ul, 0x2000ul,
            0ul);
    break;
  case 3:
    memcpy((void*)0x20000080, "./file0\000", 8);
    memcpy((void*)0x20000180, "trusted.overlay.upper\000", 22);
    syscall(__NR_setxattr, 0x20000080ul, 0x20000180ul, 0x200005c0ul, 0x2000ul,
            0ul);
    break;
  case 4:
    memcpy((void*)0x200000c0, "./file0/file0\000", 14);
    syscall(__NR_openat, 0xffffffffffffff9cul, 0x200000c0ul, 0x28200ul, 0x2dul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  use_temporary_dir();
  loop();
  return 0;
}
