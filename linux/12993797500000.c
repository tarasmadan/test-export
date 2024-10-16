// https://syzkaller.appspot.com/bug?id=9249981aa39ae8e2098804e40acb6ed594ba957d
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <linux/capability.h>

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

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setsid();
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = (200 << 20);
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 32 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 256;
  setrlimit(RLIMIT_NOFILE, &rlim);
  if (unshare(CLONE_NEWNS)) {
  }
  if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(0x02000000)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
  typedef struct {
    const char* name;
    const char* value;
  } sysctl_t;
  static const sysctl_t sysctls[] = {
      {"/proc/sys/kernel/shmmax", "16777216"},
      {"/proc/sys/kernel/shmall", "536870912"},
      {"/proc/sys/kernel/shmmni", "1024"},
      {"/proc/sys/kernel/msgmax", "8192"},
      {"/proc/sys/kernel/msgmni", "1024"},
      {"/proc/sys/kernel/msgmnb", "1024"},
      {"/proc/sys/kernel/sem", "1024 1048576 500 1024"},
  };
  unsigned i;
  for (i = 0; i < sizeof(sysctls) / sizeof(sysctls[0]); i++)
    write_file(sysctls[i].name, sysctls[i].value);
}

static int wait_for_loop(int pid)
{
  if (pid < 0)
    exit(1);
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static void drop_caps(void)
{
  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    exit(1);
  const int drop = (1 << CAP_SYS_PTRACE) | (1 << CAP_SYS_NICE);
  cap_data[0].effective &= ~drop;
  cap_data[0].permitted &= ~drop;
  cap_data[0].inheritable &= ~drop;
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    exit(1);
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);
  setup_common();
  sandbox_common();
  drop_caps();
  if (unshare(CLONE_NEWNET)) {
  }
  loop();
  exit(1);
}

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0x0};

