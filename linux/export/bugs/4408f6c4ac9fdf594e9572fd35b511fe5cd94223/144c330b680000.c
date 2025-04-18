// https://syzkaller.appspot.com/bug?id=4408f6c4ac9fdf594e9572fd35b511fe5cd94223
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
      if (current_time_ms() - start < 5000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  memcpy((void*)0x20002040, "./file0\000", 8);
  syscall(__NR_mkdirat, /*fd=*/0xffffff9c, /*path=*/0x20002040ul, /*mode=*/0ul);
  res = syscall(__NR_pipe2, /*pipefd=*/0x20000240ul, /*flags=*/0ul);
  if (res != -1) {
    r[0] = *(uint32_t*)0x20000240;
    r[1] = *(uint32_t*)0x20000244;
  }
  memcpy((void*)0x20000080,
         "\x15\x00\x00\x00\x65\xff\xff\x09\x7b\x00\x00\x08\x00\x39\x50\x32\x30"
         "\x30\x30\x2e\x4c",
         21);
  syscall(__NR_write, /*fd=*/r[1], /*data=*/0x20000080ul, /*size=*/0x15ul);
  res = syscall(__NR_dup, /*oldfd=*/r[1]);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x200001c0 = 0x18;
  *(uint32_t*)0x200001c4 = 0;
  *(uint64_t*)0x200001c8 = 0;
  *(uint64_t*)0x200001d0 = 0;
  syscall(__NR_write, /*fd=*/r[2], /*arg=*/0x200001c0ul, /*len=*/0x18ul);
  *(uint32_t*)0x200000c0 = 0x14c;
  *(uint32_t*)0x200000c4 = 5;
  *(uint64_t*)0x200000c8 = 0;
  *(uint64_t*)0x200000d0 = 0;
  *(uint64_t*)0x200000d8 = 0;
  *(uint64_t*)0x200000e0 = 0;
  *(uint32_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000ec = 0;
  syscall(__NR_write, /*fd=*/r[2], /*arg=*/0x200000c0ul, /*len=*/0x137ul);
  memcpy((void*)0x200003c0, "./file0\000", 8);
  memcpy((void*)0x20000b80, "9p\000", 3);
  memcpy((void*)0x20000300, "trans=fd,", 9);
  memcpy((void*)0x20000309, "rfdno", 5);
  *(uint8_t*)0x2000030e = 0x3d;
  sprintf((char*)0x2000030f, "0x%016llx", (long long)r[0]);
  *(uint8_t*)0x20000321 = 0x2c;
  memcpy((void*)0x20000322, "wfdno", 5);
  *(uint8_t*)0x20000327 = 0x3d;
  sprintf((char*)0x20000328, "0x%016llx", (long long)r[2]);
  *(uint8_t*)0x2000033a = 0x2c;
  memcpy((void*)0x2000033b, "posixacl", 8);
  *(uint8_t*)0x20000343 = 0x2c;
  *(uint8_t*)0x20000344 = 0;
  syscall(__NR_mount, /*src=*/0ul, /*dst=*/0x200003c0ul, /*type=*/0x20000b80ul,
          /*flags=*/0ul, /*opts=*/0x20000300ul);
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul, /*prot=*/7ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  loop();
  return 0;
}
