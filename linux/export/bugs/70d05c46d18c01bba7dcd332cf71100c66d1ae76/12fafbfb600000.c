// https://syzkaller.appspot.com/bug?id=70d05c46d18c01bba7dcd332cf71100c66d1ae76
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <sched.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unistd.h>

#include <linux/capability.h>
#include <linux/if_addr.h>
#include <linux/if_ether.h>
#include <linux/if_link.h>
#include <linux/if_tun.h>
#include <linux/in6.h>
#include <linux/ip.h>
#include <linux/neighbour.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/tcp.h>
#include <linux/veth.h>

unsigned long long procid;

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

static struct {
  char* pos;
  int nesting;
  struct nlattr* nested[8];
  char buf[1024];
} nlmsg;

static void netlink_init(int typ, int flags, const void* data, int size)
{
  memset(&nlmsg, 0, sizeof(nlmsg));
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg.buf;
  hdr->nlmsg_type = typ;
  hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | flags;
  memcpy(hdr + 1, data, size);
  nlmsg.pos = (char*)(hdr + 1) + NLMSG_ALIGN(size);
}

static void netlink_attr(int typ, const void* data, int size)
{
  struct nlattr* attr = (struct nlattr*)nlmsg.pos;
  attr->nla_len = sizeof(*attr) + size;
  attr->nla_type = typ;
  memcpy(attr + 1, data, size);
  nlmsg.pos += NLMSG_ALIGN(attr->nla_len);
}

static void netlink_nest(int typ)
{
  struct nlattr* attr = (struct nlattr*)nlmsg.pos;
  attr->nla_type = typ;
  nlmsg.pos += sizeof(*attr);
  nlmsg.nested[nlmsg.nesting++] = attr;
}

static void netlink_done(void)
{
  struct nlattr* attr = nlmsg.nested[--nlmsg.nesting];
  attr->nla_len = nlmsg.pos - (char*)attr;
}

static int netlink_send(int sock)
{
  if (nlmsg.pos > nlmsg.buf + sizeof(nlmsg.buf) || nlmsg.nesting)
    exit(1);
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg.buf;
  hdr->nlmsg_len = nlmsg.pos - nlmsg.buf;
  struct sockaddr_nl addr;
  memset(&addr, 0, sizeof(addr));
  addr.nl_family = AF_NETLINK;
  unsigned n = sendto(sock, nlmsg.buf, hdr->nlmsg_len, 0,
                      (struct sockaddr*)&addr, sizeof(addr));
  if (n != hdr->nlmsg_len)
    exit(1);
  n = recv(sock, nlmsg.buf, sizeof(nlmsg.buf), 0);
  if (n < sizeof(struct nlmsghdr) + sizeof(struct nlmsgerr))
    exit(1);
  if (hdr->nlmsg_type != NLMSG_ERROR)
    exit(1);
  return -((struct nlmsgerr*)(hdr + 1))->error;
}

static void netlink_add_device_impl(const char* type, const char* name)
{
  struct ifinfomsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  netlink_init(RTM_NEWLINK, NLM_F_EXCL | NLM_F_CREATE, &hdr, sizeof(hdr));
  if (name)
    netlink_attr(IFLA_IFNAME, name, strlen(name));
  netlink_nest(IFLA_LINKINFO);
  netlink_attr(IFLA_INFO_KIND, type, strlen(type));
}

static void netlink_add_device(int sock, const char* type, const char* name)
{
  netlink_add_device_impl(type, name);
  netlink_done();
  int err = netlink_send(sock);
  (void)err;
}

static void netlink_add_veth(int sock, const char* name, const char* peer)
{
  netlink_add_device_impl("veth", name);
  netlink_nest(IFLA_INFO_DATA);
  netlink_nest(VETH_INFO_PEER);
  nlmsg.pos += sizeof(struct ifinfomsg);
  netlink_attr(IFLA_IFNAME, peer, strlen(peer));
  netlink_done();
  netlink_done();
  netlink_done();
  int err = netlink_send(sock);
  (void)err;
}

static void netlink_add_hsr(int sock, const char* name, const char* slave1,
                            const char* slave2)
{
  netlink_add_device_impl("hsr", name);
  netlink_nest(IFLA_INFO_DATA);
  int ifindex1 = if_nametoindex(slave1);
  netlink_attr(IFLA_HSR_SLAVE1, &ifindex1, sizeof(ifindex1));
  int ifindex2 = if_nametoindex(slave2);
  netlink_attr(IFLA_HSR_SLAVE2, &ifindex2, sizeof(ifindex2));
  netlink_done();
  netlink_done();
  int err = netlink_send(sock);
  (void)err;
}

