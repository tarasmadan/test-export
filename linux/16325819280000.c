// https://syzkaller.appspot.com/bug?id=cdaddfd3e9c92ee20436e8794d79b8032b4539cc
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
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

#define KMEMLEAK_FILE "/sys/kernel/debug/kmemleak"

static void setup_leak()
{
  if (!write_file(KMEMLEAK_FILE, "scan"))
    exit(1);
  sleep(5);
  if (!write_file(KMEMLEAK_FILE, "scan"))
    exit(1);
  if (!write_file(KMEMLEAK_FILE, "clear"))
    exit(1);
}

static void check_leaks(void)
{
  int fd = open(KMEMLEAK_FILE, O_RDWR);
  if (fd == -1)
    exit(1);
  uint64_t start = current_time_ms();
  if (write(fd, "scan", 4) != 4)
    exit(1);
  sleep(1);
  while (current_time_ms() - start < 4 * 1000)
    sleep(1);
  if (write(fd, "scan", 4) != 4)
    exit(1);
  static char buf[128 << 10];
  ssize_t n = read(fd, buf, sizeof(buf) - 1);
  if (n < 0)
    exit(1);
  int nleaks = 0;
  if (n != 0) {
    sleep(1);
    if (write(fd, "scan", 4) != 4)
      exit(1);
    if (lseek(fd, 0, SEEK_SET) < 0)
      exit(1);
    n = read(fd, buf, sizeof(buf) - 1);
    if (n < 0)
      exit(1);
    buf[n] = 0;
    char* pos = buf;
    char* end = buf + n;
    while (pos < end) {
      char* next = strstr(pos + 1, "unreferenced object");
      if (!next)
        next = end;
      char prev = *next;
      *next = 0;
      fprintf(stderr, "BUG: memory leak\n%s\n", pos);
      *next = prev;
      pos = next;
      nleaks++;
    }
  }
  if (write(fd, "clear", 5) != 5)
    exit(1);
  close(fd);
  if (nleaks)
    exit(1);
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
    check_leaks();
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  res = -1;
  res = syz_open_dev(0xc, 4, 1);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000040 = 2;
  *(uint32_t*)0x20000044 = 0;
  *(uint32_t*)0x20000048 = 0;
  *(uint32_t*)0x2000004c = 0;
  *(uint32_t*)0x20000050 = 0;
  *(uint64_t*)0x20000058 = 0;
  inject_fault(6);
  syscall(__NR_ioctl, r[0], 0x4b72, 0x20000040ul);
  res = -1;
  res = syz_open_dev(0xc, 4, 1);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20000040 = 0;
  *(uint32_t*)0x20000044 = 3;
  *(uint32_t*)0x20000048 = 0x1b;
  *(uint32_t*)0x2000004c = 0xd;
  *(uint32_t*)0x20000050 = 0x200;
  *(uint64_t*)0x20000058 = 0x20000080;
  memcpy(
      (void*)0x20000080,
      "\x0d\xe4\x73\x70\x56\x3e\xd4\x50\xe7\x4f\xba\x9e\xe1\x5c\xc0\xc3\xe9\xad"
      "\xc8\x5c\xaf\x8b\x84\x72\x24\x62\xad\x15\x24\xc6\x6b\xfb\x8e\x45\xba\x6e"
      "\x38\x74\xc6\x5b\x82\x9b\x1f\x1a\x23\x5b\x81\x00\x48\xfb\x05\x15\xce\xe6"
      "\x7c\xda\xf9\xae\xae\x59\x5c\x1e\x8e\xa1\xa6\x1d\x94\x24\x98\x1d\x3f\x26"
      "\xe4\x69\x9a\x20\x6b\xcd\xd0\xf8\xf5\x37\x45\x66\x88\xf3\xcd\xfc\x70\x45"
      "\xda\x32\x84\x2f\x9b\x8e\x41\x12\x7e\xd9\x9b\x2c\xfa\x30\xc0\x52\x1f\x50"
      "\xac\x45\xb1\x37\x69\x68\xc3\xce\xfe\xbb\x90\xe3\x45\x94\xfd\x8c\x1c\x76"
      "\x7c\x68\x84\x50\x44\xd3\x3e\xcc\x85\x38\x8e\x44\xfa\xdd\x6f\xf0\x4e\x3f"
      "\xf6\x46\xef\x3e\xa4\x81\x56\xec\x22\x08\x31\xe2\x04\x18\x1e\x64\xba\x8e"
      "\xd6\x60\xfb\x38\x49\xb9\x7c\xf8\xd3\xbc\x64\xc1\x71\x65\x52\x98\x8b\x6a"
      "\xbe\xb6\x81\x89\x0c\x0e\x7c\xa0\x07\x04\xa1\xc4\x64\x24\x3e\x28\x30\x55"
      "\xb0\xd9\x48\x78\xfa\xa0\xe1\x57\xbe\xde\xec\x96\x69\x6e\x32\xd5\xea\x55"
      "\xc2\xb8\xd7\x1d\x9c\x5e\xa9\x7e\xd4\x8e\x5b\x69\x0f\x2e\xb5\x80\x22\x72"
      "\x4c\x6b\x8d\x78\x2b\x67\x14\xa1\x35\x9f\xbd\x34\x4b\x7c\xde\xde\xdf\x7a"
      "\xd3\x7d\xda\xf7\x51\x0a\x0e\x4b\x91\xb2\x8f\x9d\x6f\x07\x28\x92\x71\x39"
      "\x2d\x00\x2d\x3e\x1a\x3b\xb3\xc8\x33\xa1\xe8\xff\xd0\x05\x3f\x68\x14\xb3"
      "\xff\x52\xe6\x04\x00\x00\x00\x00\x00\x00\x00\xcb\xd5\xa9\x76\xf1\x7a\x60"
      "\x0b\xa5\x41\xc4\x14\x8e\x9d\x4e\xb2\x7f\x8e\x5d\xa4\x1b\xee\x54\x0b\x02"
      "\x05\x99\x25\x4f\x3e\x27\x4d\x88\x34\xfe\xcc\x7f\x0a\xc2\xfa\x8c\x05\x00"
      "\x00\x00\xaf\x2c\x76\x52\x57\x44\x4a\x06\x00\x00\x00\x00\x00\x00\x00\x93"
      "\x72\x40\xae\x45\xd7\x8b\x49\xed\xad\xf0\xba\xb1\x5d\x48\xa0\xc0\x13\xff"
      "\x92\x44\xc3\x19\x6f\xe8\xf6\x38\x38\x92\xdf\xe6\xf9\x1a\x41\xc7\x74\x60"
      "\x22\x47\xaa\xe6\x6c\x10\x29\xcd\x3e\xd3\x7c\xd8\xeb\x12\xa6\x24\xb0\x4f"
      "\xf6\x93\x7c\xa7\xdd\x1f\x2c\xd9\x9a\x8e\xc0\xf0\x3d\x41\x6f\x40\x13\x97"
      "\xca\x9c\x82\xe2\x4f\xfa\x9d\x94\x62\xf0\x05\x00\xbc\xc7\xf0\x68\xa9\x38"
      "\xba\x40\x2d\x65\x5e\x96\x2c\xc7\xa9\x96\x95\xdc\xc9\xb2\xa8\x8d\x46\xdd"
      "\xb9\x6c\x2a\xb9\x5c\xdb\x41\xfb\x79\xe4\x18\x1f\x08\x79\x6a\xd3\x40\x4b"
      "\xd1\xee\x43\xbc\xdd\x48\xba\xeb\x38\xaf\x36\xfb\x70\x20\x1c\x7b\x72\x19"
      "\xe6\x4f\x2d\x92\x03\x7c\x9a\x0f\x42\x0a\x9f\x72\x53\xf3\x5e\x8b\x2d\x2c"
      "\x52\x8b\x76\x69\x35\xed\x69\x7e\x00\x27\xbf\x6e\xe6\x0d\x60\xd1\x57\xab"
      "\x1a\x41\x53\xf9\x55\x5b\x22\xf2\xab\x3b\x61\x40\xfa\x15\xbb\x8f\x05\x9e"
      "\xfe\x6e\xb9\x37\x1d\x47\x94\x6a\x99\xdb\x68\x19\x23\xe9\xb1\x51\x13\x87"
      "\xe2\xb8\x77\xcf\x12\x7d\x98\xc0\x95\x65\x22\xd9\x1c\x4d\x86\x1e\xeb\x3b"
      "\xbb\x4f\x45\xd2\xae\x36\x7b\xa7\xf7\xd4\xa9\x04\xa6\x43\x94\x26\xfb\x02"
      "\xaa\xb5\x84\xa0\x5a\x1e\xe7\x34\xaf\xa4\xc1\x5f\x38\xb5\xb6\x72\x17\x86"
      "\x3c\xd7\xf9\x40\xf2\x1d\x76\x7f\x77\x98\xdf\x55\x26\x03\x7f\x17\x59\x68"
      "\xac\xdd\xfa\x42\xb3\x77\xb7\x65\x06\xd4\x26\x2f\xae\x4d\x68\xe1\xab\x84"
      "\xf2\x79\xf1\xb0\x90\x60\xc0\xec\x58\x48\x02\x05\xeb\xa6\x1e\xf9\x59\x25"
      "\x45\x36\x7a\x97\x8d\x1e\x19\x3c\xe8\xe5\xff\x8f\xe6\x81\xa3\x20\xbc\x84"
      "\x00\xc1\xce\xd7\x9a\x87\xfa\x36\x23\x55\x9c\x3b\xae\x1e\x96\xb8\x8e\xb6"
      "\xf2\x18\xb5\x83\x96\xa0\x53\x75\x5b\x9c\x60\xf4\x60\x5f\x34\x73\xef\x80"
      "\x03\xb1\xb2\x8c\xbd\x43\xa4\xab\xa8\x7e\x4e\xad\x0c\x84\xc4\x1a\xf5\x05"
      "\x00\x00\x00\x8a\xef\x3e\x16\x53\x03\x56\x67\xa1\xd9\x37\x72\x60\x4f\x4c"
      "\x9b\x6b\x59\x2c\x0e\x83\x10\x6a\x63\x00\x9e\x5c\x9c\x54\xeb\x56\x08\x26"
      "\xbe\x1f\x00\x00\x00\x00\x00\x00\x00\x62\x65\x89\xd8\x72\xce\xad\xae\xac"
      "\x87\x8e\x2a\xe5\x66\xbc\x2a\x6c\x67\x39\xdc\x0c\xf0\x88\x79\x56\x3b\x96"
      "\xfe\xe0\x03\x5c\x22\xd3\x73\x6a\x07\xb3\xb0\xed\xa0\xe8\x27\xd7\x5b\x06"
      "\xfa\x7a\xc4\x39\x27\xef\x83\x83\xe7\xcc\xad\x06\x64\xf6\x63\x2e\x17\x4b"
      "\x99\x47\x40\x55\xe5\x86\x73\xc6\x53\x04\x24\x97\x15\x9a\x91\x14\x2f\x47"
      "\x28\x78\x95\x10\xbd\x2e\x94\x4d\x51\x07\x4b\x58\x34\x2c\xe8\xbc\x3d\x3f"
      "\xd4\xdc\x4b\xe0\x4a\x3f\x1c\xcd\xc7\x92\xf1\x42\x70\x3c\xfe\x1c\x2b\x10"
      "\x4c\x7d\x62\xde\xc4\xba\x9e\x0c\xb9\xcb\x1c\xa9\xae\x9a\x58\xb7\x89\xfa"
      "\x93\x07\x76\x15\x51\x74\x7a\xc4\xe6\x89\x01\x07\xf8\x94\x1c\xa6\x48\x6c"
      "\xb4\x43\xab\xb0\xc7\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
      1024);
  syscall(__NR_ioctl, r[1], 0x4b72, 0x20000040ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  setup_leak();
  setup_fault();
  loop();
  return 0;
}
