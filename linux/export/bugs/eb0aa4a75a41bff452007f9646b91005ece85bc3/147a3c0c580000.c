// https://syzkaller.appspot.com/bug?id=eb0aa4a75a41bff452007f9646b91005ece85bc3
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

#ifndef __NR_ioctl
#define __NR_ioctl 29
#endif
#ifndef __NR_mmap
#define __NR_mmap 222
#endif
#ifndef __NR_prlimit64
#define __NR_prlimit64 261
#endif
#ifndef __NR_sched_setscheduler
#define __NR_sched_setscheduler 119
#endif
#ifndef __NR_socket
#define __NR_socket 198
#endif
#ifndef __NR_userfaultfd
#define __NR_userfaultfd 282
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

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  syscall(
      __NR_mmap, /*addr=*/0x20400000ul, /*len=*/0xc00000ul,
      /*prot=PROT_GROWSUP|PROT_WRITE|PROT_READ|PROT_EXEC*/ 0x2000007ul,
      /*flags=MAP_UNINITIALIZED|MAP_POPULATE|MAP_NORESERVE|MAP_NONBLOCK|MAP_FIXED|0x1021*/
      0x401d031ul, /*fd=*/(intptr_t)-1, /*offset=*/0ul);
  res = syscall(__NR_userfaultfd, /*flags=UFFD_USER_MODE_ONLY*/ 1ul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000000 = 0xaa;
  *(uint64_t*)0x20000008 = 0;
  *(uint64_t*)0x20000010 = 0;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0xc018aa3f, /*arg=*/0x20000000ul);
  *(uint64_t*)0x200000c0 = 0x20800000;
  *(uint64_t*)0x200000c8 = 0x800000;
  *(uint64_t*)0x200000d0 = 5;
  *(uint64_t*)0x200000d8 = 0;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0xc020aa00, /*arg=*/0x200000c0ul);
  syscall(__NR_prlimit64, /*pid=*/0, /*res=RLIMIT_RTPRIO*/ 0xeul, /*new=*/0ul,
          /*old=*/0ul);
  syscall(__NR_sched_setscheduler, /*pid=*/0, /*policy=SCHED_FIFO*/ 1ul,
          /*prio=*/0ul);
  syscall(
      __NR_mmap, /*addr=*/0x20000000ul, /*len=*/0xb36000ul,
      /*prot=PROT_GROWSUP|PROT_SEM|PROT_WRITE|PROT_READ|PROT_EXEC|0xb635773f04ebbee0*/
      0xb635773f06ebbeeful,
      /*flags=MAP_POPULATE|MAP_FIXED|MAP_ANONYMOUS|0x1*/ 0x8031ul,
      /*fd=*/(intptr_t)-1, /*offset=*/0ul);
  syscall(__NR_socket, /*domain=*/2ul,
          /*type=SOCK_STREAM|0x4000000000000000*/ 0x4000000000000001ul,
          /*proto=*/0);
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul,
          /*fd=*/(intptr_t)-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul,
          /*fd=*/(intptr_t)-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul,
          /*fd=*/(intptr_t)-1, /*offset=*/0ul);
  const char* reason;
  (void)reason;
  loop();
  return 0;
}
