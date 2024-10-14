// https://syzkaller.appspot.com/bug?id=aa3dcb8dc275eeec3958f09873547b76ae4434de
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
    res = syscall(__NR_socket, 0x2b, 1, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint16_t*)0x20000040 = 2;
    *(uint16_t*)0x20000042 = htobe16(0x4e23);
    *(uint32_t*)0x20000044 = htobe32(0x7f000001);
    *(uint8_t*)0x20000048 = 0;
    *(uint8_t*)0x20000049 = 0;
    *(uint8_t*)0x2000004a = 0;
    *(uint8_t*)0x2000004b = 0;
    *(uint8_t*)0x2000004c = 0;
    *(uint8_t*)0x2000004d = 0;
    *(uint8_t*)0x2000004e = 0;
    *(uint8_t*)0x2000004f = 0;
    syscall(__NR_connect, r[0], 0x20000040, 0x10);
    break;
  case 2:
    *(uint16_t*)0x20000000 = 2;
    *(uint16_t*)0x20000002 = htobe16(0x4e23);
    *(uint32_t*)0x20000004 = htobe32(0);
    *(uint8_t*)0x20000008 = 0;
    *(uint8_t*)0x20000009 = 0;
    *(uint8_t*)0x2000000a = 0;
    *(uint8_t*)0x2000000b = 0;
    *(uint8_t*)0x2000000c = 0;
    *(uint8_t*)0x2000000d = 0;
    *(uint8_t*)0x2000000e = 0;
    *(uint8_t*)0x2000000f = 0;
    syscall(__NR_bind, r[0], 0x20000000, 0x10);
    break;
  case 3:
    syscall(__NR_listen, r[0], 0);
    break;
  case 4:
    *(uint8_t*)0x20000080 = 1;
    *(uint8_t*)0x20000081 = 0x80;
    *(uint8_t*)0x20000082 = 0xc2;
    *(uint8_t*)0x20000083 = 0;
    *(uint8_t*)0x20000084 = 0;
    *(uint8_t*)0x20000085 = 0;
    *(uint8_t*)0x20000086 = 0xaa;
    *(uint8_t*)0x20000087 = 0xaa;
    *(uint8_t*)0x20000088 = 0xaa;
    *(uint8_t*)0x20000089 = 0xaa;
    *(uint8_t*)0x2000008a = 0;
    *(uint8_t*)0x2000008b = 0;
    *(uint16_t*)0x2000008c = htobe16(0x814f);
    break;
  }
}

void loop()
{
  execute(5);
  collide = 1;
  execute(5);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
