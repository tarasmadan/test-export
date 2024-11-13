// https://syzkaller.appspot.com/bug?id=478fc753a448c1fab3f34b5147fbaab0a134f3c6
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

static long syz_open_dev(volatile long a0, volatile long a1, volatile long a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

static int inject_fault(int nth)
{
  int fd;
  fd = open("/proc/thread-self/fail-nth", O_RDWR);
  if (fd == -1)
    exit(1);
  char buf[16];
  sprintf(buf, "%d", nth);
  if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
    exit(1);
  return fd;
}

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  for (int i = 0; i < 100; i++) {
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

static void setup_fault()
{
  static struct {
    const char* file;
    const char* val;
    bool fatal;
  } files[] = {
      {"/sys/kernel/debug/failslab/ignore-gfp-wait", "N", true},
      {"/sys/kernel/debug/fail_futex/ignore-private", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/ignore-gfp-highmem", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/ignore-gfp-wait", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/min-order", "0", false},
  };
  unsigned i;
  for (i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
    if (!write_file(files[i].file, files[i].val)) {
      if (files[i].fatal)
        exit(1);
    }
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

static void execute_one(void)
{
  int i, call, thread;
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
      event_timedwait(&th->done, 50);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter = 0;
  for (;; iter++) {
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
      if (current_time_ms() - start < 5000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[5] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0, 0x0, 0x0};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    memcpy((void*)0x20000000, "/dev/dri/card#\000", 15);
    res = -1;
    res = syz_open_dev(/*dev=*/0x20000000, /*id=*/0, /*flags=*/0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    memcpy((void*)0x20000040, "/dev/dri/card#\000", 15);
    res = -1;
    res = syz_open_dev(/*dev=*/0x20000040, /*id=*/0xd21, /*flags=*/0);
    if (res != -1)
      r[1] = res;
    break;
  case 2:
    *(uint64_t*)0x200001c0 = 0;
    *(uint64_t*)0x200001c8 = 0x200000c0;
    *(uint64_t*)0x200001d0 = 0;
    *(uint64_t*)0x200001d8 = 0;
    *(uint32_t*)0x200001e0 = 0;
    *(uint32_t*)0x200001e4 = 1;
    *(uint32_t*)0x200001e8 = 0;
    *(uint32_t*)0x200001ec = 0;
    *(uint32_t*)0x200001f0 = 0;
    *(uint32_t*)0x200001f4 = 0;
    *(uint32_t*)0x200001f8 = 0;
    *(uint32_t*)0x200001fc = 0;
    res = syscall(__NR_ioctl, /*fd=*/r[1], /*cmd=*/0xc04064a0,
                  /*arg=*/0x200001c0ul);
    if (res != -1)
      r[2] = *(uint32_t*)0x200000c0;
    break;
  case 3:
    *(uint64_t*)0x200003c0 = 0;
    *(uint32_t*)0x200003c8 = 0;
    *(uint32_t*)0x200003cc = r[2];
    res = syscall(__NR_ioctl, /*fd=*/r[1], /*cmd=*/0xc06864a1,
                  /*arg=*/0x200003c0ul);
    if (res != -1)
      r[3] = *(uint32_t*)0x200003d0;
    break;
  case 4:
    *(uint32_t*)0x20000440 = r[3];
    *(uint32_t*)0x20000444 = 0;
    *(uint32_t*)0x20000448 = 0;
    *(uint32_t*)0x2000044c = 0;
    *(uint32_t*)0x20000450 = 0;
    *(uint32_t*)0x20000464 = 0;
    *(uint32_t*)0x20000468 = 0;
    *(uint32_t*)0x2000046c = 0;
    *(uint32_t*)0x20000470 = 0;
    *(uint32_t*)0x20000474 = 0;
    *(uint32_t*)0x20000478 = 0;
    *(uint32_t*)0x2000047c = 0;
    *(uint32_t*)0x20000480 = 0;
    *(uint64_t*)0x20000488 = 0;
    *(uint64_t*)0x20000490 = 0;
    *(uint64_t*)0x20000498 = 0;
    *(uint64_t*)0x200004a0 = 0;
    res = syscall(__NR_ioctl, /*fd=*/r[1], /*cmd=*/0xc06864ce,
                  /*arg=*/0x20000440ul);
    if (res != -1)
      r[4] = *(uint32_t*)0x20000454;
    break;
  case 5:
    *(uint32_t*)0x20000100 = 1;
    *(uint32_t*)0x20000104 = 0x101;
    *(uint32_t*)0x20000108 = 0x101;
    *(uint32_t*)0x2000010c = 0;
    inject_fault(11);
    syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0xc02064b2, /*arg=*/0x20000100ul);
    break;
  case 6:
    *(uint32_t*)0x20000080 = r[4];
    *(uint32_t*)0x20000084 = 0;
    syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0xc00c642d, /*arg=*/0x20000080ul);
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
  setup_fault();
  loop();
  return 0;
}