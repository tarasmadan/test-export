// https://syzkaller.appspot.com/bug?id=fdb203aad0287c069dfbf8c6d518285c5b4fb64d
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

#ifndef __NR_ioctl
#define __NR_ioctl 54
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_openat
#define __NR_openat 295
#endif
#ifndef __NR_write
#define __NR_write 4
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  memcpy((void*)0x20009480, "/dev/mixer\000", 11);
  res = syscall(__NR_openat, 0xffffff9c, 0x20009480, 0, 0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_ioctl, (intptr_t)r[0], 0x80254d18, 0);
  memcpy((void*)0x20000000, "/proc/asound/card0/oss_mixer\000", 29);
  res = syscall(__NR_openat, 0xffffff9c, 0x20000000, 0x2002, 0);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000080, "TREBLE", 6);
  memcpy((void*)0x20000086, " \'", 2);
  memcpy((void*)0x20000088, "Mic Capture Switch", 18);
  memcpy((void*)0x2000009a, "\' ", 2);
  sprintf((char*)0x2000009c, "%020llu", (long long)0);
  *(uint8_t*)0x200000b0 = 0xa;
  memcpy((void*)0x200000b1, "VIDEO", 5);
  *(uint8_t*)0x200000b6 = 0xa;
  memcpy((void*)0x200000b7, "VOLUME", 6);
  *(uint8_t*)0x200000bd = 0xa;
  memcpy((void*)0x200000be, "MONITOR", 7);
  memcpy((void*)0x200000c5, " \'", 2);
  memcpy((void*)0x200000c7, "Master", 6);
  memcpy((void*)0x200000cd, "\' ", 2);
  sprintf((char*)0x200000cf, "%020llu", (long long)0);
  *(uint8_t*)0x200000e3 = 0xa;
  memcpy((void*)0x200000e4, "LINE", 4);
  *(uint8_t*)0x200000e8 = 0xa;
  memcpy((void*)0x200000e9, "ALTPCM", 6);
  *(uint8_t*)0x200000ef = 0xa;
  memcpy((void*)0x200000f0, "TREBLE", 6);
  memcpy((void*)0x200000f6, " \'", 2);
  memcpy((void*)0x200000f8, "Master Playback Switch", 22);
  memcpy((void*)0x2000010e, "\' ", 2);
  sprintf((char*)0x20000110, "%020llu", (long long)0);
  *(uint8_t*)0x20000124 = 0xa;
  memcpy((void*)0x20000125, "SYNTH", 5);
  *(uint8_t*)0x2000012a = 0xa;
  syscall(__NR_write, (intptr_t)r[1], 0x20000080, 0xab);
  memcpy((void*)0x20000000, "/proc/asound/card0/oss_mixer\000", 29);
  res = syscall(__NR_openat, 0xffffff9c, 0x20000000, 0x2002, 0);
  if (res != -1)
    r[2] = res;
  memcpy((void*)0x20000080, "VIDEO", 5);
  *(uint8_t*)0x20000085 = 0xa;
  memcpy((void*)0x20000086, "RECLEV", 6);
  memcpy((void*)0x2000008c, " \'", 2);
  memcpy((void*)0x2000008e, "CD Capture Switch", 17);
  memcpy((void*)0x2000009f, "\' ", 2);
  sprintf((char*)0x200000a1, "%020llu", (long long)0);
  *(uint8_t*)0x200000b5 = 0xa;
  memcpy((void*)0x200000b6, "PCM", 3);
  memcpy((void*)0x200000b9, " \'", 2);
  memcpy((void*)0x200000bb, "Master", 6);
  memcpy((void*)0x200000c1, "\' ", 2);
  sprintf((char*)0x200000c3, "%020llu", (long long)0);
  *(uint8_t*)0x200000d7 = 0xa;
  memcpy((void*)0x200000d8, "OGAIN", 5);
  *(uint8_t*)0x200000dd = 0xa;
  memcpy((void*)0x200000de, "LINE2", 5);
  memcpy((void*)0x200000e3, " \'", 2);
  memcpy((void*)0x200000e5, "Synth", 5);
  memcpy((void*)0x200000ea, "\' ", 2);
  sprintf((char*)0x200000ec, "%020llu", (long long)0);
  *(uint8_t*)0x20000100 = 0xa;
  memcpy((void*)0x20000101, "PCM", 3);
  memcpy((void*)0x20000104, " \'", 2);
  memcpy((void*)0x20000106, "Synth Capture", 13);
  memcpy((void*)0x20000113, "\' ", 2);
  sprintf((char*)0x20000115, "%020llu", (long long)0);
  *(uint8_t*)0x20000129 = 0xa;
  memcpy((void*)0x2000012a, "MONITOR", 7);
  memcpy((void*)0x20000131, " \'", 2);
  memcpy((void*)0x20000133, "Line", 4);
  memcpy((void*)0x20000137, "\' ", 2);
  sprintf((char*)0x20000139, "%020llu", (long long)0);
  *(uint8_t*)0x2000014d = 0xa;
  syscall(__NR_write, (intptr_t)r[2], 0x20000080, 0xce);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000, 0x1000, 0, 0x32, -1, 0);
  syscall(__NR_mmap, 0x20000000, 0x1000000, 7, 0x32, -1, 0);
  syscall(__NR_mmap, 0x21000000, 0x1000, 0, 0x32, -1, 0);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}