// https://syzkaller.appspot.com/bug?id=3c558412597cc402fd7fbb250ca30d04d46c8c60
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
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
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/usb/ch9.h>

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

#define MAX_FDS 30

#define USB_MAX_IFACE_NUM 4
#define USB_MAX_EP_NUM 32
#define USB_MAX_FDS 6

struct usb_endpoint_index {
  struct usb_endpoint_descriptor desc;
  int handle;
};

struct usb_iface_index {
  struct usb_interface_descriptor* iface;
  uint8_t bInterfaceNumber;
  uint8_t bAlternateSetting;
  uint8_t bInterfaceClass;
  struct usb_endpoint_index eps[USB_MAX_EP_NUM];
  int eps_num;
};

struct usb_device_index {
  struct usb_device_descriptor* dev;
  struct usb_config_descriptor* config;
  uint8_t bDeviceClass;
  uint8_t bMaxPower;
  int config_length;
  struct usb_iface_index ifaces[USB_MAX_IFACE_NUM];
  int ifaces_num;
  int iface_cur;
};

struct usb_info {
  int fd;
  struct usb_device_index index;
};

static struct usb_info usb_devices[USB_MAX_FDS];
static int usb_devices_num;

static bool parse_usb_descriptor(const char* buffer, size_t length,
                                 struct usb_device_index* index)
{
  if (length < sizeof(*index->dev) + sizeof(*index->config))
    return false;
  memset(index, 0, sizeof(*index));
  index->dev = (struct usb_device_descriptor*)buffer;
  index->config = (struct usb_config_descriptor*)(buffer + sizeof(*index->dev));
  index->bDeviceClass = index->dev->bDeviceClass;
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
      index->ifaces[index->ifaces_num].bInterfaceClass = iface->bInterfaceClass;
      index->ifaces_num++;
    }
    if (desc_type == USB_DT_ENDPOINT && index->ifaces_num > 0) {
      struct usb_iface_index* iface = &index->ifaces[index->ifaces_num - 1];
      if (iface->eps_num < USB_MAX_EP_NUM) {
        memcpy(&iface->eps[iface->eps_num].desc, buffer + offset,
               sizeof(iface->eps[iface->eps_num].desc));
        iface->eps_num++;
      }
    }
    offset += desc_length;
  }
  return true;
}

static struct usb_device_index* add_usb_index(int fd, const char* dev,
                                              size_t dev_len)
{
  int i = __atomic_fetch_add(&usb_devices_num, 1, __ATOMIC_RELAXED);
  if (i >= USB_MAX_FDS)
    return NULL;
  if (!parse_usb_descriptor(dev, dev_len, &usb_devices[i].index))
    return NULL;
  __atomic_store_n(&usb_devices[i].fd, fd, __ATOMIC_RELEASE);
  return &usb_devices[i].index;
}

static struct usb_device_index* lookup_usb_index(int fd)
{
  for (int i = 0; i < USB_MAX_FDS; i++) {
    if (__atomic_load_n(&usb_devices[i].fd, __ATOMIC_ACQUIRE) == fd) {
      return &usb_devices[i].index;
    }
  }
  return NULL;
}

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

static bool
lookup_connect_response_in(int fd, const struct vusb_connect_descriptors* descs,
                           const struct usb_ctrlrequest* ctrl,
                           char** response_data, uint32_t* response_length)
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
        break;
      }
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
  return false;
}

typedef bool (*lookup_connect_out_response_t)(
    int fd, const struct vusb_connect_descriptors* descs,
    const struct usb_ctrlrequest* ctrl, bool* done);

static bool lookup_connect_response_out_generic(
    int fd, const struct vusb_connect_descriptors* descs,
    const struct usb_ctrlrequest* ctrl, bool* done)
{
  switch (ctrl->bRequestType & USB_TYPE_MASK) {
  case USB_TYPE_STANDARD:
    switch (ctrl->bRequest) {
    case USB_REQ_SET_CONFIGURATION:
      *done = true;
      return true;
    default:
      break;
    }
    break;
  }
  return false;
}

#define UDC_NAME_LENGTH_MAX 128

struct usb_raw_init {
  __u8 driver_name[UDC_NAME_LENGTH_MAX];
  __u8 device_name[UDC_NAME_LENGTH_MAX];
  __u8 speed;
};

enum usb_raw_event_type {
  USB_RAW_EVENT_INVALID = 0,
  USB_RAW_EVENT_CONNECT = 1,
  USB_RAW_EVENT_CONTROL = 2,
};

struct usb_raw_event {
  __u32 type;
  __u32 length;
  __u8 data[0];
};

struct usb_raw_ep_io {
  __u16 ep;
  __u16 flags;
  __u32 length;
  __u8 data[0];
};

#define USB_RAW_EPS_NUM_MAX 30
#define USB_RAW_EP_NAME_MAX 16
#define USB_RAW_EP_ADDR_ANY 0xff

struct usb_raw_ep_caps {
  __u32 type_control : 1;
  __u32 type_iso : 1;
  __u32 type_bulk : 1;
  __u32 type_int : 1;
  __u32 dir_in : 1;
  __u32 dir_out : 1;
};

struct usb_raw_ep_limits {
  __u16 maxpacket_limit;
  __u16 max_streams;
  __u32 reserved;
};

