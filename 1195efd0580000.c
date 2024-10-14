// https://syzkaller.appspot.com/bug?id=e577e83a2928bb47693791855b6572d57cffa3be
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
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

static void loop(void)
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    memcpy((void*)0x20000040, "ext4\000", 5);
    memcpy((void*)0x20000200, "./file1\000", 8);
    *(uint8_t*)0x20000140 = 0;
    memcpy(
        (void*)0x200008c0,
        "\x78\x9c\xec\xdd\xdf\x6b\x5b\x55\x1c\x00\xf0\xef\x4d\xdb\xad\xfb\xa1"
        "\xed\x60\x0c\xf5\x41\x0a\x7b\x70\x32\x97\xae\xad\x3f\x26\xf8\x30\x1f"
        "\x45\x87\x03\x7d\xd7\x90\x64\x65\x34\x5d\x46\x93\x8e\xb5\x0e\xdc\x1e"
        "\xdc\x8b\x2f\x32\x04\x11\x07\xa2\xef\xbe\xfb\x38\xfc\x07\xfc\x2b\x06"
        "\x3a\x18\x32\x8a\x3e\xec\x25\x72\xd3\x9b\x2e\x5b\x93\x36\xed\xd2\xb5"
        "\x5b\x3e\x1f\xb8\xed\x39\xf7\xde\xf4\x9c\x6f\xee\xfd\x9e\x9e\x9b\x9b"
        "\x90\x00\x06\xd6\x44\xfa\x23\x17\xf1\x6a\x44\x7c\x97\x44\x8c\xb5\x6d"
        "\x1b\x8e\x6c\xe3\xc4\xea\x7e\x2b\x0f\xae\x15\xd3\x25\x89\x46\xe3\xb3"
        "\x7f\x92\x48\xb2\x75\xad\xfd\x93\xec\xf7\xa1\xac\xf2\x4a\x44\xfc\xf1"
        "\x4d\xc4\xc9\xdc\xfa\x76\x6b\x4b\xcb\x73\x85\x4a\xa5\xbc\x90\xd5\x27"
        "\xeb\xf3\x97\x27\x6b\x4b\xcb\xa7\x2e\xce\x17\x66\xcb\xb3\xe5\x4b\xd3"
        "\x33\x33\x67\xde\x99\x99\x7e\xff\xbd\x77\xfb\x16\xeb\x9b\xe7\xff\xfb"
        "\xf1\xd3\x3b\x1f\x9d\xf9\xf6\xf8\xca\x0f\xbf\xdd\x3b\x72\x2b\x89\xb3"
        "\x71\x38\xdb\xd6\x1e\xc7\x53\xb8\xde\x5e\x99\x88\x89\xec\x39\x19\x89"
        "\xb3\x4f\xec\x38\xd5\x87\xc6\xf6\x92\x64\xb7\x3b\xc0\xb6\x0c\x65\x79"
        "\x3e\x12\xe9\x18\x30\x16\x43\x59\xd6\x77\xd4\x18\x7b\x96\x5d\x03\x76"
        "\xd8\xd7\x69\x5a\x03\x03\x2a\x91\xff\x30\xa0\x5a\xf3\x80\xd6\xb5\x7d"
        "\x9f\xae\x83\x9f\x1b\xf7\x3f\x5c\xbd\x00\x5a\x1f\xff\xf0\xea\x6b\x23"
        "\x31\xda\xbc\x36\x3a\xb8\x92\x3c\x76\x65\x94\x5e\xef\x8e\xf7\xa1\xfd"
        "\xb4\x8d\xdf\xff\xbe\x7d\x2b\x5d\xa2\x7f\xaf\x43\x00\x6c\xea\xfa\x8d"
        "\x88\x38\x3d\x3c\xbc\x7e\xfc\x4b\xb2\xf1\x6f\xfb\x4e\xf7\xb0\xcf\x93"
        "\x6d\x18\xff\xe0\xd9\xb9\x93\xce\x7f\xde\xea\x34\xff\xc9\xad\xcd\x7f"
        "\xa2\xc3\xfc\xe7\x50\x87\xdc\xdd\x8e\xcd\xf3\x3f\x77\xaf\x0f\xcd\x74"
        "\x95\xce\xff\x3e\xe8\x38\xff\x5d\xbb\x69\x35\x3e\x94\xd5\x5e\x6a\xce"
        "\xf9\x46\x92\x0b\x17\x2b\xe5\x74\x6c\x7b\x39\x22\x4e\xc4\xc8\xfe\xb4"
        "\xbe\xc1\xfd\x9c\x2f\x73\x2b\x77\x1b\xdd\x36\xb6\xcf\xff\xd2\x25\x6d"
        "\xbf\x35\x17\xcc\xfa\x71\x6f\x78\xff\xe3\x8f\x29\x15\xea\x85\xa7\x0a"
        "\xba\xcd\xfd\x1b\x11\xaf\x75\x9c\xff\x26\x6b\xc7\x3f\xe9\x70\xfc\xd3"
        "\xe7\xe3\x7c\x8f\x6d\x1c\x2b\xdf\x7e\xbd\xdb\xb6\xcd\xe3\xdf\x59\x8d"
        "\x5f\x22\xde\xe8\x78\xfc\x1f\xdd\xd1\x4a\x36\xbe\x3f\x39\xd9\x3c\x1f"
        "\x26\x5b\x67\xc5\x7a\xff\xde\x3c\xf6\x67\xb7\xf6\x77\x3b\xfe\xf4\xf8"
        "\x1f\xdc\x38\xfe\xf1\xa4\xfd\x7e\x6d\x6d\xeb\x6d\xfc\x3c\xfa\xb0\xdc"
        "\x6d\xdb\x76\xcf\xff\x7d\xc9\xe7\xcd\xf2\xbe\x6c\xdd\xd5\x42\xbd\xbe"
        "\x30\x15\xb1\x2f\xf9\x64\xfd\xfa\xe9\x47\x8f\x6d\xd5\x5b\xfb\xa7\xf1"
        "\x9f\x38\xbe\xf1\xf8\xd7\xe9\xfc\x3f\x90\x26\x76\x8f\xf1\xdf\x3c\x7a"
        "\xb3\x7d\xd7\xd1\xad\xc5\xbf\xb3\xd2\xf8\x4b\x5b\x3a\xfe\x5b\x2f\xdc"
        "\xfd\xf8\xab\x9f\xba\xb5\xdf\xdb\xf1\x7f\xbb\x59\x3a\x91\xad\xe9\x65"
        "\xfc\xeb\xb5\x83\x4f\xf3\xdc\x01\x00\x00\x00\x00\x00\xc0\x5e\x93\x8b"
        "\x88\xc3\x91\xe4\xf2\x6b\xe5\x5c\x2e\x9f\x5f\x7d\x7f\xc7\xd1\x38\x98"
        "\xab\x54\x6b\xf5\x93\x17\xaa\x8b\x97\x4a\xd1\xfc\xac\xec\x78\x8c\xe4"
        "\x5a\x77\xba\xc7\xda\xde\x0f\x31\x95\xbd\x1f\xb6\x55\x9f\x7e\xa2\x3e"
        "\x13\x11\x47\x22\xe2\xfb\xa1\x03\xcd\x7a\xbe\x58\xad\x94\x76\x3b\x78"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xd8\x23"
        "\x0e\x45\x8c\x76\xfa\xfc\x7f\xea\xaf\xa1\xdd\xee\x1d\xb0\xe3\x36\xf8"
        "\xca\x6f\xe0\x05\xd7\x3d\xff\xb3\x2d\xfd\xf8\xa6\x27\x60\x4f\xf2\xff"
        "\x1f\x06\x97\xfc\x87\xc1\x25\xff\x61\x70\xc9\x7f\x18\x5c\xf2\x1f\x06"
        "\x97\xfc\x87\xc1\x25\xff\x61\x70\x6d\x25\xff\x7f\x3d\xb7\x83\x1d\x01"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x80\x17\xc3\xf9\x73\xe7\xd2\xa5\xb1\xf2\xe0\x5a\x31\xad"
        "\x97\xae\x2c\x2d\xce\x55\xaf\x9c\x2a\x95\x6b\x73\xf9\xf9\xc5\x62\xbe"
        "\x58\x5d\xb8\x9c\x9f\xad\x56\x67\x2b\xe5\x7c\xb1\x3a\xbf\xd9\xdf\xab"
        "\x54\xab\x97\xa7\xa6\x63\xf1\xea\x64\xbd\x5c\xab\x4f\xd6\x96\x96\xbf"
        "\x98\xaf\x2e\x5e\x7a\xd8\x58\x55\x1e\x79\x26\x51\x01\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xc0\xf3\xa5\xb6\xb4\x3c\x57"
        "\xa8\x54\xca\x0b\x0a\x0a\xdb\x2a\x0c\xef\x8d\x6e\x28\xf4\xb9\xb0\xdb"
        "\x23\x13\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
        "\x00\x00\x00\x00\x3c\xf2\x7f\x00\x00\x00\xff\xff\xbe\x62\x3f\xb0",
        1376);
    syz_mount_image(/*fs=*/0x20000040, /*dir=*/0x20000200,
                    /*flags=MS_RELATIME*/ 0x200000, /*opts=*/0x20000140,
                    /*chdir=*/0xfc, /*size=*/0x560, /*img=*/0x200008c0);
    break;
  case 1:
    memcpy((void*)0x200000c0, "blkio.throttle.io_service_bytes\000", 32);
    res = syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x200000c0ul,
                  /*flags=*/0x275a, /*mode=*/0);
    if (res != -1)
      r[0] = res;
    break;
  case 2:
    syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0xb36000ul,
            /*prot=PROT_WRITE*/ 2ul,
            /*flags=MAP_STACK|MAP_POPULATE|MAP_FIXED|MAP_SHARED*/ 0x28011ul,
            /*fd=*/r[0], /*offset=*/0ul);
    break;
  case 3:
    syscall(__NR_ftruncate, /*fd=*/r[0], /*len=*/0xc17aul);
    break;
  case 4:
    memcpy((void*)0x20000200, "/dev/loop", 9);
    *(uint8_t*)0x20000209 = 0x30;
    *(uint8_t*)0x2000020a = 0;
    syscall(__NR_quotactl, /*cmd=*/0xffffffff80000300ul,
            /*special=*/0x20000200ul, /*id=*/0, /*addr=*/0ul);
    break;
  case 5:
    memcpy((void*)0x200001c0, "/dev/fuse\000", 10);
    res = syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul,
                  /*file=*/0x200001c0ul, /*flags=*/2, /*mode=*/0);
    if (res != -1)
      r[1] = res;
    break;
  case 6:
    memcpy((void*)0x20000100, "./file1\000", 8);
    memcpy((void*)0x20000140, "fuse\000", 5);
    memcpy((void*)0x20002380, "fd", 2);
    *(uint8_t*)0x20002382 = 0x3d;
    sprintf((char*)0x20002383, "0x%016llx", (long long)r[1]);
    *(uint8_t*)0x20002395 = 0x2c;
    memcpy((void*)0x20002396, "rootmode", 8);
    *(uint8_t*)0x2000239e = 0x3d;
    sprintf((char*)0x2000239f, "%023llo", (long long)0x8000);
    *(uint8_t*)0x200023b6 = 0x2c;
    memcpy((void*)0x200023b7, "user_id", 7);
    *(uint8_t*)0x200023be = 0x3d;
    sprintf((char*)0x200023bf, "%020llu", (long long)0);
    *(uint8_t*)0x200023d3 = 0x2c;
    memcpy((void*)0x200023d4, "group_id", 8);
    *(uint8_t*)0x200023dc = 0x3d;
    sprintf((char*)0x200023dd, "%020llu", (long long)0);
    *(uint8_t*)0x200023f1 = 0x2c;
    *(uint8_t*)0x200023f2 = 0;
    syscall(__NR_mount, /*src=*/0ul, /*dst=*/0x20000100ul,
            /*type=*/0x20000140ul, /*flags=MS_NOSUID*/ 2ul,
            /*opts=*/0x20002380ul);
    break;
  case 7:
    syscall(__NR_read, /*fd=*/r[1], /*buf=*/0x20000200ul, /*len=*/0x2020ul);
    break;
  case 8:
    syscall(__NR_fcntl, /*fd=*/-1, /*cmd=F_SETLKW*/ 7ul, /*lock=*/0ul);
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
