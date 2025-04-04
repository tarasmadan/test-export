// https://syzkaller.appspot.com/bug?id=c733313531000b174899fbfc2e59e8b1c6042549
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <pthread.h>
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
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/capability.h>
#include <linux/futex.h>
#include <linux/genetlink.h>
#include <linux/if_addr.h>
#include <linux/if_ether.h>
#include <linux/if_link.h>
#include <linux/in6.h>
#include <linux/neighbour.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/nl80211.h>
#include <linux/rfkill.h>
#include <linux/rtnetlink.h>
#include <linux/veth.h>

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

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  int i = 0;
  for (; i < 100; i++) {
    if (pthread_create(&th, &attr, fn, arg) == 0) {
      pthread_attr_destroy(&attr);
      return;
    }
    if (errno == EAGAIN) {
      usleep(50);
      continue;
    }
    break;
  }
  exit(1);
}

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

typedef struct {
  int state;
} event_t;

static void event_init(event_t* ev)
{
  ev->state = 0;
}

static void event_reset(event_t* ev)
{
  ev->state = 0;
}

static void event_set(event_t* ev)
{
  if (ev->state)
    exit(1);
  __atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1000000);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, 0);
}

static int event_isset(event_t* ev)
{
  return __atomic_load_n(&ev->state, __ATOMIC_ACQUIRE);
}

static int event_timedwait(event_t* ev, uint64_t timeout)
{
  uint64_t start = current_time_ms();
  uint64_t now = start;
  for (;;) {
    uint64_t remain = timeout - (now - start);
    struct timespec ts;
    ts.tv_sec = remain / 1000;
    ts.tv_nsec = (remain % 1000) * 1000 * 1000;
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, &ts);
    if (__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
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

struct nlmsg {
  char* pos;
  int nesting;
  struct nlattr* nested[8];
  char buf[4096];
};

static void netlink_init(struct nlmsg* nlmsg, int typ, int flags,
                         const void* data, int size)
{
  memset(nlmsg, 0, sizeof(*nlmsg));
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg->buf;
  hdr->nlmsg_type = typ;
  hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | flags;
  memcpy(hdr + 1, data, size);
  nlmsg->pos = (char*)(hdr + 1) + NLMSG_ALIGN(size);
}

static void netlink_attr(struct nlmsg* nlmsg, int typ, const void* data,
                         int size)
{
  struct nlattr* attr = (struct nlattr*)nlmsg->pos;
  attr->nla_len = sizeof(*attr) + size;
  attr->nla_type = typ;
  if (size > 0)
    memcpy(attr + 1, data, size);
  nlmsg->pos += NLMSG_ALIGN(attr->nla_len);
}

static int netlink_send_ext(struct nlmsg* nlmsg, int sock, uint16_t reply_type,
                            int* reply_len, bool dofail)
{
  if (nlmsg->pos > nlmsg->buf + sizeof(nlmsg->buf) || nlmsg->nesting)
    exit(1);
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg->buf;
  hdr->nlmsg_len = nlmsg->pos - nlmsg->buf;
  struct sockaddr_nl addr;
  memset(&addr, 0, sizeof(addr));
  addr.nl_family = AF_NETLINK;
  ssize_t n = sendto(sock, nlmsg->buf, hdr->nlmsg_len, 0,
                     (struct sockaddr*)&addr, sizeof(addr));
  if (n != (ssize_t)hdr->nlmsg_len) {
    if (dofail)
      exit(1);
    return -1;
  }
  n = recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0);
  if (reply_len)
    *reply_len = 0;
  if (n < 0) {
    if (dofail)
      exit(1);
    return -1;
  }
  if (n < (ssize_t)sizeof(struct nlmsghdr)) {
    errno = EINVAL;
    if (dofail)
      exit(1);
    return -1;
  }
  if (hdr->nlmsg_type == NLMSG_DONE)
    return 0;
  if (reply_len && hdr->nlmsg_type == reply_type) {
    *reply_len = n;
    return 0;
  }
  if (n < (ssize_t)(sizeof(struct nlmsghdr) + sizeof(struct nlmsgerr))) {
    errno = EINVAL;
    if (dofail)
      exit(1);
    return -1;
  }
  if (hdr->nlmsg_type != NLMSG_ERROR) {
    errno = EINVAL;
    if (dofail)
      exit(1);
    return -1;
  }
  errno = -((struct nlmsgerr*)(hdr + 1))->error;
  return -errno;
}

static int netlink_send(struct nlmsg* nlmsg, int sock)
{
  return netlink_send_ext(nlmsg, sock, 0, NULL, true);
}

static int netlink_query_family_id(struct nlmsg* nlmsg, int sock,
                                   const char* family_name, bool dofail)
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = CTRL_CMD_GETFAMILY;
  netlink_init(nlmsg, GENL_ID_CTRL, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, CTRL_ATTR_FAMILY_NAME, family_name,
               strnlen(family_name, GENL_NAMSIZ - 1) + 1);
  int n = 0;
  int err = netlink_send_ext(nlmsg, sock, GENL_ID_CTRL, &n, dofail);
  if (err < 0) {
    return -1;
  }
  uint16_t id = 0;
  struct nlattr* attr = (struct nlattr*)(nlmsg->buf + NLMSG_HDRLEN +
                                         NLMSG_ALIGN(sizeof(genlhdr)));
  for (; (char*)attr < nlmsg->buf + n;
       attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
    if (attr->nla_type == CTRL_ATTR_FAMILY_ID) {
      id = *(uint16_t*)(attr + 1);
      break;
    }
  }
  if (!id) {
    errno = EINVAL;
    return -1;
  }
  recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0);
  return id;
}

