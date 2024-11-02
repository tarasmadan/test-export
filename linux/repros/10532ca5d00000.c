// https://syzkaller.appspot.com/bug?id=a3de6f3ba49d31ec1a48d451200c902fda8f1a7b
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
  memcpy(
      (void*)0x20000800,
      "\x12\x01\x00\x00\x08\x2c\xbe\x08\xaa\x07\x04\x00\x8b\x6e\x01\x02\x03\x01"
      "\x09\x02\x81\x07\x02\x00\xff\x60\x02\x09\x04\xac\x09\x03\xc9\xc3\x26\x80"
      "\x8d\x02\x07\x10\x34\xb0\x52\x56\xaa\xb1\xcd\x09\x3c\x4f\xdd\x6f\x1c\x26"
      "\xdc\x5a\x2a\x1f\x9c\x13\xa9\x7b\xdd\xa1\x6a\x01\xe8\x42\x74\xd9\xd3\x38"
      "\x60\x9f\xe1\x3e\xb5\xf6\xab\xa7\x6b\x3a\xc1\x1c\x65\x1a\xf7\x9d\xc7\xcd"
      "\xf7\x4c\xd2\x47\x61\x66\xee\x11\x10\xcd\x17\xe0\xfa\x95\x9a\xe6\x59\x1f"
      "\x79\x24\xc1\x1b\x2d\x3d\xca\x13\x98\xd7\x0c\xd6\xff\x12\x89\x2c\x9f\xb2"
      "\x55\x72\x05\x54\x46\x73\xbd\x43\x1f\x87\x11\x26\xc4\xdd\xe4\x53\xdb\x98"
      "\xe4\xbc\x0f\xa8\x82\x80\x6f\xdd\xec\xf0\xe8\xf0\x7f\x71\xc5\xc1\x69\xdc"
      "\xdc\xfc\x52\xd0\xdb\xfa\x8e\x7e\xf9\x30\x4b\x6c\xce\xcf\x5c\x09\x21\x01"
      "\x80\x02\x01\x22\x2a\x0b\x09\x05\x04\x03\x2e\x92\x01\x04\x02\x07\x25\x01"
      "\x02\x00\x02\x00\x09\x05\x0d\x10\x00\x02\x00\x80\x02\x07\x25\x01\x00\x07"
      "\x07\x00\x09\x05\x08\x00\x00\x02\x03\x04\x05\x09\x04\xc2\x40\x10\x24\xd1"
      "\x4b\x06\x08\x24\x06\x00\x01\x1f\x4b\xe1\x05\x24\x00\xff\x00\x0d\x24\x0f"
      "\x01\x01\x04\x00\x00\x05\x00\xff\x00\x09\x06\x24\x1a\x00\x00\x02\x05\x24"
      "\x15\xe7\x08\x08\x24\x1c\x07\x00\x81\x18\x00\x07\x24\x0a\x20\x20\x06\x03"
      "\x0e\x24\x07\x03\x05\x00\x40\x00\x04\x00\x04\x00\x06\x00\x05\x24\x01\x01"
      "\x80\x09\x21\x07\x00\x00\x01\x22\xf2\x0d\x09\x05\x0a\x01\x00\x00\x7f\xc4"
      "\x01\x40\x08\x3d\x76\xb1\xde\x37\x46\x43\xd8\x60\xd0\x8a\x12\x50\x2c\x77"
      "\xda\xa0\xdd\x56\xd9\x2a\xb1\xe8\x4c\x70\xcc\x2b\xdc\x54\xba\x00\x86\xba"
      "\xad\x84\x44\x73\x93\xd0\x7d\x84\x83\x63\x6d\x38\x0b\xff\x79\xc5\xb5\x5b"
      "\xaf\xca\xed\x27\x22\x41\x15\xeb\x68\x18\x55\xa8\x05\xa4\xe6\xd0\xa3\x61"
      "\xdf\x31\xa9\x87\x24\xf3\xa4\x24\xb7\x77\x77\xd5\x6d\xb0\xc5\xcb\x2e\xb2"
      "\x4b\x1a\xe0\xa8\x10\x20\x34\x00\xed\xc3\xcf\x4b\x1b\x52\x3f\x7a\x05\xb4"
      "\xb4\x79\xcc\x89\x63\x82\x08\x0a\xee\x4a\xa4\x41\x2f\x43\xab\x54\xc3\xd0"
      "\x4a\xff\xa0\x36\x45\x8e\x9e\xfc\x92\xb2\xc5\xf3\xa0\x7a\x92\x39\x95\x11"
      "\x9b\xea\x2b\xb0\x12\xf6\xb4\x61\xe8\xe7\x4a\xcf\xe2\x0c\x09\xb7\x5d\xc4"
      "\x1c\x27\xac\x6d\xe3\xf2\x9b\x4f\x8e\x5f\x4a\x6b\x70\x48\x8e\xf4\x88\x91"
      "\x07\xaa\x6b\x7f\xbf\xb5\x99\x9e\xf4\x24\x80\x0c\x4f\x68\x3c\x11\x86\xfe"
      "\x76\x07\x6c\x9b\x64\x3e\x60\x1b\x1e\x3c\xe1\x59\xc7\x6e\xbc\xef\x29\xee"
      "\x3d\x84\x87\x0f\xa9\xe8\x9f\xa1\x58\x6c\x27\xb9\x39\x33\x5a\xc7\x68\xa2"
      "\x53\xcf\x09\x05\x0f\x10\x40\x00\x03\xfd\x86\x09\x05\x05\x01\x10\x00\x01"
      "\x02\x01\xa6\x0f\x25\x94\x5f\x14\xc5\xfe\xc0\x98\x5c\x12\xee\x86\x05\xe0"
      "\x1d\x8e\xb6\x68\xcb\x3d\x2e\x49\x82\xeb\xc0\x56\x64\xdd\xe0\x23\x3e\x1a"
      "\x15\x76\x22\x26\xcb\x32\x1d\xe3\xe1\xca\x6b\x37\xbc\x4e\xb2\xbf\x40\x2e"
      "\xd2\xef\x62\x4d\xdb\xcd\x39\x69\xdb\x3f\x5d\xea\xe9\x08\x5e\x3d\x0c\x10"
      "\xab\xd5\xb5\x6b\x2c\xec\x5b\x7f\x1c\x59\x54\x16\x86\x02\x9e\xc3\x3f\x7f"
      "\x7c\xe2\xe3\x07\xbd\xe2\x1f\x36\xb7\x36\xd9\xa0\x9b\x81\xb8\x81\x2e\x8c"
      "\xcf\x05\x56\xe8\xf9\xad\x59\x4c\x58\x36\xa8\x48\x5e\x6a\x0e\x9b\x84\x08"
      "\x15\xff\x43\xbb\x5f\xd7\x3e\xd0\xb6\x5e\x51\x1d\xe1\x47\xd0\x6c\x8c\x1d"
      "\x29\x82\x99\x1b\xce\x9f\x5d\x58\x86\x17\x1b\x3b\xe3\xf5\x61\xe1\xe1\x38"
      "\x77\xc2\xd6\xab\xcf\xb3\x99\x0d\xf7\x86\xbd\x4e\xa3\x19\xcc\xf1\x9d\x74"
      "\x1d\x17\xd4\x13\xd6\x8e\x61\xc2\x65\x19\x4e\x2e\x95\x94\x30\x38\x6a\xc3"
      "\x9c\x4f\x02\x09\x9f\xbb\xbc\xec\xa8\x85\xa7\xbd\x07\xbf\x08\x2b\xdc\x35"
      "\xc0\x47\x4a\x42\x9b\x4d\x41\x6f\x69\x1e\x0e\x27\xdd\xf8\xaa\x43\x12\x48"
      "\x10\x53\x2a\x6d\x2b\x91\x07\x9b\xd3\xde\xa1\xe3\xfb\x09\x78\x0f\x03\xab"
      "\x2f\xf5\x80\x55\x47\x04\x6e\xc0\x7f\xe3\x4a\x76\xe5\x57\x8f\xc4\x63\xfd"
      "\x06\xaf\x77\x1a\x4e\xaf\xb1\x42\x14\xba\x7d\x88\xd2\xfa\x5a\xd4\x3f\x26"
      "\xfc\x00\x3b\x24\xa6\xa2\x22\x18\xc6\x56\x66\xf5\xb6\xe2\x58\x8b\x27\x01"
      "\x5d\x12\x75\xf6\x44\xb2\x55\x7d\x88\xe8\xc6\x8d\x60\x1d\x85\x09\x05\x00"
      "\x02\x20\x00\x02\x05\x80\x51\x0e\xa7\x9e\x23\xf9\xf3\x25\xfa\x74\xf0\x21"
      "\x93\x06\xa8\x52\xc0\x75\x25\x14\x5d\x73\x67\x36\x1a\xa3\x35\x7e\x3f\xfc"
      "\x5a\x5f\xbf\xd5\x40\x7b\x4c\x49\xd4\x43\xb1\x8e\xd5\x40\x80\x4e\xf0\xea"
      "\x5e\x3e\xe0\xb0\x92\xeb\xbe\xcb\x98\x2f\x37\x4d\x94\x9b\x29\xc0\xdb\x8e"
      "\x21\x0a\x86\x87\x36\x91\x24\x8d\x8d\x3b\x92\x21\xab\x70\xe9\x09\x05\x0f"
      "\x00\xff\x03\x80\x07\x3c\x07\x25\x01\x80\x00\x02\x00\x61\x10\x5f\x3c\x6f"
      "\xc3\xa7\x9c\xa9\xab\x7b\xa5\x0d\x9d\xe5\x65\x46\xa7\x50\xa9\x2c\x71\x66"
      "\x4e\x85\x59\x75\x57\xc2\x08\x90\x20\x80\x90\x88\xb8\xeb\x4a\xd1\x4e\xec"
      "\x7d\x01\x3d\xa2\x1a\x47\x4c\xec\xca\xb5\xa3\xe4\x34\x01\xb5\x9d\x4a\x80"
      "\x98\x66\x7e\xb8\x9e\x29\xfc\xa4\x32\xc7\x71\x54\xfa\xce\x82\x80\x1d\x29"
      "\x99\x74\x88\x1a\xda\x12\x24\xec\x5d\x19\x61\x90\x90\x56\xd8\xc5\xa4\x24"
      "\x9c\x74\x09\x05\x06\x00\x20\x00\x01\x09\x05\x09\x05\x04\x10\x10\x00\x06"
      "\x07\x01\x09\x05\x04\x10\x40\x00\x1f\x06\x80\x46\x21\xc0\x13\xaa\x0d\x9d"
      "\xed\x6b\xdb\x24\xec\x45\xbe\x4f\x33\x5f\x68\x40\x57\x71\x05\x11\x43\x69"
      "\x56\xc7\xdc\xc3\x67\xdb\x2e\xa2\x63\xf1\x17\x5f\xf6\x67\x80\xf4\xe2\x7f"
      "\x7d\xe4\x59\x86\x30\x81\x89\xd2\xea\x64\xa8\x39\x23\x88\xb8\x7a\xd5\x83"
      "\xd0\x17\xdc\xb5\x36\x10\x39\x94\x2f\x07\x25\x01\x01\x3f\xfb\x48\x09\x05"
      "\x04\x02\x00\x04\x08\x01\xc1\xdc\x31\xae\x39\xa7\x65\xb7\x96\x4f\xf5\x69"
      "\x70\x60\xd0\xdc\x86\x5d\x5b\x52\x40\xa0\x92\x5b\xed\x5f\x70\x47\xe6\xcd"
      "\x14\xb3\xc3\x64\xec\x91\xa7\xfb\x22\x44\xb7\x9a\x62\xeb\x65\x4f\x59\x89"
      "\xab\x68\x2b\x63\x35\x7a\xa8\xa0\x2e\x15\x83\xe2\xa4\x12\x27\x52\x49\x76"
      "\xb9\xf4\x4a\x97\xcd\x78\xd4\x77\xe8\x42\x6c\x24\xf4\x14\xbf\x7b\x3d\x6f"
      "\x37\x01\x87\xed\xbc\xc1\x66\x09\x4f\x04\xa9\xf5\x34\x32\xa7\xb5\xb0\x3e"
      "\xb3\x3a\x80\xbb\xde\x3d\xc9\xff\x1b\xe7\x91\xc0\xb7\x8a\x85\xf9\xd3\x9d"
      "\xb9\x34\x46\x32\x0c\x4b\x5a\xc3\x94\x31\xbf\x06\xd0\xe3\xc3\xe0\x7f\x7a"
      "\xde\x1c\x48\x19\x0a\x5b\x3f\x41\xdd\xe4\xdb\x65\x46\x9a\xbd\xed\xb7\x23"
      "\x03\x7d\xb9\x21\xfa\xac\x81\xed\x95\xe8\xf6\x95\xfe\x05\x68\x64\xca\xb8"
      "\xfe\x57\x6d\x83\x80\x62\xd9\x17\x2c\xbc\x6a\xce\xb7\x0f\x85\xb5\xfc\x24"
      "\x56\xe1\x90\x41\x5e\x12\x66\xb0\xf2\x0d\x67\x3a\x47\x50\x16\x03\x1f\x08"
      "\xc7\x14\x75\xf0\xea\x26\x1b\x42\x87\x35\x9b\xb3\x10\x6f\xeb\x73\x95\xb4"
      "\xde\x4d\xd7\xdd\xb6\xf8\xe6\x98\x3e\x25\xee\xc4\xba\xfe\xdc\x45\x0b\x3b"
      "\xc9\x64\x24\x04\x42\x99\x38\xd8\xb7\x02\x4e\xac\x83\x54\xbf\x0d\xa5\xe8"
      "\x65\x18\xb5\x85\x9e\xdd\x65\x2b\xf0\xe5\xd6\xd9\xfb\x99\xbf\x95\x38\xd8"
      "\xed\x4d\x98\x8b\x66\x27\xf5\x54\x93\x20\x04\x6a\x28\x7d\xed\xbc\xbf\xac"
      "\xb7\xaa\x75\xb1\x5c\x6f\xdf\x92\x9e\x45\xe0\x87\x8a\x5f\xeb\x0c\x18\xd7"
      "\xb5\xa8\x0e\x04\xde\x83\x26\x3d\x0b\xc8\x21\xbe\x9e\x3c\xce\xef\xe3\x75"
      "\x3f\xc6\x78\xf6\x96\x65\xde\x19\x65\x9e\x2c\x8c\x26\xa0\x26\xaa\xb2\x0a"
      "\xfc\xd6\xd6\xb5\xb9\x26\x16\xb8\x66\x99\xd0\xc1\x58\xa4\xc3\xb7\xb2\xc5"
      "\xde\x49\xb2\x3b\x54\x5b\xcd\x03\x29\x26\x19\x7c\x77\x21\x17\x90\x50\x2a"
      "\x1e\x70\x11\x99\x2d\x29\x64\xdf\x3e\x89\x78\x19\xbc\xd6\xf0\xff\x09\x05"
      "\x07\x01\x00\x02\x1f\x03\x80\xbc\x22\xbe\x96\x0d\x9a\xf6\x04\x40\x32\x1e"
      "\x0b\x85\x15\xf2\x3c\xdc\xe8\x2b\xa9\x36\x43\x9e\x3e\xdc\x01\x8c\x4c\xd8"
      "\x7a\x3e\x96\xd5\xde\x84\x94\x8f\x7f\xa4\x87\x27\x20\xfe\x87\x14\x86\x1d"
      "\xac\x21\xe1\x16\x8f\x11\xe3\x38\xb6\xa4\x54\x75\xfd\xb7\xff\x0a\xa6\x74"
      "\x02\x8c\xca\xf2\x5d\x61\xcd\x5d\x6a\x32\x31\xe5\x19\xe7\xc2\x16\x1d\x32"
      "\x3d\x33\xd0\x35\x55\x3e\x24\xb6\xde\x0c\xed\xd0\x30\x5a\x21\xad\x7c\x08"
      "\xa6\x1c\xe7\x12\x8f\xc9\x42\x52\xd5\x44\x0e\x3f\xe1\x75\x0d\xdc\x66\xa8"
      "\xe0\x01\xf2\x78\xa5\xca\x60\x6d\x57\x60\xe8\x43\xc9\x1d\xb9\xa7\xf3\xfa"
      "\xd8\x5d\xfc\xf8\xff\xff\xc2\x73\x92\xec\x36\xc4\x86\xfa\xc7\xf7\x01\x1a"
      "\xaf\xbf\x31\xb1\xe9\x5a\xd4\x88\xe3\x59\xc0\x74\xb1\x9e\xfc\x1d\x75\x68"
      "\x5a\x08\x9d\xec\xf7\x7d\xad\x07\x0d\x5c\x3d\x76\x68\x11\xa7\x09\x05\x0c"
      "\x08\x08\x00\x1f\x01\x05\x09\x05\x06\x00\xc7\x01\x09\x80\x00\x48\x03\xa2"
      "\xb2\xb8\x40\x50\x57\xc4\x25\x7c\xfe\x14\xcf\xd7\x3f\xcb\x48\xff\x88\x36"
      "\xab\xbf\xda\xf5\x4a\x6f\x21\x9f\xe2\x0b\xc7\xb7\x97\x87\xaf\xe0\x39\xc3"
      "\xd6\x6d\x63\x80\xec\x6d\xbb\xbb\x75\xcb\xaa\x21\x29\xdb\x03\x11\x3c\x92"
      "\xd1\xcd\x9e\x9b\xe9\x73\x02\x20\x63\xd4\x5f\x72\x59\xfa\x4d\x07\x25\x01"
      "\x02\x02\x00\x00\x09\x05\x0a\x00\xff\x03\x00\x40\xc2\x09\x05\x05\x10\x00"
      "\x04\x01\x09\x81\x09\x05\x86\x10\x10\x00\x40\x3f\x7f\x09\x05\x04",
      1942);
  syz_usb_connect(2, 0x793, 0x20000800, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}