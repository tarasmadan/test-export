// https://syzkaller.appspot.com/bug?id=84950da5f4452669e5a1ec286c06815fc1596767
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <signal.h>
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
#include <time.h>
#include <unistd.h>

#include <linux/capability.h>

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

#define MAX_FDS 30

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
  rlim.rlim_cur = rlim.rlim_max = 0;
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

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  write_file("/proc/self/oom_score_adj", "1000");
}

static void close_fds()
{
  for (int fd = 3; fd < MAX_FDS; fd++)
    close(fd);
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter = 0;
  for (;; iter++) {
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      execute_one();
      close_fds();
      exit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      sleep_ms(1);
      if (current_time_ms() - start < 5000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  memcpy((void*)0x200000c0, "./binderfs/binder0\000", 19);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x200000c0ul, 0ul, 0ul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000080 = 8;
  *(uint64_t*)0x20000088 = 0;
  *(uint64_t*)0x20000090 = 0x20000400;
  memcpy((void*)0x20000400, "\x04\x63\x04\x40", 4);
  *(uint64_t*)0x20000098 = 0;
  *(uint64_t*)0x200000a0 = 0;
  *(uint64_t*)0x200000a8 = 0;
  syscall(__NR_ioctl, r[0], 0xc0306201, 0x20000080ul);
  memcpy((void*)0x200000c0, "./binderfs/binder0\000", 19);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x200000c0ul, 0ul, 0ul);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x200000c0, "./binderfs/binder0\000", 19);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x200000c0ul, 0ul, 0ul);
  if (res != -1)
    r[2] = res;
  res = syscall(__NR_dup2, r[1], r[2]);
  if (res != -1)
    r[3] = res;
  *(uint32_t*)0x20000000 = 0x73622a85;
  *(uint32_t*)0x20000004 = 0;
  *(uint64_t*)0x20000008 = 0;
  *(uint64_t*)0x20000010 = 0;
  syscall(__NR_ioctl, r[3], 0x4018620d, 0x20000000ul);
  *(uint64_t*)0x20001480 = 8;
  *(uint64_t*)0x20001488 = 0;
  *(uint64_t*)0x20001490 = 0x20001380;
  *(uint32_t*)0x20001380 = 0x40046305;
  *(uint32_t*)0x20001384 = 0;
  *(uint64_t*)0x20001498 = 0x47;
  *(uint64_t*)0x200014a0 = 0;
  *(uint64_t*)0x200014a8 = 0;
  syscall(__NR_ioctl, r[0], 0xc0306201, 0x20001480ul);
  syscall(__NR_mmap, 0x20ffc000ul, 0x1000ul, 1ul, 0x11ul, r[2], 0ul);
  *(uint64_t*)0x20000680 = 0x4c;
  *(uint64_t*)0x20000688 = 0;
  *(uint64_t*)0x20000690 = 0x20000440;
  *(uint32_t*)0x20000440 = 0x40486311;
  *(uint32_t*)0x20000444 = 1;
  *(uint32_t*)0x20000448 = 0;
  *(uint64_t*)0x2000044c = 0;
  *(uint32_t*)0x20000454 = 0;
  *(uint32_t*)0x20000458 = 0;
  *(uint32_t*)0x2000045c = 0;
  *(uint32_t*)0x20000460 = 0;
  *(uint64_t*)0x20000464 = 0;
  *(uint64_t*)0x2000046c = 0;
  *(uint64_t*)0x20000474 = 0;
  *(uint64_t*)0x2000047c = 0;
  *(uint64_t*)0x20000484 = 0;
  *(uint64_t*)0x20000698 = 0;
  *(uint64_t*)0x200006a0 = 0;
  *(uint64_t*)0x200006a8 = 0;
  syscall(__NR_ioctl, r[0], 0xc0306201, 0x20000680ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      do_sandbox_none();
    }
  }
  sleep(1000000);
  return 0;
}
