// https://syzkaller.appspot.com/bug?id=6d412510c4799ff347cfae839f69506cde183ef8
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
  res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20003e00 = 0;
  *(uint32_t*)0x20003e08 = 0;
  *(uint64_t*)0x20003e10 = 0x20003dc0;
  *(uint64_t*)0x20003dc0 = 0x200002c0;
  *(uint32_t*)0x200002c0 = 0x14;
  *(uint16_t*)0x200002c4 = 0x10;
  *(uint16_t*)0x200002c6 = 1;
  *(uint32_t*)0x200002c8 = 0;
  *(uint32_t*)0x200002cc = 0;
  *(uint8_t*)0x200002d0 = 0;
  *(uint8_t*)0x200002d1 = 0;
  *(uint16_t*)0x200002d2 = htobe16(0xa);
  *(uint32_t*)0x200002d4 = 0x20;
  *(uint8_t*)0x200002d8 = 0;
  *(uint8_t*)0x200002d9 = 0xa;
  *(uint16_t*)0x200002da = 0x1405;
  *(uint32_t*)0x200002dc = 0;
  *(uint32_t*)0x200002e0 = 0;
  *(uint8_t*)0x200002e4 = 1;
  *(uint8_t*)0x200002e5 = 0;
  *(uint16_t*)0x200002e6 = htobe16(0);
  *(uint16_t*)0x200002e8 = 9;
  *(uint16_t*)0x200002ea = 1;
  memcpy((void*)0x200002ec, "syz0\000", 5);
  *(uint32_t*)0x200002f4 = 0x60;
  *(uint8_t*)0x200002f8 = 0x16;
  *(uint8_t*)0x200002f9 = 0xa;
  *(uint16_t*)0x200002fa = 1;
  *(uint32_t*)0x200002fc = 0;
  *(uint32_t*)0x20000300 = 0;
  *(uint8_t*)0x20000304 = 1;
  *(uint8_t*)0x20000305 = 0;
  *(uint16_t*)0x20000306 = htobe16(0);
  *(uint16_t*)0x20000308 = 9;
  *(uint16_t*)0x2000030a = 1;
  memcpy((void*)0x2000030c, "syz0\000", 5);
  *(uint16_t*)0x20000314 = 9;
  *(uint16_t*)0x20000316 = 2;
  memcpy((void*)0x20000318, "syz2\000", 5);
  *(uint16_t*)0x20000320 = 0x2c;
  STORE_BY_BITMASK(uint16_t, , 0x20000322, 3, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000323, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000323, 1, 7, 1);
  *(uint16_t*)0x20000324 = 0x18;
  STORE_BY_BITMASK(uint16_t, , 0x20000326, 3, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000327, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000327, 1, 7, 1);
  *(uint16_t*)0x20000328 = 0x14;
  *(uint16_t*)0x2000032a = 1;
  memcpy((void*)0x2000032c,
         "sit0\000\000\000\000\000\000\000\000\000\000\000\000", 16);
  *(uint16_t*)0x2000033c = 8;
  STORE_BY_BITMASK(uint16_t, , 0x2000033e, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000033f, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000033f, 0, 7, 1);
  *(uint32_t*)0x20000340 = htobe32(0);
  *(uint16_t*)0x20000344 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x20000346, 1, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000347, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000347, 0, 7, 1);
  *(uint32_t*)0x20000348 = htobe32(0);
  *(uint16_t*)0x2000034c = 8;
  STORE_BY_BITMASK(uint16_t, , 0x2000034e, 7, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000034f, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000034f, 0, 7, 1);
  *(uint32_t*)0x20000350 = htobe32(1);
  *(uint32_t*)0x20000354 = 0x14;
  *(uint16_t*)0x20000358 = 0x11;
  *(uint16_t*)0x2000035a = 1;
  *(uint32_t*)0x2000035c = 0;
  *(uint32_t*)0x20000360 = 0;
  *(uint8_t*)0x20000364 = 0;
  *(uint8_t*)0x20000365 = 0;
  *(uint16_t*)0x20000366 = htobe16(0xa);
  *(uint64_t*)0x20003dc8 = 0xa8;
  *(uint64_t*)0x20003e18 = 1;
  *(uint64_t*)0x20003e20 = 0;
  *(uint64_t*)0x20003e28 = 0;
  *(uint32_t*)0x20003e30 = 0;
  syscall(__NR_sendmsg, r[0], 0x20003e00ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  setup_leak();
  loop();
  return 0;
}