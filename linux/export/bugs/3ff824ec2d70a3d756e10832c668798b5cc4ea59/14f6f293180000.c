// https://syzkaller.appspot.com/bug?id=3ff824ec2d70a3d756e10832c668798b5cc4ea59
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
#define __NR_bpf 386
#endif
#ifndef __NR_close
#define __NR_close 6
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_recvmsg
#define __NR_recvmsg 297
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 296
#endif
#ifndef __NR_socketpair
#define __NR_socketpair 288
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

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
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      sleep_ms(1);
      if (current_time_ms() - start < 15000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[5] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  *(uint32_t*)0x2000e000 = 0x10;
  *(uint32_t*)0x2000e004 = 4;
  *(uint64_t*)0x2000e008 = 0x20000040;
  memcpy((void*)0x20000040,
         "\xb4\x00\x00\x00\x00\x00\x00\x00\x79\x10\x48\x00\x00\x00\x00\x00\x61"
         "\x04\x00\x00\x00\x00\x00\x00\x95\x00\x00\x00",
         28);
  *(uint64_t*)0x2000e010 = 0x20003ff6;
  memcpy((void*)0x20003ff6, "GPL\000", 4);
  *(uint32_t*)0x2000e018 = 2;
  *(uint32_t*)0x2000e01c = 0xfd90;
  *(uint64_t*)0x2000e020 = 0x2000cf3d;
  *(uint32_t*)0x2000e028 = 0;
  *(uint32_t*)0x2000e02c = 0;
  memset((void*)0x2000e030, 0, 16);
  *(uint32_t*)0x2000e040 = 0;
  *(uint32_t*)0x2000e044 = 0;
  *(uint32_t*)0x2000e048 = -1;
  *(uint32_t*)0x2000e04c = 8;
  *(uint64_t*)0x2000e050 = 0;
  *(uint32_t*)0x2000e058 = 0;
  *(uint32_t*)0x2000e05c = 0x10;
  *(uint64_t*)0x2000e060 = 0;
  *(uint32_t*)0x2000e068 = 0;
  *(uint32_t*)0x2000e06c = 0;
  *(uint32_t*)0x2000e070 = -1;
  *(uint32_t*)0x2000e074 = 0;
  *(uint64_t*)0x2000e078 = 0;
  *(uint64_t*)0x2000e080 = 0;
  *(uint32_t*)0x2000e088 = 0x10;
  *(uint32_t*)0x2000e08c = 0;
  res = syscall(__NR_bpf, /*cmd=*/5, /*arg=*/0x2000e000, /*size=*/0x48);
  if (res != -1)
    r[0] = res;
  syscall(__NR_close, /*fd=*/(intptr_t)r[0]);
  res = syscall(__NR_socketpair, /*domain=*/1, /*type=SOCK_DGRAM*/ 2,
                /*proto=*/0, /*fds=*/0x20000480);
  if (res != -1) {
    r[1] = *(uint32_t*)0x20000480;
    r[2] = *(uint32_t*)0x20000484;
  }
  *(uint32_t*)0x2000e000 = 0xe;
  *(uint32_t*)0x2000e004 = 4;
  *(uint64_t*)0x2000e008 = 0x20000540;
  memcpy(
      (void*)0x20000540,
      "\xb4\x05\x00\x00\xfd\xff\x7f\x00\x61\x10\x58\x00\x00\x00\x00\x00\xc6\x00"
      "\x00\x00\x00\x00\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00\x9f\x33\xef\x60"
      "\x91\x6e\x6e\x71\x3f\x1e\xeb\x0b\x72\x5a\xd9\x9b\x81\x7f\xd9\x8c\xd8\x24"
      "\x49\x89\x49\x71\x4f\xfa\xac\x8a\x6f\x77\x06\x00\xdc\xca\x55\xf2\x1f\x3c"
      "\xa9\xe8\x22\xd1\x82\x05\x4d\x54\xd5\x3c\xd2\xb6\xdb\x71\x4e\x4b\xeb\x54"
      "\x47\x00\x00\x01\x00\x00\x00\x00\x8f\x2b\x90\x00\xf2\x24\x25\xe4\x09\x7e"
      "\xd6\x2c\xbc\x89\x10\x61\x01\x7c\xfa\x6f\xa2\x6f\xa7\x08\x8c\x60\x89\x7d"
      "\x4a\x61\x48\xa1\xc1\xe4\x3f\x00\x00\x1b\xde\x60\xbe\xac\x67\x1e\x8e\x8f"
      "\xde\xcb\x03\x58\x8a\xa6\x23\xfa\x71\xf3\x1b\xf0\xf8\x71\xab\x5c\x2f\xf8"
      "\x8a\xfc\x60\x02\x7f\x4e\x5b\x52\x71\xed\x58\xe8\x35\xcf\x0d\x00\x00\x00"
      "\x00\x98\xb5\x1f\xe6\xb1\xb8\xd9\xdb\xe8\x7d\xcf\xf4\x14\xed\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\xb3\x47\xab\xe6\x35\x2a\x08\x0f\x81\x40\xe5\xfd"
      "\x10\x74\x7b\x6e\xcd\xb3\x54\x05\x46\xbf\x63\x6e\x3d\x6e\x70\x0e\x5b\x05"
      "\x00\x00\x00\x00\x00\x00\x00\xeb\x9e\x14\x03\xe6\xc8\xf7\xa1\x87\xea\xf6"
      "\x0f\x3a\x17\xf0\xf0\x46\xa3\x07\xa4\x03\xc1\x9d\x98\x29\xc9\x0b\xd2\x11"
      "\x42\x52\x58\x15\x67\xac\xae\x71\x5c\xbe\x1b\x57\xd5\xcd\xa4\x32\xc5\xb9"
      "\x10\x40\x06\x23\xd2\x41\x95\x40\x5f\x2e\x76\xcc\xb7\xb3\x7b\x41\x21\x5c"
      "\x18\x4e\x73\x1f\xb1",
      329);
  *(uint64_t*)0x2000e010 = 0x20003ff6;
  memcpy((void*)0x20003ff6, "GPL\000", 4);
  *(uint32_t*)0x2000e018 = 4;
  *(uint32_t*)0x2000e01c = 0xfd90;
  *(uint64_t*)0x2000e020 = 0x2000cf3d;
  *(uint32_t*)0x2000e028 = 0;
  *(uint32_t*)0x2000e02c = 0;
  memset((void*)0x2000e030, 0, 16);
  *(uint32_t*)0x2000e040 = 0;
  *(uint32_t*)0x2000e044 = 0;
  *(uint32_t*)0x2000e048 = -1;
  *(uint32_t*)0x2000e04c = 8;
  *(uint64_t*)0x2000e050 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x2000e058 = 0x366;
  *(uint32_t*)0x2000e05c = 0x10;
  *(uint64_t*)0x2000e060 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint32_t*)0x2000e068 = 0x1dd;
  *(uint32_t*)0x2000e06c = 0;
  *(uint32_t*)0x2000e070 = -1;
  *(uint32_t*)0x2000e074 = 0;
  *(uint64_t*)0x2000e078 = 0;
  *(uint64_t*)0x2000e080 = 0;
  *(uint32_t*)0x2000e088 = 0x10;
  *(uint32_t*)0x2000e08c = 0;
  res = syscall(__NR_bpf, /*cmd=*/5, /*arg=*/0x2000e000, /*size=*/0x48);
  if (res != -1)
    r[3] = res;
  *(uint32_t*)0x20000200 = 0xf;
  *(uint32_t*)0x20000204 = 4;
  *(uint32_t*)0x20000208 = 4;
  *(uint32_t*)0x2000020c = 0x12;
  *(uint32_t*)0x20000210 = 0;
  *(uint32_t*)0x20000214 = -1;
  *(uint32_t*)0x20000218 = 0;
  memset((void*)0x2000021c, 0, 16);
  *(uint32_t*)0x2000022c = 0;
  *(uint32_t*)0x20000230 = -1;
  *(uint32_t*)0x20000234 = 0;
  *(uint32_t*)0x20000238 = 0;
  *(uint32_t*)0x2000023c = 0;
  *(uint64_t*)0x20000240 = 0;
  res = syscall(__NR_bpf, /*cmd=*/0, /*arg=*/0x20000200, /*size=*/0x48);
  if (res != -1)
    r[4] = res;
  *(uint32_t*)0x20000080 = r[4];
  *(uint32_t*)0x20000084 = r[3];
  *(uint32_t*)0x20000088 = 0x26;
  *(uint32_t*)0x2000008c = 0;
  *(uint32_t*)0x20000090 = 0;
  *(uint32_t*)0x20000094 = -1;
  *(uint64_t*)0x20000098 = 0;
  syscall(__NR_bpf, /*cmd=*/8, /*arg=*/0x20000080, /*size=*/0x10);
  *(uint32_t*)0x200000c0 = r[4];
  *(uint64_t*)0x200000c8 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint64_t*)0x200000d0 = 0x20000080;
  *(uint32_t*)0x20000080 = r[0];
  *(uint64_t*)0x200000d8 = 0;
  syscall(__NR_bpf, /*cmd=*/2, /*arg=*/0x200000c0, /*size=*/0x20);
  *(uint32_t*)0x20000500 = 0;
  *(uint32_t*)0x20000504 = 0;
  *(uint32_t*)0x20000508 = 0;
  *(uint32_t*)0x2000050c = 0;
  *(uint32_t*)0x20000510 = 0;
  *(uint32_t*)0x20000514 = 0;
  *(uint32_t*)0x20000518 = 0;
  syscall(__NR_sendmsg, /*fd=*/(intptr_t)r[2], /*msg=*/0x20000500, /*f=*/0);
  *(uint32_t*)0x200001c0 = 0;
  *(uint32_t*)0x200001c4 = 0;
  *(uint32_t*)0x200001c8 = 0x20000180;
  *(uint32_t*)0x20000180 = 0x20000280;
  *(uint32_t*)0x20000184 = 0xd4;
  *(uint32_t*)0x200001cc = 1;
  *(uint32_t*)0x200001d0 = 0;
  *(uint32_t*)0x200001d4 = 0;
  *(uint32_t*)0x200001d8 = 0;
  syscall(__NR_recvmsg, /*fd=*/(intptr_t)r[1], /*msg=*/0x200001c0, /*f=*/0, 0);
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000, /*len=*/0x1000, /*prot=*/0,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32, /*fd=*/-1,
          /*offset=*/0);
  syscall(__NR_mmap, /*addr=*/0x20000000, /*len=*/0x1000000,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32, /*fd=*/-1,
          /*offset=*/0);
  syscall(__NR_mmap, /*addr=*/0x21000000, /*len=*/0x1000, /*prot=*/0,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32, /*fd=*/-1,
          /*offset=*/0);
  loop();
  return 0;
}