static void netlink_device_change(int sock, const char* name, bool up,
                                  const char* master, const void* mac,
                                  int macsize)
{
  struct ifinfomsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  if (up)
    hdr.ifi_flags = hdr.ifi_change = IFF_UP;
  netlink_init(RTM_NEWLINK, 0, &hdr, sizeof(hdr));
  netlink_attr(IFLA_IFNAME, name, strlen(name));
  if (master) {
    int ifindex = if_nametoindex(master);
    netlink_attr(IFLA_MASTER, &ifindex, sizeof(ifindex));
  }
  if (macsize)
    netlink_attr(IFLA_ADDRESS, mac, macsize);
  int err = netlink_send(sock);
  (void)err;
}

static int netlink_add_addr(int sock, const char* dev, const void* addr,
                            int addrsize)
{
  struct ifaddrmsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  hdr.ifa_family = addrsize == 4 ? AF_INET : AF_INET6;
  hdr.ifa_prefixlen = addrsize == 4 ? 24 : 120;
  hdr.ifa_scope = RT_SCOPE_UNIVERSE;
  hdr.ifa_index = if_nametoindex(dev);
  netlink_init(RTM_NEWADDR, NLM_F_CREATE | NLM_F_REPLACE, &hdr, sizeof(hdr));
  netlink_attr(IFA_LOCAL, addr, addrsize);
  netlink_attr(IFA_ADDRESS, addr, addrsize);
  return netlink_send(sock);
}

static void netlink_add_addr4(int sock, const char* dev, const char* addr)
{
  struct in_addr in_addr;
  inet_pton(AF_INET, addr, &in_addr);
  int err = netlink_add_addr(sock, dev, &in_addr, sizeof(in_addr));
  (void)err;
}

static void netlink_add_addr6(int sock, const char* dev, const char* addr)
{
  struct in6_addr in6_addr;
  inet_pton(AF_INET6, addr, &in6_addr);
  int err = netlink_add_addr(sock, dev, &in6_addr, sizeof(in6_addr));
  (void)err;
}

static void netlink_add_neigh(int sock, const char* name, const void* addr,
                              int addrsize, const void* mac, int macsize)
{
  struct ndmsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  hdr.ndm_family = addrsize == 4 ? AF_INET : AF_INET6;
  hdr.ndm_ifindex = if_nametoindex(name);
  hdr.ndm_state = NUD_PERMANENT;
  netlink_init(RTM_NEWNEIGH, NLM_F_EXCL | NLM_F_CREATE, &hdr, sizeof(hdr));
  netlink_attr(NDA_DST, addr, addrsize);
  netlink_attr(NDA_LLADDR, mac, macsize);
  int err = netlink_send(sock);
  (void)err;
}

static int tunfd = -1;
static int tun_frags_enabled;
#define SYZ_TUN_MAX_PACKET_SIZE 1000

#define TUN_IFACE "syz_tun"

#define LOCAL_MAC 0xaaaaaaaaaaaa
#define REMOTE_MAC 0xaaaaaaaaaabb

#define LOCAL_IPV4 "172.20.20.170"
#define REMOTE_IPV4 "172.20.20.187"

#define LOCAL_IPV6 "fe80::aa"
#define REMOTE_IPV6 "fe80::bb"

#define IFF_NAPI 0x0010
#define IFF_NAPI_FRAGS 0x0020

static void initialize_tun(void)
{
  tunfd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
  if (tunfd == -1) {
    printf("tun: can't open /dev/net/tun: please enable CONFIG_TUN=y\n");
    printf("otherwise fuzzing or reproducing might not work as intended\n");
    return;
  }
  const int kTunFd = 240;
  if (dup2(tunfd, kTunFd) < 0)
    exit(1);
  close(tunfd);
  tunfd = kTunFd;
  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, TUN_IFACE, IFNAMSIZ);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI | IFF_NAPI | IFF_NAPI_FRAGS;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0) {
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0)
      exit(1);
  }
  if (ioctl(tunfd, TUNGETIFF, (void*)&ifr) < 0)
    exit(1);
  tun_frags_enabled = (ifr.ifr_flags & IFF_NAPI_FRAGS) != 0;
  char sysctl[64];
  sprintf(sysctl, "/proc/sys/net/ipv6/conf/%s/accept_dad", TUN_IFACE);
  write_file(sysctl, "0");
  sprintf(sysctl, "/proc/sys/net/ipv6/conf/%s/router_solicitations", TUN_IFACE);
  write_file(sysctl, "0");
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock == -1)
    exit(1);
  netlink_add_addr4(sock, TUN_IFACE, LOCAL_IPV4);
  netlink_add_addr6(sock, TUN_IFACE, LOCAL_IPV6);
  uint64_t macaddr = REMOTE_MAC;
  struct in_addr in_addr;
  inet_pton(AF_INET, REMOTE_IPV4, &in_addr);
  netlink_add_neigh(sock, TUN_IFACE, &in_addr, sizeof(in_addr), &macaddr,
                    ETH_ALEN);
  struct in6_addr in6_addr;
  inet_pton(AF_INET6, REMOTE_IPV6, &in6_addr);
  netlink_add_neigh(sock, TUN_IFACE, &in6_addr, sizeof(in6_addr), &macaddr,
                    ETH_ALEN);
  macaddr = LOCAL_MAC;
  netlink_device_change(sock, TUN_IFACE, true, 0, &macaddr, ETH_ALEN);
  close(sock);
}

