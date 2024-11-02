// https://syzkaller.appspot.com/bug?id=6e33f9966ade60fc2932bb2059228cbb275c2fdc
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
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

static void snprintf_check(char* str, size_t size, const char* format,
                           ...)
{
  va_list args;

  va_start(args, format);
  vsnprintf_check(str, size, format, args);
  va_end(args);
}

#define COMMAND_MAX_LEN 128
#define PATH_PREFIX                                                    \
  "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin "
#define PATH_PREFIX_LEN (sizeof(PATH_PREFIX) - 1)

static void execute_command(const char* format, ...)
{
  va_list args;
  char command[PATH_PREFIX_LEN + COMMAND_MAX_LEN];
  int rv;

  va_start(args, format);
  memcpy(command, PATH_PREFIX, PATH_PREFIX_LEN);
  vsnprintf_check(command + PATH_PREFIX_LEN, COMMAND_MAX_LEN, format,
                  args);
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
    printf(
        "tun: can't open /dev/net/tun: please enable CONFIG_TUN=y\n");
    printf("otherwise fuzzing or reproducing might not work as "
           "intended\n");
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

  execute_command("sysctl -w net.ipv6.conf.%s.router_solicitations=0",
                  iface);

  execute_command("ip link set dev %s address %s", iface, local_mac);
  execute_command("ip addr add %s/24 dev %s", local_ipv4, iface);
  execute_command("ip -6 addr add %s/120 dev %s", local_ipv6, iface);
  execute_command("ip neigh add %s lladdr %s dev %s nud permanent",
                  remote_ipv4, remote_mac, iface);
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

static void test();

void loop()
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      fail("clone failed");
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

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

long r[52];
void test()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xf7f000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] = syscall(__NR_socket, 0x26ul, 0x5ul, 0x0ul);
  *(uint16_t*)0x200f8000 = (uint16_t)0x26;
  memcpy((void*)0x200f8002,
         "\x61\x65\x61\x64\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         14);
  *(uint32_t*)0x200f8010 = (uint32_t)0x0;
  *(uint32_t*)0x200f8014 = (uint32_t)0x0;
  memcpy((void*)0x200f8018,
         "\x70\x63\x72\x79\x70\x74\x28\x67\x63\x6d\x5f\x62\x61\x73\x65"
         "\x28\x63\x74\x72\x28\x61\x65\x73\x2d\x61\x65\x73\x6e\x69\x29"
         "\x2c\x67\x68\x61\x73\x68\x2d\x67\x65\x6e\x65\x72\x69\x63\x29"
         "\x29\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00",
         64);
  r[7] = syscall(__NR_bind, r[1], 0x200f8000ul, 0x58ul);
  memcpy((void*)0x20f73000, "\x0a\x07\x75\xb0\xcc\xe3\x83\xe5\xb3\xb6"
                            "\x0c\xed\x5c\x74\xdb\xb6",
         16);
  r[9] = syscall(__NR_setsockopt, r[1], 0x117ul, 0x1ul, 0x20f73000ul,
                 0x10ul);
  r[10] = syscall(__NR_accept, r[1], 0x0ul, 0x0ul);
  memcpy((void*)0x204ee000, "\x70\x72\x6f\x63\x26\x00", 6);
  r[12] = syscall(__NR_memfd_create, 0x204ee000ul, 0x1ul);
  *(uint64_t*)0x20f74fc8 = (uint64_t)0x0;
  *(uint32_t*)0x20f74fd0 = (uint32_t)0x0;
  *(uint64_t*)0x20f74fd8 = (uint64_t)0x20bbdfc0;
  *(uint64_t*)0x20f74fe0 = (uint64_t)0x4;
  *(uint64_t*)0x20f74fe8 = (uint64_t)0x20f75000;
  *(uint64_t*)0x20f74ff0 = (uint64_t)0x18;
  *(uint32_t*)0x20f74ff8 = (uint32_t)0x0;
  *(uint64_t*)0x20bbdfc0 = (uint64_t)0x20f7c000;
  *(uint64_t*)0x20bbdfc8 = (uint64_t)0x0;
  *(uint64_t*)0x20bbdfd0 = (uint64_t)0x20dc4f5d;
  *(uint64_t*)0x20bbdfd8 = (uint64_t)0x0;
  *(uint64_t*)0x20bbdfe0 = (uint64_t)0x20130f10;
  *(uint64_t*)0x20bbdfe8 = (uint64_t)0x0;
  *(uint64_t*)0x20bbdff0 = (uint64_t)0x20f7d000;
  *(uint64_t*)0x20bbdff8 = (uint64_t)0x1000;
  memcpy(
      (void*)0x20f7d000,
      "\x4d\xda\xfe\xf9\xb0\x3d\xe1\x7f\x14\x1e\xf3\x3c\xe2\x7c\x44\x5d"
      "\xaf\xaa\x27\x46\xe4\x2b\x90\xb8\xbd\xc5\x8b\xba\x34\x0e\x4c\x28"
      "\xe0\xd6\x5c\x0c\xf3\x05\x96\xc8\x6f\x1d\x93\x11\xb2\x2e\x65\xf6"
      "\xfb\xeb\x44\x90\xac\x51\xe8\x63\x8f\x32\x3c\xec\xcb\x17\x16\xc5"
      "\xc7\x62\x9b\x34\x9e\x47\xc6\xe4\xdb\xa8\xc6\xc2\x5d\x4d\x57\x12"
      "\xd9\x8f\x58\xa5\x19\xfd\x24\x40\x88\x08\x77\xbe\xf0\xe6\x91\x4b"
      "\x32\x85\x1c\x69\xec\x4a\xfe\x59\x8b\x64\x1f\x17\xd7\x90\xdc\x26"
      "\xf1\xe2\x51\xff\x53\xde\xf0\x07\x81\xfc\xb7\x4d\xcd\xcb\xa7\xaa"
      "\x9f\x07\xe7\xb9\xb7\xbd\x9a\x4a\x10\x28\xd1\x9e\x57\xbe\x5a\xcf"
      "\x9d\x51\x1b\x3c\x34\xee\xb6\x65\x0c\x41\x2a\x78\xdb\xc5\x6d\x31"
      "\x25\xff\xfb\xe2\xf9\xd4\x4e\xc1\x90\x1d\x9d\xf6\xb5\x5f\xb5\x51"
      "\x31\xcf\xa5\x66\x08\xbb\x8c\x33\x44\x87\x25\x3b\x1e\x68\x39\xc3"
      "\xb1\x04\x71\x64\x85\x5b\x2d\xa6\xe8\xbb\x88\xbd\xd2\xc8\x9f\x1b"
      "\x01\xbd\x77\x1a\xb3\xbf\x36\x78\x67\x69\x8e\xad\xfc\x13\x89\x3a"
      "\xa1\x52\x00\xaf\xeb\xf6\xf3\x02\x29\x51\xc6\x18\x77\x16\xce\x4c"
      "\x31\xc0\x5b\xf1\x64\x8d\x84\xa7\x85\x6b\xd2\x40\xee\xd2\x04\x10"
      "\x98\x82\x13\x5b\x06\xf0\xd6\x81\xc8\xa4\x82\x2c\x50\xb4\xb8\x65"
      "\x75\x03\x02\xe7\x8e\x27\x66\x79\x45\x6a\xa6\x92\xfc\xe9\xbd\x2b"
      "\x65\xbe\xb0\x43\x17\xaf\x95\x91\xf3\x68\x10\xa2\x22\xb0\x38\xf6"
      "\x74\x2f\xab\x2b\x5b\x17\x3f\x37\xa6\x18\x7d\xf2\x5c\x2a\x6b\x86"
      "\xba\xce\x5a\x60\xd0\xc1\x9f\x9b\xc4\x73\xf8\x06\x77\x86\x85\x42"
      "\x00\x47\xee\xf0\x42\xca\x1c\x9a\xc6\x8b\x2d\x0a\x90\x2b\xe6\x2f"
      "\x1f\x36\xb5\x89\x96\xda\xd3\x4a\x30\xd9\xa0\xba\x77\x77\x9e\xca"
      "\x7c\x9e\x5a\x85\x78\xa0\x68\xfd\x9c\x43\xd7\x27\xb8\x7b\xfb\x3d"
      "\x2c\x65\xb3\x57\xf8\xdf\xab\x5e\xef\xf8\x3e\x44\x3a\x38\x3f\xfe"
      "\x2e\xce\xbb\x52\x27\xe2\xf4\x29\x3a\xc6\xd4\x01\x25\x13\x51\xb9"
      "\x13\x97\x98\x3e\x52\xff\x3a\xfb\x94\xaa\x12\xb3\x99\x67\x2c\x7f"
      "\x92\xcd\xba\x58\xc7\x67\x4e\xd4\x25\x83\x79\xec\xe6\x4f\xa6\x65"
      "\xa6\xad\xd1\x0b\x38\x37\x79\x1e\x64\x84\xd1\xa2\xc4\xf8\x33\xb3"
      "\xe8\xd2\x42\x5d\x01\x5c\x21\xf8\x42\x84\x0b\xca\x7f\x9c\xf7\xf7"
      "\x66\x65\xc1\xb9\x09\x8f\x42\xf1\xcc\x43\xdc\x7e\x20\x36\xf8\xf1"
      "\xfd\x58\x88\xae\xe7\xe2\xaa\xb9\x9b\x3b\x06\x53\x13\x83\x07\xde"
      "\x61\xe1\x92\x49\x70\xc4\xc4\xf3\xd1\x4d\xdc\x12\xf0\x0e\x26\x89"
      "\x63\x2e\xd8\xc4\x87\x65\xcf\x81\xae\x82\x87\x51\xba\xfb\x18\x36"
      "\x7e\x10\x4e\x57\x42\x0c\xd4\x47\xc2\xd1\xa9\xe3\x75\x36\xce\xab"
      "\x1f\xa0\x64\x58\x95\x64\x71\x56\x04\x4c\xa9\xfe\xc7\x11\x90\xf1"
      "\x0b\x4f\xa8\x79\x2e\x65\x4f\x55\xb4\x46\x8a\x9b\x35\x2a\xc4\x9b"
      "\x0c\x3d\xad\x1a\xa5\xb5\x43\x4c\xcc\xab\x60\x0f\xe7\xce\x42\xd2"
      "\x1c\xff\x40\x9e\x2e\x24\x4a\x31\xb9\xb9\xc8\x10\x87\xd5\x88\x96"
      "\xd9\x46\x7c\x99\xb5\x27\x93\x91\x0f\xbb\x93\xd4\xc9\xc4\x9f\x7e"
      "\xa0\xd8\xab\x4e\x8b\x0f\x69\xd7\x1a\x30\x55\x8a\x55\x1e\x72\xf6"
      "\x29\x71\xf6\x60\xa9\xe0\x43\x3b\x91\x2c\xe4\x0a\x5b\x51\xfe\x06"
      "\x61\xc9\x02\x59\x99\xed\xc7\xcf\xd2\x01\x88\x31\x4e\x7d\x26\x2a"
      "\x48\x6c\x3c\xb2\x3c\x77\x3f\xc3\x03\x0c\x69\x02\x99\x2b\xb2\x5f"
      "\x05\xaf\x36\xa6\xa3\xc6\xc0\xda\x38\x45\x9b\x50\x7d\xc2\x76\x97"
      "\x47\xf7\xac\x24\xea\x29\x3e\xfb\x1a\x1b\xd2\x9e\x6d\x6b\xa3\xd3"
      "\xba\xcf\x8b\xfa\x1b\x9c\xa6\x31\x21\x14\x23\x63\x89\xda\x6b\x95"
      "\x48\x82\xa1\xf6\xa7\xc0\xf4\x4f\x32\xf6\xbc\xb7\x44\x8e\x68\x1e"
      "\xf0\xf9\x3a\x82\x83\x36\xa3\x3b\xd6\x38\x31\xbb\xb2\xc8\xd4\x38"
      "\xae\x06\x48\x9a\x05\xfd\xf7\x09\x7b\xf6\x88\xfa\x7b\x29\xd2\xd8"
      "\x8d\xd4\x92\x5e\x1f\xb4\x0c\xb7\x3d\x37\x40\x5a\x19\x59\x16\x1a"
      "\xf4\xf5\x2b\x01\x16\x79\x85\x3b\xb1\x50\x7f\xa6\xf7\xff\x70\x70"
      "\xfe\x1f\x50\x4f\x4e\x88\x66\x9b\xa8\xcb\x64\x20\x14\x95\x5e\xc2"
      "\xb1\x66\x41\x0c\x06\xb3\x0d\xd4\xc1\x87\x86\x79\x6e\x3a\x3a\x5d"
      "\x1d\x1f\x99\xb3\x0b\x7b\x36\x0b\xf0\x4e\xd8\xb0\xc4\xa0\x34\xb8"
      "\xd6\x1f\xe6\xce\xc6\x94\x9d\x90\x9b\x5f\x4c\xc4\xf9\x44\x00\xed"
      "\x55\x8a\x8a\x93\x48\x4b\xb9\x86\xf9\x1f\x5e\xb6\x85\x01\xba\x13"
      "\xae\x4c\x04\x07\x66\xcb\x46\x0f\xf0\x97\x22\xb2\xb4\xc0\xa4\x2d"
      "\xc4\x84\xd1\xc7\x21\x49\x7b\x08\xaf\x89\x33\x5c\x91\xe1\x93\xfe"
      "\x4e\x4f\xba\x45\x24\xb5\x88\x8d\x5b\x00\x0b\x19\x5d\x35\xac\x45"
      "\x61\xfd\xf5\xd3\x8c\x0f\xc1\x70\x9c\xa3\xc6\xe1\x79\x38\xf5\x2f"
      "\xfe\x5f\x34\xaf\x47\x0b\xc3\x5c\x1d\x10\x6c\x53\xcc\x78\x00\xa3"
      "\xfc\xfb\x4f\xa4\x04\x96\x87\x2e\xf3\x58\x60\x0c\xd7\xad\x6a\xd1"
      "\xa1\x76\x21\xb3\xdd\x0f\xc5\xe0\xf7\xf6\x15\x48\x76\xda\x93\x7f"
      "\x2c\xe1\xc1\xf4\xf5\xe2\xbb\x89\xf9\x44\xa9\x56\x1e\x71\x0d\xfb"
      "\xeb\xaa\x51\x02\xe9\xd2\x0a\xc9\xbf\x50\x25\x18\x0a\x58\x2e\x7b"
      "\xe5\x7f\x12\x81\xd1\x7e\x1b\x5c\x8e\x4c\x98\x4c\xf3\xaf\xdc\xba"
      "\xaa\xd0\x32\x94\xba\x40\xad\x4f\x3c\x49\x01\xbf\xf6\x10\x50\x7d"
      "\x7f\xb6\xae\x34\x90\x59\x6a\x51\x7f\x8d\xcf\xa0\xdc\xf0\xf0\x6d"
      "\xc8\x87\xcd\x37\x32\xf9\x41\x01\xd7\xae\x1e\x77\xa4\x1b\x74\xbf"
      "\xa4\x43\xb0\xb2\xd1\xcb\xa7\xde\x22\x9d\x99\x76\x88\x95\xc3\x9d"
      "\x8b\xf6\xe4\x20\x19\xda\xfd\xe4\x8d\x78\x9f\x92\xab\xdd\xd7\xb2"
      "\xc8\x24\x76\x28\xdb\x9a\xdc\x5f\x61\x45\x43\xc2\xfa\xc8\xef\x1f"
      "\x8d\xb9\xe3\x1f\xf8\xba\xa1\xbd\xd2\xa2\x1e\x25\xc8\x16\xf2\xc6"
      "\x63\x56\xc5\x33\x6c\xb5\x20\xfb\x56\x7b\x33\x58\xdc\x79\x5c\xb1"
      "\x6b\x72\x9f\x76\x7c\x64\x95\x47\xb3\x0f\x99\x1b\x2a\xd4\xb3\xb0"
      "\xfd\x8e\xee\xe8\x69\x6e\xe6\x41\x46\x23\x10\x92\xee\xe5\xcb\x86"
      "\x8c\xdd\x96\xaa\xfa\x20\xa1\xd0\x2b\x30\x78\x90\xa5\x19\xb7\x96"
      "\x08\xa9\xad\xbf\xbe\xb0\xe3\xdd\x8c\x73\x9a\xc1\x71\x3f\x6f\x2a"
      "\x14\xf3\x5b\xf9\x85\x47\xd1\x18\x5b\xd7\x63\xbb\x5e\xd5\xb3\xcb"
      "\x9a\xa4\xcf\xbb\xa0\xa4\xdf\x98\xe2\x80\x28\x63\xc6\x9d\x05\xdc"
      "\xec\x69\x93\xfd\x6b\x35\xa5\xbe\x95\x63\x47\x43\x81\xd1\xd1\x12"
      "\xaa\x22\x51\xf4\xc8\xaa\xac\x43\x0e\x5a\xa0\xa2\xdb\x81\xf0\x76"
      "\x72\x92\xa4\xfc\xaa\x0f\x79\x35\x77\x60\xeb\x06\xb7\x96\x82\x97"
      "\x89\x36\xca\x4e\x30\x61\x72\xce\x63\x5f\xac\x8b\x9a\x62\xa6\x99"
      "\x00\x21\xad\x55\xf5\x84\xa8\x9d\x48\x3c\x44\x14\x2e\x44\x0e\xe1"
      "\x67\x03\x09\x51\x6e\xd5\x35\x18\x3a\x09\x6a\xd1\x40\x79\x54\x22"
      "\xc6\xec\xdd\xb5\xaf\xc9\xd4\x22\xd5\xaa\x97\x15\xa0\x5a\x6d\x50"
      "\x72\x1e\x55\x04\xd4\x26\x92\x3b\x1b\xcd\xd2\xb8\xec\xc2\xf7\xcc"
      "\xa7\xf3\x25\x8d\xa2\x60\xd6\x8d\xb0\xe0\xeb\x3b\xc1\x6b\x38\xee"
      "\x3e\xdd\x09\x32\xa8\x5e\x56\x4a\x88\x37\xc3\x32\xba\x26\xd8\xa4"
      "\x2d\xa5\x63\xcc\xe6\x79\x8a\xe1\x82\xdc\xf5\xa2\x92\x16\xd7\x7a"
      "\xd1\x04\x26\xaa\xf2\xb3\x70\x86\x96\x8c\xc0\x5d\xbb\x5e\x8c\x89"
      "\xcf\xe3\x5d\xb4\x7c\x06\xff\x82\xa0\x93\xe8\x09\xd7\xef\xd2\x20"
      "\x10\x16\x71\xdf\xf5\xb3\xa7\x9a\xf0\x89\xe6\xfd\x91\x60\x2d\x65"
      "\xe2\x0d\xb0\x13\x8a\x05\xc0\xd3\x75\xaf\x25\x4e\x82\x16\x6c\x34"
      "\xae\xe6\xba\x7d\x42\x00\x50\x81\xa2\x8e\x34\xf4\x9c\xf1\xd3\x02"
      "\x1b\x54\xe9\xf4\x71\x28\x31\x3a\x98\xee\x99\x85\x98\x72\xe3\x66"
      "\xda\xe2\x3c\x20\x0d\x2c\xa5\x41\x62\x5c\x44\xc1\x7b\x76\x00\xcf"
      "\x72\xde\xd2\x98\xff\x8c\x07\xc9\x53\x7b\xae\x6c\x1b\xdb\x24\xec"
      "\x72\xc2\x37\xa8\x93\x32\x3d\x6d\x8e\xd8\x6d\xc2\xb3\x21\x67\x24"
      "\x0d\xab\xe9\xa7\x5d\xf6\x41\x69\xc5\x79\xa8\x23\x65\x2e\xe2\xd2"
      "\x7a\xc1\x15\xa3\x37\xb0\x0d\xa2\xe2\xf2\x92\x62\x86\x3c\x3f\xfa"
      "\x8c\xb8\x06\xf3\x23\x78\x9f\xf4\x68\x5f\x21\x75\x2a\x6f\xf4\x3a"
      "\xa2\xd9\x6e\xae\x34\xcb\x25\x63\x6f\x6e\xf7\x8a\xc4\x7c\x14\x18"
      "\x20\xa5\xb2\x10\x44\x82\x76\xcf\x84\x1d\x8d\xc9\xc9\x3c\x3f\x77"
      "\x82\x55\x28\xb6\xe6\x0d\xb2\x55\x1a\x80\x80\xc7\x2c\xb1\xd8\x00"
      "\x75\x0c\x15\x3b\x2b\xea\x60\x2d\xf9\xa7\xaf\x67\x4e\x2e\xe5\x7c"
      "\xe7\xde\x57\x34\x97\x8d\x58\xac\x7e\x1c\x53\x7d\x37\x55\xb4\x62"
      "\x71\xb6\x24\x40\x85\xf0\x97\xfd\xfe\x36\x1e\x6b\x3a\x92\x11\x54"
      "\xad\x46\x0c\x38\x4f\x58\x66\x60\xc8\xe4\x55\xe2\xd1\xc7\x0e\x60"
      "\xda\xf3\x36\x69\x9a\x97\x71\x7a\x3d\x0e\xaf\xae\x14\x84\x27\x90"
      "\xbc\xa8\x40\x1d\xc9\xaa\x01\x16\x4f\xa8\x08\x01\xc8\xde\xc8\xd7"
      "\xa6\x4e\x5d\x37\xe2\xd8\x48\x04\x91\x3b\xaa\x5e\xc8\x50\xa2\xd0"
      "\x90\x09\x3d\x53\x63\x58\x69\x78\x5d\xa9\x81\x88\x6a\x64\xf8\x6f"
      "\x43\x70\x8d\x37\x41\xae\xfd\xd1\x72\xdb\xec\xe9\xda\x33\x0c\x09"
      "\x78\x1f\x9e\x90\x48\xe2\x69\x94\x3c\xed\x3c\x1f\xab\x8b\x06\x38"
      "\xe0\xce\xbb\xbc\xf0\x64\x4b\x98\x45\xf1\x93\xcb\x97\x67\xed\x02"
      "\xc8\xdc\x66\x8a\x4c\x3c\x8e\x24\xe9\xed\xdc\x6f\xc1\x6a\x0a\xce"
      "\x3c\x0e\xbe\x81\xd8\xdd\x6f\xa3\x05\x29\x35\x14\x34\x57\x4f\xea"
      "\x1d\xdd\xad\xce\x9d\xcf\xbb\xad\x4f\x09\x02\xfc\xf6\x1a\xe6\x67"
      "\xcc\x47\xd5\x18\xbe\xae\x00\x9d\xc3\x5b\x74\xaf\x21\x46\x14\x9e"
      "\x4f\xeb\xf3\xe2\x71\xb7\x50\xda\x2f\x31\xa8\x27\x2c\x6f\xd5\x40"
      "\x99\x30\xab\x1d\x3b\x8e\xaa\xfa\x92\x5e\x95\xaf\x0c\x3a\x93\x40"
      "\x04\x7e\x5e\x83\x10\x61\x03\x9a\x36\x63\x79\xf0\xe0\xa2\x51\xac"
      "\x69\xc5\x94\x07\xfd\x6c\xb0\x2e\x57\x16\x31\xf3\xd7\xa5\xa6\x27"
      "\x80\xc6\xba\xf9\x66\x18\x2b\xb3\xca\xd0\xee\x0a\x2e\x32\x5f\xb8"
      "\x47\xd3\xd3\xaf\x99\x25\xe5\x17\x41\xb3\x20\x16\x4d\x0a\xb5\x4d"
      "\x3e\xe3\x26\xde\xc8\xf0\xe2\xf9\xb1\x00\x40\x69\xd4\xec\x28\x54"
      "\x35\x1f\x75\x27\x96\x54\x8c\x84\xf2\x44\x4a\x9a\x17\x9f\x59\x9a"
      "\xdb\xcd\xf8\x50\xea\x52\x5e\xde\x90\x51\x98\x66\xbd\x54\xda\x72"
      "\xb4\xed\xbd\xba\x5a\x62\xda\x0a\x0c\x45\xb2\x58\x77\x3d\x85\x09"
      "\x85\xb4\x50\x83\xea\xe8\x48\xed\x5a\x58\xd9\x7e\x49\x07\x98\xef"
      "\x16\x29\x33\x29\x96\x08\xb6\xdf\x43\x61\x3c\xbf\x36\x18\xbf\xb4"
      "\x1f\x9a\x4b\xfe\xbc\x40\xe6\xed\x27\x79\xe6\x30\xbd\x6b\x69\x08"
      "\x57\xb2\xd1\xd8\xb3\x7e\x76\x26\x12\x83\x72\x75\xe1\xa4\xd0\xe4"
      "\xb0\x78\x2f\xcc\xae\x7e\x1f\xd2\xff\x49\xeb\xb1\x8b\x35\x26\xc1"
      "\xc9\xef\x76\xad\x5b\xa5\x99\x0f\x28\xfd\x75\x1e\xd8\x1a\x07\x58"
      "\x29\x03\xef\x69\x4a\xfb\x1b\x8f\x75\x31\x1e\x28\xd1\x88\x6a\x21"
      "\x49\x39\x93\x51\xd9\xb3\x20\x16\x53\xd8\x55\x0e\x55\x0e\x42\x50"
      "\x02\xae\xd3\xe3\xcd\x49\x30\x8d\xec\x24\xc1\x1b\xeb\x4c\x35\x1f"
      "\xf8\x0a\x71\x81\xfc\xaf\x55\x6b\xfa\x58\xd8\xa3\x5f\xd5\xdc\xf4"
      "\x85\xd7\x74\xc8\xd2\x23\xcf\x20\x9c\x4e\xa1\xf5\x70\xeb\xbd\xc7"
      "\xcc\x33\xd6\xfe\x7b\xdd\xe8\xb0\xd1\x58\x9a\xb8\x47\x6e\xaa\xe1"
      "\x0e\x6f\x89\xb9\xbc\x4b\x7b\x00\x97\xc1\x7b\x4c\xc8\x26\x0b\x2a"
      "\x5a\x53\x1e\x52\xc9\x28\x1e\xdb\xa1\x60\x67\x8e\x98\xba\x69\x10"
      "\x32\x8d\x54\x0c\xb5\xef\x00\x0a\xa0\x45\xc1\x81\xf0\xcc\x01\xca"
      "\x50\x00\x0d\x7d\x40\x82\x50\x5c\x14\xae\xbd\xd3\xf9\xce\xa9\x87"
      "\xb8\xc1\x26\x0c\x12\xc0\xc2\xa2\xef\x61\x28\x84\x1f\x85\x9e\x5f"
      "\xfd\xbc\x1e\xea\x1b\x5c\x62\x86\xfb\x09\x06\xed\x02\x70\x98\x54"
      "\x98\x56\x46\xfd\xd4\x98\x1f\x05\x87\x4b\x62\x04\x9e\xff\xf0\x66"
      "\x02\x7c\xd1\x2e\xd7\x05\x44\x77\x29\xd1\xd7\xae\x81\x60\x3c\xf9"
      "\x12\x94\x06\x34\x30\x1d\xf6\xd4\xcf\x93\x0b\xdd\xd9\xda\x13\x14"
      "\x55\x3e\xaf\x70\x7a\xab\x4f\x91\xad\xeb\x32\xc5\xe4\x98\x3e\x9f"
      "\xe1\x67\x80\xbe\x8f\xc5\x39\xd5\xfc\x65\x1d\x21\xa9\xde\x53\xe7"
      "\x26\xe5\xbf\xe0\xcd\x96\x94\x61\x81\x09\xfd\xb5\x8f\xbe\x8b\x33"
      "\xfd\xb0\xc3\xdf\x8d\x67\xd7\x0a\x03\x2f\x47\x16\x78\x60\xdd\x7e"
      "\x5e\xc1\x30\xac\xc0\x3c\xec\x35\xb6\x22\x39\xf6\x73\x3b\x36\x52"
      "\xfc\xa5\xd9\x0f\x50\xb8\x5d\xa5\x75\xfd\x55\xce\xbe\x45\x3c\xa9"
      "\x3b\x5b\xf0\x33\x77\xd2\xc1\x93\x7b\x11\xfe\xc2\x3b\x84\x8a\x0c"
      "\xb2\xc0\x16\x48\xb6\xcc\xb7\x17\xe2\x7a\x7f\xc6\x28\x1d\xae\x43"
      "\x12\xb9\x8c\xc4\x48\x67\x46\x0d\x5f\x2c\x70\x88\x87\x5b\x67\x0f"
      "\xa9\xb8\xd7\x2f\xb3\xcc\xc3\x55\x3f\x1a\x51\x48\x31\xea\x66\xd3"
      "\xd6\xc6\x94\x52\x78\x65\x1d\x58\x21\xd1\x39\x99\x27\x96\xf3\x82"
      "\x90\x0a\xf9\x74\x96\x6c\x36\x1b\x2d\x03\xb9\xeb\x06\x58\x86\xf0"
      "\x5b\x8c\x70\xbf\x32\xf8\xd7\x03\x56\x10\x78\x76\x8f\xd4\xd3\xdc"
      "\x43\x23\x9a\xe6\xdb\xfd\x56\x14\x86\x3c\x73\xdb\xda\x2b\xc5\xcd"
      "\x74\x0d\x7e\xdd\x51\x60\x1c\x11\x4a\xd2\xe6\x7a\xa3\xf7\x99\xcb"
      "\x33\xfb\xcc\xbb\x81\x35\xe3\xd4\xc9\xc2\x37\x95\x03\x5b\xd2\xf0"
      "\xe7\xcf\x4e\x60\x97\x0a\x28\x02\x1a\x82\xeb\x82\x1a\x5e\xe4\x99"
      "\xc9\xa6\xc1\x9a\x58\xc3\x98\xaa\xe9\x86\x6a\x7c\xcd\x7d\x88\x55"
      "\xc9\x15\x91\x17\xc7\x46\xa4\x7d\x37\x38\x25\x53\x3a\x69\x93\x72"
      "\x82\x72\xe3\x7c\x20\xb4\x83\xbb\xda\x67\x3a\xba\x21\xf8\xc7\xe2"
      "\xe5\xcf\x88\x77\x5d\x04\x8e\xea\x8e\x76\xa5\x29\xba\x49\x45\x8e"
      "\xf7\x8d\x8d\xde\x2a\xdf\x0f\xeb\x7e\xa2\xd4\xc4\x2b\x9a\xfd\x00"
      "\x42\x72\x52\x34\xb5\xe9\xc7\x86\xa2\xeb\x1b\x2d\xe9\xc5\x9d\x66"
      "\xad\x2b\xc4\x3a\x2e\x93\x62\x36\xc4\x99\x39\xcf\x4b\xa6\xa5\xd4"
      "\x46\x46\x11\xef\xfb\xd6\xe2\x0a\xf0\x37\x88\xd5\xd4\x6a\x6e\xb0"
      "\xf2\xe5\x9c\x5a\x0d\x78\xf4\x33\x32\x21\x48\x5c\xd4\xbc\x7b\x5f"
      "\x19\x1b\x39\xd3\xdf\x09\x38\xbe\xae\x17\xbe\xb0\x1d\x0e\xde\xec"
      "\xab\x4b\x8f\x66\x75\x48\x85\xba\xa7\xc2\xc7\xa9\x85\xe4\x5e\xbf"
      "\xb5\x67\x09\x75\x51\x7e\x1d\x07\x73\xc7\xbc\x01\x59\xb0\x3a\x94"
      "\x60\xe4\x45\x2d\x0b\xad\xf9\x6f\xc5\xd1\x5c\x41\x9d\x9a\xfe\x2e"
      "\x30\x61\x75\x64\x0f\xfa\xd9\xf7\x75\xee\xcb\x0c\x95\x44\x3c\xf8"
      "\x5a\xc8\x1a\x33\xcc\x52\x6b\xbc\x57\x0c\x54\xd6\x46\xa5\xc1\xea"
      "\xcc\x41\x79\x0d\x3e\xd1\x92\xf8\x83\x86\x95\x30\x63\xac\xe3\x8a"
      "\xb2\x0d\xdc\x61\x21\x59\xad\x17\x64\x42\x07\x0a\x08\xb8\xbe\xe1"
      "\x55\xe7\x91\xdc\xf6\xbb\x49\x94\xb3\xf0\x73\x6d\xed\x65\xd1\x20"
      "\xab\x0f\x12\x65\xaf\xe7\x4c\x2c\xc8\x9a\x3d\xc3\x53\x2a\xcf\x96"
      "\xef\x53\xa0\xbb\x82\x65\x2b\xac\x8d\xb2\x16\xbc\x1a\x5d\xf2\xd9"
      "\x77\xec\x03\xb8\x94\x54\xd8\x5e\x7d\x99\x53\x7f\x04\xce\x5d\x9b"
      "\xcc\xa6\xfe\x07\x8a\x7a\x70\xbc\xd7\xb1\x67\xb8\x5d\x6a\x39\xde"
      "\x68\xc2\x31\xe2\xf9\xee\xbf\xc3\xec\xbf\x0c\xc5\x0c\x3d\xad\xc4"
      "\x4b\x6c\x0b\x92\xd1\xc3\xf4\xe3\x40\xc1\x4f\xc8\x8d\xd6\xd9\xa3"
      "\x84\x0a\x26\x9f\x56\xd3\xe1\xe6\x3a\x38\x4e\x01\x35\xb2\xe9\xcd"
      "\x71\x22\x92\x5c\x97\xf9\xae\xab\x56\x6b\x17\xbb\x98\x0b\x0f\x77"
      "\x05\xf2\x2b\x8a\x8c\x3c\xae\xad\xbb\x03\xe6\x7c\xf6\x5a\xb7\x1a"
      "\x47\x9b\x7e\x06\x39\x36\xdc\x01\xcb\xa5\x85\x02\x2d\x0f\xf1\x10"
      "\x80\xcb\xb0\xbf\x41\x08\x96\xff\x82\xf3\x8e\xe4\x1f\x08\xf4\x4c"
      "\x4e\xed\xa9\x8e\x94\x51\xba\x58\x9b\x36\x59\xe3\x9d\x04\xaa\xa8"
      "\xe6\x08\x90\x28\x30\xd3\x4f\x18\x12\xc6\x37\xfc\xf6\xfc\x17\x13"
      "\xbf\xef\x1a\x93\x98\xea\xaa\x8c\x8e\x90\xa5\x7b\x6d\x39\x28\x52"
      "\xf4\xc5\xb5\x0f\x79\x74\xed\xc0\xfb\x3d\x71\x9c\x88\xea\x9d\x3e"
      "\x16\x23\xcc\x0e\x5a\xf9\x5d\x04\x28\xdc\xb9\x97\xdf\xdb\x95\x01"
      "\x05\xf0\xce\x6c\xd9\x4c\xcf\xd0\xd0\x27\xcb\x4d\x98\xb2\x33\x45"
      "\x2e\x2a\xed\x63\xc6\xcc\xf8\x97\xfe\xaf\x15\xe1\xba\xec\xf6\x03"
      "\x91\xbb\x0f\x22\xf1\x65\x3a\xf4\x6f\xc5\x47\x1a\x17\xed\x4e\xd9"
      "\xa9\xc5\x4c\x9a\xc3\xbc\x57\xf2\xe1\x68\x1b\x6e\xde\xe8\x68\x26"
      "\x41\x8d\x41\x56\xca\x22\x24\x5a\x42\x41\x2d\x35\x36\xe5\xbf\x03"
      "\xf4\xf9\xd9\xf9\x29\x9d\x6b\xf6\xfc\x11\x13\x47\x2e\x9f\xaf\x2f"
      "\x36\x2f\xe9\x2c\x46\x54\x7e\xce\x4a\x7a\x7c\x70\xc1\x55\xb8\xbf"
      "\x72\x0d\x81\x83\xa3\xe4\xbd\x6f\x5c\xdf\x61\x09\x4b\x7b\x40\xe0"
      "\x5b\xfe\x01\xbc\x3c\xa6\xe5\x2d\x4d\x2a\x30\x42\x55\x95\x0b\x52"
      "\xea\x58\x26\xe0\x74\x53\xc3\x50\x98\xd7\x31\xd0\x41\x94\xc0\x64"
      "\x45\x0b\xd6\xad\x99\x9b\x6d\xa0\x9a\x04\x74\xd0\xa9\xd2\xe3\x37"
      "\x57\x91\xba\xac\x96\x24\x12\x86\x37\x0b\xc9\x63\x4f\x8d\xb8\xe6"
      "\x31\xea\x48\xb6\x06\x88\x85\xed\x79\xe3\x93\x2e\xa0\x18\x64\xc3"
      "\xc3\x20\xdc\x6f\x32\x68\x91\xfd\x2f\xd0\x76\x55\x0a\xd5\xe8\xc2"
      "\x5e\x4e\x55\x8f\xdb\x1e\xa4\x52\x4f\x55\x3a\x2a\x60\xcc\xf9\xde"
      "\xba\xfe\x3b\x06\x8d\x96\x7c\x61\x88\xb8\x3f\xcb\x4c\xc2\x7f\xfe"
      "\x23\x5c\x50\xd7\xbc\x53\x68\x23\xd5\x84\x1d\x56\x31\xde\x09\xc6"
      "\x63\x7b\x55\xde\x67\xb2\x44\x44\xbf\x9c\x33\x61\x91\xd5\xab\x8d"
      "\x0d\x48\x5d\xb9\xb6\x31\x89\x50\xa3\x53\x86\x5b\x86\x7c\xd7\xc2"
      "\x44\x0b\xc8\x4d\xf4\x83\x5e\xbe\x7b\x04\xf5\xaa\xc7\xfb\x78\x00"
      "\xe6\x85\x27\x6d\xf4\x71\xd4\xbb\x35\xe9\x73\x23\x27\x9c\xca\xbe"
      "\x0e\xd2\x69\x37\x12\x14\xb7\xd8\x4a\xb6\xae\x1b\x9a\x07\x05\x6b"
      "\x12\x06\x5e\x89\xe0\x76\xc9\x34\x91\x39\xd4\x66\x72\xfe\x35\x9f"
      "\x82\xd7\xea\x6c\x91\x72\x01\x0c\x80\x61\x37\x97\xce\xb3\x2e\x2f"
      "\xab\x76\xc6\x16\xd8\x7c\x43\x3a\x09\xaf\x5e\x65\xd7\xa8\x16\x0c"
      "\x87\x59\x06\x8b\x37\xac\x63\xbd\xde\xb5\xe3\x8d\xac\x8f\x73\x73"
      "\x14\x1c\x3b\x20\xce\x33\x17\xaf\x0f\xa5\x9c\x3d\x0d\xee\x58\x0c"
      "\xd1\x50\x9e\xa7\x70\x8e\x7e\xad\x62\xea\xcd\x02\xa6\x82\x76\x68"
      "\x41\x7d\x31\x11\x7f\x78\x0a\x4c\xbe\x85\x0d\xff\x15\x39\x5a\x2a"
      "\x09\xc3\x47\xdf\xb0\xb5\x33\xdb\x58\x90\x9f\x2b\x3a\xb0\x90\x69"
      "\xb6\xee\xab\x68\xac\xe0\x15\x4c\x00\x16\x4c\x6d\xaf\xc2\xef\x30"
      "\x5e\xdc\x55\x8e\xc6\xb7\x1f\x6f\xf7\xd4\x58\x67\x39\x51\xdd\x79"
      "\xaa\x6b\xbd\x7c\x0e\x24\x95\xfe\x54\x16\x9c\x5e\xd3\x40\x0f\xcd"
      "\xeb\xcc\xea\xe4\x29\x77\xab\x04\x5a\x2f\x68\xba\x5b\xdb\x86\xf0"
      "\xce\xf3\xa4\x85\xb1\xb6\xcb\x56\x0a\xf6\x98\x29\x5e\xa9\xe8\x6a"
      "\x7c\x28\xdb\xf6\xa3\x60\xbe\x17\xe1\x48\xce\xf4\x95\xaf\xcf\xcb"
      "\x88\xfa\x1b\x42\xc1\xe1\x43\x5a\xfb\xec\xa1\x7e\x94\x64\x39\x8e"
      "\x24\xb3\x94\x81\x66\x52\x75\xfd\x98\xee\x80\x52\xff\xbd\x9e\xcf"
      "\xaf\xdc\x3f\x43\x84\xe1\xe5\x88\xff\x16\x04\x47\xce\xcd\x4d\x34"
      "\x95\x00\x6a\x24\x55\xd9\x0d\xc8\xe3\xa6\x1a\x8f\x53\x68\x02\xc7"
      "\x5a\x92\xf9\xdb\xd2\xcb\x67\x70\xd1\xc8\x7b\x4e\x84\x1c\x8d\xd7"
      "\xf5\xf7\x10\x8a\xd3\x30\x6f\x24\xa3\x24\x1e\x3c\x4d\x40\xcf\x57"
      "\x9a\xc8\xb0\xd1\xae\x5b\x07\xa5\xfe\x0c\xea\x19\x72\x6c\x92\x68"
      "\x6d\xee\x15\x64\xbd\x3a\x2a\x14\xc0\x23\xec\xcb\xdc\xbc\x1d\x8f"
      "\x3c\xbb\x7c\xe4\xfc\xef\x5c\xc9\x28\xdf\x5b\x8f\x57\x5e\x42\xe8"
      "\x76\xc3\x01\xd4\xf6\x3d\xe7\x66\x2c\x1f\x8f\x58\x7a\x55\x19\xb5"
      "\x29\x0a\x80\x83\x86\x96\xe2\x80\x06\x89\xd3\xf4\xc0\x45\x02\x7b"
      "\xc2\xdd\xce\x1f\xbb\x70\x30\xae\x23\x31\xe6\x8e\x01\x70\x26\x34"
      "\x3b\x34\x8a\xef\x2a\x34\x58\xba\x74\x6c\xa4\x02\x3a\x1d\x89\x34"
      "\x0b\x65\x17\x54\xa1\x21\xaa\xcd\x0a\x1d\x79\xec\xec\x8c\x27\x40"
      "\xb7\x33\x83\xf1\x6c\xaf\x64\x25\xe7\xed\x5e\x09\xd2\xbf\x9f\x49"
      "\x11\xaa\x63\xd6\xee\x42\xb4\xfa\x26\x81\x4c\xa3\xff\x47\x6c"
      "\x47",
      4096);
  *(uint64_t*)0x20f75000 = (uint64_t)0x18;
  *(uint32_t*)0x20f75008 = (uint32_t)0x117;
  *(uint32_t*)0x20f7500c = (uint32_t)0x3;
  *(uint32_t*)0x20f75010 = (uint32_t)0x0;
  r[33] = syscall(__NR_sendmsg, r[10], 0x20f74fc8ul, 0x0ul);
  r[34] = syscall(__NR_io_setup, 0x8000000001ul, 0x20b73ff8ul);
  if (r[34] != -1)
    r[35] = *(uint64_t*)0x20b73ff8;
  *(uint64_t*)0x20738000 = (uint64_t)0x20f73fc0;
  *(uint64_t*)0x20f73fc0 = (uint64_t)0x0;
  *(uint32_t*)0x20f73fc8 = (uint32_t)0x0;
  *(uint32_t*)0x20f73fcc = (uint32_t)0x0;
  *(uint16_t*)0x20f73fd0 = (uint16_t)0x0;
  *(uint16_t*)0x20f73fd2 = (uint16_t)0x0;
  *(uint32_t*)0x20f73fd4 = r[10];
  *(uint64_t*)0x20f73fd8 = (uint64_t)0x2079a000;
  *(uint64_t*)0x20f73fe0 = (uint64_t)0x10;
  *(uint64_t*)0x20f73fe8 = (uint64_t)0x0;
  *(uint64_t*)0x20f73ff0 = (uint64_t)0x0;
  *(uint32_t*)0x20f73ff8 = (uint32_t)0x0;
  *(uint32_t*)0x20f73ffc = (uint32_t)0xffffffffffffffff;
  memcpy((void*)0x2079a000, "\x16\x80\xb5\x6c\x88\xe4\x52\xf0\xc9\x0a"
                            "\xfe\xde\x60\x61\xa2\x0a",
         16);
  r[50] = syscall(__NR_io_submit, r[35], 0x1ul, 0x20738000ul);
  r[51] = syscall(__NR_sched_yield);
}

int main()
{
  int i;
  for (i = 0; i < 8; i++) {
    if (fork() == 0) {
      setup_tun(i, true);
      loop();
      return 0;
    }
  }
  sleep(1000000);
  return 0;
}