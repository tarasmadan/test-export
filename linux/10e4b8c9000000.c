// https://syzkaller.appspot.com/bug?id=dc27604a888bd71bab2691b1b9867e84834d3049
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}

__attribute__((noreturn)) static void fail(const char* msg, ...)
{
  int e = errno;
  fflush(stdout);
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit((e == ENOMEM || e == EAGAIN) ? kRetryStatus : kFailStatus);
}

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                          \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)              \
  if ((bf_off) == 0 && (bf_len) == 0) {                                \
    *(type*)(addr) = (type)(val);                                      \
  } else {                                                             \
    type new_val = *(type*)(addr);                                     \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));             \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);  \
    *(type*)(addr) = new_val;                                          \
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

static void execute_command(const char* format, ...)
{
  va_list args;
  char command[COMMAND_MAX_LEN];
  int rv;

  va_start(args, format);

  vsnprintf_check(command, sizeof(command), format, args);
  rv = system(command);
  if (rv != 0)
    fail("tun: command \"%s\" failed with code %d", &command[0], rv);

  va_end(args);
}

static int tunfd = -1;

#define SYZ_TUN_MAX_PACKET_SIZE 1000

#define MAX_PIDS 32
#define ADDR_MAX_LEN 32

#define LOCAL_MAC "aa:aa:aa:aa:aa:%02hx"
#define REMOTE_MAC "bb:bb:bb:bb:bb:%02hx"

#define LOCAL_IPV4 "172.20.%d.170"
#define REMOTE_IPV4 "172.20.%d.187"

#define LOCAL_IPV6 "fe80::%02hxaa"
#define REMOTE_IPV6 "fe80::%02hxbb"

static void initialize_tun(uint64_t pid)
{
  if (pid >= MAX_PIDS)
    fail("tun: no more than %d executors", MAX_PIDS);
  int id = pid;

  tunfd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
  if (tunfd == -1)
    fail("tun: can't open /dev/net/tun");

  char iface[IFNAMSIZ];
  snprintf_check(iface, sizeof(iface), "syz%d", id);

  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, iface, IFNAMSIZ);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0)
    fail("tun: ioctl(TUNSETIFF) failed");

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
  int rv = read(tunfd, data, size);
  if (rv < 0) {
    if (errno == EAGAIN)
      return -1;
    fail("tun: read failed with %d, errno: %d", rv, errno);
  }
  return rv;
}

struct csum_inet {
  uint32_t acc;
};

static void csum_inet_init(struct csum_inet* csum)
{
  csum->acc = 0;
}

static void csum_inet_update(struct csum_inet* csum,
                             const uint8_t* data, size_t length)
{
  if (length == 0)
    return;

  size_t i;
  for (i = 0; i < length - 1; i += 2)
    csum->acc += *(uint16_t*)&data[i];

  if (length & 1)
    csum->acc += (uint16_t)data[length - 1];

  while (csum->acc > 0xffff)
    csum->acc = (csum->acc & 0xffff) + (csum->acc >> 16);
}

static uint16_t csum_inet_digest(struct csum_inet* csum)
{
  return ~csum->acc;
}

static uintptr_t syz_emit_ethernet(uintptr_t a0, uintptr_t a1)
{

  if (tunfd < 0)
    return (uintptr_t)-1;

  int64_t length = a0;
  char* data = (char*)a1;
  return write(tunfd, data, length);
}

struct ipv6hdr {
  __u8 priority : 4, version : 4;
  __u8 flow_lbl[3];

  __be16 payload_len;
  __u8 nexthdr;
  __u8 hop_limit;

  struct in6_addr saddr;
  struct in6_addr daddr;
};

struct tcp_resources {
  int32_t seq;
  int32_t ack;
};

