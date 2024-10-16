// https://syzkaller.appspot.com/bug?id=6c009c466863e3293fa563d847e717dfd6438e95
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/uio.h>
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
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
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

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* uctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  doexit(sig);
}

static void install_segv_handler()
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
  {                                                                            \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    }                                                                          \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
  }

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void vsnprintf_check(char* str, size_t size, const char* format,
                            va_list args)
{
  int rv;

  rv = vsnprintf(str, size, format, args);
  if (rv < 0)
    fail("tun: snprintf failed");
  if ((size_t)rv >= size)
    fail("tun: string '%s...' doesn't fit into buffer", str);
}

static void snprintf_check(char* str, size_t size, const char* format, ...)
{
  va_list args;

  va_start(args, format);
  vsnprintf_check(str, size, format, args);
  va_end(args);
}

#define COMMAND_MAX_LEN 128
#define PATH_PREFIX                                                            \
  "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin "
#define PATH_PREFIX_LEN (sizeof(PATH_PREFIX) - 1)

static void execute_command(const char* format, ...)
{
  va_list args;
  char command[PATH_PREFIX_LEN + COMMAND_MAX_LEN];
  int rv;

  va_start(args, format);
  memcpy(command, PATH_PREFIX, PATH_PREFIX_LEN);
  vsnprintf_check(command + PATH_PREFIX_LEN, COMMAND_MAX_LEN, format, args);
  rv = system(command);
  if (rv != 0)
    fail("tun: command \"%s\" failed with code %d", &command[0], rv);

  va_end(args);
}

static int tunfd = -1;
static int tun_frags_enabled;

#define SYZ_TUN_MAX_PACKET_SIZE 1000

#define MAX_PIDS 32
#define ADDR_MAX_LEN 32

#define LOCAL_MAC "aa:aa:aa:aa:aa:%02hx"
#define REMOTE_MAC "bb:bb:bb:bb:bb:%02hx"

#define LOCAL_IPV4 "172.20.%d.170"
#define REMOTE_IPV4 "172.20.%d.187"

#define LOCAL_IPV6 "fe80::%02hxaa"
#define REMOTE_IPV6 "fe80::%02hxbb"

#define IFF_NAPI 0x0010
#define IFF_NAPI_FRAGS 0x0020

static void initialize_tun(uint64_t pid)
{
  if (pid >= MAX_PIDS)
    fail("tun: no more than %d executors", MAX_PIDS);
  int id = pid;

  tunfd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
  if (tunfd == -1) {
    printf("tun: can't open /dev/net/tun: please enable CONFIG_TUN=y\n");
    printf("otherwise fuzzing or reproducing might not work as intended\n");
    return;
  }

  char iface[IFNAMSIZ];
  snprintf_check(iface, sizeof(iface), "syz%d", id);

  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, iface, IFNAMSIZ);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI | IFF_NAPI | IFF_NAPI_FRAGS;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0) {
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0)
      fail("tun: ioctl(TUNSETIFF) failed");
  }
  if (ioctl(tunfd, TUNGETIFF, (void*)&ifr) < 0)
    fail("tun: ioctl(TUNGETIFF) failed");
  tun_frags_enabled = (ifr.ifr_flags & IFF_NAPI_FRAGS) != 0;

  char local_mac[ADDR_MAX_LEN];
  snprintf_check(local_mac, sizeof(local_mac), LOCAL_MAC, id);
  char remote_mac[ADDR_MAX_LEN];
  snprintf_check(remote_mac, sizeof(remote_mac), REMOTE_MAC, id);

  char local_ipv4[ADDR_MAX_LEN];
  snprintf_check(local_ipv4, sizeof(local_ipv4), LOCAL_IPV4, id);
  char remote_ipv4[ADDR_MAX_LEN];
  snprintf_check(remote_ipv4, sizeof(remote_ipv4), REMOTE_IPV4, id);

  char local_ipv6[ADDR_MAX_LEN];
  snprintf_check(local_ipv6, sizeof(local_ipv6), LOCAL_IPV6, id);
  char remote_ipv6[ADDR_MAX_LEN];
  snprintf_check(remote_ipv6, sizeof(remote_ipv6), REMOTE_IPV6, id);

  execute_command("sysctl -w net.ipv6.conf.%s.accept_dad=0", iface);

  execute_command("sysctl -w net.ipv6.conf.%s.router_solicitations=0", iface);

  execute_command("ip link set dev %s address %s", iface, local_mac);
  execute_command("ip addr add %s/24 dev %s", local_ipv4, iface);
  execute_command("ip -6 addr add %s/120 dev %s", local_ipv6, iface);
  execute_command("ip neigh add %s lladdr %s dev %s nud permanent", remote_ipv4,
                  remote_mac, iface);
  execute_command("ip -6 neigh add %s lladdr %s dev %s nud permanent",
                  remote_ipv6, remote_mac, iface);
  execute_command("ip link set dev %s up", iface);
}

