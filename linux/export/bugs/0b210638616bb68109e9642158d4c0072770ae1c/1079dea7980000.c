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

#ifndef __NR_bpf
#define __NR_bpf 321
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

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  res = syscall(__NR_socket, /*domain=AF_INET6*/ 0xaul, /*type=SOCK_RAW*/ 3ul,
                /*proto=*/4);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000280 = 0x20000000;
  *(uint16_t*)0x20000000 = 0xeb9f;
  *(uint8_t*)0x20000002 = 1;
  *(uint8_t*)0x20000003 = 0;
  *(uint32_t*)0x20000004 = 0x18;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0x1c;
  *(uint32_t*)0x20000010 = 0x1c;
  *(uint32_t*)0x20000014 = 2;
  *(uint32_t*)0x20000018 = 0;
  *(uint16_t*)0x2000001c = 0;
  *(uint8_t*)0x2000001e = 0;
  *(uint8_t*)0x2000001f = 1;
  *(uint32_t*)0x20000020 = 5;
  *(uint8_t*)0x20000024 = 0;
  *(uint8_t*)0x20000025 = 0;
  *(uint8_t*)0x20000026 = 0;
  *(uint8_t*)0x20000027 = 0;
  *(uint32_t*)0x20000028 = 0;
  *(uint16_t*)0x2000002c = 0;
  *(uint8_t*)0x2000002e = 0;
  STORE_BY_BITMASK(uint8_t, , 0x2000002f, 9, 0, 7);
  STORE_BY_BITMASK(uint8_t, , 0x2000002f, 0, 7, 1);
  *(uint32_t*)0x20000030 = 0;
  *(uint8_t*)0x20000034 = 0;
  *(uint8_t*)0x20000035 = 0;
  *(uint64_t*)0x20000288 = 0;
  *(uint32_t*)0x20000290 = 0x36;
  *(uint32_t*)0x20000294 = 0;
  *(uint32_t*)0x20000298 = 0;
  *(uint32_t*)0x2000029c = 0;
  *(uint32_t*)0x200002a0 = 0;
  *(uint32_t*)0x200002a4 = 0;
  syscall(__NR_bpf, /*cmd=*/0x12ul, /*arg=*/0x20000280ul, /*size=*/0x20ul);
  memcpy((void*)0x20000000, "bridge0\000\000\000\000\000\000\000\000\000", 16);
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x89a2, /*arg=*/0x20000000ul);
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
