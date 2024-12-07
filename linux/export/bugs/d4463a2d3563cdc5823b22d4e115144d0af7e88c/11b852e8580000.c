// https://syzkaller.appspot.com/bug?id=d4463a2d3563cdc5823b22d4e115144d0af7e88c
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

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0,
                 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  res = syscall(__NR_socket, /*domain=*/0x10ul, /*type=*/3ul, /*proto=*/0x10);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000100 = 0;
  *(uint32_t*)0x20000108 = 2;
  *(uint64_t*)0x20000110 = 0x20000000;
  *(uint64_t*)0x20000000 = 0x200001c0;
  *(uint32_t*)0x200001c0 = 0x14;
  *(uint16_t*)0x200001c4 = 0x24;
  *(uint16_t*)0x200001c6 = 9;
  *(uint32_t*)0x200001c8 = 0x3000000;
  *(uint32_t*)0x200001cc = 0;
  *(uint8_t*)0x200001d0 = 6;
  *(uint8_t*)0x200001d1 = 0;
  *(uint16_t*)0x200001d2 = 0;
  *(uint64_t*)0x20000008 = 0x14;
  *(uint64_t*)0x20000118 = 1;
  *(uint64_t*)0x20000120 = 0;
  *(uint64_t*)0x20000128 = 0;
  *(uint32_t*)0x20000130 = 0;
  syscall(__NR_sendmsg, /*fd=*/r[0], /*msg=*/0x20000100ul, /*f=*/0ul);
  res = syscall(__NR_socket, /*domain=AF_QIPCRTR*/ 0x2aul,
                /*type=SOCK_DGRAM*/ 2ul, /*proto=*/0);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20001480 = 0x14;
  res = syscall(__NR_getsockname, /*fd=*/r[1], /*addr=*/0x20000200ul,
                /*addrlen=*/0x20001480ul);
  if (res != -1)
    r[2] = *(uint32_t*)0x20000204;
  *(uint64_t*)0x200002c0 = 0;
  *(uint32_t*)0x200002c8 = 0;
  *(uint64_t*)0x200002d0 = 0x20000280;
  *(uint64_t*)0x20000280 = 0x20000540;
  *(uint32_t*)0x20000540 = 0x2c;
  *(uint16_t*)0x20000544 = 0x24;
  *(uint16_t*)0x20000546 = 0xf0b;
  *(uint32_t*)0x20000548 = 0;
  *(uint32_t*)0x2000054c = 0;
  *(uint8_t*)0x20000550 = 0;
  *(uint8_t*)0x20000551 = 0;
  *(uint16_t*)0x20000552 = 0;
  *(uint32_t*)0x20000554 = r[2];
  *(uint16_t*)0x20000558 = 0;
  *(uint16_t*)0x2000055a = 0;
  *(uint16_t*)0x2000055c = -1;
  *(uint16_t*)0x2000055e = -1;
  *(uint16_t*)0x20000560 = 0;
  *(uint16_t*)0x20000562 = 0;
  *(uint16_t*)0x20000564 = 8;
  *(uint16_t*)0x20000566 = 1;
  memcpy((void*)0x20000568, "drr\000", 4);
  *(uint64_t*)0x20000288 = 0x2c;
  *(uint64_t*)0x200002d8 = 1;
  *(uint64_t*)0x200002e0 = 0;
  *(uint64_t*)0x200002e8 = 0;
  *(uint32_t*)0x200002f0 = 0;
  syscall(__NR_sendmsg, /*fd=*/-1, /*msg=*/0x200002c0ul, /*f=*/0ul);
  res = syscall(__NR_socket, /*domain=*/0x10ul, /*type=*/3ul, /*proto=*/0);
  if (res != -1)
    r[3] = res;
  syscall(__NR_sendmmsg, /*fd=*/r[3], /*mmsg=*/0x200002c0ul,
          /*vlen=*/0x40000000000009ful, /*f=*/0ul);
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
  const char* reason;
  (void)reason;
  for (procid = 0; procid < 5; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
