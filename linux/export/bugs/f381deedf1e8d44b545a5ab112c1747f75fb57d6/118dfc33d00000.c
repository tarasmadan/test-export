// https://syzkaller.appspot.com/bug?id=f381deedf1e8d44b545a5ab112c1747f75fb57d6
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
      if (current_time_ms() - start < 5000) {
        continue;
      }
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  memcpy((void*)0x200001c0,
         "NETMAP\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000",
         29);
  *(uint8_t*)0x200001dd = 0;
  *(uint32_t*)0x20000280 = 0x1e;
  syscall(__NR_getsockopt, -1, 0, 0x63, 0x200001c0ul, 0x20000280ul);
  memcpy((void*)0x20000080, "/dev/snd/seq\000", 13);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000080ul, 2ul, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000100 = 0;
  *(uint32_t*)0x20000104 = 0;
  STORE_BY_BITMASK(uint32_t, , 0x20000108, 0, 0, 1);
  memcpy((void*)0x20000109,
         "queue1\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000",
         64);
  *(uint32_t*)0x2000014c = 0;
  memset((void*)0x20000150, 0, 60);
  syscall(__NR_ioctl, r[0], 0xc08c5332, 0x20000100ul);
  *(uint8_t*)0x200003c0 = 0xfe;
  *(uint8_t*)0x200003c1 = 0x80;
  memset((void*)0x200003c2, 0, 13);
  *(uint8_t*)0x200003cf = 0xaa;
  *(uint32_t*)0x200003d0 = htobe32(0xe0000001);
  *(uint16_t*)0x200003e0 = htobe16(0);
  *(uint16_t*)0x200003e2 = htobe16(0);
  *(uint16_t*)0x200003e4 = htobe16(0);
  *(uint16_t*)0x200003e6 = htobe16(0);
  *(uint16_t*)0x200003e8 = 0;
  *(uint8_t*)0x200003ea = 0;
  *(uint8_t*)0x200003eb = 0;
  *(uint8_t*)0x200003ec = 0;
  *(uint32_t*)0x200003f0 = 0;
  *(uint32_t*)0x200003f4 = 0;
  *(uint64_t*)0x200003f8 = 0;
  *(uint64_t*)0x20000400 = 0;
  *(uint64_t*)0x20000408 = 0;
  *(uint64_t*)0x20000410 = 0;
  *(uint64_t*)0x20000418 = 0;
  *(uint64_t*)0x20000420 = 0;
  *(uint64_t*)0x20000428 = 0;
  *(uint64_t*)0x20000430 = 0;
  *(uint64_t*)0x20000438 = 0;
  *(uint64_t*)0x20000440 = 0;
  *(uint64_t*)0x20000448 = 0;
  *(uint64_t*)0x20000450 = 0;
  *(uint32_t*)0x20000458 = 0;
  *(uint32_t*)0x2000045c = 0;
  *(uint8_t*)0x20000460 = 0;
  *(uint8_t*)0x20000461 = 0;
  *(uint8_t*)0x20000462 = 0;
  *(uint8_t*)0x20000463 = 0;
  *(uint32_t*)0x20000468 = htobe32(0);
  *(uint32_t*)0x20000478 = htobe32(0);
  *(uint8_t*)0x2000047c = 0;
  *(uint16_t*)0x20000480 = 0;
  memcpy((void*)0x20000484,
         " \001\000\000\000\000\000\000\000\000\000\000\000\000\000\000", 16);
  *(uint32_t*)0x20000494 = 0;
  *(uint8_t*)0x20000498 = 0;
  *(uint8_t*)0x20000499 = 0;
  *(uint8_t*)0x2000049a = 0;
  *(uint32_t*)0x2000049c = 0;
  *(uint32_t*)0x200004a0 = 0;
  *(uint32_t*)0x200004a4 = 0;
  syscall(__NR_setsockopt, -1, 0, 0x11, 0x200003c0ul, 0xe8ul);
  *(uint32_t*)0x20000200 = 0;
  *(uint32_t*)0x20000204 = 0;
  *(uint32_t*)0x20000208 = 2;
  *(uint32_t*)0x2000020c = 0;
  *(uint32_t*)0x20000210 = 0;
  *(uint32_t*)0x20000214 = 0;
  *(uint32_t*)0x20000218 = 0;
  *(uint32_t*)0x2000021c = 0x3ff;
  memset((void*)0x20000220, 0, 64);
  syscall(__NR_ioctl, r[0], 0x40605346, 0x20000200ul);
  *(uint8_t*)0x200000c0 = 0;
  *(uint8_t*)0x200000c1 = 0;
  *(uint8_t*)0x200000c2 = 0;
  *(uint8_t*)0x200000c3 = 0;
  *(uint32_t*)0x200000c4 = 0;
  *(uint32_t*)0x200000c8 = 0;
  *(uint8_t*)0x200000cc = 0;
  *(uint8_t*)0x200000cd = 0;
  *(uint8_t*)0x200000ce = 0;
  *(uint8_t*)0x200000cf = 0;
  memcpy((void*)0x200000d0, "\xa3\x57\xb6\xb1\x40\xcb\xb6\x21\x5d\xd3\x34\x59",
         12);
  syscall(__NR_write, r[0], 0x200000c0ul, 0xfffffee4ul);
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