void loop(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_socket, 0x10ul, 0x803ul, 0);
  if (res != -1)
    r[2] = res;
  *(uint64_t*)0x200001c0 = 0;
  *(uint32_t*)0x200001c8 = 0;
  *(uint64_t*)0x200001d0 = 0x20000180;
  *(uint64_t*)0x20000180 = 0;
  *(uint64_t*)0x20000188 = 0;
  *(uint64_t*)0x200001d8 = 1;
  *(uint64_t*)0x200001e0 = 0;
  *(uint64_t*)0x200001e8 = 0;
  *(uint32_t*)0x200001f0 = 0;
  syscall(__NR_sendmsg, r[2], 0x200001c0ul, 0ul);
  *(uint32_t*)0x20000200 = 0x14;
  res = syscall(__NR_getsockname, r[2], 0x20000100ul, 0x20000200ul);
  if (res != -1)
    r[3] = *(uint32_t*)0x20000104;
  *(uint64_t*)0x20000140 = 0;
  *(uint32_t*)0x20000148 = 0;
  *(uint64_t*)0x20000150 = 0x20000780;
  *(uint64_t*)0x20000780 = 0x20000340;
  memcpy((void*)0x20000340, "\x44\x00\x00\x00\x24\x00\x0b\x0f\x00\x10\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         20);
  *(uint32_t*)0x20000354 = r[3];
  memcpy((void*)0x20000358, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00"
                            "\x09\x00\x01\x00\x67\x72\x65\x64\x00\x00\x00\x00"
                            "\x14\x00\x02\x00\x10\x00\x03\x00\x0a",
         33);
  *(uint64_t*)0x20000788 = 0x44;
  *(uint64_t*)0x20000158 = 1;
  *(uint64_t*)0x20000160 = 0;
  *(uint64_t*)0x20000168 = 0;
  *(uint32_t*)0x20000170 = 0;
  syscall(__NR_sendmsg, r[1], 0x20000140ul, 0ul);
  *(uint64_t*)0x20000080 = 0;
  *(uint32_t*)0x20000088 = 0;
  *(uint64_t*)0x20000090 = 0x20001c40;
  *(uint64_t*)0x20001c40 = 0x20000380;
  *(uint32_t*)0x20000380 = 0x170;
  *(uint16_t*)0x20000384 = 0x24;
  *(uint16_t*)0x20000386 = 1;
  *(uint32_t*)0x20000388 = 0;
  *(uint32_t*)0x2000038c = 0;
  *(uint8_t*)0x20000390 = 0;
  *(uint8_t*)0x20000391 = 0;
  *(uint16_t*)0x20000392 = 0;
  *(uint32_t*)0x20000394 = r[3];
  *(uint16_t*)0x20000398 = 0;
  *(uint16_t*)0x2000039a = 0;
  *(uint16_t*)0x2000039c = -1;
  *(uint16_t*)0x2000039e = -1;
  *(uint16_t*)0x200003a0 = 0;
  *(uint16_t*)0x200003a2 = 0;
  *(uint16_t*)0x200003a4 = 9;
  *(uint16_t*)0x200003a6 = 1;
  memcpy((void*)0x200003a8, "gred\000", 5);
  *(uint16_t*)0x200003b0 = 0x140;
  *(uint16_t*)0x200003b2 = 2;
  *(uint16_t*)0x200003b4 = 0x104;
  *(uint16_t*)0x200003b6 = 2;
  memcpy((void*)0x200003b8,
         "\x7f\x90\xfe\xd7\x05\x92\xb8\xb9\x20\x35\x25\x2c\x4f\xdb\xa7\x3d\xc0"
         "\xad\xa6\x68\x98\x7a\x04\x15\xbf\x10\x79\x73\xfa\x68\x1b\xc0\x08\x75"
         "\x1c\xbf\xed\xf6\x6a\x94\xd7\x62\x35\x59\x17\x3e\xf1\xc8\x62\x5d\x29"
         "\x74\x90\xa4\xe1\x48\x5d\x12\x08\x08\x10\x38\xf4\x1d\x55\xfb\x14\x7b"
         "\x09\x7d\x9e\x5c\xdd\x08\xfa\xf0\xde\x52\xbc\x4e\xa2\xd5\x54\xa7\xce"
         "\xbf\x37\xca\xa8\xc3\xd5\x27\x63\x64\xdb\xc8\x74\x1c\xf2\x66\x65\xd0"
         "\x61\x5f\xf8\xc4\xf0\x26\xd1\x65\x76\xdf\x5e\xd2\x9b\x27\xd1\xa2\x5a"
         "\x9c\x7d\x6c\x7d\xb4\xac\xcf\xc5\x6b\x3e\xcd\x9f\xbb\xaf\x1a\x4f\xf0"
         "\x81\xd3\xb9\x59\x2e\x94\x98\x47\xbe\xde\x1f\x4a\xc1\x54\xe8\x8d\x1a"
         "\x56\x73\xb4\x23\xc1\xa7\xd8\xb1\x0e\xb3\xc1\x4e\xee\xa7\x6b\x19\x66"
         "\x99\x7b\x28\x9d\xa7\xa6\xa2\x1a\xf4\xfe\xa3\x32\x45\x82\xf5\x6e\x6a"
         "\x1b\xbd\x8f\x00\xfc\x59\x7a\xb3\x3b\x68\xe0\x29\x13\x8d\xeb\x5f\xc6"
         "\xd2\x02\xdc\xf9\x89\xe5\xd0\x99\x17\x17\xe5\xe8\xd6\xe5\x05\x07\x98"
         "\xa0\x62\xea\xc4\x2f\x28\xd6\xdf\x2c\xc9\x8e\x53\x9c\x02\x37\x81\x9c"
         "\xe2\x02\xa4\xc2\xb2\x1c\x39\xa4\x42\xc3\x2a\xac\x65\x00\x2f\x29\x6d"
         "\x0c",
         256);
  *(uint16_t*)0x200004b8 = 0x38;
  *(uint16_t*)0x200004ba = 1;
  *(uint32_t*)0x200004bc = 0;
  *(uint32_t*)0x200004c0 = 0;
  *(uint32_t*)0x200004c4 = 0;
  *(uint32_t*)0x200004c8 = 9;
  *(uint32_t*)0x200004cc = 0;
  *(uint32_t*)0x200004d0 = 0;
  *(uint32_t*)0x200004d4 = 0;
  *(uint32_t*)0x200004d8 = 0;
  *(uint32_t*)0x200004dc = 0;
  *(uint32_t*)0x200004e0 = 0;
  *(uint8_t*)0x200004e4 = 0;
  *(uint8_t*)0x200004e5 = 0;
  *(uint8_t*)0x200004e6 = -1;
  *(uint8_t*)0x200004e7 = 0;
  *(uint32_t*)0x200004e8 = 0;
  *(uint32_t*)0x200004ec = 0xfffffffd;
  *(uint64_t*)0x20001c48 = 0x170;
  *(uint64_t*)0x20000098 = 1;
  *(uint64_t*)0x200000a0 = 0;
  *(uint64_t*)0x200000a8 = 0;
  *(uint32_t*)0x200000b0 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000080ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  do_sandbox_none();
  return 0;
}
