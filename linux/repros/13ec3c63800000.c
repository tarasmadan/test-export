// https://syzkaller.appspot.com/bug?id=17486feafe3890260729f5fe75a25f8d865bdd5d
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <linux/futex.h>
#include <linux/net.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}
#include <stdint.h>
#include <string.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

static void fail(const char* msg, ...)
{
  int e = errno;
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit((e == ENOMEM || e == EAGAIN) ? kRetryStatus : kFailStatus);
}

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

#define XT_TABLE_SIZE 1536
#define XT_MAX_ENTRIES 10

struct xt_counters {
  uint64_t pcnt, bcnt;
};

struct ipt_getinfo {
  char name[32];
  unsigned int valid_hooks;
  unsigned int hook_entry[5];
  unsigned int underflow[5];
  unsigned int num_entries;
  unsigned int size;
};

struct ipt_get_entries {
  char name[32];
  unsigned int size;
  void* entrytable[XT_TABLE_SIZE / sizeof(void*)];
};

struct ipt_replace {
  char name[32];
  unsigned int valid_hooks;
  unsigned int num_entries;
  unsigned int size;
  unsigned int hook_entry[5];
  unsigned int underflow[5];
  unsigned int num_counters;
  struct xt_counters* counters;
  char entrytable[XT_TABLE_SIZE];
};

struct ipt_table_desc {
  const char* name;
  struct ipt_getinfo info;
  struct ipt_replace replace;
};

static struct ipt_table_desc ipv4_tables[] = {
    {.name = "filter"}, {.name = "nat"},      {.name = "mangle"},
    {.name = "raw"},    {.name = "security"},
};

static struct ipt_table_desc ipv6_tables[] = {
    {.name = "filter"}, {.name = "nat"},      {.name = "mangle"},
    {.name = "raw"},    {.name = "security"},
};

#define IPT_BASE_CTL 64
#define IPT_SO_SET_REPLACE (IPT_BASE_CTL)
#define IPT_SO_GET_INFO (IPT_BASE_CTL)
#define IPT_SO_GET_ENTRIES (IPT_BASE_CTL + 1)

struct arpt_getinfo {
  char name[32];
  unsigned int valid_hooks;
  unsigned int hook_entry[3];
  unsigned int underflow[3];
  unsigned int num_entries;
  unsigned int size;
};

struct arpt_get_entries {
  char name[32];
  unsigned int size;
  void* entrytable[XT_TABLE_SIZE / sizeof(void*)];
};

struct arpt_replace {
  char name[32];
  unsigned int valid_hooks;
  unsigned int num_entries;
  unsigned int size;
  unsigned int hook_entry[3];
  unsigned int underflow[3];
  unsigned int num_counters;
  struct xt_counters* counters;
  char entrytable[XT_TABLE_SIZE];
};

struct arpt_table_desc {
  const char* name;
  struct arpt_getinfo info;
  struct arpt_replace replace;
};

static struct arpt_table_desc arpt_tables[] = {
    {.name = "filter"},
};

#define ARPT_BASE_CTL 96
#define ARPT_SO_SET_REPLACE (ARPT_BASE_CTL)
#define ARPT_SO_GET_INFO (ARPT_BASE_CTL)
#define ARPT_SO_GET_ENTRIES (ARPT_BASE_CTL + 1)