static void setup_tun(uint64_t pid, bool enable_tun)
{
  if (enable_tun)
    initialize_tun(pid);
}

static int read_tun(char* data, int size)
{
  if (tunfd < 0)
    return -1;

  int rv = read(tunfd, data, size);
  if (rv < 0) {
    if (errno == EAGAIN)
      return -1;
    if (errno == EBADFD)
      return -1;
    fail("tun: read failed with %d", rv);
  }
  return rv;
}

static void flush_tun()
{
  char data[SYZ_TUN_MAX_PACKET_SIZE];
  while (read_tun(&data[0], sizeof(data)) != -1)
    ;
}

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    NONFAILING(strncpy(buf, (char*)a0, sizeof(buf)));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

static void test();

void loop()
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      fail("loop fork failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
      flush_tun();
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

#ifndef __NR_ioctl
#define __NR_ioctl 54
#endif
#ifndef __NR_getsockopt
#define __NR_getsockopt 365
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 370
#endif
#ifndef __NR_setsockopt
#define __NR_setsockopt 366
#endif
#ifndef __NR_writev
#define __NR_writev 146
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[2];
uint64_t procid;
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x20000000, 0x9000, 3, 0x32, -1, 0);
    break;
  case 1:
    NONFAILING(memcpy((void*)0x20006ff6, "/dev/dsp#", 10));
    r[0] = syz_open_dev(0x20006ff6, 1, 2);
    break;
  case 2:
    syscall(__NR_mmap, 0x20009000, 0x1000, 3, 0x32, -1, 0);
    break;
  case 3:
    syscall(__NR_mmap, 0x20009000, 0x1000, 3, 0x32, -1, 0);
    break;
  case 4:
    syscall(__NR_mmap, 0x20009000, 0x1000, 3, 0x32, -1, 0);
    break;
  case 5:
    syscall(__NR_mmap, 0x2000a000, 0x1000, 3, 0x32, -1, 0);
    break;
  case 6:
    NONFAILING(memcpy(
        (void*)0x2000afd8,
        "\x77\x7e\x87\x19\x20\x61\x5d\xa9\x97\x82\x31\x52\xd0\xa6\xd4\x0f",
        16));
    NONFAILING(*(uint16_t*)0x2000afe8 = 0x1b);
    NONFAILING(
        memcpy((void*)0x2000afea,
               "\x81\x25\xc1\x52\xc9\x79\x13\x65\x17\xa5\xb0\x0a\x16\xc4", 14));
    syscall(__NR_ioctl, r[0], 0x400454ca, 0x2000afd8);
    break;
  case 7:
    NONFAILING(*(uint32_t*)0x20009fe8 = 0);
    NONFAILING(*(uint16_t*)0x20009fec = -1);
    NONFAILING(*(uint16_t*)0x20009fee = 0x10);
    NONFAILING(*(uint64_t*)0x20009ff0 = 0xffffffffffffffec);
    NONFAILING(*(uint64_t*)0x20009ff8 = 0);
    NONFAILING(*(uint32_t*)0x20009ffc = 0x18);
    if (syscall(__NR_getsockopt, r[0], 0x84, 0x73, 0x20009fe8, 0x20009ffc) !=
        -1)
      NONFAILING(r[1] = *(uint32_t*)0x20009fe8);
    break;
  case 8:
    NONFAILING(*(uint32_t*)0x20003000 = 0x20005000);
    NONFAILING(*(uint32_t*)0x20003004 = 0x1c);
    NONFAILING(*(uint32_t*)0x20003008 = 0x20007fe0);
    NONFAILING(*(uint32_t*)0x2000300c = 2);
    NONFAILING(*(uint32_t*)0x20003010 = 0x20007fd0);
    NONFAILING(*(uint32_t*)0x20003014 = 0x2c);
    NONFAILING(*(uint32_t*)0x20003018 = 0x8000);
    NONFAILING(*(uint16_t*)0x20005000 = 0xa);
    NONFAILING(*(uint16_t*)0x20005002 = htobe16(0x4e20 + procid * 4));
    NONFAILING(*(uint32_t*)0x20005004 = 7);
    NONFAILING(*(uint8_t*)0x20005008 = -1);
    NONFAILING(*(uint8_t*)0x20005009 = 2);
    NONFAILING(*(uint8_t*)0x2000500a = 0);
    NONFAILING(*(uint8_t*)0x2000500b = 0);
    NONFAILING(*(uint8_t*)0x2000500c = 0);
    NONFAILING(*(uint8_t*)0x2000500d = 0);
    NONFAILING(*(uint8_t*)0x2000500e = 0);
    NONFAILING(*(uint8_t*)0x2000500f = 0);
    NONFAILING(*(uint8_t*)0x20005010 = 0);
    NONFAILING(*(uint8_t*)0x20005011 = 0);
    NONFAILING(*(uint8_t*)0x20005012 = 0);
    NONFAILING(*(uint8_t*)0x20005013 = 0);
    NONFAILING(*(uint8_t*)0x20005014 = 0);
    NONFAILING(*(uint8_t*)0x20005015 = 0);
    NONFAILING(*(uint8_t*)0x20005016 = 0);
    NONFAILING(*(uint8_t*)0x20005017 = 1);
    NONFAILING(*(uint32_t*)0x20005018 = 0x80000001);
    NONFAILING(*(uint32_t*)0x20007fe0 = 0x20003fae);
    NONFAILING(*(uint32_t*)0x20007fe4 = 0);
    NONFAILING(*(uint32_t*)0x20007fe8 = 0x20009000);
    NONFAILING(*(uint32_t*)0x20007fec = 0);
    NONFAILING(*(uint32_t*)0x20007fd0 = 0x2c);
    NONFAILING(*(uint32_t*)0x20007fd4 = 0x84);
    NONFAILING(*(uint32_t*)0x20007fd8 = 1);
    NONFAILING(*(uint16_t*)0x20007fdc = 6);
    NONFAILING(*(uint16_t*)0x20007fde = 3);
    NONFAILING(*(uint16_t*)0x20007fe0 = 3);
    NONFAILING(*(uint32_t*)0x20007fe4 = 1);
    NONFAILING(*(uint32_t*)0x20007fe8 = 5);
    NONFAILING(*(uint32_t*)0x20007fec = 0x81);
    NONFAILING(*(uint32_t*)0x20007ff0 = 0xf2c);
    NONFAILING(*(uint32_t*)0x20007ff4 = 1);
    NONFAILING(*(uint32_t*)0x20007ff8 = r[1]);
    syscall(__NR_sendmsg, r[0], 0x20003000, 0x20000000);
    break;
  case 9:
    syscall(__NR_mmap, 0x2000a000, 0x1000, 3, 0x32, -1, 0);
    break;
  case 10:
    NONFAILING(*(uint32_t*)0x2000affc = 0x3f);
    syscall(__NR_setsockopt, r[0], 0x84, 0x12, 0x2000affc, 4);
    break;
  case 11:
    NONFAILING(*(uint32_t*)0x20003000 = 0x20008f83);
    NONFAILING(*(uint32_t*)0x20003004 = 1);
    NONFAILING(memcpy((void*)0x20008f83, "\xbc", 1));
    syscall(__NR_writev, r[0], 0x20003000, 1);
    break;
  case 12:
    NONFAILING(*(uint64_t*)0x20005000 = 0x20000000);
    syscall(__NR_ioctl, r[0], 0xc0045002, 0x20005000);
    break;
  }
}

void test()
{
  memset(r, -1, sizeof(r));
  execute(13);
  collide = 1;
  execute(13);
}

int main()
{
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      install_segv_handler();
      for (;;) {
        setup_tun(procid, true);
        loop();
      }
    }
  }
  sleep(1000000);
  return 0;
}
