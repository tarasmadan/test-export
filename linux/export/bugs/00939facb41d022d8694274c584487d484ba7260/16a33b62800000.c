// https://syzkaller.appspot.com/bug?id=00939facb41d022d8694274c584487d484ba7260
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/capability.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

static void fail(const char* msg, ...)
{
  int e = errno;
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit((e == ENOMEM || e == EAGAIN) ? kRetryStatus : kFailStatus);
}

static void exitf(const char* msg, ...)
{
  int e = errno;
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit(kRetryStatus);
}

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void use_temporary_dir()
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    fail("failed to mkdtemp");
  if (chmod(tmpdir, 0777))
    fail("failed to chmod");
  if (chdir(tmpdir))
    fail("failed to chdir");
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();

  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 128 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);

  unshare(CLONE_NEWNS);
  unshare(CLONE_NEWIPC);
  unshare(CLONE_IO);
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
    close(fd);
    return false;
  }
  close(fd);
  return true;
}

static int real_uid;
static int real_gid;
__attribute__((aligned(64 << 10))) static char sandbox_stack[1 << 20];

static int namespace_sandbox_proc(void* arg)
{
  sandbox_common();

  write_file("/proc/self/setgroups", "deny");
  if (!write_file("/proc/self/uid_map", "0 %d 1\n", real_uid))
    fail("write of /proc/self/uid_map failed");
  if (!write_file("/proc/self/gid_map", "0 %d 1\n", real_gid))
    fail("write of /proc/self/gid_map failed");

  if (mkdir("./syz-tmp", 0777))
    fail("mkdir(syz-tmp) failed");
  if (mount("", "./syz-tmp", "tmpfs", 0, NULL))
    fail("mount(tmpfs) failed");
  if (mkdir("./syz-tmp/newroot", 0777))
    fail("mkdir failed");
  if (mkdir("./syz-tmp/newroot/dev", 0700))
    fail("mkdir failed");
  if (mount("/dev", "./syz-tmp/newroot/dev", NULL,
            MS_BIND | MS_REC | MS_PRIVATE, NULL))
    fail("mount(dev) failed");
  if (mkdir("./syz-tmp/newroot/proc", 0700))
    fail("mkdir failed");
  if (mount(NULL, "./syz-tmp/newroot/proc", "proc", 0, NULL))
    fail("mount(proc) failed");
  if (mkdir("./syz-tmp/pivot", 0777))
    fail("mkdir failed");
  if (syscall(SYS_pivot_root, "./syz-tmp", "./syz-tmp/pivot")) {
    if (chdir("./syz-tmp"))
      fail("chdir failed");
  } else {
    if (chdir("/"))
      fail("chdir failed");
    if (umount2("./pivot", MNT_DETACH))
      fail("umount failed");
  }
  if (chroot("./newroot"))
    fail("chroot failed");
  if (chdir("/"))
    fail("chdir failed");

  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    fail("capget failed");
  cap_data[0].effective &= ~(1 << CAP_SYS_PTRACE);
  cap_data[0].permitted &= ~(1 << CAP_SYS_PTRACE);
  cap_data[0].inheritable &= ~(1 << CAP_SYS_PTRACE);
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    fail("capset failed");

  loop();
  doexit(1);
}

static int do_sandbox_namespace(int executor_pid, bool enable_tun)
{

  real_uid = getuid();
  real_gid = getgid();
  mprotect(sandbox_stack, 4096, PROT_NONE);
  return clone(
      namespace_sandbox_proc,
      &sandbox_stack[sizeof(sandbox_stack) - 64],
      CLONE_NEWUSER | CLONE_NEWPID | CLONE_NEWUTS | CLONE_NEWNET, NULL);
}

