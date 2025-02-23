// https://syzkaller.appspot.com/bug?id=82ee167a55d45618bcbf303528c26d3c44b06559
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

#ifndef __NR_mmap
#define __NR_mmap 222
#endif

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

static int usb_raw_ep_write(int fd, struct usb_raw_ep_io* io)
{
  return ioctl(fd, USB_RAW_IOCTL_EP_WRITE, io);
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

static int lookup_endpoint(int fd, uint8_t bEndpointAddress)
{
  struct usb_device_index* index = lookup_usb_index(fd);
  if (!index)
    return -1;
  if (index->iface_cur < 0)
    return -1;
  for (int ep = 0; ep < index->ifaces[index->iface_cur].eps_num; ep++)
    if (index->ifaces[index->iface_cur].eps[ep].desc.bEndpointAddress ==
        bEndpointAddress)
      return index->ifaces[index->iface_cur].eps[ep].handle;
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

static volatile long syz_usb_ep_write(volatile long a0, volatile long a1,
                                      volatile long a2, volatile long a3)
{
  int fd = a0;
  uint8_t ep = a1;
  uint32_t len = a2;
  char* data = (char*)a3;
  int ep_handle = lookup_endpoint(fd, ep);
  if (ep_handle < 0) {
    return -1;
  }
  struct usb_raw_ep_io_data io_data;
  io_data.inner.ep = ep_handle;
  io_data.inner.flags = 0;
  if (len > sizeof(io_data.data))
    len = sizeof(io_data.data);
  io_data.inner.length = len;
  memcpy(&io_data.data[0], data, len);
  int rv = usb_raw_ep_write(fd, (struct usb_raw_ep_io*)&io_data);
  if (rv < 0) {
    return rv;
  }
  sleep_ms(200);
  return 0;
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
      if (current_time_ms() - start < 15000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  memcpy((void*)0x20000000,
         "\x12\x01\x00\x00\x00\x00\x00\x40\x26\x09\x33\x33\x40\x00\x00\x00\x00"
         "\x01\x09\x02\x24\x00\x01\x00\x00\x00\x00\x09\x04\x00\x00\x01\x03\x01"
         "\x00\x00\x09\x21\x00\x00\x00\x01\x22\x01\x00\x09\x05\x81\x03\x08",
         50);
  res = -1;
  res = syz_usb_connect(/*speed=*/0, /*dev_len=*/0x36, /*dev=*/0x20000000,
                        /*conn_descs=*/0);
  if (res != -1)
    r[0] = res;
  syz_usb_control_io(/*fd=*/r[0], /*descs=*/0, /*resps=*/0);
  *(uint16_t*)0x20000880 = 0;
  *(uint16_t*)0x20000882 = 0x4e00;
  *(uint16_t*)0x20000884 = 0xc1;
  *(uint16_t*)0x20000886 = 0x4e00;
  memcpy(
      (void*)0x20000888,
      "\xa7\x6c\xcb\x9e\xff\x46\x94\x98\x3d\xf6\x11\xac\x03\xc6\xc2\x3d\xb1\xe3"
      "\xb8\xa3\x09\x04\xe4\x11\x01\xcd\xd2\x1d\x87\x99\x82\x69\x64\xaa\x88\x86"
      "\xe9\x36\xf6\x6f\x48\xc2\x39\x37\x91\x40\x22\x4e\xad\x55\x29\xbe\x60\xe4"
      "\xe1\x03\xee\x38\xd7\x53\x04\x3d\xae\x6f\xf2\xa1\x08\x34\x31\x38\x51\x1e"
      "\x17\x8a\x6f\xbf\xe3\xbe\x80\x2b\xaf\x1d\x00\x7c\xa1\x75\x28\xd3\x26\x30"
      "\x1c\x1a\x60\xaf\x74\xa5\xe3\x17\x28\x24\x52\x62\xab\x41\x92\xb1\xd4\x75"
      "\x7e\xc2\xfb\x54\x91\x5f\x77\x91\xbf\xfa\x73\x4d\xae\xea\x15\x8a\xcb\x4f"
      "\xb0\x3e\x46\xee\xa1\x93\xd2\xb4\x6d\x98\x75\x6c\x84\xcc\x0e\x39\x18\x61"
      "\x8c\x11\x49\x1b\x37\xf7\x2d\x16\x2d\x85\x3d\x34\x5d\x61\x88\x71\x42\x4f"
      "\xfc\x80\x0e\xda\xb5\x55\xb0\xbc\x60\x61\x48\xfb\x32\x9c\x5d\x90\x5b\x80"
      "\xdf\x39\x04\xba\x01\xe9\x0a\xc4\x63\xf9\xc3\xd9\x10",
      193);
  *(uint16_t*)0x2000094c = 0xdb;
  *(uint16_t*)0x2000094e = 0x4e00;
  memcpy((void*)0x20000950,
         "\x6b\xe3\x5c\xba\xdd\x41\x62\xa5\x69\xc2\xc7\x6b\x2d\xd6\x75\x50\x24"
         "\xd1\x11\x34\x6a\xd9\x14\x19\x36\x26\xdf\xea\xb8\x59\x01\x7c\x21\xde"
         "\xfb\xe1\x01\xf6\xab\xfa\x76\x2a\xa3\x7a\xff\x45\x68\x72\x0b\xdf\xf2"
         "\xac\x99\x8a\x07\xd8\xfd\xb7\x64\xee\x24\x3e\x16\x6d\xb6\x25\xae\x10"
         "\x4b\x3d\x70\x08\x28\x6e\xc6\xd3\xba\xc1\x14\x72\xe0\x08\x70\x1d\x0b"
         "\x14\xd4\x72\xff\x87\x1c\x10\xa0\x36\x32\xe0\x91\x5a\x63\xf9\xb6\x4b"
         "\x71\x47\xd9\xe1\xd5\x88\x04\x8c\x5e\x07\xda\xc9\xc4\xca\xfb\x0d\x8b"
         "\xd1\x3f\xa6\x93\xdf\x56\x4e\x62\xaa\x3a\xc6\x1f\x5f\x8f\xbf\xbe\xe5"
         "\xe7\x9e\xb2\x0a\x26\xd1\xc0\x2c\x7b\x4f\x1a\x60\xe3\xa8\x06\x07\x86"
         "\xab\x1c\x9c\xdc\x0e\x06\x08\x91\x64\xfd\xa6\x3e\x28\x79\xe8\xff\x4e"
         "\x23\x31\xb3\x39\xbf\x01\xf6\xcd\x2f\x48\x2e\x64\xc3\x76\x98\x42\x3f"
         "\xb5\x85\x71\xe7\xf7\x67\xf7\xd3\x49\x26\xb9\x3c\x6c\x4f\x07\xd5\x56"
         "\x7a\x38\x78\x9b\x32\x7a\xfd\xad\xe7\x3f\x55\x17\xf8\xb6\x03",
         219);
  *(uint16_t*)0x20000a2c = 0xd6;
  *(uint16_t*)0x20000a2e = 0x4e00;
  memcpy(
      (void*)0x20000a30,
      "\x75\xfb\x4c\x30\x8d\x67\x0b\xfe\xd5\xd2\x33\xee\xd8\xd6\xe1\xb2\x2b\x9d"
      "\xe2\x75\xbf\x97\xe7\x45\xcb\xc7\x50\xd4\x42\x20\x2b\x41\x38\xe8\xc1\x9e"
      "\x60\x98\xab\xa4\x1a\x8e\xc2\x08\x71\x8e\x2d\xf1\x66\x76\x30\x22\xfd\x4c"
      "\x49\x30\xa5\x76\x00\xf9\x30\xbe\xc7\x12\xfa\xef\xbb\x5a\x08\x3f\xf9\x2b"
      "\xcf\xb0\x39\xc9\xc1\x64\x45\xf9\xd9\x1d\x9c\x23\xef\x84\x49\xc3\x70\x1b"
      "\xcc\x93\xd7\xa2\xac\x5e\x58\xc8\x36\x42\x51\xef\x80\xa1\xdc\x9c\xc6\xb1"
      "\x22\x88\x1c\x8e\xa6\x7c\x52\xa4\x4e\x4f\x68\x61\xd9\x20\xf6\xf4\xa1\x66"
      "\x0d\x04\xb1\xe3\xb1\xe7\x66\x83\x75\x26\x02\x04\x67\xff\x3c\x53\x28\xcb"
      "\xe2\x7e\x9c\x25\xe7\xa5\x07\x78\xe7\xfb\x9b\x27\xc9\xbe\x7b\xd9\x1a\x2c"
      "\xda\xc7\x09\xec\x83\xdd\x7c\xd7\xa5\xdc\x92\x06\x94\xf4\x2f\x2f\x3c\x01"
      "\x1c\x71\x77\xad\xb9\x6c\x55\x87\x93\xcb\x74\xb1\x93\x9c\x5e\x2c\xc1\xe3"
      "\xb2\x9e\xd9\xba\xb2\x0b\x4d\x6d\x59\x4d\xbf\xc6\xa9\x11\x96\x49",
      214);
  *(uint16_t*)0x20000b08 = 0;
  *(uint16_t*)0x20000b0a = 0x4e00;
  *(uint16_t*)0x20000b0c = 0;
  *(uint16_t*)0x20000b0e = 0x4e00;
  *(uint16_t*)0x20000b10 = 0;
  *(uint16_t*)0x20000b12 = 0x4e00;
  *(uint16_t*)0x20000b14 = 0;
  *(uint16_t*)0x20000b16 = 0x4e00;
  syz_usb_ep_write(/*fd=*/-1, /*ep=*/0x82, /*len=*/0x298, /*data=*/0x20000880);
  *(uint32_t*)0x20000080 = 0x2c;
  *(uint64_t*)0x20000084 = 0x20000100;
  memcpy((void*)0x20000100, "\x00\x00\x02", 3);
  *(uint64_t*)0x2000008c = 0;
  *(uint64_t*)0x20000094 = 0;
  *(uint64_t*)0x2000009c = 0;
  *(uint64_t*)0x200000a4 = 0;
  syz_usb_control_io(/*fd=*/r[0], /*descs=*/0x20000080, /*resps=*/0);
  memcpy(
      (void*)0x200002c0,
      "\xb9\x42\x5b\x44\x65\x1d\xd2\x32\x41\x96\x35\x99\x00\x00\x00\x11\x00\x00"
      "\x00\x4a\x16\x94\x1f\xf5\xf4\xb4\xf1\xf0\xad\xd7\xfc\xf2\xb8\x77\xfc\xea"
      "\xff\xff\xff\xff\xff\xf1\xff\xdf\x4c\xd9\xf5\xd3\x96\x98\x90\x52\x2c\x77"
      "\x15\x7d\x88\x01\x00\x00\x00\x3a\x5b\xd5\x53\x1d\x45\x9d\xff\xff\x03\x00"
      "\x00\x00\x00\x00\x91\xff\x00\x00\x00\xe8\xf5\xb3\x37\x1d\xa3\x63\x5b\x8b"
      "\x4f\xa6\x37\x13\x58\x00\x00\x1f\x65\xe4\xb4\x36\xaa\x9e\x50\xbc\x0f\x19"
      "\xb7\xd3\x37\x2f\xf9\xeb\xce\xde\x1f\xb5\xe9\x42\x8f\x54\xd5\xd1\xf0\xcc"
      "\x75\x2c\xf2\x46\xa5\xd2\xda\x34\xa5\xaa\x97\xdc\x14\xa4\x69\xc3\xdd\x3e"
      "\x26\xb4\x1c\x35\x64\x84\xe4\x6f\xd6\x6e\x3f\x2c\x78\x07\xe8\x77\x3e\xed"
      "\x7b\x94\xfa\x09\x9a\xb8\x4f\xea\xde\xc2\xea\x95\xf6\x5b\xba\x45\x2e\xae"
      "\x5b\x09\x00\xf9\x8a\x97\x9a\x88\xc5\x17\xa2\xdc\x36\x0a\x00\x23\x77\x23"
      "\xe2\xf4\x67\xaf\x70\x6e\xa1\x72\x26\x29\x6b\x3a\x10\xa3\x51\xcb\x47\xab"
      "\xa2\xc6\xb8\x36\xc9\x06\x79\xb4\xdd\x85\x9d\xdc\x9e\x48\x00\x44\x8a\xab"
      "\x00\x00\x00\x00\x00\x00\x0d\x75\xf3\x4b\xb5\x0d\x8d\x70\x84",
      249);
  syz_usb_ep_write(/*fd=*/r[0], /*ep=*/0x81, /*len=*/0xffffff75,
                   /*data=*/0x200002c0);
  syz_usb_control_io(/*fd=*/-1, /*descs=*/0, /*resps=*/0);
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
