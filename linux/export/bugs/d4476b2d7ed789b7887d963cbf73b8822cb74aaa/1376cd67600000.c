// https://syzkaller.appspot.com/bug?id=d4476b2d7ed789b7887d963cbf73b8822cb74aaa
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/usb/ch9.h>

unsigned long long procid;

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

#define USB_DEBUG 0

#define USB_MAX_IFACE_NUM 4
#define USB_MAX_EP_NUM 32

struct usb_iface_index {
  struct usb_interface_descriptor* iface;
  struct usb_endpoint_descriptor* eps[USB_MAX_EP_NUM];
  unsigned eps_num;
};

struct usb_device_index {
  struct usb_device_descriptor* dev;
  struct usb_config_descriptor* config;
  unsigned config_length;
  struct usb_iface_index ifaces[USB_MAX_IFACE_NUM];
  unsigned ifaces_num;
};

static bool parse_usb_descriptor(char* buffer, size_t length,
                                 struct usb_device_index* index)
{
  if (length < sizeof(*index->dev) + sizeof(*index->config))
    return false;
  memset(index, 0, sizeof(*index));
  index->dev = (struct usb_device_descriptor*)buffer;
  index->config = (struct usb_config_descriptor*)(buffer + sizeof(*index->dev));
  index->config_length = length - sizeof(*index->dev);
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
      index->ifaces[index->ifaces_num++].iface = iface;
    }
    if (desc_type == USB_DT_ENDPOINT && index->ifaces_num > 0) {
      struct usb_iface_index* iface = &index->ifaces[index->ifaces_num - 1];
      if (iface->eps_num < USB_MAX_EP_NUM)
        iface->eps[iface->eps_num++] =
            (struct usb_endpoint_descriptor*)(buffer + offset);
    }
    offset += desc_length;
  }
  return true;
}

enum usb_fuzzer_event_type {
  USB_FUZZER_EVENT_INVALID,
  USB_FUZZER_EVENT_CONNECT,
  USB_FUZZER_EVENT_DISCONNECT,
  USB_FUZZER_EVENT_SUSPEND,
  USB_FUZZER_EVENT_RESUME,
  USB_FUZZER_EVENT_CONTROL,
};

struct usb_fuzzer_event {
  uint32_t type;
  uint32_t length;
  char data[0];
};

struct usb_fuzzer_init {
  uint64_t speed;
  const char* driver_name;
  const char* device_name;
};

struct usb_fuzzer_ep_io {
  uint16_t ep;
  uint16_t flags;
  uint32_t length;
  char data[0];
};

#define USB_FUZZER_IOCTL_INIT _IOW('U', 0, struct usb_fuzzer_init)
#define USB_FUZZER_IOCTL_RUN _IO('U', 1)
#define USB_FUZZER_IOCTL_EVENT_FETCH _IOR('U', 2, struct usb_fuzzer_event)
#define USB_FUZZER_IOCTL_EP0_WRITE _IOW('U', 3, struct usb_fuzzer_ep_io)
#define USB_FUZZER_IOCTL_EP0_READ _IOWR('U', 4, struct usb_fuzzer_ep_io)
#define USB_FUZZER_IOCTL_EP_ENABLE _IOW('U', 5, struct usb_endpoint_descriptor)
#define USB_FUZZER_IOCTL_EP_WRITE _IOW('U', 7, struct usb_fuzzer_ep_io)
#define USB_FUZZER_IOCTL_EP_READ _IOWR('U', 8, struct usb_fuzzer_ep_io)
#define USB_FUZZER_IOCTL_CONFIGURE _IO('U', 9)
#define USB_FUZZER_IOCTL_VBUS_DRAW _IOW('U', 10, uint32_t)

int usb_fuzzer_open()
{
  return open("/sys/kernel/debug/usb-fuzzer", O_RDWR);
}

int usb_fuzzer_init(int fd, uint32_t speed, const char* driver,
                    const char* device)
{
  struct usb_fuzzer_init arg;
  arg.speed = speed;
  arg.driver_name = driver;
  arg.device_name = device;
  return ioctl(fd, USB_FUZZER_IOCTL_INIT, &arg);
}

int usb_fuzzer_run(int fd)
{
  return ioctl(fd, USB_FUZZER_IOCTL_RUN, 0);
}

int usb_fuzzer_event_fetch(int fd, struct usb_fuzzer_event* event)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EVENT_FETCH, event);
}

int usb_fuzzer_ep0_write(int fd, struct usb_fuzzer_ep_io* io)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP0_WRITE, io);
}

int usb_fuzzer_ep0_read(int fd, struct usb_fuzzer_ep_io* io)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP0_READ, io);
}

int usb_fuzzer_ep_write(int fd, struct usb_fuzzer_ep_io* io)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP_WRITE, io);
}

int usb_fuzzer_ep_read(int fd, struct usb_fuzzer_ep_io* io)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP_READ, io);
}

int usb_fuzzer_ep_enable(int fd, struct usb_endpoint_descriptor* desc)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP_ENABLE, desc);
}

