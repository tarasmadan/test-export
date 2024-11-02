// https://syzkaller.appspot.com/bug?id=4f3f8a33785d1cf028fb80dafd9ee9d9cbfa2106
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#ifndef __NR_io_uring_register
#define __NR_io_uring_register 427
#endif
#ifndef __NR_io_uring_setup
#define __NR_io_uring_setup 425
#endif
#ifndef __NR_ioctl
#define __NR_ioctl 54
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_userfaultfd
#define __NR_userfaultfd 388
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

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
      exit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      sleep_ms(10);
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      if (current_time_ms() - start < 15000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  res = syscall(__NR_userfaultfd, /*flags=UFFD_USER_MODE_ONLY*/ 1);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x200000c0 = 0xaa;
  *(uint64_t*)0x200000c8 = 0;
  *(uint64_t*)0x200000d0 = 0;
  syscall(__NR_ioctl, /*fd=*/(intptr_t)r[0], /*cmd=*/0xc018aa3f,
          /*arg=*/0x200000c0);
  *(uint32_t*)0x20000404 = 0xfffffffd;
  *(uint32_t*)0x20000408 = 0;
  *(uint32_t*)0x2000040c = 0x4001;
  *(uint32_t*)0x20000410 = 0;
  *(uint32_t*)0x20000418 = -1;
  memset((void*)0x2000041c, 0, 12);
  res = syscall(__NR_io_uring_setup, /*entries=*/0x3eae, /*params=*/0x20000400);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x200002c0 = 0x20001700;
  *(uint32_t*)0x200002c4 = 0x440000;
  syscall(__NR_io_uring_register, /*fd=*/(intptr_t)r[1], /*opcode=*/0,
          /*arg=*/0x200002c0, /*nr_args=*/0x11a);
  *(uint64_t*)0x20000040 = 0x200e2000;
  *(uint64_t*)0x20000048 = 0xc00000;
  *(uint64_t*)0x20000050 = 1;
  *(uint64_t*)0x20000058 = 0;
  syscall(__NR_ioctl, /*fd=*/(intptr_t)r[0], /*cmd=*/0xc020aa00,
          /*arg=*/0x20000040);
  *(uint64_t*)0x20000080 = 0x20c15000;
  *(uint64_t*)0x20000088 = 0x20508000;
  *(uint64_t*)0x20000090 = 0x1000;
  *(uint64_t*)0x20000098 = 0;
  *(uint64_t*)0x200000a0 = 0;
  syscall(__NR_ioctl, /*fd=*/(intptr_t)r[0], /*cmd=*/0xc028aa05,
          /*arg=*/0x20000080);
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000, /*len=*/0x1000, /*prot=*/0,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32, /*fd=*/-1,
          /*offset=*/0);
  syscall(__NR_mmap, /*addr=*/0x20000000, /*len=*/0x1000000,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32, /*fd=*/-1,
          /*offset=*/0);
  syscall(__NR_mmap, /*addr=*/0x21000000, /*len=*/0x1000, /*prot=*/0,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32, /*fd=*/-1,
          /*offset=*/0);
  const char* reason;
  (void)reason;
  loop();
  return 0;
}
