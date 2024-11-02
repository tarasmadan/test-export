// https://syzkaller.appspot.com/bug?id=be49c2247d07ad4a4faa018ebea0432f6f0c1a82
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

#ifndef __NR_bpf
#define __NR_bpf 386
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
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

void execute_one(void)
{
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  *(uint32_t*)0x20000680 = 0x10;
  *(uint32_t*)0x20000684 = 4;
  *(uint64_t*)0x20000688 = 0x20000380;
  memcpy((void*)0x20000380,
         "\x18\x02\x00\x00\x00\xc4\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x85"
         "\x00\x00\x00\x3e\x00\x00\x00\x95",
         25);
  *(uint64_t*)0x20000690 = 0x200000c0;
  memcpy((void*)0x200000c0, "GPL\000", 4);
  *(uint32_t*)0x20000698 = 1;
  *(uint32_t*)0x2000069c = 0;
  *(uint64_t*)0x200006a0 = 0;
  *(uint32_t*)0x200006a8 = 0;
  *(uint32_t*)0x200006ac = 0;
  memset((void*)0x200006b0, 0, 16);
  *(uint32_t*)0x200006c0 = 0;
  *(uint32_t*)0x200006c4 = 0;
  *(uint32_t*)0x200006c8 = -1;
  *(uint32_t*)0x200006cc = 8;
  *(uint64_t*)0x200006d0 = 0;
  *(uint32_t*)0x200006d8 = 0;
  *(uint32_t*)0x200006dc = 0x10;
  *(uint64_t*)0x200006e0 = 0;
  *(uint32_t*)0x200006e8 = 0;
  *(uint32_t*)0x200006ec = 0;
  *(uint32_t*)0x200006f0 = 0;
  *(uint32_t*)0x200006f4 = 0;
  *(uint64_t*)0x200006f8 = 0;
  *(uint64_t*)0x20000700 = 0;
  *(uint32_t*)0x20000708 = 0x10;
  *(uint32_t*)0x2000070c = 0;
  *(uint32_t*)0x20000710 = 0;
  syscall(__NR_bpf, /*cmd=*/5, /*arg=*/0x20000680, /*size=*/0x94);
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