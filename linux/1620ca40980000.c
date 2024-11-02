// https://syzkaller.appspot.com/bug?id=c4e5450bc09c95f49c37dfd12936def60d8738cc
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <setjmp.h>
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
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <linux/capability.h>
#include <linux/loop.h>

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

static unsigned long long procid;

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

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
}

static void setup_binderfs()
{
  if (mkdir("/dev/binderfs", 0777)) {
  }
  if (mount("binder", "/dev/binderfs", "binder", 0, NULL)) {
  }
  if (symlink("/dev/binderfs", "./binderfs")) {
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setsid();
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = (200 << 20);
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 32 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 128 << 20;
  setrlimit(RLIMIT_CORE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 256;
  setrlimit(RLIMIT_NOFILE, &rlim);
  if (unshare(CLONE_NEWNS)) {
  }
  if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(0x02000000)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
  typedef struct {
    const char* name;
    const char* value;
  } sysctl_t;
  static const sysctl_t sysctls[] = {
      {"/proc/sys/kernel/shmmax", "16777216"},
      {"/proc/sys/kernel/shmall", "536870912"},
      {"/proc/sys/kernel/shmmni", "1024"},
      {"/proc/sys/kernel/msgmax", "8192"},
      {"/proc/sys/kernel/msgmni", "1024"},
      {"/proc/sys/kernel/msgmnb", "1024"},
      {"/proc/sys/kernel/sem", "1024 1048576 500 1024"},
  };
  unsigned i;
  for (i = 0; i < sizeof(sysctls) / sizeof(sysctls[0]); i++)
    write_file(sysctls[i].name, sysctls[i].value);
}

static int wait_for_loop(int pid)
{
  if (pid < 0)
    exit(1);
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static void drop_caps(void)
{
  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    exit(1);
  const int drop = (1 << CAP_SYS_PTRACE) | (1 << CAP_SYS_NICE);
  cap_data[0].effective &= ~drop;
  cap_data[0].permitted &= ~drop;
  cap_data[0].inheritable &= ~drop;
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    exit(1);
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);
  setup_common();
  sandbox_common();
  drop_caps();
  if (unshare(CLONE_NEWNET)) {
  }
  write_file("/proc/sys/net/ipv4/ping_group_range", "0 65535");
  setup_binderfs();
  loop();
  exit(1);
}

uint64_t r[1] = {0xffffffffffffffff};

void loop(void)
{
  intptr_t res = 0;
  memcpy((void*)0x20000040, "ext4\000", 5);
  memcpy((void*)0x20000000, "./bus\000", 6);
  memcpy((void*)0x20000200, "quota", 5);
  *(uint8_t*)0x20000205 = 0x2c;
  memcpy((void*)0x20000206, "oldalloc", 8);
  *(uint8_t*)0x2000020e = 0x2c;
  memcpy((void*)0x2000020f, "barrier", 7);
  *(uint8_t*)0x20000216 = 0x3d;
  sprintf((char*)0x20000217, "0x%016llx", (long long)3);
  *(uint8_t*)0x20000229 = 0x2c;
  memcpy((void*)0x2000022a, "debug_want_extra_isize", 22);
  *(uint8_t*)0x20000240 = 0x3d;
  sprintf((char*)0x20000241, "0x%016llx", (long long)0x80);
  *(uint8_t*)0x20000253 = 0x2c;
  memcpy((void*)0x20000254, "block_validity", 14);
  *(uint8_t*)0x20000262 = 0x2c;
  memcpy((void*)0x20000263, "jqfmt=vfsv1", 11);
  *(uint8_t*)0x2000026e = 0x2c;
  *(uint8_t*)0x2000026f = 0;
  memcpy(
      (void*)0x20000940,
      "\x78\x9c\xec\xdb\xcf\x6f\x14\x55\x1c\x00\xf0\xef\xcc\xb6\x45\x28\xd8\x8a"
      "\xf8\x83\x82\x5a\x45\x63\xe3\x8f\x96\x16\x54\x0e\x5e\x34\x9a\x78\xd0\xc4"
      "\x44\x0f\x78\xac\x6d\x21\x95\x85\x1a\x5a\x13\x21\x8d\x56\x63\xf0\x68\x48"
      "\xbc\x1b\x8f\x26\xfe\x05\x9e\xf4\x62\xd4\x93\x89\x57\xbd\x1b\x12\x62\x7a"
      "\x01\x3d\xad\x99\xdd\x99\x76\xbb\xdd\x2d\x6d\xd9\x76\xd1\xfd\x7c\x92\x81"
      "\xf7\x66\xde\xe6\x7d\xbf\x3b\xf3\x76\xdf\xcc\xeb\x06\xd0\xb5\x86\xb3\x7f"
      "\x92\x88\xfd\x11\xf1\x7b\x44\x0c\xd4\xaa\x6b\x1b\x0c\xd7\xfe\xbb\xb9\xbc"
      "\x38\xf5\xf7\xf2\xe2\x54\x12\x95\xca\x5b\x7f\x25\xd5\x76\x37\x96\x17\xa7"
      "\x8a\xa6\xc5\xeb\xfa\xf3\xca\x48\x1a\x91\x7e\x96\xc4\x91\x26\xfd\xce\x5f"
      "\xba\x7c\x6e\xb2\x5c\x9e\xb9\x98\xd7\xc7\x16\xce\xbf\x3f\x36\x7f\xe9\xf2"
      "\xb3\xb3\xe7\x27\xcf\xce\x9c\x9d\xb9\x30\x71\xea\xd4\xc9\x13\xe3\x2f\x3c"
      "\x3f\xf1\x5c\x5b\xf2\xcc\x62\xba\x31\xf4\xd1\xdc\xd1\xc3\xaf\xbd\x73\xf5"
      "\x8d\xa9\xd3\x57\xdf\xfd\xf9\xdb\xa4\xc8\xbf\x21\x8f\x36\x19\xde\xe8\xe0"
      "\x13\x95\x4a\x9b\xbb\xeb\xac\x03\x75\xe5\xa4\xa7\x83\x81\xb0\x25\xa5\x88"
      "\xc8\x4e\x57\x6f\x75\xfc\x0f\x44\x29\x56\x4f\xde\x40\xbc\xfa\x69\x47\x83"
      "\x03\x76\x54\xa5\x52\xa9\xf4\xb7\x3e\xbc\x54\x01\xfe\xc7\x92\xe8\x74\x04"
      "\x40\x67\x14\x5f\xf4\xd9\xfd\x6f\xb1\xed\xd2\xd4\xe3\x8e\x70\xfd\xa5\xda"
      "\x0d\x50\x96\xf7\xcd\x7c\xab\x1d\xe9\x89\x34\x6f\xd3\xdb\x70\x7f\xdb\x4e"
      "\xc3\x11\x71\x7a\xe9\x9f\xaf\xb2\x2d\x76\xe6\x39\x04\x00\xc0\x1a\xdf\x67"
      "\xf3\x9f\x67\x9a\xcd\xff\xd2\xb8\xbf\xae\xdd\xdd\xf9\xda\xd0\x60\x44\xdc"
      "\x13\x11\x07\x23\xe2\xde\x88\x38\x14\x11\xf7\x45\x54\xdb\x3e\x10\x11\x0f"
      "\x6e\xb1\xff\xc6\x45\x92\xf5\xf3\x9f\xf4\xda\xb6\x12\xdb\xa4\x6c\xfe\xf7"
      "\x62\xbe\xb6\xb5\x76\xfe\x57\xcc\xfe\x62\xb0\x94\xd7\x0e\x54\xf3\xef\x4d"
      "\xce\xcc\x96\x67\x8e\xe7\xef\xc9\x48\xf4\xee\xc9\xea\xe3\x1b\xf4\xf1\xc3"
      "\x2b\xbf\x7d\xd1\xea\x58\xfd\xfc\x2f\xdb\xb2\xfe\x8b\xb9\x60\x1e\xc7\xb5"
      "\x9e\x3d\x6b\x5f\x33\x3d\xb9\x30\x79\x3b\x39\xd7\xbb\xfe\x49\xc4\x50\x4f"
      "\xb3\xfc\x93\x95\x95\x80\x24\x22\x0e\x47\xc4\xd0\x36\xfb\x98\x7d\xea\x9b"
      "\xa3\xad\x8e\xdd\x3a\xff\x0d\xb4\x61\x9d\xa9\xf2\x75\xc4\x93\xb5\xf3\xbf"
      "\x14\x0d\xf9\x17\x92\x8d\xd7\x27\xc7\xee\x8a\xf2\xcc\xf1\xb1\xe2\xaa\x58"
      "\xef\x97\x5f\xaf\xbc\xd9\xaa\xff\xdb\xca\xbf\x0d\xb2\xf3\xbf\xaf\xe9\xf5"
      "\xbf\x92\xff\x60\x52\xbf\x5e\x3b\xbf\xf5\x3e\xae\xfc\xf1\x79\xcb\x7b\x9a"
      "\xed\x5e\xff\x7d\xc9\xdb\xd5\x72\x5f\xbe\xef\xc3\xc9\x85\x85\x8b\xe3\x11"
      "\x7d\xc9\xeb\xb5\xa0\xeb\xf7\x4f\xac\xbe\xb6\xa8\x17\xed\xb3\xfc\x47\x8e"
      "\x35\x1f\xff\x07\x63\xf5\x9d\x38\x12\x11\xd9\x45\xfc\x50\x44\x3c\x1c\x11"
      "\x8f\xe4\xb1\x3f\x1a\x11\x8f\x45\xc4\xb1\x0d\xf2\xff\xe9\xe5\xc7\xdf\xdb"
      "\x7e\xfe\x3b\x2b\xcb\x7f\x7a\x4b\xe7\x7f\xb5\xd0\x17\x8d\x7b\x9a\x17\x4a"
      "\xe7\x7e\xfc\x6e\x4d\xa7\x83\x5b\xc9\x3f\x3b\xff\x27\xab\xa5\x91\x7c\xcf"
      "\x66\x3e\xff\x36\x13\xd7\xf6\xae\x66\x00\x00\x00\xf8\xef\x49\x23\x62\x7f"
      "\x24\xe9\xe8\x4a\x39\x4d\x47\x47\x6b\x7f\x2f\x7f\x28\xf6\xa5\xe5\xb9\xf9"
      "\x85\xa7\xcf\xcc\x7d\x70\x61\xba\xf6\x1b\x81\xc1\xe8\x4d\x8b\x27\x5d\x03"
      "\x75\xcf\x43\xc7\xf3\xdb\xfa\xa2\x3e\xd1\x50\x3f\x91\x3f\x37\xfe\xb2\xb4"
      "\xb7\x5a\x1f\x9d\x9a\x2b\x4f\x77\x3a\x79\xe8\x72\xfd\x2d\xc6\x7f\xe6\xcf"
      "\x52\xa7\xa3\x03\x76\x9c\xdf\x6b\x41\xf7\x32\xfe\xa1\x7b\x19\xff\xd0\xbd"
      "\x8c\x7f\xe8\x5e\x4d\xc6\xff\xde\x4e\xc4\x01\xec\xbe\x66\xdf\xff\x1f\x77"
      "\x20\x0e\x60\xf7\x35\x8c\x7f\xcb\x7e\xd0\x45\xdc\xff\x43\xf7\x32\xfe\xa1"
      "\x7b\x19\xff\xd0\x95\xe6\xf7\xc6\xad\x7f\x24\xaf\xa0\xb0\xae\x10\xe9\x1d"
      "\x11\x86\xc2\x0e\x15\x3a\xfd\xc9\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00"
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
      "\x00\x00\xd0\x1e\xff\x06\x00\x00\xff\xff\x92\xd7\xe6\xcf",
      1076);
  syz_mount_image(/*fs=*/0x20000040, /*dir=*/0x20000000,
                  /*flags=MS_NOSUID|MS_NOEXEC|MS_NODEV*/ 0xe,
                  /*opts=*/0x20000200, /*chdir=*/3, /*size=*/0x434,
                  /*img=*/0x20000940);
  memcpy((void*)0x20000340, "./bus\000", 6);
  syscall(__NR_open, /*file=*/0x20000340ul,
          /*flags=O_SYNC|O_NOCTTY|O_NOATIME|O_CREAT|FASYNC|0x2*/ 0x143142ul,
          /*mode=*/0ul);
  memcpy((void*)0x20000380, "/dev/loop", 9);
  *(uint8_t*)0x20000389 = 0x30;
  *(uint8_t*)0x2000038a = 0;
  memcpy((void*)0x20000140, "./bus\000", 6);
  syscall(__NR_mount, /*src=*/0x20000380ul, /*dst=*/0x20000140ul, /*type=*/0ul,
          /*flags=MS_BIND*/ 0x1000ul, /*data=*/0ul);
  memcpy((void*)0x20000000, "./bus\000", 6);
  res = syscall(__NR_open, /*file=*/0x20000000ul, /*flags=*/0ul, /*mode=*/0ul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000540 = 0;
  *(uint64_t*)0x20000548 = 0;
  *(uint64_t*)0x20000550 = 0;
  *(uint64_t*)0x20000558 = 0;
  *(uint64_t*)0x20000560 = 0x8001;
  *(uint32_t*)0x20000568 = 0;
  *(uint32_t*)0x2000056c = 0;
  *(uint32_t*)0x20000570 = 0;
  *(uint32_t*)0x20000574 = 0;
  memcpy((void*)0x20000578,
         "\xef\x35\x9f\x41\x3b\xb9\x38\x52\xf7\xd6\xa4\xae\x6d\xdd\xfb\xd1\xce"
         "\x5d\x29\xc2\xee\x5e\x5c\xa9\x00\x0f\xf8\xee\x09\xe7\x37\xff\x0e\xdf"
         "\x11\x0f\xf4\x11\x76\x39\xc2\xeb\x4b\x78\xc6\x60\xe6\x77\xdf\x70\x19"
         "\x05\xb9\xaa\xfa\xb4\xaf\xaa\xf7\x55\xa3\xf6\xa0\x04",
         64);
  memcpy((void*)0x200005b8,
         "\x03\x6c\x47\xc6\x78\x08\x20\xd1\xcb\xf7\x96\x6d\x61\xfd\xcf\x33\x52"
         "\x63\xbd\x9b\xff\xbc\xc2\x54\x2d\xed\x71\x03\x82\x59\xca\x17\x1c\xe1"
         "\xa3\x11\xef\x54\xec\x32\xd7\x1e\x14\xef\x9c\xc0\x93\xfc\xe4\x7d\x85"
         "\x27\x20\x36\xdc\x78\x38\x8e\x3d\xc1\x77\xe9\xb4\x96",
         64);
  memcpy((void*)0x200005f8,
         "\xf2\x83\x59\x73\x8e\x22\x9a\x4c\x66\x81\x00\x00\x00\x00\x00\xd3\x00"
         "\xe6\xd6\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01",
         32);
  *(uint64_t*)0x20000618 = 0;
  *(uint64_t*)0x20000620 = 0;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x4c04, /*arg=*/0x20000540ul);
  memcpy((void*)0x20000080, "memory.events\000", 14);
  syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0x20000080ul,
          /*flags=*/0x275aul, /*mode=*/0ul);
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
  do_sandbox_none();
  return 0;
}