static struct nlmsg nlmsg;

#define WIFI_INITIAL_DEVICE_COUNT 2
#define WIFI_MAC_BASE                                                          \
  {                                                                            \
    0x08, 0x02, 0x11, 0x00, 0x00, 0x00                                         \
  }
#define WIFI_IBSS_BSSID                                                        \
  {                                                                            \
    0x50, 0x50, 0x50, 0x50, 0x50, 0x50                                         \
  }
#define WIFI_IBSS_SSID                                                         \
  {                                                                            \
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10                                         \
  }
#define WIFI_DEFAULT_FREQUENCY 2412
#define WIFI_DEFAULT_SIGNAL 0
#define WIFI_DEFAULT_RX_RATE 1
#define HWSIM_CMD_REGISTER 1
#define HWSIM_CMD_FRAME 2
#define HWSIM_CMD_NEW_RADIO 4
#define HWSIM_ATTR_SUPPORT_P2P_DEVICE 14
#define HWSIM_ATTR_PERM_ADDR 22

#define IF_OPER_UP 6
struct join_ibss_props {
  int wiphy_freq;
  bool wiphy_freq_fixed;
  uint8_t* mac;
  uint8_t* ssid;
  int ssid_len;
};

static int set_interface_state(const char* interface_name, int on)
{
  struct ifreq ifr;
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    return -1;
  }
  memset(&ifr, 0, sizeof(ifr));
  strcpy(ifr.ifr_name, interface_name);
  int ret = ioctl(sock, SIOCGIFFLAGS, &ifr);
  if (ret < 0) {
    close(sock);
    return -1;
  }
  if (on)
    ifr.ifr_flags |= IFF_UP;
  else
    ifr.ifr_flags &= ~IFF_UP;
  ret = ioctl(sock, SIOCSIFFLAGS, &ifr);
  close(sock);
  if (ret < 0) {
    return -1;
  }
  return 0;
}

static int nl80211_set_interface(struct nlmsg* nlmsg, int sock,
                                 int nl80211_family, uint32_t ifindex,
                                 uint32_t iftype, bool dofail)
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = NL80211_CMD_SET_INTERFACE;
  netlink_init(nlmsg, nl80211_family, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, NL80211_ATTR_IFINDEX, &ifindex, sizeof(ifindex));
  netlink_attr(nlmsg, NL80211_ATTR_IFTYPE, &iftype, sizeof(iftype));
  int err = netlink_send_ext(nlmsg, sock, 0, NULL, dofail);
  if (err < 0) {
  }
  return err;
}

