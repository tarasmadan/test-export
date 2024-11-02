// https://syzkaller.appspot.com/bug?id=56038cbafead8d07c79de8fd880471ae8114a838
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

uint64_t r[4] = {0x0, 0xffffffffffffffff, 0xffffffffffffffff, 0x0};

void execute_one(void)
{
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  memcpy((void*)0x20000000, "logon\000", 6);
  memcpy((void*)0x20000040, "fscrypt:", 8);
  memcpy((void*)0x20000048, "0000111122223333", 16);
  *(uint8_t*)0x20000058 = 0;
  *(uint32_t*)0x20000080 = 0;
  memcpy((void*)0x20000084,
         "\x5d\x9b\xc1\x36\xc9\x63\x25\x4c\x66\x1f\xb6\x20\x14\x8b\x6f\x72\xca"
         "\x6a\xe2\xa4\x48\x29\xbf\xa7\x9e\xc1\x34\x99\xf8\xec\x90\x77\xd8\x5d"
         "\x87\x97\x11\xd9\x8b\xb1\x68\x7a\xd3\x6d\xfe\x5f\x14\xa7\xb0\xce\x15"
         "\xc1\xe6\xbe\x0e\x7e\xca\xbf\xdf\xde\x0d\xfa\x00\xb1",
         64);
  *(uint32_t*)0x200000c4 = 0;
  res = syscall(__NR_add_key, /*type=*/0x20000000ul, /*desc=*/0x20000040ul,
                /*payload=*/0x20000080ul, /*paylen=*/0x48ul, /*keyring=*/-1);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_pipe2, /*pipefd=*/0x200002c0ul, /*flags=*/0x80ul);
  if (res != -1)
    r[1] = *(uint32_t*)0x200002c4;
  res = syscall(__NR_socket, /*domain=*/1ul, /*type=SOCK_DGRAM*/ 2ul,
                /*proto=*/0);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x20cab000 = 0xc;
  res = syscall(__NR_getsockopt, /*fd=*/r[2], /*level=*/1, /*optname=*/0x11,
                /*optval=*/0x20caaffbul, /*optlen=*/0x20cab000ul);
  if (res != -1)
    r[3] = *(uint32_t*)0x20caafff;
  syscall(__NR_setresuid, /*ruid=*/0xee01, /*euid=*/r[3], /*suid=*/-1);
  syscall(__NR_keyctl, /*code=*/0x20ul, /*id=*/r[0], /*watch_queue_fd=*/r[1],
          /*watch_id=*/0ul, 0);
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