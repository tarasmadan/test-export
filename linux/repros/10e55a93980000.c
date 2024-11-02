// https://syzkaller.appspot.com/bug?id=2aabc827b8aea09353150377f96630a3ff18f6a3
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

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  res = syscall(__NR_socket, /*domain=*/2ul, /*type=*/1ul, /*proto=*/0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000040 = 1;
  syscall(__NR_setsockopt, /*fd=*/r[0], /*level=*/1,
          /*optname=SO_ZEROCOPY*/ 0x3c, /*optval=*/0x20000040ul,
          /*optlen=*/0xfff0ul);
  *(uint32_t*)0x200000c0 = 1;
  syscall(__NR_setsockopt, /*fd=*/r[0], /*level=*/6, /*optname=*/0x13,
          /*optval=*/0x200000c0ul, /*optlen=*/4ul);
  *(uint16_t*)0x20000080 = 2;
  *(uint16_t*)0x20000082 = htobe16(0);
  *(uint32_t*)0x20000084 = htobe32(0x7f000001);
  syscall(__NR_connect, /*fd=*/r[0], /*addr=*/0x20000080ul, /*addrlen=*/0x10ul);
  *(uint32_t*)0x200001c0 = -1;
  syscall(__NR_setsockopt, /*fd=*/r[0], /*level=*/6, /*optname=*/0x13,
          /*optval=*/0x200001c0ul, /*optlen=*/4ul);
  syscall(__NR_write, /*fd=*/r[0], /*data=*/0x200014c0ul, /*len=*/0x46bul);
  *(uint64_t*)0x20000f40 = 0;
  *(uint32_t*)0x20000f48 = 0;
  *(uint64_t*)0x20000f50 = 0x20000500;
  *(uint64_t*)0x20000f58 = 5;
  *(uint64_t*)0x20000f60 = 0;
  *(uint64_t*)0x20000f68 = 0;
  *(uint32_t*)0x20000f70 = 0;
  *(uint32_t*)0x20000f78 = 0;
  *(uint64_t*)0x20000f80 = 0;
  *(uint32_t*)0x20000f88 = 0;
  *(uint64_t*)0x20000f90 = 0x20000900;
  *(uint64_t*)0x20000900 = 0x20000580;
  memset((void*)0x20000580, 241, 1);
  *(uint64_t*)0x20000908 = 1;
  *(uint64_t*)0x20000910 = 0x20000c80;
  memset((void*)0x20000c80, 97, 1);
  *(uint64_t*)0x20000918 = 1;
  *(uint64_t*)0x20000920 = 0x20000b40;
  memset((void*)0x20000b40, 77, 1);
  *(uint64_t*)0x20000928 = 1;
  *(uint64_t*)0x20000930 = 0x20000400;
  memcpy((void*)0x20000400,
         "\x3f\x67\xfd\x57\x3d\x1f\x8d\x4e\x78\x91\xa7\x7b\x45\x9e\x77\x98\xc2"
         "\x95\xad\x31\x0c\x9b\xe7\xe2\x87\x16",
         26);
  *(uint64_t*)0x20000938 = 0x1a;
  *(uint64_t*)0x20000940 = 0x20000e80;
  memset((void*)0x20000e80, 8, 1);
  *(uint64_t*)0x20000948 = 1;
  *(uint64_t*)0x20000f98 = 0;
  *(uint64_t*)0x20000fa0 = 0;
  *(uint64_t*)0x20000fa8 = 0;
  *(uint32_t*)0x20000fb0 = 0;
  *(uint32_t*)0x20000fb8 = 0x70040000;
  *(uint64_t*)0x20000fc0 = 0;
  *(uint32_t*)0x20000fc8 = 0;
  *(uint64_t*)0x20000fd0 = 0x20000540;
  *(uint64_t*)0x20000540 = 0x20000180;
  memcpy((void*)0x20000180,
         "\xa1\x9f\x68\x1c\xa3\x14\x83\x2d\x00\x36\xeb\x5e\x8a\x8a\x19\xaf\xb1"
         "\x8f\x2c\x80\x00\xc0\xd9\x73\x41\x3f\x28\xa7",
         28);
  *(uint64_t*)0x20000548 = 0;
  *(uint64_t*)0x20000550 = 0x200003c0;
  memcpy(
      (void*)0x200003c0,
      "\x4a\x18\x58\x70\xb6\x35\x02\x1d\x69\x50\x66\x47\x36\xf4\x41\xfa\x8d\x5c"
      "\xf5\xac\xa8\x06\xf0\x55\xe2\x3f\x89\xa5\x83\x3e\xfa\xda\x9d\xf3\x05\x41"
      "\xa8\x0d\x53\x81\xd1\xba\xe2\xd6\xca\x7c\x2b\xc0\x09\x99\xd1\x7b",
      52);
  *(uint64_t*)0x20000558 = 0;
  *(uint64_t*)0x20000560 = 0x20000480;
  memcpy(
      (void*)0x20000480,
      "\xdb\xa1\x34\x13\xf5\xbf\x3a\xa9\x52\x2f\x25\x1d\xa4\xe8\x05\x26\x51\xc3"
      "\xe1\x38\xf2\xc9\x23\x5b\x73\xd9\x4f\xae\x4b\x09\x84\x78\x0f\xe8\xe1\xe1"
      "\x06\x48\x81\x3a\x26\xc1\xce\xcc\x25\x2c\xf5\x7a\xcd\x3b\x34\x99\x72\x23"
      "\x09\x8d\x1b\x6e\xaf\x4b\xc2\x26\xc2\x9d\xe5\x65\xb5\x19\x97\xca\xf8\xf4"
      "\xd7\x16\x47\xf0\xd1\xfd\x5d\xc5\x9f\xf4\xd0\xfa\xbe\xf5\xfd\x56\xde\xe2"
      "\xa6\xf1\x32\x1c\x72\x39\xda\x2c\xfd\xbf\x6c\x7b\x64\x60\xe9\x0d\x83\xb0"
      "\xb0\x68\xda\x9f\xdb\xdf\xc8\xd8\xdd\x0e\x51\x99\xb9\x8d\x1d\xe4\xd8\x77"
      "\xa8\x46\x7c\xa5\x51\x74\x67\x8a\x00\x3a\x9d\xee\x5b\xe0\xe7",
      141);
  *(uint64_t*)0x20000568 = 0;
  *(uint64_t*)0x20000fd8 = 0x10000000000000c8;
  *(uint64_t*)0x20000fe0 = 0;
  *(uint64_t*)0x20000fe8 = 0;
  *(uint32_t*)0x20000ff0 = 0;
  *(uint32_t*)0x20000ff8 = 0;
  *(uint64_t*)0x20001000 = 0;
  *(uint32_t*)0x20001008 = 0;
  *(uint64_t*)0x20001010 = 0x20000dc0;
  *(uint64_t*)0x20000dc0 = 0x20000440;
  memset((void*)0x20000440, 136, 1);
  *(uint64_t*)0x20000dc8 = 1;
  *(uint64_t*)0x20000dd0 = 0x20000840;
  memset((void*)0x20000840, 229, 1);
  *(uint64_t*)0x20000dd8 = 1;
  *(uint64_t*)0x20000de0 = 0x20000340;
  memcpy((void*)0x20000340,
         "\x96\xa3\x77\x22\x11\x5f\x95\x50\xe7\x3d\x72\x5d\xa8\xfc\x28\x61\x30"
         "\xde\xca\x89\x2b\x03\x37\xaf\xc6\x49\x0b\x04\x83\x9f\xb8\x05\x00\x03"
         "\x83\x82\x7c\x75\xcc\x29\x76\x13\x12\x97\x81\x38\x36\x80\x11\xa8\xb9"
         "\x9a\x6c\x1a\xdb\x96\x4e\x9c\x3b\x01\xd9\x75\x2e\x15\xea\x90\x44\x52"
         "\x05\x8d\x59\x53\x51\xd0\x3f\xaf\x56\x14\xd7\xb2\xd5\x94\x1c\x24\xc4"
         "\xcf\x19\xa8\x5b\x3e\xca\x7a\x57\x24\x3d\x3f\x07\xd5\x11\x18",
         100);
  *(uint64_t*)0x20000de8 = 1;
  *(uint64_t*)0x20001018 = 0x1046;
  *(uint64_t*)0x20001020 = 0;
  *(uint64_t*)0x20001028 = 0;
  *(uint32_t*)0x20001030 = 0;
  *(uint32_t*)0x20001038 = 0;
  syscall(__NR_sendmmsg, /*fd=*/r[0], /*mmsg=*/0x20000f40ul, /*vlen=*/4ul,
          /*f=MSG_ZEROCOPY|MSG_BATCH|MSG_OOB|MSG_MORE|MSG_DONTWAIT|MSG_CONFIRM*/
          0x4048841ul);
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