static void remove_dir(const char* dir)
{
  DIR* dp;
  struct dirent* ep;
  int iter = 0;
retry:
  dp = opendir(dir);
  if (dp == NULL) {
    if (errno == EMFILE) {
      exitf("opendir(%s) failed due to NOFILE, exiting");
    }
    exitf("opendir(%s) failed", dir);
  }
  while ((ep = readdir(dp))) {
    if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
      continue;
    char filename[FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", dir, ep->d_name);
    struct stat st;
    if (lstat(filename, &st))
      exitf("lstat(%s) failed", filename);
    if (S_ISDIR(st.st_mode)) {
      remove_dir(filename);
      continue;
    }
    int i;
    for (i = 0;; i++) {
      if (unlink(filename) == 0)
        break;
      if (errno == EROFS) {
        break;
      }
      if (errno != EBUSY || i > 100)
        exitf("unlink(%s) failed", filename);
      if (umount2(filename, MNT_DETACH))
        exitf("umount(%s) failed", filename);
    }
  }
  closedir(dp);
  int i;
  for (i = 0;; i++) {
    if (rmdir(dir) == 0)
      break;
    if (i < 100) {
      if (errno == EROFS) {
        break;
      }
      if (errno == EBUSY) {
        if (umount2(dir, MNT_DETACH))
          exitf("umount(%s) failed", dir);
        continue;
      }
      if (errno == ENOTEMPTY) {
        if (iter < 100) {
          iter++;
          goto retry;
        }
      }
    }
    exitf("rmdir(%s) failed", dir);
  }
}

static void test();

void loop()
{
  int iter;
  for (iter = 0;; iter++) {
    char cwdbuf[256];
    sprintf(cwdbuf, "./%d", iter);
    if (mkdir(cwdbuf, 0777))
      fail("failed to mkdir");
    int pid = fork();
    if (pid < 0)
      fail("clone failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
      if (chdir(cwdbuf))
        fail("failed to chdir");
      test();
      doexit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      int res = waitpid(-1, &status, __WALL | WNOHANG);
      if (res == pid)
        break;
      usleep(1000);
      if (current_time_ms() - start > 5 * 1000) {
        kill(-pid, SIGKILL);
        kill(pid, SIGKILL);
        while (waitpid(-1, &status, __WALL) != pid) {
        }
        break;
      }
    }
    remove_dir(cwdbuf);
  }
}

#ifndef __NR_mmap
#define __NR_mmap 90
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#ifndef __NR_setsockopt
#define __NR_setsockopt 366
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 370
#endif
#ifndef __NR_sendto
#define __NR_sendto 369
#endif
#ifndef __NR_ioctl
#define __NR_ioctl 54
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[65];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    r[1] = syscall(__NR_socket, 0xaul, 0x5ul, 0x8010000000000084ul);
    break;
  case 2:
    *(uint32_t*)0x20c01ffc = (uint32_t)0xac8;
    r[3] = syscall(__NR_setsockopt, r[1], 0x1ul, 0x7ul, 0x20c01ffcul,
                   0x4ul);
    break;
  case 3:
    *(uint32_t*)0x20267000 = (uint32_t)0x205e3000;
    *(uint32_t*)0x20267004 = (uint32_t)0x10;
    *(uint32_t*)0x20267008 = (uint32_t)0x20ca6f80;
    *(uint32_t*)0x2026700c = (uint32_t)0x8;
    *(uint32_t*)0x20267010 = (uint32_t)0x20000000;
    *(uint32_t*)0x20267014 = (uint32_t)0x0;
    *(uint32_t*)0x20267018 = (uint32_t)0x20000000;
    *(uint16_t*)0x205e3000 = (uint16_t)0x2;
    *(uint16_t*)0x205e3002 = (uint16_t)0x234e;
    *(uint8_t*)0x205e3004 = (uint8_t)0xac;
    *(uint8_t*)0x205e3005 = (uint8_t)0x14;
    *(uint8_t*)0x205e3006 = (uint8_t)0x0;
    *(uint8_t*)0x205e3007 = (uint8_t)0xaa;
    *(uint8_t*)0x205e3008 = (uint8_t)0x0;
    *(uint8_t*)0x205e3009 = (uint8_t)0x0;
    *(uint8_t*)0x205e300a = (uint8_t)0x0;
    *(uint8_t*)0x205e300b = (uint8_t)0x0;
    *(uint8_t*)0x205e300c = (uint8_t)0x0;
    *(uint8_t*)0x205e300d = (uint8_t)0x0;
    *(uint8_t*)0x205e300e = (uint8_t)0x0;
    *(uint8_t*)0x205e300f = (uint8_t)0x0;
    *(uint32_t*)0x20ca6f80 = (uint32_t)0x20230000;
    *(uint32_t*)0x20ca6f84 = (uint32_t)0x1000;
    *(uint32_t*)0x20ca6f88 = (uint32_t)0x208d5f07;
    *(uint32_t*)0x20ca6f8c = (uint32_t)0x0;
    *(uint32_t*)0x20ca6f90 = (uint32_t)0x200adf5c;
    *(uint32_t*)0x20ca6f94 = (uint32_t)0x0;
    *(uint32_t*)0x20ca6f98 = (uint32_t)0x2091b000;
    *(uint32_t*)0x20ca6f9c = (uint32_t)0x0;
    *(uint32_t*)0x20ca6fa0 = (uint32_t)0x203dcf92;
    *(uint32_t*)0x20ca6fa4 = (uint32_t)0x0;
    *(uint32_t*)0x20ca6fa8 = (uint32_t)0x209aa000;
    *(uint32_t*)0x20ca6fac = (uint32_t)0x0;
    *(uint32_t*)0x20ca6fb0 = (uint32_t)0x205b0000;
    *(uint32_t*)0x20ca6fb4 = (uint32_t)0x0;
    *(uint32_t*)0x20ca6fb8 = (uint32_t)0x20d62000;
    *(uint32_t*)0x20ca6fbc = (uint32_t)0x0;
    memcpy(
        (void*)0x20230000,
        "\x8a\x7b\x77\x6a\x6d\x88\x97\x91\x9f\x77\xfb\x32\x26\x29\x3c"
        "\xc6\x51\x99\x6b\x46\x29\xf4\x55\x3e\x9c\xb8\x55\x67\x03\x0b"
        "\x24\x12\x2d\x35\x1a\xd3\x51\x09\x06\xbf\x08\xf7\xb1\xea\x62"
        "\xac\xbc\x38\x00\xbb\xbc\xcb\x29\x08\x63\xd9\x95\xd9\x79\xbb"
        "\xa3\x7e\xe4\x17\x89\xfc\x06\x00\x7d\x95\x77\xae\x51\x6a\x59"
        "\xf4\xa7\x6a\xe6\xe2\x82\x2c\x8e\x4f\xee\x3d\x2b\xd8\x43\x3f"
        "\x87\xbd\x75\x8a\x51\x75\xe8\xfd\x33\xf9\x99\xf2\xf9\x7e\xfc"
        "\x57\xa2\xea\x29\xd1\x60\x8d\xd2\x28\x3f\x61\x20\x07\x11\x04"
        "\x49\xca\xe4\xcc\xad\xf3\x67\x84\x77\xb3\x38\xd5\x2d\x1c\x0f"
        "\x51\x6a\x76\xa9\xd1\x7f\x54\x97\x87\x51\x6e\x69\x06\x78\x0d"
        "\xaa\xcd\x7f\x27\x2d\x9e\xcd\xa2\x36\x54\xcc\x5b\x1b\xb4\x0a"
        "\xf8\xeb\x8d\x1b\x8c\x5a\xe6\x8f\xc4\xe4\x13\xa9\x76\xad\x3e"
        "\x15\x88\x2e\x9a\xcc\xe1\xe5\xa0\xf6\x4b\xe1\xde\x46\xf3\x46"
        "\x18\xde\xde\x13\x8d\x00\x4e\x0b\xd3\xce\x10\x7a\x3d\x32\xeb"
        "\xc4\x8c\x3d\x43\xcd\x27\xc3\x34\xc3\xb4\x82\xf1\x53\x8c\xe2"
        "\x7f\xcd\x56\xff\x58\xf3\xe9\xb9\xf4\x24\x90\xdc\x6c\xbc\x61"
        "\x79\xaf\x5f\xca\xad\x37\xd9\x9f\xf8\x88\xf3\x73\x8b\x68\x66"
        "\x82\x8d\x18\x83\x93\x8e\x5a\xd8\xde\x85\x56\xa3\xbd\x97\xfb"
        "\x28\xac\x3c\xe6\xc0\x6b\x19\x80\x99\xfe\xf7\x81\x3d\x30\xeb"
        "\x49\xca\x57\x95\xb2\x84\xaf\xb3\xdd\xac\xcd\xb1\xe0\x35\x06"
        "\xa2\x15\x94\x7b\xac\xdb\xf0\x46\xf4\x03\x26\xfe\x40\xa7\x19"
        "\xd2\xac\x40\x28\x0d\x14\xe0\xb0\x93\x3f\x30\x4e\xce\x95\xef"
        "\x71\x5f\x5a\x21\xda\xdb\x55\xfb\x27\x99\x6c\x42\x3a\xf5\x65"
        "\x48\x4d\x47\x7f\x00\xf4\x9f\x2f\xa4\x7e\x10\x50\x51\x13\xb0"
        "\x3a\xfb\x13\x37\xbe\x33\x2e\x51\x49\xc2\x0f\xc1\x4c\xbd\x9e"
        "\xbe\x76\x6a\x37\x5f\xfa\xf6\xe1\x9f\xc1\xde\x40\xd7\x82\xa1"
        "\x35\x87\xf9\x58\x3f\x3b\x33\x31\xb6\x04\x5f\x3a\x75\x78\xbd"
        "\xf3\x32\x59\xc9\x45\xce\xc1\xe8\x87\x1a\x1f\x19\xd3\x09\xaf"
        "\xf9\x67\x13\x15\xab\x69\x9d\xc8\x9b\x27\xd0\x66\x0e\xfc\x0a"
        "\xb5\x0d\x9c\xa5\xbc\x6b\xd2\x35\xdf\x98\xce\xbb\x0f\x62\xba"
        "\xc5\xe7\xe8\x8a\x21\x09\x83\xe0\x98\x31\xba\xa4\xbf\x27\x6d"
        "\xa9\xad\xf3\xda\x0f\x4b\x88\x7a\x23\x9d\xf6\xba\x9d\x92\x0b"
        "\x31\xfe\x05\x92\x8f\x90\xf5\xd3\x0c\x6b\x45\x35\xe4\xeb\x00"
        "\x44\xc4\xc6\x24\x85\x89\xad\xf7\x7f\xbf\x96\x75\xd4\x66\x89"
        "\x01\x28\xb9\x7c\x50\xd8\x31\x47\x6d\x9a\x29\x40\x80\xbd\x74"
        "\x73\x89\x23\x6e\xe2\xd3\x9f\x52\xa3\xb1\x7f\x6c\x5e\x86\x3f"
        "\x3f\xf0\x35\xbb\x52\xf2\xcb\x50\xe5\xc2\xa2\x01\x5b\x52\x36"
        "\x7a\x17\x16\xfd\x27\xd5\xf2\x1d\x7b\x73\x19\x6d\x8a\x06\x3f"
        "\xb5\x67\xf7\x26\x8b\xe0\xb5\x82\x53\x0d\xe1\xe9\xec\xc8\xc6"
        "\x86\xd0\xc6\x5d\x48\x8f\x2b\xb1\x85\x0d\xb8\xb8\xae\x97\xf7"
        "\xb3\x76\x91\x2d\xd5\x24\x62\x65\x56\xcc\x65\x95\x21\x04\xd4"
        "\xd9\x6c\x60\x7a\x73\x71\xbc\xc7\x46\x39\x9a\xea\xa8\x04\xa5"
        "\x90\xaf\x68\x8f\x62\x16\x42\xe3\x45\x5c\x87\x63\x95\x61\xcb"
        "\x53\x7d\xa6\xb5\x27\x55\x9a\x0b\xb7\x2f\xe2\x5b\x7e\xaa\x6a"
        "\xe0\xe6\x7a\xca\x61\x72\x13\xe3\xba\x18\x49\x40\x01\x65\x3a"
        "\xf2\x0e\x34\xa1\x84\xc4\x23\x39\x76\x2b\xcf\x69\x9c\x4b\xa7"
        "\x23\xf7\xec\x25\x40\x57\x54\x32\x26\x12\x53\xec\x9d\xb8\x64"
        "\x42\xf5\xa3\xa8\x07\x1b\xa2\x13\x0d\xbc\x5e\xb0\xf9\x5b\x8e"
        "\x38\xa2\x1c\xe5\xef\xf9\x8f\x6a\xd4\x67\xac\x45\x25\x21\x29"
        "\x36\xdb\xa5\x7e\xda\x10\xe9\x60\xb5\xfe\x23\xf4\x1d\x4a\x52"
        "\x24\xe4\x79\x9b\x04\x7d\xa0\x3a\x5e\x75\xfa\x39\x0a\x13\xd7"
        "\x1a\x3f\xcd\x9f\xe0\x7a\x2f\x7e\x75\x3b\x12\x0c\x19\x7d\x04"
        "\x99\x4b\x9d\xaa\x9f\x66\xba\x66\x61\x5f\xe2\x39\x7c\x0f\xe0"
        "\x77\x08\x55\x51\xcf\x16\xb4\xfa\xb2\x7e\x65\x30\xb2\xce\x77"
        "\x31\x55\x4f\xba\x2d\x68\xa6\x17\x7e\x08\x93\xb3\xea\x65\x35"
        "\x1f\x07\xb9\x6b\x6b\xa3\xe0\xa7\xa2\x49\x1c\xd3\x67\x03\x26"
        "\x7e\x2b\xd0\x92\xd3\xb4\x87\x05\x90\x15\x4c\x01\x44\xbb\x88"
        "\xe4\x50\xba\xf9\x5b\x2f\xc4\xcb\x1d\x43\x35\x18\x9c\x13\x91"
        "\x8c\x1d\x4d\x56\x33\x29\x28\xfd\xd1\x9e\x4b\x17\x38\x7e\x16"
        "\xbb\x02\xcb\x5b\xaa\x91\x65\xbd\x3c\x13\x5c\xf8\xbe\x4a\x9d"
        "\x85\xb0\x72\x42\x90\x9d\x96\x05\x2b\xd6\x5e\xf2\x37\x77\xa9"
        "\x89\x64\xf1\x51\xbe\x4e\xce\xfa\x01\xdb\x33\xc4\x43\xc3\x0b"
        "\x11\xea\x9a\x2e\xb5\x0f\xba\xd2\xd2\x6d\x05\x95\xf3\x9e\x3b"
        "\x28\xff\xa7\x35\x48\x29\x06\xcc\x11\x6a\xb8\x76\x20\x6f\x99"
        "\x40\xc9\xb7\x74\x49\x00\xd5\x7a\x52\x67\x6a\xe5\x45\x81\x3e"
        "\xe6\x7a\x74\x04\x42\x61\x46\xc5\x91\x0c\x4b\x6c\xb7\x55\x91"
        "\x98\x1d\x6c\x79\x79\x9f\x86\x24\x81\x88\xc3\x14\xa3\xe0\x77"
        "\xdc\x26\x88\x71\x50\x1d\x3c\xcd\x42\x43\x2a\xfb\xcd\xe5\xdd"
        "\xa5\x65\x71\x3d\xb8\x82\x4b\xfd\xe5\xb3\x4f\x0a\xdf\x97\x02"
        "\xa5\x3a\xa8\x81\x4e\xc9\xaa\x95\x88\xcc\x24\x32\xa6\xe5\x42"
        "\xf9\x15\xd3\x54\x5a\x00\x65\x64\x4e\xa3\xf1\x82\x69\x66\x19"
        "\x14\x40\x9a\x1b\xc8\x7c\xbe\x78\xc4\xa8\xd4\xc2\xc9\xee\xaa"
        "\xd5\x6c\x6d\xec\xf2\xe2\xc5\x5f\x66\xac\xa9\x0d\x99\x89\x86"
        "\xf6\xd1\x7b\x55\xec\xa4\x6c\xaf\x68\xa7\xcb\x28\xd3\xca\xcb"
        "\xe6\xc3\xbc\xf3\xff\x37\x1b\xd5\xdd\x41\x81\xe1\xed\x7d\x49"
        "\xb1\x55\xb1\x24\xd1\x77\x6d\xcf\xb8\xe7\x39\xff\x17\xd4\x6b"
        "\x3f\xc4\x10\xf8\x7b\xd4\x96\x73\x1a\x5e\xb8\x24\x91\x16\x67"
        "\x1d\x41\xb8\xbc\x5e\x1e\x92\x24\xa9\x7c\xba\x63\xe8\x4f\x15"
        "\x39\xbe\xb7\xf2\x3b\x78\x81\x45\x04\x8e\x2b\xb7\xb7\x14\x74"
        "\x8b\x84\x8b\x73\x51\xb9\x18\x44\x1f\x95\x58\x4a\x6b\x5f\x76"
        "\xc9\xd0\xc4\xca\x98\x31\x25\x68\x86\x4c\x00\xa7\x92\x68\xfa"
        "\x4a\x6b\xa0\xbd\x21\xab\x09\x22\x96\x67\x46\x85\xdc\x75\x3d"
        "\x66\xa8\x33\x2a\x71\xbc\x19\xda\xaf\x3e\x0a\xb5\x7e\xac\xc8"
        "\x8b\xde\xc5\x4a\x10\xd5\xd0\x95\xde\xed\x7a\xb9\x61\xe6\x78"
        "\x6e\x84\x74\xf4\x9b\x17\xe3\x59\x85\x01\xef\xe2\xfb\xd6\xf8"
        "\x5e\xf8\xc0\x1d\x27\xed\xab\x43\x2b\x70\x6e\xc6\x07\x4d\x35"
        "\x43\x11\xac\x0e\x02\xbd\xe5\xdc\x7a\x38\x22\x25\x0a\xa3\x3c"
        "\x06\xaa\x55\x92\xd7\x60\xe6\x91\xbf\x4e\x3d\x90\xe7\xc6\x46"
        "\xb4\xa2\xa0\xa9\x0e\x2f\xe2\x93\xa2\x81\x49\x3e\x9c\x80\xed"
        "\x3d\xcd\xc6\x5a\x66\xbc\x45\xff\x57\x63\xf7\x9e\xb8\xbe\x22"
        "\x7f\xed\xcd\xca\x8a\xb8\x3e\x16\xc6\x3f\xfb\x6b\x29\x49\x47"
        "\xf8\xfe\x81\x30\xdb\x8e\x2c\x07\xe4\x39\x4f\x48\x22\xfa\xa6"
        "\x48\x05\xdd\xf2\xdd\x3a\xf8\x99\x83\xb8\x80\x7c\xa8\x3c\x47"
        "\x4e\xdd\x2b\xea\xd1\xc0\x05\x48\x80\xc3\x20\x88\xfe\xee\xf9"
        "\xca\x53\xd0\x6f\x76\xde\x70\xad\x29\xe6\x2e\x47\xc7\x5f\x3c"
        "\x0f\x1f\xfb\xd0\x2c\x1e\xf2\x7e\x25\x4b\x8f\x14\x1f\x9b\x0a"
        "\x8a\x98\x31\x0c\x25\xbb\x18\xb9\xa4\x52\xd7\xd8\x43\xb3\xbc"
        "\x47\x29\xc0\x8c\xc8\x47\xfb\x36\x0a\xd2\x3d\x7b\xc4\x1c\x15"
        "\x88\xa2\xab\xe1\x6d\xd7\x5b\x25\x4f\x72\xbe\xb9\x76\x29\xa5"
        "\x64\x07\x46\x61\xa3\x1f\xd3\xdc\xad\xd5\x7d\x40\xf8\xac\x42"
        "\x18\x88\x9a\x5e\x66\x7e\xc8\xf7\x6d\x8c\x1c\x40\xdc\xa1\xbb"
        "\xaa\x83\x27\x7d\x16\x3d\xf4\x9a\x3a\xc2\xe7\x9c\x3c\x48\xb4"
        "\x63\xc1\xac\x32\xc8\x37\xff\xad\x01\x7e\x87\x3a\x11\xaa\xb5"
        "\xcd\x44\xfe\xf3\xeb\x5e\x59\x50\xed\x53\x78\x73\x9a\x75\x93"
        "\x18\xb7\xee\xba\x31\x1b\xa7\x43\x86\x9c\x50\xc4\x38\xf8\xa0"
        "\x5d\x0b\xa7\xeb\xf0\xd7\xd0\xbe\xdb\x5a\x95\x01\xb0\x84\x56"
        "\x79\xd3\x92\x21\x59\xf6\x3b\xa7\x4c\x4f\x0c\x1b\x21\xd0\x62"
        "\x1e\x09\xb0\x52\x71\xa7\xd1\x43\xe3\xcd\x70\xa8\x81\x33\x6c"
        "\xfb\x01\x4c\x7a\xba\xb3\xa7\x2f\x1d\xaf\x54\x7f\xc2\x46\x68"
        "\x0f\xfd\x97\x02\xdd\x19\x33\x81\xc8\xd4\x88\x32\x02\xf9\xdd"
        "\xaa\xc6\xca\x7b\xab\xc3\x34\xd6\x4f\xcb\xed\xde\xb8\xe5\xe0"
        "\xf7\xdf\x39\xd1\x46\x91\x04\xe6\x01\x9f\x64\x48\x40\xdf\x01"
        "\xc2\x96\xce\x8c\x1c\xbf\x19\x4c\x4a\xdd\x99\x1b\xc8\xf4\xa6"
        "\x2e\x0b\xe5\x1d\xb9\x84\x57\x22\xef\x6d\xd3\x12\xf0\xfc\xc3"
        "\x87\x42\xd5\xb2\x6b\xfb\xd8\x0c\x7e\xd5\xad\xa4\x8a\xf3\xae"
        "\x20\xf4\x3d\x28\x17\x3e\xd5\x7a\x9c\xad\x5b\xdc\x4a\xab\x4a"
        "\x7d\x04\x4f\x87\xba\x80\x4d\xac\x9c\x6c\x1e\xca\x47\x55\x2d"
        "\x90\x88\x5d\x89\x98\x5d\xc8\xc0\x61\x8c\x99\x0c\xf3\x0f\x65"
        "\xa1\x28\xfd\x7c\x70\xe9\x73\xa0\x3d\x7f\xea\xa8\x05\x9e\x85"
        "\xe1\x30\xe6\x61\x3a\x3c\xf1\xa1\xc2\x80\x0d\x8b\x7b\xff\xe1"
        "\x5b\xde\x0c\xae\x2c\x0b\xa8\x91\x08\xc4\x77\x99\xb0\xad\xa3"
        "\x98\xf6\xb5\x75\xd7\x66\xee\x35\xca\xfa\x0d\x0e\xfe\x81\x95"
        "\x30\xdc\xd9\x9d\x94\x63\xf8\x98\xc4\xdb\x6d\xf8\x59\x69\x9e"
        "\xe8\x36\xaa\x13\x89\x53\xb4\x7c\xb6\x96\x09\xec\x2f\x66\x96"
        "\x8f\x71\xd0\x33\x35\x78\x99\xc7\x7b\xaf\x9b\xe8\x66\xf0\x75"
        "\xa6\xd2\x52\x00\x04\x87\x4b\xcf\x9f\x92\x60\x0b\xbb\x1c\xeb"
        "\xac\xa4\xdb\xc2\x08\x99\xaf\xfe\xc4\x54\x3d\x20\x6c\x39\x5b"
        "\x3c\x7f\x8b\x35\x08\x3a\x3f\xe3\xc4\x88\x13\xa0\x35\x6f\xc3"
        "\xc1\x48\x1b\x3b\xad\x91\xc1\x60\x65\xd5\x80\x5e\xfa\xd0\x71"
        "\x5b\x67\x2a\x35\x7b\xb9\xbe\x84\x5f\xcb\x40\x68\x5b\xa9\x47"
        "\x98\x43\xf9\x61\x4f\x8c\xae\x9b\xac\xe7\x16\xd5\x96\xd7\x32"
        "\x8f\x33\xa2\x99\x35\x2f\x65\xb1\x83\xd8\x53\xcb\xfb\xff\x19"
        "\xd4\x35\x13\xd0\x64\xd0\x59\xb4\x00\xb6\xfe\x5c\x0f\xa5\x3d"
        "\x0d\xac\x2e\x9c\x9d\x79\x70\x66\x1c\x01\x64\x6e\x6e\x60\x25"
        "\xca\x6e\x6b\x06\x02\x3e\x81\x84\xd3\x1d\x1c\x13\x4b\x8e\x77"
        "\x70\x3b\x5b\xd7\x4f\xbd\xc5\xa8\x7c\xc3\x91\x8d\xaa\x8b\x8a"
        "\x19\x5b\xf5\xb5\x47\x98\x86\xfb\x81\xa5\xad\x63\x3b\x03\x62"
        "\xa9\xf0\x5d\xba\x73\x9a\x1d\x82\x65\xb2\x2e\x8a\xc7\x09\xc2"
        "\x86\xd2\x1e\xd6\xa2\x28\xc4\xb4\xae\x54\x73\x75\x9c\xd3\xbf"
        "\xff\x20\x62\x51\x6c\xdf\x1a\x3b\x6d\x34\x96\x6d\x1a\xe1\xeb"
        "\xac\x57\x12\x71\xd4\x85\xe0\xad\xeb\x92\xf6\x2b\x5a\x36\xf0"
        "\xc6\xf6\xba\x28\x13\x4c\xce\x34\xd7\xdc\xf9\xcc\xd3\xfe\x87"
        "\x6a\x28\x7e\x2c\xad\x19\xe7\x98\x3a\x45\xf9\x44\xe6\x9b\x95"
        "\x67\x6e\xfd\xe1\xe4\xfc\xb6\xd2\xcb\x49\xc2\x8b\xf0\xba\x5b"
        "\xa8\x6f\x63\x20\x3e\x50\x55\x21\x51\x9d\xb9\xf5\x70\xe5\x63"
        "\xf3\xd5\xab\x01\xb7\x83\xd5\x66\x46\xe6\xcf\xb4\x60\xe4\x5f"
        "\x11\xc9\x59\xc3\xf8\x9f\x91\x27\x64\x53\x81\xbe\xcb\xbb\x05"
        "\x47\x20\x7d\x77\x88\xfa\x98\x9a\xc3\x6f\xd7\xd0\x72\x12\xac"
        "\x3f\x77\xb7\x40\xc4\x2d\xcd\x4a\x8e\xfb\x16\x67\x9a\xbe\x04"
        "\xb4\x86\x86\x0e\xf5\xe9\x02\x45\x2d\xfc\xb3\x61\xf2\x86\x4c"
        "\xc4\xd4\xa2\x06\x38\x0d\xae\x0f\x96\x24\x2f\xac\x79\x33\x33"
        "\x5f\x71\x2b\xeb\x64\x14\x7c\x94\x58\xa9\x38\x62\x93\x68\x54"
        "\x70\x00\x8a\x12\xaf\x4a\x07\xc5\xd5\x4f\x11\xef\xdd\x35\x5f"
        "\x96\x6c\x10\x8b\x03\x4f\x7e\x7e\xc8\xaa\xc6\x35\x48\xe9\xb0"
        "\x9a\xf9\x5d\x2b\x4b\xf1\x79\xcf\xda\x5c\xae\xed\xa1\x85\x6f"
        "\xba\xd8\x0f\x4d\x7a\x50\x68\x93\x8f\x5b\xab\x7d\xf1\x03\x04"
        "\x64\xb7\x1d\x62\x1c\xca\x21\x20\x2c\x13\x3c\x68\xd4\xc0\x22"
        "\x38\x3f\xdd\xc9\x67\xb7\x7f\xfc\x67\x78\xc4\x06\x91\x5f\x2e"
        "\x7b\x6d\x84\xb8\x4a\x22\xd6\x76\xae\xdd\x29\x5c\x7c\x6a\xcb"
        "\x97\xb6\xda\x6f\xfb\x58\xae\xf4\x7c\xb5\x95\x12\x18\xeb\x6e"
        "\x95\x7e\x94\xf5\xa3\x7e\x0d\x09\xf7\xf3\x47\x66\x51\xa2\xd5"
        "\xef\x5f\x0d\x01\x1e\xaa\x84\x7c\xf3\x6c\x47\xa3\xaf\xb0\xc1"
        "\xbb\xb4\x69\xb7\x6d\xe5\xb5\x6e\x2d\x2f\xbf\xe6\x00\xb1\x8e"
        "\x35\xc1\xd1\xec\x2f\x15\x9f\xa0\xba\x80\x79\xd0\x3a\x84\x30"
        "\x16\x7a\x69\xfe\xf9\xbf\x5a\x87\x11\x62\x5a\x99\xcb\x1a\x85"
        "\x47\x6a\xa1\x2f\x87\x6d\xec\x2f\xcc\xea\x9a\xc7\xf7\x22\x9f"
        "\x11\xa4\x0c\x4a\x32\xd4\x2c\x70\xd1\xa9\x0c\x96\x95\xc6\x65"
        "\x5f\x12\x42\xd4\xf3\xbf\x6a\x32\x68\x8d\x7a\x35\x37\xf3\x45"
        "\xb6\x0f\x60\x65\x0a\x11\xca\xfa\x55\xc9\x4a\x44\xb5\xf1\x0c"
        "\xde\x08\xb3\xbd\x6c\xfc\x5c\xc0\x7c\xa6\xc9\xd4\x63\xf7\xe7"
        "\xf2\x33\x94\xd9\x5b\x33\x5a\x47\x8d\x01\x26\x24\xdc\xbf\x8b"
        "\x89\x1d\x76\x6b\x02\xe7\xbd\x93\x82\x21\x18\xfe\xda\x2b\xe7"
        "\xcc\xc1\x04\x66\xf5\x5d\x44\x1f\x26\x8c\xa0\x3f\x2c\x08\xa2"
        "\xa2\x3b\x6d\x16\xe8\xb5\x8a\x88\xfc\x96\x30\x2c\xd4\x1c\xe4"
        "\x0d\x05\x69\xd2\x25\x7d\x88\xe7\x38\x8c\x26\x2f\x52\xbd\xed"
        "\xa8\xde\x3e\x4b\x8a\x6c\x41\xd7\xf2\xda\xd4\xb6\x35\x89\xaf"
        "\x0e\x22\xbf\x09\x18\x0f\x43\x63\x7d\xcb\x2c\xf5\x75\xaf\x57"
        "\xfd\x10\x78\xfa\xf1\xdd\xd1\xa4\x49\x0b\xae\xef\x4f\xb3\x94"
        "\x7e\x9e\x9c\x20\x46\x9e\xb8\x2a\x50\x2c\xe0\x9a\x1b\x0b\x1b"
        "\x00\xf7\x49\x64\xfc\xe1\xc1\x1d\xcb\xe6\xce\x5f\x4b\x62\xde"
        "\x1d\xe4\x58\x28\xce\x4b\xa2\x1a\x3d\xc8\xe0\x02\xb4\x9f\x93"
        "\xec\x44\x84\x60\x1b\x07\xdd\xf5\x0c\x59\xc8\xcd\xcc\x48\x04"
        "\x11\x61\x2a\xe3\x4d\x67\x85\x63\xe7\xd8\xb3\xbf\x7b\x41\x32"
        "\x05\x60\x7f\x8b\x51\x3d\xc5\xee\xdc\xea\x10\xb1\xe7\x51\x9a"
        "\xc2\x6b\x62\x9c\x95\xc7\xa5\xda\x43\x90\x90\x10\xfa\x61\xb8"
        "\xe6\x64\x1e\x55\x83\x27\x37\xe7\x95\x16\x33\x20\x10\x50\x9b"
        "\x7d\xd9\x15\x6b\x6f\x44\xc5\x1c\xda\x24\xbd\x4d\x69\xdb\x6b"
        "\x1a\x44\x8d\x9d\xd4\x6e\xc3\xb3\x1c\xda\x37\x05\xa1\x61\x6e"
        "\x27\x66\xbe\xc3\xa5\xa7\x85\x9b\xab\x12\x87\x52\x2e\xf4\x6e"
        "\x6a\xbb\xa2\xf1\xb8\x34\x6f\xb7\x6e\x44\xa2\x78\x0a\x1a\x43"
        "\x07\x28\x9a\x2d\xb4\x8a\xb2\xb9\x32\xe6\x26\x5e\x99\x4e\x4e"
        "\xc2\x2d\xd2\x4e\xb4\xa0\x4a\x48\x27\xb6\x73\x61\xab\xad\xc0"
        "\x2d\xc5\x1c\x70\x98\xd1\x26\x96\x7d\x12\xfa\xe8\xd7\x2e\xf4"
        "\x27\x0d\xfe\x97\xbb\x74\xab\x20\xd8\x52\x63\xf6\x29\x53\x35"
        "\xfe\x34\x75\x43\x6b\xa8\xba\xb3\x08\xa0\x8e\xc9\xbc\xa5\xc5"
        "\x50\x0c\x06\xeb\xb1\xfd\x69\xac\xb6\xe0\xf2\xc4\x5c\x17\x83"
        "\xde\x0e\x40\xfc\x8f\xa0\x25\xaa\x55\xd5\x8d\x6d\xf6\x01\xba"
        "\x48\x1f\x58\x68\xcb\x0a\x86\x34\x21\x9e\xe7\x32\x9d\x4d\x5c"
        "\xb2\xda\xa3\x23\x76\xb7\x56\x4a\xb8\x26\xd3\x87\xeb\xb7\x78"
        "\x62\x7d\xbc\x48\xc0\xef\x2b\xb3\xbe\xb7\x64\x07\xb2\x77\xc1"
        "\x49\xfd\x9a\x23\x79\x15\x51\xe3\xe6\x1a\x59\x8b\x56\xc1\xc8"
        "\x73\x35\x79\x6c\xb2\x22\x61\x40\x9d\xd3\xf0\xfa\x95\x95\x2d"
        "\xda\xbd\xe1\x66\x1b\x12\x5e\x50\x7b\x0f\x92\xb1\x75\x46\x7e"
        "\x27\x90\x77\xcc\x64\xd0\xe3\xb8\xe8\x09\xd3\xc4\x2c\xd3\xc0"
        "\xca\xd0\x15\x4e\x13\x9a\x9b\x86\xbf\x2e\x1a\xd4\xe4\x17\xd5"
        "\x7a\x3e\xc5\xb3\x10\xa8\x23\x1b\xd9\x65\xe3\xa3\x48\x23\x34"
        "\x6e\x4c\x09\xee\x5d\xe1\x63\xb7\x4b\x92\xe5\xdd\xe6\xb4\xc2"
        "\x00\x9e\x7b\x01\x50\x68\x56\x17\x47\xb1\x8b\x62\x03\x0c\xab"
        "\x72\xa7\xbe\x3d\x60\x8e\x73\x6a\x9a\x13\x1f\x3c\xf7\xf1\x59"
        "\xee\x17\x99\x07\xe3\xc3\x5b\x93\xa9\xfd\x06\x93\xb2\xd0\x21"
        "\x27\x54\x8a\x02\x29\x13\x58\x8a\x05\xbb\x6f\xd9\x46\x54\x2f"
        "\x65\x2f\x1b\x4b\xe9\xbe\x23\x45\x91\x21\x8b\xda\x3a\x21\x8a"
        "\xac\x74\x4d\xfd\x0f\xa1\x2b\x37\x6a\xbe\x19\xfd\xfe\xd3\xe9"
        "\xe3\x32\x65\x90\x38\x0e\x8e\x53\x39\xd7\xc2\x9f\x96\x7d\x4c"
        "\x59\xae\xb4\x0b\xe3\x5c\xe0\x9d\xa5\x8e\x7d\x7b\x53\x15\x55"
        "\x2f\x8e\xe1\x78\x71\x16\x56\x73\xe8\xc5\x80\xab\x0a\x28\x0a"
        "\x90\x27\x31\x69\x10\x2d\xd2\x84\xda\x16\x53\xd1\x36\x16\xeb"
        "\x69\x83\xa4\x58\xf2\x6c\xa0\xbe\xff\xa4\xae\xaf\x95\x36\xac"
        "\xfb\x83\x70\x39\x4d\x4e\xb8\xf8\xb3\xee\x04\x2c\xb8\x11\xb6"
        "\xa7\xed\xa8\x0e\x50\x84\x88\xb2\x19\xaa\xa5\x99\xb4\xb1\x17"
        "\x60\x7b\xfb\x3a\x09\x43\x44\x38\xf7\xb6\x78\x49\x5a\x35\xcc"
        "\xb3\x0f\x5f\x2a\x45\x17\x60\x51\x14\x67\x18\x04\x10\xb0\xba"
        "\x38\x2c\x89\x98\x9d\x5d\x52\x25\xfb\xc6\x6a\x0d\x96\xa0\x38"
        "\xcc\xf0\x88\x39\xb9\x07\xe5\x16\x08\xa2\xa7\x2a\xa2\xa9\x92"
        "\x24\x0e\x45\x1f\x34\x5a\x88\x20\x4c\xc5\x54\xe0\x59\x8b\xb8"
        "\x2a\x44\x53\x10\x98\xdc\xb9\x37\xcd\x75\x6c\xca\xd8\xc3\x49"
        "\x98\x22\xdf\x92\xb8\x0e\xd9\xdd\x67\x89\xd5\x0a\x58\xe4\x59"
        "\xd2\x16\x16\x3c\x80\x75\x72\xfd\x08\xe9\x30\x7d\x6c\xc9\x47"
        "\xcf\xab\x58\x2d\xf5\xec\x71\x50\x7a\x17\x4d\xab\x62\xf4\x74"
        "\xf1\x67\xb7\xb7\x5e\x0f\xba\x54\x79\xc6\x36\x4a\x56\xfb\x91"
        "\xa9\x65\x2c\x25\x7e\x67\x40\x16\xc9\xc6\x65\x1f\x49\xcc\xb7"
        "\x30\xb9\x0f\x42\xcc\xb3\xde\xfb\x3b\x60\xa2\xee\x78\x0d\xe7"
        "\xbd\x33\xbf\xba\x59\x61\xcb\x10\x22\xfc\x0e\x3d\x60\xa4\x11"
        "\xd0\x27\x4f\x17\x65\x0f\x6c\x98\x23\x55\x0b\xa4\x39\x2b\xcc"
        "\xe2\x7b\x6d\x87\x51\x84\x9e\x0f\x62\x48\xc8\x45\xbf\x0d\x84"
        "\x3f\xb7\x7a\x32\x40\x4a\x1e\x43\x78\xa6\xaa\xb1\xa2\x18\x5b"
        "\x4c\x16\x9e\xbf\xe0\x87\xbc\x2b\x5d\xf8\xf4\x43\x8e\x29\xd7"
        "\x0e\xe7\x43\x15\xe3\xaa\xc9\x99\xa3\xe0\x1b\xe5\xb4\x2c\xcd"
        "\xd6\x50\xff\x92\x19\x48\x97\xbb\x1e\xe8\x5c\x6e\x6d\xcf\x94"
        "\x13\x5b\x9f\x3a\xfb\x16\xa2\xa8\x94\x1c\x99\xd5\x57\x44\xb7"
        "\xce\x68\x1f\x3f\xdf\x00\x59\xe6\x5b\xed\x48\x75\xdb\xbc\xf0"
        "\x7c\xba\x91\x58\x47\x49\x01\x2d\x3c\x5f\x83\xbb\xc5\x08\x6b"
        "\x3f\x7d\x83\x1f\xa3\xda\x4c\x0e\x18\x6a\x86\x67\xd8\xb2\x30"
        "\xd4\x9a\x4a\xdc\xba\x9a\x8f\xc0\x6d\x30\x04\x32\x46\x4f\x82"
        "\xc1\xcc\x1d\x5d\x4e\xb2\x53\x83\xde\xee\x2c\x7b\x23\x71\x91"
        "\x17\x13\x73\xa7\xfa\x2e\x11\xfb\xb3\x38\xb0\x19\x91\x53\x99"
        "\x45\x2f\x2a\x72\xf3\x7c\x61\x64\x38\x7a\x3f\xd8\xf9\xb1\x63"
        "\xae\x86\x5a\xce\x4e\x6b\x7a\x5e\xae\x35\x55\xca\x2f\x8d\x61"
        "\x9d\xac\x1d\x0b\x69\x38\xd0\xf1\x0f\xff\x54\x2a\x54\xd9\x89"
        "\xe3\xdf\xd7\x6d\xb1\x01\x9b\xba\xcb\x78\x02\x1e\x24\xfd\x81"
        "\x21\x0a\xee\x50\xe5\x90\xdc\x81\xb5\x56\x13\x4b\x10\x67\xc8"
        "\x66\x60\x81\xaa\x6d\xa4\x46\xcb\x3e\x88\xe3\x0d\x13\x2f\xea"
        "\xcb\x4d\x58\xbd\xb5\x8f\x11\xfd\xf9\xec\x0c\x71\xe9\xf3\x7d"
        "\x2a\xcb\x9d\x3d\xb5\x4d\x1b\x23\x7c\x84\xd3\xb1\xb5\x38\x04"
        "\xdb\xef\x1e\xeb\xad\x1c\x0b\x39\x35\x71\x37\x40\x54\xd5\xe1"
        "\x34\xb8\x55\x8a\x3a\x59\x6a\x13\x38\xaa\x11\x14\xf6\x71\x59"
        "\x9d\xf1\x4e\x81\x5b\x28\x9c\xaa\x6f\xc8\x25\x2e\x15\x2f\xf3"
        "\x48\xe6\xe7\xe4\x92\xb5\x26\x64\x99\x47\x01\x87\x89\x39\x93"
        "\x57\xe8\x65\x7e\xfd\x55\xec\xda\x84\xca\x20\xa8\x3c\x00\x6d"
        "\x8f\xaa\xf4\xa9\x73\xa3\x0e\xef\x37\xaa\x3b\x8d\xe3\x0b\x6d"
        "\xdf\xb3\x10\xd8\x32\x35\x37\x04\x56\x47\x49\x21\xae\xcb\xa0"
        "\xd0\x68\xa9\x1e\xf4\x7f\xa8\x6d\xa9\xee\xea\xb3\x48\x64\x83"
        "\x5b\xb7\x9a\xc6\xdd\xce\x17\x87\xdf\xdc\x1b\x7c\x4c\x7f\xa1"
        "\x1e\x38\xfb\x24\xa9\x14\xdf\xc7\x1f\x9a\x34\x6d\x40\x96\x30"
        "\x9a\xf6\x40\x70\xd1\xd4\x05\x18\x8b\xa7\x83\x6c\xd5\xc8\xf8"
        "\x45\x76\xb7\x8e\xdf\xe5\x86\x78\xaf\xcd\x6f\x46\x12\x0e\x01"
        "\xfc\x03\xcd\x67\x85\xe0\x55\x01\x5b\xaf\x83\xc7\x0d\x2d\x08"
        "\xbc\x62\x5f\xed\x38\xbc\xc6\x83\x17\xb2\xf7\x4c\xee\x75\x1c"
        "\x2d\xe6\xb2\x9f\x77\xb8\x97\xb4\x3b\x35\xae\xab\x42\x84\x9c"
        "\x8a\x7f\x15\x11\xc3\x76\x1f\x5e\x0b\xfd\x83\x9c\x98\x9d\x61"
        "\x8d\xf5\x20\x15\xa3\x53\x4f\x5e\xf4\x3d\x22\x77\x53\x4d\x88"
        "\xb3\xb9\xe2\x49\x15\x69\xf5\xe5\x06\x2b\x66\x48\x38\xb2\xac"
        "\xf1\x98\xa7\xc9\x68\x2b\xd3\x6a\x4f\xcb\xf4\x8e\xd2\x02\x51"
        "\x76",
        4096);
    r[42] = syscall(__NR_sendmsg, r[1], 0x20267000ul, 0x8000ul);
    break;
  case 4:
    memcpy((void*)0x20925000, "\xe0\x05", 2);
    *(uint16_t*)0x209e1000 = (uint16_t)0xa;
    *(uint16_t*)0x209e1002 = (uint16_t)0x204e;
    *(uint32_t*)0x209e1004 = (uint32_t)0x0;
    *(uint64_t*)0x209e1008 = (uint64_t)0x0;
    *(uint64_t*)0x209e1010 = (uint64_t)0x100000000000000;
    *(uint32_t*)0x209e1018 = (uint32_t)0x0;
    r[50] = syscall(__NR_sendto, r[1], 0x20925000ul, 0x2ul, 0x0ul,
                    0x209e1000ul, 0x1cul);
    break;
  case 5:
    *(uint32_t*)0x203f2000 = (uint32_t)0x184cc268;
    r[52] = syscall(__NR_setsockopt, r[1], 0x1ul, 0x7ul, 0x203f2000ul,
                    0x4ul);
    break;
  case 6:
    memcpy((void*)0x20f78ffa, "\xd0", 1);
    *(uint16_t*)0x20acffe4 = (uint16_t)0xa;
    *(uint16_t*)0x20acffe6 = (uint16_t)0x204e;
    *(uint32_t*)0x20acffe8 = (uint32_t)0x6;
    *(uint64_t*)0x20acffec = (uint64_t)0x0;
    *(uint64_t*)0x20acfff4 = (uint64_t)0x100000000000000;
    *(uint32_t*)0x20acfffc = (uint32_t)0x0;
    r[60] = syscall(__NR_sendto, r[1], 0x20f78ffaul, 0x1ul, 0x4040ul,
                    0x20acffe4ul, 0x1cul);
    break;
  case 7:
    r[61] = syscall(__NR_socket, 0x2ul, 0x806ul, 0x0ul);
    break;
  case 8:
    memcpy((void*)0x20915fe0, "\x6c\x6f\x00\x00\x00\x00\x00\x00\x00\x00"
                              "\x00\x00\x00\x00\x00\x00",
           16);
    *(uint16_t*)0x20915ff0 = (uint16_t)0xfffffffffffffffd;
    r[64] = syscall(__NR_ioctl, r[61], 0x8914ul, 0x20915fe0ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[18];

  memset(r, -1, sizeof(r));
  for (i = 0; i < 9; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  usleep(rand() % 100000);
}

int main()
{
  use_temporary_dir();
  int pid = do_sandbox_namespace(0, false);
  int status = 0;
  while (waitpid(pid, &status, __WALL) != pid) {
  }
  return 0;
}