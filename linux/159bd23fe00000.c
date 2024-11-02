// https://syzkaller.appspot.com/bug?id=4d86d7ffb1fd63cc638602b45d9998323856981c
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
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

#include <linux/futex.h>

static unsigned long long procid;

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
  int i;
  for (i = 0; i < 100; i++) {
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

static void execute_one(void)
{
  int i, call, thread;
  int collide = 0;
again:
  for (call = 0; call < 7; call++) {
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
      if (collide && (call % 2) == 0)
        break;
      event_timedwait(&th->done, 45);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
  if (!collide) {
    collide = 1;
    goto again;
  }
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res;
  switch (call) {
  case 0:
    syscall(__NR_socket, 0xaul, 1ul, 0);
    break;
  case 1:
    syscall(__NR_mmap, 0x20000000ul, 0xb36000ul, 3ul, 0x8031ul, -1, 0ul);
    break;
  case 2:
    res = syscall(__NR_socket, 0x26ul, 5ul, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 3:
    *(uint16_t*)0x20000280 = 0x26;
    memcpy((void*)0x20000282, "hash\000\000\000\000\000\000\000\000\000\000",
           14);
    *(uint32_t*)0x20000290 = 0;
    *(uint32_t*)0x20000294 = 0;
    memcpy((void*)0x20000298, "cryptd(sha224-generic)"
                              "\000\000\000\000\000\000\000\000\000\000\000\000"
                              "\000\000\000\000\000\000\000\000\000\000\000\000"
                              "\000\000\000\000\000\000\000\000\000\000\000\000"
                              "\000\000\000\000\000\000",
           64);
    syscall(__NR_bind, r[0], 0x20000280ul, 0x58ul);
    break;
  case 4:
    res = syscall(__NR_accept4, r[0], 0ul, 0ul, 0ul);
    if (res != -1)
      r[1] = res;
    break;
  case 5:
    *(uint64_t*)0x200002c0 = 0x20000000;
    *(uint16_t*)0x20000000 = 0x10;
    *(uint16_t*)0x20000002 = 0;
    *(uint32_t*)0x20000004 = 0;
    *(uint32_t*)0x20000008 = 0;
    *(uint32_t*)0x200002c8 = 0xc;
    *(uint64_t*)0x200002d0 = 0x20000280;
    *(uint64_t*)0x20000280 = 0x20000100;
    memcpy((void*)0x20000100,
           "\xb1\xaa\x26\x11\x86\x95\x81\x81\x10\x50\x5f\x5f\x38\xa4\x08\x00"
           "\x00\x00\x0f\x6f\x87\x97\xe2\x34\x50\xba\xc0\x73\x1d\xba\xc2\xf1"
           "\x3f\x7e\x74\x54\xaf\x4f\x58\x53\xc9\x88\xbd\xd2\xdd\xb4\x01\x8c"
           "\x5a\x53\x65\x16\xe0\xa8\x56\xd1\x10\x23\x4b\xf6\x01\x00\x00\x00"
           "\x00\x00\x00\x45\xa5\x44\x8b\xe4\xc4\xf3\x7c\x8c\xcb\x37\xe4\x5f"
           "\x13\x6c\x97\x65\x34\xe1\x89\xfe\x31\x04\x7d\xe8\x03\xf7\x68\x9e"
           "\x00\x00\x00\x1f\x00\x00\x00\x00\x00\x00\x00\x00\x00\xc2\xe3\x8f"
           "\x0b\xdb\x5a\x01\x49\x4a\x34\x3b\xd2\x2e\x43\x7f\x61\xb9\x7f\xef"
           "\x0f\xe7\x59\xcf\xb3\xef\x52\xef\xcd\x30\xe2\xd3\xac\x1c\x92\x49"
           "\x8b\x69\xde\x06\xe1\x0c\xc8\x9b\xe1\xdb\x8a\xcd\xa4\x61\xe8\xce"
           "\x61\x02\x00\x8f\x32\x56\xf4\x08\xbb\x30\xb9\x05\x59\x09\x2a\x9d"
           "\x0a\xdf\x4a\x64\x59\x4b\x84\xc2\x5e\x10\xed\x28\x90\x7d\xa5\xab"
           "\xb1\x01",
           194);
    *(uint64_t*)0x20000288 = 0x20002154;
    *(uint64_t*)0x200002d8 = 1;
    *(uint64_t*)0x200002e0 = 0;
    *(uint64_t*)0x200002e8 = 0;
    *(uint32_t*)0x200002f0 = 0;
    syscall(__NR_sendmsg, r[1], 0x200002c0ul, 0ul);
    break;
  case 6:
    syscall(__NR_madvise, 0x20000000ul, 0x600003ul, 9ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0ul);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}