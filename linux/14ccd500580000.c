// https://syzkaller.appspot.com/bug?id=11a375d719b6c2a898a06115d279e636266c2fe6
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

#ifndef __NR_io_uring_setup
#define __NR_io_uring_setup 425
#endif
#ifndef __NR_pidfd_open
#define __NR_pidfd_open 434
#endif

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

static void sandbox_common_mount_tmpfs(void)
{
  write_file("/proc/sys/fs/mount-max", "100000");
  if (mkdir("./syz-tmp", 0777))
    exit(1);
  if (mount("", "./syz-tmp", "tmpfs", 0, NULL))
    exit(1);
  if (mkdir("./syz-tmp/newroot", 0777))
    exit(1);
  if (mkdir("./syz-tmp/newroot/dev", 0700))
    exit(1);
  unsigned bind_mount_flags = MS_BIND | MS_REC | MS_PRIVATE;
  if (mount("/dev", "./syz-tmp/newroot/dev", NULL, bind_mount_flags, NULL))
    exit(1);
  if (mkdir("./syz-tmp/newroot/proc", 0700))
    exit(1);
  if (mount("syz-proc", "./syz-tmp/newroot/proc", "proc", 0, NULL))
    exit(1);
  if (mkdir("./syz-tmp/newroot/selinux", 0700))
    exit(1);
  const char* selinux_path = "./syz-tmp/newroot/selinux";
  if (mount("/selinux", selinux_path, NULL, bind_mount_flags, NULL)) {
    if (errno != ENOENT)
      exit(1);
    if (mount("/sys/fs/selinux", selinux_path, NULL, bind_mount_flags, NULL) &&
        errno != ENOENT)
      exit(1);
  }
  if (mkdir("./syz-tmp/newroot/sys", 0700))
    exit(1);
  if (mount("/sys", "./syz-tmp/newroot/sys", 0, bind_mount_flags, NULL))
    exit(1);
  if (mkdir("./syz-tmp/pivot", 0777))
    exit(1);
  if (syscall(SYS_pivot_root, "./syz-tmp", "./syz-tmp/pivot")) {
    if (chdir("./syz-tmp"))
      exit(1);
  } else {
    if (chdir("/"))
      exit(1);
    if (umount2("./pivot", MNT_DETACH))
      exit(1);
  }
  if (chroot("./newroot"))
    exit(1);
  if (chdir("/"))
    exit(1);
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
  sandbox_common_mount_tmpfs();
  setup_binderfs();
  loop();
  exit(1);
}

void loop(void)
{
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  syscall(__NR_socket, /*domain=*/0x10ul, /*type=*/3ul, /*proto=*/0x10);
  syscall(__NR_pidfd_open, /*pid=*/1ul, /*flags=*/0ul);
  syscall(__NR_socket, /*domain=AF_INET*/ 2ul, /*type=SOCK_DGRAM*/ 2ul,
          /*proto=*/1);
  syscall(__NR_mmap, /*addr=*/0ul, /*len=*/8ul, /*prot=*/0xdful,
          /*flags=*/0x9b72ul, /*fd=*/2ul, /*off=*/0x8000ul);
  syscall(__NR_socket, /*domain=AF_CAN*/ 0x1dul, /*type=SOCK_RAW*/ 3ul,
          /*proto=*/1);
  syscall(__NR_socketpair, /*family=*/0xfffful, /*type=*/6ul,
          /*protocol=*/0xfffffffffffffff7ul, /*usockvec=*/0xbul);
  syscall(__NR_io_uring_setup, /*entries=*/0x3fful, /*params=*/3ul);
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
  do_sandbox_none();
  return 0;
}
