// https://syzkaller.appspot.com/bug?id=9f81f36264dc24fb50efce0818c8fe5d262d13fe
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
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

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

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

uint64_t r[3] = {0xffffffffffffffff, 0x0, 0x0};

void execute_one(void)
{
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  res = syscall(__NR_socket, /*domain=*/0x10ul, /*type=*/3ul, /*proto=*/0x10);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000080, "nl80211\000", 8);
  res = -1;
  res = syz_genetlink_get_family_id(/*name=*/0x20000080, /*fd=*/-1);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x200000c0, "wlan0\000\000\000\000\000\000\000\000\000\000\000",
         16);
  res = syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x8933, /*arg=*/0x200000c0ul);
  if (res != -1)
    r[2] = *(uint32_t*)0x200000d0;
  *(uint64_t*)0x20000100 = 0;
  *(uint32_t*)0x20000108 = 0;
  *(uint64_t*)0x20000110 = 0x20000140;
  *(uint64_t*)0x20000140 = 0x20000180;
  *(uint32_t*)0x20000180 = 0x24;
  *(uint16_t*)0x20000184 = r[1];
  *(uint16_t*)0x20000186 = 5;
  *(uint32_t*)0x20000188 = 0x70bd2c;
  *(uint32_t*)0x2000018c = 0;
  *(uint8_t*)0x20000190 = 6;
  *(uint8_t*)0x20000191 = 0;
  *(uint16_t*)0x20000192 = 0;
  *(uint16_t*)0x20000194 = 8;
  *(uint16_t*)0x20000196 = 3;
  *(uint32_t*)0x20000198 = r[2];
  *(uint16_t*)0x2000019c = 8;
  *(uint16_t*)0x2000019e = 5;
  *(uint32_t*)0x200001a0 = 3;
  *(uint64_t*)0x20000148 = 0x24;
  *(uint64_t*)0x20000118 = 1;
  *(uint64_t*)0x20000120 = 0;
  *(uint64_t*)0x20000128 = 0;
  *(uint32_t*)0x20000130 = 0;
  syscall(__NR_sendmsg, /*fd=*/r[0], /*msg=*/0x20000100ul, /*f=*/0ul);
  *(uint64_t*)0x200001c0 = 0;
  *(uint32_t*)0x200001c8 = 0;
  *(uint64_t*)0x200001d0 = 0x200002c0;
  *(uint64_t*)0x200002c0 = 0x20000600;
  *(uint32_t*)0x20000600 = 0xb0;
  *(uint16_t*)0x20000604 = r[1];
  *(uint16_t*)0x20000606 = 5;
  *(uint32_t*)0x20000608 = 0;
  *(uint32_t*)0x2000060c = 0;
  *(uint8_t*)0x20000610 = 0xf;
  *(uint8_t*)0x20000611 = 0;
  *(uint16_t*)0x20000612 = 0;
  *(uint16_t*)0x20000614 = 8;
  *(uint16_t*)0x20000616 = 3;
  *(uint32_t*)0x20000618 = r[2];
  *(uint16_t*)0x2000061c = 0x72;
  *(uint16_t*)0x2000061e = 0xe;
  STORE_BY_BITMASK(uint8_t, , 0x20000620, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000620, 0, 2, 2);
  STORE_BY_BITMASK(uint8_t, , 0x20000620, 8, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000621, 0, 0, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000621, 0, 1, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000621, 0, 2, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000621, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000621, 0, 4, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000621, 0, 5, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000621, 0, 6, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000621, 0, 7, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000622, 0x7fff, 0, 15);
  STORE_BY_BITMASK(uint16_t, , 0x20000623, 0, 7, 1);
  *(uint8_t*)0x20000624 = 8;
  *(uint8_t*)0x20000625 = 2;
  *(uint8_t*)0x20000626 = 0x11;
  *(uint8_t*)0x20000627 = 0;
  *(uint8_t*)0x20000628 = 0;
  *(uint8_t*)0x20000629 = 1;
  *(uint8_t*)0x2000062a = 8;
  *(uint8_t*)0x2000062b = 2;
  *(uint8_t*)0x2000062c = 0x11;
  *(uint8_t*)0x2000062d = 0;
  *(uint8_t*)0x2000062e = 0;
  *(uint8_t*)0x2000062f = 1;
  memset((void*)0x20000630, 80, 6);
  STORE_BY_BITMASK(uint16_t, , 0x20000636, 0, 0, 4);
  STORE_BY_BITMASK(uint16_t, , 0x20000636, 0, 4, 12);
  *(uint64_t*)0x20000638 = 0;
  *(uint16_t*)0x20000640 = 0x64;
  *(uint16_t*)0x20000642 = 0;
  *(uint8_t*)0x20000644 = 0;
  *(uint8_t*)0x20000645 = 6;
  memset((void*)0x20000646, 1, 6);
  *(uint8_t*)0x2000064c = 1;
  *(uint8_t*)0x2000064d = 0;
  *(uint8_t*)0x2000064e = 3;
  *(uint8_t*)0x2000064f = 1;
  *(uint8_t*)0x20000650 = 0x8c;
  *(uint8_t*)0x20000651 = 5;
  *(uint8_t*)0x20000652 = 3;
  *(uint8_t*)0x20000653 = 0;
  *(uint8_t*)0x20000654 = 0;
  *(uint8_t*)0x20000655 = 0;
  *(uint8_t*)0x20000656 = 0x2a;
  *(uint8_t*)0x20000657 = 1;
  STORE_BY_BITMASK(uint8_t, , 0x20000658, 0, 0, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000658, 0, 1, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000658, 0, 2, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000658, 0, 3, 5);
  *(uint8_t*)0x20000659 = 0x2d;
  *(uint8_t*)0x2000065a = 0x1a;
  *(uint16_t*)0x2000065b = 0;
  STORE_BY_BITMASK(uint8_t, , 0x2000065d, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, , 0x2000065d, 0, 2, 3);
  STORE_BY_BITMASK(uint8_t, , 0x2000065d, 0, 5, 3);
  *(uint64_t*)0x2000065e = 0;
  STORE_BY_BITMASK(uint64_t, , 0x20000666, 0, 0, 13);
  STORE_BY_BITMASK(uint64_t, , 0x20000667, 0, 5, 3);
  STORE_BY_BITMASK(uint64_t, , 0x20000668, 0, 0, 10);
  STORE_BY_BITMASK(uint64_t, , 0x20000669, 0, 2, 6);
  STORE_BY_BITMASK(uint64_t, , 0x2000066a, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000066a, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000066a, 0, 2, 2);
  STORE_BY_BITMASK(uint64_t, , 0x2000066a, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2000066a, 0, 5, 27);
  *(uint16_t*)0x2000066e = 0;
  *(uint32_t*)0x20000670 = 0;
  *(uint8_t*)0x20000674 = 0;
  *(uint8_t*)0x20000675 = 0x72;
  *(uint8_t*)0x20000676 = 6;
  memset((void*)0x20000677, 3, 6);
  *(uint8_t*)0x2000067d = 0x71;
  *(uint8_t*)0x2000067e = 7;
  *(uint8_t*)0x2000067f = 0;
  *(uint8_t*)0x20000680 = 0;
  *(uint8_t*)0x20000681 = 0;
  *(uint8_t*)0x20000682 = 0;
  *(uint8_t*)0x20000683 = 8;
  *(uint8_t*)0x20000684 = 0;
  *(uint8_t*)0x20000685 = 0;
  *(uint8_t*)0x20000686 = 0x76;
  *(uint8_t*)0x20000687 = 6;
  *(uint8_t*)0x20000688 = 0;
  *(uint8_t*)0x20000689 = 0;
  *(uint16_t*)0x2000068a = 0;
  *(uint16_t*)0x2000068c = 0;
  *(uint16_t*)0x20000690 = 8;
  *(uint16_t*)0x20000692 = 0x26;
  *(uint32_t*)0x20000694 = 0x96c;
  *(uint16_t*)0x20000698 = 8;
  *(uint16_t*)0x2000069a = 0x9f;
  *(uint32_t*)0x2000069c = 7;
  *(uint16_t*)0x200006a0 = 8;
  *(uint16_t*)0x200006a2 = 0xc;
  *(uint32_t*)0x200006a4 = 0x64;
  *(uint16_t*)0x200006a8 = 8;
  *(uint16_t*)0x200006aa = 0xd;
  *(uint32_t*)0x200006ac = 0;
  *(uint64_t*)0x200002c8 = 0xb0;
  *(uint64_t*)0x200001d8 = 1;
  *(uint64_t*)0x200001e0 = 0;
  *(uint64_t*)0x200001e8 = 0;
  *(uint32_t*)0x200001f0 = 0;
  syscall(__NR_sendmsg, /*fd=*/r[0], /*msg=*/0x200001c0ul, /*f=*/0ul);
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