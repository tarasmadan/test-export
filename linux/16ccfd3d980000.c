// https://syzkaller.appspot.com/bug?id=0544e9d95c4fc424d257a27fdda32c4c685c611c
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

static struct usb_device_index* lookup_usb_index(int fd)
{
  for (int i = 0; i < USB_MAX_FDS; i++) {
    if (__atomic_load_n(&usb_devices[i].fd, __ATOMIC_ACQUIRE) == fd)
      return &usb_devices[i].index;
  }
  return NULL;
}

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
                           struct usb_qualifier_descriptor* qual,
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
          qual->bLength = sizeof(*qual);
          qual->bDescriptorType = USB_DT_DEVICE_QUALIFIER;
          qual->bcdUSB = index->dev->bcdUSB;
          qual->bDeviceClass = index->dev->bDeviceClass;
          qual->bDeviceSubClass = index->dev->bDeviceSubClass;
          qual->bDeviceProtocol = index->dev->bDeviceProtocol;
          qual->bMaxPacketSize0 = index->dev->bMaxPacketSize0;
          qual->bNumConfigurations = index->dev->bNumConfigurations;
          qual->bRESERVED = 0;
          *response_data = (char*)qual;
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

static int usb_raw_configure(int fd)
{
  return ioctl(fd, USB_RAW_IOCTL_CONFIGURE, 0);
}

static int usb_raw_vbus_draw(int fd, uint32_t power)
{
  return ioctl(fd, USB_RAW_IOCTL_VBUS_DRAW, power);
}

static int usb_raw_ep0_write(int fd, struct usb_raw_ep_io* io)
{
  return ioctl(fd, USB_RAW_IOCTL_EP0_WRITE, io);
}

static int usb_raw_ep0_read(int fd, struct usb_raw_ep_io* io)
{
  return ioctl(fd, USB_RAW_IOCTL_EP0_READ, io);
}

static int usb_raw_event_fetch(int fd, struct usb_raw_event* event)
{
  return ioctl(fd, USB_RAW_IOCTL_EVENT_FETCH, event);
}

static int usb_raw_ep_enable(int fd, struct usb_endpoint_descriptor* desc)
{
  return ioctl(fd, USB_RAW_IOCTL_EP_ENABLE, desc);
}

static int usb_raw_ep_disable(int fd, int ep)
{
  return ioctl(fd, USB_RAW_IOCTL_EP_DISABLE, ep);
}

