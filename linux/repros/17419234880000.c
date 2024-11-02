// https://syzkaller.appspot.com/bug?id=37556d70764e226e660023fad17c9c0316cb6f1d
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
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

#define MAX_FDS 30

struct fs_image_segment {
  void* data;
  uintptr_t size;
  uintptr_t offset;
};
static int setup_loop_device(long unsigned size, long unsigned nsegs,
                             struct fs_image_segment* segs,
                             const char* loopname, int* memfd_p, int* loopfd_p)
{
  int err = 0, loopfd = -1;
  int memfd = syscall(__NR_memfd_create, "syzkaller", 0);
  if (memfd == -1) {
    err = errno;
    goto error;
  }
  if (ftruncate(memfd, size)) {
    err = errno;
    goto error_close_memfd;
  }
  for (size_t i = 0; i < nsegs; i++) {
    if (pwrite(memfd, segs[i].data, segs[i].size, segs[i].offset) < 0) {
    }
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
  *memfd_p = memfd;
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
                            volatile unsigned long size,
                            volatile unsigned long nsegs,
                            volatile long segments, volatile long flags,
                            volatile long optsarg, volatile long change_dir)
{
  struct fs_image_segment* segs = (struct fs_image_segment*)segments;
  int res = -1, err = 0, loopfd = -1, memfd = -1, need_loop_device = !!segs;
  char* mount_opts = (char*)optsarg;
  char* target = (char*)dir;
  char* fs = (char*)fsarg;
  char* source = NULL;
  char loopname[64];
  if (need_loop_device) {
    memset(loopname, 0, sizeof(loopname));
    snprintf(loopname, sizeof(loopname), "/dev/loop%llu", procid);
    if (setup_loop_device(size, nsegs, segs, loopname, &memfd, &loopfd) == -1)
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
    if (strstr(opts, "errors=panic") || strstr(opts, "errors=remount-ro") == 0)
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
    close(memfd);
  }
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

static void close_fds()
{
  for (int fd = 3; fd < MAX_FDS; fd++)
    close(fd);
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void loop(void)
{
  intptr_t res = 0;
  memcpy((void*)0x200000c0, "vfat\000", 5);
  memcpy((void*)0x20000080, "./file0\000", 8);
  *(uint64_t*)0x20000140 = 0x20000000;
  memcpy((void*)0x20000000,
         "\xeb\x3c\x90\x6d\x8d\x66\x73\xfd\xd2\x61\x74\x00\x02\x80\x01\x00\x02"
         "\x40\x00\x00\x04\xf8\x01",
         23);
  *(uint64_t*)0x20000148 = 0x17;
  *(uint64_t*)0x20000150 = 0;
  *(uint64_t*)0x20000158 = 0x20000400;
  memcpy(
      (void*)0x20000400,
      "\x57\x59\x5a\x4b\x41\x4c\x4c\x45\x52\x20\x20\x08\x5a\xc1\x9f\x69\xf2\xb2"
      "\xb1\xea\x1b\x8a\x0a\xc9\x13\x5e\xed\x1d\xf1\xd1\x00\x1c\xc2\xde\x85\x0f"
      "\x06\x00\x00\x00\x00\x00\x00\x00\xf7\xe7\x5e\xff\xac\x2a\xc4\xc1\x5e\x29"
      "\xfb\x3c\x18\xfa\xff\xf8\xd1\x98\xe3\x12\x47\x5f\xfa\x1d\x00\x00\x00\x00"
      "\x00\x00\xad\x25\x82\x2a\x17\xb1\x7f\x46\x3e\x10\x41\x79\xc1\x9c\x2a\xd2"
      "\xfb\xdd\xc0\x77\x7d\xf2\xec\x4f\x62\x82\x60\x86\x70\x4d\xc5\x75\xb6\x97"
      "\x06\xd1\x15\x47\x81\x27\xd9\xf0\xbe\x59\xcd\xc0\x76\x84\x48\x0b\xe4\xb8"
      "\x86\x93\x7d\x8f\xb4\xf0\xff\x94\xe3\xa7\x6e\xcb\xc6\x3c\x2a\xe0\xb3\x87"
      "\xef\x96\xd2\x06\x6a\x83\x2e\xb0\x74\x3d\x5b\x8d\x2d\xd7\x53\x1c\x1b\x63"
      "\xa8\xb2\x81\xdd\xaa\x3d\x1a\x5a\xb2\xe1\xe3\xed\x7a\xba\x34\x0e\xad\x25"
      "\x71\x05\xf3\x6c\xf4\x99\x31\x92\x41\x1d\x5a\xbb\x4c\x65\xf2\xe6\xc2\xb2"
      "\x8b\x69\xb9\x68\x1b\x56\xdb\xb1\x9b\x01\x1d\x41\xf6\x17\x3b\x1e\x9d\xe0"
      "\xae\x2d\x37\xcc\x0d\x67\x74\x57\xb0\x61\x22\x0e\x7a\xa9\x70\x46\x35\x16"
      "\xf0\x82\x4f\x1d\xf4\xdd\x9b\xbf\x16\x5f\xa3\xb0\xea\x49\xdc\xa4\x04\x73"
      "\x07\x74\x59\xda\xe8\xce\x5e\x07\x33\x2c\xa1\x1f\x60\xaa\x39\x88\x64\xd5"
      "\x2c\x56\x96\x0c\x7d\x5c\x65\xcf\x9f\xe7\xdf\x48\x48\x7d\xf8\x6c\x42\x22"
      "\xae\x62\x1f\x0c\x1e\xc0\xbc\x3c\xf7\x94\x3f\xa8\x31\x5d\x96\x31\x40\x0b"
      "\x5c\x16\x57\xd4\xfd\x72\x91\xfe\xbd\xc1\xf9\xe0\x04\x65\xfa\xac\x75\xf3"
      "\xd1\x60\x46\xef\x88\x84\xbe\xc8\x06\x7e\x1d\xe4\xd0\x6e\xcd\x94\x44\xd6"
      "\x3d\x34\x8e\xb2\x20\x9f\xbe\xbc\x00\xc1\xcf\x2a\x4a\x09\x6e\x52\xde\xe0"
      "\xdd\xd6\x81\xa9\xf8\xb9\x12\x55\xa8\xa5\xd4\xaf\xd8\x97\xa2\x39\xf4\xae"
      "\x53\x95\x97\x3e\xd6\x93\xfa\x0a\xcf\x68\x79\x7f\x73\xd1\xd5\xb7\x26\x90"
      "\xe6\x05\x63\xd9\x0d\x8b\x58\xde\x72\xaf\x8d\x1f\x7b\x7e\x9e\xe0\xa9\x39"
      "\xd1\x01\x8d\x95\xf0\xe4\x01\x37\x4c\x40\x10\x08\xa7\x0e\x5a\x4b\x32\x42"
      "\x3c\x70\xd2\xc6\x99\xeb\xdd\x13\x95\xf4\x00\x00\x00\x00\x00\x00\x00"
      "\x00",
      450);
  *(uint64_t*)0x20000160 = 0x1c2;
  *(uint64_t*)0x20000168 = 0x5fe;
  memcpy(
      (void*)0x200001c0,
      "\x69\x6f\x63\x68\x61\x72\x73\x65\x74\x3d\x63\x70\x38\x35\x32\x2c\x6e\x6f"
      "\x6e\x75\x6d\x74\x61\x69\x6c\x3d\x30\x2c\x73\x68\x6f\x72\x74\x6e\x61\x6d"
      "\x65\x3d\x6d\x69\x78\x65\x64\x2c\x73\x68\x6f\x72\x74\x6e\x61\x6d\x65\x3d"
      "\x6c\x6f\x77\x65\x72\x2c\x63\x68\x65\x63\x6b\x3d\x73\x74\x72\x69\x63\x74"
      "\x2c\x73\x68\x6f\x72\x74\x6e\x61\x6d\x65\x3d\x77\x69\x6e\x39\x35\x2c\x64"
      "\x69\x73\x63\x61\x72\x64\x2c\x69\x6f\x63\x68\x61\x72\x73\x65\x74\x3d\x6b"
      "\x6f\x69\x38\x2d\x72\x2c\x69\x6f\x63\x68\x61\x72\x73\x65\x74\x3d\x63\x70"
      "\x38\x36\x35\x2c\x75\x6e\x69\x5f\x78\x6c\x61\x74\x65\x3d\x31\x2c\x75\x73"
      "\x65\x66\x72\x65\x65\x2c\x63\x6f\x64\x65\x70\x61\x67\x65\x3d\x39\x35\x30"
      "\x2c\x72\x6f\x64\x69\x72\x2c\x66\x6c\x75\x73\x68\x2c\x73\x68\x6f\x72\x74"
      "\x6e\x61\x6d\x65\x3d\x77\x69\x6e\x6e\x74\x2c\x00\xbe\x40\x08\xf1\x70\x1f"
      "\x99\x0e\x33\x4c\x5c\x4f\xb7\x1d\x09\xff\x36\x79\xa3\x00\x97\xd1\x9a\xc8"
      "\xfc\x38\xaa\x26\x70\xf4\xd8\x28\x94\x8f\xb6\x09\x3f\x51\x92\x97\xc6\xfa"
      "\x0a\x5e\xe5\x46\x9a\xba\xb4\x09\x99\xd5\x52\xb7\x3c\x57\x9a\xcd\xc3\xa5"
      "\xe8\x81\x99\x5a\x96\xd6\x7f\x91\x43\x25\xb5\x0e\x82\x4b\x16\x3c\x60\x7c"
      "\xf5\xd0\x4d\x02\xaa\x00\xb3\xf9\x0c\xa4\x46\x6f\xcb\xee\xa5\x05\xea\x92"
      "\xe5\xa9\xa3\xb4\xc8\x46\x81\xc5\x1e\x0d",
      298);
  syz_mount_image(0x200000c0, 0x20000080, 0x8000, 2, 0x20000140, 0x2010000,
                  0x200001c0, 0);
  memcpy((void*)0x20000140, "./file0\000", 8);
  syscall(__NR_chdir, 0x20000140ul);
  memcpy((void*)0x20000000, "./file0\000", 8);
  res =
      syscall(__NR_openat, 0xffffff9c, 0x20000000ul, 0x34caa03f8d9ceef2ul, 0ul);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x200003c0, "./file0\000", 8);
  syscall(__NR_unlink, 0x200003c0ul);
  memcpy((void*)0x200000c0, "cgroup.controllers\000", 19);
  res = syscall(__NR_openat, 0xffffff9c, 0x200000c0ul, 0x275aul, 0ul);
  if (res != -1)
    r[1] = res;
  syscall(__NR_write, r[1], 0x20000140ul, 0x3af4701eul);
  *(uint64_t*)0x20000580 = 0x200016c0;
  memset((void*)0x200016c0, 74, 1);
  *(uint64_t*)0x20000588 = 1;
  syscall(__NR_pwritev, r[0], 0x20000580ul, 1ul, 0, 0);
  close_fds();
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  do_sandbox_none();
  return 0;
}