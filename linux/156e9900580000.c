// https://syzkaller.appspot.com/bug?id=69039effb74169ecc96b287f7fc7a30ef721a17f
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

uint64_t r[6] = {0xffffffffffffffff, 0x0, 0x0, 0xffffffffffffffff, 0x0, 0x0};

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
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  res = syscall(__NR_socket, /*domain=*/0x10ul, /*type=*/3ul, /*proto=*/0x10);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000200, "nl80211\000", 8);
  res = -1;
  res = syz_genetlink_get_family_id(/*name=*/0x20000200, /*fd=*/-1);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000700, "wlan1\000\000\000\000\000\000\000\000\000\000\000",
         16);
  res = syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x8933, /*arg=*/0x20000700ul);
  if (res != -1)
    r[2] = *(uint32_t*)0x20000710;
  *(uint64_t*)0x20000340 = 0;
  *(uint32_t*)0x20000348 = 0;
  *(uint64_t*)0x20000350 = 0x200001c0;
  *(uint64_t*)0x200001c0 = 0x20000240;
  *(uint32_t*)0x20000240 = 0x24;
  *(uint16_t*)0x20000244 = r[1];
  *(uint16_t*)0x20000246 = 5;
  *(uint32_t*)0x20000248 = 0;
  *(uint32_t*)0x2000024c = 0;
  *(uint8_t*)0x20000250 = 6;
  *(uint8_t*)0x20000251 = 0;
  *(uint16_t*)0x20000252 = 0;
  *(uint16_t*)0x20000254 = 8;
  *(uint16_t*)0x20000256 = 3;
  *(uint32_t*)0x20000258 = r[2];
  *(uint16_t*)0x2000025c = 8;
  *(uint16_t*)0x2000025e = 5;
  *(uint32_t*)0x20000260 = 3;
  *(uint64_t*)0x200001c8 = 0x24;
  *(uint64_t*)0x20000358 = 1;
  *(uint64_t*)0x20000360 = 0;
  *(uint64_t*)0x20000368 = 0;
  *(uint32_t*)0x20000370 = 0;
  syscall(__NR_sendmsg, /*fd=*/r[0], /*msg=*/0x20000340ul, /*f=*/0ul);
  res = syscall(__NR_socket, /*domain=*/0x10ul, /*type=*/3ul, /*proto=*/0x10);
  if (res != -1)
    r[3] = res;
  memcpy((void*)0x20000140, "nl80211\000", 8);
  res = -1;
  res = syz_genetlink_get_family_id(/*name=*/0x20000140, /*fd=*/-1);
  if (res != -1)
    r[4] = res;
  memcpy((void*)0x20000000, "wlan1\000\000\000\000\000\000\000\000\000\000\000",
         16);
  res = syscall(__NR_ioctl, /*fd=*/r[3], /*cmd=*/0x8933, /*arg=*/0x20000000ul);
  if (res != -1)
    r[5] = *(uint32_t*)0x20000010;
  *(uint64_t*)0x200004c0 = 0;
  *(uint32_t*)0x200004c8 = 0;
  *(uint64_t*)0x200004d0 = 0x200001c0;
  *(uint64_t*)0x200001c0 = 0x20000440;
  *(uint32_t*)0x20000440 = 0x40;
  *(uint16_t*)0x20000444 = r[4];
  *(uint16_t*)0x20000446 = 1;
  *(uint32_t*)0x20000448 = 0;
  *(uint32_t*)0x2000044c = 0;
  *(uint8_t*)0x20000450 = 0x13;
  *(uint8_t*)0x20000451 = 0;
  *(uint16_t*)0x20000452 = 0;
  *(uint16_t*)0x20000454 = 8;
  *(uint16_t*)0x20000456 = 3;
  *(uint32_t*)0x20000458 = r[5];
  *(uint16_t*)0x2000045c = 0xa;
  *(uint16_t*)0x2000045e = 6;
  *(uint8_t*)0x20000460 = 8;
  *(uint8_t*)0x20000461 = 2;
  *(uint8_t*)0x20000462 = 0x11;
  *(uint8_t*)0x20000463 = 0;
  *(uint8_t*)0x20000464 = 0;
  *(uint8_t*)0x20000465 = 0;
  *(uint16_t*)0x20000468 = 6;
  *(uint16_t*)0x2000046a = 0x12;
  *(uint16_t*)0x2000046c = 0;
  *(uint16_t*)0x20000470 = 4;
  *(uint16_t*)0x20000472 = 0x13;
  *(uint16_t*)0x20000474 = 4;
  STORE_BY_BITMASK(uint16_t, , 0x20000476, 0x11, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000477, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000477, 1, 7, 1);
  *(uint16_t*)0x20000478 = 6;
  *(uint16_t*)0x2000047a = 0x10;
  *(uint16_t*)0x2000047c = 0x37;
  *(uint64_t*)0x200001c8 = 0x40;
  *(uint64_t*)0x200004d8 = 1;
  *(uint64_t*)0x200004e0 = 0;
  *(uint64_t*)0x200004e8 = 0;
  *(uint32_t*)0x200004f0 = 0;
  syscall(__NR_sendmsg, /*fd=*/r[3], /*msg=*/0x200004c0ul, /*f=*/0ul);
  return 0;
}