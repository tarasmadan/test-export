// https://syzkaller.appspot.com/bug?id=70d491b839984c756992b68c93e666828a4a301c
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

#define KMEMLEAK_FILE "/sys/kernel/debug/kmemleak"

static void setup_leak()
{
  if (!write_file(KMEMLEAK_FILE, "scan"))
    exit(1);
  sleep(5);
  if (!write_file(KMEMLEAK_FILE, "scan"))
    exit(1);
  if (!write_file(KMEMLEAK_FILE, "clear"))
    exit(1);
}

static void check_leaks(void)
{
  int fd = open(KMEMLEAK_FILE, O_RDWR);
  if (fd == -1)
    exit(1);
  uint64_t start = current_time_ms();
  if (write(fd, "scan", 4) != 4)
    exit(1);
  sleep(1);
  while (current_time_ms() - start < 4 * 1000)
    sleep(1);
  if (write(fd, "scan", 4) != 4)
    exit(1);
  static char buf[128 << 10];
  ssize_t n = read(fd, buf, sizeof(buf) - 1);
  if (n < 0)
    exit(1);
  int nleaks = 0;
  if (n != 0) {
    sleep(1);
    if (write(fd, "scan", 4) != 4)
      exit(1);
    if (lseek(fd, 0, SEEK_SET) < 0)
      exit(1);
    n = read(fd, buf, sizeof(buf) - 1);
    if (n < 0)
      exit(1);
    buf[n] = 0;
    char* pos = buf;
    char* end = buf + n;
    while (pos < end) {
      char* next = strstr(pos + 1, "unreferenced object");
      if (!next)
        next = end;
      char prev = *next;
      *next = 0;
      fprintf(stderr, "BUG: memory leak\n%s\n", pos);
      *next = prev;
      pos = next;
      nleaks++;
    }
  }
  if (write(fd, "clear", 5) != 5)
    exit(1);
  close(fd);
  if (nleaks)
    exit(1);
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
    check_leaks();
  }
}

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 0x80002ul, 0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20002980 = 0;
  *(uint32_t*)0x20002988 = 0;
  *(uint64_t*)0x20002990 = 0x20002940;
  *(uint64_t*)0x20002940 = 0x200000c0;
  *(uint32_t*)0x200000c0 = 0x58;
  *(uint16_t*)0x200000c4 = 0x30;
  *(uint16_t*)0x200000c6 = 0x53b;
  *(uint32_t*)0x200000c8 = 0;
  *(uint32_t*)0x200000cc = 0;
  *(uint8_t*)0x200000d0 = 0;
  *(uint8_t*)0x200000d1 = 0;
  *(uint16_t*)0x200000d2 = 0;
  *(uint16_t*)0x200000d4 = 0x44;
  *(uint16_t*)0x200000d6 = 1;
  *(uint16_t*)0x200000d8 = 0x40;
  STORE_BY_BITMASK(uint16_t, , 0x200000da, 1, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200000db, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200000db, 0, 7, 1);
  *(uint16_t*)0x200000dc = 0xb;
  *(uint16_t*)0x200000de = 1;
  memcpy((void*)0x200000e0, "ctinfo\000", 7);
  *(uint16_t*)0x200000e8 = 0x2c;
  STORE_BY_BITMASK(uint16_t, , 0x200000ea, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200000eb, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200000eb, 1, 7, 1);
  *(uint16_t*)0x200000ec = 0x18;
  *(uint16_t*)0x200000ee = 3;
  *(uint32_t*)0x200000f0 = 0;
  *(uint32_t*)0x200000f4 = 0;
  *(uint32_t*)0x200000f8 = 0;
  *(uint32_t*)0x200000fc = 0;
  *(uint32_t*)0x20000100 = 0;
  *(uint16_t*)0x20000104 = 6;
  *(uint16_t*)0x20000106 = 4;
  *(uint16_t*)0x20000108 = 0;
  *(uint16_t*)0x2000010c = 8;
  *(uint16_t*)0x2000010e = 7;
  *(uint32_t*)0x20000110 = 0;
  *(uint16_t*)0x20000114 = 4;
  *(uint16_t*)0x20000116 = 6;
  *(uint64_t*)0x20002948 = 0x58;
  *(uint64_t*)0x20002998 = 1;
  *(uint64_t*)0x200029a0 = 0;
  *(uint64_t*)0x200029a8 = 0;
  *(uint32_t*)0x200029b0 = 0;
  syscall(__NR_sendmsg, r[0], 0x20002980ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  setup_leak();
  loop();
  return 0;
}