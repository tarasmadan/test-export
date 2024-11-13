// https://syzkaller.appspot.com/bug?id=61b54ba5cc42164169a3c5c3b1dd40830c610aad
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/genetlink.h>
#include <linux/if_addr.h>
#include <linux/if_link.h>
#include <linux/in6.h>
#include <linux/neighbour.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/veth.h>

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

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

static void netlink_nest(struct nlmsg* nlmsg, int typ)
{
  struct nlattr* attr = (struct nlattr*)nlmsg->pos;
  attr->nla_type = typ;
  nlmsg->pos += sizeof(*attr);
  nlmsg->nested[nlmsg->nesting++] = attr;
}

static void netlink_done(struct nlmsg* nlmsg)
{
  struct nlattr* attr = nlmsg->nested[--nlmsg->nesting];
  attr->nla_len = nlmsg->pos - (char*)attr;
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

static void netlink_add_device_impl(struct nlmsg* nlmsg, const char* type,
                                    const char* name, bool up)
{
  struct ifinfomsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  if (up)
    hdr.ifi_flags = hdr.ifi_change = IFF_UP;
  netlink_init(nlmsg, RTM_NEWLINK, NLM_F_EXCL | NLM_F_CREATE, &hdr,
               sizeof(hdr));
  if (name)
    netlink_attr(nlmsg, IFLA_IFNAME, name, strlen(name));
  netlink_nest(nlmsg, IFLA_LINKINFO);
  netlink_attr(nlmsg, IFLA_INFO_KIND, type, strlen(type));
}

static void netlink_device_change(struct nlmsg* nlmsg, int sock,
                                  const char* name, bool up, const char* master,
                                  const void* mac, int macsize,
                                  const char* new_name)
{
  struct ifinfomsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  if (up)
    hdr.ifi_flags = hdr.ifi_change = IFF_UP;
  hdr.ifi_index = if_nametoindex(name);
  netlink_init(nlmsg, RTM_NEWLINK, 0, &hdr, sizeof(hdr));
  if (new_name)
    netlink_attr(nlmsg, IFLA_IFNAME, new_name, strlen(new_name));
  if (master) {
    int ifindex = if_nametoindex(master);
    netlink_attr(nlmsg, IFLA_MASTER, &ifindex, sizeof(ifindex));
  }
  if (macsize)
    netlink_attr(nlmsg, IFLA_ADDRESS, mac, macsize);
  int err = netlink_send(nlmsg, sock);
  if (err < 0) {
  }
}

static struct nlmsg nlmsg;

#define NL802154_CMD_SET_SHORT_ADDR 11
#define NL802154_ATTR_IFINDEX 3
#define NL802154_ATTR_SHORT_ADDR 10

static void setup_802154()
{
  int sock_route = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock_route == -1)
    exit(1);
  int sock_generic = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock_generic < 0)
    exit(1);
  int nl802154_family_id =
      netlink_query_family_id(&nlmsg, sock_generic, "nl802154", true);
  for (int i = 0; i < 2; i++) {
    char devname[] = "wpan0";
    devname[strlen(devname) - 1] += i;
    uint64_t hwaddr = 0xaaaaaaaaaaaa0002 + (i << 8);
    uint16_t shortaddr = 0xaaa0 + i;
    int ifindex = if_nametoindex(devname);
    struct genlmsghdr genlhdr;
    memset(&genlhdr, 0, sizeof(genlhdr));
    genlhdr.cmd = NL802154_CMD_SET_SHORT_ADDR;
    netlink_init(&nlmsg, nl802154_family_id, 0, &genlhdr, sizeof(genlhdr));
    netlink_attr(&nlmsg, NL802154_ATTR_IFINDEX, &ifindex, sizeof(ifindex));
    netlink_attr(&nlmsg, NL802154_ATTR_SHORT_ADDR, &shortaddr,
                 sizeof(shortaddr));
    int err = netlink_send(&nlmsg, sock_generic);
    if (err < 0) {
    }
    netlink_device_change(&nlmsg, sock_route, devname, true, 0, &hwaddr,
                          sizeof(hwaddr), 0);
    if (i == 0) {
      netlink_add_device_impl(&nlmsg, "lowpan", "lowpan0", false);
      netlink_done(&nlmsg);
      netlink_attr(&nlmsg, IFLA_LINK, &ifindex, sizeof(ifindex));
      int err = netlink_send(&nlmsg, sock_route);
      if (err < 0) {
      }
    }
  }
  close(sock_route);
  close(sock_generic);
}

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

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
  setup_802154();
  intptr_t res = 0;
  *(uint32_t*)0x2000e000 = 0x10;
  *(uint32_t*)0x2000e004 = 4;
  *(uint64_t*)0x2000e008 = 0x20000040;
  memcpy((void*)0x20000040,
         "\xb4\x00\x00\x00\x00\x00\x00\x00\x79\x10\x48\x00\x00\x00\x00\x00\x61"
         "\x04\x00\x00\x00\x00\x00\x00\x95\x00\x00\x80",
         28);
  *(uint64_t*)0x2000e010 = 0x20003ff6;
  memcpy((void*)0x20003ff6, "GPL\000", 4);
  *(uint32_t*)0x2000e018 = 2;
  *(uint32_t*)0x2000e01c = 0xfd90;
  *(uint64_t*)0x2000e020 = 0x2000cf3d;
  *(uint32_t*)0x2000e028 = 0;
  *(uint32_t*)0x2000e02c = 0;
  memset((void*)0x2000e030, 0, 16);
  *(uint32_t*)0x2000e040 = 0;
  *(uint32_t*)0x2000e044 = 0;
  *(uint32_t*)0x2000e048 = -1;
  *(uint32_t*)0x2000e04c = 8;
  *(uint64_t*)0x2000e050 = 0;
  *(uint32_t*)0x2000e058 = 0;
  *(uint32_t*)0x2000e05c = 0x10;
  *(uint64_t*)0x2000e060 = 0;
  *(uint32_t*)0x2000e068 = 0;
  *(uint32_t*)0x2000e06c = 0;
  *(uint32_t*)0x2000e070 = -1;
  *(uint32_t*)0x2000e074 = 0;
  *(uint64_t*)0x2000e078 = 0;
  *(uint64_t*)0x2000e080 = 0;
  *(uint32_t*)0x2000e088 = 0x10;
  *(uint32_t*)0x2000e08c = 0;
  res = syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x2000e000ul, /*size=*/0x48ul);
  if (res != -1)
    r[0] = res;
  syscall(__NR_close, /*fd=*/r[0]);
  syscall(__NR_socketpair, /*domain=*/1ul, /*type=SOCK_STREAM*/ 1ul,
          /*proto=*/0, /*fds=*/0x200029c0ul);
  *(uint32_t*)0x20000200 = 0xf;
  *(uint32_t*)0x20000204 = 4;
  *(uint32_t*)0x20000208 = 4;
  *(uint32_t*)0x2000020c = 0x12;
  *(uint32_t*)0x20000210 = 0;
  *(uint32_t*)0x20000214 = -1;
  *(uint32_t*)0x20000218 = 0;
  memset((void*)0x2000021c, 0, 16);
  *(uint32_t*)0x2000022c = 0;
  *(uint32_t*)0x20000230 = -1;
  *(uint32_t*)0x20000234 = 0;
  *(uint32_t*)0x20000238 = 0;
  *(uint32_t*)0x2000023c = 0;
  *(uint64_t*)0x20000240 = 0;
  res = syscall(__NR_bpf, /*cmd=*/0ul, /*arg=*/0x20000200ul, /*size=*/0x48ul);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x200000c0 = r[1];
  *(uint64_t*)0x200000c8 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint64_t*)0x200000d0 = 0x20000080;
  *(uint32_t*)0x20000080 = r[0];
  *(uint64_t*)0x200000d8 = 0;
  res = syscall(__NR_bpf, /*cmd=*/2ul, /*arg=*/0x200000c0ul, /*size=*/0x20ul);
  if (res != -1)
    r[2] = *(uint32_t*)0x200000c0;
  *(uint32_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c4 = 0xc;
  *(uint64_t*)0x200000c8 = 0x20000440;
  *(uint8_t*)0x20000440 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000441, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000441, 0, 4, 4);
  *(uint16_t*)0x20000442 = 0;
  *(uint32_t*)0x20000444 = 0;
  *(uint8_t*)0x20000448 = 0;
  *(uint8_t*)0x20000449 = 0;
  *(uint16_t*)0x2000044a = 0;
  *(uint32_t*)0x2000044c = 0;
  *(uint8_t*)0x20000450 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000451, 1, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000451, 1, 4, 4);
  *(uint16_t*)0x20000452 = 0;
  *(uint32_t*)0x20000454 = r[2];
  *(uint8_t*)0x20000458 = 0;
  *(uint8_t*)0x20000459 = 0;
  *(uint16_t*)0x2000045a = 0;
  *(uint32_t*)0x2000045c = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000460, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000460, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000460, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000461, 8, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000461, 0, 4, 4);
  *(uint16_t*)0x20000462 = 0;
  *(uint32_t*)0x20000464 = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000468, 3, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000468, 3, 3, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000468, 3, 5, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000469, 0xa, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000469, 8, 4, 4);
  *(uint16_t*)0x2000046a = 0xfff8;
  *(uint32_t*)0x2000046c = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000470, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000470, 1, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000470, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000471, 2, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000471, 0xa, 4, 4);
  *(uint16_t*)0x20000472 = 0;
  *(uint32_t*)0x20000474 = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000478, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000478, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000478, 0, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000479, 2, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000479, 0, 4, 4);
  *(uint16_t*)0x2000047a = 0;
  *(uint32_t*)0x2000047c = 0xfffffff8;
  STORE_BY_BITMASK(uint8_t, , 0x20000480, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000480, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000480, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000481, 3, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000481, 0, 4, 4);
  *(uint16_t*)0x20000482 = 0;
  *(uint32_t*)0x20000484 = 8;
  STORE_BY_BITMASK(uint8_t, , 0x20000488, 7, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000488, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000488, 0xb, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000489, 4, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000489, 0, 4, 4);
  *(uint16_t*)0x2000048a = 0;
  *(uint32_t*)0x2000048c = 0;
  *(uint8_t*)0x20000490 = 0x85;
  *(uint8_t*)0x20000491 = 0;
  *(uint16_t*)0x20000492 = 0;
  *(uint32_t*)0x20000494 = 3;
  *(uint8_t*)0x20000498 = 0x95;
  *(uint8_t*)0x20000499 = 0;
  *(uint16_t*)0x2000049a = 0;
  *(uint32_t*)0x2000049c = 0;
  *(uint64_t*)0x200000d0 = 0;
  *(uint32_t*)0x200000d8 = 0;
  *(uint32_t*)0x200000dc = 0;
  *(uint64_t*)0x200000e0 = 0;
  *(uint32_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000ec = 0;
  memset((void*)0x200000f0, 0, 16);
  *(uint32_t*)0x20000100 = 0;
  *(uint32_t*)0x20000104 = 0;
  *(uint32_t*)0x20000108 = -1;
  *(uint32_t*)0x2000010c = 0;
  *(uint64_t*)0x20000110 = 0;
  *(uint32_t*)0x20000118 = 0;
  *(uint32_t*)0x2000011c = 0;
  *(uint64_t*)0x20000120 = 0;
  *(uint32_t*)0x20000128 = 0;
  *(uint32_t*)0x2000012c = 0;
  *(uint32_t*)0x20000130 = 0;
  *(uint32_t*)0x20000134 = 0;
  *(uint64_t*)0x20000138 = 0;
  *(uint64_t*)0x20000140 = 0;
  *(uint32_t*)0x20000148 = 0;
  *(uint32_t*)0x2000014c = 0;
  syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x200000c0ul, /*size=*/0x90ul);
  *(uint32_t*)0x200000c0 = 0x11;
  *(uint32_t*)0x200000c4 = 0xc;
  *(uint64_t*)0x200000c8 = 0x20000440;
  *(uint64_t*)0x200000d0 = 0x20000240;
  memcpy((void*)0x20000240, "GPL\000", 4);
  *(uint32_t*)0x200000d8 = 0;
  *(uint32_t*)0x200000dc = 0;
  *(uint64_t*)0x200000e0 = 0;
  *(uint32_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000ec = 0;
  memset((void*)0x200000f0, 0, 16);
  *(uint32_t*)0x20000100 = 0;
  *(uint32_t*)0x20000104 = 0;
  *(uint32_t*)0x20000108 = -1;
  *(uint32_t*)0x2000010c = 0;
  *(uint64_t*)0x20000110 = 0;
  *(uint32_t*)0x20000118 = 0;
  *(uint32_t*)0x2000011c = 0;
  *(uint64_t*)0x20000120 = 0;
  *(uint32_t*)0x20000128 = 0;
  *(uint32_t*)0x2000012c = 0;
  *(uint32_t*)0x20000130 = 0;
  *(uint32_t*)0x20000134 = 0;
  *(uint64_t*)0x20000138 = 0;
  *(uint64_t*)0x20000140 = 0;
  *(uint32_t*)0x20000148 = 0;
  *(uint32_t*)0x2000014c = 0;
  res = syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x200000c0ul, /*size=*/0x90ul);
  if (res != -1)
    r[3] = res;
  *(uint64_t*)0x20000040 = 0x20000080;
  memcpy((void*)0x20000080, "tlb_flush\000", 10);
  *(uint32_t*)0x20000048 = r[3];
  syscall(__NR_bpf, /*cmd=*/0x11ul, /*arg=*/0x20000040ul, /*size=*/0x10ul);
  return 0;
}