static int nl80211_join_ibss(struct nlmsg* nlmsg, int sock, int nl80211_family,
                             uint32_t ifindex, struct join_ibss_props* props,
                             bool dofail)
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = NL80211_CMD_JOIN_IBSS;
  netlink_init(nlmsg, nl80211_family, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, NL80211_ATTR_IFINDEX, &ifindex, sizeof(ifindex));
  netlink_attr(nlmsg, NL80211_ATTR_SSID, props->ssid, props->ssid_len);
  netlink_attr(nlmsg, NL80211_ATTR_WIPHY_FREQ, &(props->wiphy_freq),
               sizeof(props->wiphy_freq));
  if (props->mac)
    netlink_attr(nlmsg, NL80211_ATTR_MAC, props->mac, ETH_ALEN);
  if (props->wiphy_freq_fixed)
    netlink_attr(nlmsg, NL80211_ATTR_FREQ_FIXED, NULL, 0);
  int err = netlink_send_ext(nlmsg, sock, 0, NULL, dofail);
  if (err < 0) {
  }
  return err;
}

static int get_ifla_operstate(struct nlmsg* nlmsg, int ifindex, bool dofail)
{
  struct ifinfomsg info;
  memset(&info, 0, sizeof(info));
  info.ifi_family = AF_UNSPEC;
  info.ifi_index = ifindex;
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock == -1) {
    return -1;
  }
  netlink_init(nlmsg, RTM_GETLINK, 0, &info, sizeof(info));
  int n;
  int err = netlink_send_ext(nlmsg, sock, RTM_NEWLINK, &n, dofail);
  close(sock);
  if (err) {
    return -1;
  }
  struct rtattr* attr = IFLA_RTA(NLMSG_DATA(nlmsg->buf));
  for (; RTA_OK(attr, n); attr = RTA_NEXT(attr, n)) {
    if (attr->rta_type == IFLA_OPERSTATE)
      return *((int32_t*)RTA_DATA(attr));
  }
  return -1;
}

static int await_ifla_operstate(struct nlmsg* nlmsg, char* interface,
                                int operstate, bool dofail)
{
  int ifindex = if_nametoindex(interface);
  while (true) {
    usleep(1000);
    int ret = get_ifla_operstate(nlmsg, ifindex, dofail);
    if (ret < 0)
      return ret;
    if (ret == operstate)
      return 0;
  }
  return 0;
}

static int nl80211_setup_ibss_interface(struct nlmsg* nlmsg, int sock,
                                        int nl80211_family_id, char* interface,
                                        struct join_ibss_props* ibss_props,
                                        bool dofail)
{
  int ifindex = if_nametoindex(interface);
  if (ifindex == 0) {
    return -1;
  }
  int ret = nl80211_set_interface(nlmsg, sock, nl80211_family_id, ifindex,
                                  NL80211_IFTYPE_ADHOC, dofail);
  if (ret < 0) {
    return -1;
  }
  ret = set_interface_state(interface, 1);
  if (ret < 0) {
    return -1;
  }
  ret = nl80211_join_ibss(nlmsg, sock, nl80211_family_id, ifindex, ibss_props,
                          dofail);
  if (ret < 0) {
    return -1;
  }
  return 0;
}

static int hwsim80211_create_device(struct nlmsg* nlmsg, int sock,
                                    int hwsim_family,
                                    uint8_t mac_addr[ETH_ALEN])
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = HWSIM_CMD_NEW_RADIO;
  netlink_init(nlmsg, hwsim_family, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, HWSIM_ATTR_SUPPORT_P2P_DEVICE, NULL, 0);
  netlink_attr(nlmsg, HWSIM_ATTR_PERM_ADDR, mac_addr, ETH_ALEN);
  int err = netlink_send(nlmsg, sock);
  if (err < 0) {
  }
  return err;
}

