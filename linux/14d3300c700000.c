// https://syzkaller.appspot.com/bug?id=03a8c2ce78209467d06fff861be8d37703868cd7
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
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

static unsigned long long procid;

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
                                 uint32_t iftype)
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = NL80211_CMD_SET_INTERFACE;
  netlink_init(nlmsg, nl80211_family, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, NL80211_ATTR_IFINDEX, &ifindex, sizeof(ifindex));
  netlink_attr(nlmsg, NL80211_ATTR_IFTYPE, &iftype, sizeof(iftype));
  int err = netlink_send(nlmsg, sock);
  if (err < 0) {
  }
  return err;
}

static int nl80211_join_ibss(struct nlmsg* nlmsg, int sock, int nl80211_family,
                             uint32_t ifindex, struct join_ibss_props* props)
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
  int err = netlink_send(nlmsg, sock);
  if (err < 0) {
  }
  return err;
}

static int get_ifla_operstate(struct nlmsg* nlmsg, int ifindex)
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
  int err = netlink_send_ext(nlmsg, sock, RTM_NEWLINK, &n, true);
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
                                int operstate)
{
  int ifindex = if_nametoindex(interface);
  while (true) {
    usleep(1000);
    int ret = get_ifla_operstate(nlmsg, ifindex);
    if (ret < 0)
      return ret;
    if (ret == operstate)
      return 0;
  }
  return 0;
}

static int nl80211_setup_ibss_interface(struct nlmsg* nlmsg, int sock,
                                        int nl80211_family_id, char* interface,
                                        struct join_ibss_props* ibss_props)
{
  int ifindex = if_nametoindex(interface);
  if (ifindex == 0) {
    return -1;
  }
  int ret = nl80211_set_interface(nlmsg, sock, nl80211_family_id, ifindex,
                                  NL80211_IFTYPE_ADHOC);
  if (ret < 0) {
    return -1;
  }
  ret = set_interface_state(interface, 1);
  if (ret < 0) {
    return -1;
  }
  ret = nl80211_join_ibss(nlmsg, sock, nl80211_family_id, ifindex, ibss_props);
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
  if (rfkill == -1) {
    if (errno != ENOENT && errno != EACCES)
      exit(1);
  } else {
    struct rfkill_event event = {0};
    event.type = RFKILL_TYPE_ALL;
    event.op = RFKILL_OP_CHANGE_ALL;
    if (write(rfkill, &event, sizeof(event)) != (ssize_t)(sizeof(event)))
      exit(1);
    close(rfkill);
  }
  uint8_t mac_addr[6] = WIFI_MAC_BASE;
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock < 0) {
    return;
  }
  int hwsim_family_id =
      netlink_query_family_id(&nlmsg, sock, "MAC80211_HWSIM", true);
  int nl80211_family_id =
      netlink_query_family_id(&nlmsg, sock, "nl80211", true);
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
                                     &ibss_props) < 0)
      exit(1);
  }
  for (int device_id = 0; device_id < WIFI_INITIAL_DEVICE_COUNT; device_id++) {
    char interface[6] = "wlan0";
    interface[4] += device_id;
    int ret = await_ifla_operstate(&nlmsg, interface, IF_OPER_UP);
    if (ret < 0)
      exit(1);
  }
  close(sock);
}

#define MAX_FDS 30

#define BTPROTO_HCI 1
#define ACL_LINK 1
#define SCAN_PAGE 2

typedef struct {
  uint8_t b[6];
} __attribute__((packed)) bdaddr_t;

#define HCI_COMMAND_PKT 1
#define HCI_EVENT_PKT 4
#define HCI_VENDOR_PKT 0xff

struct hci_command_hdr {
  uint16_t opcode;
  uint8_t plen;
} __attribute__((packed));

struct hci_event_hdr {
  uint8_t evt;
  uint8_t plen;
} __attribute__((packed));

#define HCI_EV_CONN_COMPLETE 0x03
struct hci_ev_conn_complete {
  uint8_t status;
  uint16_t handle;
  bdaddr_t bdaddr;
  uint8_t link_type;
  uint8_t encr_mode;
} __attribute__((packed));

