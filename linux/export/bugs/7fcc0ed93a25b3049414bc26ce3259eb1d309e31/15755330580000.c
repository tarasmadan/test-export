// https://syzkaller.appspot.com/bug?id=7fcc0ed93a25b3049414bc26ce3259eb1d309e31
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
  for (call = 0; call < 4; call++) {
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

void execute_call(int call)
{
  switch (call) {
  case 0:
    memcpy((void*)0x20000040, "nilfs2\000", 7);
    memcpy((void*)0x200001c0, "./bus\000", 6);
    memcpy(
        (void*)0x20002340,
        "\x78\x9c\xec\xdd\x4d\x8c\x5b\x47\xe1\x00\xf0\xb1\x77\xbd\xc9\x26\xe9"
        "\x3f\x4e\xff\x09\x5d\xd2\xd0\x26\x14\xda\xf2\xd1\x4d\xb3\xbb\x84\x8f"
        "\x08\x92\x2a\x11\x12\x51\x53\x21\x2e\x95\x22\x2e\x51\x9a\x96\x88\x10"
        "\x24\x8a\x04\x54\x95\x48\x72\xe2\x46\xab\x2a\x48\x9c\xf8\x10\xa7\x5e"
        "\xaa\x82\x90\xe8\x05\x45\x3d\x71\xa9\x44\x23\x55\x48\x3d\x15\x0e\x1c"
        "\x88\x82\xa8\xc4\x01\x02\x89\xd1\x7a\x67\xbc\xf6\xc4\xee\xb3\x37\xbb"
        "\x7e\x76\xfc\xfb\x49\xb3\xe3\x79\x33\xcf\x33\xcf\xfb\xfc\xfc\x3e\x67"
        "\x02\x30\xb1\xaa\xcd\xbf\x4b\x4b\x73\x95\x10\x2e\xbf\xf1\xca\xb1\xbf"
        "\x3d\xfc\xd7\xd9\xe5\x29\x87\x5b\x25\xea\xcd\xbf\xd3\x6d\xa9\x5a\x08"
        "\xa1\x12\xd3\xd3\xd9\xfb\xbd\x37\xb5\x12\xdf\x7c\xff\xc5\xd3\xdd\xe2"
        "\x4a\x58\x68\xfe\x4d\xe9\xf0\xd4\xf5\xd6\xbc\x5b\x43\x08\x17\xc2\xde"
        "\x70\x25\xd4\xc3\xee\xcb\x57\x5f\x7e\x6b\xe1\xc9\x13\x17\x8f\x5f\xda"
        "\xf7\xf6\xab\x87\xae\x6d\xcc\xd2\x03\x00\xc0\x64\xf9\xea\x95\x43\x4b"
        "\xbb\xfe\xfc\xc7\xfb\x77\xdc\x78\xed\x81\x23\x61\x53\x9a\xbc\xfc\xa2"
        "\xb9\x7f\x5e\x8f\x13\xb6\xc5\xfd\xfe\x23\x71\xc7\x3f\xed\xff\x57\x43"
        "\x67\xba\xd2\x16\xda\xcd\x64\xe5\xa6\x63\xa8\xce\x76\x96\x9b\xea\x52"
        "\xae\xbd\x9e\x5a\x56\x6e\xba\x47\xfd\x33\x59\xfd\xb5\x1e\xe5\x36\x85"
        "\x0f\xae\x7f\xaa\x6d\x5a\xb7\xe5\x86\x71\x96\xd6\xe3\x7a\xa8\x54\xe7"
        "\x3b\xd2\xd5\xea\xfc\xfc\xca\x31\x79\x68\x1e\xd7\xcf\x54\xe6\xcf\x9f"
        "\x3d\xf7\xec\xf3\x25\x35\x14\x58\x77\xff\x7c\x30\x84\xb0\xb7\x2d\x1c"
        "\xbd\xd4\x99\x1e\xb5\x70\x78\x04\xda\x90\x85\xe6\xbe\x45\x1f\xe5\x1a"
        "\x23\xd0\xd6\xb1\x0c\x47\x86\x57\xd7\x8d\xc6\x8a\xd2\x97\x79\x48\xa1"
        "\xb1\xbd\xec\x2d\x10\xc0\x8a\xfc\x7a\xe1\x6d\x2e\xe4\x67\x16\xee\x4c"
        "\xeb\xdd\xa6\xfb\xab\xff\xfa\x13\xd5\xee\xf3\xc3\x3a\x18\xf6\xfa\x3f"
        "\x50\xfd\x33\x25\xd7\x1f\xd4\xff\xab\x8b\xb6\x38\xac\x9f\xbb\x75\x6d"
        "\x4a\xcb\x95\xbe\x47\xdb\x62\x3a\xbf\x8e\x90\xdf\xbf\xd4\xfb\xfb\x97"
        "\x5f\xe9\xe8\x9c\x9a\x5f\x8f\xa8\xf5\xd9\xce\x5e\xd7\x11\xc6\xe5\xfa"
        "\x42\xaf\x76\x4e\x0d\xb9\x1d\x6b\xd5\xab\xfd\xf9\x7a\x71\xb7\xfa\x62"
        "\x8c\xd3\xe7\xf0\xa5\x2c\xbf\xfd\xfb\x93\xff\x4f\xc7\xe5\x7f\x0c\x74"
        "\xf7\xaf\xfc\xfc\xbf\x20\x08\xa3\x1d\x42\x47\xba\x76\x27\xef\xd5\x28"
        "\x79\xfb\x03\x8c\xae\xfc\xbe\xb9\x46\xba\x3e\x1a\xe5\xf7\xf5\xe5\xf9"
        "\x9b\x0a\xf2\x37\x17\xe4\xcf\x16\xe4\x6f\x29\xc8\xdf\x5a\x90\x0f\x93"
        "\xec\x37\xdf\xfd\x71\x78\xa9\xb2\x7a\x9c\x9f\x1f\xd3\x0f\x7a\x3e\x3c"
        "\x9d\x67\xbb\x27\xc6\xff\x37\x60\x7b\xf2\xf3\x91\x83\xd6\x9f\xdf\xf7"
        "\x3b\xa8\x3b\xad\x3f\xbf\x9f\x18\x46\xd9\xef\x4e\x3d\x7d\xe6\x73\xcf"
        "\x9c\xbc\xba\x72\xff\x7f\xa5\xb5\xfe\xdf\x8a\xeb\x7b\x3a\xdc\xa8\xc7"
        "\xef\xd6\x95\x58\x20\x9d\x2f\xcc\xcf\xab\xb7\xee\xfd\xaf\x77\xd6\x53"
        "\xed\x51\xee\xde\xac\x3d\xf7\x74\x29\xdf\x7c\xbd\xb3\xb3\x5c\x65\xe7"
        "\xea\xfb\x84\xb6\xed\xcc\x6d\xed\x98\xeb\x9c\x6f\x7b\xaf\x72\x7b\x3a"
        "\xcb\xd5\xb3\x72\xb3\x31\x6c\xce\xda\x9b\xef\x9f\x6c\xc9\xe6\x4b\xfb"
        "\x1f\x69\xbb\x9a\x3e\xaf\xe9\x6c\x79\x6b\xd9\x72\xcc\x64\xed\x48\xdb"
        "\x95\x1d\x31\xce\xdb\x01\x6b\x91\xd6\xc7\x5e\xf7\xff\xa7\xf5\x73\x2e"
        "\xd4\x2a\xcf\x9e\x3d\x77\xe6\xf1\x98\x4e\xeb\xe9\x1f\xa6\x6a\x9b\x96"
        "\xa7\x1f\x18\x72\xbb\x81\x3b\xd7\xef\xf3\x3f\x73\xa1\xf3\xf9\x9f\x6d"
        "\xad\xe9\xb5\x6a\xfb\x76\x61\xfb\xea\xf4\xca\xca\x76\xe1\xf5\xf8\x7e"
        "\x9d\xd3\x17\x5a\xf5\x74\x4e\x5f\x8c\xe9\xf4\x3b\xf7\x8d\xa9\xd9\xe6"
        "\xf4\xf9\xd3\xdf\x3e\xf7\xcc\xfa\x2f\x3e\x4c\xb4\xe7\x7f\xf0\xc2\x37"
        "\x4f\x9d\x3b\x77\xe6\x3b\xa5\xbf\x58\x9c\x0d\x61\x04\x9a\xb1\x86\x17"
        "\x5f\x1e\x8d\x66\x0c\xf2\x22\x1d\xb6\x8c\x4a\x7b\xbc\x18\xb9\x17\x25"
        "\x6f\x98\x80\x0d\xb7\xff\x87\x2b\x3b\x01\x8f\x9d\xfd\xd6\xa9\xe7\xce"
        "\x3c\x77\xe6\xfc\xe2\xc1\x83\x8b\x0b\x0b\x07\x3f\xbf\xb8\xb4\xbf\xb9"
        "\x5f\xbf\xbf\x7d\xef\xbe\xdd\x85\x12\x5a\x0b\xac\xa7\xd5\x1f\xfd\xb2"
        "\x5b\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf4\xeb\x7b\xc7\x8f\x5d"
        "\x7d\xe7\xcd\xcf\xbe\xbb\xf2\xfc\xff\xea\xf3\x7f\xe9\xf9\xff\x74\xe7"
        "\x6f\x7a\xfe\xff\x47\xd9\xf3\xff\xf9\x73\xf2\xe9\x39\xf8\xf4\x1c\xe0"
        "\x8e\x2e\xf9\xcd\x32\x59\x07\xab\x33\x59\xb9\x5a\x0c\xff\x9f\xb5\x77"
        "\x67\x56\xcf\xae\x6c\xbe\x0f\xc5\xb8\x35\x8e\x5f\x7c\xfe\x3f\x55\x97"
        "\xf7\xeb\x9a\xda\x73\x5f\x36\xbd\xd6\x23\x99\x75\x27\x70\x5b\x7f\x29"
        "\x33\x59\x1f\x24\xf9\x78\x81\x1f\x8d\xf1\xa5\x18\xff\x32\x40\x89\x2a"
        "\xb3\xdd\x27\xc7\xb8\xa8\x7f\xeb\xb4\xae\xa7\xfe\x29\xda\xfa\xa5\x68"
        "\xe8\x1f\x78\x7c\xa4\xff\x5b\x5a\x1b\x52\x3f\x26\xe9\xf9\xef\xae\xfd"
        "\x3a\xb5\xfd\xb3\x77\x0c\xa1\x8d\xac\xbf\x61\x3c\x4e\x58\xf6\x32\x02"
        "\xdd\xfd\x7d\xa2\xfa\xff\xfe\xc7\xea\x82\x97\xde\x16\xa1\x77\x98\x1e"
        "\x6e\x7d\x3f\x9d\xdc\x75\xa2\xd1\x73\x2f\xbd\xdf\x11\x6c\x00\xd6\x47"
        "\xd9\xe3\x7f\xa6\xf3\x9e\x29\x3e\xff\xfb\xaf\x6c\x5e\x0e\xa9\xd8\xf5"
        "\x27\x3a\xb7\x97\x79\xff\xa5\x30\x88\x3f\xbd\xd3\x99\x1e\xf5\xf1\x27"
        "\x37\xba\xfe\x7c\xdc\xbe\x61\xd7\x5f\xf6\xf2\x0f\x7b\xfc\xcf\xd6\xf8"
        "\x77\x7d\x6f\xff\xb2\x11\xf3\xea\x6b\xab\xf7\xdf\x3f\xbb\xf6\x6e\x5b"
        "\xb5\x61\x77\xbf\xf5\xe7\xcb\x9f\xfa\x81\xde\x39\x58\xfd\x37\x62\xfd"
        "\x69\x69\x1e\x09\xfd\xd5\xdf\xf8\x45\x56\x7f\x7e\x41\xa8\x4f\xff\xc9"
        "\xea\xdf\xd2\x67\xfd\xb7\x2d\xff\x9e\xb5\xd5\xff\xdf\x58\x7f\xfa\xd8"
        "\x1e\x7d\xa8\xdf\xfa\x57\x5a\x5c\xa9\x76\xb6\x23\x3f\x6f\x9c\xae\xff"
        "\xe5\xe7\x8d\x93\x9b\xd9\xf2\xa7\xbe\x3d\x3f\xa0\xfe\xaf\xbd\xd0\x6d"
        "\xf9\xd7\x38\x50\xe3\xad\x58\x3f\x4c\xb2\x71\x19\x67\x76\x50\xd9\x7e"
        "\x44\x6b\xa7\x7d\xed\xe3\xff\x46\x17\xd6\x77\xfc\xdf\x56\x63\xb3\xcd"
        "\x5a\x7e\x1f\xc6\x67\x62\x3a\x6d\x88\xd3\x7d\x0e\xf9\x78\x27\x83\xb6"
        "\x3f\xdd\x5f\x91\x7e\x07\x76\x65\xef\x5f\x29\xf8\x7d\x33\xfe\xef\x78"
        "\xfb\x42\x8c\x8b\xbe\x0f\x69\xfc\xdf\xb4\x3e\xd6\xe3\x4f\x7e\x5b\xba"
        "\xf9\x59\xa6\x74\xad\xcb\x67\x7b\xb7\x6e\x6b\x60\x5c\xbd\x37\x51\xd7"
        "\xff\xd6\x21\x9c\xdc\xf0\x3a\x36\x97\xbe\x8c\x23\x1f\x8e\x8c\x40\x1b"
        "\x5a\xa1\x31\xb5\x86\xf9\x5a\xe3\xc4\x95\xdc\xfe\x46\xa3\xb1\xb1\x27"
        "\xb4\x0a\x94\x5a\x39\xa5\x7f\xfe\x65\x1f\x27\x94\x5d\x7f\xd9\x9f\x7f"
        "\x91\x7c\xfc\xdf\x7c\x1f\x3e\x1f\xff\x37\xcf\xcf\xc7\xff\xcd\xf3\xf3"
        "\xf1\x7f\xf3\xfc\xd9\xf8\x1f\xea\x95\x9f\x8f\xff\x9b\x7f\x9e\xf9\xf8"
        "\xbf\x79\xfe\x7d\xd9\xfb\xe6\xe3\x03\xcf\x15\xe4\x7f\xb8\x20\x7f\x77"
        "\xf7\xfc\xd6\x61\xfb\xfd\x05\xf3\xef\x29\xc8\xff\x48\x41\xfe\xbe\x56"
        "\xfe\xe1\x8e\x12\x29\xff\x81\x82\xf9\x1f\x2c\xc8\xbf\xb7\x20\xff\xa1"
        "\x82\xfc\x8f\x15\xe4\x7f\xbc\x20\xff\xe1\x82\xfc\x47\xdb\xf2\xdb\xc7"
        "\x80\x4e\xf9\x9f\x28\x98\xff\x6e\x97\x9e\x47\x99\xd4\xe5\x87\x49\x96"
        "\x3f\x9f\xe7\xfb\x0f\x93\x23\x5d\xff\xe9\xf5\xfd\xdf\x59\x90\x0f\x8c"
        "\xaf\x9f\xbc\x76\xe0\xe8\xc9\x5f\x7f\xbd\xbe\xf2\xfc\xff\x4c\xeb\x7c"
        "\x48\xba\x8e\x77\x24\xa6\x6b\xf1\xf8\xe9\xfb\x31\x9d\x5f\xf7\x0e\x6d"
        "\xe9\xe5\xbc\x37\x63\xfa\x2f\x59\xfe\xa8\x9f\xef\x80\x49\x92\xf7\x9f"
        "\x91\xff\xbe\x3f\x52\x90\x0f\x8c\xaf\x74\x9f\x97\xef\x37\x4c\xa0\xca"
        "\xe6\xee\x93\x63\x5c\xd4\x6f\x55\xaf\xfd\x7c\xc6\xcb\x27\x63\xfc\xa9"
        "\x18\x7f\x3a\xc6\x8f\xc5\x78\x3e\xc6\xfb\x63\x7c\x20\xc6\x0b\x43\x6a"
        "\x1f\x1b\xe3\xe8\xeb\xbf\x3d\xf4\x52\x65\xf5\x78\x7f\x7b\x96\xdf\xef"
        "\xfd\xe4\xf9\xf3\x40\x1d\xfd\x44\x85\x10\x16\xfb\x6c\x4f\x7e\x7e\x60"
        "\xd0\xfb\xd9\xf3\x7e\xfc\x06\x75\xa7\xf5\xaf\xf1\x71\x30\x00\x00\x00"
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
        "\x00\x00\x00\x00\x80\xd2\x54\x9b\x7f\x97\x96\xe6\x2a\x21\x5c\x7e\xe3"
        "\x95\x63\x4f\x9f\x38\xbb\x7f\x79\xca\xe1\x56\x89\x7a\xf3\xef\x74\x5b"
        "\xaa\xd6\x9a\x2f\x84\xc7\x63\x3c\x15\xe3\x9f\xc7\x17\x37\xdf\x7f\xf1"
        "\x74\x7b\x7c\x2b\xc6\x95\xb0\x10\x2a\xa1\xd2\x9a\x1e\x9e\xba\xde\xaa"
        "\x69\x6b\x08\xe1\x42\xd8\x1b\xae\x84\x7a\xd8\x7d\xf9\xea\xcb\x6f\x2d"
        "\x3c\x79\xe2\xe2\xf1\x4b\xfb\xde\x7e\xf5\xd0\xb5\x8d\xfb\x04\x00\x00"
        "\x00\xe0\xee\xf7\xbf\x00\x00\x00\xff\xff\x8d\x9c\x0b\xa8",
        2751);
    syz_mount_image(/*fs=*/0x20000040, /*dir=*/0x200001c0, /*flags=*/0,
                    /*opts=*/0x200002c0, /*chdir=*/1, /*size=*/0xabf,
                    /*img=*/0x20002340);
    break;
  case 1:
    memcpy((void*)0x200005c0, "./bus\000", 6);
    syscall(__NR_open, /*file=*/0x200005c0ul,
            /*flags=O_NONBLOCK|O_NOFOLLOW|O_NOATIME|O_DIRECT|O_CREAT|0x2*/
            0x64842ul, /*mode=*/0ul);
    break;
  case 2:
    memcpy((void*)0x200001c0, ".\000", 2);
    syscall(
        __NR_mount, /*src=*/0ul, /*dst=*/0x200001c0ul, /*type=*/0ul,
        /*flags=MS_SLAVE|MS_PRIVATE|MS_UNBINDABLE|MS_POSIXACL|MS_REC|0x1000c2a*/
        0x10f4c2aul, /*data=*/0ul);
    break;
  case 3:
    memcpy((void*)0x200000c0, "./file1\000", 8);
    memcpy((void*)0x20000080, "./bus\000", 6);
    syscall(__NR_rename, /*old=*/0x200000c0ul, /*new=*/0x20000080ul);
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
  use_temporary_dir();
  loop();
  return 0;
}