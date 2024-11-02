// https://syzkaller.appspot.com/bug?id=f3ddd7724674ca780476e946287afab776e7c590
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

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

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
  for (call = 0; call < 34; call++) {
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
      if (call == 15 || call == 33)
        break;
      event_timedwait(&th->done,
                      50 + (call == 7 ? 500 : 0) + (call == 8 ? 500 : 0) +
                          (call == 25 ? 500 : 0) + (call == 26 ? 500 : 0));
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

uint64_t r[1] = {0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    syscall(__NR_pipe, 0ul);
    break;
  case 1:
    syscall(__NR_timer_settime, 0, 0ul, 0ul, 0ul);
    break;
  case 2:
    syscall(__NR_setsockopt, -1, 6, 0xd, 0ul, 0ul);
    break;
  case 3:
    syscall(__NR_sendto, -1, 0ul, 0ul, 0x20008005ul, 0ul, 0ul);
    break;
  case 4:
    res = syscall(__NR_socket, 0x10ul, 0x80003ul, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 5:
    syscall(__NR_sendmsg, r[0], 0ul, 0x10ul);
    break;
  case 6:
    syscall(__NR_ioctl, r[0], 0x8982, 0ul);
    break;
  case 7:
    syscall(__NR_bpf, 5ul, 0ul, 0ul);
    break;
  case 8:
    syscall(__NR_bpf, 5ul, 0ul, 0ul);
    break;
  case 9:
    syscall(__NR_recvfrom, -1, 0ul, 0ul, 0ul, 0ul, 0ul);
    break;
  case 10:
    syscall(__NR_sendmsg, -1, 0ul, 0x24004800ul);
    break;
  case 11:
    *(uint32_t*)0x20000180 = 1;
    memcpy((void*)0x20000184,
           "vcan0\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
           "\000\000\000\000",
           24);
    *(uint32_t*)0x2000019c = 0;
    *(uint16_t*)0x200001b4 = 8;
    syscall(__NR_ioctl, -1, 0x8982, 0x20000180ul);
    break;
  case 12:
    syscall(__NR_vmsplice, -1, 0ul, 0ul, 0ul);
    break;
  case 13:
    syscall(__NR_bind, -1, 0ul, 0ul);
    break;
  case 14:
    memcpy((void*)0x20000000, "/dev/ttyS3\000", 11);
    syscall(__NR_openat, 0xffffff9cul, 0x20000000ul, 0x107640ul, 0ul);
    break;
  case 15:
    syscall(__NR_unshare, 0x6c060000ul);
    break;
  case 16:
    syscall(__NR_pipe, 0ul);
    break;
  case 17:
    syscall(__NR_timer_create, 9ul, 0ul, 0x20000540ul);
    break;
  case 18:
    *(uint64_t*)0x2006b000 = 0;
    *(uint64_t*)0x2006b008 = 8;
    *(uint64_t*)0x2006b010 = 0;
    *(uint64_t*)0x2006b018 = 9;
    syscall(__NR_timer_settime, 0, 0ul, 0x2006b000ul, 0ul);
    break;
  case 19:
    syscall(__NR_socket, 2ul, 1ul, 0);
    break;
  case 20:
    syscall(__NR_setsockopt, -1, 6, 0xd, 0ul, 0ul);
    break;
  case 21:
    syscall(__NR_sendto, -1, 0ul, 0ul, 0x20008005ul, 0ul, 0ul);
    break;
  case 22:
    syscall(__NR_socket, 0x10ul, 0x80003ul, 0);
    break;
  case 23:
    syscall(__NR_sendmsg, r[0], 0ul, 0x10ul);
    break;
  case 24:
    syscall(__NR_ioctl, r[0], 0x8982, 0ul);
    break;
  case 25:
    syscall(__NR_bpf, 5ul, 0ul, 0ul);
    break;
  case 26:
    syscall(__NR_bpf, 5ul, 0ul, 0ul);
    break;
  case 27:
    syscall(__NR_recvfrom, -1, 0ul, 0ul, 0ul, 0ul, 0ul);
    break;
  case 28:
    syscall(__NR_sendmsg, -1, 0ul, 0x24004800ul);
    break;
  case 29:
    syz_open_dev(0, 0, 0x80000);
    break;
  case 30:
    syscall(__NR_ioctl, -1, 0x8982, 0ul);
    break;
  case 31:
    syscall(__NR_vmsplice, -1, 0ul, 0ul, 0ul);
    break;
  case 32:
    syscall(__NR_bind, -1, 0ul, 0ul);
    break;
  case 33:
    syscall(__NR_openat, 0xffffff9cul, 0ul, 0x107640ul, 0ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}