#define HCI_EV_CONN_REQUEST 0x04
struct hci_ev_conn_request {
  bdaddr_t bdaddr;
  uint8_t dev_class[3];
  uint8_t link_type;
} __attribute__((packed));

#define HCI_EV_REMOTE_FEATURES 0x0b
struct hci_ev_remote_features {
  uint8_t status;
  uint16_t handle;
  uint8_t features[8];
} __attribute__((packed));

#define HCI_EV_CMD_COMPLETE 0x0e
struct hci_ev_cmd_complete {
  uint8_t ncmd;
  uint16_t opcode;
} __attribute__((packed));

#define HCI_OP_WRITE_SCAN_ENABLE 0x0c1a

#define HCI_OP_READ_BUFFER_SIZE 0x1005
struct hci_rp_read_buffer_size {
  uint8_t status;
  uint16_t acl_mtu;
  uint8_t sco_mtu;
  uint16_t acl_max_pkt;
  uint16_t sco_max_pkt;
} __attribute__((packed));

#define HCI_OP_READ_BD_ADDR 0x1009
struct hci_rp_read_bd_addr {
  uint8_t status;
  bdaddr_t bdaddr;
} __attribute__((packed));

#define HCI_EV_LE_META 0x3e
struct hci_ev_le_meta {
  uint8_t subevent;
} __attribute__((packed));

#define HCI_EV_LE_CONN_COMPLETE 0x01
struct hci_ev_le_conn_complete {
  uint8_t status;
  uint16_t handle;
  uint8_t role;
  uint8_t bdaddr_type;
  bdaddr_t bdaddr;
  uint16_t interval;
  uint16_t latency;
  uint16_t supervision_timeout;
  uint8_t clk_accurancy;
} __attribute__((packed));

struct hci_dev_req {
  uint16_t dev_id;
  uint32_t dev_opt;
};

struct vhci_vendor_pkt {
  uint8_t type;
  uint8_t opcode;
  uint16_t id;
};

#define HCIDEVUP _IOW('H', 201, int)
#define HCISETSCAN _IOW('H', 221, int)

static int vhci_fd = -1;

static void rfkill_unblock_all()
{
  int fd = open("/dev/rfkill", O_WRONLY);
  if (fd < 0)
    exit(1);
  struct rfkill_event event = {0};
  event.idx = 0;
  event.type = RFKILL_TYPE_ALL;
  event.op = RFKILL_OP_CHANGE_ALL;
  event.soft = 0;
  event.hard = 0;
  if (write(fd, &event, sizeof(event)) < 0)
    exit(1);
  close(fd);
}

static void hci_send_event_packet(int fd, uint8_t evt, void* data,
                                  size_t data_len)
{
  struct iovec iv[3];
  struct hci_event_hdr hdr;
  hdr.evt = evt;
  hdr.plen = data_len;
  uint8_t type = HCI_EVENT_PKT;
  iv[0].iov_base = &type;
  iv[0].iov_len = sizeof(type);
  iv[1].iov_base = &hdr;
  iv[1].iov_len = sizeof(hdr);
  iv[2].iov_base = data;
  iv[2].iov_len = data_len;
  if (writev(fd, iv, sizeof(iv) / sizeof(struct iovec)) < 0)
    exit(1);
}

static void hci_send_event_cmd_complete(int fd, uint16_t opcode, void* data,
                                        size_t data_len)
{
  struct iovec iv[4];
  struct hci_event_hdr hdr;
  hdr.evt = HCI_EV_CMD_COMPLETE;
  hdr.plen = sizeof(struct hci_ev_cmd_complete) + data_len;
  struct hci_ev_cmd_complete evt_hdr;
  evt_hdr.ncmd = 1;
  evt_hdr.opcode = opcode;
  uint8_t type = HCI_EVENT_PKT;
  iv[0].iov_base = &type;
  iv[0].iov_len = sizeof(type);
  iv[1].iov_base = &hdr;
  iv[1].iov_len = sizeof(hdr);
  iv[2].iov_base = &evt_hdr;
  iv[2].iov_len = sizeof(evt_hdr);
  iv[3].iov_base = data;
  iv[3].iov_len = data_len;
  if (writev(fd, iv, sizeof(iv) / sizeof(struct iovec)) < 0)
    exit(1);
}