static void checkpoint_iptables(struct ipt_table_desc* tables, int num_tables,
                                int family, int level)
{
  struct ipt_get_entries entries;
  socklen_t optlen;
  int fd, i;

  fd = socket(family, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(%d, SOCK_STREAM, IPPROTO_TCP)", family);
  for (i = 0; i < num_tables; i++) {
    struct ipt_table_desc* table = &tables[i];
    strcpy(table->info.name, table->name);
    strcpy(table->replace.name, table->name);
    optlen = sizeof(table->info);
    if (getsockopt(fd, level, IPT_SO_GET_INFO, &table->info, &optlen)) {
      switch (errno) {
      case EPERM:
      case ENOENT:
      case ENOPROTOOPT:
        continue;
      }
      fail("getsockopt(IPT_SO_GET_INFO)");
    }
    if (table->info.size > sizeof(table->replace.entrytable))
      fail("table size is too large: %u", table->info.size);
    if (table->info.num_entries > XT_MAX_ENTRIES)
      fail("too many counters: %u", table->info.num_entries);
    memset(&entries, 0, sizeof(entries));
    strcpy(entries.name, table->name);
    entries.size = table->info.size;
    optlen = sizeof(entries) - sizeof(entries.entrytable) + table->info.size;
    if (getsockopt(fd, level, IPT_SO_GET_ENTRIES, &entries, &optlen))
      fail("getsockopt(IPT_SO_GET_ENTRIES)");
    table->replace.valid_hooks = table->info.valid_hooks;
    table->replace.num_entries = table->info.num_entries;
    table->replace.size = table->info.size;
    memcpy(table->replace.hook_entry, table->info.hook_entry,
           sizeof(table->replace.hook_entry));
    memcpy(table->replace.underflow, table->info.underflow,
           sizeof(table->replace.underflow));
    memcpy(table->replace.entrytable, entries.entrytable, table->info.size);
  }
  close(fd);
}

static void reset_iptables(struct ipt_table_desc* tables, int num_tables,
                           int family, int level)
{
  struct xt_counters counters[XT_MAX_ENTRIES];
  struct ipt_get_entries entries;
  struct ipt_getinfo info;
  socklen_t optlen;
  int fd, i;

  fd = socket(family, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(%d, SOCK_STREAM, IPPROTO_TCP)", family);
  for (i = 0; i < num_tables; i++) {
    struct ipt_table_desc* table = &tables[i];
    if (table->info.valid_hooks == 0)
      continue;
    memset(&info, 0, sizeof(info));
    strcpy(info.name, table->name);
    optlen = sizeof(info);
    if (getsockopt(fd, level, IPT_SO_GET_INFO, &info, &optlen))
      fail("getsockopt(IPT_SO_GET_INFO)");
    if (memcmp(&table->info, &info, sizeof(table->info)) == 0) {
      memset(&entries, 0, sizeof(entries));
      strcpy(entries.name, table->name);
      entries.size = table->info.size;
      optlen = sizeof(entries) - sizeof(entries.entrytable) + entries.size;
      if (getsockopt(fd, level, IPT_SO_GET_ENTRIES, &entries, &optlen))
        fail("getsockopt(IPT_SO_GET_ENTRIES)");
      if (memcmp(table->replace.entrytable, entries.entrytable,
                 table->info.size) == 0)
        continue;
    }
    table->replace.num_counters = info.num_entries;
    table->replace.counters = counters;
    optlen = sizeof(table->replace) - sizeof(table->replace.entrytable) +
             table->replace.size;
    if (setsockopt(fd, level, IPT_SO_SET_REPLACE, &table->replace, optlen))
      fail("setsockopt(IPT_SO_SET_REPLACE)");
  }
  close(fd);
}

static void checkpoint_arptables(void)
{
  struct arpt_get_entries entries;
  socklen_t optlen;
  unsigned i;
  int fd;

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)");
  for (i = 0; i < sizeof(arpt_tables) / sizeof(arpt_tables[0]); i++) {
    struct arpt_table_desc* table = &arpt_tables[i];
    strcpy(table->info.name, table->name);
    strcpy(table->replace.name, table->name);
    optlen = sizeof(table->info);
    if (getsockopt(fd, SOL_IP, ARPT_SO_GET_INFO, &table->info, &optlen)) {
      switch (errno) {
      case EPERM:
      case ENOENT:
      case ENOPROTOOPT:
        continue;
      }
      fail("getsockopt(ARPT_SO_GET_INFO)");
    }
    if (table->info.size > sizeof(table->replace.entrytable))
      fail("table size is too large: %u", table->info.size);
    if (table->info.num_entries > XT_MAX_ENTRIES)
      fail("too many counters: %u", table->info.num_entries);
    memset(&entries, 0, sizeof(entries));
    strcpy(entries.name, table->name);
    entries.size = table->info.size;
    optlen = sizeof(entries) - sizeof(entries.entrytable) + table->info.size;
    if (getsockopt(fd, SOL_IP, ARPT_SO_GET_ENTRIES, &entries, &optlen))
      fail("getsockopt(ARPT_SO_GET_ENTRIES)");
    table->replace.valid_hooks = table->info.valid_hooks;
    table->replace.num_entries = table->info.num_entries;
    table->replace.size = table->info.size;
    memcpy(table->replace.hook_entry, table->info.hook_entry,
           sizeof(table->replace.hook_entry));
    memcpy(table->replace.underflow, table->info.underflow,
           sizeof(table->replace.underflow));
    memcpy(table->replace.entrytable, entries.entrytable, table->info.size);
  }
  close(fd);
}

static void reset_arptables()
{
  struct xt_counters counters[XT_MAX_ENTRIES];
  struct arpt_get_entries entries;
  struct arpt_getinfo info;
  socklen_t optlen;
  unsigned i;
  int fd;

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)");
  for (i = 0; i < sizeof(arpt_tables) / sizeof(arpt_tables[0]); i++) {
    struct arpt_table_desc* table = &arpt_tables[i];
    if (table->info.valid_hooks == 0)
      continue;
    memset(&info, 0, sizeof(info));
    strcpy(info.name, table->name);
    optlen = sizeof(info);
    if (getsockopt(fd, SOL_IP, ARPT_SO_GET_INFO, &info, &optlen))
      fail("getsockopt(ARPT_SO_GET_INFO)");
    if (memcmp(&table->info, &info, sizeof(table->info)) == 0) {
      memset(&entries, 0, sizeof(entries));
      strcpy(entries.name, table->name);
      entries.size = table->info.size;
      optlen = sizeof(entries) - sizeof(entries.entrytable) + entries.size;
      if (getsockopt(fd, SOL_IP, ARPT_SO_GET_ENTRIES, &entries, &optlen))
        fail("getsockopt(ARPT_SO_GET_ENTRIES)");
      if (memcmp(table->replace.entrytable, entries.entrytable,
                 table->info.size) == 0)
        continue;
    }
    table->replace.num_counters = info.num_entries;
    table->replace.counters = counters;
    optlen = sizeof(table->replace) - sizeof(table->replace.entrytable) +
             table->replace.size;
    if (setsockopt(fd, SOL_IP, ARPT_SO_SET_REPLACE, &table->replace, optlen))
      fail("setsockopt(ARPT_SO_SET_REPLACE)");
  }
  close(fd);
}

