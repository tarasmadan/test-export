// https://syzkaller.appspot.com/bug?id=4f3a544c4a1f113216b11517018d36d2007c85d6
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
#include <sys/ioctl.h>
#include <sys/mount.h>
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

static void use_temporary_dir(void)
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    exit(1);
  if (chmod(tmpdir, 0777))
    exit(1);
  if (chdir(tmpdir))
    exit(1);
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

#define FS_IOC_SETFLAGS _IOW('f', 2, long)
static void remove_dir(const char* dir)
{
  DIR* dp;
  struct dirent* ep;
  int iter = 0;
retry:
  dp = opendir(dir);
  if (dp == NULL) {
    if (errno == EMFILE) {
      exit(1);
    }
    exit(1);
  }
  while ((ep = readdir(dp))) {
    if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
      continue;
    char filename[FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", dir, ep->d_name);
    struct stat st;
    if (lstat(filename, &st))
      exit(1);
    if (S_ISDIR(st.st_mode)) {
      remove_dir(filename);
      continue;
    }
    int i;
    for (i = 0;; i++) {
      if (unlink(filename) == 0)
        break;
      if (errno == EPERM) {
        int fd = open(filename, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0) {
          }
          close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno != EBUSY || i > 100)
        exit(1);
    }
  }
  closedir(dp);
  int i;
  for (i = 0;; i++) {
    if (rmdir(dir) == 0)
      break;
    if (i < 100) {
      if (errno == EPERM) {
        int fd = open(dir, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0) {
          }
          close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno == EBUSY) {
        continue;
      }
      if (errno == ENOTEMPTY) {
        if (iter < 100) {
          iter++;
          goto retry;
        }
      }
    }
    exit(1);
  }
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
  for (call = 0; call < 11; call++) {
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
      event_timedwait(&th->done, 45);
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
  int iter;
  for (iter = 0;; iter++) {
    char cwdbuf[32];
    sprintf(cwdbuf, "./%d", iter);
    if (mkdir(cwdbuf, 0777))
      exit(1);
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      if (chdir(cwdbuf))
        exit(1);
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
    remove_dir(cwdbuf);
  }
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[1] = {0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    syscall(__NR_unshare, 0x40000000ul);
    break;
  case 1:
    syscall(__NR_mmap, 0x20000000ul, 0xb36000ul, 0x2000003ul, 0x10ul, -1, 0ul);
    break;
  case 2:
    memcpy((void*)0x20000180,
           "vcan0\000\000\000\000\000\000\000\000\000\000\000", 16);
    *(uint16_t*)0x20000190 = 0x200;
    syscall(__NR_ioctl, -1, 0x400454d9, 0x20000180ul);
    break;
  case 3:
    res = syscall(__NR_socket, 2ul, 1ul, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 4:
    *(uint32_t*)0x20000240 = -1;
    *(uint64_t*)0x20000248 = 0x20000440;
    *(uint64_t*)0x20000250 = 0x20000540;
    *(uint64_t*)0x20000258 = 0;
    syscall(__NR_bpf, 1ul, 0x20000240ul, 0x20ul);
    break;
  case 5:
    *(uint32_t*)0x20000400 = 0;
    syscall(__NR_getsockopt, r[0], 6, 0x1a, 0ul, 0x20000400ul);
    break;
  case 6:
    *(uint64_t*)0x20000280 = 0;
    *(uint16_t*)0x20000288 = 2;
    *(uint16_t*)0x2000028a = htobe16(0x4e22);
    *(uint32_t*)0x2000028c = htobe32(0x7f000001);
    *(uint16_t*)0x20000298 = 2;
    *(uint16_t*)0x2000029a = htobe16(0x4e20);
    *(uint8_t*)0x2000029c = 0xac;
    *(uint8_t*)0x2000029d = 0x14;
    *(uint8_t*)0x2000029e = 0x14;
    *(uint8_t*)0x2000029f = 0x17;
    *(uint16_t*)0x200002a8 = 2;
    *(uint16_t*)0x200002aa = htobe16(0x4e24);
    *(uint32_t*)0x200002ac = htobe32(0xa010101);
    *(uint16_t*)0x200002b8 = 0x201;
    *(uint16_t*)0x200002ba = 0;
    *(uint64_t*)0x200002c0 = 0;
    *(uint64_t*)0x200002c8 = 0;
    *(uint16_t*)0x200002d0 = 5;
    *(uint64_t*)0x200002d8 = 0x200003c0;
    memcpy((void*)0x200003c0, "ip6gretap0\000\000\000\000\000\000", 16);
    *(uint64_t*)0x200002e0 = 0;
    *(uint64_t*)0x200002e8 = 3;
    *(uint16_t*)0x200002f0 = 0;
    syscall(__NR_ioctl, -1, 0x890c, 0x20000280ul);
    break;
  case 7:
    syscall(__NR_setsockopt, -1, 0x29, 0xc8, 0ul, 0ul);
    break;
  case 8:
    *(uint16_t*)0x20000000 = 0xa;
    *(uint16_t*)0x20000002 = htobe16(0x4e24);
    *(uint32_t*)0x20000004 = htobe32(0x3a);
    *(uint64_t*)0x20000008 = htobe64(0);
    *(uint64_t*)0x20000010 = htobe64(1);
    *(uint32_t*)0x20000018 = 0x3f;
    *(uint16_t*)0x2000001c = 0xa;
    *(uint16_t*)0x2000001e = htobe16(0x4e24);
    *(uint32_t*)0x20000020 = htobe32(0);
    *(uint8_t*)0x20000024 = -1;
    *(uint8_t*)0x20000025 = 1;
    *(uint8_t*)0x20000026 = 0;
    *(uint8_t*)0x20000027 = 0;
    *(uint8_t*)0x20000028 = 0;
    *(uint8_t*)0x20000029 = 0;
    *(uint8_t*)0x2000002a = 0;
    *(uint8_t*)0x2000002b = 0;
    *(uint8_t*)0x2000002c = 0;
    *(uint8_t*)0x2000002d = 0;
    *(uint8_t*)0x2000002e = 0;
    *(uint8_t*)0x2000002f = 0;
    *(uint8_t*)0x20000030 = 0;
    *(uint8_t*)0x20000031 = 0;
    *(uint8_t*)0x20000032 = 0;
    *(uint8_t*)0x20000033 = 1;
    *(uint32_t*)0x20000034 = 0x3ff;
    *(uint16_t*)0x20000038 = 7;
    *(uint32_t*)0x2000003c = 9;
    *(uint32_t*)0x20000040 = 2;
    *(uint32_t*)0x20000044 = 1;
    *(uint32_t*)0x20000048 = 1;
    *(uint32_t*)0x2000004c = 0x4d0e;
    *(uint32_t*)0x20000050 = 0x800;
    *(uint32_t*)0x20000054 = 8;
    *(uint32_t*)0x20000058 = 0x8ab4;
    syscall(__NR_setsockopt, -1, 0x29, 0xcc, 0x20000000ul, 0x5cul);
    break;
  case 9:
    *(uint16_t*)0x20000300 = 0xa;
    *(uint16_t*)0x20000302 = htobe16(0xce21);
    *(uint32_t*)0x20000304 = htobe32(0xfffffffc);
    *(uint8_t*)0x20000308 = -1;
    *(uint8_t*)0x20000309 = 1;
    *(uint8_t*)0x2000030a = 0;
    *(uint8_t*)0x2000030b = 0;
    *(uint8_t*)0x2000030c = 0;
    *(uint8_t*)0x2000030d = 0;
    *(uint8_t*)0x2000030e = 0;
    *(uint8_t*)0x2000030f = 0;
    *(uint8_t*)0x20000310 = 0;
    *(uint8_t*)0x20000311 = 0;
    *(uint8_t*)0x20000312 = 0;
    *(uint8_t*)0x20000313 = 0;
    *(uint8_t*)0x20000314 = 0;
    *(uint8_t*)0x20000315 = 0;
    *(uint8_t*)0x20000316 = 0;
    *(uint8_t*)0x20000317 = 1;
    *(uint32_t*)0x20000318 = 0x916d;
    *(uint16_t*)0x2000031c = 0xa;
    *(uint16_t*)0x2000031e = htobe16(0x4e23);
    *(uint32_t*)0x20000320 = htobe32(0x8651);
    *(uint8_t*)0x20000324 = -1;
    *(uint8_t*)0x20000325 = 2;
    *(uint8_t*)0x20000326 = 0;
    *(uint8_t*)0x20000327 = 0;
    *(uint8_t*)0x20000328 = 0;
    *(uint8_t*)0x20000329 = 0;
    *(uint8_t*)0x2000032a = 0;
    *(uint8_t*)0x2000032b = 0;
    *(uint8_t*)0x2000032c = 0;
    *(uint8_t*)0x2000032d = 0;
    *(uint8_t*)0x2000032e = 0;
    *(uint8_t*)0x2000032f = 0;
    *(uint8_t*)0x20000330 = 0;
    *(uint8_t*)0x20000331 = 0;
    *(uint8_t*)0x20000332 = 0;
    *(uint8_t*)0x20000333 = 1;
    *(uint32_t*)0x20000334 = 7;
    *(uint16_t*)0x20000338 = 0;
    *(uint32_t*)0x2000033c = 0;
    *(uint32_t*)0x20000340 = 0;
    *(uint32_t*)0x20000344 = 0x81;
    *(uint32_t*)0x20000348 = 8;
    *(uint32_t*)0x2000034c = 0x90000;
    *(uint32_t*)0x20000350 = 1;
    *(uint32_t*)0x20000354 = 0xfffffffc;
    *(uint32_t*)0x20000358 = 0x8000000;
    syscall(__NR_setsockopt, -1, 0x29, 0xd2, 0x20000300ul, 0x5cul);
    break;
  case 10:
    syscall(__NR_setsockopt, -1, 0x29, 0xc9, 0ul, 0ul);
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
      use_temporary_dir();
      loop();
    }
  }
  sleep(1000000);
  return 0;
}