// https://syzkaller.appspot.com/bug?id=8049241712bb636804bab6fcae751b36530464bc
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

unsigned long long procid;

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
  }
}

#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 370
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10, 3, 0xc);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000300 = 0;
  *(uint32_t*)0x20000304 = 0;
  *(uint32_t*)0x20000308 = 0x200002c0;
  *(uint32_t*)0x200002c0 = 0x20000440;
  *(uint32_t*)0x20000440 = 0x50;
  *(uint8_t*)0x20000444 = 2;
  *(uint8_t*)0x20000445 = 6;
  *(uint16_t*)0x20000446 = 1;
  *(uint32_t*)0x20000448 = 0;
  *(uint32_t*)0x2000044c = 0;
  *(uint8_t*)0x20000450 = 0;
  *(uint8_t*)0x20000451 = 0;
  *(uint16_t*)0x20000452 = htobe16(0);
  *(uint16_t*)0x20000454 = 9;
  *(uint16_t*)0x20000456 = 2;
  memcpy((void*)0x20000458, "syz2\000", 5);
  *(uint16_t*)0x20000460 = 0xc;
  *(uint16_t*)0x20000462 = 3;
  memcpy((void*)0x20000464, "hash:ip\000", 8);
  *(uint16_t*)0x2000046c = 0xc;
  STORE_BY_BITMASK(uint16_t, , 0x2000046e, 7, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000046f, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000046f, 1, 7, 1);
  *(uint16_t*)0x20000470 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x20000472, 6, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000473, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000473, 0, 7, 1);
  *(uint32_t*)0x20000474 = htobe32(0);
  *(uint16_t*)0x20000478 = 5;
  *(uint16_t*)0x2000047a = 1;
  *(uint8_t*)0x2000047c = 7;
  *(uint16_t*)0x20000480 = 5;
  *(uint16_t*)0x20000482 = 4;
  *(uint8_t*)0x20000484 = 0;
  *(uint16_t*)0x20000488 = 5;
  *(uint16_t*)0x2000048a = 5;
  *(uint8_t*)0x2000048c = 2;
  *(uint32_t*)0x200002c4 = 0x50;
  *(uint32_t*)0x2000030c = 1;
  *(uint32_t*)0x20000310 = 0;
  *(uint32_t*)0x20000314 = 0;
  *(uint32_t*)0x20000318 = 0;
  syscall(__NR_sendmsg, (intptr_t)r[0], 0x20000300, 0);
  res = syscall(__NR_socket, 0x10, 3, 0xc);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20000d00 = 0;
  *(uint32_t*)0x20000d04 = 0;
  *(uint32_t*)0x20000d08 = 0x20000cc0;
  *(uint32_t*)0x20000cc0 = 0x20000180;
  *(uint32_t*)0x20000180 = 0x44;
  *(uint8_t*)0x20000184 = 9;
  *(uint8_t*)0x20000185 = 6;
  *(uint16_t*)0x20000186 = 0x801;
  *(uint32_t*)0x20000188 = 0;
  *(uint32_t*)0x2000018c = 0;
  *(uint8_t*)0x20000190 = 0;
  *(uint8_t*)0x20000191 = 0;
  *(uint16_t*)0x20000192 = htobe16(0);
  *(uint16_t*)0x20000194 = 5;
  *(uint16_t*)0x20000196 = 1;
  *(uint8_t*)0x20000198 = 7;
  *(uint16_t*)0x2000019c = 0x1c;
  STORE_BY_BITMASK(uint16_t, , 0x2000019e, 7, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000019f, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000019f, 1, 7, 1);
  *(uint16_t*)0x200001a0 = 0xc;
  STORE_BY_BITMASK(uint16_t, , 0x200001a2, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200001a3, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200001a3, 1, 7, 1);
  *(uint16_t*)0x200001a4 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x200001a6, 1, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200001a7, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200001a7, 0, 7, 1);
  *(uint32_t*)0x200001a8 = htobe32(0);
  *(uint16_t*)0x200001ac = 0xc;
  STORE_BY_BITMASK(uint16_t, , 0x200001ae, 1, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200001af, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200001af, 1, 7, 1);
  *(uint16_t*)0x200001b0 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x200001b2, 1, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200001b3, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200001b3, 0, 7, 1);
  *(uint32_t*)0x200001b4 = htobe32(-1);
  *(uint16_t*)0x200001b8 = 9;
  *(uint16_t*)0x200001ba = 2;
  memcpy((void*)0x200001bc, "syz2\000", 5);
  *(uint32_t*)0x20000cc4 = 0x44;
  *(uint32_t*)0x20000d0c = 1;
  *(uint32_t*)0x20000d10 = 0;
  *(uint32_t*)0x20000d14 = 0;
  *(uint32_t*)0x20000d18 = 0;
  syscall(__NR_sendmsg, (intptr_t)r[1], 0x20000d00, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
