// https://syzkaller.appspot.com/bug?id=fba1b3ae3426bf4d77e9b131e07d44e18ad0bbb5
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
retry:
  const int umount_flags = MNT_FORCE | UMOUNT_NOFOLLOW;
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

uint64_t r[1] = {0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    memcpy((void*)0x200000c0, "ext4\000", 5);
    memcpy((void*)0x20000500, "./bus\000", 6);
    memcpy((void*)0x20000040, "delalloc", 8);
    *(uint8_t*)0x20000048 = 0x2c;
    memcpy((void*)0x20000049, "data=ordered", 12);
    *(uint8_t*)0x20000055 = 0x2c;
    memcpy((void*)0x20000056, "block_validity", 14);
    *(uint8_t*)0x20000064 = 0x2c;
    *(uint8_t*)0x20000065 = 0;
    memcpy(
        (void*)0x20001000,
        "\x78\x9c\xec\xdd\x41\x6f\x1b\x59\x1d\x00\xf0\xff\x4c\x92\x26\x9b\xcd"
        "\x6e\xb2\xb0\x07\x40\xc0\x96\x65\xa1\xa0\xaa\x76\xe2\xee\x46\xab\x3d"
        "\x95\x0b\x08\xad\x56\x42\xac\x38\x71\x68\x43\xe2\x46\x51\xec\x38\x8a"
        "\x9d\xd2\x84\x1e\xd2\xef\x80\x44\x25\x4e\xf0\x11\x38\x20\x71\x40\xea"
        "\x89\x3b\x37\xb8\x71\x29\x07\xa4\x02\x15\xa8\x41\xe2\x60\x34\xe3\x49"
        "\x9a\x26\x76\x12\xd1\x34\xee\x66\x7e\x3f\xe9\x69\xe6\xbd\xe7\xf8\xff"
        "\x5e\xac\x79\x6f\xfc\x12\xfb\x05\x50\x5a\x97\x23\x62\x27\x22\x2e\x45"
        "\xc4\xad\x88\x98\x2e\xca\x93\x22\xc5\x8d\x5e\xca\x1e\xf7\xf4\xc9\xbd"
        "\xc5\xdd\x27\xf7\x16\x93\xe8\x76\x3f\xfd\x47\x92\xd7\x67\x65\x71\xe0"
        "\x67\x32\xaf\x17\xcf\x39\x11\x11\x3f\xfc\x5e\xc4\x4f\x92\xa3\x71\xdb"
        "\x5b\xdb\xab\x0b\x8d\x46\x7d\xa3\xc8\x57\x3b\xcd\xf5\x6a\x7b\x6b\xfb"
        "\xda\x4a\x73\x61\xb9\xbe\x5c\x5f\xab\xd5\xe6\xe7\xe6\x67\x3f\xbc\xfe"
        "\x41\xed\xcc\xfa\xfa\x4e\xf3\x37\x8f\xbf\xbb\xf2\xf1\x8f\x7e\xff\xbb"
        "\xaf\x3c\xfa\xe3\xce\xb7\x7f\x96\x35\x6b\xaa\xa8\x3b\xd8\x8f\xb3\xd4"
        "\xeb\xfa\xd8\x7e\x9c\xcc\x68\x44\x7c\xfc\x32\x82\x0d\xc1\x48\xd1\x9f"
        "\x4b\xc3\x6e\x08\xff\x97\x34\x22\x3e\x17\x11\xef\xe6\xd7\xff\x74\x8c"
        "\xe4\xaf\x26\x00\x70\x91\x75\xbb\xd3\xd1\x9d\x3e\x98\x07\x00\x2e\xba"
        "\x34\x5f\x03\x4b\xd2\x4a\xb1\x16\x30\x15\x69\x5a\xa9\xf4\xd6\xf0\xde"
        "\x8e\xc9\xb4\xd1\x6a\x77\xae\xde\x6e\x6d\xae\x2d\xf5\xd6\xca\x66\x62"
        "\x2c\xbd\xbd\xd2\xa8\xcf\x16\x6b\x85\x33\x31\x96\x64\xf9\xb9\xfc\xfc"
        "\x59\xbe\x76\x28\x7f\x3d\x22\xde\x8a\x88\x9f\x8f\xbf\x96\xe7\x2b\x8b"
        "\xad\xc6\xd2\x30\x6f\x7c\x00\xa0\xc4\x5e\x3f\x34\xff\xff\x7b\xbc\x37"
        "\xff\x03\x00\x17\xdc\xc4\xb0\x1b\x00\x00\x9c\x3b\xf3\x3f\x00\x94\x8f"
        "\xf9\x1f\x00\xca\xc7\xfc\x0f\x00\xe5\x63\xfe\x07\x80\xf2\x31\xff\x03"
        "\x40\xf9\x98\xff\x01\xa0\x54\x7e\xf0\xc9\x27\x59\xea\xee\x16\xdf\x7f"
        "\xbd\x74\x67\x6b\x73\xb5\x75\xe7\xda\x52\xbd\xbd\x5a\x69\x6e\x2e\x56"
        "\x16\x5b\x1b\xeb\x95\xe5\x56\x6b\x39\xff\xce\x9e\xe6\x49\xcf\xd7\x68"
        "\xb5\xd6\xe7\xde\x8f\xcd\xbb\xd5\x4e\xbd\xdd\xa9\xb6\xb7\xb6\x6f\x36"
        "\x5b\x9b\x6b\x9d\x9b\xf9\xf7\x7a\xdf\xac\x8f\x9d\x4b\xaf\x00\x80\xe3"
        "\xbc\xf5\xce\xc3\x3f\x27\x11\xb1\xf3\xd1\x6b\x79\x8a\x03\x7b\x39\x98"
        "\xab\xe1\x62\x4b\x87\xdd\x00\x60\x68\x46\x86\xdd\x00\x60\x68\xec\xf6"
        "\x05\xe5\xe5\x3d\x3e\xd0\x67\x8b\xde\xe7\x0c\xfc\x17\xa1\x07\x47\x4a"
        "\xbc\xa5\x80\xcf\x88\x2b\x5f\xb4\xfe\x0f\x65\x65\xfd\x1f\xca\xcb\xcd"
        "\x3a\x94\x97\xf5\x7f\x28\xaf\x6e\x37\xb1\xe7\x3f\x00\x94\x8c\x35\x7e"
        "\xe0\x0c\xff\xfe\x0f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xa5\x31\x95\xa7\x24"
        "\xad\x14\x7b\x81\x4f\x45\x9a\x56\x2a\x11\x6f\x44\xc4\x4c\x8c\x25\xb7"
        "\x57\x1a\xf5\xd9\x88\x78\x33\x22\xfe\x34\x3e\x36\x9e\xe5\xe7\x86\xdd"
        "\x68\x00\xe0\x05\xa5\x7f\x4b\x8a\xfd\xbf\xae\x4c\xbf\x37\x75\xb8\xf6"
        "\x52\xf2\x9f\xf1\xfc\x18\x11\x3f\xfd\xe5\xa7\xbf\xb8\xbb\xd0\xe9\x6c"
        "\xcc\x65\xe5\xff\xdc\x2f\xef\x3c\x28\xca\x6b\xc3\x68\x3f\x00\x70\x92"
        "\xbd\x79\x7a\x6f\x1e\xdf\xf3\xf4\xc9\xbd\xc5\xbd\x74\x9e\xed\x79\xfc"
        "\x9d\xde\xe6\xa2\x59\xdc\xdd\x22\xf5\x6a\x46\x63\x34\x3f\x4e\xc4\x58"
        "\x44\x4c\xfe\x2b\x29\xf2\x3d\xd9\xfd\xca\xc8\x19\xc4\xdf\xb9\x1f\x11"
        "\x5f\xe8\xd7\xff\x24\x5f\x1b\x99\x29\x76\x3e\x3d\x1c\x3f\x8b\xfd\xc6"
        "\xb9\xc6\x4f\x9f\x8b\x9f\xe6\x75\xbd\x63\xf6\xbb\xf8\xfc\x19\xb4\x05"
        "\xca\xe6\x61\x36\xfe\xdc\xe8\x77\xfd\xa5\x71\x39\x3f\xf6\xbf\xfe\x27"
        "\xf2\x11\xea\xc5\xed\x8d\x7f\xbb\x47\xc6\xbf\x74\x7f\xfc\x1b\x19\x30"
        "\xfe\x5d\x3e\x6d\x8c\xf7\xff\xf0\xfd\x81\x75\xf7\x23\xbe\x34\xda\x2f"
        "\x7e\xb2\x1f\x3f\x19\x10\xff\xbd\x53\xc6\xff\xcb\x97\xbf\xfa\xee\xa0"
        "\xba\xee\xaf\x22\xae\x44\xff\xf8\x07\x63\x55\x3b\xcd\xf5\x6a\x7b\x6b"
        "\xfb\xda\x4a\x73\x61\xb9\xbe\x5c\x5f\xab\xd5\xe6\xe7\xe6\x67\x3f\xbc"
        "\xfe\x41\xad\x9a\xaf\x51\x57\xf7\x56\xaa\x8f\xfa\xfb\x47\x57\xdf\x3c"
        "\xae\xff\x93\x03\xe2\x4f\x9c\xd0\xff\x6f\x9c\xb2\xff\xbf\xfe\xef\xad"
        "\x1f\x7f\xed\x98\xf8\xdf\xfa\x7a\xff\xd7\xff\xed\x63\xe2\x67\x73\xe2"
        "\x37\x4f\x19\x7f\x61\xf2\xb7\x03\xb7\xef\xce\xe2\x2f\x0d\xe8\xff\x49"
        "\xaf\xff\xd5\x53\xc6\x7f\xf4\xd7\xed\xa5\x53\x3e\x14\x00\x38\x07\xed"
        "\xad\xed\xd5\x85\x46\xa3\xbe\xd1\xe7\xe4\xc6\xe0\x2a\x27\x67\x70\x92"
        "\xdd\x43\xbd\x02\xcd\x28\xdf\x49\x32\xfa\x4a\x34\xe3\xd5\x3e\x19\xf6"
        "\xc8\x04\xbc\x6c\xcf\x2e\xfa\x61\xb7\x04\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x18\x64\xd0\xa7\x7f\xba\x85\xb3\xf8\x38\xd1\xb0\xfb"
        "\x08\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xc0\xc5"
        "\xf5\xbf\x00\x00\x00\xff\xff\xb9\x22\xda\xef",
        1218);
    syz_mount_image(/*fs=*/0x200000c0, /*dir=*/0x20000500,
                    /*flags=MS_NODEV|MS_DIRSYNC*/ 0x84, /*opts=*/0x20000040,
                    /*chdir=*/0x21, /*size=*/0x4c2, /*img=*/0x20001000);
    break;
  case 1:
    memcpy((void*)0x20000340, "./bus\000", 6);
    syscall(__NR_open, /*file=*/0x20000340ul,
            /*flags=O_SYNC|O_NOCTTY|O_NOATIME|O_CREAT|FASYNC|0x2*/ 0x143142ul,
            /*mode=*/0ul);
    break;
  case 2:
    memcpy((void*)0x20000380, "/dev/loop", 9);
    *(uint8_t*)0x20000389 = 0x30;
    *(uint8_t*)0x2000038a = 0;
    memcpy((void*)0x20000140, "./bus\000", 6);
    syscall(__NR_mount, /*src=*/0x20000380ul, /*dst=*/0x20000140ul,
            /*type=*/0ul, /*flags=MS_BIND*/ 0x1000ul, /*data=*/0ul);
    break;
  case 3:
    memcpy((void*)0x20000000, "./bus\000", 6);
    res =
        syscall(__NR_open, /*file=*/0x20000000ul, /*flags=*/0ul, /*mode=*/0ul);
    if (res != -1)
      r[0] = res;
    break;
  case 4:
    *(uint64_t*)0x20000540 = 0;
    *(uint64_t*)0x20000548 = 0;
    *(uint64_t*)0x20000550 = 0;
    *(uint64_t*)0x20000558 = 0;
    *(uint64_t*)0x20000560 = 7;
    *(uint32_t*)0x20000568 = 0;
    *(uint32_t*)0x2000056c = 0;
    *(uint32_t*)0x20000570 = 0;
    *(uint32_t*)0x20000574 = 0;
    memcpy((void*)0x20000578,
           "\xef\x35\x9f\x41\x3b\xb9\x38\x52\xf7\xd6\xa4\xae\x6d\xdd\xfb\xd1"
           "\xce\x5d\x29\xc2\xee\x5e\x5c\xa9\x00\x0f\xf8\xee\x09\xe7\x37\xff"
           "\x0e\xdf\x11\x0f\xf4\x11\x76\x39\xc2\xeb\x4b\x78\xc6\x60\xe6\x77"
           "\xdf\x70\x19\x05\xb9\xaa\xfa\xb4\xaf\xaa\xf7\x55\xa3\xf6\xa0\x04",
           64);
    memcpy((void*)0x200005b8,
           "\x03\x6c\x47\xc6\x78\x08\x20\xd1\xcb\xf7\x96\x6d\x61\xed\xcf\x33"
           "\x52\x63\xbd\x9b\xff\xbc\xc2\x54\x2d\xed\x71\x03\x82\x59\xca\x17"
           "\x1c\xe1\xa3\x11\xef\x54\xec\x32\xd7\x1e\x14\x10\x9d\xc0\x93\xfc"
           "\xe4\x7d\x85\x27\x20\x36\xdc\x78\x38\x8e\x3d\xc1\x77\xe9\xb4\x96",
           64);
    memcpy((void*)0x200005f8,
           "\xf2\x83\x59\x73\x8e\x22\x9a\x4c\x66\x81\x00\x00\x00\x00\x00\x00"
           "\x00\xe6\xbb\xc2\xeb\xce\x21\xaa\x45\xa7\xfe\xa6\x18\x07\x66\xbb",
           32);
    *(uint64_t*)0x20000618 = 0;
    *(uint64_t*)0x20000620 = 0;
    syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x4c04, /*arg=*/0x20000540ul);
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
  use_temporary_dir();
  loop();
  return 0;
}
