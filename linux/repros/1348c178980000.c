// https://syzkaller.appspot.com/bug?id=c5845e404a30925106c5c7dbe483140341a7eacf
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
#define __NR_bpf 321
#endif

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

const int kInitNetNsFd = 201;

static long syz_init_net_socket(volatile long domain, volatile long type,
                                volatile long proto)
{
  return syscall(__NR_socket, domain, type, proto);
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
                 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  res = -1;
  res = syz_init_net_socket(/*domain=*/6, /*type=*/5, /*proto=*/0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000100 = 1;
  *(uint8_t*)0x20000104 = 0xbb;
  *(uint8_t*)0x20000105 = 0xbb;
  *(uint8_t*)0x20000106 = 0xbb;
  *(uint8_t*)0x20000107 = 0xbb;
  *(uint8_t*)0x20000108 = 0xbb;
  *(uint8_t*)0x20000109 = 0;
  *(uint8_t*)0x2000010a = 0 + procid * 1;
  memcpy((void*)0x2000010b,
         "bpq0\000\000\000\000\000\000\000\000\000\000\000\000", 16);
  *(uint32_t*)0x2000011c = 0;
  memcpy((void*)0x20000120, "syz0\000\000\000", 7);
  *(uint8_t*)0x20000127 = 0xbb;
  *(uint8_t*)0x20000128 = 0xbb;
  *(uint8_t*)0x20000129 = 0xbb;
  *(uint8_t*)0x2000012a = 0xbb;
  *(uint8_t*)0x2000012b = 0xbb;
  *(uint8_t*)0x2000012c = 0;
  *(uint8_t*)0x2000012d = 0 + procid * 1;
  *(uint32_t*)0x20000130 = 0;
  *(uint32_t*)0x20000134 = 0;
  *(uint8_t*)0x20000138 = 0xbb;
  *(uint8_t*)0x20000139 = 0xbb;
  *(uint8_t*)0x2000013a = 0xbb;
  *(uint8_t*)0x2000013b = 1;
  *(uint8_t*)0x2000013c = 0 + procid * 1;
  *(uint8_t*)0x2000013f = 0x98;
  *(uint8_t*)0x20000140 = 0x92;
  *(uint8_t*)0x20000141 = 0x9c;
  *(uint8_t*)0x20000142 = 0xaa;
  *(uint8_t*)0x20000143 = 0xb0;
  *(uint8_t*)0x20000144 = 0x40;
  *(uint8_t*)0x20000145 = 2;
  *(uint8_t*)0x20000146 = 0xbb;
  *(uint8_t*)0x20000147 = 0xbb;
  *(uint8_t*)0x20000148 = 0xbb;
  *(uint8_t*)0x20000149 = 1;
  *(uint8_t*)0x2000014a = 0 + procid * 1;
  *(uint8_t*)0x2000014d = 0xa2;
  *(uint8_t*)0x2000014e = 0xa6;
  *(uint8_t*)0x2000014f = 0xa8;
  *(uint8_t*)0x20000150 = 0x40;
  *(uint8_t*)0x20000151 = 0x40;
  *(uint8_t*)0x20000152 = 0x40;
  *(uint8_t*)0x20000153 = 0;
  *(uint8_t*)0x20000154 = 0xbb;
  *(uint8_t*)0x20000155 = 0xbb;
  *(uint8_t*)0x20000156 = 0xbb;
  *(uint8_t*)0x20000157 = 1;
  *(uint8_t*)0x20000158 = 0 + procid * 1;
  *(uint8_t*)0x2000015b = 0xa2;
  *(uint8_t*)0x2000015c = 0xa6;
  *(uint8_t*)0x2000015d = 0xa8;
  *(uint8_t*)0x2000015e = 0x40;
  *(uint8_t*)0x2000015f = 0x40;
  *(uint8_t*)0x20000160 = 0x40;
  *(uint8_t*)0x20000161 = 0;
  *(uint8_t*)0x20000162 = 0xa2;
  *(uint8_t*)0x20000163 = 0xa6;
  *(uint8_t*)0x20000164 = 0xa8;
  *(uint8_t*)0x20000165 = 0x40;
  *(uint8_t*)0x20000166 = 0x40;
  *(uint8_t*)0x20000167 = 0x40;
  *(uint8_t*)0x20000168 = 0;
  *(uint8_t*)0x20000169 = 0x98;
  *(uint8_t*)0x2000016a = 0x92;
  *(uint8_t*)0x2000016b = 0x9c;
  *(uint8_t*)0x2000016c = 0xaa;
  *(uint8_t*)0x2000016d = 0xb0;
  *(uint8_t*)0x2000016e = 0x40;
  *(uint8_t*)0x2000016f = 2;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x890b, /*arg=*/0x20000100ul);
  res = -1;
  res = syz_init_net_socket(/*domain=*/6, /*type=*/5, /*proto=*/0);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20000100 = 1;
  *(uint8_t*)0x20000104 = 0xbb;
  *(uint8_t*)0x20000105 = 0xbb;
  *(uint8_t*)0x20000106 = 0xbb;
  *(uint8_t*)0x20000107 = 0xbb;
  *(uint8_t*)0x20000108 = 0xbb;
  *(uint8_t*)0x20000109 = 0;
  *(uint8_t*)0x2000010a = 0 + procid * 1;
  memcpy((void*)0x2000010b,
         "bpq0\000\000\000\000\000\000\000\000\000\000\000\000", 16);
  *(uint32_t*)0x2000011c = 0;
  memcpy((void*)0x20000120, "syz0\000\000\000", 7);
  *(uint8_t*)0x20000127 = 0xbb;
  *(uint8_t*)0x20000128 = 0xbb;
  *(uint8_t*)0x20000129 = 0xbb;
  *(uint8_t*)0x2000012a = 0xbb;
  *(uint8_t*)0x2000012b = 0xbb;
  *(uint8_t*)0x2000012c = 0;
  *(uint8_t*)0x2000012d = 0 + procid * 1;
  *(uint32_t*)0x20000130 = 0;
  *(uint32_t*)0x20000134 = 0;
  *(uint8_t*)0x20000138 = 0x40;
  *(uint8_t*)0x20000139 = 0x40;
  *(uint8_t*)0x2000013a = 0x40;
  *(uint8_t*)0x2000013b = 0x40;
  *(uint8_t*)0x2000013c = 0x40;
  *(uint8_t*)0x2000013d = 0x40;
  *(uint8_t*)0x2000013e = 0;
  *(uint8_t*)0x2000013f = 0x98;
  *(uint8_t*)0x20000140 = 0x92;
  *(uint8_t*)0x20000141 = 0x9c;
  *(uint8_t*)0x20000142 = 0xaa;
  *(uint8_t*)0x20000143 = 0xb0;
  *(uint8_t*)0x20000144 = 0x40;
  *(uint8_t*)0x20000145 = 2;
  *(uint8_t*)0x20000146 = 0xbb;
  *(uint8_t*)0x20000147 = 0xbb;
  *(uint8_t*)0x20000148 = 0xbb;
  *(uint8_t*)0x20000149 = 1;
  *(uint8_t*)0x2000014a = 0 + procid * 1;
  *(uint8_t*)0x2000014d = 0xa2;
  *(uint8_t*)0x2000014e = 0xa6;
  *(uint8_t*)0x2000014f = 0xa8;
  *(uint8_t*)0x20000150 = 0x40;
  *(uint8_t*)0x20000151 = 0x40;
  *(uint8_t*)0x20000152 = 0x40;
  *(uint8_t*)0x20000153 = 0;
  *(uint8_t*)0x20000154 = 0xbb;
  *(uint8_t*)0x20000155 = 0xbb;
  *(uint8_t*)0x20000156 = 0xbb;
  *(uint8_t*)0x20000157 = 1;
  *(uint8_t*)0x20000158 = 0 + procid * 1;
  *(uint8_t*)0x2000015b = 0xa2;
  *(uint8_t*)0x2000015c = 0xa6;
  *(uint8_t*)0x2000015d = 0xa8;
  *(uint8_t*)0x2000015e = 0x40;
  *(uint8_t*)0x2000015f = 0x40;
  *(uint8_t*)0x20000160 = 0x40;
  *(uint8_t*)0x20000161 = 0;
  *(uint8_t*)0x20000162 = 0x40;
  *(uint8_t*)0x20000163 = 0x40;
  *(uint8_t*)0x20000164 = 0x40;
  *(uint8_t*)0x20000165 = 0x40;
  *(uint8_t*)0x20000166 = 0x40;
  *(uint8_t*)0x20000167 = 0x40;
  *(uint8_t*)0x20000168 = 0;
  *(uint8_t*)0x20000169 = 0xbb;
  *(uint8_t*)0x2000016a = 0xbb;
  *(uint8_t*)0x2000016b = 0xbb;
  *(uint8_t*)0x2000016c = 1;
  *(uint8_t*)0x2000016d = 0 + procid * 1;
  syscall(__NR_ioctl, /*fd=*/r[1], /*cmd=*/0x890c, /*arg=*/0x20000100ul);
  *(uint32_t*)0x20000200 = 0;
  *(uint32_t*)0x20000204 = 5;
  *(uint64_t*)0x20000208 = 0x20000000;
  memcpy((void*)0x20000000,
         "\x18\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x85",
         17);
  *(uint64_t*)0x20000210 = 0;
  *(uint32_t*)0x20000218 = 0;
  *(uint32_t*)0x2000021c = 0;
  *(uint64_t*)0x20000220 = 0;
  *(uint32_t*)0x20000228 = 0;
  *(uint32_t*)0x2000022c = 0;
  memset((void*)0x20000230, 0, 16);
  *(uint32_t*)0x20000240 = 0;
  *(uint32_t*)0x20000244 = 2;
  *(uint32_t*)0x20000248 = -1;
  *(uint32_t*)0x2000024c = 8;
  *(uint64_t*)0x20000250 = 0;
  *(uint32_t*)0x20000258 = 0;
  *(uint32_t*)0x2000025c = 0x10;
  *(uint64_t*)0x20000260 = 0;
  *(uint32_t*)0x20000268 = 0;
  *(uint32_t*)0x2000026c = 0;
  *(uint32_t*)0x20000270 = 0;
  *(uint32_t*)0x20000274 = 0;
  *(uint64_t*)0x20000278 = 0;
  *(uint64_t*)0x20000280 = 0;
  *(uint32_t*)0x20000288 = 0x10;
  *(uint32_t*)0x2000028c = 0;
  syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x20000200ul, /*size=*/0x90ul);
  res = -1;
  res = syz_init_net_socket(/*fam=*/0x1f, /*type=*/5, /*proto=*/2);
  if (res != -1)
    r[2] = res;
  memcpy((void*)0x20000000,
         "bpq0\000\000\000\000\000\000\000\000\000\000\000\000", 16);
  syscall(__NR_setsockopt, /*fd=*/-1, /*level=*/0x101, /*optname=*/0x19,
          /*optval=*/0x20000000ul, /*optlen=*/0x10ul);
  syscall(__NR_ioctl, /*fd=*/r[2], /*cmd=*/0x8914, /*arg=*/0x20000000ul);
  res = -1;
  res = syz_init_net_socket(/*domain=*/6, /*type=*/5, /*proto=*/0);
  if (res != -1)
    r[3] = res;
  syscall(__NR_ioctl, /*fd=*/r[3], /*cmd=*/0x89e2, /*arg=*/0ul);
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
  for (procid = 0; procid < 5; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
