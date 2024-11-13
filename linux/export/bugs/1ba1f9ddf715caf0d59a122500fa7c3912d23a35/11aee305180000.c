// https://syzkaller.appspot.com/bug?id=1ba1f9ddf715caf0d59a122500fa7c3912d23a35
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

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    *(uint32_t*)0x200009c0 = 0x19;
    *(uint32_t*)0x200009c4 = 4;
    *(uint32_t*)0x200009c8 = 8;
    *(uint32_t*)0x200009cc = 8;
    *(uint32_t*)0x200009d0 = 0;
    *(uint32_t*)0x200009d4 = -1;
    *(uint32_t*)0x200009d8 = 0;
    memset((void*)0x200009dc, 0, 16);
    *(uint32_t*)0x200009ec = 0;
    *(uint32_t*)0x200009f0 = -1;
    *(uint32_t*)0x200009f4 = 0;
    *(uint32_t*)0x200009f8 = 0;
    *(uint32_t*)0x200009fc = 0;
    *(uint64_t*)0x20000a00 = 0;
    res = syscall(__NR_bpf, /*cmd=*/0ul, /*arg=*/0x200009c0ul, /*size=*/0x48ul);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    res = syscall(__NR_socketpair, /*domain=*/1ul, /*type=SOCK_STREAM*/ 1ul,
                  /*proto=*/0, /*fds=*/0x20000040ul);
    if (res != -1) {
      r[1] = *(uint32_t*)0x20000040;
      r[2] = *(uint32_t*)0x20000044;
    }
    break;
  case 2:
    *(uint64_t*)0x20000300 = 0;
    *(uint32_t*)0x20000308 = 0;
    *(uint64_t*)0x20000310 = 0;
    *(uint64_t*)0x20000318 = 0;
    *(uint64_t*)0x20000320 = 0;
    *(uint64_t*)0x20000328 = 0;
    *(uint32_t*)0x20000330 = 0;
    syscall(__NR_recvmsg, /*fd=*/r[2], /*msg=*/0x20000300ul, /*f=*/0ul, 0);
    break;
  case 3:
    *(uint32_t*)0x200009c0 = 0x1b;
    *(uint32_t*)0x200009c4 = 0;
    *(uint32_t*)0x200009c8 = 0;
    *(uint32_t*)0x200009cc = 0x40000;
    *(uint32_t*)0x200009d0 = 0;
    *(uint32_t*)0x200009d4 = 0;
    *(uint32_t*)0x200009d8 = 0;
    memset((void*)0x200009dc, 0, 16);
    *(uint32_t*)0x200009ec = 0;
    *(uint32_t*)0x200009f0 = 0;
    *(uint32_t*)0x200009f4 = 0;
    *(uint32_t*)0x200009f8 = 0;
    *(uint32_t*)0x200009fc = 0;
    *(uint64_t*)0x20000a00 = 0;
    res = syscall(__NR_bpf, /*cmd=*/0ul, /*arg=*/0x200009c0ul, /*size=*/0x48ul);
    if (res != -1)
      r[3] = res;
    break;
  case 4:
    *(uint64_t*)0x200007c0 = 0;
    *(uint32_t*)0x200007c8 = 0;
    *(uint64_t*)0x200007d0 = 0x20000700;
    *(uint64_t*)0x20000700 = 0x200003c0;
    memcpy((void*)0x200003c0,
           "\xaf\x41\x58\x18\xf0\xf6\xc5\x80\xe6\xe9\x76\x09\x97\x88\x3e\x48"
           "\xb6\xc8\x0f\x20\xaa\xbb\xe4\x88\xa2\xb1\x86\xd9\xd8\xc8\xcc\xf9"
           "\x3e\x60\xcd\xeb\x9f\xdf\x99\xeb\x99\xa2\x57\x7e\x97\x4a\x48\xea"
           "\xfe\x67\xfb\x97\xca\xcd\xc1\x67\xf0\xd3\x6f\x12\xf0\xbd\xf1\xf3"
           "\xcb\x05\x6d\x86\xdd\x86\x8b\x68\xfc\x71\x99\x0e\x0b\x19\xdf\x6e"
           "\xaf\x66\x40\x15\xa1\x75\x7e\xa6\x27\x1e\xdc\xa7\xa1\x48\xac\xba"
           "\xa4\xbb\x6e\x79\x6c\xc3\x60\x48\xfc\x50\x82\x09\x18\xcd\xca\x79"
           "\x0a\xbd\x53\x9e\xd2\x4a\x4c\x07\x41\x1e\x4e\x0e\xec\x5e\x4a\x00"
           "\x9a\x3e\x95\x21\xcf\x6e\x27\x52\xf8\xd5\x79\x80\x99\xcc\x4a\x02"
           "\xb9\xc1\x90\x9c\xdd\x31\xe7\xc4\xed\x74\xc1\x84\x1b\xab\x85\x05"
           "\xa9\xbd\x13\x34\xc4\xac\x6e\x69\xdc\x44\x67\x06\x33\xac\x25\xe0"
           "\xdf\x11\xf8\xa1\xf0\x2b\xc0\x0a\xab\xca\x5a\x94\xb2\xd0\xe4\xa6"
           "\x21\x46\x6c\x98\xeb\xde\x84\x71\x5c\x65\x5e\x3e\xc0\xbd\x68\x9a"
           "\xd3\xbb\xd1\x71\x55\x23\xea\x44\x01\x7b\x40\x31\xf3\x95",
           222);
    *(uint64_t*)0x20000708 = 0xde;
    *(uint64_t*)0x20000710 = 0;
    *(uint64_t*)0x20000718 = 0;
    *(uint64_t*)0x20000720 = 0;
    *(uint64_t*)0x20000728 = 0;
    *(uint64_t*)0x20000730 = 0;
    *(uint64_t*)0x20000738 = 0;
    *(uint64_t*)0x20000740 = 0;
    *(uint64_t*)0x20000748 = 0;
    *(uint64_t*)0x20000750 = 0;
    *(uint64_t*)0x20000758 = 0;
    *(uint64_t*)0x20000760 = 0;
    *(uint64_t*)0x20000768 = 0;
    *(uint64_t*)0x200007d8 = 7;
    *(uint64_t*)0x200007e0 = 0x20000780;
    *(uint64_t*)0x20000780 = 0x2c;
    *(uint32_t*)0x20000788 = 1;
    *(uint32_t*)0x2000078c = 1;
    *(uint32_t*)0x20000790 = r[3];
    *(uint32_t*)0x20000794 = r[0];
    *(uint32_t*)0x20000798 = r[1];
    *(uint32_t*)0x2000079c = r[3];
    *(uint32_t*)0x200007a0 = r[3];
    *(uint32_t*)0x200007a4 = r[2];
    *(uint32_t*)0x200007a8 = r[3];
    *(uint64_t*)0x200007e8 = 0x30;
    *(uint32_t*)0x200007f0 = 0x8000;
    syscall(__NR_sendmsg, /*fd=*/r[2], /*msg=*/0x200007c0ul,
            /*f=MSG_PROBE|MSG_OOB|MSG_MORE|MSG_DONTWAIT*/ 0x8051ul);
    break;
  case 5:
    *(uint64_t*)0x200001c0 = 0;
    *(uint32_t*)0x200001c8 = 0;
    *(uint64_t*)0x200001d0 = 0x20000000;
    *(uint64_t*)0x20000000 = 0x20000080;
    memset((void*)0x20000080, 28, 1);
    *(uint64_t*)0x20000008 = 1;
    *(uint64_t*)0x200001d8 = 1;
    *(uint64_t*)0x200001e0 = 0x20001080;
    memcpy((void*)0x20001080,
           "\x14\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01", 13);
    *(uint64_t*)0x200001e8 = 0x18;
    *(uint32_t*)0x200001f0 = 0;
    syscall(__NR_sendmsg, /*fd=*/r[1], /*msg=*/0x200001c0ul,
            /*f=MSG_OOB|MSG_DONTWAIT*/ 0x41ul);
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