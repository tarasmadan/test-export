// https://syzkaller.appspot.com/bug?id=5d5821ad053bbbd4a11174989f2bf9d111873c52
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

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0x0};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0xaul, 2ul, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_socket, 0x10ul, 0x803ul, 0);
  if (res != -1)
    r[2] = res;
  *(uint64_t*)0x20000300 = 0;
  *(uint32_t*)0x20000308 = 0;
  *(uint64_t*)0x20000310 = 0x200001c0;
  *(uint64_t*)0x200001c0 = 0;
  *(uint64_t*)0x200001c8 = 0x14;
  *(uint64_t*)0x20000318 = 1;
  *(uint64_t*)0x20000320 = 0;
  *(uint64_t*)0x20000328 = 0;
  *(uint32_t*)0x20000330 = 0;
  syscall(__NR_sendmsg, r[2], 0x20000300ul, 0ul);
  *(uint32_t*)0x20000f80 = 0x15;
  res = syscall(__NR_getsockname, r[2], 0x20000f40ul, 0x20000f80ul);
  if (res != -1)
    r[3] = *(uint32_t*)0x20000f44;
  *(uint64_t*)0x20000380 = 0;
  *(uint32_t*)0x20000388 = 0;
  *(uint64_t*)0x20000390 = 0x200002c0;
  *(uint64_t*)0x200002c0 = 0x20000640;
  *(uint32_t*)0x20000640 = 0x68;
  *(uint16_t*)0x20000644 = 0x10;
  *(uint16_t*)0x20000646 = 0x437;
  *(uint32_t*)0x20000648 = 0;
  *(uint32_t*)0x2000064c = 0;
  *(uint8_t*)0x20000650 = 0;
  *(uint8_t*)0x20000651 = 0;
  *(uint16_t*)0x20000652 = 0;
  *(uint32_t*)0x20000654 = r[3];
  *(uint32_t*)0x20000658 = 0x4048b;
  *(uint32_t*)0x2000065c = 0;
  *(uint16_t*)0x20000660 = 0x48;
  STORE_BY_BITMASK(uint16_t, , 0x20000662, 0x12, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000663, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000663, 1, 7, 1);
  *(uint16_t*)0x20000664 = 8;
  *(uint16_t*)0x20000666 = 1;
  memcpy((void*)0x20000668, "sit\000", 4);
  *(uint16_t*)0x2000066c = 0x3c;
  STORE_BY_BITMASK(uint16_t, , 0x2000066e, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000066f, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000066f, 1, 7, 1);
  *(uint16_t*)0x20000670 = 8;
  *(uint16_t*)0x20000672 = 1;
  *(uint32_t*)0x20000674 = r[3];
  *(uint16_t*)0x20000678 = 8;
  *(uint16_t*)0x2000067a = 3;
  *(uint8_t*)0x2000067c = 0xac;
  *(uint8_t*)0x2000067d = 0x1e;
  *(uint8_t*)0x2000067e = 0;
  *(uint8_t*)0x2000067f = 1;
  *(uint16_t*)0x20000680 = 8;
  *(uint16_t*)0x20000682 = 0x14;
  *(uint32_t*)0x20000684 = 0xef;
  *(uint16_t*)0x20000688 = 6;
  *(uint16_t*)0x2000068a = 0x10;
  *(uint16_t*)0x2000068c = 0x1c;
  *(uint16_t*)0x20000690 = 8;
  *(uint16_t*)0x20000692 = 0x14;
  *(uint32_t*)0x20000694 = 0x2001;
  *(uint16_t*)0x20000698 = 5;
  *(uint16_t*)0x2000069a = 0xa;
  *(uint8_t*)0x2000069c = 0;
  *(uint16_t*)0x200006a0 = 6;
  *(uint16_t*)0x200006a2 = 0xf;
  *(uint16_t*)0x200006a4 = 2;
  *(uint64_t*)0x200002c8 = 0x68;
  *(uint64_t*)0x20000398 = 1;
  *(uint64_t*)0x200003a0 = 0;
  *(uint64_t*)0x200003a8 = 0;
  *(uint32_t*)0x200003b0 = 0;
  syscall(__NR_sendmsg, r[1], 0x20000380ul, 0ul);
  *(uint64_t*)0x200017c0 = 0x20000040;
  *(uint16_t*)0x20000040 = 2;
  *(uint16_t*)0x20000042 = htobe16(0x4e1c);
  *(uint8_t*)0x20000044 = 0xac;
  *(uint8_t*)0x20000045 = 0x14;
  *(uint8_t*)0x20000046 = 0x14;
  *(uint8_t*)0x20000047 = 0xbb;
  *(uint32_t*)0x200017c8 = 0x10;
  *(uint64_t*)0x200017d0 = 0;
  *(uint64_t*)0x200017d8 = 0;
  *(uint64_t*)0x200017e0 = 0x200004c0;
  *(uint64_t*)0x200004c0 = 0x1c;
  *(uint32_t*)0x200004c8 = 0;
  *(uint32_t*)0x200004cc = 8;
  *(uint32_t*)0x200004d0 = r[3];
  *(uint32_t*)0x200004d4 = htobe32(0);
  *(uint32_t*)0x200004d8 = htobe32(0);
  *(uint64_t*)0x200017e8 = 0x20;
  *(uint32_t*)0x200017f0 = 0;
  *(uint32_t*)0x200017f8 = 0;
  syscall(__NR_sendmmsg, r[0], 0x200017c0ul, 1ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