#define DEV_IPV4 "172.20.20.%d"
#define DEV_IPV6 "fe80::%02x"
#define DEV_MAC 0x00aaaaaaaaaa
static void initialize_netdevices(void)
{
  char netdevsim[16];
  sprintf(netdevsim, "netdevsim%d", (int)procid);
  struct {
    const char* type;
    const char* dev;
  } devtypes[] = {
      {"ip6gretap", "ip6gretap0"}, {"bridge", "bridge0"},
      {"vcan", "vcan0"},           {"bond", "bond0"},
      {"team", "team0"},           {"dummy", "dummy0"},
      {"nlmon", "nlmon0"},         {"caif", "caif0"},
      {"batadv", "batadv0"},       {"vxcan", "vxcan1"},
      {"netdevsim", netdevsim},    {"veth", 0},
  };
  const char* devmasters[] = {"bridge", "bond", "team"};
  struct {
    const char* name;
    int macsize;
    bool noipv6;
  } devices[] = {
      {"lo", ETH_ALEN},
      {"sit0", 0},
      {"bridge0", ETH_ALEN},
      {"vcan0", 0, true},
      {"tunl0", 0},
      {"gre0", 0},
      {"gretap0", ETH_ALEN},
      {"ip_vti0", 0},
      {"ip6_vti0", 0},
      {"ip6tnl0", 0},
      {"ip6gre0", 0},
      {"ip6gretap0", ETH_ALEN},
      {"erspan0", ETH_ALEN},
      {"bond0", ETH_ALEN},
      {"veth0", ETH_ALEN},
      {"veth1", ETH_ALEN},
      {"team0", ETH_ALEN},
      {"veth0_to_bridge", ETH_ALEN},
      {"veth1_to_bridge", ETH_ALEN},
      {"veth0_to_bond", ETH_ALEN},
      {"veth1_to_bond", ETH_ALEN},
      {"veth0_to_team", ETH_ALEN},
      {"veth1_to_team", ETH_ALEN},
      {"veth0_to_hsr", ETH_ALEN},
      {"veth1_to_hsr", ETH_ALEN},
      {"hsr0", 0},
      {"dummy0", ETH_ALEN},
      {"nlmon0", 0},
      {"vxcan1", 0, true},
      {"caif0", ETH_ALEN},
      {"batadv0", ETH_ALEN},
      {netdevsim, ETH_ALEN},
  };
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock == -1)
    exit(1);
  unsigned i;
  for (i = 0; i < sizeof(devtypes) / sizeof(devtypes[0]); i++)
    netlink_add_device(sock, devtypes[i].type, devtypes[i].dev);
  for (i = 0; i < sizeof(devmasters) / (sizeof(devmasters[0])); i++) {
    char master[32], slave0[32], veth0[32], slave1[32], veth1[32];
    sprintf(slave0, "%s_slave_0", devmasters[i]);
    sprintf(veth0, "veth0_to_%s", devmasters[i]);
    netlink_add_veth(sock, slave0, veth0);
    sprintf(slave1, "%s_slave_1", devmasters[i]);
    sprintf(veth1, "veth1_to_%s", devmasters[i]);
    netlink_add_veth(sock, slave1, veth1);
    sprintf(master, "%s0", devmasters[i]);
    netlink_device_change(sock, slave0, false, master, 0, 0);
    netlink_device_change(sock, slave1, false, master, 0, 0);
  }
  netlink_device_change(sock, "bridge_slave_0", true, 0, 0, 0);
  netlink_device_change(sock, "bridge_slave_1", true, 0, 0, 0);
  netlink_add_veth(sock, "hsr_slave_0", "veth0_to_hsr");
  netlink_add_veth(sock, "hsr_slave_1", "veth1_to_hsr");
  netlink_add_hsr(sock, "hsr0", "hsr_slave_0", "hsr_slave_1");
  netlink_device_change(sock, "hsr_slave_0", true, 0, 0, 0);
  netlink_device_change(sock, "hsr_slave_1", true, 0, 0, 0);
  for (i = 0; i < sizeof(devices) / (sizeof(devices[0])); i++) {
    char addr[32];
    sprintf(addr, DEV_IPV4, i + 10);
    netlink_add_addr4(sock, devices[i].name, addr);
    if (!devices[i].noipv6) {
      sprintf(addr, DEV_IPV6, i + 10);
      netlink_add_addr6(sock, devices[i].name, addr);
    }
    uint64_t macaddr = DEV_MAC + ((i + 10ull) << 40);
    netlink_device_change(sock, devices[i].name, true, 0, &macaddr,
                          devices[i].macsize);
  }
  close(sock);
}
static void initialize_netdevices_init(void)
{
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock == -1)
    exit(1);
  struct {
    const char* type;
    int macsize;
    bool noipv6;
    bool noup;
  } devtypes[] = {
      {"nr", 7, true}, {"rose", 5, true, true},
  };
  unsigned i;
  for (i = 0; i < sizeof(devtypes) / sizeof(devtypes[0]); i++) {
    char dev[32], addr[32];
    sprintf(dev, "%s%d", devtypes[i].type, (int)procid);
    sprintf(addr, "172.30.%d.%d", i, (int)procid + 1);
    netlink_add_addr4(sock, dev, addr);
    if (!devtypes[i].noipv6) {
      sprintf(addr, "fe88::%02x:%02x", i, (int)procid + 1);
      netlink_add_addr6(sock, dev, addr);
    }
    int macsize = devtypes[i].macsize;
    uint64_t macaddr = 0xbbbbbb +
                       ((unsigned long long)i << (8 * (macsize - 2))) +
                       (procid << (8 * (macsize - 1)));
    netlink_device_change(sock, dev, !devtypes[i].noup, 0, &macaddr, macsize);
  }
  close(sock);
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
  setpgrp();
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

