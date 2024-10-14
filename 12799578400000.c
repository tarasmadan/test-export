// https://syzkaller.appspot.com/bug?id=2a622455acd7051c6cf85c360cd116118a587726
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

uint64_t current_time_ms()
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

#define SYZ_HAVE_SETUP_TEST 1
static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
}

#define SYZ_HAVE_RESET_TEST 1
static void reset_test()
{
  int fd;
  for (fd = 3; fd < 30; fd++)
    close(fd);
}

static void execute_one();

#define WAIT_FLAGS __WALL

static void loop()
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      execute_one();
      reset_test();
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
      kill(-pid, SIGKILL);
      kill(pid, SIGKILL);
      while (waitpid(-1, &status, WAIT_FLAGS) != pid) {
      }
      break;
    }
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one()
{
  long res = 0;
  memcpy((void*)0x200001c0, "./file0", 8);
  res = syscall(__NR_open, 0x200001c0, 0x101142, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_pipe2, 0x20000240, 0);
  if (res != -1)
    r[1] = *(uint32_t*)0x20000244;
  memcpy((void*)0x20000040, "./file0", 8);
  memcpy((void*)0x200000c0, "9p", 3);
  memcpy((void*)0x20000100, "trans=fd,", 9);
  memcpy((void*)0x20000109, "rfdno", 5);
  *(uint8_t*)0x2000010e = 0x3d;
  sprintf((char*)0x2000010f, "0x%016llx", (long long)r[0]);
  *(uint8_t*)0x20000121 = 0x2c;
  memcpy((void*)0x20000122, "wfdno", 5);
  *(uint8_t*)0x20000127 = 0x3d;
  sprintf((char*)0x20000128, "0x%016llx", (long long)r[1]);
  *(uint8_t*)0x2000013a = 0x2c;
  *(uint8_t*)0x2000013b = 0;
  syscall(__NR_mount, 0, 0x20000040, 0x200000c0, 0, 0x20000100);
}
int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