static int usb_raw_ep0_stall(int fd)
{
  return ioctl(fd, USB_RAW_IOCTL_EP0_STALL, 0);
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
    struct usb_qualifier_descriptor qual;
    if (event.ctrl.bRequestType & USB_DIR_IN) {
      if (!lookup_connect_response_in(fd, descs, &event.ctrl, &qual,
                                      &response_data, &response_length)) {
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
      sleep_ms(10);
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      if (current_time_ms() - start < 5000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

void execute_one(void)
{
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  *(uint8_t*)0x20000100 = 0x12;
  *(uint8_t*)0x20000101 = 1;
  *(uint16_t*)0x20000102 = 0x300;
  *(uint8_t*)0x20000104 = 0x50;
  *(uint8_t*)0x20000105 = 0x8f;
  *(uint8_t*)0x20000106 = 0xa5;
  *(uint8_t*)0x20000107 = 8;
  *(uint16_t*)0x20000108 = 0x2ef5;
  *(uint16_t*)0x2000010a = 0xa;
  *(uint16_t*)0x2000010c = 0x21c2;
  *(uint8_t*)0x2000010e = 1;
  *(uint8_t*)0x2000010f = 2;
  *(uint8_t*)0x20000110 = 3;
  *(uint8_t*)0x20000111 = 1;
  *(uint8_t*)0x20000112 = 9;
  *(uint8_t*)0x20000113 = 2;
  *(uint16_t*)0x20000114 = 0x1ee;
  *(uint8_t*)0x20000116 = 3;
  *(uint8_t*)0x20000117 = 1;
  *(uint8_t*)0x20000118 = 9;
  *(uint8_t*)0x20000119 = 0xd0;
  *(uint8_t*)0x2000011a = 0x3f;
  *(uint8_t*)0x2000011b = 9;
  *(uint8_t*)0x2000011c = 4;
  *(uint8_t*)0x2000011d = 0xa7;
  *(uint8_t*)0x2000011e = 0xe4;
  *(uint8_t*)0x2000011f = 0xb;
  *(uint8_t*)0x20000120 = 0x99;
  *(uint8_t*)0x20000121 = 0x4f;
  *(uint8_t*)0x20000122 = 0x11;
  *(uint8_t*)0x20000123 = 3;
  *(uint8_t*)0x20000124 = 0xa;
  *(uint8_t*)0x20000125 = 0x24;
  *(uint8_t*)0x20000126 = 1;
  *(uint16_t*)0x20000127 = 6;
  *(uint8_t*)0x20000129 = 6;
  *(uint8_t*)0x2000012a = 2;
  *(uint8_t*)0x2000012b = 1;
  *(uint8_t*)0x2000012c = 2;
  *(uint8_t*)0x2000012d = 0xc;
  *(uint8_t*)0x2000012e = 0x24;
  *(uint8_t*)0x2000012f = 2;
  *(uint8_t*)0x20000130 = 4;
  *(uint16_t*)0x20000131 = 0x306;
  *(uint8_t*)0x20000133 = 4;
  *(uint8_t*)0x20000134 = 7;
  *(uint16_t*)0x20000135 = 1;
  *(uint8_t*)0x20000137 = -1;
  *(uint8_t*)0x20000138 = 0x18;
  *(uint8_t*)0x20000139 = 5;
  *(uint8_t*)0x2000013a = 0x24;
  *(uint8_t*)0x2000013b = 5;
  *(uint8_t*)0x2000013c = 4;
  *(uint8_t*)0x2000013d = 0x10;
  *(uint8_t*)0x2000013e = 0xc;
  *(uint8_t*)0x2000013f = 0x24;
  *(uint8_t*)0x20000140 = 2;
  *(uint8_t*)0x20000141 = 3;
  *(uint16_t*)0x20000142 = 0x708;
  *(uint8_t*)0x20000144 = 5;
  *(uint8_t*)0x20000145 = 0xf3;
  *(uint16_t*)0x20000146 = 0xfffd;
  *(uint8_t*)0x20000148 = 4;
  *(uint8_t*)0x20000149 = 3;
  *(uint8_t*)0x2000014a = 5;
  *(uint8_t*)0x2000014b = 0x24;
  *(uint8_t*)0x2000014c = 5;
  *(uint8_t*)0x2000014d = 1;
  *(uint8_t*)0x2000014e = 1;
  *(uint8_t*)0x2000014f = 0xc;
  *(uint8_t*)0x20000150 = 0x24;
  *(uint8_t*)0x20000151 = 2;
  *(uint8_t*)0x20000152 = 4;
  *(uint16_t*)0x20000153 = 0x203;
  *(uint8_t*)0x20000155 = 2;
  *(uint8_t*)0x20000156 = 2;
  *(uint16_t*)0x20000157 = 1;
  *(uint8_t*)0x20000159 = 0x64;
  *(uint8_t*)0x2000015a = 6;
  *(uint8_t*)0x2000015b = 7;
  *(uint8_t*)0x2000015c = 0x24;
  *(uint8_t*)0x2000015d = 8;
  *(uint8_t*)0x2000015e = 5;
  *(uint16_t*)0x2000015f = 4;
  *(uint8_t*)0x20000161 = 0;
  *(uint8_t*)0x20000162 = 9;
  *(uint8_t*)0x20000163 = 5;
  *(uint8_t*)0x20000164 = 7;
  *(uint8_t*)0x20000165 = 0;
  *(uint16_t*)0x20000166 = 0x40;
  *(uint8_t*)0x20000168 = 0;
  *(uint8_t*)0x20000169 = 0x7f;
  *(uint8_t*)0x2000016a = 0xf7;
  *(uint8_t*)0x2000016b = 7;
  *(uint8_t*)0x2000016c = 0x25;
  *(uint8_t*)0x2000016d = 1;
  *(uint8_t*)0x2000016e = 1;
  *(uint8_t*)0x2000016f = 0x80;
  *(uint16_t*)0x20000170 = 6;
  *(uint8_t*)0x20000172 = 9;
  *(uint8_t*)0x20000173 = 5;
  *(uint8_t*)0x20000174 = 0xe;
  *(uint8_t*)0x20000175 = 0x10;
  *(uint16_t*)0x20000176 = 0x208;
  *(uint8_t*)0x20000178 = 7;
  *(uint8_t*)0x20000179 = 6;
  *(uint8_t*)0x2000017a = 8;
  *(uint8_t*)0x2000017b = 2;
  *(uint8_t*)0x2000017c = 0xb;
  *(uint8_t*)0x2000017d = 9;
  *(uint8_t*)0x2000017e = 5;
  *(uint8_t*)0x2000017f = 3;
  *(uint8_t*)0x20000180 = 0;
  *(uint16_t*)0x20000181 = 0x20;
  *(uint8_t*)0x20000183 = 6;
  *(uint8_t*)0x20000184 = 0x80;
  *(uint8_t*)0x20000185 = 0xa;
  *(uint8_t*)0x20000186 = 2;
  *(uint8_t*)0x20000187 = 0x31;
  *(uint8_t*)0x20000188 = 9;
  *(uint8_t*)0x20000189 = 5;
  *(uint8_t*)0x2000018a = 0xc;
  *(uint8_t*)0x2000018b = 0;
  *(uint16_t*)0x2000018c = 0x40;
  *(uint8_t*)0x2000018e = 0;
  *(uint8_t*)0x2000018f = 8;
  *(uint8_t*)0x20000190 = 1;
  *(uint8_t*)0x20000191 = 9;
  *(uint8_t*)0x20000192 = 5;
  *(uint8_t*)0x20000193 = 2;
  *(uint8_t*)0x20000194 = 0;
  *(uint16_t*)0x20000195 = 0x200;
  *(uint8_t*)0x20000197 = 4;
  *(uint8_t*)0x20000198 = 0xd;
  *(uint8_t*)0x20000199 = 5;
  *(uint8_t*)0x2000019a = 7;
  *(uint8_t*)0x2000019b = 0x25;
  *(uint8_t*)0x2000019c = 1;
  *(uint8_t*)0x2000019d = 2;
  *(uint8_t*)0x2000019e = 0x7f;
  *(uint16_t*)0x2000019f = 1;
  *(uint8_t*)0x200001a1 = 7;
  *(uint8_t*)0x200001a2 = 0x25;
  *(uint8_t*)0x200001a3 = 1;
  *(uint8_t*)0x200001a4 = 2;
  *(uint8_t*)0x200001a5 = 0;
  *(uint16_t*)0x200001a6 = 4;
  *(uint8_t*)0x200001a8 = 9;
  *(uint8_t*)0x200001a9 = 5;
  *(uint8_t*)0x200001aa = 6;
  *(uint8_t*)0x200001ab = 0;
  *(uint16_t*)0x200001ac = 0x400;
  *(uint8_t*)0x200001ae = 5;
  *(uint8_t*)0x200001af = 0x81;
  *(uint8_t*)0x200001b0 = 2;
  *(uint8_t*)0x200001b1 = 2;
  *(uint8_t*)0x200001b2 = 0x31;
  *(uint8_t*)0x200001b3 = 2;
  *(uint8_t*)0x200001b4 = 0x10;
  *(uint8_t*)0x200001b5 = 9;
  *(uint8_t*)0x200001b6 = 5;
  *(uint8_t*)0x200001b7 = 0xe;
  *(uint8_t*)0x200001b8 = 0xc;
  *(uint16_t*)0x200001b9 = 0x3ff;
  *(uint8_t*)0x200001bb = 6;
  *(uint8_t*)0x200001bc = 6;
  *(uint8_t*)0x200001bd = 7;
  *(uint8_t*)0x200001be = 2;
  *(uint8_t*)0x200001bf = 0xa;
  *(uint8_t*)0x200001c0 = 7;
  *(uint8_t*)0x200001c1 = 0x25;
  *(uint8_t*)0x200001c2 = 1;
  *(uint8_t*)0x200001c3 = 0x80;
  *(uint8_t*)0x200001c4 = 0x15;
  *(uint16_t*)0x200001c5 = 0xfffd;
  *(uint8_t*)0x200001c7 = 9;
  *(uint8_t*)0x200001c8 = 5;
  *(uint8_t*)0x200001c9 = 9;
  *(uint8_t*)0x200001ca = 3;
  *(uint16_t*)0x200001cb = 8;
  *(uint8_t*)0x200001cd = 0xca;
  *(uint8_t*)0x200001ce = 0x80;
  *(uint8_t*)0x200001cf = 7;
  *(uint8_t*)0x200001d0 = 2;
  *(uint8_t*)0x200001d1 = 4;
  *(uint8_t*)0x200001d2 = 9;
  *(uint8_t*)0x200001d3 = 5;
  *(uint8_t*)0x200001d4 = 0x8e;
  *(uint8_t*)0x200001d5 = 0x10;
  *(uint16_t*)0x200001d6 = 8;
  *(uint8_t*)0x200001d8 = 6;
  *(uint8_t*)0x200001d9 = 5;
  *(uint8_t*)0x200001da = 1;
  *(uint8_t*)0x200001db = 2;
  *(uint8_t*)0x200001dc = 3;
  *(uint8_t*)0x200001dd = 9;
  *(uint8_t*)0x200001de = 5;
  *(uint8_t*)0x200001df = 0xe;
  *(uint8_t*)0x200001e0 = 0x10;
  *(uint16_t*)0x200001e1 = 0x40;
  *(uint8_t*)0x200001e3 = 9;
  *(uint8_t*)0x200001e4 = 0;
  *(uint8_t*)0x200001e5 = 0xe0;
  *(uint8_t*)0x200001e6 = 9;
  *(uint8_t*)0x200001e7 = 5;
  *(uint8_t*)0x200001e8 = 0;
  *(uint8_t*)0x200001e9 = 0;
  *(uint16_t*)0x200001ea = 0x200;
  *(uint8_t*)0x200001ec = 0x80;
  *(uint8_t*)0x200001ed = 0xf4;
  *(uint8_t*)0x200001ee = 4;
  *(uint8_t*)0x200001ef = 2;
  *(uint8_t*)0x200001f0 = 0xe;
  *(uint8_t*)0x200001f1 = 7;
  *(uint8_t*)0x200001f2 = 0x25;
  *(uint8_t*)0x200001f3 = 1;
  *(uint8_t*)0x200001f4 = 1;
  *(uint8_t*)0x200001f5 = 7;
  *(uint16_t*)0x200001f6 = 3;
  *(uint8_t*)0x200001f8 = 9;
  *(uint8_t*)0x200001f9 = 4;
  *(uint8_t*)0x200001fa = 0xd5;
  *(uint8_t*)0x200001fb = 0xfd;
  *(uint8_t*)0x200001fc = 9;
  *(uint8_t*)0x200001fd = 0x76;
  *(uint8_t*)0x200001fe = 0xa0;
  *(uint8_t*)0x200001ff = 0xbb;
  *(uint8_t*)0x20000200 = 8;
  *(uint8_t*)0x20000201 = 2;
  *(uint8_t*)0x20000202 = 0xd;
  *(uint8_t*)0x20000203 = 9;
  *(uint8_t*)0x20000204 = 5;
  *(uint8_t*)0x20000205 = 7;
  *(uint8_t*)0x20000206 = 0x10;
  *(uint16_t*)0x20000207 = 0x200;
  *(uint8_t*)0x20000209 = 7;
  *(uint8_t*)0x2000020a = 1;
  *(uint8_t*)0x2000020b = 0xa1;
  *(uint8_t*)0x2000020c = 2;
  *(uint8_t*)0x2000020d = 0x23;
  *(uint8_t*)0x2000020e = 9;
  *(uint8_t*)0x2000020f = 5;
  *(uint8_t*)0x20000210 = 8;
  *(uint8_t*)0x20000211 = 0;
  *(uint16_t*)0x20000212 = 0x20;
  *(uint8_t*)0x20000214 = 0xfe;
  *(uint8_t*)0x20000215 = 2;
  *(uint8_t*)0x20000216 = 0x7f;
  *(uint8_t*)0x20000217 = 2;
  *(uint8_t*)0x20000218 = 0x31;
  *(uint8_t*)0x20000219 = 7;
  *(uint8_t*)0x2000021a = 0x25;
  *(uint8_t*)0x2000021b = 1;
  *(uint8_t*)0x2000021c = 2;
  *(uint8_t*)0x2000021d = 2;
  *(uint16_t*)0x2000021e = -1;
  *(uint8_t*)0x20000220 = 9;
  *(uint8_t*)0x20000221 = 5;
  *(uint8_t*)0x20000222 = 0;
  *(uint8_t*)0x20000223 = 0;
  *(uint16_t*)0x20000224 = 0x40;
  *(uint8_t*)0x20000226 = 4;
  *(uint8_t*)0x20000227 = 9;
  *(uint8_t*)0x20000228 = 9;
  *(uint8_t*)0x20000229 = 9;
  *(uint8_t*)0x2000022a = 5;
  *(uint8_t*)0x2000022b = 9;
  *(uint8_t*)0x2000022c = 0x10;
  *(uint16_t*)0x2000022d = 0x7f7;
  *(uint8_t*)0x2000022f = 3;
  *(uint8_t*)0x20000230 = 7;
  *(uint8_t*)0x20000231 = 0;
  *(uint8_t*)0x20000232 = 9;
  *(uint8_t*)0x20000233 = 5;
  *(uint8_t*)0x20000234 = 0xd;
  *(uint8_t*)0x20000235 = 0x10;
  *(uint16_t*)0x20000236 = 0x200;
  *(uint8_t*)0x20000238 = 9;
  *(uint8_t*)0x20000239 = 9;
  *(uint8_t*)0x2000023a = 2;
  *(uint8_t*)0x2000023b = 9;
  *(uint8_t*)0x2000023c = 5;
  *(uint8_t*)0x2000023d = 0x80;
  *(uint8_t*)0x2000023e = 0x10;
  *(uint16_t*)0x2000023f = 8;
  *(uint8_t*)0x20000241 = 0x90;
  *(uint8_t*)0x20000242 = 0xe;
  *(uint8_t*)0x20000243 = 0x81;
  *(uint8_t*)0x20000244 = 9;
  *(uint8_t*)0x20000245 = 5;
  *(uint8_t*)0x20000246 = 5;
  *(uint8_t*)0x20000247 = 4;
  *(uint16_t*)0x20000248 = 0x200;
  *(uint8_t*)0x2000024a = 0x10;
  *(uint8_t*)0x2000024b = 0xf7;
  *(uint8_t*)0x2000024c = 0x3c;
  *(uint8_t*)0x2000024d = 2;
  *(uint8_t*)0x2000024e = 0x23;
  *(uint8_t*)0x2000024f = 2;
  *(uint8_t*)0x20000250 = 0xa;
  *(uint8_t*)0x20000251 = 9;
  *(uint8_t*)0x20000252 = 5;
  *(uint8_t*)0x20000253 = 0xf;
  *(uint8_t*)0x20000254 = 2;
  *(uint16_t*)0x20000255 = 0x20;
  *(uint8_t*)0x20000257 = 8;
  *(uint8_t*)0x20000258 = 4;
  *(uint8_t*)0x20000259 = 6;
  *(uint8_t*)0x2000025a = 7;
  *(uint8_t*)0x2000025b = 0x25;
  *(uint8_t*)0x2000025c = 1;
  *(uint8_t*)0x2000025d = 0x83;
  *(uint8_t*)0x2000025e = 0x91;
  *(uint16_t*)0x2000025f = 3;
  *(uint8_t*)0x20000261 = 9;
  *(uint8_t*)0x20000262 = 5;
  *(uint8_t*)0x20000263 = 8;
  *(uint8_t*)0x20000264 = 0xc;
  *(uint16_t*)0x20000265 = 0x20;
  *(uint8_t*)0x20000267 = 2;
  *(uint8_t*)0x20000268 = 0xfd;
  *(uint8_t*)0x20000269 = 9;
  *(uint8_t*)0x2000026a = 9;
  *(uint8_t*)0x2000026b = 4;
  *(uint8_t*)0x2000026c = 0xfb;
  *(uint8_t*)0x2000026d = 9;
  *(uint8_t*)0x2000026e = 0xb;
  *(uint8_t*)0x2000026f = 0xb5;
  *(uint8_t*)0x20000270 = 0x5b;
  *(uint8_t*)0x20000271 = 0x85;
  *(uint8_t*)0x20000272 = 0xf;
  *(uint8_t*)0x20000273 = 9;
  *(uint8_t*)0x20000274 = 5;
  *(uint8_t*)0x20000275 = 0;
  *(uint8_t*)0x20000276 = 2;
  *(uint16_t*)0x20000277 = 0x40;
  *(uint8_t*)0x20000279 = 8;
  *(uint8_t*)0x2000027a = 2;
  *(uint8_t*)0x2000027b = 6;
  *(uint8_t*)0x2000027c = 7;
  *(uint8_t*)0x2000027d = 0x25;
  *(uint8_t*)0x2000027e = 1;
  *(uint8_t*)0x2000027f = 0x80;
  *(uint8_t*)0x20000280 = 6;
  *(uint16_t*)0x20000281 = 0xc000;
  *(uint8_t*)0x20000283 = 9;
  *(uint8_t*)0x20000284 = 5;
  *(uint8_t*)0x20000285 = 0xa;
  *(uint8_t*)0x20000286 = 1;
  *(uint16_t*)0x20000287 = 0x200;
  *(uint8_t*)0x20000289 = 0x92;
  *(uint8_t*)0x2000028a = 8;
  *(uint8_t*)0x2000028b = 0x2f;
  *(uint8_t*)0x2000028c = 2;
  *(uint8_t*)0x2000028d = 0x31;
  *(uint8_t*)0x2000028e = 7;
  *(uint8_t*)0x2000028f = 0x25;
  *(uint8_t*)0x20000290 = 1;
  *(uint8_t*)0x20000291 = 3;
  *(uint8_t*)0x20000292 = 0x40;
  *(uint16_t*)0x20000293 = 3;
  *(uint8_t*)0x20000295 = 9;
  *(uint8_t*)0x20000296 = 5;
  *(uint8_t*)0x20000297 = 0xf;
  *(uint8_t*)0x20000298 = 4;
  *(uint16_t*)0x20000299 = 0x10;
  *(uint8_t*)0x2000029b = 0x83;
  *(uint8_t*)0x2000029c = 7;
  *(uint8_t*)0x2000029d = 4;
  *(uint8_t*)0x2000029e = 2;
  *(uint8_t*)0x2000029f = 0x23;
  *(uint8_t*)0x200002a0 = 9;
  *(uint8_t*)0x200002a1 = 5;
  *(uint8_t*)0x200002a2 = 9;
  *(uint8_t*)0x200002a3 = 0;
  *(uint16_t*)0x200002a4 = 0x20;
  *(uint8_t*)0x200002a6 = 1;
  *(uint8_t*)0x200002a7 = 0x7f;
  *(uint8_t*)0x200002a8 = 1;
  *(uint8_t*)0x200002a9 = 2;
  *(uint8_t*)0x200002aa = 0x21;
  *(uint8_t*)0x200002ab = 9;
  *(uint8_t*)0x200002ac = 5;
  *(uint8_t*)0x200002ad = 3;
  *(uint8_t*)0x200002ae = 1;
  *(uint16_t*)0x200002af = 0x3ff;
  *(uint8_t*)0x200002b1 = 0x36;
  *(uint8_t*)0x200002b2 = 2;
  *(uint8_t*)0x200002b3 = 4;
  *(uint8_t*)0x200002b4 = 2;
  *(uint8_t*)0x200002b5 = 0x31;
  *(uint8_t*)0x200002b6 = 9;
  *(uint8_t*)0x200002b7 = 5;
  *(uint8_t*)0x200002b8 = 6;
  *(uint8_t*)0x200002b9 = 0;
  *(uint16_t*)0x200002ba = 0x400;
  *(uint8_t*)0x200002bc = 3;
  *(uint8_t*)0x200002bd = 0x80;
  *(uint8_t*)0x200002be = 7;
  *(uint8_t*)0x200002bf = 9;
  *(uint8_t*)0x200002c0 = 5;
  *(uint8_t*)0x200002c1 = 2;
  *(uint8_t*)0x200002c2 = 1;
  *(uint16_t*)0x200002c3 = 8;
  *(uint8_t*)0x200002c5 = 0xc;
  *(uint8_t*)0x200002c6 = 0xe;
  *(uint8_t*)0x200002c7 = 0x7f;
  *(uint8_t*)0x200002c8 = 2;
  *(uint8_t*)0x200002c9 = 0x21;
  *(uint8_t*)0x200002ca = 2;
  *(uint8_t*)0x200002cb = 0xd;
  *(uint8_t*)0x200002cc = 9;
  *(uint8_t*)0x200002cd = 5;
  *(uint8_t*)0x200002ce = 0xf;
  *(uint8_t*)0x200002cf = 0xc;
  *(uint16_t*)0x200002d0 = 8;
  *(uint8_t*)0x200002d2 = 0xe2;
  *(uint8_t*)0x200002d3 = 0x40;
  *(uint8_t*)0x200002d4 = 0;
  *(uint8_t*)0x200002d5 = 2;
  *(uint8_t*)0x200002d6 = 0xa;
  *(uint8_t*)0x200002d7 = 9;
  *(uint8_t*)0x200002d8 = 5;
  *(uint8_t*)0x200002d9 = 0xc;
  *(uint8_t*)0x200002da = 2;
  *(uint16_t*)0x200002db = 0;
  *(uint8_t*)0x200002dd = 7;
  *(uint8_t*)0x200002de = 2;
  *(uint8_t*)0x200002df = 0x81;
  *(uint8_t*)0x200002e0 = 9;
  *(uint8_t*)0x200002e1 = 5;
  *(uint8_t*)0x200002e2 = 0xd;
  *(uint8_t*)0x200002e3 = 8;
  *(uint16_t*)0x200002e4 = 8;
  *(uint8_t*)0x200002e6 = 5;
  *(uint8_t*)0x200002e7 = 6;
  *(uint8_t*)0x200002e8 = 5;
  *(uint8_t*)0x200002e9 = 9;
  *(uint8_t*)0x200002ea = 5;
  *(uint8_t*)0x200002eb = 0xf;
  *(uint8_t*)0x200002ec = 0;
  *(uint16_t*)0x200002ed = 0x20;
  *(uint8_t*)0x200002ef = 1;
  *(uint8_t*)0x200002f0 = 0xe;
  *(uint8_t*)0x200002f1 = 8;
  *(uint8_t*)0x200002f2 = 7;
  *(uint8_t*)0x200002f3 = 0x25;
  *(uint8_t*)0x200002f4 = 1;
  *(uint8_t*)0x200002f5 = 0;
  *(uint8_t*)0x200002f6 = 1;
  *(uint16_t*)0x200002f7 = 2;
  *(uint8_t*)0x200002f9 = 7;
  *(uint8_t*)0x200002fa = 0x25;
  *(uint8_t*)0x200002fb = 1;
  *(uint8_t*)0x200002fc = 0x86;
  *(uint8_t*)0x200002fd = 8;
  *(uint16_t*)0x200002fe = 3;
  *(uint32_t*)0x20000fc0 = 0;
  *(uint64_t*)0x20000fc4 = 0;
  *(uint32_t*)0x20000fcc = 0;
  *(uint64_t*)0x20000fd0 = 0;
  *(uint32_t*)0x20000fd8 = 7;
  *(uint32_t*)0x20000fdc = 0;
  *(uint64_t*)0x20000fe0 = 0;
  *(uint32_t*)0x20000fe8 = 0;
  *(uint64_t*)0x20000fec = 0;
  *(uint32_t*)0x20000ff4 = 0;
  *(uint64_t*)0x20000ff8 = 0;
  *(uint32_t*)0x20001000 = 0;
  *(uint64_t*)0x20001004 = 0;
  *(uint32_t*)0x2000100c = 0;
  *(uint64_t*)0x20001010 = 0;
  *(uint32_t*)0x20001018 = 0;
  *(uint64_t*)0x2000101c = 0;
  *(uint32_t*)0x20001024 = 0;
  *(uint64_t*)0x20001028 = 0;
  syz_usb_connect(/*speed=USB_SPEED_SUPER*/ 5, /*dev_len=*/0x200,
                  /*dev=*/0x20000100, /*conn_descs=*/0x20000fc0);
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
  const char* reason;
  (void)reason;
  loop();
  return 0;
}