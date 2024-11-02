// https://syzkaller.appspot.com/bug?id=25e00dd59f31783f233185cb60064b0ab645310f
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

static void fail(const char* msg, ...)
{
  int e = errno;
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit((e == ENOMEM || e == EAGAIN) ? kRetryStatus : kFailStatus);
}

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void execute_one();
extern unsigned long long procid;

static void loop()
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      fail("clone failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
      execute_one();
      doexit(0);
    }

    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      int res = waitpid(-1, &status, __WALL | WNOHANG);
      if (res == pid) {
        break;
      }
      usleep(1000);
      if (current_time_ms() - start < 3 * 1000)
        continue;
      kill(-pid, SIGKILL);
      kill(pid, SIGKILL);
      while (waitpid(-1, &status, __WALL) != pid) {
      }
      break;
    }
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffff};
void execute_one()
{
  long res = 0;
  memcpy((void*)0x20000240, "/dev/infiniband/rdma_cm", 24);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000240, 2, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000100 = 0;
  *(uint16_t*)0x20000104 = 0x18;
  *(uint16_t*)0x20000106 = 0xfa00;
  *(uint64_t*)0x20000108 = 0;
  *(uint64_t*)0x20000110 = 0x200000c0;
  *(uint16_t*)0x20000118 = 2;
  *(uint8_t*)0x2000011a = 0;
  *(uint8_t*)0x2000011b = 0;
  *(uint8_t*)0x2000011c = 0;
  *(uint8_t*)0x2000011d = 0;
  *(uint8_t*)0x2000011e = 0;
  *(uint8_t*)0x2000011f = 0;
  res = syscall(__NR_write, r[0], 0x20000100, 0x20);
  if (res != -1)
    r[1] = *(uint32_t*)0x200000c0;
  *(uint32_t*)0x20000180 = 3;
  *(uint16_t*)0x20000184 = 0x40;
  *(uint16_t*)0x20000186 = 0xfa00;
  *(uint16_t*)0x20000188 = 0xa;
  *(uint16_t*)0x2000018a = htobe16(0);
  *(uint32_t*)0x2000018c = 0;
  *(uint8_t*)0x20000190 = 0;
  *(uint8_t*)0x20000191 = 0;
  *(uint8_t*)0x20000192 = 0;
  *(uint8_t*)0x20000193 = 0;
  *(uint8_t*)0x20000194 = 0;
  *(uint8_t*)0x20000195 = 0;
  *(uint8_t*)0x20000196 = 0;
  *(uint8_t*)0x20000197 = 0;
  *(uint8_t*)0x20000198 = 0;
  *(uint8_t*)0x20000199 = 0;
  *(uint8_t*)0x2000019a = 0;
  *(uint8_t*)0x2000019b = 0;
  *(uint8_t*)0x2000019c = 0;
  *(uint8_t*)0x2000019d = 0;
  *(uint8_t*)0x2000019e = 0;
  *(uint8_t*)0x2000019f = 0;
  *(uint32_t*)0x200001a0 = 0;
  *(uint16_t*)0x200001a4 = 0xa;
  *(uint16_t*)0x200001a6 = htobe16(0);
  *(uint32_t*)0x200001a8 = 0;
  *(uint8_t*)0x200001ac = 0xfe;
  *(uint8_t*)0x200001ad = 0x80;
  *(uint8_t*)0x200001ae = 0;
  *(uint8_t*)0x200001af = 0;
  *(uint8_t*)0x200001b0 = 0;
  *(uint8_t*)0x200001b1 = 0;
  *(uint8_t*)0x200001b2 = 0;
  *(uint8_t*)0x200001b3 = 0;
  *(uint8_t*)0x200001b4 = 0;
  *(uint8_t*)0x200001b5 = 0;
  *(uint8_t*)0x200001b6 = 0;
  *(uint8_t*)0x200001b7 = 0;
  *(uint8_t*)0x200001b8 = 0;
  *(uint8_t*)0x200001b9 = 0;
  *(uint8_t*)0x200001ba = 0;
  *(uint8_t*)0x200001bb = 0xbb;
  *(uint32_t*)0x200001bc = 0;
  *(uint32_t*)0x200001c0 = r[1];
  *(uint32_t*)0x200001c4 = 0;
  syscall(__NR_write, r[0], 0x20000180, 0x48);
  *(uint32_t*)0x20000000 = 7;
  *(uint16_t*)0x20000004 = 8;
  *(uint16_t*)0x20000006 = 0xfa00;
  *(uint32_t*)0x20000008 = r[1];
  *(uint32_t*)0x2000000c = 0;
  syscall(__NR_write, r[0], 0x20000000, 0x10);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}