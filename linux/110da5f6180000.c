// https://syzkaller.appspot.com/bug?id=2fee650ad5556cef0890b93d78bcc147553a8b67
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
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

static __thread int clone_ongoing;
static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  if (__atomic_load_n(&clone_ongoing, __ATOMIC_RELAXED) != 0) {
    exit(sig);
  }
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  int skip = __atomic_load_n(&skip_segv, __ATOMIC_RELAXED) != 0;
  int valid = addr < prog_start || addr > prog_end;
  if (skip && valid) {
    _longjmp(segv_env, 1);
  }
  exit(sig);
}

static void install_segv_handler(void)
{
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  syscall(SYS_rt_sigaction, 0x20, &sa, NULL, 8);
  syscall(SYS_rt_sigaction, 0x21, &sa, NULL, 8);
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_NODEFER | SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
}

#define NONFAILING(...)                                                        \
  ({                                                                           \
    int ok = 1;                                                                \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    } else                                                                     \
      ok = 0;                                                                  \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    ok;                                                                        \
  })

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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  NONFAILING(*(uint32_t*)0x200009c0 = 0xf);
  NONFAILING(*(uint32_t*)0x200009c4 = 4);
  NONFAILING(*(uint32_t*)0x200009c8 = 8);
  NONFAILING(*(uint32_t*)0x200009cc = 0xc);
  NONFAILING(*(uint32_t*)0x200009d0 = 0);
  NONFAILING(*(uint32_t*)0x200009d4 = -1);
  NONFAILING(*(uint32_t*)0x200009d8 = 0);
  NONFAILING(memset((void*)0x200009dc, 0, 16));
  NONFAILING(*(uint32_t*)0x200009ec = 0);
  NONFAILING(*(uint32_t*)0x200009f0 = -1);
  NONFAILING(*(uint32_t*)0x200009f4 = 0);
  NONFAILING(*(uint32_t*)0x200009f8 = 0);
  NONFAILING(*(uint32_t*)0x200009fc = 0);
  NONFAILING(*(uint64_t*)0x20000a00 = 0);
  res = syscall(__NR_bpf, /*cmd=*/0ul, /*arg=*/0x200009c0ul, /*size=*/0x48ul);
  if (res != -1)
    r[0] = res;
  NONFAILING(*(uint32_t*)0x200000c0 = 0x1b);
  NONFAILING(*(uint32_t*)0x200000c4 = 0);
  NONFAILING(*(uint32_t*)0x200000c8 = 0);
  NONFAILING(*(uint32_t*)0x200000cc = 0x8000);
  NONFAILING(*(uint32_t*)0x200000d0 = 0);
  NONFAILING(*(uint32_t*)0x200000d4 = -1);
  NONFAILING(*(uint32_t*)0x200000d8 = 0);
  NONFAILING(memset((void*)0x200000dc, 0, 16));
  NONFAILING(*(uint32_t*)0x200000ec = 0);
  NONFAILING(*(uint32_t*)0x200000f0 = -1);
  NONFAILING(*(uint32_t*)0x200000f4 = 0);
  NONFAILING(*(uint32_t*)0x200000f8 = 0);
  NONFAILING(*(uint32_t*)0x200000fc = 0);
  NONFAILING(*(uint64_t*)0x20000100 = 0);
  res = syscall(__NR_bpf, /*cmd=*/0ul, /*arg=*/0x200000c0ul, /*size=*/0x48ul);
  if (res != -1)
    r[1] = res;
  NONFAILING(*(uint32_t*)0x200000c0 = 0x11);
  NONFAILING(*(uint32_t*)0x200000c4 = 0x10);
  NONFAILING(*(uint64_t*)0x200000c8 = 0x20000280);
  NONFAILING(memcpy((void*)0x20000280,
                    "\x18\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                    "\x00\x00\x18\x11\x00\x00",
                    20));
  NONFAILING(*(uint32_t*)0x20000294 = r[1]);
  NONFAILING(memcpy((void*)0x20000298,
                    "\x00\x00\x00\x00\x00\x00\x00\x00\xb7\x02\x00\x00\x02\x00"
                    "\x00\x00\x85\x00\x00\x00\x86\x00\x00\x00\x18\x11\x00\x00",
                    28));
  NONFAILING(*(uint32_t*)0x200002b4 = r[0]);
  NONFAILING(memcpy(
      (void*)0x200002b8,
      "\x00\x00\x00\x00\x00\x00\x00\x00\xb7\x08\x00\x00\x00\x00\x00\x00\x7b\x8a"
      "\xf8\xff\x00\x00\x00\x00\xbf\xa2\x00\x00\x00\x00\x00\x00\x07\x02\x00\x00"
      "\xf8\xff\xff\xff\xb7\x03\x00\x00\x08\x00\x00\x00\xb7\x04\x00\x00\x00\x00"
      "\x00\x00\x85\x00\x00\x00\x03\x00\x00\x00\x95",
      65));
  NONFAILING(*(uint64_t*)0x200000d0 = 0x20000040);
  NONFAILING(memcpy((void*)0x20000040, "syzkaller\000", 10));
  NONFAILING(*(uint32_t*)0x200000d8 = 0);
  NONFAILING(*(uint32_t*)0x200000dc = 0);
  NONFAILING(*(uint64_t*)0x200000e0 = 0);
  NONFAILING(*(uint32_t*)0x200000e8 = 0);
  NONFAILING(*(uint32_t*)0x200000ec = 0);
  NONFAILING(memset((void*)0x200000f0, 0, 16));
  NONFAILING(*(uint32_t*)0x20000100 = 0);
  NONFAILING(*(uint32_t*)0x20000104 = 0);
  NONFAILING(*(uint32_t*)0x20000108 = -1);
  NONFAILING(*(uint32_t*)0x2000010c = 0);
  NONFAILING(*(uint64_t*)0x20000110 = 0);
  NONFAILING(*(uint32_t*)0x20000118 = 0);
  NONFAILING(*(uint32_t*)0x2000011c = 0);
  NONFAILING(*(uint64_t*)0x20000120 = 0);
  NONFAILING(*(uint32_t*)0x20000128 = 0);
  NONFAILING(*(uint32_t*)0x2000012c = 0);
  NONFAILING(*(uint32_t*)0x20000130 = 0);
  NONFAILING(*(uint32_t*)0x20000134 = 0);
  NONFAILING(*(uint64_t*)0x20000138 = 0);
  NONFAILING(*(uint64_t*)0x20000140 = 0);
  NONFAILING(*(uint32_t*)0x20000148 = 0);
  NONFAILING(*(uint32_t*)0x2000014c = 0);
  res = syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x200000c0ul, /*size=*/0x90ul);
  if (res != -1)
    r[2] = res;
  NONFAILING(*(uint64_t*)0x200001c0 = 0x20000180);
  NONFAILING(memcpy((void*)0x20000180, "kfree\000", 6));
  NONFAILING(*(uint32_t*)0x200001c8 = r[2]);
  syscall(__NR_bpf, /*cmd=*/0x11ul, /*arg=*/0x200001c0ul, /*size=*/0x10ul);
  syscall(__NR_openat, /*fd=*/0xffffff9c, /*file=*/0ul, /*flags=*/0x7a05ul,
          /*mode=*/0x1700ul);
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
  setup_sysctl();
  install_segv_handler();
  for (procid = 0; procid < 5; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
