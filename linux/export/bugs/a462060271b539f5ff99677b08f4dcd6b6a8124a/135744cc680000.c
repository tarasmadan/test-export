// https://syzkaller.appspot.com/bug?id=a462060271b539f5ff99677b08f4dcd6b6a8124a
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

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

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
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      sleep_ms(1);
      if (current_time_ms() - start < 5000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[3] = {0xffffffffffffffff, 0x0, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  *(uint32_t*)0x200008c0 = 3;
  *(uint32_t*)0x200008c4 = 3;
  *(uint64_t*)0x200008c8 = 0x20000040;
  memcpy((void*)0x20000040,
         "\x18\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x95"
         "\x00\x20\x00\x00\x00\x00\x00",
         24);
  *(uint64_t*)0x200008d0 = 0x20000700;
  memcpy((void*)0x20000700, "GPL\000", 4);
  *(uint32_t*)0x200008d8 = 0;
  *(uint32_t*)0x200008dc = 0;
  *(uint64_t*)0x200008e0 = 0;
  *(uint32_t*)0x200008e8 = 0;
  *(uint32_t*)0x200008ec = 0;
  memset((void*)0x200008f0, 0, 16);
  *(uint32_t*)0x20000900 = 0;
  *(uint32_t*)0x20000904 = 0x25;
  *(uint32_t*)0x20000908 = -1;
  *(uint32_t*)0x2000090c = 8;
  *(uint64_t*)0x20000910 = 0;
  *(uint32_t*)0x20000918 = 0;
  *(uint32_t*)0x2000091c = 0x10;
  *(uint64_t*)0x20000920 = 0;
  *(uint32_t*)0x20000928 = 0;
  *(uint32_t*)0x2000092c = 0;
  *(uint32_t*)0x20000930 = 0;
  *(uint32_t*)0x20000934 = 0;
  *(uint64_t*)0x20000938 = 0;
  *(uint64_t*)0x20000940 = 0;
  *(uint32_t*)0x20000948 = 0x10;
  *(uint32_t*)0x2000094c = 0;
  res = syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x200008c0ul, /*size=*/0x90ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000740 = r[0];
  *(uint32_t*)0x20000744 = 0xe0;
  *(uint64_t*)0x20000748 = 0x20000600;
  *(uint32_t*)0x20000634 = 0;
  *(uint64_t*)0x20000638 = 0;
  *(uint32_t*)0x20000668 = 0;
  *(uint32_t*)0x2000066c = 0;
  *(uint64_t*)0x20000670 = 0;
  *(uint64_t*)0x20000678 = 0;
  *(uint32_t*)0x20000684 = 8;
  *(uint64_t*)0x20000688 = 0;
  *(uint32_t*)0x20000690 = 0;
  *(uint32_t*)0x20000694 = 0;
  *(uint64_t*)0x20000698 = 0;
  *(uint64_t*)0x200006a0 = 0;
  *(uint32_t*)0x200006a8 = 0;
  *(uint32_t*)0x200006ac = 0x10;
  *(uint32_t*)0x200006b0 = 8;
  *(uint32_t*)0x200006b4 = 0;
  *(uint64_t*)0x200006b8 = 0;
  res = syscall(__NR_bpf, /*cmd=*/0xful, /*arg=*/0x20000740ul, /*size=*/0x10ul);
  if (res != -1)
    r[1] = *(uint32_t*)0x20000604;
  *(uint32_t*)0x20000780 = r[1];
  res = syscall(__NR_bpf, /*cmd=*/0xdul, /*arg=*/0x20000780ul, /*size=*/4ul);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x20000080 = 1;
  *(uint32_t*)0x20000084 = r[2];
  *(uint32_t*)0x20000088 = 0x2f;
  *(uint32_t*)0x2000008c = 8;
  *(uint32_t*)0x20000090 = -1;
  *(uint32_t*)0x20000094 = 0;
  *(uint64_t*)0x20000098 = 0;
  syscall(__NR_bpf, /*cmd=*/8ul, /*arg=*/0x20000080ul, /*size=*/0x20ul);
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul, /*prot=*/7ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  loop();
  return 0;
}