struct usb_raw_ep_info {
  __u8 name[USB_RAW_EP_NAME_MAX];
  __u32 addr;
  struct usb_raw_ep_caps caps;
  struct usb_raw_ep_limits limits;
};

struct usb_raw_eps_info {
  struct usb_raw_ep_info eps[USB_RAW_EPS_NUM_MAX];
};

#define USB_RAW_IOCTL_INIT _IOW('U', 0, struct usb_raw_init)
#define USB_RAW_IOCTL_RUN _IO('U', 1)
#define USB_RAW_IOCTL_EVENT_FETCH _IOR('U', 2, struct usb_raw_event)
#define USB_RAW_IOCTL_EP0_WRITE _IOW('U', 3, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP0_READ _IOWR('U', 4, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP_ENABLE _IOW('U', 5, struct usb_endpoint_descriptor)
#define USB_RAW_IOCTL_EP_DISABLE _IOW('U', 6, __u32)
#define USB_RAW_IOCTL_EP_WRITE _IOW('U', 7, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP_READ _IOWR('U', 8, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_CONFIGURE _IO('U', 9)
#define USB_RAW_IOCTL_VBUS_DRAW _IOW('U', 10, __u32)
#define USB_RAW_IOCTL_EPS_INFO _IOR('U', 11, struct usb_raw_eps_info)
#define USB_RAW_IOCTL_EP0_STALL _IO('U', 12)
#define USB_RAW_IOCTL_EP_SET_HALT _IOW('U', 13, __u32)
#define USB_RAW_IOCTL_EP_CLEAR_HALT _IOW('U', 14, __u32)
#define USB_RAW_IOCTL_EP_SET_WEDGE _IOW('U', 15, __u32)

static int usb_raw_open()
{
  return open("/dev/raw-gadget", O_RDWR);
}