static void initialize_wifi_devices(void)
{
  int rfkill = open("/dev/rfkill", O_RDWR);
  if (rfkill == -1)
    exit(1);
  struct rfkill_event event = {0};
  event.type = RFKILL_TYPE_ALL;
  event.op = RFKILL_OP_CHANGE_ALL;
  if (write(rfkill, &event, sizeof(event)) != (ssize_t)(sizeof(event)))
    exit(1);
  close(rfkill);
  uint8_t mac_addr[6] = WIFI_MAC_BASE;
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock < 0)
    exit(1);
  int hwsim_family_id =
      netlink_query_family_id(&nlmsg, sock, "MAC80211_HWSIM", true);
  int nl80211_family_id =
      netlink_query_family_id(&nlmsg, sock, "nl80211", true);
  if (hwsim_family_id < 0 || nl80211_family_id < 0)
    exit(1);
  uint8_t ssid[] = WIFI_IBSS_SSID;
  uint8_t bssid[] = WIFI_IBSS_BSSID;
  struct join_ibss_props ibss_props = {.wiphy_freq = WIFI_DEFAULT_FREQUENCY,
                                       .wiphy_freq_fixed = true,
                                       .mac = bssid,
                                       .ssid = ssid,
                                       .ssid_len = sizeof(ssid)};
  for (int device_id = 0; device_id < WIFI_INITIAL_DEVICE_COUNT; device_id++) {
    mac_addr[5] = device_id;
    int ret = hwsim80211_create_device(&nlmsg, sock, hwsim_family_id, mac_addr);
    if (ret < 0)
      exit(1);
    char interface[6] = "wlan0";
    interface[4] += device_id;
    if (nl80211_setup_ibss_interface(&nlmsg, sock, nl80211_family_id, interface,
                                     &ibss_props, true) < 0)
      exit(1);
  }
  for (int device_id = 0; device_id < WIFI_INITIAL_DEVICE_COUNT; device_id++) {
    char interface[6] = "wlan0";
    interface[4] += device_id;
    int ret = await_ifla_operstate(&nlmsg, interface, IF_OPER_UP, true);
    if (ret < 0)
      exit(1);
  }
  close(sock);
}

static long syz_genetlink_get_family_id(volatile long name,
                                        volatile long sock_arg)
{
  int fd = sock_arg;
  if (fd < 0) {
    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (fd == -1) {
      return -1;
    }
  }
  struct nlmsg nlmsg_tmp;
  int ret = netlink_query_family_id(&nlmsg_tmp, fd, (char*)name, false);
  if ((int)sock_arg < 0)
    close(fd);
  if (ret < 0) {
    return -1;
  }
  return ret;
}

static void setup_gadgetfs();
static void setup_binderfs();
static void setup_fusectl();
static void sandbox_common_mount_tmpfs(void)
{
  write_file("/proc/sys/fs/mount-max", "100000");
  if (mkdir("./syz-tmp", 0777))
    exit(1);
  if (mount("", "./syz-tmp", "tmpfs", 0, NULL))
    exit(1);
  if (mkdir("./syz-tmp/newroot", 0777))
    exit(1);
  if (mkdir("./syz-tmp/newroot/dev", 0700))
    exit(1);
  unsigned bind_mount_flags = MS_BIND | MS_REC | MS_PRIVATE;
  if (mount("/dev", "./syz-tmp/newroot/dev", NULL, bind_mount_flags, NULL))
    exit(1);
  if (mkdir("./syz-tmp/newroot/proc", 0700))
    exit(1);
  if (mount("syz-proc", "./syz-tmp/newroot/proc", "proc", 0, NULL))
    exit(1);
  if (mkdir("./syz-tmp/newroot/selinux", 0700))
    exit(1);
  const char* selinux_path = "./syz-tmp/newroot/selinux";
  if (mount("/selinux", selinux_path, NULL, bind_mount_flags, NULL)) {
    if (errno != ENOENT)
      exit(1);
    if (mount("/sys/fs/selinux", selinux_path, NULL, bind_mount_flags, NULL) &&
        errno != ENOENT)
      exit(1);
  }
  if (mkdir("./syz-tmp/newroot/sys", 0700))
    exit(1);
  if (mount("/sys", "./syz-tmp/newroot/sys", 0, bind_mount_flags, NULL))
    exit(1);
  if (mount("/sys/kernel/debug", "./syz-tmp/newroot/sys/kernel/debug", NULL,
            bind_mount_flags, NULL) &&
      errno != ENOENT)
    exit(1);
  if (mount("/sys/fs/smackfs", "./syz-tmp/newroot/sys/fs/smackfs", NULL,
            bind_mount_flags, NULL) &&
      errno != ENOENT)
    exit(1);
  if (mount("/proc/sys/fs/binfmt_misc",
            "./syz-tmp/newroot/proc/sys/fs/binfmt_misc", NULL, bind_mount_flags,
            NULL) &&
      errno != ENOENT)
    exit(1);
  if (mkdir("./syz-tmp/newroot/syz-inputs", 0700))
    exit(1);
  if (mount("/syz-inputs", "./syz-tmp/newroot/syz-inputs", NULL,
            bind_mount_flags | MS_RDONLY, NULL) &&
      errno != ENOENT)
    exit(1);
  if (mkdir("./syz-tmp/pivot", 0777))
    exit(1);
  if (syscall(SYS_pivot_root, "./syz-tmp", "./syz-tmp/pivot")) {
    if (chdir("./syz-tmp"))
      exit(1);
  } else {
    if (chdir("/"))
      exit(1);
    if (umount2("./pivot", MNT_DETACH))
      exit(1);
  }
  if (chroot("./newroot"))
    exit(1);
  if (chdir("/"))
    exit(1);
  setup_gadgetfs();
  setup_binderfs();
  setup_fusectl();
}

