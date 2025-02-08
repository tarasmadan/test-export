// https://syzkaller.appspot.com/bug?id=abb72be36acb79641b6e99442774e2555ffdff41
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

struct vusb_descriptor {
  uint8_t req_type;
  uint8_t desc_type;
  uint32_t len;
  char data[0];
} __attribute__((packed));

struct vusb_descriptors {
  uint32_t len;
  struct vusb_descriptor* generic;
  struct vusb_descriptor* descs[0];
} __attribute__((packed));

struct vusb_response {
  uint8_t type;
  uint8_t req;
  uint32_t len;
  char data[0];
} __attribute__((packed));

struct vusb_responses {
  uint32_t len;
  struct vusb_response* generic;
  struct vusb_response* resps[0];
} __attribute__((packed));

static bool lookup_control_response(const struct vusb_descriptors* descs,
                                    const struct vusb_responses* resps,
                                    struct usb_ctrlrequest* ctrl,
                                    char** response_data,
                                    uint32_t* response_length)
{
  int descs_num = 0;
  int resps_num = 0;
  if (descs)
    descs_num = (descs->len - offsetof(struct vusb_descriptors, descs)) /
                sizeof(descs->descs[0]);
  if (resps)
    resps_num = (resps->len - offsetof(struct vusb_responses, resps)) /
                sizeof(resps->resps[0]);
  uint8_t req = ctrl->bRequest;
  uint8_t req_type = ctrl->bRequestType & USB_TYPE_MASK;
  uint8_t desc_type = ctrl->wValue >> 8;
  if (req == USB_REQ_GET_DESCRIPTOR) {
    int i;
    for (i = 0; i < descs_num; i++) {
      struct vusb_descriptor* desc = descs->descs[i];
      if (!desc)
        continue;
      if (desc->req_type == req_type && desc->desc_type == desc_type) {
        *response_length = desc->len;
        if (*response_length != 0)
          *response_data = &desc->data[0];
        else
          *response_data = NULL;
        return true;
      }
    }
    if (descs && descs->generic) {
      *response_data = &descs->generic->data[0];
      *response_length = descs->generic->len;
      return true;
    }
  } else {
    int i;
    for (i = 0; i < resps_num; i++) {
      struct vusb_response* resp = resps->resps[i];
      if (!resp)
        continue;
      if (resp->type == req_type && resp->req == req) {
        *response_length = resp->len;
        if (*response_length != 0)
          *response_data = &resp->data[0];
        else
          *response_data = NULL;
        return true;
      }
    }
    if (resps && resps->generic) {
      *response_data = &resps->generic->data[0];
      *response_length = resps->generic->len;
      return true;
    }
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

static int lookup_interface(int fd, uint8_t bInterfaceNumber,
                            uint8_t bAlternateSetting)
{
  struct usb_device_index* index = lookup_usb_index(fd);
  if (!index)
    return -1;
  for (int i = 0; i < index->ifaces_num; i++) {
    if (index->ifaces[i].bInterfaceNumber == bInterfaceNumber &&
        index->ifaces[i].bAlternateSetting == bAlternateSetting)
      return i;
  }
  return -1;
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

static volatile long syz_usb_control_io(volatile long a0, volatile long a1,
                                        volatile long a2)
{
  int fd = a0;
  const struct vusb_descriptors* descs = (const struct vusb_descriptors*)a1;
  const struct vusb_responses* resps = (const struct vusb_responses*)a2;
  struct usb_raw_control_event event;
  event.inner.type = 0;
  event.inner.length = USB_MAX_PACKET_SIZE;
  int rv = usb_raw_event_fetch(fd, (struct usb_raw_event*)&event);
  if (rv < 0) {
    return rv;
  }
  if (event.inner.type != USB_RAW_EVENT_CONTROL) {
    return -1;
  }
  char* response_data = NULL;
  uint32_t response_length = 0;
  if ((event.ctrl.bRequestType & USB_DIR_IN) && event.ctrl.wLength) {
    if (!lookup_control_response(descs, resps, &event.ctrl, &response_data,
                                 &response_length)) {
      usb_raw_ep0_stall(fd);
      return -1;
    }
  } else {
    if ((event.ctrl.bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD ||
        event.ctrl.bRequest == USB_REQ_SET_INTERFACE) {
      int iface_num = event.ctrl.wIndex;
      int alt_set = event.ctrl.wValue;
      int iface_index = lookup_interface(fd, iface_num, alt_set);
      if (iface_index < 0) {
      } else {
        set_interface(fd, iface_index);
      }
    }
    response_length = event.ctrl.wLength;
  }
  struct usb_raw_ep_io_data response;
  response.inner.ep = 0;
  response.inner.flags = 0;
  if (response_length > sizeof(response.data))
    response_length = 0;
  if (event.ctrl.wLength < response_length)
    response_length = event.ctrl.wLength;
  if ((event.ctrl.bRequestType & USB_DIR_IN) && !event.ctrl.wLength) {
    response_length = USB_MAX_PACKET_SIZE;
  }
  response.inner.length = response_length;
  if (response_data)
    memcpy(&response.data[0], response_data, response_length);
  else
    memset(&response.data[0], 0, response_length);
  if ((event.ctrl.bRequestType & USB_DIR_IN) && event.ctrl.wLength) {
    rv = usb_raw_ep0_write(fd, (struct usb_raw_ep_io*)&response);
  } else {
    rv = usb_raw_ep0_read(fd, (struct usb_raw_ep_io*)&response);
  }
  if (rv < 0) {
    return rv;
  }
  sleep_ms(200);
  return 0;
}

static volatile long syz_usb_disconnect(volatile long a0)
{
  int fd = a0;
  int rv = close(fd);
  sleep_ms(200);
  return rv;
}

static long syz_open_dev(volatile long a0, volatile long a1, volatile long a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
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

static void setup_sysctl()
{
  int cad_pid = fork();
  if (cad_pid < 0)
    exit(1);
  if (cad_pid == 0) {
    for (;;)
      sleep(100);
  }
  char tmppid[32];
  snprintf(tmppid, sizeof(tmppid), "%d", cad_pid);
  struct {
    const char* name;
    const char* data;
  } files[] = {
      {"/sys/kernel/debug/x86/nmi_longest_ns", "10000000000"},
      {"/proc/sys/kernel/hung_task_check_interval_secs", "20"},
      {"/proc/sys/net/core/bpf_jit_kallsyms", "1"},
      {"/proc/sys/net/core/bpf_jit_harden", "0"},
      {"/proc/sys/kernel/kptr_restrict", "0"},
      {"/proc/sys/kernel/softlockup_all_cpu_backtrace", "1"},
      {"/proc/sys/fs/mount-max", "100"},
      {"/proc/sys/vm/oom_dump_tasks", "0"},
      {"/proc/sys/debug/exception-trace", "0"},
      {"/proc/sys/kernel/printk", "7 4 1 3"},
      {"/proc/sys/kernel/keys/gc_delay", "1"},
      {"/proc/sys/vm/oom_kill_allocating_task", "1"},
      {"/proc/sys/kernel/ctrl-alt-del", "0"},
      {"/proc/sys/kernel/cad_pid", tmppid},
  };
  for (size_t i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
    if (!write_file(files[i].name, files[i].data)) {
    }
  }
  kill(cad_pid, SIGKILL);
  while (waitpid(cad_pid, NULL, 0) != cad_pid)
    ;
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  memcpy((void*)0x200000000100,
         "\x12\x01\x50\x02\x54\x30\x05\x10\xb0\x0d\x69\x68\xb3\x73\x01\x02\x03"
         "\x01\x09\x02\x12\x00\x01\x00\x00\x00\x00\x09\x04\xf9\x00\x00\xb5\xc5"
         "\x49\x00",
         36);
  *(uint32_t*)0x200000000500 = 0;
  *(uint64_t*)0x200000000504 = 0;
  *(uint32_t*)0x20000000050c = 0;
  *(uint64_t*)0x200000000510 = 0;
  *(uint32_t*)0x200000000518 = 0;
  res = -1;
  res = syz_usb_connect(/*speed=*/0, /*dev_len=*/0x24, /*dev=*/0x200000000100,
                        /*conn_descs=*/0x200000000500);
  if (res != -1)
    r[0] = res;
  syz_usb_disconnect(/*fd=*/r[0]);
  *(uint32_t*)0x200000001000 = 0;
  *(uint64_t*)0x200000001004 = 0;
  *(uint32_t*)0x20000000100c = 0;
  *(uint64_t*)0x200000001010 = 0;
  *(uint32_t*)0x200000001018 = 0;
  res = -1;
  res = syz_usb_connect(/*speed=*/0, /*dev_len=*/0x24, /*dev=*/0x200000000b40,
                        /*conn_descs=*/0x200000001000);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x2000000000c0 = 0x14;
  *(uint64_t*)0x2000000000c4 = 0x200000000000;
  *(uint8_t*)0x200000000000 = 0;
  *(uint8_t*)0x200000000001 = 4;
  *(uint32_t*)0x200000000002 = 0x5d;
  *(uint8_t*)0x200000000006 = 0x5d;
  *(uint8_t*)0x200000000007 = 0xd;
  memcpy((void*)0x200000000008,
         "\x53\x24\x6a\xfd\x68\x16\x98\xc1\xd8\x1f\x2e\xc8\x63\x24\xd0\x71\x20"
         "\x0c\xd9\xbb\x17\x0e\xa8\x91\xfc\xd6\xd9\x15\x09\x4b\x9b\xfa\xb2\x56"
         "\x6b\x37\xe9\x9d\x52\xdd\x23\xfb\xa1\x1b\x8c\xe3\x71\x5e\x27\xca\xb8"
         "\xe7\x92\xcf\x61\x67\x1e\x2f\xd4\x07\x7b\x19\xa8\x7c\xae\xa6\xbc\xb1"
         "\x88\x8b\xe4\x9f\x50\x01\xf5\xb0\x30\x3e\x89\x48\x11\xe8\x28\x7e\x61"
         "\x0f\x0e\xa7\x09\xc7\xab",
         91);
  *(uint64_t*)0x2000000000cc = 0x200000000140;
  *(uint8_t*)0x200000000140 = 0;
  *(uint8_t*)0x200000000141 = 3;
  *(uint32_t*)0x200000000142 = 4;
  *(uint8_t*)0x200000000146 = 4;
  *(uint8_t*)0x200000000147 = 3;
  *(uint16_t*)0x200000000148 = 0x439;
  *(uint32_t*)0x200000000540 = 0x34;
  *(uint64_t*)0x200000000544 = 0x2000000001c0;
  *(uint8_t*)0x2000000001c0 = 0x40;
  *(uint8_t*)0x2000000001c1 = 0xa;
  *(uint32_t*)0x2000000001c2 = 0xffffff43;
  memcpy(
      (void*)0x2000000001c6,
      "\xb9\x86\xce\x5c\x16\x94\x23\x65\x6d\x72\xcc\x94\xe8\xd7\x61\x9b\xd0\x2f"
      "\x66\x6a\x58\xe9\xa1\x0f\x73\x70\x9e\xfa\x51\x34\x90\x8e\x66\xc6\x9a\x64"
      "\x3b\xad\x8a\xa1\x14\x8c\xff\xdd\xdb\x5c\x8f\xc4\xa1\xde\x47\x84\xa7\xd7"
      "\x59\xa1\xff\x96\x9f\xdc\xca\xc3\xa9\xb9\xb2\xda\x50\x96\x16\xd3\x2d\x5c"
      "\xd4\xfc\x1c\x60\x8c\x33\x4f\x14\x4a\xd8\x3e\xa6\xb7\xcd\xd7\x1d\x22\x87"
      "\x28\x90\x92\xbf\x67\x95\xb4\x2c\x70\x68\xa4\xc4\x1b\xe1\xbb\x23\x76\x69"
      "\xf3\x94\x92\x57\xcd\xc5\xc9\x1c\x21\x4a\x41\x6e\xbc\x84\x29\x05\x0d\xb4"
      "\x62\x08\x76\xb2\x93\x49\x39\x44\xc9\x8a\x99\x11\xf0\xa6\x4d\x14\x3a\x64"
      "\xcb\xf5\x96\x7a\x36\x77\x33\xae\x68\x50\x1c\x10\x93\xcf\x12\xbe\x03\xf8"
      "\x53\xf4\x8a\xcd\xc1\x83\x0e\x42\x7e\xb7\x22\xa3\x67",
      175);
  *(uint64_t*)0x20000000054c = 0x2000000002c0;
  *(uint8_t*)0x2000000002c0 = 0;
  *(uint8_t*)0x2000000002c1 = 0xa;
  *(uint32_t*)0x2000000002c2 = 1;
  *(uint8_t*)0x2000000002c6 = 5;
  *(uint64_t*)0x200000000554 = 0x200000000300;
  *(uint8_t*)0x200000000300 = 0;
  *(uint8_t*)0x200000000301 = 8;
  *(uint32_t*)0x200000000302 = 1;
  *(uint8_t*)0x200000000306 = 7;
  *(uint64_t*)0x20000000055c = 0x200000000080;
  *(uint8_t*)0x200000000080 = 0x20;
  *(uint8_t*)0x200000000081 = 0;
  *(uint32_t*)0x200000000082 = 0x2e;
  *(uint16_t*)0x200000000086 = htobe16(0x2c);
  memcpy((void*)0x200000000088,
         "\xe8\x53\xf6\x6f\x63\x53\xd4\x0c\x90\x0e\x02\xbb\xeb\x94\x83\xf9\xc9"
         "\xc6\xfd\xe2\x4d\xe5\x10\x43\x41\x74\x75\x5f\x5e\xcf\xe6\x3d\x8c\x03"
         "\x00\x04\x2c\x74\x41\xbb\xd3\x0b\xcc\xb5",
         44);
  *(uint64_t*)0x200000000564 = 0x200000000480;
  *(uint8_t*)0x200000000480 = 0x20;
  *(uint8_t*)0x200000000481 = 1;
  *(uint32_t*)0x200000000482 = 1;
  *(uint8_t*)0x200000000486 = 0xb;
  *(uint64_t*)0x20000000056c = 0x2000000004c0;
  *(uint8_t*)0x2000000004c0 = 0x20;
  *(uint8_t*)0x2000000004c1 = 0;
  *(uint32_t*)0x2000000004c2 = 1;
  *(uint8_t*)0x2000000004c6 = 1;
  syz_usb_control_io(/*fd=*/r[1], /*descs=*/0x2000000000c0,
                     /*resps=*/0x200000000540);
  syz_usb_control_io(/*fd=*/r[1], /*descs=*/0, /*resps=*/0);
  syz_usb_control_io(/*fd=*/r[1], /*descs=*/0, /*resps=*/0);
  syz_usb_control_io(/*fd=*/r[0], /*descs=*/0, /*resps=*/0);
  *(uint32_t*)0x200000000ec0 = 0x84;
  *(uint64_t*)0x200000000ec4 = 0;
  *(uint64_t*)0x200000000ecc = 0;
  *(uint64_t*)0x200000000ed4 = 0;
  *(uint64_t*)0x200000000edc = 0;
  *(uint64_t*)0x200000000ee4 = 0;
  *(uint64_t*)0x200000000eec = 0x200000000a00;
  *(uint8_t*)0x200000000a00 = 0x40;
  *(uint8_t*)0x200000000a01 = 7;
  *(uint32_t*)0x200000000a02 = 2;
  *(uint16_t*)0x200000000a06 = 1;
  *(uint64_t*)0x200000000ef4 = 0;
  *(uint64_t*)0x200000000efc = 0;
  *(uint64_t*)0x200000000f04 = 0;
  *(uint64_t*)0x200000000f0c = 0;
  *(uint64_t*)0x200000000f14 = 0;
  *(uint64_t*)0x200000000f1c = 0;
  *(uint64_t*)0x200000000f24 = 0;
  *(uint64_t*)0x200000000f2c = 0;
  *(uint64_t*)0x200000000f34 = 0;
  *(uint64_t*)0x200000000f3c = 0;
  syz_usb_control_io(/*fd=*/r[1], /*descs=*/0, /*resps=*/0x200000000ec0);
  res = -1;
  res = syz_usb_connect(/*speed=USB_SPEED_FULL*/ 2, /*dev_len=*/0, /*dev=*/0,
                        /*conn_descs=*/0);
  if (res != -1)
    r[2] = res;
  syz_usb_disconnect(/*fd=*/-1);
  syz_usb_connect(/*speed=*/0, /*dev_len=*/0, /*dev=*/0, /*conn_descs=*/0);
  syz_usb_control_io(/*fd=*/-1, /*descs=*/0, /*resps=*/0);
  syz_usb_disconnect(/*fd=*/-1);
  syz_open_dev(/*dev=*/0xc, /*major=*/0xb4, /*minor=*/0);
  syz_usb_control_io(/*fd=*/r[2], /*descs=*/0, /*resps=*/0xffffffffff600000);
  syz_usb_disconnect(/*fd=*/-1);
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
  setup_sysctl();
  const char* reason;
  (void)reason;
  for (procid = 0; procid < 5; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
