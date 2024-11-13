// https://syzkaller.appspot.com/bug?id=ebbbd90d92c90bd6c7c13eabe78db0ab0ab4ecf1
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
  res = syscall(__NR_socket, /*domain=*/0x10ul, /*type=*/3ul, /*proto=*/0xc);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x2000c2c0 = 0;
  *(uint32_t*)0x2000c2c8 = 0;
  *(uint64_t*)0x2000c2d0 = 0x20000200;
  *(uint64_t*)0x20000200 = 0x20000340;
  memcpy((void*)0x20000340,
         "\x14\x00\x00\x00\x10\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x0a\x28\x00\x00\x00\x00\x0a\x01\x01\x00\x00\x00\x00\x5e\x1a"
         "\xff\xd5\x02\x00\x00\x00\x09\x00\x01\x00\x73\x79\x7a\x30\x00\x00\x00"
         "\x00\x08\x00\x02\x40\x00\x00\x00\x03\x2c\x00\x00\x00\x03\x0a\x01\x03"
         "\x00\x00\xe6\xff\x00\x00\x00\x00\x02\x00\x00\x00\x09\x00\x01\x00\x73"
         "\x79\x7a\x30\x00\x00\x00\x00\x09\x00\x03\x00\x73\x79\x7a\x32\x00\x00"
         "\x00\x00\x14\x00\x00\x00\x11\x00\x01",
         111);
  *(uint64_t*)0x20000208 = 0x7c;
  *(uint64_t*)0x2000c2d8 = 1;
  *(uint64_t*)0x2000c2e0 = 0;
  *(uint64_t*)0x2000c2e8 = 0;
  *(uint32_t*)0x2000c2f0 = 0;
  syscall(__NR_sendmsg, /*fd=*/r[0], /*msg=*/0x2000c2c0ul, /*f=*/0ul);
  *(uint64_t*)0x20000080 = 0;
  *(uint32_t*)0x20000088 = 0;
  *(uint64_t*)0x20000090 = 0x20000040;
  *(uint64_t*)0x20000040 = 0x20000240;
  *(uint32_t*)0x20000240 = 0x14;
  *(uint16_t*)0x20000244 = 0x10;
  *(uint16_t*)0x20000246 = 1;
  *(uint32_t*)0x20000248 = 0;
  *(uint32_t*)0x2000024c = 0;
  *(uint8_t*)0x20000250 = 0;
  *(uint8_t*)0x20000251 = 0;
  *(uint16_t*)0x20000252 = htobe16(0xa);
  *(uint32_t*)0x20000254 = 0x2c;
  *(uint8_t*)0x20000258 = 6;
  *(uint8_t*)0x20000259 = 0xa;
  *(uint16_t*)0x2000025a = 0x40b;
  *(uint32_t*)0x2000025c = 0;
  *(uint32_t*)0x20000260 = 0;
  *(uint8_t*)0x20000264 = 2;
  *(uint8_t*)0x20000265 = 0;
  *(uint16_t*)0x20000266 = htobe16(0);
  *(uint16_t*)0x20000268 = 9;
  *(uint16_t*)0x2000026a = 1;
  memcpy((void*)0x2000026c, "syz0\000", 5);
  *(uint16_t*)0x20000274 = 9;
  *(uint16_t*)0x20000276 = 2;
  memcpy((void*)0x20000278, "syz2\000", 5);
  *(uint32_t*)0x20000280 = 0x14;
  *(uint16_t*)0x20000284 = 0x11;
  *(uint16_t*)0x20000286 = 1;
  *(uint32_t*)0x20000288 = 0;
  *(uint32_t*)0x2000028c = 0;
  *(uint8_t*)0x20000290 = 0;
  *(uint8_t*)0x20000291 = 0;
  *(uint16_t*)0x20000292 = htobe16(0xa);
  *(uint64_t*)0x20000048 = 0x54;
  *(uint64_t*)0x20000098 = 1;
  *(uint64_t*)0x200000a0 = 0;
  *(uint64_t*)0x200000a8 = 0;
  *(uint32_t*)0x200000b0 = 0;
  syscall(__NR_sendmsg, /*fd=*/r[0], /*msg=*/0x20000080ul, /*f=*/0ul);
  *(uint64_t*)0x200002c0 = 0;
  *(uint32_t*)0x200002c8 = 0;
  *(uint64_t*)0x200002d0 = 0x200001c0;
  *(uint64_t*)0x200001c0 = 0x20000780;
  *(uint32_t*)0x20000780 = 0x14;
  *(uint16_t*)0x20000784 = 0x10;
  *(uint16_t*)0x20000786 = 1;
  *(uint32_t*)0x20000788 = 0;
  *(uint32_t*)0x2000078c = 0;
  *(uint8_t*)0x20000790 = 0;
  *(uint8_t*)0x20000791 = 0;
  *(uint16_t*)0x20000792 = htobe16(0xa);
  *(uint32_t*)0x20000794 = 0x38;
  *(uint8_t*)0x20000798 = 8;
  *(uint8_t*)0x20000799 = 0xa;
  *(uint16_t*)0x2000079a = 0xd517;
  *(uint32_t*)0x2000079c = 0;
  *(uint32_t*)0x200007a0 = 0;
  *(uint8_t*)0x200007a4 = 2;
  *(uint8_t*)0x200007a5 = 0;
  *(uint16_t*)0x200007a6 = htobe16(0);
  *(uint16_t*)0x200007a8 = 9;
  *(uint16_t*)0x200007aa = 2;
  memcpy((void*)0x200007ac, "syz2\000", 5);
  *(uint16_t*)0x200007b4 = 9;
  *(uint16_t*)0x200007b6 = 1;
  memcpy((void*)0x200007b8, "syz0\000", 5);
  *(uint16_t*)0x200007c0 = 0xc;
  STORE_BY_BITMASK(uint16_t, , 0x200007c2, 3, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200007c3, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200007c3, 0, 7, 1);
  *(uint64_t*)0x200007c4 = htobe64(2);
  *(uint32_t*)0x200007cc = 0x14;
  *(uint16_t*)0x200007d0 = 0x11;
  *(uint16_t*)0x200007d2 = 1;
  *(uint32_t*)0x200007d4 = 0;
  *(uint32_t*)0x200007d8 = 0;
  *(uint8_t*)0x200007dc = 0;
  *(uint8_t*)0x200007dd = 0;
  *(uint16_t*)0x200007de = htobe16(0xa);
  *(uint64_t*)0x200001c8 = 0x60;
  *(uint64_t*)0x200002d8 = 1;
  *(uint64_t*)0x200002e0 = 0;
  *(uint64_t*)0x200002e8 = 0;
  *(uint32_t*)0x200002f0 = 0;
  syscall(__NR_sendmsg, /*fd=*/r[0], /*msg=*/0x200002c0ul, /*f=*/0ul);
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