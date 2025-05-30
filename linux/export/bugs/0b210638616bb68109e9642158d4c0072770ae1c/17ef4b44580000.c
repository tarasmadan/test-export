// https://syzkaller.appspot.com/bug?id=0b210638616bb68109e9642158d4c0072770ae1c
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

#ifndef __NR_fsopen
#define __NR_fsopen 430
#endif

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
  syscall(__NR_socket, /*domain=AF_NETLINK*/ 0x10ul, /*type=SOCK_DGRAM*/ 2ul,
          /*proto=*/0);
  syscall(__NR_sendmsg, /*fd=*/-1, /*msg=*/0ul, /*f=MSG_BATCH*/ 0x40000ul);
  syscall(__NR_mmap, /*addr=*/0ul, /*len=*/0x4000000000000004ul,
          /*prot=*/0x40000000005ful, /*flags=*/0x40eb2ul, /*fd=*/7,
          /*off=*/0x300000000000ul);
  syscall(__NR_mmap, /*addr=*/0ul, /*len=*/0x400008ul, /*prot=*/0xdful,
          /*flags=*/0x9b72ul, /*fd=*/2, /*off=*/0x8000ul);
  *(uint64_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c8 = 0;
  *(uint64_t*)0x200000d0 = 0x20000200;
  *(uint64_t*)0x20000200 = 0;
  *(uint64_t*)0x20000208 = 0x1ac;
  *(uint64_t*)0x200000d8 = 1;
  *(uint64_t*)0x200000e0 = 0;
  *(uint64_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000f0 = 0x4004810;
  syscall(__NR_sendmsg, /*fd=*/-1, /*msg=*/0x200000c0ul,
          /*f=MSG_CONFIRM*/ 0x800ul);
  syscall(__NR_sendmsg, /*fd=*/-1, /*msg=*/0ul, /*f=MSG_BATCH*/ 0x40000ul);
  syscall(__NR_sendmmsg, /*fd=*/-1, /*mmsg=*/0ul, /*vlen=*/3, /*flags=*/0);
  syscall(__NR_fsopen, /*_fs_name=*/0ul, /*flags=*/1);
  syscall(
      __NR_openat, /*fd=*/0xffffffffffffff9cul, /*file=*/0ul,
      /*flags=__O_TMPFILE|O_TRUNC|O_NOCTTY|O_CLOEXEC|O_RDWR|O_WRONLY|0x100000*/
      0x580303, /*mode=*/0);
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
  loop();
  return 0;
}