static int usb_raw_init(int fd, uint32_t speed, const char* driver,
                        const char* device)
{
  struct usb_raw_init arg;
  strncpy((char*)&arg.driver_name[0], driver, sizeof(arg.driver_name));
  strncpy((char*)&arg.device_name[0], device, sizeof(arg.device_name));
  arg.speed = speed;
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

static int usb_raw_ep0_stall(int fd)
{
  return ioctl(fd, USB_RAW_IOCTL_EP0_STALL, 0);
}

static void set_interface(int fd, int n)
{
  struct usb_device_index* index = lookup_usb_index(fd);
  if (!index)
    return;
  if (index->iface_cur >= 0 && index->iface_cur < index->ifaces_num) {
    for (int ep = 0; ep < index->ifaces[index->iface_cur].eps_num; ep++) {
      int rv = usb_raw_ep_disable(
          fd, index->ifaces[index->iface_cur].eps[ep].handle);
      if (rv < 0) {
      } else {
      }
    }
  }
  if (n >= 0 && n < index->ifaces_num) {
    for (int ep = 0; ep < index->ifaces[n].eps_num; ep++) {
      int rv = usb_raw_ep_enable(fd, &index->ifaces[n].eps[ep].desc);
      if (rv < 0) {
      } else {
        index->ifaces[n].eps[ep].handle = rv;
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

#define USB_MAX_PACKET_SIZE 4096

struct usb_raw_control_event {
  struct usb_raw_event inner;
  struct usb_ctrlrequest ctrl;
  char data[USB_MAX_PACKET_SIZE];
};

struct usb_raw_ep_io_data {
  struct usb_raw_ep_io inner;
  char data[USB_MAX_PACKET_SIZE];
};

static volatile long
syz_usb_connect_impl(uint64_t speed, uint64_t dev_len, const char* dev,
                     const struct vusb_connect_descriptors* descs,
                     lookup_connect_out_response_t lookup_connect_response_out)
{
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
    char* response_data = NULL;
    uint32_t response_length = 0;
    if (event.ctrl.bRequestType & USB_DIR_IN) {
      if (!lookup_connect_response_in(fd, descs, &event.ctrl, &response_data,
                                      &response_length)) {
        usb_raw_ep0_stall(fd);
        continue;
      }
    } else {
      if (!lookup_connect_response_out(fd, descs, &event.ctrl, &done)) {
        usb_raw_ep0_stall(fd);
        continue;
      }
      response_data = NULL;
      response_length = event.ctrl.wLength;
    }
    if ((event.ctrl.bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD &&
        event.ctrl.bRequest == USB_REQ_SET_CONFIGURATION) {
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

static volatile long syz_usb_connect(volatile long a0, volatile long a1,
                                     volatile long a2, volatile long a3)
{
  uint64_t speed = a0;
  uint64_t dev_len = a1;
  const char* dev = (const char*)a2;
  const struct vusb_connect_descriptors* descs =
      (const struct vusb_connect_descriptors*)a3;
  return syz_usb_connect_impl(speed, dev_len, dev, descs,
                              &lookup_connect_response_out_generic);
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
      if (current_time_ms() - start < 5000) {
        continue;
      }
      kill_and_wait(pid, &status);
      break;
    }
  }
}

void execute_one(void)
{
  *(uint8_t*)0x20000780 = 0x12;
  *(uint8_t*)0x20000781 = 1;
  *(uint16_t*)0x20000782 = 0x300;
  *(uint8_t*)0x20000784 = 0x62;
  *(uint8_t*)0x20000785 = 0x11;
  *(uint8_t*)0x20000786 = -1;
  *(uint8_t*)0x20000787 = 0x20;
  *(uint16_t*)0x20000788 = 0xfe9;
  *(uint16_t*)0x2000078a = 0xdb01;
  *(uint16_t*)0x2000078c = 0x87e;
  *(uint8_t*)0x2000078e = 1;
  *(uint8_t*)0x2000078f = 2;
  *(uint8_t*)0x20000790 = 3;
  *(uint8_t*)0x20000791 = 1;
  *(uint8_t*)0x20000792 = 9;
  *(uint8_t*)0x20000793 = 2;
  *(uint16_t*)0x20000794 = 0x270;
  *(uint8_t*)0x20000796 = 4;
  *(uint8_t*)0x20000797 = 9;
  *(uint8_t*)0x20000798 = 0;
  *(uint8_t*)0x20000799 = 0xc0;
  *(uint8_t*)0x2000079a = 8;
  *(uint8_t*)0x2000079b = 9;
  *(uint8_t*)0x2000079c = 4;
  *(uint8_t*)0x2000079d = 0x46;
  *(uint8_t*)0x2000079e = 0x3f;
  *(uint8_t*)0x2000079f = 0xf;
  *(uint8_t*)0x200007a0 = 0x29;
  *(uint8_t*)0x200007a1 = 0x84;
  *(uint8_t*)0x200007a2 = 0x17;
  *(uint8_t*)0x200007a3 = 0;
  *(uint8_t*)0x200007a4 = 5;
  *(uint8_t*)0x200007a5 = 0x24;
  *(uint8_t*)0x200007a6 = 6;
  *(uint8_t*)0x200007a7 = 0;
  *(uint8_t*)0x200007a8 = 0;
  *(uint8_t*)0x200007a9 = 5;
  *(uint8_t*)0x200007aa = 0x24;
  *(uint8_t*)0x200007ab = 0;
  *(uint16_t*)0x200007ac = 1;
  *(uint8_t*)0x200007ae = 0xd;
  *(uint8_t*)0x200007af = 0x24;
  *(uint8_t*)0x200007b0 = 0xf;
  *(uint8_t*)0x200007b1 = 1;
  *(uint32_t*)0x200007b2 = 4;
  *(uint16_t*)0x200007b6 = 7;
  *(uint16_t*)0x200007b8 = 0xfd;
  *(uint8_t*)0x200007ba = 2;
  *(uint8_t*)0x200007bb = 0xc;
  *(uint8_t*)0x200007bc = 0x24;
  *(uint8_t*)0x200007bd = 0x1b;
  *(uint16_t*)0x200007be = 5;
  *(uint16_t*)0x200007c0 = 5;
  *(uint8_t*)0x200007c2 = 0;
  *(uint8_t*)0x200007c3 = 8;
  *(uint16_t*)0x200007c4 = 0x400;
  *(uint8_t*)0x200007c6 = 4;
  *(uint8_t*)0x200007c7 = 0xc;
  *(uint8_t*)0x200007c8 = 0x24;
  *(uint8_t*)0x200007c9 = 0x1b;
  *(uint16_t*)0x200007ca = 2;
  *(uint16_t*)0x200007cc = 9;
  *(uint8_t*)0x200007ce = 2;
  *(uint8_t*)0x200007cf = 0x80;
  *(uint16_t*)0x200007d0 = 4;
  *(uint8_t*)0x200007d2 = 9;
  *(uint8_t*)0x200007d3 = 0xc;
  *(uint8_t*)0x200007d4 = 0x24;
  *(uint8_t*)0x200007d5 = 0x1b;
  *(uint16_t*)0x200007d6 = 0x3f;
  *(uint16_t*)0x200007d8 = 1;
  *(uint8_t*)0x200007da = 0;
  *(uint8_t*)0x200007db = 8;
  *(uint16_t*)0x200007dc = 0x800;
  *(uint8_t*)0x200007de = 0x7f;
  *(uint8_t*)0x200007df = 9;
  *(uint8_t*)0x200007e0 = 5;
  *(uint8_t*)0x200007e1 = 6;
  *(uint8_t*)0x200007e2 = 3;
  *(uint16_t*)0x200007e3 = 8;
  *(uint8_t*)0x200007e5 = 1;
  *(uint8_t*)0x200007e6 = 2;
  *(uint8_t*)0x200007e7 = 0x40;
  *(uint8_t*)0x200007e8 = 2;
  *(uint8_t*)0x200007e9 = 1;
  *(uint8_t*)0x200007ea = 9;
  *(uint8_t*)0x200007eb = 5;
  *(uint8_t*)0x200007ec = 0xd;
  *(uint8_t*)0x200007ed = 0x10;
  *(uint16_t*)0x200007ee = 0x400;
  *(uint8_t*)0x200007f0 = 0;
  *(uint8_t*)0x200007f1 = -1;
  *(uint8_t*)0x200007f2 = 7;
  *(uint8_t*)0x200007f3 = 2;
  *(uint8_t*)0x200007f4 = 2;
  *(uint8_t*)0x200007f5 = 7;
  *(uint8_t*)0x200007f6 = 0x25;
  *(uint8_t*)0x200007f7 = 1;
  *(uint8_t*)0x200007f8 = 0;
  *(uint8_t*)0x200007f9 = 3;
  *(uint16_t*)0x200007fa = 0xe6;
  *(uint8_t*)0x200007fc = 9;
  *(uint8_t*)0x200007fd = 5;
  *(uint8_t*)0x200007fe = 0xe;
  *(uint8_t*)0x200007ff = 3;
  *(uint16_t*)0x20000800 = 8;
  *(uint8_t*)0x20000802 = 8;
  *(uint8_t*)0x20000803 = 6;
  *(uint8_t*)0x20000804 = 3;
  *(uint8_t*)0x20000805 = 2;
  *(uint8_t*)0x20000806 = 4;
  *(uint8_t*)0x20000807 = 9;
  *(uint8_t*)0x20000808 = 5;
  *(uint8_t*)0x20000809 = 8;
  *(uint8_t*)0x2000080a = 0;
  *(uint16_t*)0x2000080b = 0x20;
  *(uint8_t*)0x2000080d = 0x85;
  *(uint8_t*)0x2000080e = 5;
  *(uint8_t*)0x2000080f = 0;
  *(uint8_t*)0x20000810 = 9;
  *(uint8_t*)0x20000811 = 5;
  *(uint8_t*)0x20000812 = 8;
  *(uint8_t*)0x20000813 = 0;
  *(uint16_t*)0x20000814 = 0x40;
  *(uint8_t*)0x20000816 = 5;
  *(uint8_t*)0x20000817 = 3;
  *(uint8_t*)0x20000818 = 8;
  *(uint8_t*)0x20000819 = 2;
  *(uint8_t*)0x2000081a = 0x23;
  *(uint8_t*)0x2000081b = 9;
  *(uint8_t*)0x2000081c = 5;
  *(uint8_t*)0x2000081d = 0xc;
  *(uint8_t*)0x2000081e = 0x10;
  *(uint16_t*)0x2000081f = 0x400;
  *(uint8_t*)0x20000821 = -1;
  *(uint8_t*)0x20000822 = 0x45;
  *(uint8_t*)0x20000823 = 0x81;
  *(uint8_t*)0x20000824 = 9;
  *(uint8_t*)0x20000825 = 5;
  *(uint8_t*)0x20000826 = 0xf;
  *(uint8_t*)0x20000827 = 0x10;
  *(uint16_t*)0x20000828 = 0x240;
  *(uint8_t*)0x2000082a = 0xe5;
  *(uint8_t*)0x2000082b = 0x7c;
  *(uint8_t*)0x2000082c = 0x81;
  *(uint8_t*)0x2000082d = 7;
  *(uint8_t*)0x2000082e = 0x25;
  *(uint8_t*)0x2000082f = 1;
  *(uint8_t*)0x20000830 = 2;
  *(uint8_t*)0x20000831 = 0xd7;
  *(uint16_t*)0x20000832 = 0;
  *(uint8_t*)0x20000834 = 9;
  *(uint8_t*)0x20000835 = 5;
  *(uint8_t*)0x20000836 = 0xe;
  *(uint8_t*)0x20000837 = 0;
  *(uint16_t*)0x20000838 = 0x3ff;
  *(uint8_t*)0x2000083a = 0;
  *(uint8_t*)0x2000083b = 0x40;
  *(uint8_t*)0x2000083c = 1;
  *(uint8_t*)0x2000083d = 7;
  *(uint8_t*)0x2000083e = 0x25;
  *(uint8_t*)0x2000083f = 1;
  *(uint8_t*)0x20000840 = 1;
  *(uint8_t*)0x20000841 = 7;
  *(uint16_t*)0x20000842 = 9;
  *(uint8_t*)0x20000844 = 7;
  *(uint8_t*)0x20000845 = 0x25;
  *(uint8_t*)0x20000846 = 1;
  *(uint8_t*)0x20000847 = 2;
  *(uint8_t*)0x20000848 = 0x1f;
  *(uint16_t*)0x20000849 = 0xce9;
  *(uint8_t*)0x2000084b = 9;
  *(uint8_t*)0x2000084c = 5;
  *(uint8_t*)0x2000084d = 9;
  *(uint8_t*)0x2000084e = 0x10;
  *(uint16_t*)0x2000084f = 0x3ff;
  *(uint8_t*)0x20000851 = 0;
  *(uint8_t*)0x20000852 = 5;
  *(uint8_t*)0x20000853 = 1;
  *(uint8_t*)0x20000854 = 2;
  *(uint8_t*)0x20000855 = 0xc;
  *(uint8_t*)0x20000856 = 2;
  *(uint8_t*)0x20000857 = 0x23;
  *(uint8_t*)0x20000858 = 9;
  *(uint8_t*)0x20000859 = 5;
  *(uint8_t*)0x2000085a = 7;
  *(uint8_t*)0x2000085b = 0x10;
  *(uint16_t*)0x2000085c = 0x10;
  *(uint8_t*)0x2000085e = 7;
  *(uint8_t*)0x2000085f = 0x3f;
  *(uint8_t*)0x20000860 = 6;
  *(uint8_t*)0x20000861 = 7;
  *(uint8_t*)0x20000862 = 0x25;
  *(uint8_t*)0x20000863 = 1;
  *(uint8_t*)0x20000864 = 1;
  *(uint8_t*)0x20000865 = 7;
  *(uint16_t*)0x20000866 = 9;
  *(uint8_t*)0x20000868 = 7;
  *(uint8_t*)0x20000869 = 0x25;
  *(uint8_t*)0x2000086a = 1;
  *(uint8_t*)0x2000086b = 0;
  *(uint8_t*)0x2000086c = 6;
  *(uint16_t*)0x2000086d = 0x1f;
  *(uint8_t*)0x2000086f = 9;
  *(uint8_t*)0x20000870 = 5;
  *(uint8_t*)0x20000871 = 0xf;
  *(uint8_t*)0x20000872 = 0x13;
  *(uint16_t*)0x20000873 = 0x200;
  *(uint8_t*)0x20000875 = 0x7f;
  *(uint8_t*)0x20000876 = 6;
  *(uint8_t*)0x20000877 = 3;
  *(uint8_t*)0x20000878 = 9;
  *(uint8_t*)0x20000879 = 5;
  *(uint8_t*)0x2000087a = 3;
  *(uint8_t*)0x2000087b = 0;
  *(uint16_t*)0x2000087c = 0x3ff;
  *(uint8_t*)0x2000087e = 1;
  *(uint8_t*)0x2000087f = 5;
  *(uint8_t*)0x20000880 = 1;
  *(uint8_t*)0x20000881 = 9;
  *(uint8_t*)0x20000882 = 5;
  *(uint8_t*)0x20000883 = 5;
  *(uint8_t*)0x20000884 = 0x10;
  *(uint16_t*)0x20000885 = 0x400;
  *(uint8_t*)0x20000887 = 7;
  *(uint8_t*)0x20000888 = 1;
  *(uint8_t*)0x20000889 = 2;
  *(uint8_t*)0x2000088a = 7;
  *(uint8_t*)0x2000088b = 0x25;
  *(uint8_t*)0x2000088c = 1;
  *(uint8_t*)0x2000088d = 0x42;
  *(uint8_t*)0x2000088e = 6;
  *(uint16_t*)0x2000088f = 9;
  *(uint8_t*)0x20000891 = 2;
  *(uint8_t*)0x20000892 = 0x23;
  *(uint8_t*)0x20000893 = 9;
  *(uint8_t*)0x20000894 = 5;
  *(uint8_t*)0x20000895 = 0x80;
  *(uint8_t*)0x20000896 = 4;
  *(uint16_t*)0x20000897 = 0x230;
  *(uint8_t*)0x20000899 = 7;
  *(uint8_t*)0x2000089a = -1;
  *(uint8_t*)0x2000089b = 1;
  *(uint8_t*)0x2000089c = 2;
  *(uint8_t*)0x2000089d = 3;
  *(uint8_t*)0x2000089e = 7;
  *(uint8_t*)0x2000089f = 0x25;
  *(uint8_t*)0x200008a0 = 1;
  *(uint8_t*)0x200008a1 = 0;
  *(uint8_t*)0x200008a2 = 8;
  *(uint16_t*)0x200008a3 = 1;
  *(uint8_t*)0x200008a5 = 9;
  *(uint8_t*)0x200008a6 = 5;
  *(uint8_t*)0x200008a7 = 0;
  *(uint8_t*)0x200008a8 = 0xc;
  *(uint16_t*)0x200008a9 = 0x10;
  *(uint8_t*)0x200008ab = 6;
  *(uint8_t*)0x200008ac = 0x80;
  *(uint8_t*)0x200008ad = 0x42;
  *(uint8_t*)0x200008ae = 9;
  *(uint8_t*)0x200008af = 4;
  *(uint8_t*)0x200008b0 = 0x66;
  *(uint8_t*)0x200008b1 = 0x40;
  *(uint8_t*)0x200008b2 = 2;
  *(uint8_t*)0x200008b3 = -1;
  *(uint8_t*)0x200008b4 = -1;
  *(uint8_t*)0x200008b5 = -1;
  *(uint8_t*)0x200008b6 = 0x3f;
  *(uint8_t*)0x200008b7 = 8;
  *(uint8_t*)0x200008b8 = 0x24;
  *(uint8_t*)0x200008b9 = 2;
  *(uint8_t*)0x200008ba = 1;
  *(uint8_t*)0x200008bb = 0x40;
  *(uint8_t*)0x200008bc = 4;
  *(uint8_t*)0x200008bd = 0x81;
  *(uint8_t*)0x200008be = 0x7f;
  *(uint8_t*)0x200008bf = 9;
  *(uint8_t*)0x200008c0 = 0x24;
  *(uint8_t*)0x200008c1 = 2;
  *(uint8_t*)0x200008c2 = 2;
  *(uint16_t*)0x200008c3 = 0x760f;
  *(uint16_t*)0x200008c5 = 7;
  *(uint8_t*)0x200008c7 = 0;
  *(uint8_t*)0x200008c8 = 8;
  *(uint8_t*)0x200008c9 = 0x24;
  *(uint8_t*)0x200008ca = 2;
  *(uint8_t*)0x200008cb = 1;
  *(uint8_t*)0x200008cc = 0xc1;
  *(uint8_t*)0x200008cd = 3;
  *(uint8_t*)0x200008ce = 0x1f;
  *(uint8_t*)0x200008cf = 4;
  *(uint8_t*)0x200008d0 = 7;
  *(uint8_t*)0x200008d1 = 0x24;
  *(uint8_t*)0x200008d2 = 1;
  *(uint8_t*)0x200008d3 = 4;
  *(uint8_t*)0x200008d4 = 5;
  *(uint16_t*)0x200008d5 = 0x1003;
  *(uint8_t*)0x200008d7 = 9;
  *(uint8_t*)0x200008d8 = 5;
  *(uint8_t*)0x200008d9 = 0x80;
  *(uint8_t*)0x200008da = 0x10;
  *(uint16_t*)0x200008db = 0x610;
  *(uint8_t*)0x200008dd = 4;
  *(uint8_t*)0x200008de = 1;
  *(uint8_t*)0x200008df = 2;
  *(uint8_t*)0x200008e0 = 2;
  *(uint8_t*)0x200008e1 = 0xa;
  *(uint8_t*)0x200008e2 = 7;
  *(uint8_t*)0x200008e3 = 0x25;
  *(uint8_t*)0x200008e4 = 1;
  *(uint8_t*)0x200008e5 = 3;
  *(uint8_t*)0x200008e6 = 3;
  *(uint16_t*)0x200008e7 = 0x595;
  *(uint8_t*)0x200008e9 = 9;
  *(uint8_t*)0x200008ea = 5;
  *(uint8_t*)0x200008eb = 9;
  *(uint8_t*)0x200008ec = 0x10;
  *(uint16_t*)0x200008ed = 0x400;
  *(uint8_t*)0x200008ef = 0x36;
  *(uint8_t*)0x200008f0 = -1;
  *(uint8_t*)0x200008f1 = 0x81;
  *(uint8_t*)0x200008f2 = 2;
  *(uint8_t*)0x200008f3 = 3;
  *(uint8_t*)0x200008f4 = 9;
  *(uint8_t*)0x200008f5 = 4;
  *(uint8_t*)0x200008f6 = 0x76;
  *(uint8_t*)0x200008f7 = 9;
  *(uint8_t*)0x200008f8 = 5;
  *(uint8_t*)0x200008f9 = 0xca;
  *(uint8_t*)0x200008fa = 0x1d;
  *(uint8_t*)0x200008fb = 0x25;
  *(uint8_t*)0x200008fc = 9;
  *(uint8_t*)0x200008fd = 0xa;
  *(uint8_t*)0x200008fe = 0x24;
  *(uint8_t*)0x200008ff = 1;
  *(uint16_t*)0x20000900 = 0xf1;
  *(uint8_t*)0x20000902 = 7;
  *(uint8_t*)0x20000903 = 2;
  *(uint8_t*)0x20000904 = 1;
  *(uint8_t*)0x20000905 = 2;
  *(uint8_t*)0x20000906 = 9;
  *(uint8_t*)0x20000907 = 0x24;
  *(uint8_t*)0x20000908 = 3;
  *(uint8_t*)0x20000909 = 2;
  *(uint16_t*)0x2000090a = 0;
  *(uint8_t*)0x2000090c = 5;
  *(uint8_t*)0x2000090d = 2;
  *(uint8_t*)0x2000090e = 1;
  *(uint8_t*)0x2000090f = 8;
  *(uint8_t*)0x20000910 = 0x24;
  *(uint8_t*)0x20000911 = 2;
  *(uint8_t*)0x20000912 = 1;
  *(uint8_t*)0x20000913 = 0x97;
  *(uint8_t*)0x20000914 = 1;
  *(uint8_t*)0x20000915 = 3;
  *(uint8_t*)0x20000916 = 0x93;
  *(uint8_t*)0x20000917 = 9;
  *(uint8_t*)0x20000918 = 0x24;
  *(uint8_t*)0x20000919 = 2;
  *(uint8_t*)0x2000091a = 2;
  *(uint16_t*)0x2000091b = 0x800;
  *(uint16_t*)0x2000091d = 1;
  *(uint8_t*)0x2000091f = 2;
  *(uint8_t*)0x20000920 = 8;
  *(uint8_t*)0x20000921 = 0x24;
  *(uint8_t*)0x20000922 = 2;
  *(uint8_t*)0x20000923 = 1;
  *(uint8_t*)0x20000924 = 0;
  *(uint8_t*)0x20000925 = 1;
  *(uint8_t*)0x20000926 = 5;
  *(uint8_t*)0x20000927 = 5;
  *(uint8_t*)0x20000928 = 8;
  *(uint8_t*)0x20000929 = 0x24;
  *(uint8_t*)0x2000092a = 2;
  *(uint8_t*)0x2000092b = 1;
  *(uint8_t*)0x2000092c = 5;
  *(uint8_t*)0x2000092d = 4;
  *(uint8_t*)0x2000092e = 0x6c;
  *(uint8_t*)0x2000092f = 1;
  *(uint8_t*)0x20000930 = 9;
  *(uint8_t*)0x20000931 = 5;
  *(uint8_t*)0x20000932 = 0xd;
  *(uint8_t*)0x20000933 = 0x10;
  *(uint16_t*)0x20000934 = 0x400;
  *(uint8_t*)0x20000936 = 2;
  *(uint8_t*)0x20000937 = 7;
  *(uint8_t*)0x20000938 = 0;
  *(uint8_t*)0x20000939 = 2;
  *(uint8_t*)0x2000093a = 0x30;
  *(uint8_t*)0x2000093b = 7;
  *(uint8_t*)0x2000093c = 0x25;
  *(uint8_t*)0x2000093d = 1;
  *(uint8_t*)0x2000093e = 2;
  *(uint8_t*)0x2000093f = 2;
  *(uint16_t*)0x20000940 = 0xff;
  *(uint8_t*)0x20000942 = 9;
  *(uint8_t*)0x20000943 = 5;
  *(uint8_t*)0x20000944 = 0xa;
  *(uint8_t*)0x20000945 = 8;
  *(uint16_t*)0x20000946 = 0x40;
  *(uint8_t*)0x20000948 = 0x20;
  *(uint8_t*)0x20000949 = 5;
  *(uint8_t*)0x2000094a = 0x9d;
  *(uint8_t*)0x2000094b = 9;
  *(uint8_t*)0x2000094c = 5;
  *(uint8_t*)0x2000094d = 0;
  *(uint8_t*)0x2000094e = 1;
  *(uint16_t*)0x2000094f = 0x40;
  *(uint8_t*)0x20000951 = 0;
  *(uint8_t*)0x20000952 = 0;
  *(uint8_t*)0x20000953 = 8;
  *(uint8_t*)0x20000954 = 9;
  *(uint8_t*)0x20000955 = 5;
  *(uint8_t*)0x20000956 = 0xa;
  *(uint8_t*)0x20000957 = 0;
  *(uint16_t*)0x20000958 = 0x400;
  *(uint8_t*)0x2000095a = 2;
  *(uint8_t*)0x2000095b = 9;
  *(uint8_t*)0x2000095c = 1;
  *(uint8_t*)0x2000095d = 7;
  *(uint8_t*)0x2000095e = 0x25;
  *(uint8_t*)0x2000095f = 1;
  *(uint8_t*)0x20000960 = 0x85;
  *(uint8_t*)0x20000961 = 0x7f;
  *(uint16_t*)0x20000962 = 0x8001;
  *(uint8_t*)0x20000964 = 9;
  *(uint8_t*)0x20000965 = 5;
  *(uint8_t*)0x20000966 = 0xd;
  *(uint8_t*)0x20000967 = 0;
  *(uint16_t*)0x20000968 = 0x5dfd;
  *(uint8_t*)0x2000096a = 7;
  *(uint8_t*)0x2000096b = -1;
  *(uint8_t*)0x2000096c = 0xd5;
  *(uint8_t*)0x2000096d = 2;
  *(uint8_t*)0x2000096e = 0x31;
  *(uint8_t*)0x2000096f = 9;
  *(uint8_t*)0x20000970 = 4;
  *(uint8_t*)0x20000971 = 0x48;
  *(uint8_t*)0x20000972 = 0xa2;
  *(uint8_t*)0x20000973 = 7;
  *(uint8_t*)0x20000974 = 0x1c;
  *(uint8_t*)0x20000975 = 0x24;
  *(uint8_t*)0x20000976 = 0x21;
  *(uint8_t*)0x20000977 = 0xd1;
  *(uint8_t*)0x20000978 = 5;
  *(uint8_t*)0x20000979 = 0x24;
  *(uint8_t*)0x2000097a = 6;
  *(uint8_t*)0x2000097b = 0;
  *(uint8_t*)0x2000097c = 0;
  *(uint8_t*)0x2000097d = 5;
  *(uint8_t*)0x2000097e = 0x24;
  *(uint8_t*)0x2000097f = 0;
  *(uint16_t*)0x20000980 = 8;
  *(uint8_t*)0x20000982 = 0xd;
  *(uint8_t*)0x20000983 = 0x24;
  *(uint8_t*)0x20000984 = 0xf;
  *(uint8_t*)0x20000985 = 1;
  *(uint32_t*)0x20000986 = 7;
  *(uint16_t*)0x2000098a = 0x3f;
  *(uint16_t*)0x2000098c = 0x825d;
  *(uint8_t*)0x2000098e = 3;
  *(uint8_t*)0x2000098f = 5;
  *(uint8_t*)0x20000990 = 0x24;
  *(uint8_t*)0x20000991 = 0x15;
  *(uint16_t*)0x20000992 = 8;
  *(uint8_t*)0x20000994 = 9;
  *(uint8_t*)0x20000995 = 0x21;
  *(uint16_t*)0x20000996 = 5;
  *(uint8_t*)0x20000998 = 1;
  *(uint8_t*)0x20000999 = 1;
  *(uint8_t*)0x2000099a = 0x22;
  *(uint16_t*)0x2000099b = 0x888;
  *(uint8_t*)0x2000099d = 9;
  *(uint8_t*)0x2000099e = 5;
  *(uint8_t*)0x2000099f = 8;
  *(uint8_t*)0x200009a0 = 8;
  *(uint16_t*)0x200009a1 = 0x400;
  *(uint8_t*)0x200009a3 = 2;
  *(uint8_t*)0x200009a4 = 1;
  *(uint8_t*)0x200009a5 = 1;
  *(uint8_t*)0x200009a6 = 9;
  *(uint8_t*)0x200009a7 = 5;
  *(uint8_t*)0x200009a8 = 0xa;
  *(uint8_t*)0x200009a9 = 0x10;
  *(uint16_t*)0x200009aa = 0x3ff;
  *(uint8_t*)0x200009ac = 0x20;
  *(uint8_t*)0x200009ad = 0xd9;
  *(uint8_t*)0x200009ae = 0xc6;
  *(uint8_t*)0x200009af = 2;
  *(uint8_t*)0x200009b0 = 3;
  *(uint8_t*)0x200009b1 = 7;
  *(uint8_t*)0x200009b2 = 0x25;
  *(uint8_t*)0x200009b3 = 1;
  *(uint8_t*)0x200009b4 = 0x81;
  *(uint8_t*)0x200009b5 = 9;
  *(uint16_t*)0x200009b6 = 6;
  *(uint8_t*)0x200009b8 = 9;
  *(uint8_t*)0x200009b9 = 5;
  *(uint8_t*)0x200009ba = 0x80;
  *(uint8_t*)0x200009bb = 4;
  *(uint16_t*)0x200009bc = 0x20;
  *(uint8_t*)0x200009be = 0x25;
  *(uint8_t*)0x200009bf = 0x82;
  *(uint8_t*)0x200009c0 = 0xb0;
  *(uint8_t*)0x200009c1 = 9;
  *(uint8_t*)0x200009c2 = 5;
  *(uint8_t*)0x200009c3 = 0xf;
  *(uint8_t*)0x200009c4 = 0;
  *(uint16_t*)0x200009c5 = 8;
  *(uint8_t*)0x200009c7 = 0x81;
  *(uint8_t*)0x200009c8 = 6;
  *(uint8_t*)0x200009c9 = 7;
  *(uint8_t*)0x200009ca = 2;
  *(uint8_t*)0x200009cb = 5;
  *(uint8_t*)0x200009cc = 9;
  *(uint8_t*)0x200009cd = 5;
  *(uint8_t*)0x200009ce = 0xa;
  *(uint8_t*)0x200009cf = 8;
  *(uint16_t*)0x200009d0 = 0x10;
  *(uint8_t*)0x200009d2 = 4;
  *(uint8_t*)0x200009d3 = 0xd8;
  *(uint8_t*)0x200009d4 = 6;
  *(uint8_t*)0x200009d5 = 2;
  *(uint8_t*)0x200009d6 = 0x31;
  *(uint8_t*)0x200009d7 = 2;
  *(uint8_t*)0x200009d8 = 0x23;
  *(uint8_t*)0x200009d9 = 9;
  *(uint8_t*)0x200009da = 5;
  *(uint8_t*)0x200009db = 0xb;
  *(uint8_t*)0x200009dc = 2;
  *(uint16_t*)0x200009dd = 8;
  *(uint8_t*)0x200009df = 0x81;
  *(uint8_t*)0x200009e0 = 0x81;
  *(uint8_t*)0x200009e1 = 0x81;
  *(uint8_t*)0x200009e2 = 7;
  *(uint8_t*)0x200009e3 = 0x25;
  *(uint8_t*)0x200009e4 = 1;
  *(uint8_t*)0x200009e5 = 1;
  *(uint8_t*)0x200009e6 = 2;
  *(uint16_t*)0x200009e7 = 7;
  *(uint8_t*)0x200009e9 = 7;
  *(uint8_t*)0x200009ea = 0x25;
  *(uint8_t*)0x200009eb = 1;
  *(uint8_t*)0x200009ec = 0x83;
  *(uint8_t*)0x200009ed = 0x80;
  *(uint16_t*)0x200009ee = 7;
  *(uint8_t*)0x200009f0 = 9;
  *(uint8_t*)0x200009f1 = 5;
  *(uint8_t*)0x200009f2 = 0xf;
  *(uint8_t*)0x200009f3 = 0;
  *(uint16_t*)0x200009f4 = 0x40;
  *(uint8_t*)0x200009f6 = 1;
  *(uint8_t*)0x200009f7 = 7;
  *(uint8_t*)0x200009f8 = 0x33;
  *(uint8_t*)0x200009f9 = 2;
  *(uint8_t*)0x200009fa = 2;
  *(uint8_t*)0x200009fb = 7;
  *(uint8_t*)0x200009fc = 0x25;
  *(uint8_t*)0x200009fd = 1;
  *(uint8_t*)0x200009fe = 0x82;
  *(uint8_t*)0x200009ff = 0x65;
  *(uint16_t*)0x20000a00 = 4;
  *(uint32_t*)0x20001440 = 0;
  *(uint64_t*)0x20001444 = 0;
  *(uint32_t*)0x2000144c = 0;
  *(uint64_t*)0x20001450 = 0;
  *(uint32_t*)0x20001458 = 2;
  *(uint32_t*)0x2000145c = 0;
  *(uint64_t*)0x20001460 = 0;
  *(uint32_t*)0x20001468 = 0;
  *(uint64_t*)0x2000146c = 0;
  syz_usb_connect(0, 0x282, 0x20000780, 0x20001440);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}