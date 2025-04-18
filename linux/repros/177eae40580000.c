// https://syzkaller.appspot.com/bug?id=d71fda02529e58af09416d0df49c45ce5ee68fc1
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
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

#ifndef __NR_userfaultfd
#define __NR_userfaultfd 323
#endif

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

static int inject_fault(int nth)
{
  int fd;
  fd = open("/proc/thread-self/fail-nth", O_RDWR);
  if (fd == -1)
    exit(1);
  char buf[16];
  sprintf(buf, "%d", nth);
  if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
    exit(1);
  return fd;
}

static const char* setup_fault()
{
  int fd = open("/proc/self/make-it-fail", O_WRONLY);
  if (fd == -1)
    return "CONFIG_FAULT_INJECTION is not enabled";
  close(fd);
  fd = open("/proc/thread-self/fail-nth", O_WRONLY);
  if (fd == -1)
    return "kernel does not have systematic fault injection support";
  close(fd);
  static struct {
    const char* file;
    const char* val;
    bool fatal;
  } files[] = {
      {"/sys/kernel/debug/failslab/ignore-gfp-wait", "N", true},
      {"/sys/kernel/debug/fail_futex/ignore-private", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/ignore-gfp-highmem", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/ignore-gfp-wait", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/min-order", "0", false},
  };
  unsigned i;
  for (i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
    if (!write_file(files[i].file, files[i].val)) {
      if (files[i].fatal)
        return "failed to write fault injection file";
    }
  }
  return NULL;
}

#define NL802154_CMD_SET_SHORT_ADDR 11
#define NL802154_ATTR_IFINDEX 3
#define NL802154_ATTR_SHORT_ADDR 10

static const char* setup_802154()
{
  const char* error = NULL;
  int sock_generic = -1;
  int sock_route = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock_route == -1) {
    error = "socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE) failed";
    goto fail;
  }
  sock_generic = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock_generic == -1) {
    error = "socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC) failed";
    goto fail;
  }
  {
    int nl802154_family_id =
        netlink_query_family_id(&nlmsg, sock_generic, "nl802154", true);
    if (nl802154_family_id < 0) {
      error = "netlink_query_family_id failed";
      goto fail;
    }
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
      if (netlink_send(&nlmsg, sock_generic) < 0) {
        error = "NL802154_CMD_SET_SHORT_ADDR failed";
        goto fail;
      }
      netlink_device_change(&nlmsg, sock_route, devname, true, 0, &hwaddr,
                            sizeof(hwaddr), 0);
      if (i == 0) {
        netlink_add_device_impl(&nlmsg, "lowpan", "lowpan0", false);
        netlink_done(&nlmsg);
        netlink_attr(&nlmsg, IFLA_LINK, &ifindex, sizeof(ifindex));
        if (netlink_send(&nlmsg, sock_route) < 0) {
          error = "netlink: adding device lowpan0 type lowpan link wpan0";
          goto fail;
        }
      }
    }
  }
fail:
  close(sock_route);
  close(sock_generic);
  return error;
}

uint64_t r[1] = {0xffffffffffffffff};

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
  if ((reason = setup_fault()))
    printf("the reproducer may not work as expected: fault injection setup "
           "failed: %s\n",
           reason);
  if ((reason = setup_802154()))
    printf("the reproducer may not work as expected: 802154 injection setup "
           "failed: %s\n",
           reason);
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  syscall(
      __NR_mmap, /*addr=*/0x20800000ul, /*len=*/0x800000ul, /*prot=*/0ul,
      /*flags=MAP_NORESERVE|MAP_LOCKED|MAP_HUGETLB|MAP_FIXED|0x22*/ 0x46032ul,
      /*fd=*/-1, /*offset=*/0ul);
  res = syscall(__NR_userfaultfd,
                /*flags=UFFD_USER_MODE_ONLY|O_CLOEXEC|O_NONBLOCK*/ 0x80801ul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x200000c0 = 0xaa;
  *(uint64_t*)0x200000c8 = 0;
  *(uint64_t*)0x200000d0 = 0;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0xc018aa3f, /*arg=*/0x200000c0ul);
  *(uint64_t*)0x20000000 = 0x20400000;
  *(uint64_t*)0x20000008 = 0xc00000;
  *(uint64_t*)0x20000010 = 1;
  *(uint64_t*)0x20000018 = 0;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0xc020aa00, /*arg=*/0x20000000ul);
  *(uint64_t*)0x20000240 = 0x20c00000;
  *(uint64_t*)0x20000248 = 0x400000;
  *(uint64_t*)0x20000250 = 0;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0xc020aa08, /*arg=*/0x20000240ul);
  inject_fault(9);
  syscall(
      __NR_mmap, /*addr=*/0x20400000ul, /*len=*/0xc00000ul,
      /*prot=PROT_GROWSUP|PROT_WRITE|PROT_READ|PROT_EXEC*/ 0x2000007ul,
      /*flags=MAP_UNINITIALIZED|MAP_POPULATE|MAP_NORESERVE|MAP_NONBLOCK|MAP_FIXED|0x1021*/
      0x401d031ul, /*fd=*/-1, /*offset=*/0ul);
  return 0;
}