static bool process_command_pkt(int fd, char* buf, ssize_t buf_size)
{
  struct hci_command_hdr* hdr = (struct hci_command_hdr*)buf;
  if (buf_size < (ssize_t)sizeof(struct hci_command_hdr) ||
      hdr->plen != buf_size - sizeof(struct hci_command_hdr))
    exit(1);
  switch (hdr->opcode) {
  case HCI_OP_WRITE_SCAN_ENABLE: {
    uint8_t status = 0;
    hci_send_event_cmd_complete(fd, hdr->opcode, &status, sizeof(status));
    return true;
  }
  case HCI_OP_READ_BD_ADDR: {
    struct hci_rp_read_bd_addr rp = {0};
    rp.status = 0;
    memset(&rp.bdaddr, 0xaa, 6);
    hci_send_event_cmd_complete(fd, hdr->opcode, &rp, sizeof(rp));
    return false;
  }
  case HCI_OP_READ_BUFFER_SIZE: {
    struct hci_rp_read_buffer_size rp = {0};
    rp.status = 0;
    rp.acl_mtu = 1021;
    rp.sco_mtu = 96;
    rp.acl_max_pkt = 4;
    rp.sco_max_pkt = 6;
    hci_send_event_cmd_complete(fd, hdr->opcode, &rp, sizeof(rp));
    return false;
  }
  }
  char dummy[0xf9] = {0};
  hci_send_event_cmd_complete(fd, hdr->opcode, dummy, sizeof(dummy));
  return false;
}

static void* event_thread(void* arg)
{
  while (1) {
    char buf[1024] = {0};
    ssize_t buf_size = read(vhci_fd, buf, sizeof(buf));
    if (buf_size < 0)
      exit(1);
    if (buf_size > 0 && buf[0] == HCI_COMMAND_PKT) {
      if (process_command_pkt(vhci_fd, buf + 1, buf_size - 1))
        break;
    }
  }
  return NULL;
}
#define HCI_HANDLE_1 200
#define HCI_HANDLE_2 201

static void initialize_vhci()
{
  int hci_sock = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
  if (hci_sock < 0)
    exit(1);
  vhci_fd = open("/dev/vhci", O_RDWR);
  if (vhci_fd == -1)
    exit(1);
  const int kVhciFd = 202;
  if (dup2(vhci_fd, kVhciFd) < 0)
    exit(1);
  close(vhci_fd);
  vhci_fd = kVhciFd;
  struct vhci_vendor_pkt vendor_pkt;
  if (read(vhci_fd, &vendor_pkt, sizeof(vendor_pkt)) != sizeof(vendor_pkt))
    exit(1);
  if (vendor_pkt.type != HCI_VENDOR_PKT)
    exit(1);
  pthread_t th;
  if (pthread_create(&th, NULL, event_thread, NULL))
    exit(1);
  int ret = ioctl(hci_sock, HCIDEVUP, vendor_pkt.id);
  if (ret) {
    if (errno == ERFKILL) {
      rfkill_unblock_all();
      ret = ioctl(hci_sock, HCIDEVUP, vendor_pkt.id);
    }
    if (ret && errno != EALREADY)
      exit(1);
  }
  struct hci_dev_req dr = {0};
  dr.dev_id = vendor_pkt.id;
  dr.dev_opt = SCAN_PAGE;
  if (ioctl(hci_sock, HCISETSCAN, &dr))
    exit(1);
  struct hci_ev_conn_request request;
  memset(&request, 0, sizeof(request));
  memset(&request.bdaddr, 0xaa, 6);
  *(uint8_t*)&request.bdaddr.b[5] = 0x10;
  request.link_type = ACL_LINK;
  hci_send_event_packet(vhci_fd, HCI_EV_CONN_REQUEST, &request,
                        sizeof(request));
  struct hci_ev_conn_complete complete;
  memset(&complete, 0, sizeof(complete));
  complete.status = 0;
  complete.handle = HCI_HANDLE_1;
  memset(&complete.bdaddr, 0xaa, 6);
  *(uint8_t*)&complete.bdaddr.b[5] = 0x10;
  complete.link_type = ACL_LINK;
  complete.encr_mode = 0;
  hci_send_event_packet(vhci_fd, HCI_EV_CONN_COMPLETE, &complete,
                        sizeof(complete));
  struct hci_ev_remote_features features;
  memset(&features, 0, sizeof(features));
  features.status = 0;
  features.handle = HCI_HANDLE_1;
  hci_send_event_packet(vhci_fd, HCI_EV_REMOTE_FEATURES, &features,
                        sizeof(features));
  struct {
    struct hci_ev_le_meta le_meta;
    struct hci_ev_le_conn_complete le_conn;
  } le_conn;
  memset(&le_conn, 0, sizeof(le_conn));
  le_conn.le_meta.subevent = HCI_EV_LE_CONN_COMPLETE;
  memset(&le_conn.le_conn.bdaddr, 0xaa, 6);
  *(uint8_t*)&le_conn.le_conn.bdaddr.b[5] = 0x11;
  le_conn.le_conn.role = 1;
  le_conn.le_conn.handle = HCI_HANDLE_2;
  hci_send_event_packet(vhci_fd, HCI_EV_LE_META, &le_conn, sizeof(le_conn));
  pthread_join(th, NULL);
  close(hci_sock);
}