static uintptr_t syz_extract_tcp_res(uintptr_t a0, uintptr_t a1,
                                     uintptr_t a2)
{

  if (tunfd < 0)
    return (uintptr_t)-1;

  char data[SYZ_TUN_MAX_PACKET_SIZE];
  int rv = read_tun(&data[0], sizeof(data));
  if (rv == -1)
    return (uintptr_t)-1;
  size_t length = rv;

  struct tcphdr* tcphdr;

  if (length < sizeof(struct ethhdr))
    return (uintptr_t)-1;
  struct ethhdr* ethhdr = (struct ethhdr*)&data[0];

  if (ethhdr->h_proto == htons(ETH_P_IP)) {
    if (length < sizeof(struct ethhdr) + sizeof(struct iphdr))
      return (uintptr_t)-1;
    struct iphdr* iphdr = (struct iphdr*)&data[sizeof(struct ethhdr)];
    if (iphdr->protocol != IPPROTO_TCP)
      return (uintptr_t)-1;
    if (length <
        sizeof(struct ethhdr) + iphdr->ihl * 4 + sizeof(struct tcphdr))
      return (uintptr_t)-1;
    tcphdr =
        (struct tcphdr*)&data[sizeof(struct ethhdr) + iphdr->ihl * 4];
  } else {
    if (length < sizeof(struct ethhdr) + sizeof(struct ipv6hdr))
      return (uintptr_t)-1;
    struct ipv6hdr* ipv6hdr =
        (struct ipv6hdr*)&data[sizeof(struct ethhdr)];
    if (ipv6hdr->nexthdr != IPPROTO_TCP)
      return (uintptr_t)-1;
    if (length < sizeof(struct ethhdr) + sizeof(struct ipv6hdr) +
                     sizeof(struct tcphdr))
      return (uintptr_t)-1;
    tcphdr = (struct tcphdr*)&data[sizeof(struct ethhdr) +
                                   sizeof(struct ipv6hdr)];
  }

  struct tcp_resources* res = (struct tcp_resources*)a0;
  res->seq = htonl((ntohl(tcphdr->seq) + (uint32_t)a1));
  res->ack = htonl((ntohl(tcphdr->ack_seq) + (uint32_t)a2));

  return 0;
}

static void test();

void loop()
{
  while (1) {
    test();
  }
}

