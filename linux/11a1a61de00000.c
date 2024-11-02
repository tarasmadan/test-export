// https://syzkaller.appspot.com/bug?id=7b960555c96033c67abee42d6eb6a95f24df8e99
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/genetlink.h>
#include <linux/netlink.h>

static long syz_genetlink_get_family_id(volatile long name)
{
  char buf[512] = {0};
  struct nlmsghdr* hdr = (struct nlmsghdr*)buf;
  struct genlmsghdr* genlhdr = (struct genlmsghdr*)NLMSG_DATA(hdr);
  struct nlattr* attr = (struct nlattr*)(genlhdr + 1);
  hdr->nlmsg_len =
      sizeof(*hdr) + sizeof(*genlhdr) + sizeof(*attr) + GENL_NAMSIZ;
  hdr->nlmsg_type = GENL_ID_CTRL;
  hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
  genlhdr->cmd = CTRL_CMD_GETFAMILY;
  attr->nla_type = CTRL_ATTR_FAMILY_NAME;
  attr->nla_len = sizeof(*attr) + GENL_NAMSIZ;
  strncpy((char*)(attr + 1), (char*)name, GENL_NAMSIZ);
  struct iovec iov = {hdr, hdr->nlmsg_len};
  struct sockaddr_nl addr = {0};
  addr.nl_family = AF_NETLINK;
  int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (fd == -1) {
    return -1;
  }
  struct msghdr msg = {&addr, sizeof(addr), &iov, 1, NULL, 0, 0};
  if (sendmsg(fd, &msg, 0) == -1) {
    close(fd);
    return -1;
  }
  ssize_t n = recv(fd, buf, sizeof(buf), 0);
  close(fd);
  if (n <= 0) {
    return -1;
  }
  if (hdr->nlmsg_type != GENL_ID_CTRL) {
    return -1;
  }
  for (; (char*)attr < buf + n;
       attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
    if (attr->nla_type == CTRL_ATTR_FAMILY_ID)
      return *(uint16_t*)(attr + 1);
  }
  return -1;
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0x10);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000440, "ethtool\000", 8);
  res = syz_genetlink_get_family_id(0x20000440);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x20000780 = 0;
  *(uint32_t*)0x20000788 = 0;
  *(uint64_t*)0x20000790 = 0x20000740;
  *(uint64_t*)0x20000740 = 0x20000040;
  memcpy((void*)0x20000040, " \000\000\000", 4);
  *(uint16_t*)0x20000044 = r[1];
  memcpy((void*)0x20000046, "\x03\x07\x00\x00\x00\x00\x00\x10\x00\x00\x04\x00"
                            "\x00\x00\x0c\x00\x01\x80\x08\x00\x03\x00\x23\x00"
                            "\x00\x00",
         26);
  *(uint64_t*)0x20000748 = 0x20;
  *(uint64_t*)0x20000798 = 1;
  *(uint64_t*)0x200007a0 = 0;
  *(uint64_t*)0x200007a8 = 0;
  *(uint32_t*)0x200007b0 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000780ul, 0ul);
  return 0;
}