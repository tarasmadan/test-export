// https://syzkaller.appspot.com/bug?id=742102e2bd4c388c2ca70fcfa88adc7a1beb186f
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
  for (call = 0; call < 10; call++) {
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

uint64_t r[7] = {
    0xffffffffffffffff, 0xffffffffffffffff, 0x0, 0x0, 0xffffffffffffffff,
    0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    res = syscall(__NR_socketpair, /*domain=*/1ul, /*type=SOCK_STREAM*/ 1ul,
                  /*proto=*/0, /*fds=*/0x20000000ul);
    if (res != -1) {
      r[0] = *(uint32_t*)0x20000000;
      r[1] = *(uint32_t*)0x20000004;
    }
    break;
  case 1:
    *(uint32_t*)0x200013c0 = 0xc;
    res = syscall(__NR_getsockopt, /*fd=*/r[1], /*level=*/1, /*optname=*/0x11,
                  /*optval=*/0x20000280ul, /*optlen=*/0x200013c0ul);
    if (res != -1) {
      r[2] = *(uint32_t*)0x20000280;
      r[3] = *(uint32_t*)0x20000284;
    }
    break;
  case 2:
    *(uint64_t*)0x20000100 = 0;
    *(uint32_t*)0x20000108 = 0;
    *(uint64_t*)0x20000110 = 0x200000c0;
    *(uint64_t*)0x200000c0 = 0x20000140;
    memset((void*)0x20000140, 56, 1);
    *(uint64_t*)0x200000c8 = 1;
    *(uint64_t*)0x20000118 = 1;
    *(uint64_t*)0x20000120 = 0x20000040;
    *(uint64_t*)0x20000040 = 0x14;
    *(uint32_t*)0x20000048 = 1;
    *(uint32_t*)0x2000004c = 1;
    *(uint32_t*)0x20000050 = r[0];
    *(uint64_t*)0x20000058 = 0x1c;
    *(uint32_t*)0x20000060 = 1;
    *(uint32_t*)0x20000064 = 2;
    *(uint32_t*)0x20000068 = r[2];
    *(uint32_t*)0x2000006c = r[3];
    *(uint32_t*)0x20000070 = 0;
    *(uint64_t*)0x20000128 = 0x38;
    *(uint32_t*)0x20000130 = 1;
    *(uint32_t*)0x20000138 = 0;
    syscall(__NR_sendmmsg, /*fd=*/r[1], /*mmsg=*/0x20000100ul, /*vlen=*/1ul,
            /*f=MSG_OOB|MSG_NOSIGNAL|MSG_DONTWAIT|MSG_CONFIRM|0x100*/ 0x4941ul);
    break;
  case 3:
    res = syscall(__NR_pipe, /*pipefd=*/0x20000080ul);
    if (res != -1)
      r[4] = *(uint32_t*)0x20000084;
    break;
  case 4:
    syscall(__NR_splice, /*fdin=*/r[0], /*offin=*/0ul, /*fdout=*/r[4],
            /*offout=*/0ul, /*len=*/0x39000ul, /*f=*/0ul);
    break;
  case 5:
    res = syscall(__NR_socket, /*domain=*/1ul, /*type=SOCK_SEQPACKET*/ 5ul,
                  /*proto=*/0);
    if (res != -1)
      r[5] = res;
    break;
  case 6:
    *(uint16_t*)0x20003000 = 1;
    *(uint8_t*)0x20003002 = 0;
    *(uint32_t*)0x20003004 = 0x4e21;
    syscall(__NR_bind, /*fd=*/r[5], /*addr=*/0x20003000ul, /*addrlen=*/0x6eul);
    break;
  case 7:
    syscall(__NR_listen, /*fd=*/r[5], /*backlog=*/0);
    break;
  case 8:
    res = syscall(__NR_socketpair, /*domain=*/1ul, /*type=SOCK_SEQPACKET*/ 5ul,
                  /*proto=*/0, /*fds=*/0x20000000ul);
    if (res != -1)
      r[6] = *(uint32_t*)0x20000004;
    break;
  case 9:
    *(uint64_t*)0x20000080 = 0;
    *(uint32_t*)0x20000088 = 0;
    *(uint64_t*)0x20000090 = 0;
    *(uint64_t*)0x20000098 = 0;
    *(uint64_t*)0x200000a0 = 0x200000c0;
    memcpy((void*)0x200000c0,
           "\x18\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00",
           16);
    *(uint64_t*)0x200000d0 = r[5];
    *(uint64_t*)0x200000a8 = 0x18;
    *(uint32_t*)0x200000b0 = 0;
    syscall(__NR_sendmsg, /*fd=*/r[6], /*msg=*/0x20000080ul, /*f=*/0ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  loop();
  return 0;
}