int usb_fuzzer_configure(int fd)
{
  return ioctl(fd, USB_FUZZER_IOCTL_CONFIGURE, 0);
}

int usb_fuzzer_vbus_draw(int fd, uint32_t power)
{
  return ioctl(fd, USB_FUZZER_IOCTL_VBUS_DRAW, power);
}

#define USB_MAX_PACKET_SIZE 1024

struct usb_fuzzer_control_event {
  struct usb_fuzzer_event inner;
  struct usb_ctrlrequest ctrl;
  char data[USB_MAX_PACKET_SIZE];
};

struct usb_fuzzer_ep_io_data {
  struct usb_fuzzer_ep_io inner;
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

static bool lookup_connect_response(struct vusb_connect_descriptors* descs,
                                    struct usb_device_index* index,
                                    struct usb_ctrlrequest* ctrl,
                                    char** response_data,
                                    uint32_t* response_length)
{
  uint8_t str_idx;
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
  struct usb_device_index index;
  memset(&index, 0, sizeof(index));
  int rv = 0;
  rv = parse_usb_descriptor(dev, dev_len, &index);
  if (!rv) {
    return rv;
  }
  int fd = usb_fuzzer_open();
  if (fd < 0) {
    return fd;
  }
  char device[32];
  sprintf(&device[0], "dummy_udc.%llu", procid);
  rv = usb_fuzzer_init(fd, speed, "dummy_udc", &device[0]);
  if (rv < 0) {
    return rv;
  }
  rv = usb_fuzzer_run(fd);
  if (rv < 0) {
    return rv;
  }
  bool done = false;
  while (!done) {
    struct usb_fuzzer_control_event event;
    event.inner.type = 0;
    event.inner.length = sizeof(event.ctrl);
    rv = usb_fuzzer_event_fetch(fd, (struct usb_fuzzer_event*)&event);
    if (rv < 0) {
      return rv;
    }
    if (event.inner.type != USB_FUZZER_EVENT_CONTROL)
      continue;
    bool response_found = false;
    char* response_data = NULL;
    uint32_t response_length = 0;
    if (event.ctrl.bRequestType & USB_DIR_IN) {
      response_found = lookup_connect_response(
          descs, &index, &event.ctrl, &response_data, &response_length);
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
      rv = usb_fuzzer_vbus_draw(fd, index.config->bMaxPower);
      if (rv < 0) {
        return rv;
      }
      rv = usb_fuzzer_configure(fd);
      if (rv < 0) {
        return rv;
      }
      unsigned ep;
      for (ep = 0; ep < index.ifaces[0].eps_num; ep++) {
        rv = usb_fuzzer_ep_enable(fd, index.ifaces[0].eps[ep]);
        if (rv < 0) {
        } else {
        }
      }
    }
    struct usb_fuzzer_ep_io_data response;
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
      rv = usb_fuzzer_ep0_write(fd, (struct usb_fuzzer_ep_io*)&response);
    } else {
      rv = usb_fuzzer_ep0_read(fd, (struct usb_fuzzer_ep_io*)&response);
    }
    if (rv < 0) {
      return rv;
    }
  }
  sleep_ms(200);
  return fd;
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

static bool lookup_control_response(struct vusb_descriptors* descs,
                                    struct vusb_responses* resps,
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

static volatile long syz_usb_control_io(volatile long a0, volatile long a1,
                                        volatile long a2)
{
  int fd = a0;
  struct vusb_descriptors* descs = (struct vusb_descriptors*)a1;
  struct vusb_responses* resps = (struct vusb_responses*)a2;
  struct usb_fuzzer_control_event event;
  event.inner.type = 0;
  event.inner.length = USB_MAX_PACKET_SIZE;
  int rv = usb_fuzzer_event_fetch(fd, (struct usb_fuzzer_event*)&event);
  if (rv < 0) {
    return rv;
  }
  if (event.inner.type != USB_FUZZER_EVENT_CONTROL) {
    return -1;
  }
  bool response_found = false;
  char* response_data = NULL;
  uint32_t response_length = 0;
  if ((event.ctrl.bRequestType & USB_DIR_IN) && event.ctrl.wLength) {
    response_found = lookup_control_response(descs, resps, &event.ctrl,
                                             &response_data, &response_length);
    if (!response_found) {
      return -1;
    }
  } else {
    response_length = event.ctrl.wLength;
  }
  struct usb_fuzzer_ep_io_data response;
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
    rv = usb_fuzzer_ep0_write(fd, (struct usb_fuzzer_ep_io*)&response);
  } else {
    rv = usb_fuzzer_ep0_read(fd, (struct usb_fuzzer_ep_io*)&response);
  }
  if (rv < 0) {
    return rv;
  }
  sleep_ms(200);
  return 0;
}

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  intptr_t res = 0;
  *(uint8_t*)0x20005380 = 0x12;
  *(uint8_t*)0x20005381 = 1;
  *(uint16_t*)0x20005382 = 0;
  *(uint8_t*)0x20005384 = 0;
  *(uint8_t*)0x20005385 = 0;
  *(uint8_t*)0x20005386 = 0;
  *(uint8_t*)0x20005387 = 8;
  *(uint16_t*)0x20005388 = 0xe8f;
  *(uint16_t*)0x2000538a = 0x12;
  *(uint16_t*)0x2000538c = 0;
  *(uint8_t*)0x2000538e = 0;
  *(uint8_t*)0x2000538f = 0;
  *(uint8_t*)0x20005390 = 0;
  *(uint8_t*)0x20005391 = 1;
  *(uint8_t*)0x20005392 = 9;
  *(uint8_t*)0x20005393 = 2;
  *(uint16_t*)0x20005394 = 0x24;
  *(uint8_t*)0x20005396 = 1;
  *(uint8_t*)0x20005397 = 0;
  *(uint8_t*)0x20005398 = 0;
  *(uint8_t*)0x20005399 = 0;
  *(uint8_t*)0x2000539a = 0;
  *(uint8_t*)0x2000539b = 9;
  *(uint8_t*)0x2000539c = 4;
  *(uint8_t*)0x2000539d = 0;
  *(uint8_t*)0x2000539e = 0;
  *(uint8_t*)0x2000539f = 9;
  *(uint8_t*)0x200053a0 = 3;
  *(uint8_t*)0x200053a1 = 0;
  *(uint8_t*)0x200053a2 = 0;
  *(uint8_t*)0x200053a3 = 0;
  *(uint8_t*)0x200053a4 = 9;
  *(uint8_t*)0x200053a5 = 0x21;
  *(uint16_t*)0x200053a6 = 0;
  *(uint8_t*)0x200053a8 = 0;
  *(uint8_t*)0x200053a9 = 1;
  *(uint8_t*)0x200053aa = 0x22;
  *(uint16_t*)0x200053ab = 0x22;
  *(uint8_t*)0x200053ad = 9;
  *(uint8_t*)0x200053ae = 5;
  *(uint8_t*)0x200053af = 0x81;
  *(uint8_t*)0x200053b0 = 3;
  *(uint16_t*)0x200053b1 = 0;
  *(uint8_t*)0x200053b3 = 0;
  *(uint8_t*)0x200053b4 = 0;
  *(uint8_t*)0x200053b5 = 0;
  res = syz_usb_connect(0, 0x36, 0x20005380, 0);
  if (res != -1)
    r[0] = res;
  syz_usb_control_io(r[0], 0, 0);
  *(uint32_t*)0x200001c0 = 0x24;
  *(uint64_t*)0x200001c4 = 0;
  *(uint64_t*)0x200001cc = 0;
  *(uint64_t*)0x200001d4 = 0x20000000;
  *(uint8_t*)0x20000000 = 0;
  *(uint8_t*)0x20000001 = 0x22;
  *(uint32_t*)0x20000002 = 0x22;
  STORE_BY_BITMASK(uint8_t, , 0x20000006, 3, 0, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000006, 1, 2, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000006, 0, 4, 4);
  memcpy((void*)0x20000007, "\xf3\xaa\xf6\x14", 4);
  STORE_BY_BITMASK(uint8_t, , 0x2000000b, 2, 0, 2);
  STORE_BY_BITMASK(uint8_t, , 0x2000000b, 1, 2, 2);
  STORE_BY_BITMASK(uint8_t, , 0x2000000b, 9, 4, 4);
  memcpy((void*)0x2000000c, "\xd1\x18", 2);
  STORE_BY_BITMASK(uint8_t, , 0x2000000e, 3, 0, 2);
  STORE_BY_BITMASK(uint8_t, , 0x2000000e, 1, 2, 2);
  STORE_BY_BITMASK(uint8_t, , 0x2000000e, 0, 4, 4);
  memcpy((void*)0x2000000f, "\000\000D\000", 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000013, 3, 0, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000013, 2, 2, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000013, 0, 4, 4);
  memcpy((void*)0x20000014, "\x8c\x0a\x56\xe4", 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000018, 3, 0, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000018, 0, 2, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000018, 9, 4, 4);
  memcpy((void*)0x20000019, "\xe6\x72\xec\xd7", 4);
  STORE_BY_BITMASK(uint8_t, , 0x2000001d, 3, 0, 2);
  STORE_BY_BITMASK(uint8_t, , 0x2000001d, 0, 2, 2);
  STORE_BY_BITMASK(uint8_t, , 0x2000001d, 0, 4, 4);
  memcpy((void*)0x2000001e, "\x77\xdd\xb6\x0f", 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000022, 2, 0, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000022, 2, 2, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000022, 0, 4, 4);
  memcpy((void*)0x20000023, "\xf4\x34", 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000025, 2, 0, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000025, 2, 2, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000025, 0, 4, 4);
  memcpy((void*)0x20000026, "\x18\xe1", 2);
  *(uint64_t*)0x200001dc = 0;
  syz_usb_control_io(r[0], 0x200001c0, 0);
  return 0;
}