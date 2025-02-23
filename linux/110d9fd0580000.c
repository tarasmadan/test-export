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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  syscall(__NR_perf_event_open, /*attr=*/0ul, /*fd=*/-1, /*cpu=*/0ul,
          /*group=*/-1, /*flags=*/1ul);
  syscall(__NR_socket, /*domain=*/0x25ul, /*type=*/1ul, /*proto=*/0);
  syscall(__NR_socket, /*domain=*/0x10ul, /*type=SOCK_DGRAM*/ 2ul, /*proto=*/0);
  syscall(__NR_bpf, /*cmd=*/0ul, /*arg=*/0ul, /*size=*/0ul);
  *(uint32_t*)0x20000500 = 2;
  *(uint32_t*)0x20000504 = 0x80;
  *(uint8_t*)0x20000508 = 0x4e;
  *(uint8_t*)0x20000509 = 1;
  *(uint8_t*)0x2000050a = 0;
  *(uint8_t*)0x2000050b = 0;
  *(uint32_t*)0x2000050c = 0;
  *(uint64_t*)0x20000510 = 0x210e;
  *(uint64_t*)0x20000518 = 0x80;
  *(uint64_t*)0x20000520 = 3;
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 5, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 29, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 30, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 31, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 32, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 33, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 34, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 35, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 36, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 37, 1);
  STORE_BY_BITMASK(uint64_t, , 0x20000528, 0, 38, 26);
  *(uint32_t*)0x20000530 = 0;
  *(uint32_t*)0x20000534 = 2;
  *(uint64_t*)0x20000538 = 0x7fff;
  *(uint64_t*)0x20000540 = 0xaea;
  *(uint64_t*)0x20000548 = 0x110104;
  *(uint64_t*)0x20000550 = 0x32;
  *(uint32_t*)0x20000558 = 0;
  *(uint32_t*)0x2000055c = 0;
  *(uint64_t*)0x20000560 = 0;
  *(uint32_t*)0x20000568 = 0;
  *(uint16_t*)0x2000056c = 4;
  *(uint16_t*)0x2000056e = 0;
  *(uint32_t*)0x20000570 = 0;
  *(uint32_t*)0x20000574 = 0;
  *(uint64_t*)0x20000578 = 9;
  res = syscall(__NR_perf_event_open, /*attr=*/0x20000500ul, /*pid=*/0,
                /*cpu=*/-1, /*group=*/-1,
                /*flags=PERF_FLAG_FD_CLOEXEC|PERF_FLAG_FD_NO_GROUP*/ 9ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000080 = 5;
  *(uint32_t*)0x20000084 = 3;
  *(uint64_t*)0x20000088 = 0x20001200;
  memcpy(
      (void*)0x20001200,
      "\x85\x00\x00\x00\x2a\x00\x00\x00\x25\x00\x00\x00\x00\x00\x00\x00\x95\x00"
      "\x00\x00\x00\x00\x00\x00\xaf\xcd\x48\xd6\x49\x4d\x61\x4d\xcc\x6f\xab\x53"
      "\x35\xec\x47\x2d\xb2\xc6\x16\x1d\xba\x39\x21\x76\xdd\x29\x63\x03\x8e\x1d"
      "\x69\xba\x7e\xa9\x4c\x50\x0d\xc4\xef\x2f\xad\x96\xed\x40\x6f\x21\xca\xf5"
      "\xad\xcf\x92\x05\x69\xc0\x0c\xc1\x19\x96\x84\xfa\x7c\x93\x83\x6d\x9e\xa2"
      "\xcf\xb0\xe6\x04\x36\xe0\x54\x25\xcc\x62\x6b\x42\x4d\xa1\xe8\xc8\x25\x35"
      "\x78\x61\xaa\x50\x05\x46\x86\xb0\x66\x70\x7d\xe9\x4a\x4f\x4d\x5f\xc7\x9c"
      "\x98\x7d\x66\x9f\x38\x1f\xac\xa0\xf9\xd9\x92\x4b\xe4\x1a\x91\x69\xbd\xfa"
      "\xf1\x6d\xa9\x15\xb2\xe2\x49\xee\x1c\x6e\xee\x84\x30\x9e\x7a\x23\xc1\x9a"
      "\x39\x48\x48\x09\x53\x9f\xcb\x4e\x0b\x6e\xab\x1a\xa7\xd5\x55\x45\xa3\x4e"
      "\xff\xa0\x77\xfa\xa5\x5c\x59\xe8\x82\x54\xf5\x40\x77\xf7\x99\xbf\x4d\x35"
      "\xb2\x13\xbd\xa8\x4c\xc1\x72\xaf\xd8\xcc\x2e\x47\xa7\xd8\xb8\x5a\x5e\x3d"
      "\x77\xac\x46\x39\x20\xe2\x31\xb7\xae\x0d\xa8\x61\x6d\x2b\x79\x58\xf9\x1f"
      "\x5d\xa6\xc0\x25\xd0\x60\xab\x18\x6d\x94\xaf\x98\xaf\x1d\xa2\xb5\x95\x2e"
      "\xb1\x58\x55\x93\x3a\x21\x23\x04\xe0\x35\xf7\xa3\x5d\xfc\x72\xc8\x12\x56"
      "\xa5\x5a\x25\xf8\xfe\x3b\x01\x00\x00\x00\x00\x00\x00\x00\xb0\x25\x5f\x34"
      "\x71\x60\xac\x83\x07\x00\x00\x00\x00\x00\x00\x40\x15\xcf\x10\x45\x3f\x6c"
      "\x0b\x97\x3b\x81\xa4\x84\xeb\xad\x04\x85\x9d\x92\x83\x65\xa7\xea\x3f\xab"
      "\x8b\x4b\x38\x0a\x00\xd7\x2b\xc0\x48\x0f\x94\x9c\x47\x97\x57\x30\x67\x20"
      "\x39\x93\x79\xd9\x27\x1c\xf5\x55\xc1\x4d\x56\xb5\x1c\x22\x98\x23\x7b\xeb"
      "\xfc\x08\xe0\xd5\x97\x6a\x94\x2b\x84\x69\x70\xcf\xd9\x8b\x9d\x41\x39\xf1"
      "\x11\x1f\x2d\xc5\xe4\x6a\xc1\xc6\x0a\x9b\x03\x00\x74\xbf\xbc\xd4\xb0\x90"
      "\x12\x17\x54\x84\x13\x5f\x0e\x51\x9f\x0b\x1e\x4a\xaa\x02\x6d\x57\x0e\xcb"
      "\x5e\x8c\xdd\xbe\xd6\x5f\xf7\x02\x00\x00\xee\xa2\xff\x4f\x8a\x4c\xf7\x96"
      "\xb0\x7a\x6f\xf6\x1c\x55\x52\x41\x7f\xd7\x03\xf7\xf1\x4d\x8b\x78\xa6\x02"
      "\xca\x3c\xdf\x6a\x66\x2d\x8b\xc9\xc8\x9c\x91\x20\x07\x29\x13\x15\x2c\x84"
      "\x5c\xf5\x72\xcf\x39\x31\x0d\x52\x2a\x5d\x00\xdc\xdd\x85\x95\x35\x6c\x9b"
      "\x24\x92\xaa\xf1\x26\x4d\x4e\xf4\xa4\x10\xc8\x82\x83\x48\x67\xbc\xd2\xb6"
      "\xe5\x58\xd1\x78\x79\x57\x0c\x8a\xba\xfe\x4f\x0f\x6e\xa5\x08\x00\x00\x00"
      "\xa0\xc5\x48\x55\x2b\x57\x1b\xed\x56\x47\x32\x3c\x78\xa9\x96\x81\x00\x00"
      "\x00\x05\x71\xcb\xb1\x7d\x9f\x37\x28\x24\x62\xf0\xe9\xc1\x47\xc0\xd4\x97"
      "\xc6\x14\x33\xc6\xcc\xc3\x56\x01\xee\xf9\x7e\xe6\x11\xbe\x8c\x97\xf4\x15"
      "\x1f\xcd\xa6\xcb\x79\x9c\x6e\x92\x49\x66\xa7\xf9\x0b\xf8\xfd\x1e\x75\xee"
      "\x76\xbd\x72\x34\x6c\xfb\xb5\x26\x89\x0a\xa7\xfe\x5e\x68\x94\x9a\x3b\x30"
      "\x56\x7e\x54\xd3\x50\x47\x23\x17\x7d\x35\x6c\x46\x04\xbc\xa4\x92\xec\xec"
      "\x37\xe8\x3e\xfc\xee\xfd\x7c\xa2\x53\x36\x59\xed\xc8\xbe\x05\xcc\x85\x45"
      "\x1c\x6a\x14\x50\x74\x34\x3c\xae\xa5\xc4\xbf\x69\x04\x41\x97\x4b\x15\x5f"
      "\x5a\xdc\x68\x1a\x03\xc0\xbb\xb8\x35\x88\x56\x17\x5e\x2c\xe8\xb0\xcb\xbb"
      "\xe3\xc0\x33\xe5\x4f\xfc\xeb\xde\x1d\x9d\x3d\x35\x00\x00\x00\x00\x00\x00"
      "\x00\x00\xe0\xf2\x09\x15\x0a\x07\x68\x2c\x4e\x14\xe3\xa8\x35\x58\xdf\x6f"
      "\x3f\xc9\x7f\x17\x30\xa1\x36\xbd\xee\x07\xe9\x8c\xb9\x84\xb2\xe2\x30\x4a"
      "\x1b\x63\xaf\xef\xdb\x63\x6e\x52\x51\xaa\xe4\xe6\x21\x36\x57\x4b\xc6\x37"
      "\x1a\x0b\xb2\xbe\x1a\x96\x2a\xae\x9c\x12\x58\xda\x6e\xf5\x90\xe1\xd8\x5e"
      "\xa9\xe1\x2b\x30\x25\xf4\x3e\x7e\x08\xcc\xff\xc5\x06\x4d\xea\x4c\x39\xcf"
      "\x4b\x98\xe1\xfc\x6e\xfb\x59\x78\xf5\x1e\x16\xb6\x78\xec\xa0\xb6\x58\x8f"
      "\x60\x08\x94\x8e\x56\x1a\x98\x45\xe4\xff\x29\xe2\xbd\xb1\xd0\xb9\x23\xb2"
      "\x72\x34\x1c\x5e\x09\x3f\xd6\x6a\x29\x46\x50\x15\x59\x33\x57\x81\x09\x2c"
      "\xf8\xce\x98\x7c\x56\xcd\x31\x12\x16\x24\xd7\x45\x5f\x2a\x36\x66\x27\x6c"
      "\x3c\x0e\x81\x2b\x28\xe2\xf3\x0d\x03\x5c\xee\x5d\x0e\x77\xa3\xc7\x22\x08"
      "\xec\x65\x1c\xc0\xae\x63\x7f\xa4\x74\x81\x6b\xc5\x9d\x2e\x2a\x00\x09\x24"
      "\x19\x30\x4b\x33\x8a\x98\x7e\x9d\x30\x44\xd8\x56\xce\x24\xf3\x70\x03\x0b"
      "\xe3\xb5\xf7\x9f\x03\x0b\x8d\x3e\xbc\xe6\x86\x63\xef\x5a\xf4\x69\xab\xe7"
      "\x53\x31\x4f\xae\x31\xa0\x44\x58\x59\xa5\xec\xe8\xfb\x11\xa4\xee\x8e\x46"
      "\x35\x4c\x9c\x3a\x04\x1a\x1e\x7b\x55\xc4\xe8\x1d\xba\x1e\x12\x28\x9e\xe3"
      "\x44\x63\xaa\xf2\x83\x45\xbd\xe0\xc1\x95\xbc\x9f\x02\x2c\xa8\xce\x37\xed"
      "\x85\x46\x4c\x31\x67\x90\x53\xe7\xf9\xd0\x4b\xb5\xcb\x51\xda\x0b\x79\x58"
      "\x98\x9f\xd7\x0f\x24\x12\x62\xd0\xaf\x32\x46\xeb\x4f\xc4\xbd\xa3\x45\x36"
      "\x02\x00\x00\xfb\xdd\xea\xcd\x3a\xda\xa4\xd2\x71\x5e\x21\xc7\x72\xcc\xd4"
      "\x43\x41\xf7\xfd\x53\xdf\x58\xae\x79\x1e\xe8\xb4\x89\xa7\xc9\xef\xe3\x62"
      "\x5a\x9d\x97\x1b\x59\x97\x48\x5d\x6a\x06\x3d\xc6\xf7\x35\x9e\x2e\xcc\xc2"
      "\xfb\x39\xd4\x19\xde\x1a\x7b\x5c\x9d\xc2\x2c\x96\x29\x5a\x46\x01\xad\xf5"
      "\x9d\x44\xe5\x8e\xb1\xc6\x0b\x34\x75\xbe\x31\xa9\xb7\xcf\x42\xb6\x40\x23"
      "\x12\xd2\x72\x5b\x8d\x9f\xa7\x00\xa8\x64\x07\xe7\x9a\xe2\x9d\x2c\x11\x7c"
      "\xa6\x5f\xc8\x6c\x2d\xce\x97\xaa\x03\x27\x9a\x66\xec\x87\x12\x22\x19\xb0"
      "\xf7\x96\xab\x92\xb1\xad\xec\xae\x50\xfd\xb4\x08\xc8\xa8\x0f\x7f\x02\xf7"
      "\x50\xd6\xc9\x77\xa1\x91\x9f\x9f\x69\xa6\xcf\xef\xdf\x87\x9d\x44\x7d\xf5"
      "\x3f\x3b\x9b\x70\xd1\x03\x55\xb0\x74\x66\xd1\xef\x00\x56\xb5\xaf\x55\x3d"
      "\x18\xa6\xcc\x50\xfe\xeb\x7b\xf8\xd9\xb7\xbe\x32\x83\xb6\x45\x0d\x26\x4e"
      "\x77\x12\xd2\xf1\xd7\x00\x45\x48\xb1\x91\x62\xce\xf0\x4d\x18\xd4\xf5\x98"
      "\x7b\xaa\xb9\x7a\x9b\xfb\xd8\xf1\x85\xb5\x63\x18\x20\x42\x0b\x75\xb6\x52"
      "\x2c\x0e\x21\xc8\x82\xc6\x6f\x4f\x25\xff\xb6\xd9\x5e\x07\xde\x02\x20\x5f"
      "\xca\x4f\x18\xa2\xeb\x5b\x63\xe4\x5d\x5d\x80\xfe\x52\x73\x40\x93\xae\x5a"
      "\xa3\xc0\xb4\xf3\xf4\x5b\xff\xf2\x01\x00\x00\x00\x00\x00\x00\x00\x2e\x31"
      "\x56\x0e\x5b\x74\x14\x45\xea\x2a\x1a\xce\xe2\xe9\x8c\x9f\x34\x27\x83\x4b"
      "\xa0\xa7\x65\xd2\x0b\x30\xf8\x7a\xf9\x76\xa4\x6f\x9a\x9a\x1a\xc7\xde\xa1"
      "\xea\x68\x45\xf9\xaa\x66\x23\x7e\x0d\xac\xc1\x07\xf5\x32\x34\x8c\xc2\x11"
      "\x64\x73\x38\x1e\x96\x1f\x3d\x9c\x8c\x21\x57\x8f\xe3\x24\x50\x97\xc2\x80"
      "\xab\xe5\x14\x27\xb9\xf6\xcd\x72\xb5\xda\x6d\x02\x52\x80\x3c\x66\x73\x0c"
      "\xd5\xea\xc9\x07\xf0\x9b\x96\x95\x90\x63\x13\xf8\x87\x35\x22\x60\x8c\x6f"
      "\xc0\x1e\x1b\x9e\x16\x58\x7b\xb5\xf7\x21\x30\x3e\x6b\x89\xe5\xc5\x4d\x68"
      "\x0a\xc6\x6d\x09\xaf\x90\xdb\xf5\x0e\xe6\x9a\x39\x26\x59\x64\x27\x9d\x17"
      "\x4b\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xfa\x08\xad\x07\x31\xba"
      "\x49\xfb\xf9\x81\xf8\x26\x5e\x7f\x1f\x4c\x2d\x97\xf4\x68\x0b\x13\x5f\x87"
      "\xc2\x28\xce\x69\x41\x8a\x28\x2b\x6c\xaa\x24\x81\xa0\xdf\x17\x74\xfa\x7d"
      "\x94\x94\x4b\xb9\x2d\x2b\x89\xf7\x3f\x0e\x8b\x63\xf6\x31\x6c\x57\x62\xf3"
      "\x28\x8b\xc9\x70\x72\x0f\x48\xb5\x64\x7d\xd1\x77\xc1\x68\x10\xfa\xe0\x53"
      "\x34\x96\x09\x00\x00\x00\x00\x00\x00\x00\x9a\x74\x38\x97\x8c\x54\x65\x11"
      "\x3f\x66\x8e\xb4\x48\x43\x50\x04\x82\x89\xd0\x7d\xbe\xf3\x25\xd3\x22\x1a"
      "\x7c\xb3\x5f\x81\x00\x25\x79\x41\xa9\x78\x1e\x32\x14\xc2\xa3\xdc\xf8\x9d"
      "\x99\x84\x4b\x76\x2a\x9c\xf1\x75\x48\xc5\x4f\xcc\xad\x2c\x7a\xe8\x07\x2b"
      "\x82\xe0\x88\x08\x15\xda\xf9\x66\xbd\x53\x43\xc1\x63\x5e\x12\x3f\x86\x8a"
      "\x71\x67\xcf\xcf\xf3\x33\x84\x25\x3a\xf5\x70\xf4\xef\x9c\x02\x54\xaf\xdd"
      "\x89\xc7\x39\x43\x56\x2b\x53\x0d\xd8\x8d\xa8\xa9\x40\x13\xbb\xaf\x20\x4b"
      "\xeb\xc3\x80\x55\xad\xc3\x9f\x07\xf7\xc2\x27\x11\xf4\xd1\xf6\xdc\xc9\x28"
      "\xd1\x57\x8a\x09\x3c\x07\x2e\x0b\x92\xba\xbc\x76\xf4\x7e\xe3\x67\xe7\x45"
      "\xa0\x24\xa2\x27\x83\x19\xd9\xa4\xd1\x37\x84\x82\xb7\x03\x04\x66\x9c\x44"
      "\x7c\x71\xca\x4d\x54\xc8\x23\x95\xa3\x95\x8d\x57\x6c\x42\xc0\x8a\x4d\x5a"
      "\xdf\xb5\x83\x06\x16\x4c\xc7\xd8\x70\xb8\x81\xf8\x08\x4a\x3d\x18\x5a\x63"
      "\xc6\xb0\x52\x92\x18\x60\x95\xc1\xf4\x07\xce\x74\x29\x7d\x16\x47\x09\x88"
      "\xf1\x64\x7f\x7b\x6f\x6c\xdc\x6a\xb8\xbe\x3c\xac\xc3\x25\xdf\x96\x3c\x2c"
      "\xb8\x0c\xfe\x07\xde\xd6\xd5\x5f\x55\x6b\xe0\xa3\xdf\xa8\x5f\x0a\x0a\xce"
      "\x87\x9b\x0a\x0a\x95\xcd\x07\xb6\x6f\xbb\xc7\x3d\x09\x45\xbe\xeb\xe8\x7a"
      "\x21\xdd\x46\xfd\x58\x04\xcd\x63\xc0\x11\x99\xc7\x8b\x1d\x77\x4b\x17\x68"
      "\x6f\xe3\xae\xad\xeb\xc4\xf3\xd2\xe6\xaf\x11\x10\x46\x6f\xec\xf4\x13\x84"
      "\xf1\xb5\xc9\x65\x31\x70\x0d\xb5\xae\xfa\x1a\x5c\x17\xa9\xeb\xca\xf3\x34"
      "\x11\x0e\xd5\x82\x99\x92\x08\xcc\x7e\xf9\x77\xce\xb2\xf8\xa5\xaa\x7d\x00"
      "\x00\x00\x00\x00\x00\x00",
      1914);
  *(uint64_t*)0x20000090 = 0x20000000;
  memcpy((void*)0x20000000, "GPL\000", 4);
  *(uint32_t*)0x20000098 = 5;
  *(uint32_t*)0x2000009c = 0x252;
  *(uint64_t*)0x200000a0 = 0x2000cf3d;
  *(uint32_t*)0x200000a8 = 0;
  *(uint32_t*)0x200000ac = 0;
  memset((void*)0x200000b0, 0, 16);
  *(uint32_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c4 = 0;
  *(uint32_t*)0x200000c8 = -1;
  *(uint32_t*)0x200000cc = 8;
  *(uint64_t*)0x200000d0 = 0;
  *(uint32_t*)0x200000d8 = 0;
  *(uint32_t*)0x200000dc = 0x10;
  *(uint64_t*)0x200000e0 = 0;
  *(uint32_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000ec = 0;
  *(uint32_t*)0x200000f0 = -1;
  *(uint32_t*)0x200000f4 = 0;
  *(uint64_t*)0x200000f8 = 0;
  *(uint64_t*)0x20000100 = 0;
  *(uint32_t*)0x20000108 = 0x10;
  *(uint32_t*)0x2000010c = 0;
  *(uint32_t*)0x20000110 = 0;
  res = syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x20000080ul, /*size=*/0x48ul);
  if (res != -1)
    r[1] = res;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x40042408, /*prog=*/r[1]);
  syscall(__NR_bpf, /*cmd=*/8ul, /*arg=*/0ul, /*size=*/0x10ul);
  syscall(__NR_socket, /*domain=*/0x10ul, /*type=SOCK_DGRAM|0x1*/ 3ul,
          /*proto=*/0x10);
  syscall(__NR_sendmsg, /*fd=*/-1, /*msg=*/0ul, /*f=*/0ul);
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
