// https://syzkaller.appspot.com/bug?id=fcb98dbab1d08063653630b4695adaae8e546515
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
  int i;
  for (i = 0; i < 100; i++) {
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
  int iter;
  for (iter = 0;; iter++) {
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
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      sleep_ms(1);
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[1] = {0x0};

void execute_one(void)
{
  intptr_t res = 0;
  syscall(__NR_prctl, 0x59616d61ul, -1, 0, 0);
  syscall(__NR_clone, 0x100ul, 0ul, 0x9999999999999999ul, 0ul, -1ul);
  res = syscall(__NR_gettid);
  if (res != -1)
    r[0] = res;
  syscall(__NR_wait4, 0, 0ul, 0x80000002ul, 0ul);
  *(uint64_t*)0x200000c0 = 0;
  *(uint64_t*)0x200000c8 = 0;
  *(uint64_t*)0x200000d0 = 0;
  *(uint64_t*)0x200000d8 = 0;
  *(uint64_t*)0x200000e0 = 0;
  *(uint64_t*)0x200000e8 = 0;
  *(uint64_t*)0x200000f0 = 0x20000100;
  memcpy((void*)0x20000100,
         "\x66\x53\x07\x00\x00\x05\x3c\x27\xbc\x33\x76\x00\x36\x39\x40\x5c\xb4"
         "\xae\xd7\xd1\x2f\x00\x00\x00\x15\x00\xae\x47\xa8\x25\xd8\xad\x79\xa4"
         "\x18\xcf\xf4\x7d\x01\x00\x00\x80\x5a\xcf\x4f\x8f\x36\x46\x00\x00\x00"
         "\x14\x79\xae\xd7\x5d\x49\x2b\x41\x5b\xce\xe0\x0a\x06\xdc\x9d\x8e\x99"
         "\xad\x2f\x81\x30\xdb\x0b\xf6\x7a\x66\xd4\xcd\x2c\x7f\x24\xb5\xdc\xfc"
         "\x6a\xfd\x98\x3f\x79\xe6\x51\x99\x06\x00\x07\x67\x6f\x8f\x9f\xc0\xeb"
         "\xf8\xb0\xb1\x6d\x86\xa6\xa4\x02\xce\x78\x3a\xa5\xbf\xc3\x9e\x6f\x2c"
         "\x64\x88\x4b\x3c\x5d\x05\x69\x2e\x66\x4e\xbf\x68\xe6\xfa\xa5\x33\x67"
         "\xf0\x5f\x4a\xd6\x14\x21\x34\xb6\x2f\x11\xe9\x31\xe7\xd6\x2e\xad\x03"
         "\x3c\xd2\x15\x7d\x13\x5a",
         159);
  *(uint64_t*)0x200000f8 = 0x9f;
  syscall(__NR_vmsplice, -1, 0x200000c0ul, 4ul, 0ul);
  syscall(__NR_ptrace, 0x4206ul, r[0], 0ul, 0ul);
  syscall(__NR_tkill, r[0], 0x3c);
  syscall(__NR_ptrace, 0xdul, r[0], 0ul, 0x20000080ul);
  syscall(__NR_ptrace, 0x20ul, r[0], 0ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}