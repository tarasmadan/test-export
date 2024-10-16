// https://syzkaller.appspot.com/bug?id=a1fe53659d56f14f5aa9d635808b68ae080d83c7
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

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 4);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20001500 = 0;
  *(uint32_t*)0x20001508 = 0;
  *(uint64_t*)0x20001510 = 0x20000280;
  *(uint64_t*)0x20000280 = 0x200012c0;
  *(uint32_t*)0x200012c0 = 0x50;
  *(uint16_t*)0x200012c4 = 0x13;
  *(uint16_t*)0x200012c6 = 0x211;
  *(uint32_t*)0x200012c8 = 0;
  *(uint32_t*)0x200012cc = 0;
  *(uint8_t*)0x200012d0 = 0;
  *(uint8_t*)0x200012d1 = 0;
  *(uint8_t*)0x200012d2 = 0;
  *(uint8_t*)0x200012d3 = 0;
  *(uint16_t*)0x200012d4 = htobe16(0);
  *(uint16_t*)0x200012d6 = htobe16(0);
  *(uint32_t*)0x200012d8 = 0;
  *(uint32_t*)0x200012dc = 0;
  *(uint32_t*)0x200012e0 = 0;
  *(uint32_t*)0x200012e4 = 0;
  *(uint32_t*)0x200012e8 = 0;
  *(uint32_t*)0x200012ec = 0;
  *(uint32_t*)0x200012f0 = 0;
  *(uint32_t*)0x200012f4 = 0;
  *(uint32_t*)0x200012f8 = 0;
  *(uint32_t*)0x200012fc = 0;
  *(uint32_t*)0x20001300 = 0;
  *(uint32_t*)0x20001304 = 0;
  *(uint32_t*)0x20001308 = 0;
  *(uint16_t*)0x2000130c = 4;
  *(uint16_t*)0x2000130e = 3;
  *(uint64_t*)0x20000288 = 0x50;
  *(uint64_t*)0x20001518 = 1;
  *(uint64_t*)0x20001520 = 0;
  *(uint64_t*)0x20001528 = 0;
  *(uint32_t*)0x20001530 = 0;
  syscall(__NR_sendmsg, r[0], 0x20001500ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