static void setup_common()
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
  initialize_vhci();
  sandbox_common();
  drop_caps();
  if (unshare(CLONE_NEWNET)) {
  }
  initialize_wifi_devices();
  setup_binderfs();
  loop();
  exit(1);
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

static void close_fds()
{
  for (int fd = 3; fd < MAX_FDS; fd++)
    close(fd);
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

static void execute_one(void)
{
  int i, call, thread;
  for (call = 0; call < 6; call++) {
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
  close_fds();
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

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 0x29ul, 2ul, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    memcpy(
        (void*)0x20000900,
        "wlan1\000\033\032\354\330n\377\303\207\342\243\200\212\262\331\250L"
        "\006\265\022\017F\331\037\212\314\272\\\316YF2C\375j\343\215\343\326"
        "\340|6l\351\331;\352\204]\337\367\276r\'\212\325W\273\254%"
        "j\235\353\272\346\304\304\251\365\325\241\365\\\233\262\a\336\273\3018"
        "\204\265:f\313\350o\aArYZ\341\311\206\376\217("
        "\241\vhb\030\363\343\241\322\223*h\327\242F\210\3257\262\310\214S\350:"
        "H}\221\221\314\247Y\313kK\360\376\236\325\241\036\231~\2353\322?"
        "\b\277U\350\213\223\352`\000\200\000\000V\277!"
        "\267\351\021\r\275\243\306-t\234;\232s\206\347\275\260\325\";"
        "\354uP\"\353rV\210\312\030\000\000\000\000\000\000\000\000\000\000\000"
        "\000\230\350\306\303\nE\221\377\330E$\304As\200\333t\016\3421_v1\330,"
        "\244\177D\224\350?\370\315[1\262U,\310w0|"
        "E\000\210IoQpH\240\350\360\177\275\274s;\311\322\031oS\254\306\233`:"
        "6\311DS\023\373\335w\nK\031\372\231\306~\0044\243+)\357@"
        "Lr\355\205\363\350#\244\204\351W8\326\200\225\272.?+O\276[&"
        "\207\341\305\327C\241\336\244\b*w\334]\222\316\346BNFj;\327 "
        "\373\f\353\261\270\206x\031\240\304\323^"
        "W\267\020\030\272\314\253J\337YB\"\226\rny3\351\354\337\304\256\372M"
        "\242k\330X\346hQ\t\223\301\375\267\244\004W\240n\377",
        396);
    syscall(__NR_ioctl, r[0], 0x8914, 0x20000900ul);
    break;
  case 2:
    res = syscall(__NR_socket, 0x29ul, 2ul, 0);
    if (res != -1)
      r[1] = res;
    break;
  case 3:
    memcpy((void*)0x20000040,
           "wlan1\000\033\032\354\265\022\003F\331U\034\311%"
           "\233\240\365\356\026\037\271\362-\332,C\375j\343\215\343\326\340|"
           "6l\351\331;"
           "\023\337\367\276r\'\212\325\325\341\365\\\233\262\a\336\273\3018"
           "\204\265:"
           "f\313\350oOArYZ\341\311\206\376\210\235\372\254J\037\353p\365\373"
           "\252d\032\240\261\234\254\350\377^9P\356\212G\3352",
           105);
    syscall(__NR_ioctl, r[1], 0x8914, 0x20000040ul);
    break;
  case 4:
    res = syscall(__NR_socket, 0x29ul, 5ul, 0);
    if (res != -1)
      r[2] = res;
    break;
  case 5:
    memcpy(
        (void*)0x20000780,
        "wlan1\000\033\032\354\265\022\003F\331U\034\006\000\000\000\177\000"
        "\000\000\000\000\001\000,C\375j\343\215\343\337\367\276rAy\372n\n\214$"
        "\236p\'\212\325\325\341\365\\\233\262\a\336\273g\3018\204\265:"
        "f\313\350oOArYZ\341\001\353p\365\373\252d\032\240\261\234\254\350\377^"
        "9P\356\212G\3352\016\330\a\312\310~\202\366\252j\345\367\031\353#;|"
        "\253A1\252\2747Tf\361Y\034\355~"
        "\351\351\371\317W0\026\306G\2436\3070Zz\307\351\215\006\324\2362I["
        "\317\367\260K\034\034\006h\214d\f6\376\267\277\256\352\231\352\263G"
        "\325\214\200\220("
        "\237\225\206\"\212\226\365\031\266\031\twp\372\251\330\277Sa\271\v3"
        "\261\005\271\3713\254\024\037Xf\375\336\036\203\177\200\335L[t% "
        "/H\355\352\200*NA\272X\r\352\v\203<"
        "\241d\3057\371\030\243\017q\231\2534%"
        "1\b\000\000\000\000\000\000\000\343\004SYQ\267\b\324\261\323\366\265"
        "\334\2250\230\305 "
        "\2444\327\f\230R\345\303\220\256j\254\003\234\252*"
        "6\201K\344Xu\353BR\032\342l8\3677>\317\027\366\3226i\330\340\316|"
        "\333\317\002\030\245\021\022\2240\347\"\372\036\236\201\350\274\253"
        "\240h\313\324:\023%C+\025\252\257\305\337#"
        "\300\006\001\350\373c\227\247M\033R\312M\273\233\'\233\370\331\254\375"
        "\350\237\260\000\262\222y89\b693\275\213\"\370g\304U\303\221\243\376O"
        "\'\313\271\264\325\037>"
        "\030\200\253\f\005\341\034\325Q\036g\025\\rO0r\346\264++{"
        "\027\377\313\3541\000\315\000\362\020\327\245\314i\352\372\204\375o"
        "\032\212\246-\016\201\v\334\346%0L1\320\247\233\000\000\033\230\274<"
        "\231\t\r\001\247\352\000\000\000\000\000\000\000\000\000\000\000\351+~"
        "\\\017\351\2632\nn_\367\020\341H\350)C\232\334N\264g\216\3256\a\245{"
        "q\327\036\033\305\302\3470\355ep\005N\230Q\362\333\3566rV\036T\334\223"
        "p\316`\201\330\343\237\177\v7`R\227c\"\257l\206\n5\367L\353\311!"
        "y\306\376\256Fa^9f\347~*"
        "\330\371\344\002\335\305\266\302\033\235\214\361\356)"
        "\a\f\0241\332\263\365\333\327\037\316\265\367\365\277\301\361\314A$"
        "\316K\244\207\017g\307\210\360ZW^\270\025\255n5\034\036\035\257="
        "\347\022\245\226>\377P\240V\233\326\000\000\v\223\360\346@"
        "\366\321\341\224\264%\204\v#\252\033\026\370ht+"
        "\300\333\220\303\024\030\3341p\177X\226&\302\n8M)LH\017S\257\244|\360&"
        "\304\235r\000y\357<"
        "\334\213\265\177\n0\274\2640U\253\2638\367G\264\022\'\210:"
        "\aX\266Q\025",
        725);
    syscall(__NR_ioctl, r[2], 0x8b06, 0x20000780ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      do_sandbox_none();
    }
  }
  sleep(1000000);
  return 0;
}
