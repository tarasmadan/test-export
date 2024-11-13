// https://syzkaller.appspot.com/bug?id=9390e51ab2a97c5d58ff3cbaeef96b286e337784
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

static void exitf(const char* msg, ...)
{
  int e = errno;
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit(kRetryStatus);
}

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
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

static int inject_fault(int nth)
{
  int fd;
  char buf[16];

  fd = open("/proc/thread-self/fail-nth", O_RDWR);
  if (fd == -1)
    exitf("failed to open /proc/thread-self/fail-nth");
  sprintf(buf, "%d", nth + 1);
  if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
    exitf("failed to write /proc/thread-self/fail-nth");
  return fd;
}

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res = 0;
  *(uint32_t*)0x2025c000 = 2;
  *(uint32_t*)0x2025c004 = 0x70;
  *(uint8_t*)0x2025c008 = 0xe5;
  *(uint8_t*)0x2025c009 = 0;
  *(uint8_t*)0x2025c00a = 0;
  *(uint8_t*)0x2025c00b = 0;
  *(uint32_t*)0x2025c00c = 0;
  *(uint64_t*)0x2025c010 = 0;
  *(uint64_t*)0x2025c018 = 0;
  *(uint64_t*)0x2025c020 = 0;
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0xffff7fffffffffff, 5, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 29, 35);
  *(uint32_t*)0x2025c030 = 0;
  *(uint32_t*)0x2025c034 = 0;
  *(uint64_t*)0x2025c038 = 0x20000000;
  *(uint64_t*)0x2025c040 = 0;
  *(uint64_t*)0x2025c048 = 0;
  *(uint64_t*)0x2025c050 = 0;
  *(uint32_t*)0x2025c058 = 0;
  *(uint32_t*)0x2025c05c = 0;
  *(uint64_t*)0x2025c060 = 0;
  *(uint32_t*)0x2025c068 = 0;
  *(uint16_t*)0x2025c06c = 0;
  *(uint16_t*)0x2025c06e = 0;
  syscall(__NR_perf_event_open, 0x2025c000, 0, -1, -1, 0);
  res = syscall(__NR_socket, 0x1e, 1, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20b89fe4 = 0x772;
  *(uint32_t*)0x20b89fe8 = 0;
  *(uint32_t*)0x20b89fec = 0;
  *(uint32_t*)0x20b89ff0 = 0;
  write_file("/sys/kernel/debug/failslab/ignore-gfp-wait", "N");
  write_file("/sys/kernel/debug/fail_futex/ignore-private", "N");
  inject_fault(3);
  syscall(__NR_setsockopt, r[0], 0x10f, 0x87, 0x20b89fe4, 0x10);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}