long r[191];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    r[1] = syscall(__NR_socket, 0xful, 0x3ul, 0x2ul);
    break;
  case 2:
    memcpy((void*)0x20002ff0, "\x02\x12\xa1\x25\x02\x00\x00\x00\x09\x05"
                              "\x00\xd9\x1d\xfb\x37\x00",
           16);
    r[3] = syscall(__NR_write, r[1], 0x20002ff0ul, 0x10ul);
    break;
  case 3:
    r[4] = syscall(__NR_socket, 0x2ul, 0x1ul, 0x0ul);
    break;
  case 4:
    *(uint16_t*)0x20001000 = (uint16_t)0x2;
    *(uint16_t*)0x20001002 = (uint16_t)0x204e;
    *(uint32_t*)0x20001004 = (uint32_t)0x0;
    *(uint8_t*)0x20001008 = (uint8_t)0x0;
    *(uint8_t*)0x20001009 = (uint8_t)0x0;
    *(uint8_t*)0x2000100a = (uint8_t)0x0;
    *(uint8_t*)0x2000100b = (uint8_t)0x0;
    *(uint8_t*)0x2000100c = (uint8_t)0x0;
    *(uint8_t*)0x2000100d = (uint8_t)0x0;
    *(uint8_t*)0x2000100e = (uint8_t)0x0;
    *(uint8_t*)0x2000100f = (uint8_t)0x0;
    r[16] = syscall(__NR_bind, r[4], 0x20001000ul, 0x10ul);
    break;
  case 5:
    r[17] = syscall(__NR_listen, r[4], 0x0ul);
    break;
  case 6:
    *(uint8_t*)0x20002000 = (uint8_t)0xaa;
    *(uint8_t*)0x20002001 = (uint8_t)0xaa;
    *(uint8_t*)0x20002002 = (uint8_t)0xaa;
    *(uint8_t*)0x20002003 = (uint8_t)0xaa;
    *(uint8_t*)0x20002004 = (uint8_t)0xaa;
    *(uint8_t*)0x20002005 = (uint8_t)0x0;
    memcpy((void*)0x20002006, "\x4c\x61\x12\xcc\x15\xd8", 6);
    *(uint16_t*)0x2000200c = (uint16_t)0x8;
    STORE_BY_BITMASK(uint8_t, 0x2000200e, 0x5, 0, 4);
    STORE_BY_BITMASK(uint8_t, 0x2000200e, 0x4, 4, 4);
    STORE_BY_BITMASK(uint8_t, 0x2000200f, 0x0, 0, 2);
    STORE_BY_BITMASK(uint8_t, 0x2000200f, 0x0, 2, 6);
    *(uint16_t*)0x20002010 = (uint16_t)0x2800;
    *(uint16_t*)0x20002012 = (uint16_t)0x6400;
    *(uint16_t*)0x20002014 = (uint16_t)0x0;
    *(uint8_t*)0x20002016 = (uint8_t)0x0;
    *(uint8_t*)0x20002017 = (uint8_t)0x6;
    *(uint16_t*)0x20002018 = (uint16_t)0x0;
    *(uint8_t*)0x2000201a = (uint8_t)0xac;
    *(uint8_t*)0x2000201b = (uint8_t)0x14;
    *(uint8_t*)0x2000201c = (uint8_t)0x0;
    *(uint8_t*)0x2000201d = (uint8_t)0xbb;
    *(uint8_t*)0x2000201e = (uint8_t)0xac;
    *(uint8_t*)0x2000201f = (uint8_t)0x14;
    *(uint8_t*)0x20002020 = (uint8_t)0x0;
    *(uint8_t*)0x20002021 = (uint8_t)0xaa;
    *(uint16_t*)0x20002022 = (uint16_t)0x214e;
    *(uint16_t*)0x20002024 = (uint16_t)0x204e;
    *(uint32_t*)0x20002026 = (uint32_t)0x42424242;
    *(uint32_t*)0x2000202a = (uint32_t)0x42424242;
    STORE_BY_BITMASK(uint8_t, 0x2000202e, 0x0, 0, 1);
    STORE_BY_BITMASK(uint8_t, 0x2000202e, 0x0, 1, 3);
    STORE_BY_BITMASK(uint8_t, 0x2000202e, 0x5, 4, 4);
    *(uint8_t*)0x2000202f = (uint8_t)0x2;
    *(uint16_t*)0x20002030 = (uint16_t)0x0;
    *(uint16_t*)0x20002032 = (uint16_t)0x0;
    *(uint16_t*)0x20002034 = (uint16_t)0x0;
    struct csum_inet csum_55;
    csum_inet_init(&csum_55);
    csum_inet_update(&csum_55, (const uint8_t*)0x2000201a, 4);
    csum_inet_update(&csum_55, (const uint8_t*)0x2000201e, 4);
    uint16_t csum_55_chunk_2 = 0x600;
    csum_inet_update(&csum_55, (const uint8_t*)&csum_55_chunk_2, 2);
    uint16_t csum_55_chunk_3 = 0x1400;
    csum_inet_update(&csum_55, (const uint8_t*)&csum_55_chunk_3, 2);
    csum_inet_update(&csum_55, (const uint8_t*)0x20002022, 20);
    *(uint16_t*)0x20002032 = csum_inet_digest(&csum_55);
    struct csum_inet csum_56;
    csum_inet_init(&csum_56);
    csum_inet_update(&csum_56, (const uint8_t*)0x2000200e, 20);
    *(uint16_t*)0x20002018 = csum_inet_digest(&csum_56);
    r[57] = syz_emit_ethernet(0x36ul, 0x20002000ul);
    break;
  case 7:
    *(uint8_t*)0x2001a000 = (uint8_t)0x0;
    *(uint8_t*)0x2001a001 = (uint8_t)0x0;
    *(uint8_t*)0x2001a002 = (uint8_t)0x0;
    *(uint8_t*)0x2001a003 = (uint8_t)0x0;
    *(uint8_t*)0x2001a004 = (uint8_t)0x0;
    *(uint8_t*)0x2001a005 = (uint8_t)0x0;
    *(uint8_t*)0x2001a006 = (uint8_t)0x0;
    *(uint8_t*)0x2001a007 = (uint8_t)0x0;
    *(uint8_t*)0x2001a008 = (uint8_t)0x0;
    *(uint8_t*)0x2001a009 = (uint8_t)0x0;
    *(uint8_t*)0x2001a00a = (uint8_t)0x0;
    *(uint8_t*)0x2001a00b = (uint8_t)0x0;
    *(uint8_t*)0x2001a00c = (uint8_t)0x0;
    *(uint8_t*)0x2001a00d = (uint8_t)0x0;
    *(uint8_t*)0x2001a00e = (uint8_t)0x0;
    *(uint8_t*)0x2001a00f = (uint8_t)0x0;
    *(uint8_t*)0x2001a010 = (uint8_t)0xfe;
    *(uint8_t*)0x2001a011 = (uint8_t)0x80;
    *(uint8_t*)0x2001a012 = (uint8_t)0x0;
    *(uint8_t*)0x2001a013 = (uint8_t)0x0;
    *(uint8_t*)0x2001a014 = (uint8_t)0x0;
    *(uint8_t*)0x2001a015 = (uint8_t)0x0;
    *(uint8_t*)0x2001a016 = (uint8_t)0x0;
    *(uint8_t*)0x2001a017 = (uint8_t)0x0;
    *(uint8_t*)0x2001a018 = (uint8_t)0x0;
    *(uint8_t*)0x2001a019 = (uint8_t)0x0;
    *(uint8_t*)0x2001a01a = (uint8_t)0x0;
    *(uint8_t*)0x2001a01b = (uint8_t)0x0;
    *(uint8_t*)0x2001a01c = (uint8_t)0x0;
    *(uint8_t*)0x2001a01d = (uint8_t)0x0;
    *(uint8_t*)0x2001a01e = (uint8_t)0x0;
    *(uint8_t*)0x2001a01f = (uint8_t)0xbb;
    *(uint16_t*)0x2001a020 = (uint16_t)0x204e;
    *(uint16_t*)0x2001a022 = (uint16_t)0x0;
    *(uint16_t*)0x2001a024 = (uint16_t)0x204e;
    *(uint16_t*)0x2001a026 = (uint16_t)0x0;
    *(uint16_t*)0x2001a028 = (uint16_t)0xa;
    *(uint8_t*)0x2001a02a = (uint8_t)0x0;
    *(uint8_t*)0x2001a02b = (uint8_t)0x0;
    *(uint8_t*)0x2001a02c = (uint8_t)0x0;
    *(uint32_t*)0x2001a030 = (uint32_t)0x0;
    *(uint32_t*)0x2001a034 = (uint32_t)0x0;
    *(uint64_t*)0x2001a038 = (uint64_t)0x0;
    *(uint64_t*)0x2001a040 = (uint64_t)0x0;
    *(uint64_t*)0x2001a048 = (uint64_t)0x0;
    *(uint64_t*)0x2001a050 = (uint64_t)0x0;
    *(uint64_t*)0x2001a058 = (uint64_t)0x0;
    *(uint64_t*)0x2001a060 = (uint64_t)0x0;
    *(uint64_t*)0x2001a068 = (uint64_t)0x0;
    *(uint64_t*)0x2001a070 = (uint64_t)0x0;
    *(uint64_t*)0x2001a078 = (uint64_t)0x0;
    *(uint64_t*)0x2001a080 = (uint64_t)0x0;
    *(uint64_t*)0x2001a088 = (uint64_t)0x0;
    *(uint64_t*)0x2001a090 = (uint64_t)0x0;
    *(uint32_t*)0x2001a098 = (uint32_t)0x0;
    *(uint32_t*)0x2001a09c = (uint32_t)0x0;
    *(uint8_t*)0x2001a0a0 = (uint8_t)0x0;
    *(uint8_t*)0x2001a0a1 = (uint8_t)0x0;
    *(uint8_t*)0x2001a0a2 = (uint8_t)0x0;
    *(uint8_t*)0x2001a0a3 = (uint8_t)0x0;
    *(uint8_t*)0x2001a0a8 = (uint8_t)0x0;
    *(uint8_t*)0x2001a0a9 = (uint8_t)0x0;
    *(uint8_t*)0x2001a0aa = (uint8_t)0x0;
    *(uint8_t*)0x2001a0ab = (uint8_t)0x0;
    *(uint8_t*)0x2001a0ac = (uint8_t)0x0;
    *(uint8_t*)0x2001a0ad = (uint8_t)0x0;
    *(uint8_t*)0x2001a0ae = (uint8_t)0x0;
    *(uint8_t*)0x2001a0af = (uint8_t)0x0;
    *(uint8_t*)0x2001a0b0 = (uint8_t)0x0;
    *(uint8_t*)0x2001a0b1 = (uint8_t)0x0;
    *(uint8_t*)0x2001a0b2 = (uint8_t)0x0;
    *(uint8_t*)0x2001a0b3 = (uint8_t)0x0;
    *(uint8_t*)0x2001a0b4 = (uint8_t)0x0;
    *(uint8_t*)0x2001a0b5 = (uint8_t)0x0;
    *(uint8_t*)0x2001a0b6 = (uint8_t)0x0;
    *(uint8_t*)0x2001a0b7 = (uint8_t)0x0;
    *(uint32_t*)0x2001a0b8 = (uint32_t)0x0;
    *(uint8_t*)0x2001a0bc = (uint8_t)0x0;
    *(uint16_t*)0x2001a0c0 = (uint16_t)0x0;
    *(uint8_t*)0x2001a0c4 = (uint8_t)0xac;
    *(uint8_t*)0x2001a0c5 = (uint8_t)0x14;
    *(uint8_t*)0x2001a0c6 = (uint8_t)0x0;
    *(uint8_t*)0x2001a0c7 = (uint8_t)0xbb;
    *(uint32_t*)0x2001a0d4 = (uint32_t)0x0;
    *(uint8_t*)0x2001a0d8 = (uint8_t)0x0;
    *(uint8_t*)0x2001a0d9 = (uint8_t)0x0;
    *(uint8_t*)0x2001a0da = (uint8_t)0x5;
    *(uint32_t*)0x2001a0dc = (uint32_t)0x0;
    *(uint32_t*)0x2001a0e0 = (uint32_t)0x0;
    *(uint32_t*)0x2001a0e4 = (uint32_t)0x0;
    r[148] = syscall(__NR_setsockopt, r[4], 0x0ul, 0x11ul, 0x2001a000ul,
                     0xe8ul);
    break;
  case 8:
    r[149] = syz_extract_tcp_res(0x20016000ul, 0x1ul, 0x0ul);
    if (r[149] != -1)
      r[150] = *(uint32_t*)0x20016000;
    break;
  case 9:
    *(uint8_t*)0x20004000 = (uint8_t)0xaa;
    *(uint8_t*)0x20004001 = (uint8_t)0xaa;
    *(uint8_t*)0x20004002 = (uint8_t)0xaa;
    *(uint8_t*)0x20004003 = (uint8_t)0xaa;
    *(uint8_t*)0x20004004 = (uint8_t)0xaa;
    *(uint8_t*)0x20004005 = (uint8_t)0x0;
    memcpy((void*)0x20004006, "\xa3\xe2\xd5\x5b\xa0\x7f", 6);
    *(uint16_t*)0x2000400c = (uint16_t)0x8;
    STORE_BY_BITMASK(uint8_t, 0x2000400e, 0x5, 0, 4);
    STORE_BY_BITMASK(uint8_t, 0x2000400e, 0x4, 4, 4);
    STORE_BY_BITMASK(uint8_t, 0x2000400f, 0x0, 0, 2);
    STORE_BY_BITMASK(uint8_t, 0x2000400f, 0x0, 2, 6);
    *(uint16_t*)0x20004010 = (uint16_t)0x2800;
    *(uint16_t*)0x20004012 = (uint16_t)0x6400;
    *(uint16_t*)0x20004014 = (uint16_t)0x0;
    *(uint8_t*)0x20004016 = (uint8_t)0x0;
    *(uint8_t*)0x20004017 = (uint8_t)0x6;
    *(uint16_t*)0x20004018 = (uint16_t)0x0;
    *(uint8_t*)0x2000401a = (uint8_t)0xac;
    *(uint8_t*)0x2000401b = (uint8_t)0x14;
    *(uint8_t*)0x2000401c = (uint8_t)0x0;
    *(uint8_t*)0x2000401d = (uint8_t)0xbb;
    *(uint8_t*)0x2000401e = (uint8_t)0xac;
    *(uint8_t*)0x2000401f = (uint8_t)0x14;
    *(uint8_t*)0x20004020 = (uint8_t)0x0;
    *(uint8_t*)0x20004021 = (uint8_t)0xaa;
    *(uint16_t*)0x20004022 = (uint16_t)0x214e;
    *(uint16_t*)0x20004024 = (uint16_t)0x204e;
    *(uint32_t*)0x20004026 = (uint32_t)0x42424242;
    *(uint32_t*)0x2000402a = r[150];
    STORE_BY_BITMASK(uint8_t, 0x2000402e, 0x0, 0, 1);
    STORE_BY_BITMASK(uint8_t, 0x2000402e, 0x0, 1, 3);
    STORE_BY_BITMASK(uint8_t, 0x2000402e, 0x5, 4, 4);
    *(uint8_t*)0x2000402f = (uint8_t)0x10;
    *(uint16_t*)0x20004030 = (uint16_t)0x0;
    *(uint16_t*)0x20004032 = (uint16_t)0x0;
    *(uint16_t*)0x20004034 = (uint16_t)0x4000;
    struct csum_inet csum_188;
    csum_inet_init(&csum_188);
    csum_inet_update(&csum_188, (const uint8_t*)0x2000401a, 4);
    csum_inet_update(&csum_188, (const uint8_t*)0x2000401e, 4);
    uint16_t csum_188_chunk_2 = 0x600;
    csum_inet_update(&csum_188, (const uint8_t*)&csum_188_chunk_2, 2);
    uint16_t csum_188_chunk_3 = 0x1400;
    csum_inet_update(&csum_188, (const uint8_t*)&csum_188_chunk_3, 2);
    csum_inet_update(&csum_188, (const uint8_t*)0x20004022, 20);
    *(uint16_t*)0x20004032 = csum_inet_digest(&csum_188);
    struct csum_inet csum_189;
    csum_inet_init(&csum_189);
    csum_inet_update(&csum_189, (const uint8_t*)0x2000400e, 20);
    *(uint16_t*)0x20004018 = csum_inet_digest(&csum_189);
    r[190] = syz_emit_ethernet(0x36ul, 0x20004000ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[20];

  memset(r, -1, sizeof(r));
  for (i = 0; i < 10; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  usleep(rand() % 100000);
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
