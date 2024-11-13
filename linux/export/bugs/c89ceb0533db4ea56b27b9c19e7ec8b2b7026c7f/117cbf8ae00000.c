// https://syzkaller.appspot.com/bug?id=c89ceb0533db4ea56b27b9c19e7ec8b2b7026c7f
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/genetlink.h>
#include <linux/if_addr.h>
#include <linux/if_link.h>
#include <linux/in6.h>
#include <linux/neighbour.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/usb/ch9.h>
#include <linux/veth.h>

unsigned long long procid;

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

static int netlink_send_ext(int sock, uint16_t reply_type, int* reply_len)
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
  if (n < sizeof(struct nlmsghdr))
    exit(1);
  if (reply_len && hdr->nlmsg_type == reply_type) {
    *reply_len = n;
    return 0;
  }
  if (n < sizeof(struct nlmsghdr) + sizeof(struct nlmsgerr))
    exit(1);
  if (hdr->nlmsg_type != NLMSG_ERROR)
    exit(1);
  return -((struct nlmsgerr*)(hdr + 1))->error;
}

static int netlink_send(int sock)
{
  return netlink_send_ext(sock, 0, NULL);
}

const int kInitNetNsFd = 239;

#define DEVLINK_FAMILY_NAME "devlink"

#define DEVLINK_CMD_RELOAD 37
#define DEVLINK_ATTR_BUS_NAME 1
#define DEVLINK_ATTR_DEV_NAME 2
#define DEVLINK_ATTR_NETNS_FD 137