static void setup_gadgetfs()
{
  if (mkdir("/dev/gadgetfs", 0777)) {
  }
  if (mount("gadgetfs", "/dev/gadgetfs", "gadgetfs", 0, NULL)) {
  }
}

static void setup_fusectl()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
}

static void setup_binderfs()
{
  if (mkdir("/dev/binderfs", 0777)) {
  }
  if (mount("binder", "/dev/binderfs", "binder", 0, NULL)) {
  }
  if (symlink("/dev/binderfs", "./binderfs")) {
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  if (getppid() == 1)
    exit(1);
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = (200 << 20);
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 32 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 128 << 20;
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
  sandbox_common();
  drop_caps();
  if (unshare(CLONE_NEWNET)) {
  }
  write_file("/proc/sys/net/ipv4/ping_group_range", "0 65535");
  initialize_wifi_devices();
  sandbox_common_mount_tmpfs();
  loop();
  exit(1);
}

#define HWSIM_ATTR_RX_RATE 5
#define HWSIM_ATTR_SIGNAL 6
#define HWSIM_ATTR_ADDR_RECEIVER 1
#define HWSIM_ATTR_FRAME 3

#define WIFI_MAX_INJECT_LEN 2048

static int hwsim_register_socket(struct nlmsg* nlmsg, int sock,
                                 int hwsim_family)
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = HWSIM_CMD_REGISTER;
  netlink_init(nlmsg, hwsim_family, 0, &genlhdr, sizeof(genlhdr));
  int err = netlink_send_ext(nlmsg, sock, 0, NULL, false);
  if (err < 0) {
  }
  return err;
}

static int hwsim_inject_frame(struct nlmsg* nlmsg, int sock, int hwsim_family,
                              uint8_t* mac_addr, uint8_t* data, int len)
{
  struct genlmsghdr genlhdr;
  uint32_t rx_rate = WIFI_DEFAULT_RX_RATE;
  uint32_t signal = WIFI_DEFAULT_SIGNAL;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = HWSIM_CMD_FRAME;
  netlink_init(nlmsg, hwsim_family, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, HWSIM_ATTR_RX_RATE, &rx_rate, sizeof(rx_rate));
  netlink_attr(nlmsg, HWSIM_ATTR_SIGNAL, &signal, sizeof(signal));
  netlink_attr(nlmsg, HWSIM_ATTR_ADDR_RECEIVER, mac_addr, ETH_ALEN);
  netlink_attr(nlmsg, HWSIM_ATTR_FRAME, data, len);
  int err = netlink_send_ext(nlmsg, sock, 0, NULL, false);
  if (err < 0) {
  }
  return err;
}

static long syz_80211_inject_frame(volatile long a0, volatile long a1,
                                   volatile long a2)
{
  uint8_t* mac_addr = (uint8_t*)a0;
  uint8_t* buf = (uint8_t*)a1;
  int buf_len = (int)a2;
  struct nlmsg tmp_msg;
  if (buf_len < 0 || buf_len > WIFI_MAX_INJECT_LEN) {
    return -1;
  }
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock < 0) {
    return -1;
  }
  int hwsim_family_id =
      netlink_query_family_id(&tmp_msg, sock, "MAC80211_HWSIM", false);
  if (hwsim_family_id < 0) {
    close(sock);
    return -1;
  }
  int ret = hwsim_register_socket(&tmp_msg, sock, hwsim_family_id);
  if (ret < 0) {
    close(sock);
    return -1;
  }
  ret = hwsim_inject_frame(&tmp_msg, sock, hwsim_family_id, mac_addr, buf,
                           buf_len);
  close(sock);
  if (ret < 0) {
    return -1;
  }
  return 0;
}

