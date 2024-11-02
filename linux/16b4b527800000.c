// https://syzkaller.appspot.com/bug?id=96b7862f6d05608efbb9c0832b2fa6d404cb264a
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

uint64_t r[1] = {0xffffffffffffffff};
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    memcpy((void*)0x201b3000, "/dev/vhost-vsock", 17);
    res = syscall(__NR_openat, 0xffffffffffffff9c, 0x201b3000, 2, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint64_t*)0x20307000 = 0x20989fff;
    syscall(__NR_ioctl, r[0], 0xaf01, 0x20307000);
    break;
  case 2:
    *(uint32_t*)0x20336f58 = 0;
    *(uint32_t*)0x20336f5c = 0;
    syscall(__NR_ioctl, r[0], 0x4008af03, 0x20336f58);
    break;
  case 3:
    *(uint32_t*)0x20df5fd8 = 0;
    *(uint32_t*)0x20df5fdc = 0;
    *(uint64_t*)0x20df5fe0 = 0x203acf97;
    *(uint64_t*)0x20df5fe8 = 0x20bf6000;
    *(uint64_t*)0x20df5ff0 = 0x2017d000;
    *(uint64_t*)0x20df5ff8 = 0;
    syscall(__NR_ioctl, r[0], 0x4028af11, 0x20df5fd8);
    break;
  case 4:
    *(uint32_t*)0x20cef000 = 1;
    *(uint32_t*)0x20cef004 = 0;
    *(uint64_t*)0x20cef008 = 0x2062a000;
    *(uint64_t*)0x20cef010 = 0x20aac000;
    *(uint64_t*)0x20cef018 = 0x202fdf52;
    *(uint64_t*)0x20cef020 = 0;
    syscall(__NR_ioctl, r[0], 0x4028af11, 0x20cef000);
    break;
  case 5:
    *(uint32_t*)0x20f82ffc = 1;
    syscall(__NR_ioctl, r[0], 0x4004af61, 0x20f82ffc);
    break;
  case 6:
    *(uint32_t*)0x208bafd8 = 0;
    *(uint32_t*)0x208bafdc = 1;
    *(uint64_t*)0x208bafe0 = 0x2096ef4f;
    *(uint64_t*)0x208bafe8 = 0x207cb000;
    *(uint64_t*)0x208baff0 = 0x20ee4fb2;
    *(uint64_t*)0x208baff8 = 0xfffffffffffffffc;
    syscall(__NR_ioctl, r[0], 0x4028af11, 0x208bafd8);
    break;
  }
}

void execute_one()
{
  execute(7);
  collide = 1;
  execute(7);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}