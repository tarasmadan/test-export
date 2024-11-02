// https://syzkaller.appspot.com/bug?id=5fae93fe246c95eadcb5a317ff3806492a3f9492
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

uint64_t r[4] = {0xffffffffffffffff, 0x0, 0xffffffffffffffff,
                 0xffffffffffffffff};

void loop(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 0x803ul, 0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000900 = 0;
  *(uint32_t*)0x20000908 = 0;
  *(uint64_t*)0x20000910 = 0x200008c0;
  *(uint64_t*)0x200008c0 = 0x20000880;
  *(uint32_t*)0x20000880 = 0x10;
  *(uint16_t*)0x20000884 = 0x3f2;
  *(uint16_t*)0x20000886 = 0;
  *(uint32_t*)0x20000888 = 0;
  *(uint32_t*)0x2000088c = 0;
  *(uint64_t*)0x200008c8 = 0x10;
  *(uint64_t*)0x20000918 = 1;
  *(uint64_t*)0x20000920 = 0;
  *(uint64_t*)0x20000928 = 0;
  *(uint32_t*)0x20000930 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000900ul, 0ul);
  *(uint64_t*)0x20000280 = 0;
  *(uint32_t*)0x20000288 = 0;
  *(uint64_t*)0x20000290 = 0x20000180;
  *(uint64_t*)0x20000180 = 0;
  *(uint64_t*)0x20000188 = 0;
  *(uint64_t*)0x20000298 = 1;
  *(uint64_t*)0x200002a0 = 0;
  *(uint64_t*)0x200002a8 = 0;
  *(uint32_t*)0x200002b0 = 0;
  syscall(__NR_sendmsg, -1, 0x20000280ul, 0ul);
  *(uint32_t*)0x20000100 = 0x14;
  res = syscall(__NR_getsockname, r[0], 0x200000c0ul, 0x20000100ul);
  if (res != -1)
    r[1] = *(uint32_t*)0x200000c4;
  *(uint64_t*)0x20000240 = 0;
  *(uint32_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0x20000140;
  *(uint64_t*)0x20000140 = 0x20000300;
  memcpy((void*)0x20000300,
         "\x38\x00\x00\x00\x24\x00\x07\x05\x00\x00\x00\x40\x13\xa2\xa3\x00\x05"
         "\x00\x00\x00",
         20);
  *(uint32_t*)0x20000314 = r[1];
  memcpy((void*)0x20000318,
         "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x09\x00\x01\x00\x68"
         "\x66\x73\x63\x00\x00\x00\x00\x08\x00\x02",
         27);
  *(uint64_t*)0x20000148 = 0x38;
  *(uint64_t*)0x20000258 = 1;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint32_t*)0x20000270 = 0;
  syscall(__NR_sendmsg, -1, 0x20000240ul, 0ul);
  *(uint64_t*)0x20000200 = 0;
  *(uint32_t*)0x20000208 = 0;
  *(uint64_t*)0x20000210 = 0x20000180;
  *(uint64_t*)0x20000180 = 0x200003c0;
  *(uint32_t*)0x200003c0 = 0x34;
  *(uint16_t*)0x200003c4 = 0x2c;
  *(uint16_t*)0x200003c6 = 0xd27;
  *(uint32_t*)0x200003c8 = 0;
  *(uint32_t*)0x200003cc = 0;
  *(uint8_t*)0x200003d0 = 0;
  *(uint8_t*)0x200003d1 = 0;
  *(uint16_t*)0x200003d2 = 0;
  *(uint32_t*)0x200003d4 = r[1];
  *(uint16_t*)0x200003d8 = 0;
  *(uint16_t*)0x200003da = 3;
  *(uint16_t*)0x200003dc = 0;
  *(uint16_t*)0x200003de = 0;
  *(uint16_t*)0x200003e0 = 0xfff1;
  *(uint16_t*)0x200003e2 = 0xfff2;
  *(uint16_t*)0x200003e4 = 0xc;
  *(uint16_t*)0x200003e6 = 1;
  memcpy((void*)0x200003e8, "tcindex\000", 8);
  *(uint16_t*)0x200003f0 = 4;
  *(uint16_t*)0x200003f2 = 2;
  *(uint64_t*)0x20000188 = 0x34;
  *(uint64_t*)0x20000218 = 1;
  *(uint64_t*)0x20000220 = 0;
  *(uint64_t*)0x20000228 = 0;
  *(uint32_t*)0x20000230 = 0;
  syscall(__NR_sendmsg, -1, 0x20000200ul, 0ul);
  res = syscall(__NR_socket, 0x1000000010ul, 0x80002ul, 0);
  if (res != -1)
    r[2] = res;
  syscall(__NR_sendmmsg, r[2], 0x20000200ul, 0x10efe10675dec16ul, 0ul);
  res = syscall(__NR_socket, 0x1000000010ul, 0x80002ul, 0);
  if (res != -1)
    r[3] = res;
  syscall(__NR_sendmmsg, r[3], 0x20000200ul, 0x4924924924926d3ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  do_sandbox_none();
  return 0;
}