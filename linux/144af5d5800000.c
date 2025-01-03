// https://syzkaller.appspot.com/bug?id=558630d9f2a0eea2faa91162e00de23d0d21d449
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <linux/futex.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}
#include <stdint.h>
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

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
  }

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();

  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 128 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);

#define CLONE_NEWCGROUP 0x02000000

  if (unshare(CLONE_NEWNS)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(CLONE_NEWCGROUP)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
}

static int do_sandbox_none(int executor_pid, bool enable_tun)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid < 0)
    fail("sandbox fork failed");
  if (pid)
    return pid;

  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }

  loop();
  doexit(1);
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

long r[2];
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
    break;
  case 1:
    *(uint32_t*)0x20bab000 = 1;
    *(uint32_t*)0x20bab004 = 0x78;
    *(uint8_t*)0x20bab008 = 0;
    *(uint8_t*)0x20bab009 = 0;
    *(uint8_t*)0x20bab00a = 0;
    *(uint8_t*)0x20bab00b = 0;
    *(uint32_t*)0x20bab00c = 0;
    *(uint64_t*)0x20bab010 = 0;
    *(uint64_t*)0x20bab018 = 0;
    *(uint64_t*)0x20bab020 = 0;
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 0, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 1, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 2, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 3, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 4, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 5, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 6, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 7, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 8, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 1, 9, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 10, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 11, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 12, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 13, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 14, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 15, 2);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 17, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 18, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 19, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 20, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 21, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 22, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 23, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 24, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 25, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 26, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 27, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 28, 1);
    STORE_BY_BITMASK(uint64_t, 0x20bab028, 0, 29, 35);
    *(uint32_t*)0x20bab030 = 0;
    *(uint32_t*)0x20bab034 = 0;
    *(uint64_t*)0x20bab038 = 0;
    *(uint64_t*)0x20bab040 = 0;
    *(uint64_t*)0x20bab048 = 0;
    *(uint64_t*)0x20bab050 = 0;
    *(uint64_t*)0x20bab058 = 0;
    *(uint32_t*)0x20bab060 = 0;
    *(uint64_t*)0x20bab068 = 0;
    *(uint32_t*)0x20bab070 = 0;
    *(uint16_t*)0x20bab074 = 0;
    *(uint16_t*)0x20bab076 = 0;
    r[0] = syscall(__NR_perf_event_open, 0x20bab000, -1, 0, -1, 0);
    break;
  case 2:
    syscall(__NR_ioctl, r[0], 0x2400, 0xffffffff00000001);
    break;
  case 3:
    r[1] = syscall(__NR_getpid);
    break;
  case 4:
    *(uint32_t*)0x20b70f88 = 2;
    *(uint32_t*)0x20b70f8c = 0x78;
    *(uint8_t*)0x20b70f90 = 0xe3;
    *(uint8_t*)0x20b70f91 = 0;
    *(uint8_t*)0x20b70f92 = 0;
    *(uint8_t*)0x20b70f93 = 0;
    *(uint32_t*)0x20b70f94 = 0;
    *(uint64_t*)0x20b70f98 = 0;
    *(uint64_t*)0x20b70fa0 = 0;
    *(uint64_t*)0x20b70fa8 = 0;
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 0, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 1, 1, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 2, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 3, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 4, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 5, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 6, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0x2000000000, 7, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 8, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 9, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 10, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 11, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 12, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 13, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 14, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 15, 2);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 17, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 18, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 19, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 20, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 21, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 22, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 23, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 24, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 25, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 26, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 27, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 28, 1);
    STORE_BY_BITMASK(uint64_t, 0x20b70fb0, 0, 29, 35);
    *(uint32_t*)0x20b70fb8 = 0;
    *(uint32_t*)0x20b70fbc = 0;
    *(uint64_t*)0x20b70fc0 = 0x20000000;
    *(uint64_t*)0x20b70fc8 = 0;
    *(uint64_t*)0x20b70fd0 = 0;
    *(uint64_t*)0x20b70fd8 = 0;
    *(uint64_t*)0x20b70fe0 = 0;
    *(uint32_t*)0x20b70fe8 = 0;
    *(uint64_t*)0x20b70ff0 = 0;
    *(uint32_t*)0x20b70ff8 = 0;
    *(uint16_t*)0x20b70ffc = 0;
    *(uint16_t*)0x20b70ffe = 0;
    syscall(__NR_perf_event_open, 0x20b70f88, r[1], 0, -1, 0);
    break;
  }
}

void loop()
{
  memset(r, -1, sizeof(r));
  execute(5);
  collide = 1;
  execute(5);
}

int main()
{
  int pid = do_sandbox_none(0, false);
  int status = 0;
  while (waitpid(pid, &status, __WALL) != pid) {
  }
  return 0;
}
