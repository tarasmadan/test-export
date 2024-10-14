// https://syzkaller.appspot.com/bug?id=896177e5e24481c07897a3bc3b8de90760c40b27
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
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
  if (pthread_create(&th, &attr, fn, arg))
    exit(1);
  pthread_attr_destroy(&attr);
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
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG);
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
    if (__atomic_load_n(&ev->state, __ATOMIC_RELAXED))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
}

static long syz_open_pts(long a0, long a1)
{
  int ptyno = 0;
  if (ioctl(a0, TIOCGPTN, &ptyno))
    return -1;
  char buf[128];
  sprintf(buf, "/dev/pts/%d", ptyno);
  return open(buf, a1, 0);
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

#define SYZ_HAVE_SETUP_TEST 1
static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
}

#define SYZ_HAVE_RESET_TEST 1
static void reset_test()
{
  int fd;
  for (fd = 3; fd < 30; fd++)
    close(fd);
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
  for (call = 0; call < 8; call++) {
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
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      execute_one();
      reset_test();
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
#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    memcpy((void*)0x203e0000, "/dev/ptmx\x00", 10);
    res = syscall(__NR_openat, 0xffffffffffffff9c, 0x203e0000, 6, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint32_t*)0x20000640 = r[0];
    sprintf((char*)0x20000644, "%023llo", (long long)-1);
    *(uint64_t*)0x2000065b = 0;
    *(uint64_t*)0x20000663 = 0x20000780;
    sprintf((char*)0x20000780, "0x%016llx", (long long)r[0]);
    sprintf((char*)0x20000792, "%020llu", (long long)0);
    *(uint64_t*)0x200007a6 = 0;
    *(uint32_t*)0x200007ae = -1;
    memcpy(
        (void*)0x200007b2,
        "\x27\x6c\x82\x11\x9f\x6f\xb8\x83\x15\x7b\xbc\x63\x88\xd7\x0a\xde\xf4"
        "\xe1\x7c\xcb\xef\xcb\x2f\x57\xa6\x81\xf3\x72\x0d\x5a\x81\xf9\x63\x8e"
        "\xcd\x27\x49\x83\x26\x12\x68\x81\x26\x7b\xc1\xe1\x00\x42\xb3\x5a\x9d"
        "\xee\xd5\xbb\x69\xcb\xbe\x44\xc8\x08\xad\x61\x6d\xf3\xf2\x58\xd4\x14"
        "\xd2\x4f\xd6\x2c\x7a\xf6\x29\x2c\xfd\x48\x02\x83\xfb\xc8\x29\x2e\x0c"
        "\xc4\x44\x45\xaf\xe6\x37\x8d\x22\xf8\xf7\x84\x4c\xb8\x8b\x04\x35\x16"
        "\xb5\x5e\xde\xe4\x85\xf7\xd3\x9c\x87\x73\xc0\xfa\x35\x90\x93\xcf\x47"
        "\x25\x22\xfd\xa9\xf0\x65\xc6\x08\x3c\x3e\x20\x92\x4b\x78\xcb\x7b\x7f"
        "\xe7\x39\xc8\x32\x36\x79\x9d\x9a\xa4\xf9\x5d\x4c\x0f\xb2\xb6\x0a\x65"
        "\x4a\x3d\x14\x3e\x38\x7d\x9d\x32\x26\xb7\x1d\xae\x03\x40\x8d\x42\x0b"
        "\x66\x72\x60\xc1\x3f\xfd\x36\xd6\x6d\xc3\xf6\x79\x32\xfb\x02\xf6\x69"
        "\x3a\x41\x4f\x66\x5b\x3a\x5e\xc3\x09\xe6\x39\xe5\x04\x77\x45\x3e\x34"
        "\x1c\x64\xf5\x35\x98\xa6\xf0\x5c\x21\x40\x73\x55\x0d\x04\xd0\x82\x3b"
        "\xc1\xce\x8f\xfe\x22\x20\x66\x0e\x97\x3b\xfc\xe1\x8d\x33\xac\x38\x0a"
        "\x54\x41\x21\x43\x20\x57\x9b\x25\xd2\x89\xa4\x74\x07\xd9\x1d\x00\x72"
        "\xa7\x44\x11\xe0\xf5\xf4\x39\x9c\xfa\x17\xa1\x57\x80\x77\x71\xc5\x94"
        "\x4a\xd3\xa4\xc7\xa1\x8e\xc4\x79\xf7\x79\x84\xc6\x76\x21\x53\xa6\x2d"
        "\x2e\xa3\xdb\xab\x87\xab\x65\x65\x2c\x6e\x11\x23\x57\xc7\xe9\xb1\xd1"
        "\xd4\x14\x8a\x6c\xef\x50\xcd\xa0\xd0\x4e\xd2\xee\xa4\x1d\x7b\x79\x1f"
        "\x2a\x64\xd3\x8e\xdf\x5a\x1a\x0d\x01\x27\xd9\x33\x4d\x3a\xa2\x2f\x78"
        "\x52\x75\x9f\xcd\x94\x75\x2d\xf6\x0a\x66\x3c\x18\xcd\x82\xb7\x1a\x5e"
        "\xdd\x9b\x5e\x42\x5f\xe2\x30\xb3\x68\x24\xf1\xd8\x91\x27\x1b\xe4\xd0"
        "\x35\xaa\xc5\xef\xf4\x38\x69\xf5\x71\xc0\xa3\x3b\x56\xe2\x30\x50\x0c"
        "\x83\xe9\x2d\xf4\x59\x28\xfa\x18\x50\x8b\x05\x85\x70\xe5\x2d\x25\x2d"
        "\x54\xb6\x43\xfa\x60\x2f\x4e\xca\x37\xec\xec\x0a\x79\x4d\x8c\xb1\xf3"
        "\xf1\x4a\x8b\xb3\x8b\xb8\x8d\x4a\x4d\xc1\x3c\x02\xc2\x65\x6c\xc0\x76"
        "\x61\xeb\x42\xd0\xe1\x1a\xad\xe5\x36\xc6\x5b\x90\x89\x07\xba\xe8\x1c"
        "\xa2\x82\xc8\xe2\x55\x2f\x27\x66\xa9\x1d\x63\x7b\x70\x3a\x5a\xdb\x14"
        "\xb7\xe2\xa1\x34\x93\xe6\xad\x69\x58\xa9\x70\xdd\xb7\xf6\x4e\x93\x53"
        "\x12\x08\xab\xa8\xee\xf8\xf0\xff\x86\xb4\xea\xb2\xec\xfd\xc3\xf9\xce"
        "\xb3\x7c\xf7\xa1\x6d\x63\xfb\x09\xbe\x25\xc3\x03\x92\x46\xaa\x99\x8c"
        "\x68\xea\x17\x03\xe6\x8f\x12\xb3\xa6\xe8\x57\x02\xf5\x13\x74\x50\x37"
        "\xe3\xa1\x44\x87\x3c\xc2\x01\x06\x4d\x2e\x67\x2f\x86\x72\x10\x22\xc9"
        "\x58\xa8\xee\xdd\xdc\x04\x85\x25\x96\x29\x5d\x23\x88\x82\x92\x2d\xc2"
        "\x01\x03\xb6\xcf\x4f\xc6\x60\x55\xbe\xa4\xd2\x1a\xcc\x0e\x18\x3d\x13"
        "\xac\xc1\x04\xa7\x85\x14\xd2\x1c\x99\xf4\xfb\x13\x5a\xba\xbd\xb3\xce"
        "\xe8\x0a\xf8\xb4\x49\x51\xd8\xb5\xce\x14\x45\x30\xf9\x49\x4e\xa7\xe5"
        "\x8d\x35\x41\xdf\x35\xd1\xef\x8b\x34\x62\x87\x80\xa5\xb8\x70\x9f\x0a"
        "\xe2\x9e\x2c\xeb\x6b\xb6\x1a\x6a\x37\x25\x1a\x41\x29\x2c\x9e\x99\x1b"
        "\xcb\xc7\xb2\x36\xa2\xba\x95\xed\x0a\xa8\x60\x1a\x58\xc0\xbb\x5a\xa0"
        "\x23\x7d\xe3\x40\x42\x62\xb3\x49\x93\x60\x1a\xf8\xce\xbe\x92\x3c\x98"
        "\x2a\x7e\x6c\x83\xaa\x10\x04\x46\xd5\x52\xf6\x53\xb8\x21\x44\x51\x46"
        "\x6e\x35\x87\xca\x44\xa1\x15\xe2\x76\xc9\x96\xa2\xb5\xac\x05\x63\xd8"
        "\x82\x68\x8a\x05\x20\x76\xc1\x72\xde\x74\x99\xe3\x62\xaa\x03\x8c\x11"
        "\x8d\x54\x7c\x46\x2d\xf0\x32\xbd\x1b\x0f\xb4\xd5\x28\xf5\x0b\x8c\xcc"
        "\xd0\xb0\x3b\xd1\x60\x0b\xa7\xf3\x40\x13\xf7\xf6\x19\x5f\x5e\x84\x38"
        "\xcc\xea\x52\x65\x65\x64\xdc\x2d\x05\xc0\x1b\x2c\x3d\xd4\x95\x11\xf4"
        "\xc7\xc6\xae\x21\xe9\x1f\x74\x16\x2a\xeb\x27\x8c\x1f\x59\x09\xaa\xe9"
        "\xdd\xd4\xfd\x65\xc7\xb0\x18\x0f\x0a\x1d\xb0\xa2\xf0\x55\x01\x0e\x44"
        "\xe9\xbe\xb7\x86\x19\xd9\xe5\x13\x58\x43\x61\x68\x77\xca\xa3\xd5\x4e"
        "\x1a\x9d\x28\x89\x59\xf4\x3a\x68\x2f\xdc\xa4\x44\xc2\x10\x1f\xf9\x33"
        "\x08\xe5\x5d\xfe\xbe\x27\x6c\x53\x93\x0b\xba\x8f\x5d\x2d\x5e\x85\x55"
        "\x5d\xac\x84\xef\x2e\xce\x17\x4f\xd6\x22\xbb\x72\x5e\x9d\x25\xfa\x2e"
        "\x91\xf3\x9a\x6d\x4e\x47\xf5\x47\x52\x1e\x13\x59\x55\xba\x43\xdc\x8e"
        "\x5a\xb0\x3f\xfc\x3a\x3d\xa4\xbe\x87\xb2\x9f\x3a\x6b\x8d\xff\x2c\x79"
        "\x76\xe5\xd4\xb3\x5e\x7b\xee\xe3\x28\xad\xd8\xd9\x6c\xcd\x5f\x89\xee"
        "\x15\x50\x74\xed\xa7\x7f\x7b\x80\xcb\x4a\x7b\x79\xd6\x69\x1d\xd4\x9b"
        "\x7a\x5b\x31\xd5\xa8\x9c\xe8\x96\x6e\xf1\x36\xc7\x89\x62\xe6\x34\x46"
        "\x7e\xd7\x2a\xf8\x8b\xe7\xdc\x6c\xc5\xa6\x1a\xa2\x26\x9f\xda\xfa\xbd"
        "\x01\x7c\x80\xdb\x91\x26\xfc\x32\x58\x0c\x9a\xc6\x37\x18\xed\xfd\xec"
        "\x40\xdb\x1f\xb4\x32\x19\xfb\xac\x0a\xa8\xc1\xa7\xc6\x72\x80\xbf\x17"
        "\x84\x5e\xfa\x88\x93\x9d\x0e\x6a\x59\xad\xd3\x6e\x57\x04\xa2\x2e\x66"
        "\x52\x79\xcf\x89\x2b\xc0\xf4\x66\x35\xbf\xa8\x38\xce\xbf\x9d\x78\x66"
        "\x24\xee\x2c\x57\x3f\x4d\x9b\xe5\x51\x84\xc6\x3c\x44\x01\xfb\x3e\x3d"
        "\xcf\x8b\x3d\x56\x0e\x37\x8d\xcf\x06\x58\xb1\x04\xb5\xf9\x01\xaa\xfd"
        "\x3c\xd9\xbe\x3e\x3a\x04\x8c\xd1\x10\x1c\x3c\x2f\x1c\xf4\xc1\x45\x43"
        "\xb2\x15\xa7\xe6\xae\x1c\xf7\xcd\xae\xd9\x75\x31\xac\xad\x28\x22\xdf"
        "\xc2\x1b\x99\x03\xe7\x16\x29\x61\x66\x07\x5c\xf1\x73\xfe\x71\xe5\xc2"
        "\x17\x5f\xb3\xd1\x1d\x95\xfe\x9e\xc1\xe7\x44\x84\xca\x07\x8c\x02\xd6"
        "\x33\x17\x18\x30\x96\xab\x6a\x7e\xc9\xe4\x4c\xd6\x5b\x58\x71\x13\x2c"
        "\x54\xbb\x06\xfd\xa1\x0c\x58\x71\xcd\xa0\xf0\xb3\xe7\x2c\x92\xa7\xb2"
        "\x8e\x49\x01\x6e\xa8\x59\x8f\x48\x9d\xe6\x61\xa7\xc1\x76\xc5\x14\xd3"
        "\x40\xee\x06\x42\x01\xec\xaf\x75\xca\x19\xf2\x61\x07\x0b\xab\xfc\x0a"
        "\x6d\x6a\xe6\x66\x37\x30\x1c\x8e\x4f\xde\x90\x40\xa9\x28\x8f\xd2\xa0"
        "\xfc\x99\x41\x61\x1e\xa6\x29\x05\x98\x63\x40\x36\x6e\xee\x51\x10\x9c"
        "\x78\x97\x69\x8b\x84\xf1\x40\x54\xf3\x14\x66\x3a\xea\xb5\xfb\x2e\x40"
        "\x4f\xf3\x25\xb2\x15\x4d\x12\xca\x6b\x9d\xe5\x59\xd2\x45\xdf\x03\x6a"
        "\x27\x89\xa4\xd0\xda\x8a\xf1\xc7\x8b\x9e\xfe\x1a\xdc\x8d\x8b\x6c\x72"
        "\x07\x68\x5c\x46\x52\xc2\x63\xaa\x13\x3a\x23\x81\xba\x37\x4a\xa2\x98"
        "\xdd\xe6\x61\x0b\xcb\xd8\x31\xfd\x8f\x54\x11\xd7\xc9\x99\x7e\x42\x6b"
        "\x2d\x02\x48\x59\x65\xc3\x0c\xb6\xc4\x11\xe5\x7d\xd4\x65\x34\xd3\x0b"
        "\x09\xe4\xcd\xff\x51\x32\xfa\xa5\x31\x6e\xa1\x74\xfc\x06\x46\x27\xaa"
        "\xa8\x8f\x42\xac\xa6\xca\x49\x35\xa1\x43\xeb\x6a\xf9\x73\x5b\x49\x0c"
        "\xff\x36\x32\xc4\xee\x0f\xa3\x49\xbb\x70\xb1\x46\xcf\x47\xfc\xd7\xad"
        "\x72\x2f\x6e\xbe\xef\xce\x38\xef\xf4\xc3\xf6\x88\x6d\x89\xbf\x12\x1c"
        "\xb0\x18\x2a\x70\x73\xf8\x13\xe2\x48\x6c\xba\x9a\xf0\xf9\x80\xfb\x6c"
        "\x20\x8f\x23\x35\xda\x46\x50\x1a\x81\xec\x99\x23\xbc\xcd\x8a\x91\x65"
        "\xd9\xfa\x9c\x94\x7a\x62\xe9\xed\x76\xfb\x6b\xb6\x04\x6d\xfa\xb5\x43"
        "\xa2\x58\x3d\xdc\x64\xbd\x5f\x0e\x12\xef\x14\x21\xf2\xfa\xa6\x18\x4a"
        "\x52\x62\xb1\x39\x84\x4b\x89\x72\x49\x92\xbb\x1f\x9a\x5a\x67\x8b\x0c"
        "\xdd\xa1\x57\xc9\x14\xbd\xe0\x94\x93\x98\x90\xba\xad\x53\xc3\x05\x6e"
        "\x18\x66\x7a\x92\x38\x5c\xcb\xac\x22\x52\x9e\x55\x4f\x3b\x29\xe9\xd5"
        "\xbf\xca\x74\xf0\xfd\x90\xc6\xe8\x33\x88\x72\x81\xd4\x2e\x77\xb4\xac"
        "\x06\x10\x1f\x12\xc1\x10\x51\x7b\x29\x72\xd3\x44\x98\xb3\xb7\x84\xf7"
        "\x5a\xe3\xd4\x21\xed\x6d\x65\x8a\x4a\xed\x35\xe1\x83\x37\xb8\x26\x33"
        "\xbe\xd9\x18\xba\x18\x65\x9f\x69\x16\x7d\x52\xfd\x18\xa5\xb8\x33\x74"
        "\x85\x29\xaf\x53\x02\x27\x6b\xd3\x28\x44\xd1\xfc\x02\x33\x60\xbd\x67"
        "\xa8\x18\x4e\x27\xac\x25\xa0\x51\xa9\xd3\x9d\x07\xc7\x75\xd6\xd0\x52"
        "\x6b\x9d\xf6\x77\x6c\x4f\x03\x18\xf5\x3e\xbb\x52\x8e\xe8\xa0\xb3\x88"
        "\x5a\x9b\x1b\x83\x48\xb4\x4b\xe7\x4a\xb7\xae\xef\x16\x5a\x85\x52\xe6"
        "\xe5\x00\xfe\x75\x16\xa1\xd2\xee\xe8\xf6\x6d\xf3\xc4\x47\x48\xfd\x4d"
        "\x20\x3c\xea\x3b\x8c\xb0\x28\x30\xd6\x0a\xb1\xfe\x4d\x06\xa0\x67\xaa"
        "\x88\x17\x5c\xd1\x0f\x4f\x78\xfd\xc8\xaa\x0f\x79\xe6\xd7\x66\x8e\x79"
        "\x7c\x03\x5c\x03\x9c\xd7\x53\x5c\xec\xe8\xe1\x2d\xcd\x63\x21\xcc\xd3"
        "\xd9\xa0\xf6\xff\xc4\xa8\x41\x2a\x16\x9e\xf2\xeb\x54\x0a\x12\xd6\xaf"
        "\xf6\xbd\xe1\x7b\x1d\xd0\x21\xe5\xca\xba\xa5\xe7\x73\x56\x60\x64\x2d"
        "\x19\x36\x12\x24\x69\x04\x70\xc7\xc1\x26\x3f\x42\x48\xf7\x54\x32\xae"
        "\xc8\xa7\xaf\x17\x78\x99\x03\x8c\x22\xe8\x67\x72\x96\x3d\x41\x6b\xeb"
        "\xc2\x6f\x40\x3e\x0a\xef\xf1\xcd\xf4\xdf\x85\x7d\xef\x3e\x14\xdd\xec"
        "\xdc\x23\x9e\x6f\xf1\x52\x51\xfb\xa3\xb9\x45\x73\x40\x31\xf5\x19\xa2"
        "\x9d\x83\xe9\x6d\x85\x2d\xa3\xd3\x78\x9b\xfe\xd2\x59\x1b\xa1\x44\xaa"
        "\x25\xff\x2e\x0c\xce\x7e\x9f\x8a\x1e\x12\xac\x93\x4c\x0c\xca\x6b\x48"
        "\xc1\x97\xf3\x1a\xf0\xe8\x80\x3d\x19\x07\x48\x3b\x24\x8b\xd7\x31\x98"
        "\xf1\x9a\xdf\xd9\xc5\xab\x3f\x7d\xdf\x34\x3f\x0f\x0e\x2f\xd2\x1c\x22"
        "\x15\xcf\x47\x09\xb2\x8f\xd1\xbe\x4f\x3f\xe0\x54\x2f\x75\x49\xab\x7f"
        "\x61\x8b\x99\xab\xd8\x59\x8c\x15\x80\xc9\xf9\x1a\x1d\x18\xa3\xdf\xb8"
        "\xa9\xf2\xb6\xc3\x87\xfc\x1e\xa7\x53\x1d\xca\x79\xfc\xa8\x14\x47\x4e"
        "\x7f\x1a\xe1\x1b\xa9\x2d\x24\x24\x36\x1d\x31\x5c\xa6\xdc\x49\x4d\x0e"
        "\x74\xd2\x4d\xfe\x15\xfa\xb9\x71\x71\x24\xf3\x1c\x79\x1d\x09\x2a\x7d"
        "\x88\x1f\x4c\x17\x74\x4e\x1d\x22\xa4\xe8\xbf\x8e\xa0\xfc\xd2\xdf\xbc"
        "\xa1\x12\x8f\xeb\xc2\xba\x30\xae\xb3\xd5\xb0\x0f\x9b\x6c\x34\x83\x4a"
        "\xa7\x0a\xa0\x29\xe7\xd8\x6b\x12\xde\x4a\xb5\x12\x08\x03\x79\x96\x3f"
        "\x73\x6a\x51\x94\x11\x02\x28\x2c\x94\x4b\xb7\xa5\x1b\x42\x94\xe9\xac"
        "\x11\xb1\x94\x25\x13\xe4\x50\x07\xfa\x1d\x9d\xaa\x14\xee\x7b\x1e\xd0"
        "\xa8\x21\x7e\x52\x2e\x1e\x10\xe4\xc8\x9a\xf7\xd0\xcf\x52\xf0\x4d\x62"
        "\x37\x66\x95\xe8\x01\xad\x71\xeb\x56\x0a\x45\x52\x51\xe2\x89\xbe\x2e"
        "\x60\x7e\x18\x7c\x05\x15\xe7\x50\x26\x3b\xa1\x6d\xea\xc0\x95\x33\x7c"
        "\x06\xf0\xab\x50\x5f\x6c\xd4\xef\xc5\x48\xfc\x81\x52\x17\x80\xbb\x68"
        "\xd9\x88\x7f\x27\xa2\x62\x69\x55\x6b\x90\xbc\xf4\x42\x34\xab\xea\xf2"
        "\xa9\x43\x15\x09\x4c\x99\xef\xd8\x31\x96\xb8\x37\x6a\xf1\x04\xf4\x0a"
        "\x90\x7a\x59\x91\xf6\xe8\x70\x54\xf5\x08\x66\x14\x13\x09\x8d\x03\xc3"
        "\xe4\x01\xf5\xaf\x28\x2b\x57\xa5\xbb\x39\xfd\x6d\x9f\x46\xc7\xf6\xe1"
        "\xdf\x14\xc5\xde\x08\x58\xf8\x21\xda\xb4\xdb\x31\x22\x14\xf6\x3b\xf4"
        "\x74\x20\x6c\x8a\xee\x47\x34\xeb\x99\x0d\xed\x84\xc7\x43\xd2\xc1\xb9"
        "\x5d\x78\xea\x42\x3c\xd1\x4c\x48\x5a\x11\xd7\x7c\x76\x14\x39\x8d\x73"
        "\x32\xd4\x07\x97\xcf\x22\x3d\x33\x7d\x29\x3a\xa6\x73\xa0\xfb\x48\xf6"
        "\x5d\x74\x00\x78\xe1\x30\xad\xf5\xf6\xfb\xd2\xfd\x30\xd7\x56\x70\x10"
        "\xfa\xd0\x3a\x77\x34\x99\xd6\xb8\x18\x7e\xb1\x60\x9b\x1f\x60\x4b\xa5"
        "\x8e\xeb\x67\x19\x3e\x21\x7b\x0e\x04\xfa\xda\x30\x21\x1e\x9e\x97\x85"
        "\x84\x64\x0e\x81\x0e\xf4\x11\x1d\x6b\x1b\xea\x70\x84\xb5\xe4\x7b\xe2"
        "\x78\xe9\x4b\x94\xf1\x21\x09\x64\xee\xda\x71\xa7\xc6\x8d\x33\x6c\x6b"
        "\x33\x59\x4b\x60\xa2\xa7\x95\xe5\x41\xb6\x6f\x72\x6b\x87\xf4\xae\x27"
        "\x32\xd1\x45\xd9\xa2\x45\x57\x30\x48\x7b\x6d\x50\x35\x92\x6f\xad\xac"
        "\x16\x09\x7d\xce\x38\xc6\xdf\x6a\x2d\x07\xb4\xe7\x44\xc2\x58\xbf\xc5"
        "\x57\xca\x56\x73\x06\x28\xdf\x59\x94\x80\x6e\xa8\xd1\x65\xd3\x1a\xdc"
        "\x45\xfa\x3f\xb9\xc2\x91\x0c\x30\x0a\x26\xb0\x5b\xd5\xee\xdb\x71\x79"
        "\x37\x33\x4d\xd1\x2d\xf5\xba\xca\x6f\x96\x4c\x10\x39\x12\x37\x9a\xd2"
        "\x42\x84\x8b\x13\xc2\xd4\xbe\xa4\xb1\x86\xa4\x45\x2f\x37\x39\xe2\xe6"
        "\xc9\x60\x52\xdf\xe6\x50\x94\x1d\xac\xb2\xe5\x73\x23\xbc\xf4\xd0\x01"
        "\x9f\xb4\x9d\x4c\xbf\x2e\x57\x43\x1b\x6b\x29\x10\x99\x96\x8b\xdb\x73"
        "\x48\x2e\x36\x3a\x82\x1d\x26\x7c\xbd\xa3\xbd\x56\xde\x02\xad\xe4\x65"
        "\xcd\xbe\xea\x87\x60\xc8\x3d\x5b\x4c\x20\x08\xf3\x4f\x66\x56\xbe\xcf"
        "\xf2\x7c\x1b\x54\x95\xab\xaa\xaa\x9d\x0a\x43\x21\x7d\x4b\xb9\x72\x1a"
        "\x3e\x1f\xf4\x71\x95\x65\xf6\x7d\x94\x61\xcb\xc0\xd5\x6d\x6d\x5e\x61"
        "\x9f\x48\x5d\xd8\x3d\x31\x3d\xf4\x2c\x0d\x25\x57\xb9\x40\x6a\xb6\x0c"
        "\xd4\xa1\x87\xb2\x69\xce\x82\xb2\x1a\x3d\xfb\xa6\x12\x96\x45\xd0\x3e"
        "\x9c\xa9\x98\xff\x69\xa1\x30\xa4\xd1\x23\x5e\xbc\x4f\xd0\x2d\x5c\x54"
        "\x44\x9e\x6b\x29\xaa\x58\xb9\x9d\xe9\xbb\x4c\xae\xe6\xd5\xd8\x88\xbf"
        "\x2a\x5a\xfd\xda\x25\x5a\xea\x73\x72\xdc\x64\x1a\x61\xe7\x27\x95\xc1"
        "\x90\x66\x11\x78\xeb\xd6\x71\xf3\x58\xa8\x25\x84\x8f\x66\x40\xaa\xed"
        "\xe4\x68\xaf\x1e\x0e\xfe\xf8\xaf\xab\x84\xce\xd0\xe0\x37\xfb\xbb\x1c"
        "\x89\xb7\xe6\x95\xc3\x07\xe3\x5d\x12\x94\xe5\xde\xbb\x54\x06\x57\x77"
        "\x8f\x1f\x84\x74\x0c\xcf\x62\xa4\x8a\x05\xca\xc8\x16\x29\xbd\x87\xf9"
        "\x3f\x16\x74\x51\x5b\xbe\x56\x10\xbc\x31\x41\x0c\x1e\xb7\xb6\x60\x85"
        "\x49\x33\xb5\xbe\x6f\xa3\x45\xd6\x52\x58\x9d\xd8\x1c\x66\xd2\xfd\x1c"
        "\x9b\x8f\x8c\xce\xe3\x78\x66\xc1\x34\x1f\x60\x33\xa0\x14\xef\x1d\xd2"
        "\x5c\xb9\x83\x2c\x42\xe8\x31\x39\x85\x4b\xa7\x81\x3b\xdf\xf7\x5d\x0d"
        "\x91\x24\x63\xa6\xc1\x5d\x36\xdc\xa1\x14\x17\xea\xd2\x64\x2b\x35\xc6"
        "\x28\xeb\x9d\xf6\x08\x70\xdc\xcc\xbf\x94\x46\x37\x95\x9d\xb8\x44\x61"
        "\x61\x73\x0b\x8e\x7e\x83\xea\xbe\xc1\x67\x22\x85\x1d\x4d\x57\xaf\x9a"
        "\x32\xec\x91\xb7\xea\x15\xae\xb4\x08\x65\x9b\x98\x89\x8e\x47\x01\x85"
        "\x2f\x12\xd5\x05\xdc\xa4\x74\xf6\x52\x16\x85\xf1\x03\x87\x96\xbd\xb1"
        "\x6f\x4a\x9e\x8e\xfa\x3b\x9f\x60\x7c\x28\x94\x6e\xb6\x1f\x0a\xa5\x90"
        "\xed\x1a\x57\x84\xa8\x5c\xb8\x54\xbd\x49\x07\xf2\x3d\xd2\x3d\xf2\xc7"
        "\xe0\xa6\x69\x02\xd5\xff\x8a\x0c\x91\x11\x73\xea\x9e\xc6\xf5\xa8\xa3"
        "\xb5\xa2\x84\x7c\x94\xd6\x46\xf5\xa9\x3d\x23\x08\xfd\x79\xb1\xd4\x9c"
        "\x0f\xed\xcc\x9e\xf9\x7f\xa5\xbe\x3c\xde\xf1\x3a\x03\x39\x82\x5b\x5e"
        "\xc3\x1e\x9d\x28\x4b\x33\x7a\x4f\xe5\x07\x42\x0d\x11\x8c\x99\xe0\xfa"
        "\xbc\xe7\x4f\xcc\xf0\x7a\x1f\x98\x36\xf9\xb3\x76\xdf\x20\xa7\x8b\xc0"
        "\xbf\x1e\x81\xa1\x58\x0f\xa6\x9b\xd3\x14\xf1\x07\x64\xe9\x0e\x2a\x66"
        "\xad\x29\x8c\x49\x3e\xdd\x4b\x17\x3c\x26\x72\x79\x1e\x16\xa5\x8b\xbf"
        "\xae\xa7\xbf\x97\xf4\x93\xd3\x3b\xb0\x0f\x91\xd9\x6b\xd4\x62\xc1\x4b"
        "\xc7\x16\x84\xf2\xc2\x8e\xf8\x6a\x4b\xbc\xe5\x7a\xc3\xc0\x6b\x0b\x1f"
        "\xe2\x4f\xc8\x15\x2c\x66\xa3\x66\x13\xdb\xf6\xfe\xb4\x32\xb6\x0a\x1b"
        "\x17\xbc\x08\x4a\xa8\xed\xe4\x00\x89\xb3\x65\x48\xd6\x49\x3a\x04\x03"
        "\x8f\xb8\xb6\x26\xfa\xa7\x93\x83\x54\x4c\x42\x96\xce\x4c\x84\x91\x31"
        "\xb8\x59\x68\x4f\x5b\xa7\x6b\x8c\xb6\xb6\x4a\x00\x65\x47\x78\xcf\x62"
        "\x4e\x6c\x6d\x22\x26\xe5\x97\xc3\x5c\xa8\xd4\x55\xe3\x80\xa2\xea\x53"
        "\x5d\x19\xf4\x3c\x99\x3e\x5d\x1c\xac\x36\x9a\xae\xac\xd6\x13\x7c\x3a"
        "\xf6\xe4\x97\xec\xe6\x74\x1c\x8d\xc1\x2a\xa8\x68\x91\x85\x2f\x16\x30"
        "\xf9\x1e\x9a\x95\x0f\xfa\x57\x44\x4c\xfc\xc8\x80\xbf\x40\x4e\xa8\x11"
        "\x71\x38\x54\x9f\xba\x8f\x7c\x59\x8b\xee\x0d\x62\xa5\xa6\x11\xb1\x3e"
        "\x3d\x3f\x92\x9d\xc2\x58\x3b\xf5\x8f\x6a\x3b\x45\x7e\xa2\xa5\xac\x06"
        "\xb7\x7c\xbc\x87\xe3\x98\xe1\x60\x24\xbe\x90\x02\x37\x6c\x8a\x74\x06"
        "\x55\xa7\x5a\xc2\xa8\x55\x94\x80\x7f\x5e\x8f\x74\x07\x46\x31\x1f\x77"
        "\x96\x31\x12\xd9\x49\x7e\x94\x35\x01\x13\xbf\xdf\xeb\xcc\x99\x2c\x39"
        "\x65\xa5\xd8\x37\xce\x0d\xae\x2b\x4e\x38\xe2\x8e\x03\xd5\xfc\x3a\x4f"
        "\xc7\x92\x05\xd2\x2b\xd9\x72\x8f\x8f\xb6\x67\x08\x69\x8b\x59\xd7\xce"
        "\x1a\x29\xbb\x0e\xf8\x8c\x57\x0e\x94\x09\x44\xde\xb9\xda\x34\xbc\x67"
        "\xde\x39\x60\x85\x46\x6e\x73\xc2\x1e\xca\x3c\x7c\x47\xe7\xe7\x74\x7a"
        "\xbf\x5d\x92\x5a\xa2\x45\xc6\x00\x66\x5b\xb0\x82\xe3\x65\xd6\x09\xe7"
        "\x7b\xf5\x7a\xeb\x46\x5a\xdb\x34\x98\x44\x6a\x15\xf2\x87\xcf\xb0\xd3"
        "\x19\xe9\x92\xed\x9b\x03\x29\xa7\x6c\xae\x13\x57\x92\x8a\xab\x1f\x91"
        "\xc0\x85\x48\x0e\x36\x3d\xcf\x69\x28\x88\xde\x9b\x7d\xce\x1d\x99\xf3"
        "\xc4\x3a\xe5\xb4\xd1\xe0\x75\xf2\xba\x64\x7a\xfb\x0f\x20\xe1\xa2\xfa"
        "\xbd\xe0\xa2\x59\x53\x09\xf9\x3a\xdb\x19\xd9\x42\x9b\x7e\x17\xe7\x00"
        "\x45\xa7\xd2\xa3\xfb\xaa\x92\xe0\xd2\x94\xfe\x4e\xaf\xd1\x15\x53\x84"
        "\xb5\xe5\x2f\xa7\xb5\x40\x80\x80\xe0\x22\xac\xbf\xd1\xef\x45\xa0\xac"
        "\x7a\xdd\xa0\x0f\xc5\x56\xc6\xf2\x14\x9a\x0f\xcb\x33\x99\xfa\x2a\x55"
        "\x63\x70\xd0\x79\xd0\xea\x8a\x1f\x32\x62\xb0\x57\xe3\xdd\xd5\x8d\x49"
        "\x06\xdf\xb3\xeb\xd4\x1c\x40\x33\x46\x75\x06\x32\x2a\xaa\xcb\xad\xb3"
        "\x4d\xdd\x26\xbb\x17\x6d\x86\x91\xaf\xb3\xe8\x5b\xee\xd9\x99\x38\x26"
        "\xb8\x0a\x64\x15\x16\xbf\x47\xb1\x65\xd1\x93\x40\x51\x22\x73\x24\x86"
        "\x82\x42\xe1\xea\xd9\x6e\xe7\x83\x1c\x29\x97\x62\xc5\x50\xdb\x26\xc7"
        "\x48\x3a\xc1\xf2\x08\xac\x03\x36\x2f\x3b\x95\x04\x9e\xd7\x5c\xd6\x15"
        "\x77\x84\xd5\x33\x99\x37\x75\xd7\xac\x2c\xf5\xa3\x29\x70\x74\x30\xb3"
        "\x57\x0a\x74\xcf\x80\x15\xbf\xe4\x17\x0c\xe7\x70\x30\x8b\x25\x6e\xfe"
        "\x7a\x4f\x31\x2a\xfb\xd7\x33\x47\x09\xfa\x06\xb5\xdc\x09\x0b\xc2\x73"
        "\x62\x11\x54\xde\xaa\x80\x7d\xec\x68\xd7\xba\x32\x3a\xe1\x53\xc0\x4b"
        "\xf7\x64\xeb\x5c\xea\x2d\x3b\x83\x9b\xdf\x93\xfa\xb3\x39\xd6\x68\x42"
        "\xe1\xa8\x3a\x6f\x68\x37\x29\x4c\x3c\xf5\x68\x7d\x8b\xc8\xd6\x08\x9c"
        "\xa2\x51\x64\xf7\xa5\x96\xa0\xc7\x87\x5f\xaa\x0a\xfb\xf4\xe7\x66\x43"
        "\xa8\xb1\x34\x89\x5a\x45\xab\xf8\x28\x4b\x64\x60\x9d\x1b\x3e\x37\x79"
        "\x69\x4c\xa9\x8c\x4f\x8a\x95\x64\x00\x83\x0d\xff\x1c\xea\xe4\x82\x3a"
        "\xea\xd7\xfa\x4c\x45\x60\x04\x73\xc8\x28\x16\x49\xc2\x17\x31\xe6\x10"
        "\xca\x80\xfe\x92\xa3\x78\x53\x1c\xa4\x62\x78\xe0\x50\x01\x78\x6b\x9c"
        "\x44\x5d\x23\x00\xe8\x9d\x9c\xff\x73\x94\x1d\x17\xa3\x63\xff\x6a\x67"
        "\x42\xca\x36\xb2\xaa\x25\xcf\x2b\x38\xc0\x64\x26\x99\x2e\x6c\x36\xfa"
        "\x7e\xf3\xff\xf3\x80\x51\x04\xc5\x54\x0b\x7e\x9a\xf8\xce\xde\xfa\x08"
        "\x1f\x64\xa2\x06\xa8\x69\x70\x1e\xf0\xa6\x9e\x9e\xb7\x6d\xaa\x9d\xce"
        "\x1c\xdd\xba\x41\x50\x97\x21\x0b\x7d\x60\x5b\x0d\x9a\xcf\xea\x36\x1f"
        "\x93\x0b\xba\xd6\x47\x58\x17\x61\xa4\x45\x59\x2e\x08\xcf\x49\xac\xfe"
        "\xc0\x05\x36\xc3\x07\x62\xa3\xc2\xba\x13\x47\x36\x38\xc2\x90\xf7\x9a"
        "\xd8\xed\xde\xc1\xe1\x62\xe0\x9c\xd2\x62\xd1\xd4\xd0\x05\xc4\x1a\xbb"
        "\x00\xb9\x20\xe8\xe8\xee\x5f\x36\xc7\x9d\x94\x21\x11\x74\x41\xd8\x07"
        "\xba\x6a\x26\x09\x26\x26\xa9\x83\x6b\x83\x54\x0d\xb9\x10\xcc\x31\x0a"
        "\x87\xa7\xa9\xa0\xcb\x19\xb9\x0c\xb1\x13\xfe\x4f\xf9\x81\x42\xc1\x4f"
        "\xf6\x7d\x85\xfc\x3b\x0f\xab\x93\x4f\x56\x10\x7e\x24\xe4\x87\xd5\x01"
        "\x46\xd1\x59\xa0\x37\x6d\x9d\x3d\x82\xd4\x6a\xf5\x21\x22\x64\x4d\x6e"
        "\xb7\x39\xb6\xd1\x14\x2e\x26\x12\x32\x1e\x1c\x00\xc4\x57\xd0\x98\x53"
        "\x1a\x5f\xca\xb5\xb1\xbf\x5c\x14\x93\xaa\xca\x10\x07\xa2\xb9\x8a",
        4096);
    sprintf((char*)0x200017b2, "0x%016llx", (long long)r[0]);
    sprintf((char*)0x200017c4, "%023llo", (long long)r[0]);
    *(uint64_t*)0x200017db = 0;
    sprintf((char*)0x2000066b, "%023llo", (long long)0);
    *(uint64_t*)0x20000682 = 0;
    sprintf((char*)0x2000068a, "0x%016llx", (long long)-1);
    syscall(__NR_ioctl, -1, 0x40085203, 0x20000640);
    break;
  case 2:
    *(uint32_t*)0x20000040 = 0;
    *(uint32_t*)0x20000044 = 0;
    *(uint32_t*)0x20000048 = 0;
    *(uint32_t*)0x2000004c = 0;
    *(uint8_t*)0x20000050 = 0;
    *(uint8_t*)0x20000051 = 0;
    *(uint8_t*)0x20000052 = 0;
    *(uint8_t*)0x20000053 = 0;
    *(uint32_t*)0x20000054 = 0;
    *(uint32_t*)0x20000058 = 0;
    *(uint32_t*)0x2000005c = 0;
    *(uint32_t*)0x20000060 = 0;
    syscall(__NR_ioctl, r[0], 0x40045431, 0x20000040);
    break;
  case 3:
    res = syz_open_pts(r[0], 0);
    if (res != -1)
      r[1] = res;
    break;
  case 4:
    syscall(__NR_bpf, 0x11, 0, 0);
    break;
  case 5:
    *(uint32_t*)0x200001c0 = 7;
    syscall(__NR_ioctl, r[1], 0x5423, 0x200001c0);
    break;
  case 6:
    syscall(__NR_write, r[0], 0x200000c0, 0xffa8);
    break;
  case 7:
    *(uint32_t*)0x20000000 = 0xfdfdffff;
    *(uint32_t*)0x20000004 = 0;
    *(uint32_t*)0x20000008 = 0;
    *(uint32_t*)0x2000000c = 0;
    *(uint8_t*)0x20000010 = 0;
    *(uint8_t*)0x20000011 = 0;
    *(uint8_t*)0x20000012 = 0;
    *(uint8_t*)0x20000013 = 0;
    *(uint32_t*)0x20000014 = 0;
    *(uint32_t*)0x20000018 = 0;
    *(uint32_t*)0x2000001c = 0;
    *(uint32_t*)0x20000020 = 0;
    syscall(__NR_ioctl, r[1], 0x5412, 0x20000000);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