static void netlink_devlink_netns_move(const char* bus_name,
                                       const char* dev_name, int netns_fd)
{
  struct genlmsghdr genlhdr;
  struct nlattr* attr;
  int sock, err, n;
  uint16_t id = 0;
  sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock == -1)
    exit(1);
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = CTRL_CMD_GETFAMILY;
  netlink_init(GENL_ID_CTRL, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(CTRL_ATTR_FAMILY_NAME, DEVLINK_FAMILY_NAME,
               strlen(DEVLINK_FAMILY_NAME) + 1);
  err = netlink_send_ext(sock, GENL_ID_CTRL, &n);
  if (err) {
    goto error;
  }
  attr =
      (struct nlattr*)(nlmsg.buf + NLMSG_HDRLEN + NLMSG_ALIGN(sizeof(genlhdr)));
  for (; (char*)attr < nlmsg.buf + n;
       attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
    if (attr->nla_type == CTRL_ATTR_FAMILY_ID) {
      id = *(uint16_t*)(attr + 1);
      break;
    }
  }
  if (!id) {
    goto error;
  }
  recv(sock, nlmsg.buf, sizeof(nlmsg.buf), 0); /* recv ack */
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = DEVLINK_CMD_RELOAD;
  netlink_init(id, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(DEVLINK_ATTR_BUS_NAME, bus_name, strlen(bus_name) + 1);
  netlink_attr(DEVLINK_ATTR_DEV_NAME, dev_name, strlen(dev_name) + 1);
  netlink_attr(DEVLINK_ATTR_NETNS_FD, &netns_fd, sizeof(netns_fd));
  netlink_send(sock);
error:
  close(sock);
}

static void initialize_devlink_pci(void)
{
  int netns = open("/proc/self/ns/net", O_RDONLY);
  if (netns == -1)
    exit(1);
  int ret = setns(kInitNetNsFd, 0);
  if (ret == -1)
    exit(1);
  netlink_devlink_netns_move("pci", "0000:00:10.0", netns);
  ret = setns(netns, 0);
  if (ret == -1)
    exit(1);
  close(netns);
}

#define MAX_FDS 30

#define USB_DEBUG 0

#define USB_MAX_IFACE_NUM 4
#define USB_MAX_EP_NUM 32

struct usb_iface_index {
  struct usb_interface_descriptor* iface;
  uint8_t bInterfaceNumber;
  uint8_t bAlternateSetting;
  struct usb_endpoint_descriptor eps[USB_MAX_EP_NUM];
  int eps_num;
};

struct usb_device_index {
  struct usb_device_descriptor* dev;
  struct usb_config_descriptor* config;
  uint8_t bMaxPower;
  int config_length;
  struct usb_iface_index ifaces[USB_MAX_IFACE_NUM];
  int ifaces_num;
  int iface_cur;
};

static bool parse_usb_descriptor(char* buffer, size_t length,
                                 struct usb_device_index* index)
{
  if (length < sizeof(*index->dev) + sizeof(*index->config))
    return false;
  memset(index, 0, sizeof(*index));
  index->dev = (struct usb_device_descriptor*)buffer;
  index->config = (struct usb_config_descriptor*)(buffer + sizeof(*index->dev));
  index->bMaxPower = index->config->bMaxPower;
  index->config_length = length - sizeof(*index->dev);
  index->iface_cur = -1;
  size_t offset = 0;
  while (true) {
    if (offset + 1 >= length)
      break;
    uint8_t desc_length = buffer[offset];
    uint8_t desc_type = buffer[offset + 1];
    if (desc_length <= 2)
      break;
    if (offset + desc_length > length)
      break;
    if (desc_type == USB_DT_INTERFACE &&
        index->ifaces_num < USB_MAX_IFACE_NUM) {
      struct usb_interface_descriptor* iface =
          (struct usb_interface_descriptor*)(buffer + offset);
      index->ifaces[index->ifaces_num].iface = iface;
      index->ifaces[index->ifaces_num].bInterfaceNumber =
          iface->bInterfaceNumber;
      index->ifaces[index->ifaces_num].bAlternateSetting =
          iface->bAlternateSetting;
      index->ifaces_num++;
    }
    if (desc_type == USB_DT_ENDPOINT && index->ifaces_num > 0) {
      struct usb_iface_index* iface = &index->ifaces[index->ifaces_num - 1];
      if (iface->eps_num < USB_MAX_EP_NUM) {
        memcpy(&iface->eps[iface->eps_num], buffer + offset,
               sizeof(iface->eps[iface->eps_num]));
        iface->eps_num++;
      }
    }
    offset += desc_length;
  }
  return true;
}

enum usb_raw_event_type {
  USB_RAW_EVENT_INVALID,
  USB_RAW_EVENT_CONNECT,
  USB_RAW_EVENT_CONTROL,
};

struct usb_raw_event {
  uint32_t type;
  uint32_t length;
  char data[0];
};

struct usb_raw_init {
  uint64_t speed;
  const char* driver_name;
  const char* device_name;
};

struct usb_raw_ep_io {
  uint16_t ep;
  uint16_t flags;
  uint32_t length;
  char data[0];
};

#define USB_RAW_IOCTL_INIT _IOW('U', 0, struct usb_raw_init)
#define USB_RAW_IOCTL_RUN _IO('U', 1)
#define USB_RAW_IOCTL_EVENT_FETCH _IOR('U', 2, struct usb_raw_event)
#define USB_RAW_IOCTL_EP0_WRITE _IOW('U', 3, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP0_READ _IOWR('U', 4, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP_ENABLE _IOW('U', 5, struct usb_endpoint_descriptor)
#define USB_RAW_IOCTL_EP_DISABLE _IOW('U', 6, int)
#define USB_RAW_IOCTL_EP_WRITE _IOW('U', 7, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP_READ _IOWR('U', 8, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_CONFIGURE _IO('U', 9)
#define USB_RAW_IOCTL_VBUS_DRAW _IOW('U', 10, uint32_t)

static int usb_raw_open()
{
  return open("/sys/kernel/debug/usb/raw-gadget", O_RDWR);
}

static int usb_raw_init(int fd, uint32_t speed, const char* driver,
                        const char* device)
{
  struct usb_raw_init arg;
  arg.speed = speed;
  arg.driver_name = driver;
  arg.device_name = device;
  return ioctl(fd, USB_RAW_IOCTL_INIT, &arg);
}

static int usb_raw_run(int fd)
{
  return ioctl(fd, USB_RAW_IOCTL_RUN, 0);
}

static int usb_raw_event_fetch(int fd, struct usb_raw_event* event)
{
  return ioctl(fd, USB_RAW_IOCTL_EVENT_FETCH, event);
}

static int usb_raw_ep0_write(int fd, struct usb_raw_ep_io* io)
{
  return ioctl(fd, USB_RAW_IOCTL_EP0_WRITE, io);
}

static int usb_raw_ep0_read(int fd, struct usb_raw_ep_io* io)
{
  return ioctl(fd, USB_RAW_IOCTL_EP0_READ, io);
}

static int usb_raw_ep_enable(int fd, struct usb_endpoint_descriptor* desc)
{
  return ioctl(fd, USB_RAW_IOCTL_EP_ENABLE, desc);
}

static int usb_raw_ep_disable(int fd, int ep)
{
  return ioctl(fd, USB_RAW_IOCTL_EP_DISABLE, ep);
}

static int usb_raw_configure(int fd)
{
  return ioctl(fd, USB_RAW_IOCTL_CONFIGURE, 0);
}

static int usb_raw_vbus_draw(int fd, uint32_t power)
{
  return ioctl(fd, USB_RAW_IOCTL_VBUS_DRAW, power);
}

#define MAX_USB_FDS 6

struct usb_info {
  int fd;
  struct usb_device_index index;
};

static struct usb_info usb_devices[MAX_USB_FDS];
static int usb_devices_num;

static struct usb_device_index* add_usb_index(int fd, char* dev, size_t dev_len)
{
  int i = __atomic_fetch_add(&usb_devices_num, 1, __ATOMIC_RELAXED);
  if (i >= MAX_USB_FDS)
    return NULL;
  int rv = 0;
  rv = parse_usb_descriptor(dev, dev_len, &usb_devices[i].index);
  if (!rv)
    return NULL;
  __atomic_store_n(&usb_devices[i].fd, fd, __ATOMIC_RELEASE);
  return &usb_devices[i].index;
}

static struct usb_device_index* lookup_usb_index(int fd)
{
  int i;
  for (i = 0; i < MAX_USB_FDS; i++) {
    if (__atomic_load_n(&usb_devices[i].fd, __ATOMIC_ACQUIRE) == fd) {
      return &usb_devices[i].index;
    }
  }
  return NULL;
}

static void set_interface(int fd, int n)
{
  struct usb_device_index* index = lookup_usb_index(fd);
  int ep;
  if (!index)
    return;
  if (index->iface_cur >= 0 && index->iface_cur < index->ifaces_num) {
    for (ep = 0; ep < index->ifaces[index->iface_cur].eps_num; ep++) {
      int rv = usb_raw_ep_disable(fd, ep);
      if (rv < 0) {
      } else {
      }
    }
  }
  if (n >= 0 && n < index->ifaces_num) {
    for (ep = 0; ep < index->ifaces[n].eps_num; ep++) {
      int rv = usb_raw_ep_enable(fd, &index->ifaces[n].eps[ep]);
      if (rv < 0) {
      } else {
      }
    }
    index->iface_cur = n;
  }
}

static int configure_device(int fd)
{
  struct usb_device_index* index = lookup_usb_index(fd);
  if (!index)
    return -1;
  int rv = usb_raw_vbus_draw(fd, index->bMaxPower);
  if (rv < 0) {
    return rv;
  }
  rv = usb_raw_configure(fd);
  if (rv < 0) {
    return rv;
  }
  set_interface(fd, 0);
  return 0;
}

#define USB_MAX_PACKET_SIZE 1024

struct usb_raw_control_event {
  struct usb_raw_event inner;
  struct usb_ctrlrequest ctrl;
  char data[USB_MAX_PACKET_SIZE];
};

struct usb_raw_ep_io_data {
  struct usb_raw_ep_io inner;
  char data[USB_MAX_PACKET_SIZE];
};

struct vusb_connect_string_descriptor {
  uint32_t len;
  char* str;
} __attribute__((packed));

struct vusb_connect_descriptors {
  uint32_t qual_len;
  char* qual;
  uint32_t bos_len;
  char* bos;
  uint32_t strs_len;
  struct vusb_connect_string_descriptor strs[0];
} __attribute__((packed));

static const char default_string[] = {8, USB_DT_STRING, 's', 0, 'y', 0, 'z', 0};

static const char default_lang_id[] = {4, USB_DT_STRING, 0x09, 0x04};

static bool lookup_connect_response(int fd,
                                    struct vusb_connect_descriptors* descs,
                                    struct usb_ctrlrequest* ctrl,
                                    char** response_data,
                                    uint32_t* response_length)
{
  struct usb_device_index* index = lookup_usb_index(fd);
  uint8_t str_idx;
  if (!index)
    return false;
  switch (ctrl->bRequestType & USB_TYPE_MASK) {
  case USB_TYPE_STANDARD:
    switch (ctrl->bRequest) {
    case USB_REQ_GET_DESCRIPTOR:
      switch (ctrl->wValue >> 8) {
      case USB_DT_DEVICE:
        *response_data = (char*)index->dev;
        *response_length = sizeof(*index->dev);
        return true;
      case USB_DT_CONFIG:
        *response_data = (char*)index->config;
        *response_length = index->config_length;
        return true;
      case USB_DT_STRING:
        str_idx = (uint8_t)ctrl->wValue;
        if (descs && str_idx < descs->strs_len) {
          *response_data = descs->strs[str_idx].str;
          *response_length = descs->strs[str_idx].len;
          return true;
        }
        if (str_idx == 0) {
          *response_data = (char*)&default_lang_id[0];
          *response_length = default_lang_id[0];
          return true;
        }
        *response_data = (char*)&default_string[0];
        *response_length = default_string[0];
        return true;
      case USB_DT_BOS:
        *response_data = descs->bos;
        *response_length = descs->bos_len;
        return true;
      case USB_DT_DEVICE_QUALIFIER:
        if (!descs->qual) {
          struct usb_qualifier_descriptor* qual =
              (struct usb_qualifier_descriptor*)response_data;
          qual->bLength = sizeof(*qual);
          qual->bDescriptorType = USB_DT_DEVICE_QUALIFIER;
          qual->bcdUSB = index->dev->bcdUSB;
          qual->bDeviceClass = index->dev->bDeviceClass;
          qual->bDeviceSubClass = index->dev->bDeviceSubClass;
          qual->bDeviceProtocol = index->dev->bDeviceProtocol;
          qual->bMaxPacketSize0 = index->dev->bMaxPacketSize0;
          qual->bNumConfigurations = index->dev->bNumConfigurations;
          qual->bRESERVED = 0;
          *response_length = sizeof(*qual);
          return true;
        }
        *response_data = descs->qual;
        *response_length = descs->qual_len;
        return true;
      default:
        exit(1);
        return false;
      }
      break;
    default:
      exit(1);
      return false;
    }
    break;
  default:
    exit(1);
    return false;
  }
  return false;
}

static volatile long syz_usb_connect(volatile long a0, volatile long a1,
                                     volatile long a2, volatile long a3)
{
  uint64_t speed = a0;
  uint64_t dev_len = a1;
  char* dev = (char*)a2;
  struct vusb_connect_descriptors* descs = (struct vusb_connect_descriptors*)a3;
  if (!dev) {
    return -1;
  }
  int fd = usb_raw_open();
  if (fd < 0) {
    return fd;
  }
  if (fd >= MAX_FDS) {
    close(fd);
    return -1;
  }
  struct usb_device_index* index = add_usb_index(fd, dev, dev_len);
  if (!index) {
    return -1;
  }
  char device[32];
  sprintf(&device[0], "dummy_udc.%llu", procid);
  int rv = usb_raw_init(fd, speed, "dummy_udc", &device[0]);
  if (rv < 0) {
    return rv;
  }
  rv = usb_raw_run(fd);
  if (rv < 0) {
    return rv;
  }
  bool done = false;
  while (!done) {
    struct usb_raw_control_event event;
    event.inner.type = 0;
    event.inner.length = sizeof(event.ctrl);
    rv = usb_raw_event_fetch(fd, (struct usb_raw_event*)&event);
    if (rv < 0) {
      return rv;
    }
    if (event.inner.type != USB_RAW_EVENT_CONTROL)
      continue;
    bool response_found = false;
    char* response_data = NULL;
    uint32_t response_length = 0;
    if (event.ctrl.bRequestType & USB_DIR_IN) {
      response_found = lookup_connect_response(
          fd, descs, &event.ctrl, &response_data, &response_length);
      if (!response_found) {
        return -1;
      }
    } else {
      if ((event.ctrl.bRequestType & USB_TYPE_MASK) != USB_TYPE_STANDARD ||
          event.ctrl.bRequest != USB_REQ_SET_CONFIGURATION) {
        exit(1);
        return -1;
      }
      done = true;
    }
    if (done) {
      rv = configure_device(fd);
      if (rv < 0) {
        return rv;
      }
    }
    struct usb_raw_ep_io_data response;
    response.inner.ep = 0;
    response.inner.flags = 0;
    if (response_length > sizeof(response.data))
      response_length = 0;
    if (event.ctrl.wLength < response_length)
      response_length = event.ctrl.wLength;
    response.inner.length = response_length;
    if (response_data)
      memcpy(&response.data[0], response_data, response_length);
    else
      memset(&response.data[0], 0, response_length);
    if (event.ctrl.bRequestType & USB_DIR_IN) {
      rv = usb_raw_ep0_write(fd, (struct usb_raw_ep_io*)&response);
    } else {
      rv = usb_raw_ep0_read(fd, (struct usb_raw_ep_io*)&response);
    }
    if (rv < 0) {
      return rv;
    }
  }
  sleep_ms(200);
  return fd;
}

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  int i;
  for (i = 0; i < 100; i++) {
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

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter;
  for (iter = 0;; iter++) {
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
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

void execute_one(void)
{
  memcpy(
      (void*)0x20000340,
      "\x12\x01\x00\x00\x00\x00\x00\x40\x6b\x1d\x01\x01\x40\x00\x01\x02\x03\x01"
      "\x09\x02\xe8\x00\x03\x01\x00\x00\x00\x09\x04\x00\x00\x00\x01\x01\x00\x00"
      "\x0a\x24\x01\x00\x00\x00\x02\x01\x02\x09\x24\x03\x00\x00\x00\x00\x00\x00"
      "\x09\x24\x03\x16\x07\x00\x00\x00\x00\x13\x24\x06\x00\x00\x06\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x07\x24\x07\x00\x00\x00\x00\x09"
      "\x24\x03\x00\x00\x00\x00\x06\x00\x7b\x24\x08\x06\x01\x00\x00\x00\xf4\x35"
      "\x4d\xd9\x03\x8d\x88\xf3\x5b\xdd\x70\xa8\x7f\x48\x13\x73\x1d\x28\x34\x09"
      "\x04\x01\x00\x00\x01\x02\x00\x00\x09\x04\x01\x01\x01\x01\x02\x00\x00\x10"
      "\x24\x21\x5d\x15\xfb\xb3\xac\x7f\x8a\x02\x02\x00\x00\x00\x00\x00\x51\x84"
      "\xb9\x08\xe2\xbe\x32\x0b\x24\x02\x01\x00\x00\x00\x00\x6f\xa8\xeb\x0b\x24"
      "\x02\x01\x00\x00\x00\x00\x6d\x48\xe4\x09\x24\x02\x01\x00\x00\x00\x00\xab"
      "\x07\x24\x01\x00\x00\x00\x00\x0a\x24\x02\x02\x00\x00\x00\x00\x00\xe7\x09"
      "\xac\x01\x09\x00\x00\x00\x00\x00\x07\x25\x01\x00\x00\x00\x00\x09\x04\x02"
      "\x00\x00\x01\x02\x00\x00\x09\x04\x02\x01\x01\x01\x02\x00\x00\x09\x05\x82"
      "\x09\x00\x00\x00\x00\x00\x07\x25\x01\x00\x00\x00\x00\xae\x76\x83\xdd\x89"
      "\xfb\x27\xfd\x66\x58\xbd\xe9\xa8\x9f\x22\x16\x28\x1c\x8c\x10\x54\xc3\x7a"
      "\x75\xfb\x8b\xcc\x4b\x54\x7f\xc5\x61\xf9\x4a\x2a\xa5\xa9\x27\xc3\x32\xa0"
      "\xa7\xe0\x7c\x3f\x12\xf6\x51\x1f\x72\x17\x09\xd1\x93\xe4\x9a\xc9\x5c\x8d"
      "\xf8\x76\xb0\x93\x54\x24\x07\x56\x3f\x15\xf8\xe9\x4a\xfc\x3e\x3b\xb9\x33"
      "\xec\x32\x00\xf9\xec\x9b\x6e\x70\x60\xc2\x8f\x72\x00\x0f\xff\xe7\x6c\x93"
      "\xf1\x0f\x4c\x67\x2b\x4c\x56\xdd\x35\xd3\x98\x7c\x80\x65\xbb\xff\xa0\x53"
      "\x19\xcc\xf7\x56\xec\x2e\x82\x16\x65\xc7\x9d\x25\x96\x1c\xc3\x96\x89\x78"
      "\xf5\x51\x79\x62\x4f\x52\xec\x18\x15\x69\x6f\xcb\x42\x98\x5a\x41\xc5\x11"
      "\x1a\xeb\xa7\x67\x1b\xc0\xd8\x84\x73\x49\xcb\x91\xc5\xd5\x1b\x83\x15\xa2"
      "\xc1\x5b\x74\x9b\x82\xf7\x81\x89\x03\x22\x6a\x70\xfa\xbb\x42\xea\x4c\x4e"
      "\x25\x1b\xc9\x1a\xdc\x48\x06\x97\xc9\xd9\x9b\x0a\x77\xb7\xd2\xd9\x14\x12"
      "\xea\x10\xf0\x4c\x84\x57\x59\x27\x9d\x52\x5c\xae\x88\xcf\x53\xe6\x3e\x05"
      "\xb9\x10\x45\x4a\xbb\xe9\xce\xab\x3f\x24\xde\x3a\x26\xe9\x47\xb1\x20\x5d"
      "\x3b\x4a\x08\xb0\xc7\x2f\x51\x09\xe0\x6f\xf4\x45\x75\x33\x39\x16\x5e\x27"
      "\x81\x8d\xfe\xe0\x8c\x49\x79\xec\xc3\x30\x00\x1b\x87\x32\xea\xb8\x71\x2e"
      "\xf3\xba\xe9\x2a\x41\xcc",
      546);
  syz_usb_connect(0, 0xf2, 0x20000340, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}