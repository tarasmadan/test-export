// https://syzkaller.appspot.com/bug?id=dcfbf59913ad505145968e89d3e4fdb58b2bdbec
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

static void setup_sysctl()
{
  int cad_pid = fork();
  if (cad_pid < 0)
    exit(1);
  if (cad_pid == 0) {
    for (;;)
      sleep(100);
  }
  char tmppid[32];
  snprintf(tmppid, sizeof(tmppid), "%d", cad_pid);
  struct {
    const char* name;
    const char* data;
  } files[] = {
      {"/sys/kernel/debug/x86/nmi_longest_ns", "10000000000"},
      {"/proc/sys/kernel/hung_task_check_interval_secs", "20"},
      {"/proc/sys/net/core/bpf_jit_kallsyms", "1"},
      {"/proc/sys/net/core/bpf_jit_harden", "0"},
      {"/proc/sys/kernel/kptr_restrict", "0"},
      {"/proc/sys/kernel/softlockup_all_cpu_backtrace", "1"},
      {"/proc/sys/fs/mount-max", "100"},
      {"/proc/sys/vm/oom_dump_tasks", "0"},
      {"/proc/sys/debug/exception-trace", "0"},
      {"/proc/sys/kernel/printk", "7 4 1 3"},
      {"/proc/sys/kernel/keys/gc_delay", "1"},
      {"/proc/sys/vm/oom_kill_allocating_task", "1"},
      {"/proc/sys/kernel/ctrl-alt-del", "0"},
      {"/proc/sys/kernel/cad_pid", tmppid},
  };
  for (size_t i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
    if (!write_file(files[i].name, files[i].data)) {
    }
  }
  kill(cad_pid, SIGKILL);
  while (waitpid(cad_pid, NULL, 0) != cad_pid)
    ;
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

void execute_one(void)
{
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  memcpy((void*)0x20000ec0, "nilfs2\000", 7);
  memcpy((void*)0x20000f00, "./file0\000", 8);
  memcpy((void*)0x20000840, "barrier", 7);
  *(uint8_t*)0x20000847 = 0x2c;
  memcpy((void*)0x20000848, "nodiscard", 9);
  *(uint8_t*)0x20000851 = 0x2c;
  memcpy((void*)0x20000852, "norecovery", 10);
  *(uint8_t*)0x2000085c = 0x2c;
  memcpy((void*)0x2000085d, "order=strict", 12);
  *(uint8_t*)0x20000869 = 0x2c;
  memcpy((void*)0x2000086a, "nobarrier", 9);
  *(uint8_t*)0x20000873 = 0x2c;
  memcpy((void*)0x20000874, "order=strict", 12);
  *(uint8_t*)0x20000880 = 0x2c;
  memcpy((void*)0x20000881, "nodiscard", 9);
  *(uint8_t*)0x2000088a = 0x2c;
  memcpy((void*)0x2000088b, "order=relaxed", 13);
  *(uint8_t*)0x20000898 = 0x2c;
  *(uint8_t*)0x20000899 = 0x2c;
  memcpy(
      (void*)0x20000f40,
      "\x78\x9c\xec\xdd\x4f\x6c\x1c\xd5\x19\x00\xf0\x37\xeb\xbf\x89\x4d\xbc\x06"
      "\x0a\x06\x4a\x48\xa1\x15\x81\x82\x1d\x92\x48\x4d\x6f\x41\xa0\x1e\x11\x97"
      "\xde\x41\x21\xa1\x11\x86\xa2\x86\x1e\x88\xf8\x13\x7a\x40\x54\x42\x14\x09"
      "\xd1\x4b\xc5\x81\x8a\x0b\xa5\x52\x8a\xd4\x4a\xa0\x4a\x15\xea\xa9\xed\xa9"
      "\x55\x6f\x3d\xa1\x5e\xa8\x54\xa5\x52\x50\x0f\x6d\xa4\xc4\x55\x9c\xf7\xec"
      "\xf5\x4b\x5e\x77\x3d\xb1\x67\xbd\xbb\xbf\x9f\xf4\xed\xdb\x37\x6f\x76\xbe"
      "\x6f\xbc\x91\x33\x33\x9e\x7d\x1b\x80\x91\xd5\x5a\x7d\x3c\x7c\x78\xa1\x0a"
      "\xe1\xdd\x4f\xdf\x79\xf4\xe5\xa7\xaa\xdf\x5c\x5e\x76\xd7\xda\x1a\xfb\x56"
      "\x1f\xab\xd8\x6b\x87\x10\x26\x3a\xfa\x55\xb6\xbd\xcf\xe3\x82\x8b\xe7\x5f"
      "\x3a\x76\xad\xb6\x0a\x07\x57\x1f\x53\x3f\x3c\x76\x6e\xed\xb5\x33\x21\x84"
      "\x33\x61\x5f\xf8\x2c\xb4\xc3\x47\x4b\xcb\x5f\x7e\xf8\xde\x23\xfb\x3f\x7e"
      "\x7d\xfa\x96\xb7\x4e\x3f\xf3\xca\x36\xed\xfe\x9a\x7c\x3f\x00\x00\x60\x18"
      "\x9d\xfd\xd3\xf2\xdf\xee\xfb\xc7\x1f\x1f\x98\xbf\x70\x76\xef\xd1\x30\xb5"
      "\xb6\x3c\x1d\x9f\xb7\x63\x7f\x26\x1e\xf7\x1f\x88\x07\xca\xe9\x78\xb9\x15"
      "\x36\xf6\xab\x8e\xe8\x34\x99\xad\x37\x16\xa3\x95\xad\x37\x96\xad\x37\x9e"
      "\xe5\x19\x2f\xe4\x9b\xc8\xb6\x33\x51\x58\x6f\xb2\x4b\xbe\xb1\x8e\x65\xd7"
      "\xda\x4f\x00\x00\x00\x18\x44\xe9\xbc\xb6\x1d\xaa\xd6\xe2\x86\x7e\xab\xb5"
      "\xb8\x78\xe5\xbc\xff\xb2\xcf\xe7\x26\xab\xc5\xe7\x4e\x2e\x9f\x38\xd5\xa7"
      "\x42\x01\x00\x00\x80\xda\xfe\xfd\xea\xea\x4d\xb7\x42\x08\x21\x84\x10\x42"
      "\x08\x21\xc4\xb6\xc7\xae\x74\x32\xda\xeb\x6b\x36\xbb\xbe\x28\xc6\xca\x5c"
      "\x7f\xae\x3b\x00\x00\x00\x00\xa3\x2b\x9f\x2f\xec\x2a\x67\xb6\x76\xa6\xae"
      "\xb5\xad\xb5\x7b\xcb\x7f\xee\xe1\xd6\xb5\x5f\x0f\x5b\xa0\xe9\x7f\xff\xf2"
      "\x0f\x56\xfe\x0f\x5e\xf3\x1b\x07\x00\x80\xfa\x86\xf5\x68\x32\xed\x57\x3a"
      "\x8e\x4e\xf3\x18\xe4\xf3\x08\x8e\x65\xaf\xdb\xec\xf1\x7f\x2b\xdb\xce\xf8"
      "\x26\xeb\x2c\xcd\x2b\x38\x28\xf3\x0d\x96\xea\xcc\x7f\xae\x3b\x55\xa9\xfe"
      "\xcd\xbe\x8f\xfd\x52\xaa\x3f\x9f\x0f\x73\xa7\x2a\xd5\x9f\xcf\xd3\xb9\x53"
      "\x95\xea\x9f\x6a\xb8\x8e\xba\x4a\xf5\x4f\x37\x5c\x47\x5d\xa5\xfa\x77\x35"
      "\x5c\x47\x5d\xa5\xfa\x77\x37\x5c\x47\x5d\xa5\xfa\x67\x1a\xae\xa3\xae\x52"
      "\xfd\xb3\x0d\xd7\x51\x57\xa9\xfe\x1b\x1a\xae\xa3\xae\x52\xfd\x7b\x1a\xae"
      "\xa3\xae\x52\xfd\x83\x72\x5b\x6d\xa9\xfe\x76\xc3\x75\xd4\x55\xaa\x7f\xbe"
      "\xe1\x3a\xea\x2a\xd5\x7f\x63\xc3\x75\xd4\x55\xaa\xff\xa6\x86\xeb\xa8\xab"
      "\x54\xff\xcd\x0d\xd7\xd1\x2f\x77\xc6\x36\xfd\x1c\xf6\x66\xe3\x9d\xe7\xcf"
      "\xf9\x39\xdd\xa0\x9c\xe3\x01\x00\x00\xc0\xa8\xfb\xaf\xf9\xff\x84\x10\x42"
      "\x08\x21\x86\x23\xd2\x1f\x74\xfa\x5d\x87\x10\x42\x88\x1d\x19\xaf\xf6\xf9"
      "\xfa\x03\x00\x00\x00\xd0\x7f\xe9\x32\x72\xfa\xd4\xfb\x4a\x94\xc6\xc7\xba"
      "\x8c\x8f\x77\x19\x9f\xe8\x32\x3e\xd9\x65\x7c\xaa\xcb\x38\x00\x00\x00\x10"
      "\xc2\x6f\xdf\x38\x71\xdb\xdb\xd5\xfa\xe7\xfc\xaf\x77\x3e\xbc\x34\x6f\x54"
      "\x9a\x7f\x69\xb3\xf3\x18\xe5\xf3\x11\x6e\x36\xff\xf5\xce\x7b\x76\xbd\xf9"
      "\x07\x65\xde\x32\x00\x00\x00\x46\x4b\xf5\x9d\xcf\x2e\xdd\xff\xe8\xfb\x2f"
      "\xcc\x5f\x38\xbb\xf7\x68\xc7\xd9\xef\xa5\x78\xbe\x9b\xe6\x01\x1d\x8f\xd7"
      "\x06\x3e\x89\xfd\x74\x5f\xc0\x6c\xd6\xaf\xd2\x39\xf4\xd1\x8d\x79\x5a\x85"
      "\xf5\xf2\xeb\x03\x37\x94\xb6\xf7\xf8\x75\xee\x28\x00\x00\x00\x8c\xb0\x74"
      "\xfe\xde\x0e\x55\x6b\xb1\xe3\xbc\xbb\x1d\x5a\xad\xc5\xc5\xf5\xf3\xf1\x85"
      "\x30\x51\x9d\x38\xb9\x7c\xfc\x40\xec\xa7\xef\x67\xf9\xc3\xdc\xc4\xd4\xe5"
      "\xe5\x0f\x35\x5c\x37\x00\x00\x00\xd0\xbb\xf5\xf3\xfd\x6b\x9f\xff\xa7\xef"
      "\xf1\x5d\x08\x93\xd5\xe2\x73\x27\x97\x4f\x9c\xba\xd2\x9f\x5d\x5b\x3e\xd1"
      "\xea\xbc\x2e\x30\xb7\xbe\xbc\xea\xbc\x2e\xd0\xce\x96\x1f\x2c\x2c\x3f\x14"
      "\xfb\xe9\xfb\x3b\xbf\x37\xb7\x6b\x75\xf9\xe2\xb1\xef\x2f\x3f\xb5\xd5\x3b"
      "\x0f\x00\x00\x00\x23\xe2\xd4\x8b\xa7\x9f\x79\x72\x79\xf9\xf8\x0f\x3c\xf1"
      "\xc4\x13\x4f\xd6\x9e\xf4\xfb\x37\x13\x00\x00\xb0\xd5\xbe\xf8\xe2\x9d\x89"
      "\x1f\x1e\x9a\xfd\xdd\x95\xcf\xff\xaf\xcf\x7f\x97\x3e\xff\xbf\x2f\xf6\xdb"
      "\x71\x6e\xbf\x3f\xc7\x15\xd2\x7d\x02\xe9\x73\x00\x57\x7d\x5e\xff\x89\x8d"
      "\x79\xe6\x4a\xeb\x3d\xbf\x71\xbd\x76\xb6\xde\x58\x8c\xa9\xac\xee\xe9\x8e"
      "\xed\x84\x8e\xf9\x06\xd3\xeb\xe6\x4b\xf9\xda\x1b\xb7\x33\x59\xc8\x37\x93"
      "\xe5\x9b\xcd\xf2\xe5\xf3\x14\x8c\x67\xeb\xa7\x7c\x7b\xb2\xe5\xf9\xfc\x84"
      "\x69\xbd\xb9\x6c\x79\x3e\x0f\xe3\x78\x96\xa3\xca\xf2\xdf\x1d\x00\x00\x00"
      "\xa0\x6c\xe9\x85\x67\x9f\x5f\x3a\xf5\xe2\xe9\x07\x4f\x3e\xfb\xe4\xd3\xc7"
      "\x9f\x3e\xfe\xdc\xa1\x83\x47\xbe\x7d\xe4\xc8\x81\x87\xbe\xf5\xd0\xd2\xea"
      "\x7d\xfd\x4b\x9d\x77\xf7\x03\x00\x00\x00\x83\x68\xfd\xa6\xdf\x7e\x57\x02"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xa3\xab\x89"
      "\xaf\x13\xeb\xf7\x3e\x02\x00\x00\xc0\xa8\xfb\xd7\xab\x21\x84\x33\x42\x88"
      "\x42\x5c\xf9\x0a\xcc\xe6\xf3\xbe\xd9\xa7\xbc\x42\x08\x21\x84\x18\xe4\x18"
      "\xdb\x01\x35\x08\x21\x76\x6c\xac\xac\xe4\xdf\x34\x0f\x00\x00\x00\xb0\xbd"
      "\x2e\x9e\x7f\xe9\x58\x67\x7b\x95\x33\xd5\x96\xe6\xab\xb2\x6d\x5f\x8a\x79"
      "\x53\x3b\xfb\xe0\x5f\xe7\x2f\x47\x5a\xe5\xdc\xc3\x1b\xaf\x97\xec\xde\xd2"
      "\x6a\x18\x75\x4d\xff\xfb\x97\x7f\xb0\xf2\x7f\xf0\xda\xd6\xe6\x9f\x4e\x4f"
      "\xda\x57\x9a\xee\xbf\xff\x5a\x1b\x37\x70\xb4\x5e\xde\x7b\x97\x7e\xb1\xd0"
      "\x99\xff\xf6\xf1\x1e\xf3\xe7\xfb\xff\x78\xbd\xfc\xfb\xb3\xfc\xf7\x86\xde"
      "\xf2\xaf\xbc\x9f\xe5\x7f\xa2\x5e\xfe\xfb\xb2\xfc\xbb\x7b\xcc\x7f\xd5\xfe"
      "\x3f\x5f\x2f\xff\xfd\x31\xff\x42\xec\xef\xbf\xa7\xd7\xfc\x1b\xdf\xff\xa9"
      "\xd8\xa6\xfd\xd8\xd5\x63\xfe\x6f\x66\xfb\xff\x54\xe8\x35\x7f\xb6\xff\xed"
      "\x1e\x13\x66\x1e\x88\xf9\x01\x60\x14\xb5\xfa\x5d\xc0\x36\x49\x47\x09\xe9"
      "\x38\x7a\x26\xf6\xd3\xfe\xc6\xc3\xcd\x90\xdf\xfd\xb0\xd9\xe3\xff\x56\xb6"
      "\x9d\xf1\xeb\xae\x7c\xe3\x76\xd3\x71\xd0\xad\xb1\x9f\x8e\x97\x66\xb3\xbc"
      "\x49\xcf\xf5\xff\xf4\x3f\x2b\x2b\x1d\x3f\x97\xb4\xbd\x1b\x6a\xd6\x99\x1b"
      "\x94\xbb\x4a\x4a\xf5\x6f\xd5\xfb\xb8\xdd\x4a\xf5\x4f\x34\x5c\x47\x5d\xa5"
      "\xfa\x27\x1b\xae\xa3\xae\x52\xfd\x53\x0d\xd7\x51\x57\xa9\xfe\xe9\x86\xeb"
      "\xa8\xab\x54\x7f\xaf\xe7\xa1\xfd\x56\xaa\x7f\x50\xae\x2b\x97\xea\x9f\x69"
      "\xb8\x8e\xba\x4a\xf5\xcf\x36\x5c\x47\x5d\xa5\xfa\x37\xfb\xff\x78\xbf\x94"
      "\xea\xdf\xd3\x70\x1d\x75\x95\xea\x9f\x6b\xb8\x8e\xba\x4a\xf5\xd7\xbc\xac"
      "\xd6\xb8\x52\xfd\xf3\x0d\xd7\x51\x57\xa9\xfe\x1b\x1b\xae\xa3\xae\x52\xfd"
      "\x37\x35\x5c\x47\x5d\xa5\xfa\x6f\x6e\xb8\x8e\x7e\xb9\x23\xb6\xa5\xf3\xe1"
      "\x74\xfe\x39\x17\xc7\x52\xbf\x9d\xf5\xa7\xae\xf1\xb3\x1c\xd6\x6b\x0b\x00"
      "\x00\x00\x30\x68\xfe\x69\xfe\x3f\x21\x86\x33\x2e\x3f\xf6\xbb\x06\xb1\x85"
      "\xd1\xf1\x57\xed\xbe\xd7\x22\x84\x10\xa3\x12\x77\xef\xf1\x7b\x57\x08\x31"
      "\x4c\xb1\x7a\x13\x38\x23\x6b\x7b\x3f\xcd\x0c\xc0\x4e\xe5\xf7\xff\x68\xf3"
      "\xfe\x8f\x36\xef\xff\x68\xf3\xfe\xf3\xff\xa4\x7b\xf8\xab\xac\x9f\x8c\x75"
      "\x19\x1f\xef\x32\x3e\xd1\x65\x7c\x32\x1b\xcf\xff\xbd\x4e\x75\x19\xbf\x29"
      "\xdb\xee\x4a\x94\xc6\x6f\xee\x32\xfe\x95\x2e\xe3\x7b\xba\x8c\xdf\xda\x65"
      "\x7c\xa1\xcb\xf8\x6d\x5d\xc6\x6f\xef\x32\x7e\x47\x97\x71\x00\x00\x00\x46"
      "\xc3\x2d\xb1\x75\x7e\x08\x00\x00\x00\xc3\xeb\xe5\x5f\x7e\xf2\xe6\xaf\xef"
      "\x7d\xe2\xfc\xfc\x85\xb3\x7b\x8f\x86\xc9\xab\xe6\x9d\x3f\x10\xfb\x53\xf1"
      "\x6f\xeb\x6f\xc4\x7e\x3e\xef\x7d\x32\x11\xff\xe6\xff\xa3\xd8\xff\x79\x6c"
      "\x7f\x1f\xdb\xbf\x67\xeb\xbb\xff\x04\x00\x00\x00\xb6\x5f\xfa\x9e\x18\x7f"
      "\xff\x07\x00\x00\x80\xe1\x95\xbe\xa7\xd4\xf9\x3f\x00\x00\x00\x0c\xaf\xf9"
      "\xd8\x3a\xff\x07\x00\x00\x80\xe1\x75\x63\x6c\x9d\xff\x03\x00\x00\xc0\x10"
      "\xab\xa6\xaf\xbd\x38\xb6\xe9\xba\xc0\xdd\xb1\xed\x75\x5e\x3f\x00\x60\xe7"
      "\xfb\x6a\x6c\xef\x8c\xed\xde\xd8\xde\x15\xdb\xaf\xc5\x36\x1d\x07\xdc\x13"
      "\xdb\xaf\x37\x54\x1f\x00\xb0\x75\x7e\xf6\xdd\x1f\x1f\x79\xbb\x5a\x9f\xef"
      "\xff\x50\x36\x7e\x31\x2e\x4f\xed\x55\xce\x5c\xb9\x52\x50\xb5\x36\xce\xe4"
      "\xbf\x2b\xb6\xbb\x63\xfb\x8d\x1e\xeb\xc9\xbf\x0f\xa0\xd7\xfc\xc9\x9e\x1e"
      "\xf3\x6c\x57\xfe\xb9\xeb\xcc\x0f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0c\x8f\xd6\xea\xe3\xe1"
      "\xc3\x0b\x55\x08\xef\x7e\xfa\xce\xa3\x3f\x99\x7c\xf3\x2f\x97\x97\xdd\xb5"
      "\xb6\xc6\xbe\xd5\xc7\x2a\xf6\xda\x21\x84\x89\xb5\xd7\xa5\xd1\xf5\xfe\xaf"
      "\xe2\x8a\x17\xcf\xbf\x74\xac\xb3\xbd\x14\xdb\x2a\x1c\x0c\x55\xa8\xd6\x96"
      "\x87\xc7\xce\xad\x65\x9a\x09\x21\x9c\x09\xfb\xc2\x67\xa1\x1d\x3e\x5a\x5a"
      "\xfe\xf2\xc3\xf7\x1e\xd9\xff\xf1\xeb\xd3\xb7\xbc\x75\xfa\x99\x57\xb6\xf1"
      "\x47\xb0\x61\xff\x00\x00\x00\x60\x18\xfd\x2f\x00\x00\xff\xff\x4e\xae\x1e"
      "\x19",
      3817);
  syz_mount_image(
      /*fs=*/0x20000ec0, /*dir=*/0x20000f00,
      /*flags=MS_LAZYTIME|MS_POSIXACL|MS_SYNCHRONOUS|MS_STRICTATIME|MS_RELATIME|0x42*/
      0x3210052, /*opts=*/0x20000840, /*chdir=*/3, /*size=*/0xee9,
      /*img=*/0x20000f40);
  memcpy((void*)0x20000080, "cgroup.controllers\000", 19);
  syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000080ul,
          /*flags=*/0x275a, /*mode=*/0);
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
  setup_sysctl();
  const char* reason;
  (void)reason;
  loop();
  return 0;
}