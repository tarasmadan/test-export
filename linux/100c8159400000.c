// https://syzkaller.appspot.com/bug?id=0766d38c656abeace60621896d705743aeefed51
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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
  rlim.rlim_cur = rlim.rlim_max = 160 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
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

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);
  setup_common();
  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }
  loop();
  exit(1);
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void loop(void)
{
  long res = 0;
  res = syscall(__NR_socketpair, 1, 2, 0, 0x20000100);
  if (res != -1)
    r[0] = *(uint32_t*)0x20000104;
  memcpy((void*)0x20000040, "/dev/net/tun", 13);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000040, 0, 0);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000000,
         "\x65\x71\x6c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint16_t*)0x20000010 = 0x802;
  syscall(__NR_ioctl, r[1], 0x400454ca, 0x20000000);
  syscall(__NR_ioctl, r[1], 0x400454cd, 0x30a);
  memcpy((void*)0x200000c0,
         "\x65\x71\x6c\x00\x00\x00\xa9\x5b\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint32_t*)0x200000d0 = 1;
  syscall(__NR_ioctl, r[0], 0x8914, 0x200000c0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  do_sandbox_none();
  return 0;
}
