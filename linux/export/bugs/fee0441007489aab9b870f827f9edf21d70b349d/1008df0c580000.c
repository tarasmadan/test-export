// https://syzkaller.appspot.com/bug?id=fee0441007489aab9b870f827f9edf21d70b349d
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  *(uint32_t*)0x200000000040 = 2;
  *(uint32_t*)0x200000000044 = 0x80;
  *(uint8_t*)0x200000000048 = 0xee;
  *(uint8_t*)0x200000000049 = 0;
  *(uint8_t*)0x20000000004a = 0;
  *(uint8_t*)0x20000000004b = 0;
  *(uint32_t*)0x20000000004c = 0;
  *(uint64_t*)0x200000000050 = 0;
  *(uint64_t*)0x200000000058 = 0;
  *(uint64_t*)0x200000000060 = 7;
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 5, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 29, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 30, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 31, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 32, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 33, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 34, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 35, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 36, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 37, 1);
  STORE_BY_BITMASK(uint64_t, , 0x200000000068, 0, 38, 26);
  *(uint32_t*)0x200000000070 = 2;
  *(uint32_t*)0x200000000074 = 5;
  *(uint64_t*)0x200000000078 = 0;
  *(uint64_t*)0x200000000080 = 0xf;
  *(uint64_t*)0x200000000088 = 0x1000;
  *(uint64_t*)0x200000000090 = 4;
  *(uint32_t*)0x200000000098 = 0;
  *(uint32_t*)0x20000000009c = 8;
  *(uint64_t*)0x2000000000a0 = 0;
  *(uint32_t*)0x2000000000a8 = 0;
  *(uint16_t*)0x2000000000ac = 2;
  *(uint16_t*)0x2000000000ae = 0;
  *(uint32_t*)0x2000000000b0 = 0;
  *(uint32_t*)0x2000000000b4 = 0;
  *(uint64_t*)0x2000000000b8 = 0;
  res = syscall(__NR_perf_event_open, /*attr=*/0x200000000040ul, /*pid=*/0,
                /*cpu=*/0ul, /*group=*/(intptr_t)-1, /*flags=*/0ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x2000000000c0 = 5;
  *(uint32_t*)0x2000000000c4 = 5;
  *(uint64_t*)0x2000000000c8 = 0x200000000180;
  memcpy((void*)0x200000000180,
         "\x18\x08\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x18"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x95",
         33);
  *(uint64_t*)0x2000000000d0 = 0x200000000980;
  memcpy((void*)0x200000000980, "GPL\000", 4);
  *(uint32_t*)0x2000000000d8 = 6;
  *(uint32_t*)0x2000000000dc = 0;
  *(uint64_t*)0x2000000000e0 = 0;
  *(uint32_t*)0x2000000000e8 = 0x41100;
  *(uint32_t*)0x2000000000ec = 0;
  memset((void*)0x2000000000f0, 0, 16);
  *(uint32_t*)0x200000000100 = 0;
  *(uint32_t*)0x200000000104 = 0x28;
  *(uint32_t*)0x200000000108 = 0;
  *(uint32_t*)0x20000000010c = 0;
  *(uint64_t*)0x200000000110 = 0;
  *(uint32_t*)0x200000000118 = 0;
  *(uint32_t*)0x20000000011c = 0;
  *(uint64_t*)0x200000000120 = 0;
  *(uint32_t*)0x200000000128 = 0;
  *(uint32_t*)0x20000000012c = 0;
  *(uint32_t*)0x200000000130 = 0;
  *(uint32_t*)0x200000000134 = 0;
  *(uint64_t*)0x200000000138 = 0;
  *(uint64_t*)0x200000000140 = 0;
  *(uint32_t*)0x200000000148 = 0;
  *(uint32_t*)0x20000000014c = 0;
  *(uint32_t*)0x200000000150 = 0;
  res =
      syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x2000000000c0ul, /*size=*/0x94ul);
  if (res != -1)
    r[1] = res;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x40042408, /*prog=*/r[1]);
  *(uint32_t*)0x200000000080 = 0x11;
  *(uint32_t*)0x200000000084 = 8;
  *(uint64_t*)0x200000000088 = 0x200000003900;
  memcpy(
      (void*)0x200000003900,
      "\x62\x0a\xf8\xff\x0c\x20\x00\x21\xbf\xa1\x00\x00\x00\x00\x00\x00\x07\x01"
      "\x00\x00\xf8\xff\xff\xff\xb7\x02\x00\x00\x03\x00\x00\x00\xbd\x12\x00\x00"
      "\x00\x00\x00\x00\x85\x00\x00\x00\x06\x00\x00\x00\xb7\x00\x00\x00\x00\x00"
      "\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00\x3f\xaf\x4f\x1e\x7f\x2a\xa3\xd9"
      "\xb1\x8e\xd8\x1c\x0c\x86\x9b\x51\xec\x6c\x0a\xf4\xe0\xe4\xa9\x44\x6c\x76"
      "\x70\x56\x89\x82\xb4\xe0\x20\xf6\x98\x39\x3a\xa0\xf3\x88\x1f\x9c\x24\xaa"
      "\x56\xf1\x51\x99\xfa\xd0\x09\x3c\x59\xd6\x6b\x5e\xce\x9f\x36\xc7\x0d\x0f"
      "\x13\x90\x5e\xa2\x3c\x22\x62\x4c\x9f\x87\xf9\x79\x3f\x50\xbb\x54\x60\x40"
      "\x67\x7b\x0c\x50\x77\xda\x80\xfb\x98\x2c\x1e\x94\x00\xe6\x93\x14\x6c\xea"
      "\x48\x4a\x41\x5b\x76\x96\x61\x18\xb6\x4f\x75\x1a\x0f\x24\x1b\x07\x08\x00"
      "\x08\x00\x2d\x75\x59\x3a\x28\x6c\xec\xc9\x3e\x64\xc2\x27\xc9\x5a\xa0\xb7"
      "\x84\x62\x57\x04\xf0\x7a\x72\xc2\x34\x66\x4c\x0a\xf9\x36\x0a\x1f\x7a\x5e"
      "\x6b\x60\x71\x30\xc8\x9f\x18\xc0\xc1\x08\x9d\x8b\x85\x32\x89\xe0\x1a\xa2"
      "\x7a\xe8\xb0\x9e\x00\xe7\x9a\xb2\x0b\x0b\x8e\x11\x48\xf4\x9f\xaf\x2a\xd0"
      "\x00\x00\x00\x00\x00\x00\x00\x6f\xa0\x3c\x64\x68\x97\x20\x89\xb3\x02\xd7"
      "\xbf\x60\x23\xcd\xce\xdb\x5e\x01\x25\xeb\xbc\x08\xde\xe5\x10\xcb\x23\x64"
      "\x14\x92\x15\x10\x83\x33\x71\x9a\xcd\x97\xcf\xa1\x07\xd4\x02\x24\xed\xc5"
      "\x46\x5a\x93\x2b\x77\xe7\x4e\x80\x2a\x0d\x42\xbc\x60\x99\xad\x23\x00\x00"
      "\x00\x80\x00\x6e\xf6\xc1\xff\x09\x00\x00\x00\x00\x00\x00\x10\xc6\x3a\x94"
      "\x9e\x8b\x79\x55\x39\x4f\xfa\x82\xb8\xe9\x42\xc8\x91\x12\xf4\xab\x87\xb1"
      "\xbf\xed\xa7\xbe\x58\x66\x02\xd9\x85\x43\x0c\xea\x01\x62\xab\x3f\xcf\x45"
      "\x91\xc9\x26\xab\xfb\x07\x67\x19\x23\x02\x00\x00\x00\xb0\xee\xa2\x44\x92"
      "\xa6\x60\x58\x3e\xec\xb4\x2c\xbc\xd3\xde\x3a\x83\x20\x9d\xa1\x7a\x0f\xaf"
      "\x60\xfd\x6a\xd9\xb9\x7a\xa5\xfa\x68\x48\x03\x66\xc9\xc6\xfd\x6f\xa5\x04"
      "\x3a\xa3\x92\x6b\x81\xe3\xb5\x9c\x95\xc2\x5a\x57\x3d\xc2\xed\xca\xea\x2b"
      "\x1a\x52\x49\x6d\xfc\xaf\x99\x43\x14\x12\xfd\x13\x4a\x99\x63\x82\xa1\xa0"
      "\x4d\x5b\xb9\x24\xcf\xe5\xf3\x18\x54\x18\xd6\x05\xff\xff\x9c\x4d\x2e\xc7"
      "\xc3\x2f\x20\x95\xe6\x3c\x80\xaf\xf9\xfa\x74\x0b\x5b\x76\x32\xf3\x20\x30"
      "\x91\x6f\x89\xc6\xda\xd7\x60\x3f\x2b\xa2\xa7\x90\xd6\x2d\x6f\xae\xc2\xfe"
      "\xd4\x4d\xa4\x92\x8b\x30\x14\x2b\xa1\x1d\xe6\xc5\xd5\x0b\x83\xba\xe6\x13"
      "\x40\x22\x16\xb5\x05\x4d\x1e\x7c\x13\xb1\x35\x5d\x6f\x4a\x82\x45\xff\xa4"
      "\x99\x7d\xa9\xc7\x7a\xf4\xc0\xeb\x97\xfc\xa5\x85\xec\x6b\xf5\xaf\x51\xd5"
      "\x64\xbe\xb6\xd9\x52\xaa\xb9\xc7\x07\x64\xb0\xa8\xa7\x58\x3c\x90\xb3\x43"
      "\x3b\x80\x9b\xdb\x9f\xbd\x48\xbc\x87\x34\x95\xcb\xff\x8a\x32\x6e\xea\x31"
      "\xae\x4e\x0f\x75\x05\xeb\xf6\xc9\xd1\x33\x30\xca\x00\x5a\xce\x1a\x84\x52"
      "\x1f\x14\x51\x8c\x9b\x47\x6f\xcc\xbd\x6c\x71\x20\x16\x21\x98\x48\x62\x4b"
      "\x87\xce\xc2\xdb\xe9\x82\x23\xa0\xeb\x4f\xa3\x9f\x6b\x5c\x02\xe6\xd6\xd9"
      "\x07\x56\xff\x57\x90\x2a\x8f\x57\x00\x00\x00\x00\x97\x00\xcf\x0b\x4b\x8b"
      "\xc2\x29\x41\x33\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x8b\xc0\xd9\x55\x97\x11\xe6"
      "\xe8\x86\x1c\x46\x49\x5b\xa5\x85\xa4\xb2\xd0\x2e\xdc\x3e\x28\xdd\x27\x1c"
      "\x89\x62\x49\xed\x85\xb9\x80\x68\x0b\x00\x00\x2b\x43\x5a\xc1\x5f\xc0\x28"
      "\x8d\x9b\x2a\x16\x9c\xdc\xac\xc4\x13\x03\x8d\xaf\xb7\xa2\xc8\xcb\x48\x2b"
      "\xac\x0a\xc5\x02\xd9\xba\x96\xff\xff\xff\x7f\x00\x00\x10\x00\x00\x00\x00"
      "\x00\x00\x7d\x5a\xd8\x97\xef\x3b\x7c\xda\x42\x01\x3d\x53\x04\x6d\xa2\x1b"
      "\x40\x21\x6e\x14\xba\x2d\x6a\xd5\x65\x6b\xff\xf1\x7a\xdd\xae\xda\xb2\x5b"
      "\x30\x00\x2a\xbb\xba\x7f\xa7\x25\xf3\x84\x00\xbe\x7c\x1f\x00\x1b\x2c\xd3"
      "\x17\x90\x2f\x19\xe3\x85\xbe\x9e\x48\xdc\xcf\xf7\x29\x43\x32\x82\x83\x06"
      "\x89\xda\x6b\x53\xb2\x63\x33\x98\x63\x29\x77\x71\x42\x9d\x12\x00\x00\x00"
      "\x33\x41\xbf\x4a\xba\xca\xc9\x59\x00\xfc\xa0\x49\x3c\xf2\x9b\x33\xdc\xc9"
      "\xff\xff\xff\xff\xff\xff\xff\xd3\x9f\xec\x22\x71\xff\x01\x58\x96\x46\xef"
      "\xd1\xcf\x87\x0c\xd7\xbb\x23\x66\xfd\xe4\x1f\x94\x29\x0c\x2a\x5f\xf8\x70"
      "\xce\x41\xfd\x34\x67\xde\xcb\x05\xcf\xd9\xfc\xb3\x2c\x8e\xd1\xdb\xd9\xd1"
      "\x0a\x64\xc1\x08\x3d\x5e\x71\xb5\x56\x5b\x17\x68\xee\x58\x96\x9c\x41\x59"
      "\x52\x29\xdf\x17\xbc\xad\x70\xfb\x40\x21\x42\x8c\xe9\x70\x27\x5d\x13\xb7"
      "\x81\x00\x78\x8f\x11\xf7\x61\x61\xd4\x6e\xa3\xab\xe0\xfa\x4d\x30\xdc\x94"
      "\xef\x24\x18\x75\xf3\xb4\xce\x02\x32\xfc\xea\x69\xc2\x71\xd7\xfa\x29\x82"
      "\x2a\xea\x68\xa6\x60\xe7\x17\xa0\x4b\xec\xff\x0f\x71\x91\x97\x72\x4f\x4f"
      "\xce\x10\x93\xb6\x2d\x7e\x8c\x71\x23\xd8\xec\x57\x1b\xe5\x4c\x72\xd9\x78"
      "\xcf\x90\x6d\xf0\x04\x2e\x36\xac\xd3\x7d\x7f\x9e\x11\x9f\x2c\x06\xf8\x15"
      "\x31\x2e\x0c\xfe\x22\x2a\x06\xf5\x6d\xd0\x22\xc0\x74\xeb\x8a\x32\x2f\xb0"
      "\xbf\x47\xc0\xa8\xd1\x54\xb4\x05\xc3\x7f\xea\xf3\xdd\x95\xf6\xef\x2a\xe5"
      "\x82\x78\x61\x05\xc7\xdf\x8b\xe5\x87\x70\x50\xc9\x13\x01\xbb\x99\x73\x16"
      "\xdb\xf1\x78\x66\xfb\x84\xd4\x17\x37\x31\xef\xe8\x95\xff\x2e\x1c\x55\x60"
      "\x92\x6e\x90\x10\x9b\x59\x85\x02\xd3\xe9\x59\xef\xc7\x1f\x66\x5c\x4d\x75"
      "\xcf\x24\x58\xe3\x54\x6c\x1c\x77\x6d\xa6\x4f\xb5\xab\xee\x0a\xcf\xd2\x35"
      "\xf2\xf4\x63\x2c\x90\x62\xec\xe8\x4c\x99\xa0\x61\x88\x7a\x20\x63\x9b\x41"
      "\xc8\xc1\x2e\xe8\x6c\x50\x80\x40\x42\xb3\xfb\x5a\xac\x51\x8a\x75\xf9\xe7"
      "\xd7\x10\x1d\x5e\x18\x6c\x48\x9b\x3a\x06\xfb\x99\xe0\xaa\x7f\x23\xa0\x54"
      "\xde\x2f\x4d\x92\xd6\xbd\x72\xee\x2c\x9f\xdc\x75\xaa\xaf\x1e\x3e\x48\x3b"
      "\x4a\xd0\x55\x73\xaf\x40\x32\x69\x93\x94\x7d\x9a\x63\x1b\xcb\xf3\x58\x37"
      "\x84\xac\xbd\xa2\x16\x55\x0d\x7a\xec\x6b\x79\xe3\x0c\xbd\x12\x8f\x54\xc2"
      "\xd3\x33\x54\x57\xac\xf3\x73\x31\x76\x6e\x47\x23\x91\xe3\x58\xc3\xb3\x77"
      "\x32\x7a\xc9\xec\xc3\x4f\x24\xc9\xae\x15\x3e\xc6\x0a\xc0\x69\x4d\xc5\x5b"
      "\xff\x9f\x5f\x45\xf9\x04\x00\x00\x00\x00\x00\x00\x00\xd6\xb2\xc5\xea\x13"
      "\x93\xfd\xf2\x42\x85\xbf\x16\xb9\x9c\x9c\xc0\xad\x18\x57\x21\x6f\x1a\x98"
      "\x5f\x36\x91\x91\xae\x95\x4f\xeb\xb3\xdf\x46\x4b\xfe\x0f\x7f\x3e\xe9\xaf"
      "\xe7\xbe\xfb\x89\xd2\x77\x73\x99\xf5\x87\x4c\x55\x3a\xeb\x37\x29\xcf\xfe"
      "\x86\xe6\x69\x64\xae\x09\xbb\x6d\x16\x31\x18\xe4\xcb\xe0\x24\xfd\x45\x00"
      "\xf8\xff\x07\x00\x00\x00\x00\xcc\x9d\x80\x46\xc2\x16\xc1\xf8\x95\x77\x8c"
      "\xb2\x51\x22\xa2\xa9\xf9\xb4\x44\xae\xad\xea\x2a\x40\xda\x8d\xac\xcf\x08"
      "\x08\x42\xa4\x86\x72\x17\x37\x39\x0c\xbf\x3a\x74\xcb\x20\x03\x01\x6f\x15"
      "\x14\x21\x6b\xdf\x57\xd2\xa4\x0d\x40\xb5\x1a\xb6\x3e\x96\xec\x84\x85\xb3"
      "\xb8\xa8\xc9\xae\x3d\x14\xf9\x31\x00\xc2\xe0\x89\x38\x62\xee\xf5\x52\xfc"
      "\xde\x29\x81\xf4\x8c\x48\x2b\xde\x8a\x16\x8c\x3f\x5d\xb2\xfe\xa6\xf2\x6e"
      "\x4a\x43\x04\xe5\x0c\x34\x9f\x4f\x9e\xce\xe2\x7d\xef\xc9\x38\x71\xc5\xf9"
      "\x9b\x35\x5b\x72\xd5\x38\xba\x49\x58\xea\x8e\x4a\xa3\x70\x94\x19\x1e\x10"
      "\x09\x6e\x7e\x60\xfc\x35\x41\xa2\xc9\x05\xa1\xa9\x5e\x95\x71\xbf\x38\xae"
      "\x19\x81\xc4\x23\x8e\xca\xee\x6f\x75\xcd\x0a\x68\x81\xbd\x15\x17\xa8\x25"
      "\x0d\xdc\x86\x74\x15\x2f\x94\xe3\xa4\x09\xe2\xa3\xbc\xe1\x09\xb6\x00\x00"
      "\x00\x00\x00\x00\x00\x00\xd6\xd5\x21\x0d\x75\x03\x00\x00\x00\xa8\x7a\x27"
      "\x60\x2b\x81\xf7\x63\x86\xf1\x53\x5b\xef\x14\x97\xf9\x21\x86\x08\x6e\x29"
      "\xc6\xbc\x5a\x1f\xad\x6e\xc9\xa3\x11\x37\xab\xf9\xa4\x04\xab\xde\x77\x50"
      "\x89\x8b\x1b\xd6\x27\xe8\x73\xf8\x70\x3b\xe8\x67\x2d\x70\xd1\xab\x57\x07"
      "\x52\x28\xa9\xf4\x6e\xd9\xbd\x1f\x08\xfb\x81\x91\xbb\xab\x2d\xc5\x1d\xe3"
      "\xa6\x1f\x08\x68\xaf\xc4\x29\x48\x59\x32\x3e\x6c\x25\x7a\x45\x31\x9f\x18"
      "\x10\x12\x88\xd1\x39\xbd\x3d\xa2\x0f\xed\x05\xa8\xfe\x64\x68\x0b\x0a\x3f"
      "\xc2\x2d\xd7\x04\x00\x00\x00\x00\x94\x69\x12\xd6\xc9\x8c\xd1\xa9\xfb\xe1"
      "\xe7\xd5\x8c\x08\xac\xaf\x30\x23\x5b\x91\x8a\x31\xd2\xec\xa5\x5f\x74\xa2"
      "\x36\x41\xf6\x1f\x2d\x5b\x30\x8c\xf0\xd0\x31\xb0\xc7\xf0\xce\xd6\x99\x93"
      "\xe9\x96\x0f\xf5\xf7\x60\x15\xe6\x00\x95\x56\x23\x7b\xad\xf4\xe7\x96\x5b"
      "\xbe\x27\x77\xe8\x08\xfc\xba\x82\x1a\xa8\xe8\xc5\xc3\x96\x09\xff\x85\x43"
      "\x52\xcb\x49\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xc1\xfe"
      "\xe3\x0a\x3f\x7a\x85\xd1\xb2\x9e\x58\xc7\x76\x85\xef\xc0\xce\xb1\xc8\xe5"
      "\x72\x9c\x66\x41\x8d\x16\x9f\xc0\x3a\xa1\x88\x54\x6b\x3a\xd2\xa1\x82\x06"
      "\x8e\x1e\x3a\x0e\x25\x05\xbc\x7f\x41\x01\x96\x45\x46\x6a\x53\xf1\xc9\x6e"
      "\x0d\x4b\x3b\xc1\x9f\xaa\x54\x49\x20\x9b\x08\x3d\xbd\x33\x4b\x47\xf0\x67"
      "\xbb\xab\x40\x74\x3b\x2a\x42\x01\x00\x82\x00\x8d\xf7\x5c\xf4\x3f\x8e\xcc"
      "\x8d\x37\x26\x60\x21\x11\xb4\x0e\x76\x1f\xd2\x10\x81\x92\x03\x82\xf1\x4d"
      "\x12\xca\x3c\x34\x31\xee\x97\x47\x1c\x78\x68\xdc\xda\x7e\xaa\x69\xeb\x7f"
      "\x7f\x80\x57\x2f\xdd\x11\xbb\x1d\x0d\x12\x80\xfb\xc2\x2b\xf7\x34\x68\x78"
      "\x8d\xf5\x17\x10\xd7\xd3\x1c\x63\x2f\xc5\xed\x17\x62\xeb\x0b\x42\x8e\xe7"
      "\x51\xc4\x7d\x8e\x89\x4f\x74\x5a\x86\x84\x04\xa0\xbf\x35\xf0\x12\x10\x08"
      "\xb7\x22\xb1\xea\xa6\xae\xdf\xa1\xbf\x2e\x7c\xcb\x2d\x61\xd5\xd7\x63\x31"
      "\x94\x5e\xce\xfa\x26\xb8\x47\x1d\x42\x64\x52\x88\xd7\x22\x6b\xbd\x9c\xcd"
      "\x62\x8a\xb8\x48\x75\xf2\xc5\x0b\xa8\x91\xce\xa5\x92\xb0\x43\x0a\x53\x7a"
      "\x39\x5d\xc7\x3b\xda\x36\x7b\xf1\x2c\xb7\xd8\x16\x91\xa5\xfe\x8c\x47\xbe"
      "\x39\x56\x56\xa2\x97\xe9\xdf\x0e\x71\xf9\x67\x56\xea\x5c\xce\x7d\xaa\xc4"
      "\xbe\x29\x01\x59\xf6\xbc\xd7\x5f\x0d\xda\x9d\xe5\x53\x2e\x71\xae\x9e\x48"
      "\xb0\xed\x02\x54\xa8\x31\x00\x00\x00\x00\xf6\xfb\xb8\x69\x60\x4d\x51\xa3"
      "\x6a\x54\xc8\x32\xe4\x5b\x25\x69\xdc\x0d\x90\xb0\x75\x22\x5f\xde\x44\xc4"
      "\xe0\x97\x31\x71\xad\x47\xd6\xb0\xfd\xf9\x74\x3a\xf9\x32\xcd\x6d\xb4\x9a"
      "\x47\x61\x38\x08\xba\xd9\x59\x71\x03\x00\x00\x00\x00\x00\x00\x00\x83\x2d"
      "\x0a\x45\xfa\x42\x42\xe2\x4c\x7e\x80\x00\x03\xc9\xe8\x09\x5e\x02\x98\x5f"
      "\x28\xe6\x78\xf6\x64\x22\x43\x6f\x94\x9e\x2a\xb8\xf1\x62\xd7\xe3\xf8\x55"
      "\xe3\x78\xf4\xa1\xf4\x0b\x0c\x6f\xb2\xd4\xb2\x05\xa8\x00\xb6\xd7\x13\xac"
      "\xeb\xc5\xb0\x14\xe6\x1a\x54\x3a\x5a\x19\x4f\x9a\xc1\x8d\x76\xb5\x44\x0e"
      "\x3b\x1a\x56\x9e\x73\x97\xf6\xca\xfa\x86\x96\x6d\x7b\xa1\x9e\x72\x04\x13"
      "\x26\x7a\x6c\xce\xa9\xc4\x39\x67\x1d\x2c\x68\x0f\x27\x53\xca\x18\x4e\xee"
      "\xb8\x43\x45\x03\x68\xac\xb4\x38\x3a\x01\xd2\x5e\xb3\xd1\xe2\x3e\x0f\x26"
      "\x45\xd1\xcd\xfa\x9f\xa4\x10\x63\x2f\x95\xa5\xf6\x22\xf8\x51\xc6\x6e\xe7"
      "\xe3\x03\x93\xcd\x7a\x4d\x67\xff\x2a\x49\xc4\xf9\x3c\x09\x84\xb5\xc2\xd4"
      "\x52\x34\x97\xe4\xd6\x4f\x95\xf0\x84\x93\x56\x4a\x1d\xf8\x71\x11\xc9\xbf"
      "\x31\x94\xfe\xf9\x7d\xce\xcc\x46\x7a\xce\x45\xfe\xeb\x68\x5c\x58\x70\xd0"
      "\x5f\x88\xa0\xf4\x63\xdb\x88\xd3\x77\x44\x2e\x13\x49\xac\xaf\x76\x62\x18"
      "\xb5\x4a\x9d\x62\x47\x78\xe1\xc4\xe0\x64\xc9\x8e\x49\x41\x98\x27\x6e\xb2"
      "\xdf\x77\x66\x41\x1b\xef\x0e\xbb\x50\x00\x00\x00\x00\x00\x60\x65\xd6\x35"
      "\xb0\xb7\xa0\x0e\xe7\x67\x22\x1d\x8a\xf9\x75\x33\x87\xe0\xcd\x8d\x71\x8f"
      "\x54\xa2\x9d\xf6\xeb\xa3\xbd\x4c\x44\x0e\x6e\x21\x72\xe3\xfc\xc0\x1b\x8b"
      "\xab\xb7\x57\xb5\xc5\x92\x17\xb8\x0d\x0d\xb3\xba\x58\x28\x14\xa6\x04\xe4"
      "\xef\x7a\x80\x3e\x9c\xa7\xc8\x5b\x35\xc9\xb9\x3a\x9e\x08\x85\xe2\x38\xb4"
      "\x4a\xe1\xc2\xe6\x4c\xce\x3b\x27\x08\x3b\x82\x46\x82\x9e\x64\x05\x60\x00"
      "\x30\x2b\xff\xff\x15\x40\x5b\xd5\xf2\xeb\xa2\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x9a\x98\x23\xfd\x8f\xbc\x5a"
      "\xa1\x65\x09\x94\x5e\xd0\x32\xb4\x8e\xa1\x2d\x8e\x05\x88\xdc\x52\x70\x2e"
      "\x40\x84\x91\x3a\x06\xd4\x68\xd0\x92\x8b\xad\x76\xd6\x97\xe1\xf8\x5a\xb0"
      "\x30\xe7\x88\xd3\x87\x88\xee\x5b\x54\x28\xd4\xa9\x71\xcc\x97\xdb\x9f\xd2"
      "\x31\x08\x8e\x57\x07\x35\xce\x12\x9e\x7e\x77\xfc\x27\x77\x69\x26\x64\xa1"
      "\x48\x8f\xd8\xd6\xdf\xf4\xda\xd6\x18\xfd\x54\xf5\x29\xd4\x55\x5c\x65\x07"
      "\x00\x9e\xe6\x9d\xd1\xbc\x55\x25\x87\x89\xb2\x40\x52\x13\x7e\x96\x37\xf3"
      "\xef\xba\xb7\x17\x20\xf8\x8c\x3c\x44\xb3\xb7\x48\x6f\x97\x9e\x8a\x31\x74"
      "\xb5\x31\xf5\x73\xfe\x0e\x52\x39\xc0\x00\xbe\x27\x33\xc4\x95\x46\xf6\xe8"
      "\xa9\x17\x5e\xc6\xf1\x4d\xbf\x72\xca\xc9\x16\x43\xb2\xfd\x99\xc2\x9e\xca"
      "\x28\xa3\xc2\xe6\x0d\x5e\x5b\x87\x95\xfa\xe1\x6a\x7c\x3e\xa5\x7e\x72\x8e"
      "\xca\x35\xea\xf0\x15\x5a\x39\xf9\x75\x80\xe0\x79\x17\x54\x26\xc0\x88\xa0"
      "\x20\x80\x40\x98\x2a\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x51\xce\xaa\xf0\x15\x9f\xe6\x1f\x2e\xad\xe7\x60\x3d\x0a\x7a\x56\xfb\x09"
      "\xcd\x11\x9a\xc0\x6a\xdb\x65\x97\x15\x5a\xe4\x78\x46\x89\x2b\xb4\x23\xc0"
      "\x24\xd8\xcb\xe9\x24\x0b\x71\xec\x6d\xc2\x12\x4d\x3a\x19\xe2\xd7\x14\xb2"
      "\x73\xd9\x5d\x1d\x3a\xa7\x37\xcb\x04\xa3\x36\x15\xff\x2a\x73\x0e\x51\x06"
      "\x7d\x5d\x67\x5d\x71\x22\x36\x1c\x37\xc6\x1a\x43\xb5\xaf\xd8\x65\xb6\x0d"
      "\x4c\xae\x89\x1b\x73\x22\x0f\x17\xd2\x59\x85\xa7\xf7\x68\x34\x99\x5e\x53"
      "\xa9\x3a\x1c\x7b\x9e\xef\x26\x7d\xf6\x91\xca\x98\x3a\x0b\x15\xbd\xa7\xf6"
      "\xc5\xc1\xca\x7a\xa5\x02\x61\xa3\x08\x9a\x1e\xbf\x07\x34\xc9\xb0\x7e\x89"
      "\x51\xff\x02\x32\x63\xad\x5a\xed\x8c\xfb\x49\xb4\x9e\x12\x8c\x69\x77\x24"
      "\xc0\x57\xd2\x2c\x5d\xf5\xae\xf2\x7c\xe3\xdb\x11\xd5\xad\x55\x27\xd1\x49"
      "\xd0\x76\xe1\xa8\x7e\x2d\xf2\x7c\x0c\xb8\xa6\x7a\xd0\x26\xbf\x95\x3e\x88"
      "\xf1\x04\x47\xe1\x25\xc2\xc0\xf1\xae\xbe\xe1\xf3\x39\x0a\x9e\x3d\xda\xd4"
      "\xe2\xa6\xe0\xf6\xe4\x56\x9f\xde\xfa\x19\xe8\x70\xe0\x4a\xcf\x94\x93\xb9"
      "\x63\xf9\x8e\x23\xcf\xc6\x65\xe4\xf4\x65\xfa\x3f\x80\x1e\x19\x57\xc3\x99"
      "\xe4\x5f\x61\xd3\x45\x9b\x1c\x60\x62\x04\x36\x8b\xb9\x31\x34\x5a\xf2\x82"
      "\x3c\x48\x7d\x2f\xd9\x9d\xb6\xea\x6e\x00\x8e\x7f\xfa\x06\xca\x86\x15\x51"
      "\x18\x9d\x15\x5b\xd0\x77\xa7\x9f\xe2\xc7\xe9\x61\x35\x2e\x56\x82\x4f\x72"
      "\x7d\x21\xd4\x1e\xae\x78\xbf\xec\x4a\x2d\x7a\x7e\xdb\xc8\xef\x95\x8c\x5e"
      "\xa5\x99\xf7\xc2\x5b\xf7\x1c\x23\x40\x55\x8a\xa1\x2f\xdd\x24\xa8\x8a\xaa"
      "\xd5\x92\x1a\xee\x7d\xae\x6a\x2f\x30\x09\xd9\xcb\x43\xab\x48\x98\xd0\xf0"
      "\xaa\x56\x54\x31\xb6\xab\xe5\x85\xd7\x5d\xb0\x4d\x1c\x9b\xa0\xb9\xde\x4a"
      "\xe8\xb0\xd3\x13\x2b\xc6\x81\x0c\xc9\xa6\x93\x97\x9f\x55\x17\x4a\x72\xe1"
      "\xdf\x9f\xde\xf3\x5b\xc4\x70\xf9\xe6\xe5\x91\x98\x27\x57\xf4\x5c\x52\xc6"
      "\x45\xd8\x91\xbf\x63\xbb\x21\xfb\x66\x92\x6e\xbe\x1a\x85\x25\x61\x1f\xc3"
      "\xe8\xbb\x87\x95\xc3\x6d\xc2\xa8\x6b\x5a\xb4\x6f\xf3\x3c\xc7\x4f\x61\x75"
      "\x1b\x2d\xae\x92\x67\x6d\xb8\x5c\x8d\x0c\x72\x1b\x7e\xa4\x54\x4b\xf5\x1c"
      "\x95\xc8\x6f\xca\xc1\xf4\x34\xd0\x9d\x1e\xe4\x92\x8a\xaf\xe2\x3d\xe6\x6f"
      "\xed\x97\x2e\x0d\xdd\xfb\x33\xf6\x4e\x48\x70\x1b\x04\x92\x39\xe7\xf5\x52"
      "\xd8\x16\x44\x1d\x11\xc4\xc2\x64\x7c\x01\x44\x62\x34\x43\x59\x19\x8d\x97"
      "\xc4\xb6\xe9\xed\x31\xca\x18\x98\x7b\x64\xde\x07\x9b\x2b\xed\x64\x1e\x8a"
      "\x92\xf1\x3c\xa7\x08\x44\xc6\x5c\xb4\x23\xd0\x19\x50\xb0\xeb\xf4\x4b\xd2"
      "\x8e\x09\xc0\x5d\x9a\xe5\xdd\x68\x9f\xb8\x80\xfb\x18\xd0\x42\x21\x9f\x5a"
      "\xc6\x0c\x3a\x03\xb0\x85\xab\xf3\xe8\xe3\xef\xc8\x42\xa8\xd3\x28\x73\x34"
      "\x61\xf0\x4c\x99\x60\x70\x61\xc6\x5e\xd1\x4c\x61\x32\x2a\x5a\xc2\xd3\x71"
      "\xa9\x5b\x8a\xd8\x67\xec\x92\xd1\x3a\x4f\xa4\xae\x03\x3a\x09\x67\x38\x66"
      "\xcd\x77\xf4\xbc\xda\xaa\x05\x20\x71\x66\xb1\x9a\x87\x58\xd8\x85\x54\x00"
      "\xd8\xc6\xa7\x24\x2d\xc2\x07\x25\x1e\x87\x97\xec\xa2\x4e\xa4\xf4\x87\x66"
      "\x3e\x60\xf2\xf5\xe1\xf1\x42\x49\x58\xfd\x14\x8f\x84\x68\x30\xe8\x8a\x42"
      "\xd9\x3e\x1f\xe9\xc0\xb4\xa4\xa2\x68\x92\x17\x38\x93\x8a\xa9\xf3\xcb\x38"
      "\x11\xac\x87\xc5\x4c\x8e\xbc\x8b\xcf\xb4\x61\x3c\xc3\xa9\x97\xff\x15\x79"
      "\xed\xbd\x4a\xde\x80\x20\xe3\xad\x00\x1b\x07\x2b\x1a\x75\x1b\x58\x8a\xc4"
      "\x63\x9f\x35\xa5\x8e\x00\xa5\x0c\x02\x70\x60\x8c\x7a\x7f\x10\x13\x2b\x1c"
      "\x25\xb9\xea\x81\x23\x2f\xbe\xf6\x65\xf6\x21\x2f\x87\x5b\x2a\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xcf\x7b\x6c\x4b\xa9\xbe\xc1"
      "\x53\xd6\x83\x4b\xfe\xf0\x80\xdf\x37\x47\x03\xa8\xff\x56\xa6\x3e\xc1\xfe"
      "\x5f\x2e\x05\xa7\x9e\x3c\xac\xe7\x28\x3d\xd6\x8d\x41\xe9\x44\x20\xc3\x25"
      "\xfe\x4d\xae\x14\x4f\xde\x5e\xc2\x5a\x87\xd6\x25\xca\xb2\x07\x53\xa7\x7b"
      "\x32\x3f\xa3\x78\x3c\x8b\x67\x58\x59\xb9\x01\x26\x47\x88\x5a\x24\x2a\xdf"
      "\xee\x2f\xe8\x12\xec\xbe\x51\x91\xe0\xa1\x51\x42\xf7\x34\x9e\x76\x27\xcc"
      "\x39\xd7\x24\xe2\xe3\x4e\x7a\x24\x15\x4f\x26\xae\x31\x25\xb3\x6d\x05\x04"
      "\x96\x52\x95\xd0\x45\x39\x02\xac\x70\x79\xb1\x1a\x3a\x1e\x65\x5e\x48\x23"
      "\x31\xe3\xdc\x35\xb2\xe7\xe4\xe3\xea\x99\x06\x4f\xe5\xb9\xc8\xae\x0c\xa3"
      "\xe5\xfd\x65\x3f\x32\x86\xa9\x9d\x81\xce\x4e\xba\x76\x5c\x38\xd0\x97\x39"
      "\x1a\xd4\xba\xba\xc3\x8c\xe5\xb4\x34\x4e\x24\xa3\x61\xcd\x54\xe5",
      3778);
  *(uint64_t*)0x200000000090 = 0x200000000380;
  memcpy((void*)0x200000000380, "GPL\000", 4);
  *(uint32_t*)0x200000000098 = 0;
  *(uint32_t*)0x20000000009c = 0;
  *(uint64_t*)0x2000000000a0 = 0;
  *(uint32_t*)0x2000000000a8 = 0;
  *(uint32_t*)0x2000000000ac = 0;
  memset((void*)0x2000000000b0, 0, 16);
  *(uint32_t*)0x2000000000c0 = 0;
  *(uint32_t*)0x2000000000c4 = 0;
  *(uint32_t*)0x2000000000c8 = -1;
  *(uint32_t*)0x2000000000cc = 8;
  *(uint64_t*)0x2000000000d0 = 0x200000000000;
  *(uint32_t*)0x200000000000 = 0;
  *(uint32_t*)0x200000000004 = 0;
  *(uint32_t*)0x2000000000d8 = 8;
  *(uint32_t*)0x2000000000dc = 0x10;
  *(uint64_t*)0x2000000000e0 = 0x200000000000;
  *(uint32_t*)0x200000000000 = 0;
  *(uint32_t*)0x200000000004 = 0;
  *(uint32_t*)0x200000000008 = 0;
  *(uint32_t*)0x20000000000c = 0;
  *(uint32_t*)0x2000000000e8 = 0x10;
  *(uint32_t*)0x2000000000ec = 0;
  *(uint32_t*)0x2000000000f0 = -1;
  *(uint32_t*)0x2000000000f4 = 0;
  *(uint64_t*)0x2000000000f8 = 0;
  *(uint64_t*)0x200000000100 = 0;
  *(uint32_t*)0x200000000108 = 0x10;
  *(uint32_t*)0x20000000010c = 0;
  *(uint32_t*)0x200000000110 = 0;
  res =
      syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x200000000080ul, /*size=*/0x2eul);
  if (res != -1)
    r[2] = res;
  *(uint64_t*)0x200000000180 = 0x200000000540;
  memcpy((void*)0x200000000540, "rcu_utilization\000", 16);
  *(uint32_t*)0x200000000188 = r[2];
  *(uint32_t*)0x20000000018c = 0;
  *(uint64_t*)0x200000000190 = 0;
  syscall(__NR_bpf, /*cmd=*/0x11ul, /*arg=*/0x200000000180ul, /*size=*/0x10ul);
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffffffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul,
          /*fd=*/(intptr_t)-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200000000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul,
          /*fd=*/(intptr_t)-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200001000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul,
          /*fd=*/(intptr_t)-1, /*offset=*/0ul);
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
