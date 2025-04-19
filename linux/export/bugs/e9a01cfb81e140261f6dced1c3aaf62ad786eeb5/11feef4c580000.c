// https://syzkaller.appspot.com/bug?id=e9a01cfb81e140261f6dced1c3aaf62ad786eeb5
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

static __thread int clone_ongoing;
static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  if (__atomic_load_n(&clone_ongoing, __ATOMIC_RELAXED) != 0) {
    exit(sig);
  }
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  int skip = __atomic_load_n(&skip_segv, __ATOMIC_RELAXED) != 0;
  int valid = addr < prog_start || addr > prog_end;
  if (skip && valid) {
    _longjmp(segv_env, 1);
  }
  exit(sig);
}

static void install_segv_handler(void)
{
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  syscall(SYS_rt_sigaction, 0x20, &sa, NULL, 8);
  syscall(SYS_rt_sigaction, 0x21, &sa, NULL, 8);
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_NODEFER | SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
}

#define NONFAILING(...)                                                        \
  ({                                                                           \
    int ok = 1;                                                                \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    } else                                                                     \
      ok = 0;                                                                  \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    ok;                                                                        \
  })

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
      if (call == 3)
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

uint64_t r[1] = {0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    NONFAILING(memcpy((void*)0x200000000040, "ext4\000", 5));
    NONFAILING(memcpy((void*)0x200000000000, "./file1\000", 8));
    NONFAILING(memcpy((void*)0x2000000000c0, "nobh", 4));
    NONFAILING(*(uint8_t*)0x2000000000c4 = 0x2c);
    NONFAILING(memcpy((void*)0x2000000000c5, "auto_da_alloc", 13));
    NONFAILING(*(uint8_t*)0x2000000000d2 = 0x2c);
    NONFAILING(memcpy((void*)0x2000000000d3, "journal_ioprio", 14));
    NONFAILING(*(uint8_t*)0x2000000000e1 = 0x3d);
    NONFAILING(sprintf((char*)0x2000000000e2, "0x%016llx", (long long)7));
    NONFAILING(*(uint8_t*)0x2000000000f4 = 0x2c);
    NONFAILING(*(uint8_t*)0x2000000000f5 = 0);
    NONFAILING(memcpy(
        (void*)0x200000000c80,
        "\x78\x9c\xec\xdc\xcf\x6f\x14\x65\x1f\x00\xf0\xef\x6c\x77\x69\x81\x97"
        "\xd7\x8a\x88\x82\x28\x05\x34\x36\x26\xb6\x50\x50\x38\x78\xc1\xc4\xc4"
        "\x83\x26\x46\x3c\xc8\xb1\x69\x2b\x41\x16\x30\xb4\x07\x21\x44\x4a\x62"
        "\xf0\x4c\xe2\xdd\x78\x34\xde\x34\xf1\xaa\x47\xe3\xc9\x3f\x00\x0f\x1e"
        "\x4c\x0c\x09\x31\x5c\x00\x4f\x6b\x66\x77\xa6\xdd\xee\xaf\xee\xc2\xb6"
        "\xa5\xec\xe7\x93\x6c\xfb\x3c\x33\xcf\xcc\xf3\x3c\x33\xf3\xcc\x3e\xfb"
        "\x3c\x3b\x1b\xc0\xc0\x1a\x4b\xff\x24\x11\xff\x8b\x88\x5b\x11\xf1\x54"
        "\x2d\xba\x32\xc1\x58\xed\xdf\xfd\xbb\x57\x67\x1e\xdc\xbd\x3a\x13\x8b"
        "\x95\xca\xa9\x7f\x92\x6a\xba\x7b\x69\x3c\x93\x6f\xb7\x3d\x8b\x8c\x17"
        "\x22\x0a\x5f\x26\x0d\x3b\xac\x99\xbf\x7c\xe5\xdc\x74\xb9\x3c\x77\x29"
        "\x8b\x4f\x2e\x9c\xff\x6c\x72\xfe\xf2\x95\xd7\xcf\x9e\x9f\x3e\x33\x77"
        "\x66\xee\xc2\xd4\x89\x13\xc7\x8e\x1e\x39\xfe\xe6\xd4\x1b\xbd\x57\xaa"
        "\x45\x7e\x69\xbd\xee\xed\xfd\xe2\xe2\xbe\x3d\xef\x9e\xbe\xf9\xfe\x4c"
        "\x31\x5f\x3e\x92\xfd\xaf\xaf\x47\x5b\xc5\xde\x8a\x31\xd6\x61\xdd\x2b"
        "\xbd\xed\xea\xb1\xb7\xa3\x2e\x9c\x34\x1f\xa7\x6b\xeb\x5a\x18\xba\x36"
        "\x92\x5d\xd6\xa5\xb4\xfd\x5f\x2d\x1f\x3c\xbd\xd1\x05\x02\xd6\x4d\xa5"
        "\x52\xa9\x0c\xb7\x5f\xbd\x58\x69\x74\xbd\x69\x09\xb0\x69\x25\xb1\xd1"
        "\x25\x00\x36\x46\xfe\x46\x9f\x7e\xfe\xcd\x5f\xeb\xd4\xf5\x78\x2c\xdc"
        "\x39\x59\xfb\x00\x94\xd6\xfb\x7e\xf6\xaa\xad\x29\x46\x21\x4b\x53\x6a"
        "\xf8\x7c\xdb\x4f\x63\x11\xf1\xf1\xe2\xbf\xdf\xa4\xaf\xe8\x34\x0e\xf1"
        "\xe7\x1a\x15\x00\x00\x18\x38\xbf\x9c\xcc\x7b\x82\x8d\xfd\xbf\x42\xec"
        "\xae\x4b\xf7\xff\x6c\x0e\x65\x34\x22\x9e\x8e\x88\x9d\x11\xf1\x4c\x44"
        "\xec\x8a\x88\x67\x23\xaa\x69\x9f\x8b\x88\xe7\x1b\x33\x48\x22\x2a\x1d"
        "\xf2\xdf\xd5\x10\x5f\xce\xff\xc7\x6c\x16\xa1\x70\xfb\x91\x2b\xd9\x41"
        "\xda\xff\x7b\x2b\x9b\xdb\x5a\xd9\xff\xcb\x7b\x7f\x31\x3a\x94\xc5\x76"
        "\x44\xe4\x1d\xe6\xb9\xc3\xd9\x31\x19\x8f\xd2\xf0\x27\x67\xcb\x73\x47"
        "\xda\xec\x7f\xcb\x2a\xf9\xd7\xf7\xff\xd2\x57\x9a\x7f\xde\x17\xcc\xca"
        "\x71\xbb\xd8\x30\x40\x37\x3b\xbd\x30\xfd\x70\xb5\x6d\x76\xe7\x7a\xc4"
        "\xde\x62\x63\xfd\x93\x62\x7a\xe2\xf2\x69\x9c\x24\x22\xf6\x44\xc4\xde"
        "\x1e\xf6\x3b\x5a\x17\x3e\xfb\xda\x77\xfb\x96\x22\xa5\x95\xe9\x56\xaf"
        "\x7f\x55\xa5\xc5\x94\x5e\xcf\xf3\x71\xad\x54\xbe\x8d\x78\xb5\x76\xfe"
        "\x17\x63\xc5\xf9\x5f\xce\x31\xe9\x3c\x3f\x39\x39\x12\xe5\xb9\xc3\x93"
        "\xe9\x55\x70\xb8\x65\x1e\xbf\xfd\x7e\xe3\x83\x76\xf9\xaf\x5a\xff\x9f"
        "\xfe\x6a\xdc\xe4\x9d\xe3\x3f\x9f\x7a\xd4\x6a\x2f\x49\xcf\xff\xb6\xba"
        "\xeb\x3f\xf2\xf9\xdb\xe5\xfa\x8f\x26\x11\xc9\xd2\x7c\xed\x7c\x44\x65"
        "\xa8\xb7\x3c\x6e\xfc\xf1\x55\x75\xbf\x63\x87\x9a\xd7\x3d\xec\xf5\xbf"
        "\x25\xf9\xa8\x1a\xce\xdb\xd7\xe7\xd3\x0b\x0b\x97\x8e\x44\x6c\x49\xde"
        "\x6b\x5e\x3e\xb5\xbc\x6d\x1e\xcf\xd3\xa7\xf5\x1f\x3f\xd4\xba\xfd\xef"
        "\xcc\xb6\x49\x8f\xc4\x0b\x11\x91\x5e\xc4\x2f\x46\xc4\x4b\x11\xb1\x3f"
        "\x2b\xfb\x81\x88\x38\x18\x11\x2d\xaa\xb6\xe4\xd7\xb7\x5f\xfe\xb4\xdd"
        "\xba\x2e\xaf\xff\x35\x93\xd6\x7f\xb6\xe5\xfd\x6f\xc5\xf9\x5f\x9e\xaf"
        "\xef\x32\x90\x6f\x9c\x2e\x19\x3a\x77\xe0\xd6\x83\x36\x37\x8f\xee\xce"
        "\xff\xb1\x6a\x68\x3c\x5b\xd2\xfa\xfe\x97\xac\xb8\x45\x74\x5b\xd2\x47"
        "\x3b\x7a\x00\x00\x00\xb0\x39\x14\xa2\xfa\xdd\xff\xc2\xc4\x52\xb8\x50"
        "\x98\x98\xa8\x8d\x01\xed\x8a\x6d\x85\xf2\xc5\xf9\x85\xfd\x11\x71\x61"
        "\xb6\xf6\x8c\xc0\x68\x94\x0a\xf9\x48\x57\x6d\x3c\xb8\x94\xe4\xe3\x9f"
        "\xa3\x75\xf1\xa9\x86\xf8\xd1\x6c\xdc\xf8\xeb\xa1\xad\xd5\xf8\xc4\xcc"
        "\xc5\xf2\xec\x46\x57\x1e\x06\xdc\xf6\x6a\x9b\x4f\x9a\xda\x7f\xea\xef"
        "\x1e\xc7\x79\x81\x4d\xa8\x0f\xf3\x68\xc0\x26\xb5\x5a\xfb\xdf\x7d\x73"
        "\x9d\x0a\x02\xac\x3b\xef\xff\x30\xb8\xea\xda\xff\x62\x9b\x24\x8b\xbe"
        "\x29\x03\x4f\x26\xef\xff\x30\xb8\x5a\xb5\xff\x6b\xf1\x7d\xc7\x67\x17"
        "\xdc\x33\x60\xf3\xab\x68\xcb\x30\xd0\xb4\x7f\x18\x5c\xc5\xf8\x70\x29"
        "\x5c\x7d\xec\xb9\xe5\xd3\xb6\xc0\x93\xc8\xfb\x3f\x0c\xa4\x5e\x9f\xeb"
        "\xef\x2d\x50\x19\x6e\xbd\x6a\x24\x5a\xfc\x62\xc0\xc8\xda\x14\x63\x6b"
        "\x8b\xbc\x36\x24\x90\xf6\xac\xfa\xb8\xc3\x52\x44\x74\x97\x78\xeb\xc3"
        "\x64\x91\x77\x01\xdb\xff\xc2\x43\xa1\xb7\x1d\x0e\x47\xf3\xaa\xa1\xe8"
        "\xb4\x55\xd2\xc3\xef\x38\xe4\x81\xf4\xa8\xac\x9a\xf8\xcc\xee\xbe\x5f"
        "\xfc\xf9\x6f\xa2\xf4\xfb\xb2\xf9\x61\xb9\x9d\x96\xba\x3c\xdd\x7d\x0a"
        "\x6c\xc8\xed\x08\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xa0\xef\xfe\x0b\x00\x00\xff"
        "\xff\xf4\x09\xd6\x78",
        1229));
    NONFAILING(syz_mount_image(/*fs=*/0x200000000040, /*dir=*/0x200000000000,
                               /*flags=MS_POSIXACL*/ 0x10000,
                               /*opts=*/0x2000000000c0, /*chdir=*/3,
                               /*size=*/0x4cd, /*img=*/0x200000000c80));
    break;
  case 1:
    NONFAILING(memcpy((void*)0x200000000180, "./file1\000", 8));
    res = syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x200000000180ul,
                  /*flags=O_SYNC|O_DIRECT|O_CREAT|O_RDWR*/ 0x105042,
                  /*mode=S_IXOTH|S_IXGRP|S_IWUSR|S_IRUSR*/ 0x189);
    if (res != -1)
      r[0] = res;
    break;
  case 2:
    syscall(__NR_mmap, /*addr=*/0x200000000000ul, /*len=*/0x600000ul,
            /*prot=PROT_WRITE|PROT_READ|PROT_EXEC|0x7ffff0*/ 0x7ffff7ul,
            /*flags=MAP_UNINITIALIZED|MAP_NONBLOCK|MAP_LOCKED|MAP_FIXED|0x1*/
            0x4012011ul, /*fd=*/r[0], /*offset=*/0x1000ul);
    break;
  case 3:
    NONFAILING(memcpy((void*)0x200000000000, "1\000", 2));
    syscall(__NR_write, /*fd=*/r[0], /*data=*/0x200000000000ul,
            /*len=*/0x2400ul);
    break;
  case 4:
    NONFAILING(memcpy((void*)0x200000000000, "1\000", 2));
    syscall(__NR_write, /*fd=*/r[0], /*data=*/0x200000000000ul,
            /*len=*/0x2400ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffffffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul,
          /*fd=*/(intptr_t)-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200000000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul,
          /*fd=*/(intptr_t)-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200001000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul,
          /*fd=*/(intptr_t)-1, /*offset=*/0ul);
  const char* reason;
  (void)reason;
  install_segv_handler();
  use_temporary_dir();
  loop();
  return 0;
}
