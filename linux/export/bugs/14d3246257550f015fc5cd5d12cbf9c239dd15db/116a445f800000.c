// https://syzkaller.appspot.com/bug?id=14d3246257550f015fc5cd5d12cbf9c239dd15db
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <linux/futex.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
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
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
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

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* uctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  doexit(sig);
}

static void install_segv_handler()
{
  struct sigaction sa;

  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  syscall(SYS_rt_sigaction, 0x20, &sa, NULL, 8);
  syscall(SYS_rt_sigaction, 0x21, &sa, NULL, 8);

  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_NODEFER | SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
}

#define NONFAILING(...)                                                        \
  {                                                                            \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    }                                                                          \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
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

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;
static int collide;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    while (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE))
      syscall(SYS_futex, &th->running, FUTEX_WAIT, 0, 0);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    __atomic_store_n(&th->running, 0, __ATOMIC_RELEASE);
    syscall(SYS_futex, &th->running, FUTEX_WAKE);
  }
  return 0;
}

static void execute(int num_calls)
{
  int call, thread;
  running = 0;
  for (call = 0; call < num_calls; call++) {
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]); thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 128 << 10);
        pthread_create(&th->th, &attr, thr, th);
      }
      if (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE)) {
        th->call = call;
        __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
        __atomic_store_n(&th->running, 1, __ATOMIC_RELEASE);
        syscall(SYS_futex, &th->running, FUTEX_WAKE);
        if (collide && call % 2)
          break;
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (running)
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0,
                 0xffffffffffffffff};
