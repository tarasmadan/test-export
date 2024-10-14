// https://syzkaller.appspot.com/bug?id=028612fd4d55db307a6f942bdb4dd731e9a8845f
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  *(uint32_t*)0x200009c0 = 0x12;
  *(uint32_t*)0x200009c4 = 6;
  *(uint32_t*)0x200009c8 = 8;
  *(uint32_t*)0x200009cc = 5;
  *(uint32_t*)0x200009d0 = 0;
  *(uint32_t*)0x200009d4 = -1;
  *(uint32_t*)0x200009d8 = 0;
  memset((void*)0x200009dc, 0, 16);
  *(uint32_t*)0x200009ec = 0;
  *(uint32_t*)0x200009f0 = -1;
  *(uint32_t*)0x200009f4 = 0;
  *(uint32_t*)0x200009f8 = 0;
  *(uint32_t*)0x200009fc = 0;
  *(uint64_t*)0x20000a00 = 0;
  res = syscall(__NR_bpf, /*cmd=*/0ul, /*arg=*/0x200009c0ul, /*size=*/0x48ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x200000c0 = 0x11;
  *(uint32_t*)0x200000c4 = 0xd;
  *(uint64_t*)0x200000c8 = 0x20000000;
  *(uint8_t*)0x20000000 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000001, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000001, 0, 4, 4);
  *(uint16_t*)0x20000002 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint8_t*)0x20000008 = 0;
  *(uint8_t*)0x20000009 = 0;
  *(uint16_t*)0x2000000a = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint8_t*)0x20000010 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000011, 1, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000011, 1, 4, 4);
  *(uint16_t*)0x20000012 = 0;
  *(uint32_t*)0x20000014 = r[0];
  *(uint8_t*)0x20000018 = 0;
  *(uint8_t*)0x20000019 = 0;
  *(uint16_t*)0x2000001a = 0;
  *(uint32_t*)0x2000001c = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000020, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000020, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000020, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000021, 8, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000021, 0, 4, 4);
  *(uint16_t*)0x20000022 = 0;
  *(uint32_t*)0x20000024 = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000028, 3, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000028, 3, 3, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000028, 3, 5, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000029, 0xa, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000029, 8, 4, 4);
  *(uint16_t*)0x2000002a = 0xfff8;
  *(uint32_t*)0x2000002c = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000030, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000030, 1, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000030, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000031, 2, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000031, 0xa, 4, 4);
  *(uint16_t*)0x20000032 = 0;
  *(uint32_t*)0x20000034 = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000038, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000038, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000038, 0, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000039, 2, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000039, 0, 4, 4);
  *(uint16_t*)0x2000003a = 0;
  *(uint32_t*)0x2000003c = 0xfffffff8;
  STORE_BY_BITMASK(uint8_t, , 0x20000040, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000040, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000040, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000041, 3, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000041, 0, 4, 4);
  *(uint16_t*)0x20000042 = 0;
  *(uint32_t*)0x20000044 = 8;
  STORE_BY_BITMASK(uint8_t, , 0x20000048, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000048, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000048, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000049, 4, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000049, 0, 4, 4);
  *(uint16_t*)0x2000004a = 0;
  *(uint32_t*)0x2000004c = 0;
  *(uint8_t*)0x20000050 = 0x85;
  *(uint8_t*)0x20000051 = 0;
  *(uint16_t*)0x20000052 = 0;
  *(uint32_t*)0x20000054 = 3;
  *(uint8_t*)0x20000058 = 0x85;
  *(uint8_t*)0x20000059 = 0;
  *(uint16_t*)0x2000005a = 0;
  *(uint32_t*)0x2000005c = 0x7d;
  *(uint8_t*)0x20000060 = 0x95;
  *(uint8_t*)0x20000061 = 0;
  *(uint16_t*)0x20000062 = 0;
  *(uint32_t*)0x20000064 = 0;
  *(uint64_t*)0x200000d0 = 0x20000240;
  memcpy((void*)0x20000240, "GPL\000", 4);
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
  res = syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x200000c0ul, /*size=*/0x90ul);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x200007c0 = 0x20000400;
  memcpy((void*)0x20000400, "hrtimer_start\000", 14);
  *(uint32_t*)0x200007c8 = r[1];
  syscall(__NR_bpf, /*cmd=*/0x11ul, /*arg=*/0x200007c0ul, /*size=*/0x10ul);
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
