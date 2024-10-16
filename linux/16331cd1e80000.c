// https://syzkaller.appspot.com/bug?id=fe0ace9ad737bf592a5f023450535bf74d10e00d
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

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

static void setup_sysctl()
{
  char mypid[32];
  snprintf(mypid, sizeof(mypid), "%d", getpid());
  struct {
    const char* name;
    const char* data;
  } files[] = {
      {"/sys/kernel/debug/x86/nmi_longest_ns", "10000000000"},
      {"/proc/sys/kernel/hung_task_check_interval_secs", "20"},
      {"/proc/sys/net/core/bpf_jit_kallsyms", "1"},
      {"/proc/sys/net/core/bpf_jit_harden", "0"},
      {"/proc/sys/kernel/kptr_restrict", "0"},
      {"/proc/sys/kernel/softlockup_all_cpu_backtrace", "1"},
      {"/proc/sys/fs/mount-max", "100"},
      {"/proc/sys/vm/oom_dump_tasks", "0"},
      {"/proc/sys/debug/exception-trace", "0"},
      {"/proc/sys/kernel/printk", "7 4 1 3"},
      {"/proc/sys/kernel/keys/gc_delay", "1"},
      {"/proc/sys/vm/oom_kill_allocating_task", "1"},
      {"/proc/sys/kernel/ctrl-alt-del", "0"},
      {"/proc/sys/kernel/cad_pid", mypid},
  };
  for (size_t i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
    if (!write_file(files[i].name, files[i].data))
      printf("write to %s failed: %s\n", files[i].name, strerror(errno));
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul, /*prot=*/7ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  setup_sysctl();
  intptr_t res = 0;
  res = syscall(__NR_socketpair, /*domain=*/1ul, /*type=*/5ul, /*proto=*/0,
                /*fds=*/0x20000080ul);
  if (res != -1) {
    r[0] = *(uint32_t*)0x20000080;
    r[1] = *(uint32_t*)0x20000084;
  }
  *(uint16_t*)0x20000000 = 3;
  *(uint64_t*)0x20000008 = 0x20000040;
  *(uint16_t*)0x20000040 = 0x20;
  *(uint8_t*)0x20000042 = 0;
  *(uint8_t*)0x20000043 = 0;
  *(uint32_t*)0x20000044 = 0xfffff010;
  *(uint16_t*)0x20000048 = 0x20;
  *(uint8_t*)0x2000004a = 0;
  *(uint8_t*)0x2000004b = 0;
  *(uint32_t*)0x2000004c = 0xfffff034;
  *(uint16_t*)0x20000050 = 6;
  *(uint8_t*)0x20000052 = 0;
  *(uint8_t*)0x20000053 = 0;
  *(uint32_t*)0x20000054 = 0;
  syscall(__NR_setsockopt, /*fd=*/r[1], /*level=*/1, /*optname=*/0x1a,
          /*optval=*/0x20000000ul, /*optlen=*/0x10ul);
  *(uint64_t*)0x20000140 = 0x20000180;
  memcpy((void*)0x20000180, "\x03\x00\x95\x76", 4);
  *(uint64_t*)0x20000148 = 4;
  *(uint64_t*)0x20000150 = 0x20000380;
  memcpy(
      (void*)0x20000380,
      "\x99\x6c\xd5\xcb\x6e\xe0\x6b\x18\x33\x96\x0f\x43\x42\xa7\xd7\xd5\xeb\xd2"
      "\x7d\x57\x45\x16\xb8\x36\x9c\x06\xc6\xbc\xf0\xff\x25\xb3\x3b\x95\x62\xcd"
      "\xa6\x9f\xd4\x60\xa4\xc8\x6c\x03\x93\xdc\x13\xbd\x95\x40\xe9\xea\x14\x34"
      "\x8f\x1d\x14\xbc\xaa\xbe\x06\x1e\x70\x5a\x8f\xbf\xa4\x78\xcc\x97\x32\x2b"
      "\x1f\x31\xd5\x40\xc0\xf4\x9e\xbc\xd3\xb5\x0c\x97\xd4\x05\x93\x81\xb6\xe5"
      "\xe0\xcc\x30\x4b\xf0\x23\xbd\x94\xd8\xfc\x39\x90\xdf\xd6\x0a\x72\x8c\xbf"
      "\x5c\xf5\x75\x7f\xd6\x41\x25\xaf\xb2\x0d\x85\x61\xc0\x66\x9d\x9f\xe8\xf2"
      "\xd5\x6e\x86\x66\x0b\x49\xe7\xf1\x90\xc9\x08\x20\xbc\xd8\xa1\xe0\x3f\x83"
      "\x3e\xfa\xeb\x30\x56\xb8\xcf\xd5\x07\x1e\x79\x29\x57\xaa\x04\xe9\x93\xf6"
      "\xbe\x2a\x5a\x85\x37\xd3\xc6\x2f\x43\x7a\xda\x89\x2d\x2f\x4f\xa4\x2e\x13"
      "\xce\x50\x61\x4a\x60\x8f\x1d\x43\xef\x85\xec\x52\xeb\x4b\x23\x26\xbe\xed"
      "\x1a\x2e\xc0\x13\xf6\xa4\xb3\x7d\x5e\xeb\xad\x69\x3f\x22\x67\x3b\x35\xf4"
      "\x38\x08\x1c\xdf\x74\x4c\x29\x71\xf0\x10\x65\x9e\xa8\x92\x4b\xd9\x82\xcc"
      "\xf4\x9f\x58\x76\xda",
      239);
  *(uint64_t*)0x20000158 = 0xef;
  *(uint64_t*)0x20000160 = 0;
  *(uint64_t*)0x20000168 = 0;
  *(uint64_t*)0x20000170 = 0;
  *(uint64_t*)0x20000178 = 0;
  syscall(__NR_writev, /*fd=*/r[0], /*vec=*/0x20000140ul, /*vlen=*/4ul);
  return 0;
}