static void checkpoint_net_namespace(void)
{
  checkpoint_arptables();
  checkpoint_iptables(ipv4_tables, sizeof(ipv4_tables) / sizeof(ipv4_tables[0]),
                      AF_INET, SOL_IP);
  checkpoint_iptables(ipv6_tables, sizeof(ipv6_tables) / sizeof(ipv6_tables[0]),
                      AF_INET6, SOL_IPV6);
}

static void reset_net_namespace(void)
{
  reset_arptables();
  reset_iptables(ipv4_tables, sizeof(ipv4_tables) / sizeof(ipv4_tables[0]),
                 AF_INET, SOL_IP);
  reset_iptables(ipv6_tables, sizeof(ipv6_tables) / sizeof(ipv6_tables[0]),
                 AF_INET6, SOL_IPV6);
}

static void test();

void loop()
{
  int iter;
  checkpoint_net_namespace();
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      fail("loop fork failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
      test();
      doexit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      int res = waitpid(-1, &status, __WALL | WNOHANG);
      if (res == pid)
        break;
      usleep(1000);
      if (current_time_ms() - start > 5 * 1000) {
        kill(-pid, SIGKILL);
        kill(pid, SIGKILL);
        while (waitpid(-1, &status, __WALL) != pid) {
        }
        break;
      }
    }
    reset_net_namespace();
  }
}

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;
static int collide;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    while (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE))
      syscall(SYS_futex, &th->running, FUTEX_WAIT, 0, 0);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    __atomic_store_n(&th->running, 0, __ATOMIC_RELEASE);
    syscall(SYS_futex, &th->running, FUTEX_WAKE);
  }
  return 0;
}

