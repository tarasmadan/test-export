// https://syzkaller.appspot.com/bug?id=015eb45ca4cbdffd42f4188b0cce3bf882fd46e1
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

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0, 0x0};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    memcpy((void*)0x20000100, "ext4\000", 5);
    memcpy((void*)0x200005c0, "./file0\000", 8);
    *(uint8_t*)0x20000080 = 0;
    memcpy(
        (void*)0x200010c0,
        "\x78\x9c\xec\xdd\xcf\x6f\x1c\x57\x1d\x00\xf0\xef\x8c\xed\xd8\x4d\xd2"
        "\x3a\x81\x1e\xa0\x02\x12\xa0\x10\x50\x94\x75\xbc\x69\xa3\xaa\x12\xb4"
        "\x5c\x40\xa8\xaa\x84\xa8\x38\x20\x90\x52\x63\x6f\x2c\x93\x75\x36\xd8"
        "\xeb\x52\x1b\x4b\xb8\x7f\x03\x48\x20\x71\x82\x3f\x81\x03\x12\x07\xa4"
        "\x9e\x38\x70\xe3\x88\xc4\x01\x21\xca\xa1\x52\x81\x08\x94\x20\x71\x18"
        "\x34\xb3\x63\x7b\xeb\xac\x9b\x8d\xf7\x57\xb3\xfb\xf9\x48\x93\xf9\xf1"
        "\xde\xec\xf7\xbd\xdd\xcc\xbc\xb7\x6f\xd7\xfb\x02\x98\x58\x17\x23\x62"
        "\x2f\x22\x4e\x45\xc4\xeb\x11\x31\x5f\x1e\x4f\xca\x25\x5e\x6e\x2d\x79"
        "\xbe\x7b\x77\x77\x97\xef\xdf\xdd\x5d\x4e\x22\xcb\x5e\xfb\x67\x52\xa4"
        "\xe7\xc7\xa2\xed\x9c\xdc\x99\xf2\x31\xe7\x22\xe2\x9b\x5f\x8b\xf8\x5e"
        "\xf2\x60\xdc\xcd\xed\x9d\x5b\x4b\xf5\x7a\x6d\xa3\xdc\x5f\x68\xae\xdf"
        "\x59\xd8\xdc\xde\xb9\xb2\xb6\xbe\xb4\x5a\x5b\xad\xdd\xae\x56\xaf\x2f"
        "\x5e\xbf\xfa\xc2\xb5\xe7\xab\x27\xac\xd9\x97\xe7\x8e\x1e\xb9\xb0\xfe"
        "\xeb\xf7\xbe\xba\xf6\xca\xb7\x7e\xf7\xdb\x4f\xbe\xf3\xc7\xbd\x2f\xfe"
        "\x28\x2f\xd6\xd9\x32\xad\xbd\x1e\xfd\xd4\xaa\xfa\xcc\x41\x9c\xdc\x74"
        "\x44\xbc\x32\x88\x60\x23\x30\x55\xae\x4f\x8d\xb8\x1c\x9c\x4c\x1a\x11"
        "\x1f\x89\x88\xcf\x14\xd7\xff\x7c\x4c\x15\xff\x3b\xbb\x31\x35\xe0\x92"
        "\x01\x00\x83\x92\x65\xf3\x91\xcd\xb7\xef\x03\x00\xe3\x2e\x2d\xc6\xc0"
        "\x92\xb4\x12\x11\x69\x5a\x76\x02\x2a\xad\x31\xbc\xa7\xe3\x74\x5a\x6f"
        "\x6c\x36\x2f\xdf\x6c\x6c\xdd\x5e\x69\x8d\x95\x9d\x8b\x99\xf4\xe6\x5a"
        "\xbd\x76\xf5\xfc\xec\x9f\x7f\x50\x64\x9e\x49\xf2\xfd\xc5\x22\xad\x48"
        "\x2f\xf6\xab\x47\xf6\xaf\x45\xc4\xf9\x88\xf8\xe9\xec\x13\xc5\x7e\x65"
        "\xb9\x51\x5f\x19\x4d\x97\x07\x00\x26\xde\x99\xf6\xf6\x3f\x22\xfe\x33"
        "\x9b\xa6\x95\x4a\x57\xa7\x76\xf8\x54\x0f\x00\x78\x6c\x3c\xf0\x4d\x19"
        "\x00\x60\xec\x69\xff\x01\x60\xf2\x68\xff\x01\x60\xf2\x74\xd1\xfe\x97"
        "\x1f\xf6\xef\x0d\xbc\x2c\x00\xc0\x70\x78\xff\x0f\x00\x93\x47\xfb\x0f"
        "\x00\x93\x47\xfb\x0f\x00\x13\xe5\x1b\xaf\xbe\x9a\x2f\xd9\xfd\xf2\xf7"
        "\xaf\x57\xde\xd8\xde\xba\xd5\x78\xe3\xca\x4a\x6d\xf3\x56\x65\x7d\x6b"
        "\xb9\xb2\xdc\xd8\xb8\x53\x59\x6d\x34\x56\x8b\xdf\xec\x59\x7f\xd8\xe3"
        "\xd5\x1b\x8d\x3b\x8b\xcf\xc5\xd6\x9b\x0b\xcd\xda\x66\x73\x61\x73\x7b"
        "\xe7\xc6\x7a\x63\xeb\x76\xf3\x46\xf1\xbb\xde\x37\x6a\x33\x43\xa9\x15"
        "\x00\xf0\x41\xce\x5f\x78\xfb\x4f\x49\x44\xec\xbd\xf8\x44\xb1\x44\xdb"
        "\x5c\x0e\xda\x6a\x18\x6f\x69\x1f\x73\x01\x8f\x97\x9e\x66\xf1\xd1\x41"
        "\x80\xc7\x5a\xb7\xb3\x7d\x01\xe3\xa7\xab\x26\xbc\xe8\x24\xfc\x61\xe0"
        "\x65\x01\x46\xa3\xe3\x8f\x79\xcf\x75\xdc\x7c\xbf\x9f\x3f\x42\x10\xdf"
        "\x33\x82\x0f\x95\x4b\x1f\xef\x7e\xfc\xdf\x1c\xcf\x30\x5e\x8c\xec\xc3"
        "\xe4\x3a\xd9\xf8\xff\x4b\x7d\x2f\x07\x30\x7c\x27\x1f\xff\xff\x6e\x5f"
        "\xcb\x01\x0c\x5f\x96\x25\x47\xe7\xfc\x3f\x75\x90\x04\x00\x8c\xa5\x1e"
        "\xbe\xc2\x97\xfd\xb8\x5f\x9d\x10\x60\xa4\x1e\x36\x99\x77\x5f\x3e\xff"
        "\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x80\x31\x73\x36\x22\xbe\x1f\x49\x5a\x29"
        "\xe6\x02\x4f\xf3\x7f\xd3\x4a\x25\xe2\xc9\x88\x38\x17\x33\xc9\xcd\xb5"
        "\x7a\xed\x6a\x44\x3c\x15\x17\x22\x62\x66\x36\xdf\x5f\x1c\x75\xa1\x01"
        "\x80\x1e\xa5\xef\x26\xe5\xfc\x5f\x97\xe6\x9f\x3d\x7b\x34\xf5\x54\xf2"
        "\xdf\xd9\x62\x1d\x11\x3f\xfc\xc5\x6b\x3f\x3b\x13\x11\x1b\x8b\xf9\xf1"
        "\x7f\x1d\x1c\x9f\xdd\x9f\x3e\xac\x7a\x78\x5e\x0f\xf3\x0a\x02\x00\xdd"
        "\xfb\x5b\x37\x99\xde\x5c\x6a\x36\x37\xaa\xe5\xba\xed\x8d\xfc\xbd\xbb"
        "\xbb\xcb\xfb\x4b\x79\xe8\x4b\x59\x36\x3b\xa0\xa2\x1e\x7a\xef\x2b\x07"
        "\x93\x8f\x2e\xdf\xbf\xbb\x5b\x2c\xad\x94\xe9\xc8\xb2\x2c\x8b\x98\x2b"
        "\xfa\x12\xa7\xff\x9d\xc4\x74\x79\xce\x5c\x44\x3c\x13\x11\x53\x7d\x88"
        "\xbf\xf7\x56\x44\x7c\xac\x53\xfd\x93\x62\x6c\xe4\x5c\x39\xf3\x69\x7b"
        "\xfc\x28\x63\x3f\x39\xd4\xf8\xe9\xfb\xe2\xa7\x45\x5a\x6b\x9d\x3f\x7d"
        "\x1f\x7d\xf4\xd0\x83\x7f\x71\xe1\x43\xee\xed\xfc\xfe\xf3\x72\xa7\xeb"
        "\x2f\x8d\x8b\xc5\xba\xf3\xf5\x3f\x57\xdc\xa1\x7a\x57\xdc\xff\xe6\x22"
        "\xf6\xef\x7d\xf7\xdb\xe2\x4f\x97\x91\xa6\x3a\xc4\xcf\xaf\xf9\x8b\xad"
        "\xb7\x5f\x0f\x8f\xf1\xdc\xef\xbf\xfe\xc0\xc1\x6c\xbe\x95\xf6\x56\xc4"
        "\x33\xd3\x9d\xe2\x27\x07\xf1\x93\x63\xe2\x3f\xdb\x65\x1d\xff\xf2\x89"
        "\x4f\xfd\xe4\xa5\x63\xd2\xb2\x5f\x46\x5c\x8a\xce\xf1\xdb\x63\x2d\x34"
        "\xd7\xef\x2c\x6c\x6e\xef\x5c\x59\x5b\x5f\x5a\xad\xad\xd6\x6e\x57\xab"
        "\xd7\x17\xaf\x5f\x7d\xe1\xda\xf3\xd5\x85\x62\x8c\x7a\x61\x7f\xa4\xfa"
        "\x41\xff\x78\xf1\xf2\x53\xc7\x95\x2d\xaf\xff\xe9\x63\xe2\xcf\x75\xac"
        "\xff\xe1\x53\xfe\xb9\x2e\xeb\xff\xab\xff\xbd\xfe\x9d\x4f\x1f\xee\xce"
        "\x1e\x8d\xff\x85\xcf\x76\x7e\xfd\x9f\xee\x18\xbf\x25\x6f\x13\x3f\xdf"
        "\x65\xfc\xa5\xd3\xbf\x39\x76\xfa\xee\x3c\xfe\x4a\xbc\xbb\x7b\x92\xd7"
        "\xff\x72\x97\xf1\xdf\xf9\xfb\xce\x4a\x97\x59\x01\x80\x21\xd8\xdc\xde"
        "\xb9\xb5\x54\xaf\xd7\x36\x7a\xda\xc8\xdf\x85\x3e\xc2\x59\xfb\x1d\x89"
        "\x87\x67\xce\x73\x75\x57\x8c\xfd\xee\x62\x6f\xd5\xf9\x6b\x14\x1b\x7d"
        "\x7a\x5a\x8e\xd9\xc8\x3b\x63\xdd\x64\x9e\xe9\xb9\x3a\x33\x03\xac\xc5"
        "\x07\x6d\x4c\x1f\xf4\x15\xfb\xfb\xc8\xdf\xce\x1f\x71\xc8\xd5\x49\xfb"
        "\x5e\x8b\x9e\x36\xee\x0d\x2b\xd6\x68\xee\x47\xc0\xf0\x1c\x5e\xf4\xa3"
        "\x2e\x09\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x70\x9c\x61\xfc"
        "\xe9\xd2\xa8\xeb\x08\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\xc0\xf8\xfa\x7f\x00\x00\x00\xff\xff\x9f\xe3\xc2\x91",
        1392);
    syz_mount_image(
        /*fs=*/0x20000100, /*dir=*/0x200005c0,
        /*flags=MS_LAZYTIME|MS_REC|MS_SILENT|MS_NOSUID|MS_NODIRATIME|0x400*/
        0x200cc02, /*opts=*/0x20000080, /*chdir=*/1, /*size=*/0x570,
        /*img=*/0x200010c0);
    break;
  case 1:
    memcpy((void*)0x20000100, "./file0\000", 8);
    syscall(__NR_mkdir, /*path=*/0x20000100ul, /*mode=*/0ul);
    break;
  case 2:
    memcpy((void*)0x20000040, ".\000", 2);
    res = syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000040ul,
                  /*flags=*/0, /*mode=*/0);
    if (res != -1)
      r[0] = res;
    break;
  case 3:
    memcpy((void*)0x20003480, "/sys/kernel/debug/binder/transactions\000", 38);
    res = syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul,
                  /*file=*/0x20003480ul, /*flags=*/0, /*mode=*/0);
    if (res != -1)
      r[1] = res;
    break;
  case 4:
    res =
        syscall(__NR_read, /*fd=*/r[1], /*buf=*/0x20000480ul, /*len=*/0x2020ul);
    if (res != -1) {
      r[2] = *(uint32_t*)0x20000490;
      r[3] = *(uint32_t*)0x20000494;
    }
    break;
  case 5:
    syscall(__NR_fchown, /*fd=*/r[0], /*uid=*/r[2], /*gid=*/r[3]);
    break;
  case 6:
    memcpy((void*)0x20000cc0, "./file0\000", 8);
    syscall(__NR_mkdirat, /*fd=*/0xffffff9c, /*path=*/0x20000cc0ul,
            /*mode=*/0ul);
    break;
  case 7:
    memcpy((void*)0x20000100, "./file1\000", 8);
    syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000100ul,
            /*flags=O_SYNC|O_CREAT|FASYNC|O_RDWR*/ 0x103042, /*mode=*/0);
    break;
  case 8:
    memcpy((void*)0x20000240, "./file0\000", 8);
    memcpy((void*)0x200004c0, "./file0\000", 8);
    syscall(__NR_rename, /*old=*/0x20000240ul, /*new=*/0x200004c0ul);
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
  loop();
  return 0;
}
