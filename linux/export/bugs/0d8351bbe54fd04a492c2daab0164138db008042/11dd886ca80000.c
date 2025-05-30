// https://syzkaller.appspot.com/bug?id=0d8351bbe54fd04a492c2daab0164138db008042
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

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, /*domain=*/0x10ul, /*type=*/3ul, /*proto=*/0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socketpair, /*domain=*/1ul, /*type=*/1ul, /*proto=*/0,
                /*fds=*/0x20000080ul);
  if (res != -1)
    r[1] = *(uint32_t*)0x20000084;
  memcpy((void*)0x20000000,
         "lo\000\000\000\000\000\000\000\000\000\000\000\000\000\000", 16);
  res = syscall(__NR_ioctl, /*fd=*/r[1], /*cmd=*/0x8933, /*arg=*/0x20000000ul);
  if (res != -1)
    r[2] = *(uint32_t*)0x20000010;
  *(uint64_t*)0x20000000 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint64_t*)0x20000010 = 0x20000780;
  *(uint64_t*)0x20000780 = 0x200002c0;
  memcpy((void*)0x200002c0,
         "\x78\x00\x00\x00\x24\x00\x0b\x0f\x00\x00\x00\x00\x00\x00\x00\x00\x60"
         "\x00\x00\x00",
         20);
  *(uint32_t*)0x200002d4 = r[2];
  memcpy((void*)0x200002d8,
         "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x0a\x00\x01\x00\x6e"
         "\x65\x74\x65\x6d\x00\x00\x00\x48\x00\x02\x00\x00\x00\x00\x00\x86\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff",
         50);
  *(uint64_t*)0x20000788 = 0x78;
  *(uint64_t*)0x20000018 = 1;
  *(uint64_t*)0x20000020 = 0;
  *(uint64_t*)0x20000028 = 0;
  *(uint32_t*)0x20000030 = 0;
  syscall(__NR_sendmsg, /*fd=*/r[0], /*msg=*/0x20000000ul, /*f=*/0ul);
  res = syscall(__NR_socket, /*domain=*/0xaul, /*type=*/1ul, /*proto=*/0x84);
  if (res != -1)
    r[3] = res;
  *(uint32_t*)0x20000100 = 0;
  *(uint16_t*)0x20000104 = 0xa;
  *(uint16_t*)0x20000106 = htobe16(0x4e20);
  *(uint32_t*)0x20000108 = htobe32(8);
  memset((void*)0x2000010c, 0, 16);
  *(uint32_t*)0x2000011c = 0x1000;
  *(uint32_t*)0x20000184 = 0;
  *(uint16_t*)0x20000188 = 0x900;
  *(uint32_t*)0x2000018a = 0x2c1;
  *(uint32_t*)0x2000018e = 0;
  *(uint32_t*)0x20000192 = 0x32;
  *(uint32_t*)0x20000196 = 0;
  *(uint8_t*)0x2000019a = 0;
  syscall(__NR_setsockopt, /*fd=*/r[3], /*level=*/0x84, /*opt=*/9,
          /*val=*/0x20000100ul, /*len=*/0x9cul);
  *(uint16_t*)0x20000580 = 0xa;
  *(uint16_t*)0x20000582 = htobe16(0x4e23);
  *(uint32_t*)0x20000584 = htobe32(0);
  *(uint64_t*)0x20000588 = htobe64(0);
  *(uint64_t*)0x20000590 = htobe64(1);
  *(uint32_t*)0x20000598 = 0;
  syscall(__NR_bind, /*fd=*/r[3], /*addr=*/0x20000580ul, /*addrlen=*/0x1cul);
  memset((void*)0x20847fff, 88, 1);
  *(uint16_t*)0x2005ffe4 = 0xa;
  *(uint16_t*)0x2005ffe6 = htobe16(0x4e23);
  *(uint32_t*)0x2005ffe8 = htobe32(0);
  *(uint64_t*)0x2005ffec = htobe64(0);
  *(uint64_t*)0x2005fff4 = htobe64(1);
  *(uint32_t*)0x2005fffc = 0;
  syscall(__NR_sendto, /*fd=*/r[3], /*buf=*/0x20847ffful, /*len=*/0x34000ul,
          /*f=*/0ul, /*addr=*/0x2005ffe4ul, /*addrlen=*/0x1cul);
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul, /*prot=*/7ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  setup_leak();
  loop();
  return 0;
}
