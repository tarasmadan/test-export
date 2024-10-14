// https://syzkaller.appspot.com/bug?id=f086f5c4765bea86799450cd3a025207fa53f82d
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
  for (call = 0; call < 7; call++) {
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    memcpy((void*)0x20000a40, "nilfs2\000", 7);
    memcpy((void*)0x20000a80, "./file0\000", 8);
    memcpy(
        (void*)0x20001540,
        "\x78\x9c\xec\xdd\x4f\x8c\x5b\x47\xfd\x00\xf0\xb1\x77\xbd\xf9\xdb\x5f"
        "\x9c\xfe\x12\xba\xa4\xa1\x4d\x28\xb4\xe5\x4f\x77\x9b\xcd\x12\xfe\x44"
        "\xd0\x54\xcd\x85\xa8\xa9\xb8\x55\xaa\xb8\x44\x69\x5a\x22\xd2\x80\x48"
        "\x25\x68\xd5\x43\x92\x13\x37\x5a\x55\xe1\xca\x1f\x71\xea\xa5\x02\x84"
        "\x44\x2f\x28\xea\x89\x4b\x25\x1a\x89\x4b\x4f\x85\x03\x07\xa2\x20\x55"
        "\xe2\x00\x85\x64\x51\xbc\x33\x5e\xfb\x1b\x5b\xcf\xbb\xd9\x5d\xaf\xd7"
        "\x9f\x8f\x34\x3b\x9e\x37\x63\xcf\x3c\xef\xf3\xf3\xf3\x7b\x6f\x66\x12"
        "\x30\xb6\xea\xad\xbf\xf3\xf3\xd3\xb5\x94\xae\xbc\xf3\xe6\xf1\xbf\x3f"
        "\xfc\xb7\x6d\xb7\x97\x3c\xd1\x2e\xd1\x6c\xfd\x9d\xec\x48\x35\x52\x4a"
        "\xb5\x9c\x9e\x0c\xaf\xf7\xe1\xc4\x62\x7c\xf3\xa3\xd7\x4e\xf7\x8a\x6b"
        "\x69\xae\xf5\xb7\xa4\xd3\x33\x37\xda\xcf\xdd\x91\x52\xba\x98\x0e\xa4"
        "\xab\xa9\x99\xf6\x5d\xb9\xf6\xc6\x7b\x73\x4f\x9f\xbc\x74\xe2\xf2\xc1"
        "\xf7\xdf\x3a\x7a\x7d\x6d\xd6\x1e\x00\x00\xc6\xcb\xb7\xae\x1e\x9d\xdf"
        "\xfb\x97\x3f\xdd\xbf\xfb\xe3\xb7\x1f\x38\x96\xb6\xb4\x97\x97\xe3\xf3"
        "\x66\x4e\xef\xcc\xc7\xfd\xc7\xf2\x81\x7f\x39\xfe\xaf\xa7\xee\x74\xad"
        "\x23\x74\x9a\x0a\xe5\x26\x73\xa8\x87\x72\x13\x3d\xca\x75\xd6\xd3\x08"
        "\xe5\x26\xfb\xd4\x3f\x15\x5e\xb7\xd1\xa7\xdc\x96\x8a\xfa\x27\x3a\x96"
        "\xf5\x5a\x6f\x18\x65\x65\x3b\x6e\xa6\x5a\x7d\xa6\x2b\x5d\xaf\xcf\xcc"
        "\x2c\xfe\x26\x4f\xad\xdf\xf5\x53\xb5\x99\xf3\x67\xcf\xbd\x70\x61\x48"
        "\x0d\x05\x56\xdd\x3f\x1f\x4c\x29\x1d\x10\x84\x0d\x1c\x8e\x6d\x80\x36"
        "\x6c\xd2\xb0\xb0\x6b\xd8\x7b\x20\x80\x45\xf1\x7a\xe1\x1d\x2e\xc6\x33"
        "\x0b\x77\xa7\xfd\x6a\x93\x83\xd5\x7f\xe3\xc9\x7a\xef\xe7\xc3\x2a\x58"
        "\xef\xed\x5f\xfd\xa3\x55\xff\xaf\x2e\xd9\xe3\xb0\x7a\x36\xeb\xd6\x54"
        "\xd6\xab\x7c\x8e\x76\xe6\x74\xbc\x8e\x10\xef\x5f\xea\xff\xf9\x8b\x57"
        "\x3a\xba\x97\xc6\xeb\x11\x8d\x01\xdb\xd9\xef\x3a\xc2\xa8\x5c\x5f\xe8"
        "\xd7\xce\x89\x75\x6e\xc7\x4a\xf5\x6b\x7f\xdc\x2e\x36\xab\xaf\xe7\xb8"
        "\xbc\x0f\xdf\x08\xf9\x9d\x9f\x9f\xf8\x3f\x1d\x95\xff\x31\xd0\xdb\xbf"
        "\x9c\xff\x17\x84\xb1\x0d\x0b\xc3\xde\x01\x01\x1b\x56\xbc\x6f\x6e\x21"
        "\x2b\xf9\xf1\xbe\xbe\x98\xbf\xa5\x22\x7f\x6b\x45\xfe\xb6\x8a\xfc\xed"
        "\x15\xf9\x3b\x2a\xf2\x61\x9c\xfd\xf6\xe5\x9f\xa4\xd7\x6b\x4b\xbf\xf3"
        "\xe3\x6f\xfa\xe5\x9e\x0f\x2f\xe7\xd9\xee\xc9\xf1\xff\x2d\xb3\x3d\xf1"
        "\x7c\xe4\x72\xeb\x8f\xf7\xfd\x2e\xd7\xdd\xd6\x1f\xef\x27\x86\x8d\xec"
        "\xf7\xa7\x9e\x3d\xf3\x95\xe7\x9f\xbb\xb6\x78\xff\x7f\xad\xbd\xfd\xdf"
        "\xca\xdb\xfb\x81\x9c\x6e\xe6\xcf\xd6\xd5\x5c\xa0\x9c\x2f\x8c\xe7\xd5"
        "\xdb\xf7\xfe\x37\xbb\xeb\xa9\xf7\x29\x77\x6f\x68\xcf\x3d\x3d\xca\xb7"
        "\x1e\xef\xe9\x2e\x57\xdb\xb3\xf4\x3a\xa9\x63\x3f\x73\x47\x3b\xa6\xbb"
        "\x9f\xb7\xab\x5f\xb9\xfd\xdd\xe5\x9a\xa1\xdc\xb6\x1c\xb6\x86\xf6\xc6"
        "\xe3\x93\xed\xe1\x79\xe5\xf8\xa3\xec\x57\xcb\xfb\x35\x19\xd6\xb7\x11"
        "\xd6\x63\x2a\xb4\xa3\xec\x57\x76\xe7\x38\xb6\x03\x56\xa2\x6c\x8f\xfd"
        "\xee\xff\x2f\xdb\xe7\x74\x6a\xd4\x5e\x38\x7b\xee\xcc\xe3\x39\x5d\xb6"
        "\xd3\x3f\x4e\x34\xb6\xdc\x5e\x7e\x68\x9d\xdb\x0d\xdc\xbd\x41\xfb\xff"
        "\x4c\xa7\xee\xfe\x3f\x3b\xdb\xcb\x1b\xf5\xce\xfd\xc2\xae\xa5\xe5\xb5"
        "\xce\xfd\x42\x33\x2c\x9f\xeb\xb3\xfc\x70\x4e\x97\xef\xb9\xef\x4c\x6c"
        "\x6b\x2d\x9f\x39\xfd\xbd\x73\xcf\xaf\xf6\xca\xc3\x98\xbb\xf0\xca\xab"
        "\xdf\x3d\x75\xee\xdc\x99\x1f\x78\xe0\x81\x07\x1e\xb4\x1f\x0c\x7b\xcf"
        "\x04\xac\xb5\xd9\x97\x5f\xfa\xfe\xec\x85\x57\x5e\x7d\xec\xec\x4b\xa7"
        "\x5e\x3c\xf3\xe2\x99\xf3\x87\x8f\x1c\x39\x3c\x37\x77\xe4\xab\x87\xe7"
        "\x67\x5b\xc7\xf5\xb3\x9d\x47\xf7\xc0\x66\xb2\xf4\xa5\x3f\xec\x96\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x83\xfa\xe1\x89\xe3\xd7\xfe\xfc"
        "\xee\x97\x3f\x58\xec\xff\xbf\xd4\xff\xaf\xf4\xff\x2f\x77\xfe\x96\xfe"
        "\xff\x3f\x0e\xfd\xff\x63\x3f\xf9\xd2\x0f\xbe\xf4\x03\xdc\xdd\x23\xbf"
        "\x55\x26\x0c\xb0\x3a\x15\xca\x35\x72\xf8\xff\xd0\xde\x3d\xa1\x9e\xbd"
        "\xe1\x79\x9f\xc8\x71\x7b\x1e\xbf\xdc\xff\xbf\x54\x17\xc7\x75\x2d\xed"
        "\xb9\x2f\x2c\x8f\xe3\xf7\x96\x72\x61\x38\x81\x3b\xc6\x4b\x99\x0a\x63"
        "\x90\xc4\xf9\x02\x3f\x9d\xe3\xcb\x39\xfe\x65\x82\x21\xaa\x6d\xeb\xbd"
        "\x38\xc7\x55\xe3\x5b\x97\x6d\xbd\x8c\x4f\x61\x5c\x8a\xd1\x54\xfe\x6f"
        "\x65\x6b\x28\xe3\x98\x94\xfe\xdf\x3d\xc7\x75\xea\xf8\x67\xef\x5e\x87"
        "\x36\xb2\xfa\xd6\xa3\x3b\xe1\xb0\xd7\x11\xe8\xed\x1f\xc6\xff\x16\x84"
        "\xb1\x0d\x0b\x0b\x66\xf1\x00\x36\x86\x61\xcf\xff\x59\xce\x7b\x96\xf8"
        "\xfc\x1f\xbe\xb9\xf5\x76\x28\xc5\x6e\x3c\xd9\xbd\xbf\x8c\xe3\x97\xc2"
        "\xdd\xd8\xe8\xf3\x4f\xaa\x7f\x73\xcd\xff\xd9\x9e\xff\x6e\xe0\xfd\x5f"
        "\x98\x31\xaf\xb9\xb2\x7a\xff\xfd\xb3\xeb\x1f\x74\x54\x9b\xf6\x0d\x5a"
        "\x7f\x5c\xff\x32\x0e\xf4\x9e\xe5\xd5\xff\x71\xae\xbf\xac\xcd\x23\x69"
        "\xb0\xfa\x17\x7e\x11\xea\x8f\x17\x84\x06\xf4\x9f\x50\xff\xf6\x01\xeb"
        "\xbf\x63\xfd\xf7\xaf\xac\xfe\xff\xe6\xfa\xcb\xdb\xf6\xe8\x43\x83\xd6"
        "\xbf\xd8\xe2\x5a\xbd\xbb\x1d\xf1\xbc\x71\xb9\xfe\x17\xcf\x1b\x17\x37"
        "\xc3\xfa\x97\xb1\x3d\x97\xbd\xfe\x2b\x9c\xa8\xf1\x56\xae\x1f\xc6\xd9"
        "\xa8\xcc\x33\xbb\x5c\xab\x3f\xff\x6f\x76\x71\x75\xe7\xff\xed\x27\xde"
        "\x87\xf1\xa5\x9c\x2e\x3b\xc2\x72\x9f\x43\x9c\xef\x64\xb9\xed\x2f\xf7"
        "\x57\x94\xef\x81\xbd\xe1\xf5\x6b\x15\xdf\x6f\xe6\xff\x1d\x6d\x5f\xcb"
        "\x71\xd5\xe7\xa1\xcc\xff\x5b\xb6\xc7\x66\xfe\xca\xef\x48\xb7\xde\xcb"
        "\x92\x6e\xf4\x78\x6f\x37\xeb\xbe\x06\x46\xd5\x87\xae\xff\x09\xc2\xba"
        "\x87\xf6\x3c\x71\x43\x6e\xc7\xc2\xc2\xc2\xda\x9e\xd0\xaa\x30\xd4\xca"
        "\x19\xfa\xfb\x3f\xec\xdf\x09\xc3\xae\x7f\xd8\xef\x7f\x95\x38\xff\x6f"
        "\x3c\x86\x8f\xf3\xff\xc6\xfc\x38\xff\x6f\xcc\x8f\xf3\xff\xc6\xfc\x38"
        "\xbf\x5e\xcc\x8f\xf3\xff\xc6\xf7\x33\xce\xff\x1b\xf3\xef\x0b\xaf\x1b"
        "\xe7\x07\x9e\xae\xc8\xff\x64\x45\xfe\xbe\xde\xf9\xed\x9f\xed\xf7\x57"
        "\x3c\x7f\x7f\x45\xfe\xa7\x2a\xf2\x0f\x56\xe4\x3f\x50\x91\xff\x60\x45"
        "\xfe\xbd\x15\xf9\x0f\x55\xe4\x7f\xa6\x22\xff\xb3\x15\xf9\x0f\x57\xe4"
        "\x3f\x5a\x91\xff\xb9\x8a\xfc\xcd\xae\xf4\x47\x19\xd7\xf5\x87\x71\x16"
        "\xfb\xe7\xf9\xfc\xc3\xf8\x28\xd7\x7f\xfa\x7d\xfe\xf7\x54\xe4\x03\xa3"
        "\xeb\xa7\x6f\x1f\x7a\xea\xb9\xdf\x7c\xbb\xb9\xd8\xff\x7f\xaa\x7d\x3e"
        "\xa4\x5c\xc7\x3b\x96\xd3\x8d\xfc\xdb\xf9\x47\x39\x1d\xaf\x7b\xa7\x8e"
        "\xf4\xed\xbc\x77\x73\xfa\xaf\x21\x7f\xa3\x9f\xef\x80\x71\x12\xc7\xcf"
        "\x88\xdf\xef\x8f\x54\xe4\x03\xa3\xab\xdc\xe7\xe5\xf3\x0d\x63\xa8\xd6"
        "\x7b\xc4\x9e\x41\xc7\xad\xea\x77\x9c\xcf\x68\xf9\x7c\x8e\xbf\x90\xe3"
        "\x2f\xe6\xf8\xb1\x1c\xcf\xe4\x78\x36\xc7\x87\x72\x3c\xb7\x4e\xed\x63"
        "\x6d\x3c\xf5\xeb\xdf\x1d\x7d\xbd\xb6\xf4\x7b\x7f\x57\xc8\x1f\xf4\x7e"
        "\xf2\xd8\x1f\xa8\x6b\x9c\xa8\x94\xd2\xe1\x01\xdb\x13\xcf\x0f\x2c\xf7"
        "\x7e\xf6\x38\x8e\xdf\x72\xdd\x6d\xfd\x2b\xec\x0e\x06\x00\x00\x00\x00"
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
        "\x00\x00\x00\x30\x34\xf5\xd6\xdf\xf9\xf9\xe9\x5a\x4a\x57\xde\x79\xf3"
        "\xf8\xb3\x27\xcf\xce\xde\x5e\xf2\x44\xbb\x44\xb3\xf5\x77\xb2\x23\xd5"
        "\x68\x3f\x2f\xa5\xc7\x73\x3c\x91\xe3\x9f\xe7\x07\x37\x3f\x7a\xed\x74"
        "\x67\x7c\x2b\xc7\xb5\x34\x97\x6a\xa9\xd6\x5e\x9e\x9e\xb9\xd1\xae\x69"
        "\x47\x4a\xe9\x62\x3a\x90\xae\xa6\x66\xda\x77\xe5\xda\x1b\xef\xcd\x3d"
        "\x7d\xf2\xd2\x89\xcb\x07\xdf\x7f\xeb\xe8\xf5\xb5\x7b\x07\x00\x00\x00"
        "\x60\xf3\xfb\x5f\x00\x00\x00\xff\xff\x25\xf2\x0c\x0b",
        2597);
    syz_mount_image(/*fs=*/0x20000a40, /*dir=*/0x20000a80, /*flags=*/0x2000008,
                    /*opts=*/0x20000080, /*chdir=*/1, /*size=*/0xa25,
                    /*img=*/0x20001540);
    break;
  case 1:
    memcpy((void*)0x200000c0, "./bus\000", 6);
    res = syscall(__NR_open, /*file=*/0x200000c0ul, /*flags=*/0x14da42ul,
                  /*mode=*/0ul);
    if (res != -1)
      r[0] = res;
    break;
  case 2:
    memcpy((void*)0x20000380, "/dev/loop", 9);
    *(uint8_t*)0x20000389 = 0x30 + procid * 1;
    *(uint8_t*)0x2000038a = 0;
    memcpy((void*)0x200003c0, "./bus\000", 6);
    syscall(__NR_mount, /*src=*/0x20000380ul, /*dst=*/0x200003c0ul,
            /*type=*/0ul, /*flags=*/0x1000ul, /*data=*/0ul);
    break;
  case 3:
    memcpy((void*)0x200000c0, "./bus\000", 6);
    res = syscall(__NR_open, /*file=*/0x200000c0ul, /*flags=*/0x14da42ul,
                  /*mode=*/0ul);
    if (res != -1)
      r[1] = res;
    break;
  case 4:
    memcpy((void*)0x20000040, "./bus\000", 6);
    res = syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000040ul,
                  /*flags=*/0ul, /*mode=*/0ul);
    if (res != -1)
      r[2] = res;
    break;
  case 5:
    syscall(__NR_sendfile, /*fdout=*/r[1], /*fdin=*/r[2], /*off=*/0ul,
            /*count=*/0x80001d00c0d0ul);
    break;
  case 6:
    *(uint32_t*)0x20000100 = 0xa0;
    *(uint32_t*)0x20000104 = 0;
    *(uint64_t*)0x20000108 = 0;
    *(uint64_t*)0x20000110 = 6;
    *(uint64_t*)0x20000118 = 1;
    *(uint64_t*)0x20000120 = 0xff;
    *(uint64_t*)0x20000128 = 0xeb;
    *(uint32_t*)0x20000130 = 1;
    *(uint32_t*)0x20000134 = 0x8001;
    *(uint64_t*)0x20000138 = 0;
    *(uint64_t*)0x20000140 = 0xaf8f;
    *(uint64_t*)0x20000148 = 0xffff;
    *(uint64_t*)0x20000150 = 0x101;
    *(uint64_t*)0x20000158 = 1;
    *(uint64_t*)0x20000160 = 0x1000;
    *(uint32_t*)0x20000168 = 0xc000;
    *(uint32_t*)0x2000016c = 0x43e;
    *(uint32_t*)0x20000170 = 2;
    *(uint32_t*)0x20000174 = 0x2000;
    *(uint32_t*)0x20000178 = 2;
    *(uint32_t*)0x2000017c = 0xee01;
    *(uint32_t*)0x20000180 = 0;
    *(uint32_t*)0x20000184 = 5;
    *(uint32_t*)0x20000188 = 2;
    *(uint32_t*)0x2000018c = 0;
    *(uint64_t*)0x20000190 = 0;
    *(uint32_t*)0x20000198 = 5;
    *(uint32_t*)0x2000019c = 0;
    syscall(__NR_write, /*fd=*/r[0], /*arg=*/0x20000100ul, /*len=*/0xa0ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul, /*prot=*/7ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      use_temporary_dir();
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