unsigned long long procid;
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 0x26, 5, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    NONFAILING(*(uint16_t*)0x2065ffa8 = 0x26);
    NONFAILING(
        memcpy((void*)0x2065ffaa,
               "\x68\x61\x73\x68\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 14));
    NONFAILING(*(uint32_t*)0x2065ffb8 = 0);
    NONFAILING(*(uint32_t*)0x2065ffbc = 0);
    NONFAILING(memcpy((void*)0x2065ffc0,
                      "\x76\x6d\x61\x63\x28\x61\x65\x73\x2d\x67\x65\x6e\x65\x72"
                      "\x69\x63\x29\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00",
                      64));
    syscall(__NR_bind, r[0], 0x2065ffa8, 0x58);
    break;
  case 2:
    NONFAILING(*(uint32_t*)0x20000080 = 0);
    res = syscall(__NR_accept, r[0], 0, 0x20000080);
    if (res != -1)
      r[1] = res;
    break;
  case 3:
    NONFAILING(*(uint32_t*)0x20000240 = 0);
    NONFAILING(*(uint32_t*)0x20000244 = 0x1000);
    NONFAILING(*(uint32_t*)0x20000280 = 8);
    res = syscall(__NR_getsockopt, -1, 0x84, 0x71, 0x20000240, 0x20000280);
    if (res != -1)
      NONFAILING(r[2] = *(uint32_t*)0x20000240);
    break;
  case 4:
    NONFAILING(*(uint32_t*)0x200002c0 = r[2]);
    NONFAILING(*(uint16_t*)0x200002c4 = 0xfff7);
    NONFAILING(*(uint16_t*)0x200002c6 = 0);
    syscall(__NR_setsockopt, r[1], 0x84, 0x17, 0x200002c0, 8);
    break;
  case 5:
    NONFAILING(memcpy(
        (void*)0x20000100,
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        16));
    syscall(__NR_setsockopt, r[0], 0x117, 1, 0x20000100, 0x10);
    break;
  case 6:
    res = syscall(__NR_accept, r[0], 0, 0);
    if (res != -1)
      r[3] = res;
    break;
  case 7:
    NONFAILING(*(uint32_t*)0x200004c0 = htobe32(0xe0000002));
    NONFAILING(*(uint8_t*)0x200004c4 = 0xac);
    NONFAILING(*(uint8_t*)0x200004c5 = 0x14);
    NONFAILING(*(uint8_t*)0x200004c6 = 0x14);
    NONFAILING(*(uint8_t*)0x200004c7 = 0x11);
    syscall(__NR_setsockopt, r[1], 0, 0x23, 0x200004c0, 8);
    break;
  case 8:
    NONFAILING(*(uint32_t*)0x200001c0 = r[2]);
    NONFAILING(*(uint32_t*)0x200001c4 = 2);
    NONFAILING(*(uint16_t*)0x200001c8 = 0x10);
    NONFAILING(*(uint32_t*)0x20000200 = 0xc);
    syscall(__NR_getsockopt, r[1], 0x84, 0x72, 0x200001c0, 0x20000200);
    break;
  case 9:
    NONFAILING(memcpy(
        (void*)0x20000380,
        "\x98\x4a\x7c\x4b\x76\x25\xb5\x21\xb2\x08\x32\xa2\x3c\xa4\x93\x3b\xce"
        "\x97\xbd\xe3\x1b\xf9\x45\x9e\x1f\xbc\xfa\x9f\x63\x0f\x51\x0c\xe5\xfa"
        "\xfc\xe5\xb3\xe4\xd2\x4a\xee\xe1\x28\x26\xb8\xd8\xce\x2c\x69\x66\x05"
        "\x5c\xf1\x84\xf7\xc4\xea\x79\xb7\x39\x95\xf5\xbc\x71\xca\x7a\x0f\xef"
        "\xd1\x95\x66\xdf\x07\x7e\xe7\x3d\xf3\x5c\x02\xb4\x5f\x7f\x0a\x6e\x15"
        "\x38\x7c\x11\xc6\x22\xa4\xa4\x05\x5d\x10\xd0\x5c\x36\x8e\xd4\xd7\x1d"
        "\x79\xbb\x1a\x56\xac\x0f\xed\x73\xc8\x1f\x9f\x12\x5e\x49\xcb\x76\x2d"
        "\x39\x23\x2c\xbe\xc7\x77\x1e\x4b\xb4\x1b\xb1\xaa\xe4\x32\xc9\xd8\xbd"
        "\x28\x06\x1e\x38\xae\xf1\xe3\x34\x34\xc2\x11\xd1\xe4\x5e\x4b\xbd\xaf"
        "\x4e\x8b\x59\xc0\xa8\x19\xf3\xcb\xd9\x68\x6f\x11\x66\x6f\x98\x98\xb5"
        "\xc6\xbe\x58\xba\x9e\x78\xd6\xf8\xab\xa6\x36\x23\x60\x3c\x8a\x6d\x80"
        "\x1b\x19\x4c\xa3\x9d\x97\xcd\x37\xac\xde\xee\xb2\x8c\xbb\x8d\x0b\x08"
        "\xa2\x95\x9e\xc3\x2e\x52\xf0\xf0\xda\xb6\x86\x45\xc0\x80\xc3\x46\x3b"
        "\xd7\x7c\x94\x9a\x4a\x46\x82\xb0\x38\xb6\xc9\x3a\xd6\x01\xf2\x29\x76"
        "\xde\x21\xf6\xfa\x8d\x31\x4e\x61\x92\xec\x3e",
        249));
    NONFAILING(*(uint16_t*)0x20000480 = 4);
    NONFAILING(*(uint16_t*)0x20000482 = htobe16(6));
    NONFAILING(*(uint32_t*)0x20000484 = htobe32(6));
    NONFAILING(memcpy((void*)0x20000488, "\x2b\x13\xd5\x31\xc8\xcd", 6));
    NONFAILING(*(uint8_t*)0x2000048e = -1);
    NONFAILING(*(uint8_t*)0x2000048f = 0);
    syscall(__NR_sendto, r[1], 0x20000380, 0xf9, 0x804, 0x20000480, 0x10);
    break;
  case 10:
    NONFAILING(*(uint64_t*)0x200000c0 = 0x20000000);
    NONFAILING(*(uint16_t*)0x20000000 = 0x18);
    NONFAILING(*(uint32_t*)0x20000002 = 2);
    NONFAILING(*(uint16_t*)0x20000006 = htobe16(0));
    NONFAILING(*(uint8_t*)0x2000000a = 0xac);
    NONFAILING(*(uint8_t*)0x2000000b = 0x14);
    NONFAILING(*(uint8_t*)0x2000000c = 0x14);
    NONFAILING(*(uint8_t*)0x2000000d = 0);
    NONFAILING(*(uint32_t*)0x200000c8 = 0x80);
    NONFAILING(*(uint64_t*)0x200000d0 = 0x20002680);
    NONFAILING(*(uint64_t*)0x20002680 = 0x20000140);
    NONFAILING(memcpy((void*)0x20000140,
                      "\x71\xa0\x3b\xe6\x3d\x7a\x01\x3c\xf2\xec\x76\xdd\x93\xe0"
                      "\x1b\x55\xc0\x56\xfc\xfd\x8c\x84\x69\x2e\x46\x67\xc1\x75"
                      "\x4f\x74\x03\xa2\x44\xaa\xf6\x69\x00\x1a\xc2\xd6\x1c\x2c"
                      "\x4c\xf2\xf7\xcf\x9c\x5a\xa9\x36\xcc\x81\x9e\x62\x23\x8f"
                      "\x5f\x88\x28\x0a\x0b\xb7\x90\x0f\x23\xaa\x50\x66\x14\xcc"
                      "\xfc\x5e\xad\xac\x2b\xbd\x01\xb0\xb1\x77\x90\x06\x99\x0a"
                      "\x1c\xa5\x51\x85\x57\xa9\x31\x67\xdc\x91\x67\x11\xce\x13"
                      "\x25\xc9\xd2\x24\x82\xfe\x81\x8c\xfc",
                      107));
    NONFAILING(*(uint64_t*)0x20002688 = 0x6b);
    NONFAILING(*(uint64_t*)0x200000d8 = 1);
    NONFAILING(*(uint64_t*)0x200000e0 = 0x200004c0);
    NONFAILING(*(uint64_t*)0x200000e8 = 0);
    NONFAILING(*(uint32_t*)0x200000f0 = 0);
    syscall(__NR_sendmsg, r[3], 0x200000c0, 0);
    break;
  }
}

void execute_one()
{
  execute(11);
  collide = 1;
  execute(11);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      install_segv_handler();
      for (;;) {
        loop();
      }
    }
  }
  sleep(1000000);
  return 0;
}