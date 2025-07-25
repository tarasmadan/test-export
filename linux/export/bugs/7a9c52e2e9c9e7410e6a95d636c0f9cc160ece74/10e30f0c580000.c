// https://syzkaller.appspot.com/bug?id=7a9c52e2e9c9e7410e6a95d636c0f9cc160ece74
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

uint64_t r[4] = {0xffffffffffffffff, 0x0, 0xffffffffffffffff, 0x0};

int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffffffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul,
          /*fd=*/(intptr_t)-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200000000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul,
          /*fd=*/(intptr_t)-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200001000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul,
          /*fd=*/(intptr_t)-1, /*offset=*/0ul);
  const char* reason;
  (void)reason;
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  res = syscall(__NR_socket, /*domain=*/0x10ul, /*type=*/3ul, /*proto=*/0x10);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x2000000000c0, "ethtool\000", 8);
  res = -1;
  res = syz_genetlink_get_family_id(/*name=*/0x2000000000c0, /*fd=*/-1);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_socket, /*domain=AF_NETLINK*/ 0x10ul,
                /*type=SOCK_RAW*/ 3ul, /*proto=*/0);
  if (res != -1)
    r[2] = res;
  memcpy((void*)0x200000000240,
         "lo\000\000\000\000\000\000\000\000\000\000\000\000\000\000", 16);
  res = syscall(__NR_ioctl, /*fd=*/r[2], /*cmd=*/0x8933,
                /*arg=*/0x200000000240ul);
  if (res != -1)
    r[3] = *(uint32_t*)0x200000000250;
  *(uint64_t*)0x2000000002c0 = 0;
  *(uint32_t*)0x2000000002c8 = 0;
  *(uint64_t*)0x2000000002d0 = 0x200000000280;
  *(uint64_t*)0x200000000280 = 0x200000000240;
  *(uint32_t*)0x200000000240 = 0x40;
  *(uint16_t*)0x200000000244 = r[1];
  *(uint16_t*)0x200000000246 = 1;
  *(uint32_t*)0x200000000248 = 0x70bd2c;
  *(uint32_t*)0x20000000024c = 0x25dfdbff;
  *(uint8_t*)0x200000000250 = 0x16;
  *(uint8_t*)0x200000000251 = 0;
  *(uint16_t*)0x200000000252 = 0;
  *(uint16_t*)0x200000000254 = 0x14;
  STORE_BY_BITMASK(uint16_t, , 0x200000000256, 1, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200000000257, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200000000257, 1, 7, 1);
  *(uint16_t*)0x200000000258 = 8;
  *(uint16_t*)0x20000000025a = 1;
  *(uint32_t*)0x20000000025c = 0;
  *(uint16_t*)0x200000000260 = 8;
  *(uint16_t*)0x200000000262 = 1;
  *(uint32_t*)0x200000000264 = r[3];
  *(uint16_t*)0x200000000268 = 5;
  *(uint16_t*)0x20000000026a = 3;
  *(uint8_t*)0x20000000026c = 1;
  *(uint16_t*)0x200000000270 = 5;
  *(uint16_t*)0x200000000272 = 3;
  *(uint8_t*)0x200000000274 = 0;
  *(uint16_t*)0x200000000278 = 5;
  *(uint16_t*)0x20000000027a = 3;
  *(uint8_t*)0x20000000027c = 1;
  *(uint64_t*)0x200000000288 = 0x40;
  *(uint64_t*)0x2000000002d8 = 1;
  *(uint64_t*)0x2000000002e0 = 0;
  *(uint64_t*)0x2000000002e8 = 0;
  *(uint32_t*)0x2000000002f0 = 0x8000000;
  syscall(__NR_sendmsg, /*fd=*/r[0], /*msg=*/0x2000000002c0ul,
          /*f=MSG_NOSIGNAL|MSG_MORE|MSG_EOR*/ 0xc080ul);
  return 0;
}
