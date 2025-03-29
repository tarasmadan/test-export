// https://syzkaller.appspot.com/bug?id=da0d8a51e89e8a43bbe9f97163f12296725c796c
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
      if (call == 2)
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

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    memcpy((void*)0x200000000080, "udf\000", 4);
    memcpy((void*)0x200000000280, ".\002\000", 3);
    memcpy(
        (void*)0x200000001940,
        "\x78\x9c\xec\xdd\x5d\x68\x5c\xe9\x79\x07\xf0\xe7\xd5\x91\xd6\x92\xd2"
        "\x34\xb3\x9b\x8d\xf3\xe1\x40\x87\x6e\x20\x5b\x6f\x76\x91\x2c\xef\x5a"
        "\xc5\x1b\x90\x63\x45\x64\xc1\x78\xcd\xca\xca\xc5\x42\x41\x63\x4b\x76"
        "\x87\xd5\x97\x25\xb9\x78\x43\x09\x2e\x24\x94\x90\xb6\xb8\xe4\x22\x97"
        "\x35\x6c\x42\x7b\x57\x43\xa1\x85\xa5\x01\xf7\x6a\x1b\x42\x40\xf4\xaa"
        "\xf4\xa2\xb8\xed\xc6\x6c\xef\x26\x21\x69\x4b\x2f\xa2\x72\x66\xde\x91"
        "\x46\x5a\x59\x52\x6d\xcb\x92\xbd\xbf\x9f\xb1\xff\x67\xce\x3c\x67\xe6"
        "\x3d\xb3\x7a\x46\x67\x66\xe7\x9d\x13\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x40\xc4\x57\xbe\x7a\x6a\x60\x30\x6d\x53\xd0"
        "\xf3\x08\x07\x03\x00\x3c\x12\x67\xc7\xdf\x18\x18\xda\xee\xf7\x3f\x00"
        "\xf0\xc4\x39\xbf\xd3\xeb\x7f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x88\x48"
        "\x51\xc4\xb7\x22\xc5\x3b\xdf\x6d\xa4\xc9\xe6\xe5\x96\xde\x33\xf5\xb9"
        "\xab\xd7\x26\x46\xc7\xb6\xde\xac\x2f\x45\x8a\xae\x28\x9a\xf5\xe5\xdf"
        "\xde\xc1\x63\x43\xc7\x5f\x7e\xe5\xc4\x70\x3b\xb7\xdf\xfe\x61\xfb\x6c"
        "\xbc\x3e\x7e\xfe\x54\xf5\xf4\xfc\xec\xc2\xe2\xf4\xd2\xd2\xf4\x54\x75"
        "\x62\xae\x7e\x71\x7e\x6a\x7a\xd7\xb7\xf0\xa0\xdb\x6f\x76\xb4\xf9\x00"
        "\x54\x67\xdf\xba\x3a\x75\xe9\xd2\x52\xf5\xd8\x4b\x43\x1b\xae\xbe\x56"
        "\xb9\x7b\xe8\x63\x87\x2b\x27\x87\x8f\x0c\xbd\xd9\xae\x9d\x18\x1d\x1b"
        "\x1b\xef\xa8\xe9\xee\xb9\xef\x7b\xff\x90\xf4\xf0\x6e\x8a\x27\xc8\x53"
        "\x51\xc4\xd7\x22\xc5\x7b\x2f\x7e\x90\x6a\x11\xd1\x15\x0f\xde\x0b\x3b"
        "\x3c\x77\xec\xb5\xbe\xe8\x2e\xfb\xaf\xb9\x13\x13\xa3\x63\xcd\x1d\x99"
        "\xa9\xd7\xe6\x96\xcb\x2b\x53\x57\xae\xea\x8e\xa8\x74\x6c\x34\xd2\xee"
        "\x91\x47\xd0\x8b\x0f\x64\x24\xe2\x7a\xf9\xdf\xa9\x1c\xf0\xd1\x72\xf7"
        "\xc6\x17\x6a\x8b\xb5\x0b\x33\xd3\xd5\x73\xb5\xc5\xe5\xfa\x72\x7d\x7e"
        "\x2e\x75\xb5\x46\x5b\xee\x4f\x25\xba\x62\x38\x45\x2c\x44\x44\xa3\xd8"
        "\xef\xc1\x73\xd0\xf4\x44\x11\xc7\x23\xc5\xdd\x5f\x34\xd2\x85\x88\x28"
        "\xda\x7d\xf0\xc2\xd9\xf1\x37\x06\x86\x76\xbe\x81\xee\x47\x30\xc8\x7b"
        "\xdc\x6d\xa5\x88\x58\x89\xc7\xa0\x67\xe1\x80\x3a\x14\x45\xfc\x79\xa4"
        "\xf8\xde\xe4\x40\x5c\xcc\x7d\xd5\x6c\x9b\xf7\x23\xbe\x58\xe6\xab\x11"
        "\x57\xca\xbc\x9d\xe2\x46\xbe\x9c\xca\x27\x88\xe1\x88\x9f\xfb\x7d\x02"
        "\x8f\xb5\xee\x28\xe2\xa7\x91\x62\x3e\x35\xd2\x54\xbb\xf7\x9b\xc7\x95"
        "\x67\xbe\x5e\x7d\x6d\xee\xd2\x7c\x47\x6d\xfb\xb8\xf2\x31\x79\x7d\xd0"
        "\x7f\x68\xeb\xf5\x7d\x7b\x7b\xb7\x9b\x38\x36\xe1\x00\xeb\x8d\x22\x2e"
        "\x34\x8f\xf8\x1b\xe9\xfe\xdf\xec\x02\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x1e\x8d\x22\xde\x8d\x14\xb7\x66\x9f\x4f\x0b\xd1\x39\xa7\xb4"
        "\x3e\x77\xb9\x7a\xbe\x76\x61\xa6\xf5\xa9\xe0\xf6\x67\xff\xab\x79\xab"
        "\xd5\xd5\xd5\xd5\x4a\x6a\x65\x35\xe7\x40\xce\x91\x9c\xe7\x72\x4e\xe6"
        "\x5c\xc8\x79\x3d\xe7\x8d\x9c\x37\x73\xde\xca\x79\x3b\xe7\x4a\xce\x3b"
        "\x39\x1b\x39\xa3\x2b\xdf\x7f\xce\x6a\xce\x81\x9c\x23\x39\xcf\xe5\x9c"
        "\xcc\xb9\x90\xf3\x7a\xce\x1b\x39\x6f\xe6\xbc\x95\xf3\x76\xce\x95\x9c"
        "\x77\x72\x36\x72\x86\x79\x4f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x3c\x64\x7d\x51\xc4\x58\xa4\xb8\xf9\xce\x1f\x34"
        "\xcf\x2b\x1d\xcd\xf3\xd2\x7f\xe2\xe4\xf0\xd9\xd1\x67\x3b\xcf\x19\xff"
        "\xe9\x1d\x6e\xa7\xac\x7d\x29\x22\xde\x8d\xdd\x9d\x93\xb7\x27\x9f\x6b"
        "\x3c\x75\x95\x7f\x1e\xfe\x7e\x01\x3b\xeb\x8d\x22\xbe\x99\xcf\xff\xf7"
        "\x47\xfb\x3d\x18\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xe0\x40\xe8\x8a\x22\xbe\x15"
        "\x29\xbe\xff\xab\x46\x8a\x14\x11\x23\x11\x93\xd1\xca\x3b\xc5\x7e\x8f"
        "\x0e\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x28\xf5\xa6"
        "\x22\x4e\x47\x8a\xff\xfc\x6a\x6f\xf3\xf2\x4a\x44\x7c\x2e\x22\x7e\xbd"
        "\x5a\xfe\x89\xf8\x9f\xd5\xcd\xf6\x7b\xc4\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\xf0\x04\x4a\x45\x5c\x89\x14\x3f\x78"
        "\xaf\x91\x2a\x11\x71\xad\x72\xf7\xd0\xc7\x0e\x57\x4e\x0e\x1f\x19\x7a"
        "\xb3\x88\x22\x52\x59\xd2\x59\xff\xfa\xf8\xf9\x53\xd5\xd3\xf3\xb3\x0b"
        "\x8b\xd3\x4b\x4b\xd3\x53\xd5\x89\xb9\xfa\xc5\xf9\xa9\xe9\xdd\xde\x5d"
        "\xef\x99\xfa\xdc\xd5\x6b\x13\xa3\x63\x7b\xb2\x33\x3b\xea\xdb\xe3\xf1"
        "\xf7\xf5\x9e\x9e\x5f\x78\x7b\xb1\x7e\xf9\xf7\x97\xb7\xbc\xbe\xbf\xf7"
        "\xd4\x85\xa5\xe5\xc5\xda\xc5\xad\xaf\x8e\xbe\xe8\x8e\x18\xe8\x5c\x73"
        "\xb4\x39\xe0\x89\xd1\xb1\xe6\xa0\x67\xea\xb5\xb9\xe6\xa6\xa9\xeb\x1e"
        "\x03\xec\x8e\xa8\xee\x76\x67\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x38\x30\xfa\x53\x11\xa3\x91\xe2\xb9\x1f\x1d\x4f"
        "\xed\x79\xe3\xdd\xad\x39\xff\x1f\x6f\x5d\x2a\xd6\x6a\x7f\xf8\x87\xeb"
        "\xdf\x05\x30\xb3\x29\xdb\x3a\xbf\x3f\x60\x7d\xb9\x3d\x59\x7d\xf3\xfa"
        "\xa1\x37\xd3\x6e\x07\x7a\xb4\x39\xf1\xbe\x3a\x31\x3a\x36\x36\xde\xb1"
        "\xba\xbb\xe7\xc3\xa5\xe5\x98\x52\x2a\xe2\x53\x91\xe2\xc8\xdf\x7f\xa6"
        "\x39\x1f\x3e\x45\xff\x96\x73\xe3\xcb\xba\x3f\x89\x14\xc3\xff\x7b\x3c"
        "\xd7\x55\x8e\x94\x75\x23\x1b\xaa\x7a\x8f\x4e\x8c\x8e\x55\xcf\xce\xcf"
        "\xbd\x78\x6a\x66\x66\xfe\x62\xad\xaf\x76\x61\x66\xba\x3a\xbe\x50\xbb"
        "\xb8\xeb\x2f\x0e\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x80\x6d\xf4\xa7\x22\xfe\x34\x52\x1c\x7f\x6d\x25\xb5\xcf\x3b"
        "\x9f\xe7\xff\x77\xb7\x2e\x75\xcc\xff\x7f\x35\xa2\x3d\x93\xbf\x37\x6d"
        "\xcc\x35\xcd\xb9\xfd\xbf\xd9\x9c\xdb\xdf\x5a\xfe\xc4\xc9\xe1\xd7\x8e"
        "\x3d\x77\xaf\xf5\x7b\x31\xff\xbf\x1c\x53\x4a\x45\xfc\x3a\x52\x3c\xfd"
        "\x17\x9f\x69\x9e\x4f\xbf\x3d\xff\x7f\x60\x53\x6d\x59\xf7\x83\x48\xf1"
        "\xd3\x6f\x7f\x3e\xd7\x75\x3d\x55\xd6\x0d\xb6\x77\xa7\x75\x8b\x97\xea"
        "\x33\xd3\x03\x65\xed\x0b\x91\xe2\x3b\xe7\xda\xb5\xd1\xac\x7d\x25\xd7"
        "\x7e\x72\xbd\x76\xb0\xac\xfd\x87\x48\xf1\xcc\xef\x6d\xac\x3d\x91\x6b"
        "\x9f\x5d\xaf\x3d\x56\xd6\xde\x8d\x14\x63\x67\xb7\xae\xfd\xd4\x7a\xed"
        "\x50\x59\xdb\x17\x29\xbe\xf4\xc7\xd5\x76\x6d\x7f\x59\xfb\x95\x5c\x7b"
        "\x78\xbd\xf6\xa5\x8b\xf3\x33\x53\xbb\x7d\x78\xf9\x68\x2a\xfb\xff\x5f"
        "\x23\xc5\x17\x06\xbf\x96\xda\x3f\xf3\xf7\xec\xff\x8e\xef\xff\xb8\xbe"
        "\x29\xd7\x7c\xa8\xe7\xb7\x5f\x7e\x58\xfd\x5f\xe9\x58\x77\x3d\xf7\xf5"
        "\x6a\xee\xff\xc1\x1d\xfa\xff\x4a\xa4\xf8\xb3\x1b\x9f\xcf\x75\xad\xde"
        "\x3b\x96\xaf\x7f\xba\xf9\xef\x7a\xff\x7f\x27\x52\xfc\xce\xc7\x37\xd6"
        "\xbe\x9c\x6b\x9f\x59\xaf\x1d\xdc\xed\x6e\xc1\x7e\x2a\xfb\xff\xc7\x91"
        "\x62\xe5\xce\x3f\xaf\xfd\xcc\xe7\xfe\xcf\x9d\xb5\xde\xa1\x9d\xfd\xff"
        "\xb9\xee\x8d\xd9\x3e\x2e\xd8\xaf\xfe\x7f\xba\x63\x5d\x25\x8f\x6b\xe8"
        "\xff\xf9\x58\xc0\x47\xcd\xd2\xdb\xdf\x78\xab\x36\x33\x33\xbd\xf8\xe1"
        "\x85\x91\x88\xb8\xc7\x55\x4f\xd6\x42\x3a\x18\xc3\xd8\x7e\xa1\x2f\x22"
        "\xf6\xee\x2e\x7e\x6b\xff\x77\xd0\xc2\xc1\x5a\xd8\xe2\xc9\xe2\x2f\x7f"
        "\xb9\xba\xba\x0f\xcf\x51\xc0\xde\x28\x8f\xff\xff\x2b\x52\x7c\xf9\x4a"
        "\x91\xda\xaf\x63\xf3\xf1\xff\x6f\xb4\x2e\xad\xbf\xfe\xff\xef\x6f\xae"
        "\x1f\xff\x9f\xdc\x94\x6b\xf6\xe9\xf8\xff\x99\x8e\x75\x27\xf3\xab\x96"
        "\x9e\xee\x88\xde\xe5\xd9\x85\x9e\x4f\x47\xf4\x2e\xbd\xfd\x8d\x17\xeb"
        "\xb3\xb5\xcb\xd3\x97\xa7\xe7\x86\x86\x86\x4f\xfc\xee\xf1\xc1\x63\x27"
        "\x06\x7b\x9e\x6a\xbf\xb8\x5f\x5f\xda\xf5\x63\x07\x8f\xbb\xb2\xff\xdf"
        "\x8a\x14\x3f\xfc\xeb\x7f\x5a\x7b\x1f\x7b\xe3\xeb\xff\xad\xdf\xff\xeb"
        "\xdf\x94\x6b\xf6\xa9\xff\x3f\xd9\xb9\x4f\x1b\x5e\xd7\xec\xfa\xa1\x80"
        "\x8f\x9c\xb2\xff\xff\x2a\x52\xfc\xcb\xcd\x0f\xd6\xfe\x7f\xd3\x76\xef"
        "\xff\xb5\xdf\xe7\x7b\xfe\xb9\x8d\xd9\xd7\x2e\xda\xa7\xfe\x7f\xb6\x63"
        "\x5d\x35\xff\x33\xdc\xb1\xee\xf9\x22\xe2\xd4\x6e\xef\x0b\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x1e\x13\xfd\xa9\x88\x1f\x45\x8a\xbf\x6d\xfc\xe3"
        "\xda\x39\xef\x37\x7e\xfe\x27\xbe\xd0\xae\xed\xfc\xfc\xdf\xbd\x6c\x7d"
        "\xfe\xff\x7b\x2f\xef\xc5\xfc\x7f\x00\x60\x7b\xe5\xef\xff\xf1\x48\xf1"
        "\x93\xfe\x2f\xa5\xf6\x77\xc8\xec\xe6\xf3\xff\x53\x9b\x72\xcd\x3e\x7d"
        "\xfe\xf7\x70\xc7\xba\xa9\xed\xe7\x35\x3f\xb4\x85\x5d\x3f\xc8\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x70\x9f\x52\x14\x71\x28\x52\xbc\xf3\xdd\x46\xba\x53\x94\x97"
        "\x5b\x7a\xcf\xd4\xe7\xae\x5e\x9b\x18\x1d\xdb\x7a\xb3\x77\xe7\x23\xa2"
        "\x2b\x8a\x66\x7d\xf9\xb7\x77\xf0\xd8\xd0\xf1\x97\x5f\x39\x31\xdc\xce"
        "\xed\xb7\x7f\xd8\x3e\x1b\xaf\x8f\x9f\x3f\x55\x3d\x3d\x3f\xbb\xb0\x38"
        "\xbd\xb4\x34\x3d\x55\x9d\x98\xab\x5f\x9c\x9f\x9a\xde\xf5\x2d\x3c\xe8"
        "\xf6\x9b\x1d\x6d\x3e\x00\xd5\xd9\xb7\xae\x4e\x5d\xba\xb4\x54\x3d\xf6"
        "\xd2\xd0\x86\xab\xaf\x55\xee\x1e\xfa\xd8\xe1\xca\xc9\xe1\x23\x43\x6f"
        "\x96\xb5\x23\x57\xaf\x55\x27\x46\xc7\xc6\xc6\x3b\x6a\xba\x7b\x36\xdf"
        "\x68\xdf\x7d\x0f\x27\xdd\xf7\x96\x3c\xc9\x9e\x8a\x22\x7e\x12\x29\xde"
        "\x7b\xf1\x83\xf4\x6f\x45\xd9\xd3\x0f\xde\x0b\x3b\x3c\x77\xec\xb5\xbe"
        "\xe8\x2e\xfb\xaf\xb9\x13\x13\xa3\x63\xcd\x1d\x99\xa9\xd7\xe6\x96\xcb"
        "\x2b\x53\x57\xae\xea\x8e\xa8\x74\x6c\x34\xd2\xee\x91\xdc\xb7\x3b\xf7"
        "\xe2\x3e\x19\x89\xb8\x5e\x3e\xf7\x96\x03\x3e\x5a\xee\xde\xf8\x42\x6d"
        "\xb1\x76\x61\x66\xba\x7a\xae\xb6\xb8\x5c\x5f\xae\xcf\xcf\xa5\xae\xd6"
        "\x68\xd3\x8f\x7f\x19\x95\xe8\x8a\xe1\x14\xb1\x10\x11\x8d\x62\xbf\x07"
        "\xcf\x41\xd3\x13\x45\xfc\x5d\xa4\xb8\xfb\x8b\x46\xfa\xf7\x22\xa2\x68"
        "\xf7\xc1\x0b\x67\xc7\xdf\x18\x18\xda\xf9\x06\xba\x1f\xc1\x20\xef\x71"
        "\xb7\x95\x22\x62\x25\x1e\x83\x9e\x85\x03\xea\x50\x14\xf1\x6c\xa4\xf8"
        "\xde\xe4\x40\xfc\x47\xd1\xea\xab\x66\xdb\xbc\x1f\xf1\xc5\x32\x5f\x8d"
        "\xb8\x52\xe6\xed\x14\x37\xf2\xe5\x54\x3e\x41\x0c\x47\xfc\xdc\xef\x13"
        "\x78\xac\x75\x47\x11\xe7\x22\xc5\x7c\x6a\xa4\xf7\x8b\xdc\xfb\xcd\xe3"
        "\xca\x33\x5f\xaf\xbe\x36\x77\x69\xbe\xa3\xb6\x7d\x5c\xf9\xd8\xbf\x3e"
        "\x78\x94\x1c\x9b\x70\x80\xf5\x46\x11\x3f\x6b\x1e\xf1\x37\xd2\xcf\xfc"
        "\x3e\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x03\xae\x88\x2f\x47"
        "\x8a\x5b\xb3\xcf\xa7\xe6\xfc\xd0\xb5\x39\xa5\xf5\xb9\xcb\xd5\xf3\xb5"
        "\x0b\x33\xad\x8f\xf5\xb7\x3f\xfb\x5f\xcd\x5b\xad\xae\xae\xae\x56\x7e"
        "\xfb\x6f\x9a\x0b\xd5\xd4\xba\x3c\x90\x73\x24\xe7\xb9\x9c\x93\x39\x17"
        "\x72\x5e\xcf\x79\x23\xe7\xcd\x9c\xb7\x72\xde\xce\xb9\x92\xf3\x4e\xce"
        "\x46\xce\xe8\xca\xf7\x9f\xb3\x9a\x73\x20\xe7\x48\xce\x73\x39\x27\x73"
        "\x2e\xe4\xbc\x9e\xf3\x46\xce\x9b\x39\x6f\xe5\xbc\x9d\x73\x25\xe7\x9d"
        "\x9c\x8d\x9c\xe1\x73\xd2\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\xec\x91\xae\x28\xe2\xdb\x91\xe2\xfb\xbf\x6a\xa4\xd5\xa2"
        "\x75\x7e\xd9\xc9\x68\xe5\x1d\xf3\x5c\xe1\x89\xf6\x7f\x01\x00\x00\xff"
        "\xff\xbc\x5b\x46\xbc",
        3184);
    syz_mount_image(
        /*fs=*/0x200000000080, /*dir=*/0x200000000280,
        /*flags=MS_LAZYTIME|MS_I_VERSION|MS_SYNCHRONOUS|MS_MANDLOCK*/ 0x2800050,
        /*opts=*/0x200000000040, /*chdir=*/1, /*size=*/0xc70,
        /*img=*/0x200000001940);
    break;
  case 1:
    memcpy((void*)0x200000000080, "memory.current\000", 15);
    res = syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x200000000080ul,
                  /*flags=*/0x275a, /*mode=*/0);
    if (res != -1)
      r[0] = res;
    break;
  case 2:
    memcpy((void*)0x2000000004c0, "#! ", 3);
    *(uint8_t*)0x2000000004c3 = 0xa;
    syscall(__NR_write, /*fd=*/r[0], /*data=*/0x2000000004c0ul,
            /*len=*/0x208e24bul);
    break;
  case 3:
    syscall(__NR_mmap, /*addr=*/0x200000000000ul, /*len=*/0xb36000ul,
            /*prot=PROT_WRITE*/ 2ul,
            /*flags=MAP_STACK|MAP_POPULATE|MAP_FIXED|MAP_SHARED*/ 0x28011ul,
            /*fd=*/r[0], /*offset=*/0ul);
    break;
  case 4:
    syscall(__NR_ftruncate, /*fd=*/r[0], /*len=*/1ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffffffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200000000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200001000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  const char* reason;
  (void)reason;
  loop();
  return 0;
}