static void execute(int num_calls)
{
  int call, thread;
  running = 0;
  for (call = 0; call < num_calls; call++) {
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]); thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 128 << 10);
        pthread_create(&th->th, &attr, thr, th);
      }
      if (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE)) {
        th->call = call;
        __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
        __atomic_store_n(&th->running, 1, __ATOMIC_RELEASE);
        syscall(SYS_futex, &th->running, FUTEX_WAKE);
        if (collide && call % 2)
          break;
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (running)
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

long r[3];
uint64_t procid;
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
    break;
  case 1:
    if (syscall(__NR_socketpair, 0xa, 0x80a, 0xf, 0x20000000) != -1)
      r[0] = *(uint32_t*)0x20000000;
    break;
  case 2:
    *(uint32_t*)0x20705000 = 0x10;
    syscall(__NR_accept, r[0], 0x20000000, 0x20705000);
    break;
  case 3:
    r[1] = syscall(__NR_socket, 0x10, 3, 6);
    break;
  case 4:
    *(uint64_t*)0x2014dfc8 = 0x206a2ff4;
    *(uint32_t*)0x2014dfd0 = 0xc;
    *(uint64_t*)0x2014dfd8 = 0x2054aff0;
    *(uint64_t*)0x2014dfe0 = 1;
    *(uint64_t*)0x2014dfe8 = 0;
    *(uint64_t*)0x2014dff0 = 0;
    *(uint32_t*)0x2014dff8 = 0;
    *(uint16_t*)0x206a2ff4 = 0x10;
    *(uint16_t*)0x206a2ff6 = 0;
    *(uint32_t*)0x206a2ff8 = 0;
    *(uint32_t*)0x206a2ffc = 0;
    *(uint64_t*)0x2054aff0 = 0x207c7000;
    *(uint64_t*)0x2054aff8 = 0xb8;
    *(uint32_t*)0x207c7000 = 0xb8;
    *(uint16_t*)0x207c7004 = 0x19;
    *(uint16_t*)0x207c7006 = 0x401;
    *(uint32_t*)0x207c7008 = 0;
    *(uint32_t*)0x207c700c = 0;
    *(uint32_t*)0x207c7010 = htobe32(0xe0000001);
    *(uint8_t*)0x207c7020 = 0xac;
    *(uint8_t*)0x207c7021 = 0x14;
    *(uint8_t*)0x207c7022 = 0;
    *(uint8_t*)0x207c7023 = 0;
    *(uint16_t*)0x207c7030 = 0;
    *(uint16_t*)0x207c7032 = htobe16(0);
    *(uint16_t*)0x207c7034 = 0;
    *(uint16_t*)0x207c7036 = htobe16(0);
    *(uint16_t*)0x207c7038 = 2;
    *(uint8_t*)0x207c703a = 0;
    *(uint8_t*)0x207c703b = 0;
    *(uint8_t*)0x207c703c = 0;
    *(uint32_t*)0x207c7040 = 0;
    *(uint32_t*)0x207c7044 = 0;
    *(uint64_t*)0x207c7048 = 0;
    *(uint64_t*)0x207c7050 = 0;
    *(uint64_t*)0x207c7058 = 0;
    *(uint64_t*)0x207c7060 = 0;
    *(uint64_t*)0x207c7068 = 0;
    *(uint64_t*)0x207c7070 = 0;
    *(uint64_t*)0x207c7078 = 0;
    *(uint64_t*)0x207c7080 = 0;
    *(uint64_t*)0x207c7088 = 0;
    *(uint64_t*)0x207c7090 = 0;
    *(uint64_t*)0x207c7098 = 0;
    *(uint64_t*)0x207c70a0 = 0;
    *(uint32_t*)0x207c70a8 = 0x7f;
    *(uint32_t*)0x207c70ac = 0;
    *(uint8_t*)0x207c70b0 = 0;
    *(uint8_t*)0x207c70b1 = 0;
    *(uint8_t*)0x207c70b2 = 0;
    *(uint8_t*)0x207c70b3 = 0;
    syscall(__NR_sendmsg, r[1], 0x2014dfc8, 0);
    break;
  case 5:
    *(uint64_t*)0x2014dfc8 = 0x206a2ff4;
    *(uint32_t*)0x2014dfd0 = 0xc;
    *(uint64_t*)0x2014dfd8 = 0x2054aff0;
    *(uint64_t*)0x2014dfe0 = 1;
    *(uint64_t*)0x2014dfe8 = 0;
    *(uint64_t*)0x2014dff0 = 0;
    *(uint32_t*)0x2014dff8 = 0;
    *(uint16_t*)0x206a2ff4 = 0x10;
    *(uint16_t*)0x206a2ff6 = 0;
    *(uint32_t*)0x206a2ff8 = 0;
    *(uint32_t*)0x206a2ffc = 0;
    *(uint64_t*)0x2054aff0 = 0x207c7000;
    *(uint64_t*)0x2054aff8 = 0xb8;
    *(uint32_t*)0x207c7000 = 0xb8;
    *(uint16_t*)0x207c7004 = 0x19;
    *(uint16_t*)0x207c7006 = 0x401;
    *(uint32_t*)0x207c7008 = 0;
    *(uint32_t*)0x207c700c = 0;
    *(uint32_t*)0x207c7010 = htobe32(0xe0000001);
    *(uint8_t*)0x207c7020 = 0xac;
    *(uint8_t*)0x207c7021 = 0x14;
    *(uint8_t*)0x207c7022 = 0;
    *(uint8_t*)0x207c7023 = 0;
    *(uint16_t*)0x207c7030 = 0;
    *(uint16_t*)0x207c7032 = htobe16(0);
    *(uint16_t*)0x207c7034 = 0;
    *(uint16_t*)0x207c7036 = htobe16(0);
    *(uint16_t*)0x207c7038 = 2;
    *(uint8_t*)0x207c703a = 0;
    *(uint8_t*)0x207c703b = 0;
    *(uint8_t*)0x207c703c = 0;
    *(uint32_t*)0x207c7040 = 0;
    *(uint32_t*)0x207c7044 = 0;
    *(uint64_t*)0x207c7048 = 0;
    *(uint64_t*)0x207c7050 = 0;
    *(uint64_t*)0x207c7058 = 0;
    *(uint64_t*)0x207c7060 = 0;
    *(uint64_t*)0x207c7068 = 0;
    *(uint64_t*)0x207c7070 = 0;
    *(uint64_t*)0x207c7078 = 0;
    *(uint64_t*)0x207c7080 = 0;
    *(uint64_t*)0x207c7088 = 0;
    *(uint64_t*)0x207c7090 = 0;
    *(uint64_t*)0x207c7098 = 0;
    *(uint64_t*)0x207c70a0 = 0;
    *(uint32_t*)0x207c70a8 = 0;
    *(uint32_t*)0x207c70ac = 0;
    *(uint8_t*)0x207c70b0 = 0;
    *(uint8_t*)0x207c70b1 = 0;
    *(uint8_t*)0x207c70b2 = 0;
    *(uint8_t*)0x207c70b3 = 0;
    syscall(__NR_sendmsg, -1, 0x2014dfc8, 0);
    break;
  case 6:
    r[2] = syscall(__NR_socket, 0x10, 3, 6);
    break;
  case 7:
    *(uint64_t*)0x2014dfc8 = 0x206a2ff4;
    *(uint32_t*)0x2014dfd0 = 0xc;
    *(uint64_t*)0x2014dfd8 = 0x2054aff0;
    *(uint64_t*)0x2014dfe0 = 1;
    *(uint64_t*)0x2014dfe8 = 0;
    *(uint64_t*)0x2014dff0 = 0;
    *(uint32_t*)0x2014dff8 = 0;
    *(uint16_t*)0x206a2ff4 = 0x10;
    *(uint16_t*)0x206a2ff6 = 0;
    *(uint32_t*)0x206a2ff8 = 0;
    *(uint32_t*)0x206a2ffc = 0;
    *(uint64_t*)0x2054aff0 = 0x207c7000;
    *(uint64_t*)0x2054aff8 = 0xc4;
    *(uint32_t*)0x207c7000 = 0xc4;
    *(uint16_t*)0x207c7004 = 0x19;
    *(uint16_t*)0x207c7006 = 0x401;
    *(uint32_t*)0x207c7008 = 0;
    *(uint32_t*)0x207c700c = 0;
    *(uint32_t*)0x207c7010 = htobe32(0xe0000001);
    *(uint8_t*)0x207c7020 = 0xac;
    *(uint8_t*)0x207c7021 = 0x14;
    *(uint8_t*)0x207c7022 = 0;
    *(uint8_t*)0x207c7023 = 0;
    *(uint16_t*)0x207c7030 = 0;
    *(uint16_t*)0x207c7032 = htobe16(0);
    *(uint16_t*)0x207c7034 = 0;
    *(uint16_t*)0x207c7036 = htobe16(0);
    *(uint16_t*)0x207c7038 = 2;
    *(uint8_t*)0x207c703a = 0;
    *(uint8_t*)0x207c703b = 0;
    *(uint8_t*)0x207c703c = 0;
    *(uint32_t*)0x207c7040 = 0;
    *(uint32_t*)0x207c7044 = 0;
    *(uint64_t*)0x207c7048 = 0;
    *(uint64_t*)0x207c7050 = 0;
    *(uint64_t*)0x207c7058 = 0;
    *(uint64_t*)0x207c7060 = 0;
    *(uint64_t*)0x207c7068 = 0;
    *(uint64_t*)0x207c7070 = 0;
    *(uint64_t*)0x207c7078 = 0;
    *(uint64_t*)0x207c7080 = 0;
    *(uint64_t*)0x207c7088 = 0;
    *(uint64_t*)0x207c7090 = 0;
    *(uint64_t*)0x207c7098 = 0;
    *(uint64_t*)0x207c70a0 = 0;
    *(uint32_t*)0x207c70a8 = 0;
    *(uint32_t*)0x207c70ac = 0;
    *(uint8_t*)0x207c70b0 = 0;
    *(uint8_t*)0x207c70b1 = 0;
    *(uint8_t*)0x207c70b2 = 0;
    *(uint8_t*)0x207c70b3 = 0;
    *(uint16_t*)0x207c70b8 = 0xc;
    *(uint16_t*)0x207c70ba = 0x15;
    *(uint32_t*)0x207c70bc = 0;
    *(uint32_t*)0x207c70c0 = 3;
    syscall(__NR_sendmsg, r[2], 0x2014dfc8, 0);
    break;
  }
}

void test()
{
  memset(r, -1, sizeof(r));
  execute(8);
  collide = 1;
  execute(8);
}

int main()
{
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        loop();
      }
    }
  }
  sleep(1000000);
  return 0;
}