struct thread_t {
  int created, call;
  event_t ready, done;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    event_wait(&th->ready);
    event_reset(&th->ready);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    event_set(&th->done);
  }
  return 0;
}

static void loop(void)
{
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  int i, call, thread;
  for (call = 0; call < 12; call++) {
    for (thread = 0; thread < (int)(sizeof(threads) / sizeof(threads[0]));
         thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        event_init(&th->ready);
        event_init(&th->done);
        event_set(&th->done);
        thread_start(thr, th);
      }
      if (!event_isset(&th->done))
        continue;
      event_reset(&th->done);
      th->call = call;
      __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
      event_set(&th->ready);
      event_timedwait(&th->done, 50);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
}

uint64_t r[5] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0x0, 0x0};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    *(uint8_t*)0x200000000300 = 8;
    *(uint8_t*)0x200000000301 = 2;
    *(uint8_t*)0x200000000302 = 0x11;
    *(uint8_t*)0x200000000303 = 0;
    *(uint8_t*)0x200000000304 = 0;
    *(uint8_t*)0x200000000305 = 1;
    STORE_BY_BITMASK(uint8_t, , 0x200000000340, 0, 0, 2);
    STORE_BY_BITMASK(uint8_t, , 0x200000000340, 0, 2, 2);
    STORE_BY_BITMASK(uint8_t, , 0x200000000340, 8, 4, 4);
    STORE_BY_BITMASK(uint8_t, , 0x200000000341, 0, 0, 1);
    STORE_BY_BITMASK(uint8_t, , 0x200000000341, 0, 1, 1);
    STORE_BY_BITMASK(uint8_t, , 0x200000000341, 0, 2, 1);
    STORE_BY_BITMASK(uint8_t, , 0x200000000341, 0, 3, 1);
    STORE_BY_BITMASK(uint8_t, , 0x200000000341, 0, 4, 1);
    STORE_BY_BITMASK(uint8_t, , 0x200000000341, 0, 5, 1);
    STORE_BY_BITMASK(uint8_t, , 0x200000000341, 0, 6, 1);
    STORE_BY_BITMASK(uint8_t, , 0x200000000341, 0, 7, 1);
    STORE_BY_BITMASK(uint16_t, , 0x200000000342, 0, 0, 15);
    STORE_BY_BITMASK(uint16_t, , 0x200000000343, 0, 7, 1);
    *(uint8_t*)0x200000000344 = 8;
    *(uint8_t*)0x200000000345 = 2;
    *(uint8_t*)0x200000000346 = 0x11;
    *(uint8_t*)0x200000000347 = 0;
    *(uint8_t*)0x200000000348 = 0;
    *(uint8_t*)0x200000000349 = 1;
    *(uint8_t*)0x20000000034a = 8;
    *(uint8_t*)0x20000000034b = 2;
    *(uint8_t*)0x20000000034c = 0x11;
    *(uint8_t*)0x20000000034d = 0;
    *(uint8_t*)0x20000000034e = 0;
    *(uint8_t*)0x20000000034f = 0;
    memset((void*)0x200000000350, 80, 6);
    STORE_BY_BITMASK(uint16_t, , 0x200000000356, 0, 0, 4);
    STORE_BY_BITMASK(uint16_t, , 0x200000000356, 0, 4, 12);
    *(uint64_t*)0x200000000358 = 0xfffffffffffffffd;
    *(uint16_t*)0x200000000360 = 0x64;
    *(uint16_t*)0x200000000362 = 0x1001;
    *(uint8_t*)0x200000000364 = 0;
    *(uint8_t*)0x200000000365 = 6;
    memset((void*)0x200000000366, 2, 6);
    *(uint8_t*)0x20000000036c = 1;
    *(uint8_t*)0x20000000036d = 1;
    STORE_BY_BITMASK(uint8_t, , 0x20000000036e, 0xb, 0, 7);
    STORE_BY_BITMASK(uint8_t, , 0x20000000036e, 0, 7, 1);
    *(uint8_t*)0x20000000036f = 5;
    *(uint8_t*)0x200000000370 = 3;
    *(uint8_t*)0x200000000371 = 5;
    *(uint8_t*)0x200000000372 = 0xdd;
    *(uint8_t*)0x200000000373 = 0;
    *(uint8_t*)0x200000000374 = 0x2a;
    *(uint8_t*)0x200000000375 = 1;
    STORE_BY_BITMASK(uint8_t, , 0x200000000376, 1, 0, 1);
    STORE_BY_BITMASK(uint8_t, , 0x200000000376, 1, 1, 1);
    STORE_BY_BITMASK(uint8_t, , 0x200000000376, 0, 2, 1);
    STORE_BY_BITMASK(uint8_t, , 0x200000000376, 0, 3, 5);
    *(uint8_t*)0x200000000377 = 0x72;
    *(uint8_t*)0x200000000378 = 6;
    memset((void*)0x200000000379, 3, 6);
    *(uint8_t*)0x20000000037f = 0x71;
    *(uint8_t*)0x200000000380 = 7;
    *(uint8_t*)0x200000000381 = 0;
    *(uint8_t*)0x200000000382 = 0;
    *(uint8_t*)0x200000000383 = 0;
    *(uint8_t*)0x200000000384 = -1;
    *(uint8_t*)0x200000000385 = 0xfd;
    *(uint8_t*)0x200000000386 = 0;
    *(uint8_t*)0x200000000387 = 0;
    *(uint8_t*)0x200000000388 = 0x76;
    *(uint8_t*)0x200000000389 = 6;
    *(uint8_t*)0x20000000038a = 1;
    *(uint8_t*)0x20000000038b = 9;
    *(uint16_t*)0x20000000038c = 0x25;
    *(uint16_t*)0x20000000038e = 0xe;
    syz_80211_inject_frame(/*mac_addr=*/0x200000000300, /*buf=*/0x200000000340,
                           /*buf_len=*/0x50);
    break;
  case 1:
    res = syscall(__NR_socket, /*domain=*/0x10ul, /*type=*/3ul, /*proto=*/0x10);
    if (res != -1)
      r[0] = res;
    break;
  case 2:
    *(uint64_t*)0x2000000001c0 = 0;
    *(uint32_t*)0x2000000001c8 = 0;
    *(uint64_t*)0x2000000001d0 = 0x200000000200;
    *(uint64_t*)0x200000000200 = 0x200000000280;
    memset((void*)0x200000000280, 48, 2);
    *(uint64_t*)0x200000000208 = 0x30;
    *(uint64_t*)0x2000000001d8 = 1;
    *(uint64_t*)0x2000000001e0 = 0;
    *(uint64_t*)0x2000000001e8 = 0;
    *(uint32_t*)0x2000000001f0 = 0x18004;
    syscall(__NR_sendmsg, /*fd=*/-1, /*msg=*/0x2000000001c0ul, /*f=*/0ul);
    break;
  case 3:
    memcpy((void*)0x200000000100, "nl80211\000", 8);
    res = -1;
    res = syz_genetlink_get_family_id(/*name=*/0x200000000100, /*fd=*/-1);
    if (res != -1)
      r[1] = res;
    break;
  case 4:
    res = syscall(__NR_socket, /*domain=*/0x10ul, /*type=*/3ul, /*proto=*/0x10);
    if (res != -1)
      r[2] = res;
    break;
  case 5:
    *(uint64_t*)0x200000000500 = 0;
    *(uint32_t*)0x200000000508 = 0;
    *(uint64_t*)0x200000000510 = 0x2000000004c0;
    *(uint64_t*)0x2000000004c0 = 0x200000000240;
    memcpy((void*)0x200000000240, "D\000\000\000", 4);
    *(uint16_t*)0x200000000244 = r[1];
    memcpy((void*)0x200000000246,
           "\x01\x00\x00\x00\x00\x00\x80\x00\x00\x00\x1a\x00\x00\x00\x28\x00"
           "\x22\x80\x04\x14\x00\x80\x04\x00\x00\x80\x04\x00\x00\x80\x83\x41"
           "\xf1\x68\x02\x00\x00\x80\x14\x00\x00\x80\x04\x00\x00\x80\x04\x00"
           "\x00\x80\x04\x00\x00\x80\x06\x00\x21",
           57);
    *(uint64_t*)0x2000000004c8 = 0x44;
    *(uint64_t*)0x200000000518 = 1;
    *(uint64_t*)0x200000000520 = 0;
    *(uint64_t*)0x200000000528 = 0;
    *(uint32_t*)0x200000000530 = 0;
    syscall(__NR_sendmsg, /*fd=*/r[2], /*msg=*/0x200000000500ul, /*f=*/0ul);
    break;
  case 6:
    syscall(__NR_sendmsg, /*fd=*/-1, /*msg=*/0ul, /*f=*/0ul);
    break;
  case 7:
    memcpy((void*)0x200000000080, "nl80211\000", 8);
    res = -1;
    res = syz_genetlink_get_family_id(/*name=*/0x200000000080, /*fd=*/-1);
    if (res != -1)
      r[3] = res;
    break;
  case 8:
    memcpy((void*)0x2000000000c0,
           "wlan1\000\000\000\000\000\000\000\000\000\000\000", 16);
    res = syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x8933,
                  /*arg=*/0x2000000000c0ul);
    if (res != -1)
      r[4] = *(uint32_t*)0x2000000000d0;
    break;
  case 9:
    *(uint64_t*)0x200000000100 = 0;
    *(uint32_t*)0x200000000108 = 0;
    *(uint64_t*)0x200000000110 = 0x200000000140;
    *(uint64_t*)0x200000000140 = 0x200000000180;
    *(uint32_t*)0x200000000180 = 0x24;
    *(uint16_t*)0x200000000184 = r[3];
    *(uint16_t*)0x200000000186 = 5;
    *(uint32_t*)0x200000000188 = 0;
    *(uint32_t*)0x20000000018c = 0;
    *(uint8_t*)0x200000000190 = 6;
    *(uint8_t*)0x200000000191 = 0;
    *(uint16_t*)0x200000000192 = 0;
    *(uint16_t*)0x200000000194 = 8;
    *(uint16_t*)0x200000000196 = 3;
    *(uint32_t*)0x200000000198 = r[4];
    *(uint16_t*)0x20000000019c = 8;
    *(uint16_t*)0x20000000019e = 5;
    *(uint32_t*)0x2000000001a0 = 2;
    *(uint64_t*)0x200000000148 = 0x24;
    *(uint64_t*)0x200000000118 = 1;
    *(uint64_t*)0x200000000120 = 0;
    *(uint64_t*)0x200000000128 = 0;
    *(uint32_t*)0x200000000130 = 0;
    syscall(__NR_sendmsg, /*fd=*/r[0], /*msg=*/0x200000000100ul, /*f=*/0ul);
    break;
  case 10:
    *(uint64_t*)0x2000000001c0 = 0;
    *(uint32_t*)0x2000000001c8 = 0;
    *(uint64_t*)0x2000000001d0 = 0x200000000200;
    *(uint64_t*)0x200000000200 = 0x200000000a00;
    *(uint32_t*)0x200000000a00 = 0x28;
    *(uint16_t*)0x200000000a04 = r[3];
    *(uint16_t*)0x200000000a06 = 5;
    *(uint32_t*)0x200000000a08 = 0;
    *(uint32_t*)0x200000000a0c = 0;
    *(uint8_t*)0x200000000a10 = 0x2e;
    *(uint8_t*)0x200000000a11 = 0;
    *(uint16_t*)0x200000000a12 = 0;
    *(uint16_t*)0x200000000a14 = 8;
    *(uint16_t*)0x200000000a16 = 3;
    *(uint32_t*)0x200000000a18 = r[4];
    *(uint16_t*)0x200000000a1c = 0xa;
    *(uint16_t*)0x200000000a1e = 0x34;
    memset((void*)0x200000000a20, 2, 6);
    *(uint64_t*)0x200000000208 = 0x28;
    *(uint64_t*)0x2000000001d8 = 1;
    *(uint64_t*)0x2000000001e0 = 0;
    *(uint64_t*)0x2000000001e8 = 0;
    *(uint32_t*)0x2000000001f0 = 0;
    syscall(__NR_sendmsg, /*fd=*/r[0], /*msg=*/0x2000000001c0ul, /*f=*/0ul);
    break;
  case 11:
    syscall(__NR_ioctl, /*fd=*/-1, /*cmd=*/0x8b04, /*arg=*/0ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffffffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200000000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200001000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  const char* reason;
  (void)reason;
  do_sandbox_none();
  return 0;
}
