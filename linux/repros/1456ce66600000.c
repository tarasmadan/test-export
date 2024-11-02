// https://syzkaller.appspot.com/bug?id=53b6555b27af2cae74e2fbdac6cadc73f9cb18aa
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
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
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 256;
  setrlimit(RLIMIT_NOFILE, &rlim);
  if (unshare(CLONE_NEWNS)) {
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

int wait_for_loop(int pid)
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
  loop();
  exit(1);
}

static void close_fds()
{
  int fd;
  for (fd = 3; fd < 30; fd++)
    close(fd);
}

uint64_t r[1] = {0xffffffffffffffff};

void loop(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x2000000000000021, 2, 2);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000080 = 0x21;
  *(uint16_t*)0x20000082 = 0;
  *(uint16_t*)0x20000084 = 2;
  *(uint16_t*)0x20000086 = 0x10;
  *(uint16_t*)0x20000088 = 2;
  *(uint16_t*)0x2000008a = htobe16(0x4e24);
  *(uint8_t*)0x2000008c = 0xac;
  *(uint8_t*)0x2000008d = 0x14;
  *(uint8_t*)0x2000008e = 0x14;
  *(uint8_t*)0x2000008f = 0x1e;
  syscall(__NR_connect, r[0], 0x20000080, 0x24);
  *(uint64_t*)0x20005c00 = 0;
  *(uint32_t*)0x20005c08 = 0;
  *(uint64_t*)0x20005c10 = 0;
  *(uint64_t*)0x20005c18 = 0;
  *(uint64_t*)0x20005c20 = 0x20000600;
  memcpy((void*)0x20000600,
         "\x18\x00\x00\x00\x00\x00\x00\x00\x10\x01\x00\x00\x01\x00\x00\x00\x77"
         "\x00\x00\x00\x00\x00\x00\x00\xf4\x1b\x25\xe4\x06\xd5\x89\x37\xe7\xe8"
         "\x99\xe0\x52\x48\x43\xc9\x57\x7f\x89\xf7\x1e\xbd\xdf\x07\x77\x98\xb4"
         "\x7a\xb4\xa7\x69\x07\x47\x7a\x20\x95\x22\xad\xca\x7e\x54\x54\xb7\xde"
         "\xc9\xd9\xa6\x73\x05\xc0\x4a\x8c\xcf\xfc\xa5\x6c\xcb\xca\xbf\xb2\x5c"
         "\xc9\x46\x28\x34\x8a\x24\x59\x3c\x5d\xbd\xf2\x00\x79\x6a\x8f\xd2\x17"
         "\x36\x7c\x01\x7f\x76\xf1\x31\xc8\x69\x3a\xc4\x3b\x77\x47\x1b\xe9\x14"
         "\x70\x7d\x2c\x35\x45\xb8\x12\xf7\xf5\x8f\x43\x67\x06\xf1\x7b\x26\xae"
         "\x32\x08\xe5\x80\x9f\x51\x52\x8f\x24\x36\x61\x0c\x52\xb7\x24\x99\x31"
         "\x41\xa5\xcb\x7a\x0a\x9c\x3d\xca\x0a\x16\x78\x7a\x4d\x0c\x82\xe2\xd9"
         "\x82\x92\x19\xe3\xd9\x81\xd1\x96\xf1\xb7\x25\x6c\x49\xaa\xc9\x6b\x00"
         "\x00\x00\x00\x21\x50\xa4\x05\x8a\x8e\x43\x47\xe1\x3f\x94\xbe\x6d\x87"
         "\xfc\x7e\xcd\xaf\xb8\x8b\x1a\x03\xb8\xd9\xe4\xea\xc3\xf0\x07\x65\x80"
         "\xf6\xe7\x6a\xec\x10\x1f\xc9\x35\xe5\xc2\x5f\xac\x97\x0f\x73\xd3\xfe"
         "\x7d\x1f\x03\x6c\x1e\x5b\x40\x46\x42\x69\xb3\xa1\x07\x1a\x96\xf3\xd5"
         "\xfd\xf7\xa8\xe7\xb9\x07\xd2\x36\xe9\x1a\x85\xb6\xe2\xa1\x3b\x6b\x5e"
         "\xb5\x58\xd0\x81\xf5\xb2\x24\x4a\x7c\x56\x69\xb2\x24\xb8\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         301);
  *(uint64_t*)0x20005c28 = 0x18;
  *(uint32_t*)0x20005c30 = 0;
  *(uint32_t*)0x20005c38 = 0;
  syscall(__NR_sendmmsg, r[0], 0x20005c00, 1, 0);
  close_fds();
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  do_sandbox_none();
  return 0;
}