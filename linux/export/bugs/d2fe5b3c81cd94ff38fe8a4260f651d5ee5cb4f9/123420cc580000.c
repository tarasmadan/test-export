// https://syzkaller.appspot.com/bug?id=d2fe5b3c81cd94ff38fe8a4260f651d5ee5cb4f9
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <linux/futex.h>

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

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  int i = 0;
  for (; i < 100; i++) {
    if (pthread_create(&th, &attr, fn, arg) == 0) {
      pthread_attr_destroy(&attr);
      return;
    }
    if (errno == EAGAIN) {
      usleep(50);
      continue;
    }
    break;
  }
  exit(1);
}

typedef struct {
  int state;
} event_t;

static void event_init(event_t* ev)
{
  ev->state = 0;
}

static void event_reset(event_t* ev)
{
  ev->state = 0;
}

static void event_set(event_t* ev)
{
  if (ev->state)
    exit(1);
  __atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1000000);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, 0);
}

static int event_isset(event_t* ev)
{
  return __atomic_load_n(&ev->state, __ATOMIC_ACQUIRE);
}

static int event_timedwait(event_t* ev, uint64_t timeout)
{
  uint64_t start = current_time_ms();
  uint64_t now = start;
  for (;;) {
    uint64_t remain = timeout - (now - start);
    struct timespec ts;
    ts.tv_sec = remain / 1000;
    ts.tv_nsec = (remain % 1000) * 1000 * 1000;
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, &ts);
    if (__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
}

struct thread_t {
  int created, call;
  event_t ready, done;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    event_wait(&th->ready);
    event_reset(&th->ready);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    event_set(&th->done);
  }
  return 0;
}

static void loop(void)
{
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  int i, call, thread;
  for (call = 0; call < 6; call++) {
    for (thread = 0; thread < (int)(sizeof(threads) / sizeof(threads[0]));
         thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        event_init(&th->ready);
        event_init(&th->done);
        event_set(&th->done);
        thread_start(thr, th);
      }
      if (!event_isset(&th->done))
        continue;
      event_reset(&th->done);
      th->call = call;
      __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
      event_set(&th->ready);
      event_timedwait(&th->done, 50);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
}

uint64_t r[1] = {0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    memcpy((void*)0x200000000000,
           "/sys/devices/virtual/block/nbd13/queue/wbt_lat_usec\000", 52);
    syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul, /*file=*/0x200000000000ul,
            /*flags=O_RDWR*/ 2, /*mode=*/0);
    break;
  case 1:
    syscall(__NR_mmap, /*addr=*/0ul, /*len=*/0x400008ul, /*prot=*/0xdful,
            /*flags=*/0x9b72ul, /*fd=*/2, /*off=*/0x8000ul);
    break;
  case 2:
    res = syscall(__NR_socket, /*domain=AF_NETLINK*/ 0x10ul,
                  /*type=SOCK_DGRAM*/ 2ul, /*proto=*/0);
    if (res != -1)
      r[0] = res;
    break;
  case 3:
    *(uint64_t*)0x200000000240 = 0;
    *(uint32_t*)0x200000000248 = 0;
    *(uint64_t*)0x200000000250 = 0x200000000200;
    *(uint64_t*)0x200000000200 = 0x200000000300;
    memcpy((void*)0x200000000300, " \000\000\000", 4);
    memcpy((void*)0x200000000304, "\x12\x00", 2);
    memset((void*)0x200000000306, 93, 1);
    *(uint64_t*)0x200000000208 = 0x1ac;
    *(uint64_t*)0x200000000258 = 1;
    *(uint64_t*)0x200000000260 = 0;
    *(uint64_t*)0x200000000268 = 0;
    *(uint32_t*)0x200000000270 = 0;
    syscall(__NR_sendmsg, /*fd=*/r[0], /*msg=*/0x200000000240ul,
            /*f=MSG_BATCH*/ 0x40000ul);
    break;
  case 4:
    *(uint64_t*)0x200000000140 = 0;
    *(uint32_t*)0x200000000148 = 1;
    *(uint64_t*)0x200000000150 = 0x200000000080;
    *(uint64_t*)0x200000000080 = 0;
    *(uint64_t*)0x200000000088 = 0x400;
    *(uint64_t*)0x200000000158 = 5;
    *(uint64_t*)0x200000000160 = 0;
    *(uint64_t*)0x200000000168 = 0x200002;
    *(uint32_t*)0x200000000170 = 8;
    *(uint32_t*)0x200000000178 = 0x803;
    syscall(__NR_recvmmsg, /*fd=*/r[0], /*mmsg=*/0x200000000140ul,
            /*vlen=*/0xfffffff9, /*flags=*/0x10, /*timeout=*/0ul);
    break;
  case 5:
    syscall(__NR_write, /*fd=*/3, /*buf=*/0ul, /*count=*/0xfffffdeful);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffffffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul,
          /*fd=*/(intptr_t)-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200000000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul,
          /*fd=*/(intptr_t)-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200001000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul,
          /*fd=*/(intptr_t)-1, /*offset=*/0ul);
  const char* reason;
  (void)reason;
  loop();
  return 0;
}
