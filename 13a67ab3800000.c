// https://syzkaller.appspot.com/bug?id=172a5a40a8625bda8f1fa7ec32137988b13c2ef4
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <linux/futex.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

static void execute_one();
extern unsigned long long procid;

void loop()
{
  while (1) {
    execute_one();
  }
}

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
unsigned long long procid;
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 2, 2, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    res = syscall(__NR_socket, 2, 0x802, 0);
    if (res != -1)
      r[1] = res;
    break;
  case 2:
    memcpy((void*)0x20000240,
           "\x67\x72\x65\x74\x61\x70\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           16);
    *(uint32_t*)0x20000250 = 0;
    syscall(__NR_ioctl, r[0], 0x8933, 0x20000240);
    break;
  case 3:
    *(uint32_t*)0x20000000 = 2;
    memcpy((void*)0x20000004,
           "\x62\x6f\x6e\x64\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           16);
    *(uint32_t*)0x20000014 = 3;
    syscall(__NR_setsockopt, r[1], 0, 0x48c, 0x20000000, 0x18);
    break;
  case 4:
    *(uint32_t*)0x20000440 = 2;
    memcpy((void*)0x20000444,
           "\x6c\x6f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           16);
    *(uint32_t*)0x20000454 = 4;
    syscall(__NR_setsockopt, r[1], 0, 0x48b, 0x20000440, 0x18);
    break;
  }
}

void execute_one()
{
  execute(5);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        loop();
      }
    }
  }
  sleep(1000000);
  return 0;
}
