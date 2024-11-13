// https://syzkaller.appspot.com/bug?id=33be1b73bbecb4b62186627650e6ce0965465ba4
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

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

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
  int iter = 0;
  DIR* dp = 0;
retry:
  while (umount2(dir, MNT_DETACH | UMOUNT_NOFOLLOW) == 0) {
  }
  dp = opendir(dir);
  if (dp == NULL) {
    if (errno == EMFILE) {
      exit(1);
    }
    exit(1);
  }
  struct dirent* ep = 0;
  while ((ep = readdir(dp))) {
    if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
      continue;
    char filename[FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", dir, ep->d_name);
    while (umount2(filename, MNT_DETACH | UMOUNT_NOFOLLOW) == 0) {
    }
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
      if (umount2(filename, MNT_DETACH | UMOUNT_NOFOLLOW))
        exit(1);
    }
  }
  closedir(dp);
  for (int i = 0;; i++) {
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
        if (umount2(dir, MNT_DETACH | UMOUNT_NOFOLLOW))
          exit(1);
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
  if (symlink("/dev/binderfs", "./binderfs")) {
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

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter = 0;
  for (;; iter++) {
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
      if (current_time_ms() - start < 5000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
    remove_dir(cwdbuf);
  }
}

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

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
    *(uint32_t*)0x200000c0 = 6;
    *(uint32_t*)0x200000c4 = 0xc;
    *(uint64_t*)0x200000c8 = 0x20000000;
    *(uint8_t*)0x20000000 = 0x18;
    STORE_BY_BITMASK(uint8_t, , 0x20000001, 0, 0, 4);
    STORE_BY_BITMASK(uint8_t, , 0x20000001, 0, 4, 4);
    *(uint16_t*)0x20000002 = 0;
    *(uint32_t*)0x20000004 = 0;
    *(uint8_t*)0x20000008 = 0;
    *(uint8_t*)0x20000009 = 0;
    *(uint16_t*)0x2000000a = 0;
    *(uint32_t*)0x2000000c = 0;
    *(uint8_t*)0x20000010 = 0x18;
    STORE_BY_BITMASK(uint8_t, , 0x20000011, 1, 0, 4);
    STORE_BY_BITMASK(uint8_t, , 0x20000011, 1, 4, 4);
    *(uint16_t*)0x20000012 = 0;
    *(uint32_t*)0x20000014 = r[0];
    *(uint8_t*)0x20000018 = 0;
    *(uint8_t*)0x20000019 = 0;
    *(uint16_t*)0x2000001a = 0;
    *(uint32_t*)0x2000001c = 0;
    STORE_BY_BITMASK(uint8_t, , 0x20000020, 7, 0, 3);
    STORE_BY_BITMASK(uint8_t, , 0x20000020, 0, 3, 1);
    STORE_BY_BITMASK(uint8_t, , 0x20000020, 0xb, 4, 4);
    STORE_BY_BITMASK(uint8_t, , 0x20000021, 8, 0, 4);
    STORE_BY_BITMASK(uint8_t, , 0x20000021, 0, 4, 4);
    *(uint16_t*)0x20000022 = 0;
    *(uint32_t*)0x20000024 = 0;
    STORE_BY_BITMASK(uint8_t, , 0x20000028, 3, 0, 3);
    STORE_BY_BITMASK(uint8_t, , 0x20000028, 3, 3, 2);
    STORE_BY_BITMASK(uint8_t, , 0x20000028, 3, 5, 3);
    STORE_BY_BITMASK(uint8_t, , 0x20000029, 0xa, 0, 4);
    STORE_BY_BITMASK(uint8_t, , 0x20000029, 8, 4, 4);
    *(uint16_t*)0x2000002a = 0xfff8;
    *(uint32_t*)0x2000002c = 0;
    STORE_BY_BITMASK(uint8_t, , 0x20000030, 4, 0, 3);
    STORE_BY_BITMASK(uint8_t, , 0x20000030, 1, 3, 1);
    STORE_BY_BITMASK(uint8_t, , 0x20000030, 0xb, 4, 4);
    STORE_BY_BITMASK(uint8_t, , 0x20000031, 2, 0, 4);
    STORE_BY_BITMASK(uint8_t, , 0x20000031, 0xa, 4, 4);
    *(uint16_t*)0x20000032 = 0;
    *(uint32_t*)0x20000034 = 0;
    STORE_BY_BITMASK(uint8_t, , 0x20000038, 6, 0, 3);
    STORE_BY_BITMASK(uint8_t, , 0x20000038, 0, 3, 1);
    STORE_BY_BITMASK(uint8_t, , 0x20000038, 0xa, 4, 4);
    STORE_BY_BITMASK(uint8_t, , 0x20000039, 2, 0, 4);
    STORE_BY_BITMASK(uint8_t, , 0x20000039, 0, 4, 4);
    *(uint16_t*)0x2000003a = 0;
    *(uint32_t*)0x2000003c = 0xfffffff8;
    STORE_BY_BITMASK(uint8_t, , 0x20000040, 7, 0, 3);
    STORE_BY_BITMASK(uint8_t, , 0x20000040, 0, 3, 1);
    STORE_BY_BITMASK(uint8_t, , 0x20000040, 0xb, 4, 4);
    STORE_BY_BITMASK(uint8_t, , 0x20000041, 3, 0, 4);
    STORE_BY_BITMASK(uint8_t, , 0x20000041, 0, 4, 4);
    *(uint16_t*)0x20000042 = 0;
    *(uint32_t*)0x20000044 = 8;
    STORE_BY_BITMASK(uint8_t, , 0x20000048, 7, 0, 3);
    STORE_BY_BITMASK(uint8_t, , 0x20000048, 0, 3, 1);
    STORE_BY_BITMASK(uint8_t, , 0x20000048, 0xb, 4, 4);
    STORE_BY_BITMASK(uint8_t, , 0x20000049, 4, 0, 4);
    STORE_BY_BITMASK(uint8_t, , 0x20000049, 0, 4, 4);
    *(uint16_t*)0x2000004a = 0;
    *(uint32_t*)0x2000004c = 0;
    *(uint8_t*)0x20000050 = 0x85;
    *(uint8_t*)0x20000051 = 0;
    *(uint16_t*)0x20000052 = 0;
    *(uint32_t*)0x20000054 = 0x33;
    *(uint8_t*)0x20000058 = 0x95;
    *(uint8_t*)0x20000059 = 0;
    *(uint16_t*)0x2000005a = 0;
    *(uint32_t*)0x2000005c = 0;
    *(uint64_t*)0x200000d0 = 0x20000180;
    memcpy((void*)0x20000180, "GPL\000", 4);
    *(uint32_t*)0x200000d8 = 0;
    *(uint32_t*)0x200000dc = 0;
    *(uint64_t*)0x200000e0 = 0;
    *(uint32_t*)0x200000e8 = 0;
    *(uint32_t*)0x200000ec = 0;
    memset((void*)0x200000f0, 0, 16);
    *(uint32_t*)0x20000100 = 0;
    *(uint32_t*)0x20000104 = 0;
    *(uint32_t*)0x20000108 = -1;
    *(uint32_t*)0x2000010c = 0;
    *(uint64_t*)0x20000110 = 0;
    *(uint32_t*)0x20000118 = 0;
    *(uint32_t*)0x2000011c = 0;
    *(uint64_t*)0x20000120 = 0;
    *(uint32_t*)0x20000128 = 0;
    *(uint32_t*)0x2000012c = 0;
    *(uint32_t*)0x20000130 = 0;
    *(uint32_t*)0x20000134 = 0;
    *(uint64_t*)0x20000138 = 0;
    *(uint64_t*)0x20000140 = 0;
    *(uint32_t*)0x20000148 = 0;
    *(uint32_t*)0x2000014c = 0;
    res = syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x200000c0ul, /*size=*/0x90ul);
    if (res != -1)
      r[1] = res;
    break;
  case 2:
    *(uint32_t*)0x20000500 = r[1];
    *(uint32_t*)0x20000504 = -1;
    *(uint32_t*)0x20000508 = 0;
    syscall(__NR_bpf, /*cmd=*/0xaul, /*arg=*/0x20000500ul, /*size=*/0xcul);
    break;
  case 3:
    *(uint32_t*)0x200000c0 = 6;
    *(uint32_t*)0x200000c4 = 0xb;
    *(uint64_t*)0x200000c8 = 0x200001c0;
    memcpy(
        (void*)0x200001c0,
        "\x18\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x18"
        "\x01\x00\x00\x78\x6c\x6c\x35\x00\x00\x00\x00\x00\x20\x20\x20\x7b\x1a"
        "\xf8\xff\x00\x00\x00\x00\xbf\xa1\x00\x00\x00\x00\x00\x00\x07\x01\x00"
        "\x00\xf8\xff\xff\xff\xb7\x02\x00\x00\x08\x00\x00\x00\xb7\x03\x00\x00"
        "\x00\x00\x00\x00\x85\x00\x00\x00\x06\x00\x00\x00\x95",
        81);
    *(uint64_t*)0x200000d0 = 0x20000000;
    memcpy((void*)0x20000000, "GPL\000", 4);
    *(uint32_t*)0x200000d8 = 0;
    *(uint32_t*)0x200000dc = 0;
    *(uint64_t*)0x200000e0 = 0;
    *(uint32_t*)0x200000e8 = 0;
    *(uint32_t*)0x200000ec = 0;
    memset((void*)0x200000f0, 0, 16);
    *(uint32_t*)0x20000100 = 0;
    *(uint32_t*)0x20000104 = 0;
    *(uint32_t*)0x20000108 = -1;
    *(uint32_t*)0x2000010c = 0;
    *(uint64_t*)0x20000110 = 0;
    *(uint32_t*)0x20000118 = 0;
    *(uint32_t*)0x2000011c = 0;
    *(uint64_t*)0x20000120 = 0;
    *(uint32_t*)0x20000128 = 0;
    *(uint32_t*)0x2000012c = 0;
    *(uint32_t*)0x20000130 = 0;
    *(uint32_t*)0x20000134 = 0;
    *(uint64_t*)0x20000138 = 0;
    *(uint64_t*)0x20000140 = 0;
    *(uint32_t*)0x20000148 = 0;
    *(uint32_t*)0x2000014c = 0;
    res = syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x200000c0ul, /*size=*/0x90ul);
    if (res != -1)
      r[2] = res;
    break;
  case 4:
    *(uint32_t*)0x20000240 = r[2];
    *(uint32_t*)0x20000244 = 0x23000002;
    *(uint32_t*)0x20000248 = 0;
    *(uint32_t*)0x2000024c = 0x2000000;
    *(uint64_t*)0x20000250 = 0;
    *(uint64_t*)0x20000258 = 0;
    *(uint32_t*)0x20000260 = 0;
    *(uint32_t*)0x20000264 = 0;
    *(uint32_t*)0x20000268 = 0;
    *(uint32_t*)0x2000026c = 0;
    *(uint64_t*)0x20000270 = 0;
    *(uint64_t*)0x20000278 = 0;
    *(uint32_t*)0x20000280 = 2;
    *(uint32_t*)0x20000284 = 0;
    *(uint32_t*)0x20000288 = 0;
    syscall(__NR_bpf, /*cmd=*/0xaul, /*arg=*/0x20000240ul, /*size=*/0x50ul);
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
  use_temporary_dir();
  loop();
  return 0;
}