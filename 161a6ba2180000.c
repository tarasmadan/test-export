// https://syzkaller.appspot.com/bug?id=e0ea50919746813b844b6cbb4814718247e2c7b3
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

#ifndef __NR_mmap
#define __NR_mmap 222
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 211
#endif
#ifndef __NR_socket
#define __NR_socket 198
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
      if (current_time_ms() - start < 5000)
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
  res = syscall(__NR_socket, /*domain=*/0x10ul, /*type=*/3ul, /*proto=*/0xc);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c8 = 0;
  *(uint64_t*)0x200000d0 = 0x20000080;
  *(uint64_t*)0x20000080 = 0x20000100;
  *(uint32_t*)0x20000100 = 0x64;
  *(uint8_t*)0x20000104 = 2;
  *(uint8_t*)0x20000105 = 6;
  *(uint16_t*)0x20000106 = 0xb;
  *(uint32_t*)0x20000108 = 0xa;
  *(uint32_t*)0x2000010c = 0;
  *(uint8_t*)0x20000110 = 0;
  *(uint8_t*)0x20000111 = 0;
  *(uint16_t*)0x20000112 = htobe16(0);
  *(uint16_t*)0x20000114 = 0x10;
  *(uint16_t*)0x20000116 = 3;
  memcpy((void*)0x20000118, "bitmap:port\000", 12);
  *(uint16_t*)0x20000124 = 5;
  *(uint16_t*)0x20000126 = 4;
  *(uint8_t*)0x20000128 = 0;
  *(uint16_t*)0x2000012c = 9;
  *(uint16_t*)0x2000012e = 2;
  memcpy((void*)0x20000130, "syz0\000", 5);
  *(uint16_t*)0x20000138 = 5;
  *(uint16_t*)0x2000013a = 5;
  *(uint8_t*)0x2000013c = 0;
  *(uint16_t*)0x20000140 = 5;
  *(uint16_t*)0x20000142 = 1;
  *(uint8_t*)0x20000144 = 6;
  *(uint16_t*)0x20000148 = 0x1c;
  STORE_BY_BITMASK(uint16_t, , 0x2000014a, 7, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000014b, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000014b, 1, 7, 1);
  *(uint16_t*)0x2000014c = 8;
  STORE_BY_BITMASK(uint16_t, , 0x2000014e, 6, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000014f, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000014f, 0, 7, 1);
  *(uint32_t*)0x20000150 = htobe32(0);
  *(uint16_t*)0x20000154 = 6;
  STORE_BY_BITMASK(uint16_t, , 0x20000156, 4, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000157, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000157, 0, 7, 1);
  *(uint16_t*)0x20000158 = htobe16(0);
  *(uint16_t*)0x2000015c = 6;
  STORE_BY_BITMASK(uint16_t, , 0x2000015e, 5, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000015f, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000015f, 0, 7, 1);
  *(uint16_t*)0x20000160 = htobe16(0);
  *(uint64_t*)0x20000088 = 0x64;
  *(uint64_t*)0x200000d8 = 1;
  *(uint64_t*)0x200000e0 = 0;
  *(uint64_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000f0 = 0;
  syscall(__NR_sendmsg, /*fd=*/r[0], /*msg=*/0x200000c0ul, /*f=*/0ul);
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
  loop();
  return 0;
}
