// https://syzkaller.appspot.com/bug?id=6eba2df1505e4d033ffa4937d15a3ed2e9d59092
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  *(uint32_t*)0x20000040 = 2;
  *(uint32_t*)0x20000044 = 0x80;
  *(uint8_t*)0x20000048 = 0xee;
  *(uint8_t*)0x20000049 = 0;
  *(uint8_t*)0x2000004a = 0;
  *(uint8_t*)0x2000004b = 0;
  *(uint32_t*)0x2000004c = 0;
  *(uint64_t*)0x20000050 = 0;
  *(uint64_t*)0x20000058 = 0;
  *(uint64_t*)0x20000060 = 0;
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 5, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 29, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 30, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 31, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 32, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 33, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 34, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 35, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 36, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 37, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000068, 0, 38, 26);
  *(uint32_t*)0x20000070 = 0;
  *(uint32_t*)0x20000074 = 0;
  *(uint64_t*)0x20000078 = 0;
  *(uint64_t*)0x20000080 = 0;
  *(uint64_t*)0x20000088 = 0;
  *(uint64_t*)0x20000090 = 0;
  *(uint32_t*)0x20000098 = 0;
  *(uint32_t*)0x2000009c = 0;
  *(uint64_t*)0x200000a0 = 0;
  *(uint32_t*)0x200000a8 = 0;
  *(uint16_t*)0x200000ac = 0;
  *(uint16_t*)0x200000ae = 0;
  *(uint32_t*)0x200000b0 = 0;
  *(uint32_t*)0x200000b4 = 0;
  *(uint64_t*)0x200000b8 = 0;
  res = syscall(__NR_perf_event_open, /*attr=*/0x20000040ul, /*pid=*/0,
                /*cpu=*/0ul, /*group=*/-1, /*flags=*/0ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000480 = 2;
  *(uint32_t*)0x20000484 = 0x80;
  *(uint8_t*)0x20000488 = 0xb4;
  *(uint8_t*)0x20000489 = 1;
  *(uint8_t*)0x2000048a = 0;
  *(uint8_t*)0x2000048b = 0;
  *(uint32_t*)0x2000048c = 0;
  *(uint64_t*)0x20000490 = 0;
  *(uint64_t*)0x20000498 = 0;
  *(uint64_t*)0x200004a0 = 0;
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 5, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 7, 1);
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
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 29, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 30, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 31, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 32, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 33, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 34, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 35, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 36, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 37, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200004a8, 0, 38, 26);
  *(uint32_t*)0x200004b0 = 0;
  *(uint32_t*)0x200004b4 = 0;
  *(uint64_t*)0x200004b8 = 0;
  *(uint64_t*)0x200004c0 = 0;
  *(uint64_t*)0x200004c8 = 0;
  *(uint64_t*)0x200004d0 = 0;
  *(uint32_t*)0x200004d8 = 0;
  *(uint32_t*)0x200004dc = 0;
  *(uint64_t*)0x200004e0 = 0;
  *(uint32_t*)0x200004e8 = 0;
  *(uint16_t*)0x200004ec = 0;
  *(uint16_t*)0x200004ee = 0;
  *(uint32_t*)0x200004f0 = 0;
  *(uint32_t*)0x200004f4 = 0;
  *(uint64_t*)0x200004f8 = 0;
  syscall(__NR_perf_event_open, /*attr=*/0x20000480ul, /*pid=*/0, /*cpu=*/0ul,
          /*group=*/-1, /*flags=*/0ul);
  *(uint32_t*)0x200000c0 = 0x1b;
  *(uint32_t*)0x200000c4 = 0;
  *(uint32_t*)0x200000c8 = 0;
  *(uint32_t*)0x200000cc = 0x8000;
  *(uint32_t*)0x200000d0 = 0;
  *(uint32_t*)0x200000d4 = -1;
  *(uint32_t*)0x200000d8 = 0;
  memset((void*)0x200000dc, 0, 16);
  *(uint32_t*)0x200000ec = 0;
  *(uint32_t*)0x200000f0 = -1;
  *(uint32_t*)0x200000f4 = 0;
  *(uint32_t*)0x200000f8 = 0;
  *(uint32_t*)0x200000fc = 0;
  *(uint64_t*)0x20000100 = 0;
  res = syscall(__NR_bpf, /*cmd=*/0ul, /*arg=*/0x200000c0ul, /*size=*/0x48ul);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c4 = 0xc;
  *(uint64_t*)0x200000c8 = 0x20000240;
  memcpy((void*)0x20000240,
         "\x18\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x18"
         "\x12\x00\x00",
         20);
  *(uint32_t*)0x20000254 = r[1];
  memcpy((void*)0x20000258,
         "\x00\x00\x00\x00\x00\x00\x00\x00\xb7\x08\x00\x00\x00\x00\x00\x00\x7b"
         "\x8a\xf8\xff\x00\x00\x00\x00\xbf\xa2\x00\x00\x00\x00\x00\x00\x07\x02"
         "\x00\x00\xf8\xff\xff\xff\xb7\x03\x00\x00\x00\x00\x00\x00\xb7\x04\x00"
         "\x00\x02\x09\x00\x00\x85\x00\x00\x00\x43\x00\x00\x00\x95",
         65);
  *(uint64_t*)0x200000d0 = 0;
  *(uint32_t*)0x200000d8 = 0;
  *(uint32_t*)0x200000dc = 0;
  *(uint64_t*)0x200000e0 = 0;
  *(uint32_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000ec = 0;
  memset((void*)0x200000f0, 0, 16);
  *(uint32_t*)0x20000100 = 0;
  *(uint32_t*)0x20000104 = 0;
  *(uint32_t*)0x20000108 = -1;
  *(uint32_t*)0x2000010c = 0;
  *(uint64_t*)0x20000110 = 0;
  *(uint32_t*)0x20000118 = 0;
  *(uint32_t*)0x2000011c = 0;
  *(uint64_t*)0x20000120 = 0;
  *(uint32_t*)0x20000128 = 0;
  *(uint32_t*)0x2000012c = 0;
  *(uint32_t*)0x20000130 = 0;
  *(uint32_t*)0x20000134 = 0;
  *(uint64_t*)0x20000138 = 0;
  *(uint64_t*)0x20000140 = 0;
  *(uint32_t*)0x20000148 = 0;
  *(uint32_t*)0x2000014c = 0;
  syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x200000c0ul, /*size=*/0x90ul);
  *(uint32_t*)0x20000500 = 5;
  *(uint32_t*)0x20000504 = 0xc;
  *(uint64_t*)0x20000508 = 0x20000240;
  *(uint64_t*)0x20000510 = 0x20000080;
  memcpy((void*)0x20000080, "GPL\000", 4);
  *(uint32_t*)0x20000518 = 0;
  *(uint32_t*)0x2000051c = 0;
  *(uint64_t*)0x20000520 = 0;
  *(uint32_t*)0x20000528 = 0;
  *(uint32_t*)0x2000052c = 0;
  memset((void*)0x20000530, 0, 16);
  *(uint32_t*)0x20000540 = 0;
  *(uint32_t*)0x20000544 = 0;
  *(uint32_t*)0x20000548 = -1;
  *(uint32_t*)0x2000054c = 0;
  *(uint64_t*)0x20000550 = 0;
  *(uint32_t*)0x20000558 = 0;
  *(uint32_t*)0x2000055c = 0;
  *(uint64_t*)0x20000560 = 0;
  *(uint32_t*)0x20000568 = 0;
  *(uint32_t*)0x2000056c = 0;
  *(uint32_t*)0x20000570 = 0;
  *(uint32_t*)0x20000574 = 0;
  *(uint64_t*)0x20000578 = 0;
  *(uint64_t*)0x20000580 = 0;
  *(uint32_t*)0x20000588 = 0;
  *(uint32_t*)0x2000058c = 0;
  res = syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x20000500ul, /*size=*/0x90ul);
  if (res != -1)
    r[2] = res;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x40042408, /*prog=*/r[2]);
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
  for (procid = 0; procid < 5; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}