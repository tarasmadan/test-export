// https://syzkaller.appspot.com/bug?id=a1d68f25c702940d846a5a7f22fc756980fdaffd
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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

static long syz_open_procfs(volatile long a0, volatile long a1)
{
  char buf[128];
  memset(buf, 0, sizeof(buf));
  if (a0 == 0) {
    snprintf(buf, sizeof(buf), "/proc/self/%s", (char*)a1);
  } else if (a0 == -1) {
    snprintf(buf, sizeof(buf), "/proc/thread-self/%s", (char*)a1);
  } else {
    snprintf(buf, sizeof(buf), "/proc/self/task/%d/%s", (int)a0, (char*)a1);
  }
  int fd = open(buf, O_RDWR);
  if (fd == -1)
    fd = open(buf, O_RDONLY);
  return fd;
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
  int i, call, thread;
  for (call = 0; call < 5; call++) {
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    res = syscall(__NR_pipe2, /*pipefd=*/0x200001c0ul, /*flags=*/0ul);
    if (res != -1)
      r[0] = *(uint32_t*)0x200001c4;
    break;
  case 1:
    res = syscall(__NR_pipe2, /*pipefd=*/0x20000000ul, /*flags=*/0x880ul);
    if (res != -1)
      r[1] = *(uint32_t*)0x20000004;
    break;
  case 2:
    memcpy((void*)0x20000100, "fd/4\000", 5);
    res = -1;
    res = syz_open_procfs(/*pid=*/-1, /*file=*/0x20000100);
    if (res != -1)
      r[2] = res;
    break;
  case 3:
    syscall(__NR_splice, /*fdin=*/r[2], /*offin=*/0ul, /*fdout=*/r[1],
            /*offout=*/0ul, /*len=*/0x100ul, /*f=*/0ul);
    break;
  case 4:
    *(uint32_t*)0x20000040 = 0x5d;
    *(uint8_t*)0x20000044 = 0x7d;
    *(uint16_t*)0x20000045 = 1;
    *(uint16_t*)0x20000047 = 0;
    *(uint16_t*)0x20000049 = 0x43;
    *(uint16_t*)0x2000004b = 0xfe00;
    *(uint32_t*)0x2000004d = 0;
    *(uint8_t*)0x20000051 = 8;
    *(uint32_t*)0x20000052 = 4;
    *(uint64_t*)0x20000056 = 2;
    *(uint32_t*)0x2000005e = 0x23893793;
    *(uint32_t*)0x20000062 = 9;
    *(uint32_t*)0x20000066 = 8;
    *(uint64_t*)0x2000006a = 0x8000;
    *(uint16_t*)0x20000072 = 3;
    memcpy((void*)0x20000074, "*{\312", 3);
    *(uint16_t*)0x20000077 = 5;
    memcpy((void*)0x20000079, "fd/4\000", 5);
    *(uint16_t*)0x2000007e = 5;
    memcpy((void*)0x20000080, "fd/4\000", 5);
    *(uint16_t*)0x20000085 = 3;
    memcpy((void*)0x20000087, "/\'.", 3);
    *(uint16_t*)0x2000008a = 5;
    memcpy((void*)0x2000008c, "fd/4\000", 5);
    *(uint32_t*)0x20000091 = -1;
    *(uint32_t*)0x20000095 = 0;
    *(uint32_t*)0x20000099 = 0;
    syscall(__NR_write, /*fd=*/r[0], /*data=*/0x20000040ul, /*size=*/0x5dul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul, /*prot=*/7ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  loop();
  return 0;
}
