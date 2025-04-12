// https://syzkaller.appspot.com/bug?id=2e7ea317bcd565c49f885637ad9697943cadf125
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
      sleep_ms(10);
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      if (current_time_ms() - start < 5000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  memcpy((void*)0x200000000080, "/proc/thread-self/net/afs/rootcell\000", 35);
  res = syscall(
      __NR_openat, /*fd=*/0xffffffffffffff9cul, /*file=*/0x200000000080ul,
      /*flags=O_NOATIME|O_LARGEFILE|O_CREAT|O_WRONLY*/ 0x48041, /*mode=*/0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x200000000000 = r[0];
  *(uint16_t*)0x200000000004 = 0x435;
  *(uint16_t*)0x200000000006 = 2;
  res = syscall(__NR_poll, /*ufds=*/0x200000000000ul, /*nfds=*/0,
                /*timeout_msecs=*/3);
  if (res != -1)
    r[1] = *(uint32_t*)0x200000000000;
  memcpy(
      (void*)0x200000000200,
      "\x2c\xdd\xe0\x2c\x4a\x23\xd0\xbf\xa2\xf1\x42\xfd\x75\xfa\xff\x3d\x3a\xf6"
      "\x4f\xae\x34\x92\xbe\x43\x93\xa7\x63\x6f\x05\xa6\xc6\x97\x16\x25\xde\x4c"
      "\x84\x2b\x45\xf7\x29\x10\x47\x4c\xe6\x39\x31\x01\x35\xb9\xa0\x25\x84\xe8"
      "\xbb\x6e\x95\xde\x80\xed\x32\x51\x2b\x49\x6d\xf8\x05\x3e\x2b\x6c\x43\x73"
      "\xd9\xca\x8a\xd0\xd0\x80\xf7\x79\xf9\xbe\x74\x95\x40\xb8\x4b\xa9\x3a\xb9"
      "\x7c\xf8\x7f\xcb\x5e\x56\x65\x60\x76\xdf\x74\x58\xcc\xa8\x73\x39\x30\x7d"
      "\xba\x53\x97\xc8\x90\xb3\xe0\xc2\xea\xb6\x13\xb7\x8b\x0c\x29\x41\xfd\xb0"
      "\x94\x46\x87\x6a\x37\x5c\xd4\xdb\xb3\xd0\x9f\xa9\x59\x51\x4f\x82\xfa\x3e"
      "\xdd\xee\x04\xa5\x06\x7d\x42\x40\x36\x57\xd1\x0b\x2c\x37",
      158);
  syscall(__NR_write, /*fd=*/r[1], /*buf=*/0x200000000200ul, /*len=*/0x9eul);
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffffffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200000000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200001000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  const char* reason;
  (void)reason;
  loop();
  return 0;
}