int wait_for_loop(int pid)
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
  initialize_netdevices_init();
  if (unshare(CLONE_NEWNET)) {
  }
  initialize_tun();
  initialize_netdevices();
  loop();
  exit(1);
}

uint64_t r[8] = {
    0xffffffffffffffff, 0xffffffffffffffff, 0x0, 0x0,
    0xffffffffffffffff, 0xffffffffffffffff, 0x0, 0xffffffffffffffff};

void loop(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10, 3, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0x11, 3, 0x300);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000000, "batadv0\000\000\000\000\000\000\000\000\000", 16);
  *(uint32_t*)0x20000010 = 0;
  res = syscall(__NR_ioctl, r[1], 0x8933, 0x20000000);
  if (res != -1)
    r[2] = *(uint32_t*)0x20000010;
  *(uint16_t*)0x20000640 = 0x11;
  *(uint16_t*)0x20000642 = htobe16(0);
  *(uint32_t*)0x20000644 = r[2];
  *(uint16_t*)0x20000648 = 1;
  *(uint8_t*)0x2000064a = 0;
  *(uint8_t*)0x2000064b = 6;
  *(uint8_t*)0x2000064c = 1;
  *(uint8_t*)0x2000064d = 0x80;
  *(uint8_t*)0x2000064e = 0xc2;
  *(uint8_t*)0x2000064f = 0;
  *(uint8_t*)0x20000650 = 0;
  *(uint8_t*)0x20000651 = 0;
  *(uint8_t*)0x20000652 = 0;
  *(uint8_t*)0x20000653 = 0;
  syscall(__NR_bind, r[1], 0x20000640, 0x14);
  *(uint32_t*)0x20000140 = 0x14;
  res = syscall(__NR_getsockname, r[1], 0x20000100, 0x20000140);
  if (res != -1)
    r[3] = *(uint32_t*)0x20000104;
  *(uint64_t*)0x20000240 = 0;
  *(uint32_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0x20000080;
  *(uint64_t*)0x20000080 = 0x20001800;
  *(uint32_t*)0x20001800 = 0x45c;
  *(uint16_t*)0x20001804 = 0x24;
  *(uint16_t*)0x20001806 = 0x507;
  *(uint32_t*)0x20001808 = 0;
  *(uint32_t*)0x2000180c = 0;
  *(uint8_t*)0x20001810 = 0;
  *(uint32_t*)0x20001814 = r[3];
  *(uint16_t*)0x20001818 = 0;
  *(uint16_t*)0x2000181a = 0;
  *(uint16_t*)0x2000181c = -1;
  *(uint16_t*)0x2000181e = -1;
  *(uint16_t*)0x20001820 = 0;
  *(uint16_t*)0x20001822 = 0;
  *(uint16_t*)0x20001824 = 8;
  *(uint16_t*)0x20001826 = 1;
  memcpy((void*)0x20001828, "cbq\000", 4);
  *(uint16_t*)0x2000182c = 0x430;
  *(uint16_t*)0x2000182e = 2;
  *(uint16_t*)0x20001830 = 0x404;
  *(uint16_t*)0x20001832 = 6;
  *(uint32_t*)0x20001834 = 0;
  *(uint32_t*)0x20001838 = 0;
  *(uint32_t*)0x2000183c = 0;
  *(uint32_t*)0x20001840 = 0;
  *(uint32_t*)0x20001844 = 0;
  *(uint32_t*)0x20001848 = 0;
  *(uint32_t*)0x2000184c = 0;
  *(uint32_t*)0x20001850 = 0;
  *(uint32_t*)0x20001854 = 0;
  *(uint32_t*)0x20001858 = 0;
  *(uint32_t*)0x2000185c = 0;
  *(uint32_t*)0x20001860 = 0;
  *(uint32_t*)0x20001864 = 0;
  *(uint32_t*)0x20001868 = 0;
  *(uint32_t*)0x2000186c = 0;
  *(uint32_t*)0x20001870 = 0;
  *(uint32_t*)0x20001874 = 0;
  *(uint32_t*)0x20001878 = 0;
  *(uint32_t*)0x2000187c = 0;
  *(uint32_t*)0x20001880 = 0;
  *(uint32_t*)0x20001884 = 0;
  *(uint32_t*)0x20001888 = 0;
  *(uint32_t*)0x2000188c = 0;
  *(uint32_t*)0x20001890 = 0;
  *(uint32_t*)0x20001894 = 0;
  *(uint32_t*)0x20001898 = 0;
  *(uint32_t*)0x2000189c = 0;
  *(uint32_t*)0x200018a0 = 0;
  *(uint32_t*)0x200018a4 = 0;
  *(uint32_t*)0x200018a8 = 0;
  *(uint32_t*)0x200018ac = 0;
  *(uint32_t*)0x200018b0 = 0;
  *(uint32_t*)0x200018b4 = 0;
  *(uint32_t*)0x200018b8 = 0;
  *(uint32_t*)0x200018bc = 0;
  *(uint32_t*)0x200018c0 = 0;
  *(uint32_t*)0x200018c4 = 0;
  *(uint32_t*)0x200018c8 = 0;
  *(uint32_t*)0x200018cc = 0;
  *(uint32_t*)0x200018d0 = 0;
  *(uint32_t*)0x200018d4 = 0;
  *(uint32_t*)0x200018d8 = 0;
  *(uint32_t*)0x200018dc = 0;
  *(uint32_t*)0x200018e0 = 0;
  *(uint32_t*)0x200018e4 = 0;
  *(uint32_t*)0x200018e8 = 0;
  *(uint32_t*)0x200018ec = 0;
  *(uint32_t*)0x200018f0 = 0;
  *(uint32_t*)0x200018f4 = 0;
  *(uint32_t*)0x200018f8 = 0;
  *(uint32_t*)0x200018fc = 0;
  *(uint32_t*)0x20001900 = 0;
  *(uint32_t*)0x20001904 = 0;
  *(uint32_t*)0x20001908 = 0;
  *(uint32_t*)0x2000190c = 0;
  *(uint32_t*)0x20001910 = 0;
  *(uint32_t*)0x20001914 = 0;
  *(uint32_t*)0x20001918 = 0;
  *(uint32_t*)0x2000191c = 0;
  *(uint32_t*)0x20001920 = 0;
  *(uint32_t*)0x20001924 = 0;
  *(uint32_t*)0x20001928 = 0;
  *(uint32_t*)0x2000192c = 0;
  *(uint32_t*)0x20001930 = 0;
  *(uint32_t*)0x20001934 = 0;
  *(uint32_t*)0x20001938 = 0;
  *(uint32_t*)0x2000193c = 0;
  *(uint32_t*)0x20001940 = 0;
  *(uint32_t*)0x20001944 = 0;
  *(uint32_t*)0x20001948 = 0;
  *(uint32_t*)0x2000194c = 0;
  *(uint32_t*)0x20001950 = 0;
  *(uint32_t*)0x20001954 = 0;
  *(uint32_t*)0x20001958 = 0;
  *(uint32_t*)0x2000195c = 0;
  *(uint32_t*)0x20001960 = 0;
  *(uint32_t*)0x20001964 = 0;
  *(uint32_t*)0x20001968 = 0;
  *(uint32_t*)0x2000196c = 0;
  *(uint32_t*)0x20001970 = 0;
  *(uint32_t*)0x20001974 = 0;
  *(uint32_t*)0x20001978 = 0;
  *(uint32_t*)0x2000197c = 0;
  *(uint32_t*)0x20001980 = 0;
  *(uint32_t*)0x20001984 = 0;
  *(uint32_t*)0x20001988 = 0;
  *(uint32_t*)0x2000198c = 0;
  *(uint32_t*)0x20001990 = 0;
  *(uint32_t*)0x20001994 = 0;
  *(uint32_t*)0x20001998 = 0;
  *(uint32_t*)0x2000199c = 0;
  *(uint32_t*)0x200019a0 = 0;
  *(uint32_t*)0x200019a4 = 0;
  *(uint32_t*)0x200019a8 = 0;
  *(uint32_t*)0x200019ac = 0;
  *(uint32_t*)0x200019b0 = 0;
  *(uint32_t*)0x200019b4 = 0;
  *(uint32_t*)0x200019b8 = 0;
  *(uint32_t*)0x200019bc = 0;
  *(uint32_t*)0x200019c0 = 0;
  *(uint32_t*)0x200019c4 = 0;
  *(uint32_t*)0x200019c8 = 0;
  *(uint32_t*)0x200019cc = 0;
  *(uint32_t*)0x200019d0 = 0;
  *(uint32_t*)0x200019d4 = 0;
  *(uint32_t*)0x200019d8 = 0;
  *(uint32_t*)0x200019dc = 0;
  *(uint32_t*)0x200019e0 = 0;
  *(uint32_t*)0x200019e4 = 0;
  *(uint32_t*)0x200019e8 = 0;
  *(uint32_t*)0x200019ec = 0;
  *(uint32_t*)0x200019f0 = 0;
  *(uint32_t*)0x200019f4 = 0;
  *(uint32_t*)0x200019f8 = 0;
  *(uint32_t*)0x200019fc = 0;
  *(uint32_t*)0x20001a00 = 0;
  *(uint32_t*)0x20001a04 = 0;
  *(uint32_t*)0x20001a08 = 0;
  *(uint32_t*)0x20001a0c = 0;
  *(uint32_t*)0x20001a10 = 0;
  *(uint32_t*)0x20001a14 = 0;
  *(uint32_t*)0x20001a18 = 0;
  *(uint32_t*)0x20001a1c = 0;
  *(uint32_t*)0x20001a20 = 0;
  *(uint32_t*)0x20001a24 = 0;
  *(uint32_t*)0x20001a28 = 0;
  *(uint32_t*)0x20001a2c = 0;
  *(uint32_t*)0x20001a30 = 0;
  *(uint32_t*)0x20001a34 = 0;
  *(uint32_t*)0x20001a38 = 0;
  *(uint32_t*)0x20001a3c = 0;
  *(uint32_t*)0x20001a40 = 0;
  *(uint32_t*)0x20001a44 = 0;
  *(uint32_t*)0x20001a48 = 0;
  *(uint32_t*)0x20001a4c = 0;
  *(uint32_t*)0x20001a50 = 0;
  *(uint32_t*)0x20001a54 = 0;
  *(uint32_t*)0x20001a58 = 0;
  *(uint32_t*)0x20001a5c = 0;
  *(uint32_t*)0x20001a60 = 0;
  *(uint32_t*)0x20001a64 = 0;
  *(uint32_t*)0x20001a68 = 0;
  *(uint32_t*)0x20001a6c = 0;
  *(uint32_t*)0x20001a70 = 0;
  *(uint32_t*)0x20001a74 = 0;
  *(uint32_t*)0x20001a78 = 0;
  *(uint32_t*)0x20001a7c = 0;
  *(uint32_t*)0x20001a80 = 0;
  *(uint32_t*)0x20001a84 = 0;
  *(uint32_t*)0x20001a88 = 0;
  *(uint32_t*)0x20001a8c = 0;
  *(uint32_t*)0x20001a90 = 0;
  *(uint32_t*)0x20001a94 = 0;
  *(uint32_t*)0x20001a98 = 0;
  *(uint32_t*)0x20001a9c = 0;
  *(uint32_t*)0x20001aa0 = 0;
  *(uint32_t*)0x20001aa4 = 0;
  *(uint32_t*)0x20001aa8 = 0;
  *(uint32_t*)0x20001aac = 0;
  *(uint32_t*)0x20001ab0 = 0;
  *(uint32_t*)0x20001ab4 = 0;
  *(uint32_t*)0x20001ab8 = 0;
  *(uint32_t*)0x20001abc = 0;
  *(uint32_t*)0x20001ac0 = 0;
  *(uint32_t*)0x20001ac4 = 0;
  *(uint32_t*)0x20001ac8 = 0;
  *(uint32_t*)0x20001acc = 0;
  *(uint32_t*)0x20001ad0 = 0;
  *(uint32_t*)0x20001ad4 = 0;
  *(uint32_t*)0x20001ad8 = 0;
  *(uint32_t*)0x20001adc = 0;
  *(uint32_t*)0x20001ae0 = 0;
  *(uint32_t*)0x20001ae4 = 0;
  *(uint32_t*)0x20001ae8 = 0;
  *(uint32_t*)0x20001aec = 0;
  *(uint32_t*)0x20001af0 = 0;
  *(uint32_t*)0x20001af4 = 0;
  *(uint32_t*)0x20001af8 = 0;
  *(uint32_t*)0x20001afc = 0;
  *(uint32_t*)0x20001b00 = 0;
  *(uint32_t*)0x20001b04 = 0;
  *(uint32_t*)0x20001b08 = 0;
  *(uint32_t*)0x20001b0c = 0;
  *(uint32_t*)0x20001b10 = 0;
  *(uint32_t*)0x20001b14 = 0;
  *(uint32_t*)0x20001b18 = 0;
  *(uint32_t*)0x20001b1c = 0;
  *(uint32_t*)0x20001b20 = 0;
  *(uint32_t*)0x20001b24 = 0;
  *(uint32_t*)0x20001b28 = 0;
  *(uint32_t*)0x20001b2c = 0;
  *(uint32_t*)0x20001b30 = 0;
  *(uint32_t*)0x20001b34 = 0;
  *(uint32_t*)0x20001b38 = 0;
  *(uint32_t*)0x20001b3c = 0;
  *(uint32_t*)0x20001b40 = 0;
  *(uint32_t*)0x20001b44 = 0;
  *(uint32_t*)0x20001b48 = 0;
  *(uint32_t*)0x20001b4c = 0;
  *(uint32_t*)0x20001b50 = 0;
  *(uint32_t*)0x20001b54 = 0;
  *(uint32_t*)0x20001b58 = 0;
  *(uint32_t*)0x20001b5c = 0;
  *(uint32_t*)0x20001b60 = 0;
  *(uint32_t*)0x20001b64 = 0;
  *(uint32_t*)0x20001b68 = 0;
  *(uint32_t*)0x20001b6c = 0;
  *(uint32_t*)0x20001b70 = 0;
  *(uint32_t*)0x20001b74 = 0;
  *(uint32_t*)0x20001b78 = 0;
  *(uint32_t*)0x20001b7c = 0;
  *(uint32_t*)0x20001b80 = 0;
  *(uint32_t*)0x20001b84 = 0;
  *(uint32_t*)0x20001b88 = 0;
  *(uint32_t*)0x20001b8c = 0;
  *(uint32_t*)0x20001b90 = 0;
  *(uint32_t*)0x20001b94 = 0;
  *(uint32_t*)0x20001b98 = 0;
  *(uint32_t*)0x20001b9c = 0;
  *(uint32_t*)0x20001ba0 = 0;
  *(uint32_t*)0x20001ba4 = 0;
  *(uint32_t*)0x20001ba8 = 0;
  *(uint32_t*)0x20001bac = 0;
  *(uint32_t*)0x20001bb0 = 0;
  *(uint32_t*)0x20001bb4 = 0;
  *(uint32_t*)0x20001bb8 = 0;
  *(uint32_t*)0x20001bbc = 0;
  *(uint32_t*)0x20001bc0 = 0;
  *(uint32_t*)0x20001bc4 = 0;
  *(uint32_t*)0x20001bc8 = 0;
  *(uint32_t*)0x20001bcc = 0;
  *(uint32_t*)0x20001bd0 = 0;
  *(uint32_t*)0x20001bd4 = 0;
  *(uint32_t*)0x20001bd8 = 0;
  *(uint32_t*)0x20001bdc = 0;
  *(uint32_t*)0x20001be0 = 0;
  *(uint32_t*)0x20001be4 = 0;
  *(uint32_t*)0x20001be8 = 0;
  *(uint32_t*)0x20001bec = 0;
  *(uint32_t*)0x20001bf0 = 0;
  *(uint32_t*)0x20001bf4 = 0;
  *(uint32_t*)0x20001bf8 = 0;
  *(uint32_t*)0x20001bfc = 0;
  *(uint32_t*)0x20001c00 = 0;
  *(uint32_t*)0x20001c04 = 0;
  *(uint32_t*)0x20001c08 = 0;
  *(uint32_t*)0x20001c0c = 0;
  *(uint32_t*)0x20001c10 = 0;
  *(uint32_t*)0x20001c14 = 0;
  *(uint32_t*)0x20001c18 = 0;
  *(uint32_t*)0x20001c1c = 0;
  *(uint32_t*)0x20001c20 = 0;
  *(uint32_t*)0x20001c24 = 0;
  *(uint32_t*)0x20001c28 = 0;
  *(uint32_t*)0x20001c2c = 0;
  *(uint32_t*)0x20001c30 = 0;
  *(uint16_t*)0x20001c34 = 0x10;
  *(uint16_t*)0x20001c36 = 5;
  *(uint8_t*)0x20001c38 = 5;
  *(uint8_t*)0x20001c39 = 0;
  *(uint16_t*)0x20001c3a = 0;
  *(uint16_t*)0x20001c3c = 0;
  *(uint16_t*)0x20001c3e = 0;
  *(uint32_t*)0x20001c40 = 0x10000;
  *(uint16_t*)0x20001c44 = 0x18;
  *(uint16_t*)0x20001c46 = 1;
  *(uint8_t*)0x20001c48 = 0;
  *(uint8_t*)0x20001c49 = 0;
  *(uint8_t*)0x20001c4a = 0;
  *(uint8_t*)0x20001c4b = 0;
  *(uint32_t*)0x20001c4c = 0;
  *(uint32_t*)0x20001c50 = 0;
  *(uint32_t*)0x20001c54 = 0;
  *(uint32_t*)0x20001c58 = 0;
  *(uint64_t*)0x20000088 = 0x45c;
  *(uint64_t*)0x20000258 = 1;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint32_t*)0x20000270 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000240, 0);
  res = syscall(__NR_socket, 0x10, 0x80002, 0);
  if (res != -1)
    r[4] = res;
  res = syscall(__NR_socket, 0x11, 2, 0x300);
  if (res != -1)
    r[5] = res;
  memcpy((void*)0x20000000, "batadv0\000\000\000\000\000\000\000\000\000", 16);
  *(uint32_t*)0x20000010 = 0;
  res = syscall(__NR_ioctl, r[5], 0x8933, 0x20000000);
  if (res != -1)
    r[6] = *(uint32_t*)0x20000010;
  *(uint16_t*)0x20000640 = 0x11;
  *(uint16_t*)0x20000642 = htobe16(0);
  *(uint32_t*)0x20000644 = r[6];
  *(uint16_t*)0x20000648 = 1;
  *(uint8_t*)0x2000064a = 0;
  *(uint8_t*)0x2000064b = 6;
  *(uint8_t*)0x2000064c = 1;
  *(uint8_t*)0x2000064d = 0x80;
  *(uint8_t*)0x2000064e = 0xc2;
  *(uint8_t*)0x2000064f = 0;
  *(uint8_t*)0x20000650 = 0;
  *(uint8_t*)0x20000651 = 0;
  *(uint8_t*)0x20000652 = 0;
  *(uint8_t*)0x20000653 = 0;
  syscall(__NR_bind, r[5], 0x20000640, 0x14);
  *(uint32_t*)0x20000140 = 0x14;
  res = syscall(__NR_getsockname, r[5], 0x20000100, 0x20000140);
  if (res != -1)
    r[7] = *(uint32_t*)0x20000104;
  *(uint64_t*)0x20000240 = 0;
  *(uint32_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0x200000c0;
  *(uint64_t*)0x200000c0 = 0x20000280;
  memcpy((void*)0x20000280, "\x48\x00\x00\x00\x28\x00\x07\x05\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x02\x00\x00\x00",
         20);
  *(uint32_t*)0x20000294 = r[7];
  memcpy((void*)0x20000298,
         "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\x08\x00\x01\x00\x68"
         "\x74\x62\x00\x1c\x00\x02\x00\x18\x00\x02\x00\xb4\x85\xd4\x0c\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x64\x5a\x38"
         "\xd4\x71\x20\x4e\xaa\xbb\x54\xf7\x3b\x1f\xfe\x22\xa3\xb6\xf1\xc4\x75"
         "\x7e\xe7\x55\x26\xb6\xd2\xc3\xff\xbb\x06\x32\x59\x9c\x72\xfc\xf9\x4e"
         "\x5e\xbc\x20\xf8\xf3\xc0\x5c\xfb\xc8\x1f\x0a\x6a\x15\xa8\x1a\x3c\x91"
         "\x20\x34\x13\xfb\x9c\xf2\x85\xbb\x46\x63\xea\x0c\x9e\xb6\x7b\x45\x08"
         "\x39\x26\xf8\xff\xe6\x61\x70\x54\x19\x7f\x0a\xa5\x9a\x65\x39\x89\xff"
         "\xeb\xe7\x1a\xab\x0b\xa4\x07\x57\x5f\x4b\xf0\xf9\xf0\x0b\x8f\xaa\x86"
         "\x73\xb1\x28\xc1\x6d\xe0\x3d\x09\xa1\x46\x90\xcd\xee\x11\x4e\x57\xc5"
         "\x75\xf1\xac\x6b\x1d\xce\x0a\xe5\xb2\xba\xa8\xbe\xb7\x03\x73\x70\xcb"
         "\xd7\xf3\xc0\xdb\x8e\x41\x42\x05\xda\xab\x4b\xb3\xbc\x6a\x83\x4f\xb5"
         "\x5b\xfe\x98\x6c\x39\xef\x4b\x0e\xce\x4d\xe1\x16\xcd\x45\x68\xab\x18"
         "\x7b\xc7\x22\x7c\x89\xd1\xf3\x00\x59\x98\x97\x49\xed\xfa\x62\x73\xb0"
         "\x8c\x00\x00\x00\x00",
         243);
  *(uint64_t*)0x200000c8 = 0x48;
  *(uint64_t*)0x20000258 = 1;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint32_t*)0x20000270 = 0;
  syscall(__NR_sendmsg, -1, 0x20000240, 0);
  syscall(__NR_sendmmsg, r[4], 0x20000140, 0x332, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  do_sandbox_none();
  return 0;
}