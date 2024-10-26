// https://syzkaller.appspot.com/bug?id=98543b7e19e51d6740a133e1d3edf2c52dbc2daf
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
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

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

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

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

#define USLEEP_FORKED_CHILD (3 * 50 * 1000)

static long handle_clone_ret(long ret)
{
  if (ret != 0) {
    return ret;
  }
  usleep(USLEEP_FORKED_CHILD);
  syscall(__NR_exit, 0);
  while (1) {
  }
}

static long syz_clone(volatile long flags, volatile long stack,
                      volatile long stack_len, volatile long ptid,
                      volatile long ctid, volatile long tls)
{
  long sp = (stack + stack_len) & ~15;
  long ret = (long)syscall(__NR_clone, flags & ~CLONE_VM, sp, ptid, ctid, tls);
  return handle_clone_ret(ret);
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

uint64_t r[1] = {0x0};

void execute_one(void)
{
  intptr_t res = 0;
  res = -1;
  res = syz_clone(0, 0, 0, 0, 0, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000480 = 2;
  *(uint32_t*)0x20000484 = 0x80;
  *(uint8_t*)0x20000488 = 1;
  *(uint8_t*)0x20000489 = 3;
  *(uint8_t*)0x2000048a = 0;
  *(uint8_t*)0x2000048b = 0;
  *(uint32_t*)0x2000048c = 0;
  *(uint64_t*)0x20000490 = 0xb5;
  *(uint64_t*)0x20000498 = 0;
  *(uint64_t*)0x200004a0 = 2;
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 1, 1, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 1, 3, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 5, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 1, 6, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 1, 7, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 1, 24, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 29, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 30, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 31, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 32, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 33, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 1, 34, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 1, 35, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 36, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 37, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 38, 26);
  *(uint32_t*)0x200004b0 = 3;
  *(uint32_t*)0x200004b4 = 0;
  *(uint64_t*)0x200004b8 = 0;
  *(uint64_t*)0x200004c0 = 4;
  *(uint64_t*)0x200004c8 = 0x88008;
  *(uint64_t*)0x200004d0 = 0;
  *(uint32_t*)0x200004d8 = 8;
  *(uint32_t*)0x200004dc = 8;
  *(uint64_t*)0x200004e0 = 0x7c8;
  *(uint32_t*)0x200004e8 = 0x6d;
  *(uint16_t*)0x200004ec = 0x3ff;
  *(uint16_t*)0x200004ee = 0;
  *(uint32_t*)0x200004f0 = -1;
  *(uint32_t*)0x200004f4 = 0;
  *(uint64_t*)0x200004f8 = 0x7fffffffffffffff;
  syscall(__NR_perf_event_open, 0x20000480ul, r[0], 0ul, -1, 8ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
