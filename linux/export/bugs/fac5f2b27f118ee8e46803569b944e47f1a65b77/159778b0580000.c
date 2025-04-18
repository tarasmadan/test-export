// https://syzkaller.appspot.com/bug?id=fac5f2b27f118ee8e46803569b944e47f1a65b77
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  *(uint32_t*)0x200009c0 = 0x1b;
  *(uint32_t*)0x200009c4 = 0;
  *(uint32_t*)0x200009c8 = 0;
  *(uint32_t*)0x200009cc = 0x40000;
  *(uint32_t*)0x200009d0 = 0;
  *(uint32_t*)0x200009d4 = 0;
  *(uint32_t*)0x200009d8 = 0;
  memset((void*)0x200009dc, 0, 16);
  *(uint32_t*)0x200009ec = 0;
  *(uint32_t*)0x200009f0 = 0;
  *(uint32_t*)0x200009f4 = 0;
  *(uint32_t*)0x200009f8 = 0;
  *(uint32_t*)0x200009fc = 0;
  *(uint64_t*)0x20000a00 = 0;
  *(uint32_t*)0x20000a08 = 0;
  *(uint32_t*)0x20000a0c = 0;
  res = syscall(__NR_bpf, /*cmd=*/0ul, /*arg=*/0x200009c0ul, /*size=*/0x48ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000240 = 0x11;
  *(uint32_t*)0x20000244 = 0x11;
  *(uint64_t*)0x20000248 = 0x20000500;
  *(uint8_t*)0x20000500 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000501, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000501, 0, 4, 4);
  *(uint16_t*)0x20000502 = 0;
  *(uint32_t*)0x20000504 = 1;
  *(uint8_t*)0x20000508 = 0;
  *(uint8_t*)0x20000509 = 0;
  *(uint16_t*)0x2000050a = 0;
  *(uint32_t*)0x2000050c = 0x1000;
  *(uint8_t*)0x20000510 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000511, 1, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000511, 1, 4, 4);
  *(uint16_t*)0x20000512 = 0;
  *(uint32_t*)0x20000514 = r[0];
  *(uint8_t*)0x20000518 = 0;
  *(uint8_t*)0x20000519 = 0;
  *(uint16_t*)0x2000051a = 0;
  *(uint32_t*)0x2000051c = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000520, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000520, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000520, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000521, 2, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000521, 0, 4, 4);
  *(uint16_t*)0x20000522 = 0;
  *(uint32_t*)0x20000524 = 0x14;
  STORE_BY_BITMASK(uint8_t, , 0x20000528, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000528, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000528, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000529, 3, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000529, 0, 4, 4);
  *(uint16_t*)0x2000052a = 0;
  *(uint32_t*)0x2000052c = 0;
  *(uint8_t*)0x20000530 = 0x85;
  *(uint8_t*)0x20000531 = 0;
  *(uint16_t*)0x20000532 = 0;
  *(uint32_t*)0x20000534 = 0x83;
  STORE_BY_BITMASK(uint8_t, , 0x20000538, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000538, 1, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000538, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000539, 9, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000539, 0, 4, 4);
  *(uint16_t*)0x2000053a = 0;
  *(uint32_t*)0x2000053c = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000540, 5, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000540, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000540, 5, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000541, 9, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000541, 0, 4, 4);
  *(uint16_t*)0x20000542 = 1;
  *(uint32_t*)0x20000544 = 0;
  *(uint8_t*)0x20000548 = 0x95;
  *(uint8_t*)0x20000549 = 0;
  *(uint16_t*)0x2000054a = 0;
  *(uint32_t*)0x2000054c = 0;
  *(uint8_t*)0x20000550 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000551, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000551, 0, 4, 4);
  *(uint16_t*)0x20000552 = 0;
  *(uint32_t*)0x20000554 = 8;
  *(uint8_t*)0x20000558 = 0;
  *(uint8_t*)0x20000559 = 0;
  *(uint16_t*)0x2000055a = 0;
  *(uint32_t*)0x2000055c = 0x401;
  STORE_BY_BITMASK(uint8_t, , 0x20000560, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000560, 1, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000560, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000561, 1, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000561, 9, 4, 4);
  *(uint16_t*)0x20000562 = 0;
  *(uint32_t*)0x20000564 = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000568, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000568, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000568, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000569, 2, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000569, 0, 4, 4);
  *(uint16_t*)0x2000056a = 0;
  *(uint32_t*)0x2000056c = 1;
  *(uint8_t*)0x20000570 = 0x85;
  *(uint8_t*)0x20000571 = 0;
  *(uint16_t*)0x20000572 = 0;
  *(uint32_t*)0x20000574 = 0x84;
  STORE_BY_BITMASK(uint8_t, , 0x20000578, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000578, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000578, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000579, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000579, 0, 4, 4);
  *(uint16_t*)0x2000057a = 0;
  *(uint32_t*)0x2000057c = 0;
  *(uint8_t*)0x20000580 = 0x95;
  *(uint8_t*)0x20000581 = 0;
  *(uint16_t*)0x20000582 = 0;
  *(uint32_t*)0x20000584 = 0;
  *(uint64_t*)0x20000250 = 0x20000000;
  memcpy((void*)0x20000000, "GPL\000", 4);
  *(uint32_t*)0x20000258 = 0;
  *(uint32_t*)0x2000025c = 0;
  *(uint64_t*)0x20000260 = 0;
  *(uint32_t*)0x20000268 = 0;
  *(uint32_t*)0x2000026c = 0;
  memset((void*)0x20000270, 0, 16);
  *(uint32_t*)0x20000280 = 0;
  *(uint32_t*)0x20000284 = 0;
  *(uint32_t*)0x20000288 = -1;
  *(uint32_t*)0x2000028c = 8;
  *(uint64_t*)0x20000290 = 0;
  *(uint32_t*)0x20000298 = 0;
  *(uint32_t*)0x2000029c = 0x10;
  *(uint64_t*)0x200002a0 = 0;
  *(uint32_t*)0x200002a8 = 0;
  *(uint32_t*)0x200002ac = 0;
  *(uint32_t*)0x200002b0 = 0;
  *(uint32_t*)0x200002b4 = 0;
  *(uint64_t*)0x200002b8 = 0;
  *(uint64_t*)0x200002c0 = 0;
  *(uint32_t*)0x200002c8 = 0x10;
  *(uint32_t*)0x200002cc = 0;
  *(uint32_t*)0x200002d0 = 0;
  res = syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x20000240ul, /*size=*/0x90ul);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x20000200 = 0x200004c0;
  memcpy((void*)0x200004c0, "contention_begin\000", 17);
  *(uint32_t*)0x20000208 = r[1];
  *(uint32_t*)0x2000020c = 0;
  *(uint64_t*)0x20000210 = 0;
  syscall(__NR_bpf, /*cmd=*/0x11ul, /*arg=*/0x20000200ul, /*size=*/0x10ul);
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
  for (procid = 0; procid < 4; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
