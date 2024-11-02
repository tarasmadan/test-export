// https://syzkaller.appspot.com/bug?id=2d8aaf46f34de38f570d620644c55be9aa5da103
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

#ifndef __NR_bpf
#define __NR_bpf 280
#endif
#ifndef __NR_ioctl
#define __NR_ioctl 29
#endif
#ifndef __NR_memfd_create
#define __NR_memfd_create 279
#endif
#ifndef __NR_mmap
#define __NR_mmap 222
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

static long syz_open_procfs(volatile long a0, volatile long a1)
{
  char buf[128];
  memset(buf, 0, sizeof(buf));
  if (a0 == 0) {
    snprintf(buf, sizeof(buf), "/proc/self/%s", (char*)a1);
  } else if (a0 == -1) {
    snprintf(buf, sizeof(buf), "/proc/thread-self/%s", (char*)a1);
  } else {
    snprintf(buf, sizeof(buf), "/proc/self/task/%d/%s", (int)a0, (char*)a1);
  }
  int fd = open(buf, O_RDWR);
  if (fd == -1)
    fd = open(buf, O_RDONLY);
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
      if (current_time_ms() - start < 15000)
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
  memcpy((void*)0x20000140,
         "y\0205\373\367u\203%:r\302\271x\244q\301\352_"
         "\214Z7\347a\233\021x\016\241\317\032\230S7\311\000\000\000\000\000"
         "\000\a\000\000\000\000\000\000\004\2079\2424\251am\336\262\323\313ZJo"
         "a\304\032cB\252\301\373 "
         "Q\324\364\001\2452\342DG\324\275{\237\251\227\233@"
         "\333\000b\341br\266\3527\343\020\377\302\235\r2\236\216\004sW\033\267"
         "\263\242\311&@\312\332\334\342/"
         "\227X\254\b\260\302<"
         "\200E\032\274\307W\3329VsA\257\306\317\341\241\265M\242\205\246y\304J"
         "\361\367\374D\225\343\353\307\274\221\260\250\236o\353F("
         "\235L\001vRk\252cB\004\247I\v\206EZ\226\325\024OD\\\350R\344\315\354"
         "\314\321\017re\3506\315\353\304$\230\006J\326dD\215_U`ji{"
         "\253\227\257;l\037\257\2638U\313\372\263j\222\f\201\240\242-"
         "g\b\231\016\215\215\026\331w\\\370\316\260j\235\'\223\357\035\240H"
         "\315\275\331\257\022$\215\026%\213\000",
         272);
  syscall(__NR_memfd_create, /*name=*/0x20000140ul, /*flags=*/0ul);
  *(uint32_t*)0x200000c0 = 0x1f;
  *(uint32_t*)0x200000c4 = 0;
  *(uint32_t*)0x200000c8 = 0;
  *(uint32_t*)0x200000cc = 0x1000;
  *(uint32_t*)0x200000d0 = 0;
  *(uint32_t*)0x200000d4 = 1;
  *(uint32_t*)0x200000d8 = 0;
  memset((void*)0x200000dc, 0, 16);
  *(uint32_t*)0x200000ec = 0;
  *(uint32_t*)0x200000f0 = -1;
  *(uint32_t*)0x200000f4 = 0;
  *(uint32_t*)0x200000f8 = 0;
  *(uint32_t*)0x200000fc = 0;
  *(uint64_t*)0x20000100 = 0;
  *(uint32_t*)0x20000108 = 0;
  *(uint32_t*)0x2000010c = 0;
  res = syscall(__NR_bpf, /*cmd=*/0ul, /*arg=*/0x200000c0ul, /*size=*/0x48ul);
  if (res != -1)
    r[0] = res;
  syscall(__NR_mmap, /*addr=*/0x20ffd000ul, /*len=*/0x3000ul,
          /*prot=PROT_GROWSDOWN|PROT_SEM|PROT_WRITE|PROT_READ|PROT_EXEC*/
          0x100000ful,
          /*flags=MAP_STACK|MAP_POPULATE|MAP_FIXED|MAP_SHARED*/ 0x28011ul,
          /*fd=*/r[0], /*offset=*/0x1000ul);
  memcpy((void*)0x200001c0, "maps\000", 5);
  res = -1;
  res = syz_open_procfs(/*pid=*/0, /*file=*/0x200001c0);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20000180 = 0x68;
  *(uint32_t*)0x20000184 = 0;
  *(uint64_t*)0x20000188 = 0x3f;
  *(uint64_t*)0x20000190 = 0x2000;
  *(uint64_t*)0x20000198 = 0x20ffe000;
  syscall(__NR_ioctl, /*fd=*/r[1], /*cmd=*/0xc0686611, /*arg=*/0x20000180ul);
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
  loop();
  return 0;
}