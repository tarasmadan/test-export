// https://syzkaller.appspot.com/bug?id=4a0cc45ea1628b55671583457d498d3eb0af1376
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  memcpy((void*)0x400000000000, "/dev/vmci\000", 10);
  res = syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul,
                /*file=*/0x400000000000ul, /*flags=*/2, /*mode=*/0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x400000000200 = 0xa0000;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x7a7, /*arg=*/0x400000000200ul);
  *(uint32_t*)0x400000000280 = 1;
  *(uint32_t*)0x400000000284 = 0;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x7a0, /*arg=*/0x400000000280ul);
  *(uint32_t*)0x400000001340 = 1;
  *(uint32_t*)0x400000001344 = 0;
  *(uint32_t*)0x400000001348 = -1;
  *(uint32_t*)0x40000000134c = 0;
  *(uint64_t*)0x400000001350 = 4;
  *(uint64_t*)0x400000001358 = 0;
  *(uint64_t*)0x400000001360 = 0;
  *(uint64_t*)0x400000001368 = 0;
  *(uint32_t*)0x400000001370 = 0;
  *(uint32_t*)0x400000001374 = 0;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x7a8, /*arg=*/0x400000001340ul);
  res = syscall(__NR_socket, /*domain=*/0xaul, /*type=SOCK_RAW*/ 3ul,
                /*proto=*/0x3c);
  if (res != -1)
    r[1] = res;
  *(uint16_t*)0x400000000080 = 0xa;
  *(uint16_t*)0x400000000082 = htobe16(3);
  *(uint32_t*)0x400000000084 = htobe32(8);
  *(uint8_t*)0x400000000088 = 0xfe;
  *(uint8_t*)0x400000000089 = 0x80;
  memset((void*)0x40000000008a, 0, 13);
  *(uint8_t*)0x400000000097 = 0xbb;
  *(uint32_t*)0x400000000098 = 7;
  syscall(__NR_connect, /*fd=*/r[1], /*addr=*/0x400000000080ul,
          /*addrlen=*/0x1cul);
  *(uint64_t*)0x4000000000c0 = 0;
  *(uint32_t*)0x4000000000c8 = 0x953c;
  *(uint64_t*)0x4000000000d0 = 0x400000000100;
  *(uint64_t*)0x400000000100 = 0x400000000000;
  memcpy((void*)0x400000000000, "\x2b\x10", 2);
  *(uint64_t*)0x400000000108 = 0xffbd;
  *(uint64_t*)0x4000000000d8 = 1;
  *(uint64_t*)0x4000000000e0 = 0;
  *(uint64_t*)0x4000000000e8 = 0;
  *(uint32_t*)0x4000000000f0 = 0x2c;
  syscall(__NR_sendmsg, /*fd=*/r[1], /*msg=*/0x4000000000c0ul,
          /*f=MSG_DONTROUTE*/ 4ul);
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x3ffffffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x400000000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x400001000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
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
