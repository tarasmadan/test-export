// https://syzkaller.appspot.com/bug?id=6a37b51599e2722c38d28c5407ee31bfba6e3677
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <pthread.h>
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

#include <linux/futex.h>
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

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  int i = 0;
  for (; i < 100; i++) {
    if (pthread_create(&th, &attr, fn, arg) == 0) {
      pthread_attr_destroy(&attr);
      return;
    }
    if (errno == EAGAIN) {
      usleep(50);
      continue;
    }
    break;
  }
  exit(1);
}

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

typedef struct {
  int state;
} event_t;

static void event_init(event_t* ev)
{
  ev->state = 0;
}

static void event_reset(event_t* ev)
{
  ev->state = 0;
}

static void event_set(event_t* ev)
{
  if (ev->state)
    exit(1);
  __atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1000000);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, 0);
}

static int event_isset(event_t* ev)
{
  return __atomic_load_n(&ev->state, __ATOMIC_ACQUIRE);
}

static int event_timedwait(event_t* ev, uint64_t timeout)
{
  uint64_t start = current_time_ms();
  uint64_t now = start;
  for (;;) {
    uint64_t remain = timeout - (now - start);
    struct timespec ts;
    ts.tv_sec = remain / 1000;
    ts.tv_nsec = (remain % 1000) * 1000 * 1000;
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, &ts);
    if (__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
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
                            int* reply_len)
{
  if (nlmsg->pos > nlmsg->buf + sizeof(nlmsg->buf) || nlmsg->nesting)
    exit(1);
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg->buf;
  hdr->nlmsg_len = nlmsg->pos - nlmsg->buf;
  struct sockaddr_nl addr;
  memset(&addr, 0, sizeof(addr));
  addr.nl_family = AF_NETLINK;
  unsigned n = sendto(sock, nlmsg->buf, hdr->nlmsg_len, 0,
                      (struct sockaddr*)&addr, sizeof(addr));
  if (n != hdr->nlmsg_len)
    exit(1);
  n = recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0);
  if (reply_len)
    *reply_len = 0;
  if (hdr->nlmsg_type == NLMSG_DONE)
    return 0;
  if (n < sizeof(struct nlmsghdr))
    exit(1);
  if (reply_len && hdr->nlmsg_type == reply_type) {
    *reply_len = n;
    return 0;
  }
  if (n < sizeof(struct nlmsghdr) + sizeof(struct nlmsgerr))
    exit(1);
  if (hdr->nlmsg_type != NLMSG_ERROR)
    exit(1);
  return ((struct nlmsgerr*)(hdr + 1))->error;
}

static int netlink_query_family_id(struct nlmsg* nlmsg, int sock,
                                   const char* family_name)
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = CTRL_CMD_GETFAMILY;
  netlink_init(nlmsg, GENL_ID_CTRL, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, CTRL_ATTR_FAMILY_NAME, family_name,
               strnlen(family_name, GENL_NAMSIZ - 1) + 1);
  int n = 0;
  int err = netlink_send_ext(nlmsg, sock, GENL_ID_CTRL, &n);
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
    return -1;
  }
  recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0);
  return id;
}

static long syz_genetlink_get_family_id(volatile long name)
{
  struct nlmsg nlmsg_tmp;
  int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (fd == -1) {
    return -1;
  }
  int ret = netlink_query_family_id(&nlmsg_tmp, fd, (char*)name);
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

struct thread_t {
  int created, call;
  event_t ready, done;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    event_wait(&th->ready);
    event_reset(&th->ready);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    event_set(&th->done);
  }
  return 0;
}

static void execute_one(void)
{
  int i, call, thread;
  int collide = 0;
again:
  for (call = 0; call < 7; call++) {
    for (thread = 0; thread < (int)(sizeof(threads) / sizeof(threads[0]));
         thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        event_init(&th->ready);
        event_init(&th->done);
        event_set(&th->done);
        thread_start(thr, th);
      }
      if (!event_isset(&th->done))
        continue;
      event_reset(&th->done);
      th->call = call;
      __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
      event_set(&th->ready);
      if (collide && (call % 2) == 0)
        break;
      event_timedwait(&th->done, 50);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
  if (!collide) {
    collide = 1;
    goto again;
  }
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

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0, 0x0};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 2ul, 0xaul, 0x10);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    res = syscall(__NR_socket, 0x10ul, 3ul, 0x10);
    if (res != -1)
      r[1] = res;
    break;
  case 2:
    memcpy((void*)0x20000200, "nl80211\000", 8);
    res = -1;
    res = syz_genetlink_get_family_id(0x20000200);
    if (res != -1)
      r[2] = res;
    break;
  case 3:
    memcpy((void*)0x20000700,
           "wlan0\000\000\000\000\000\000\000\000\000\000\000", 16);
    res = syscall(__NR_ioctl, r[1], 0x8933, 0x20000700ul);
    if (res != -1)
      r[3] = *(uint32_t*)0x20000710;
    break;
  case 4:
    *(uint64_t*)0x20000340 = 0;
    *(uint32_t*)0x20000348 = 0;
    *(uint64_t*)0x20000350 = 0x20000300;
    *(uint64_t*)0x20000300 = 0x20000480;
    *(uint32_t*)0x20000480 = 0x50;
    *(uint16_t*)0x20000484 = r[2];
    *(uint16_t*)0x20000486 = 5;
    *(uint32_t*)0x20000488 = 0;
    *(uint32_t*)0x2000048c = 0;
    *(uint8_t*)0x20000490 = 6;
    *(uint8_t*)0x20000491 = 0;
    *(uint16_t*)0x20000492 = 0;
    *(uint16_t*)0x20000494 = 8;
    *(uint16_t*)0x20000496 = 3;
    *(uint32_t*)0x20000498 = r[3];
    *(uint16_t*)0x2000049c = 8;
    STORE_BY_BITMASK(uint16_t, , 0x2000049e, 0x17, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x2000049f, 0, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x2000049f, 1, 7, 1);
    *(uint16_t*)0x200004a0 = 4;
    *(uint16_t*)0x200004a2 = 2;
    *(uint16_t*)0x200004a4 = 0x24;
    STORE_BY_BITMASK(uint16_t, , 0x200004a6, 0x17, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x200004a7, 0, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x200004a7, 1, 7, 1);
    *(uint16_t*)0x200004a8 = 4;
    *(uint16_t*)0x200004aa = 6;
    *(uint16_t*)0x200004ac = 4;
    *(uint16_t*)0x200004ae = 1;
    *(uint16_t*)0x200004b0 = 4;
    *(uint16_t*)0x200004b2 = 4;
    *(uint16_t*)0x200004b4 = 4;
    *(uint16_t*)0x200004b6 = 6;
    *(uint16_t*)0x200004b8 = 4;
    *(uint16_t*)0x200004ba = 2;
    *(uint16_t*)0x200004bc = 4;
    *(uint16_t*)0x200004be = 4;
    *(uint16_t*)0x200004c0 = 4;
    *(uint16_t*)0x200004c2 = 4;
    *(uint16_t*)0x200004c4 = 4;
    *(uint16_t*)0x200004c6 = 5;
    *(uint16_t*)0x200004c8 = 8;
    *(uint16_t*)0x200004ca = 5;
    *(uint32_t*)0x200004cc = 6;
    *(uint64_t*)0x20000308 = 0x50;
    *(uint64_t*)0x20000358 = 1;
    *(uint64_t*)0x20000360 = 0;
    *(uint64_t*)0x20000368 = 0;
    *(uint32_t*)0x20000370 = 0;
    syscall(__NR_sendmsg, r[1], 0x20000340ul, 0x40040ul);
    break;
  case 5:
    memcpy((void*)0x20001000,
           "wlan0\000\000\000\000\000\000\000\000\000\000\000", 16);
    syscall(__NR_ioctl, r[0], 0x8914, 0x20001000ul);
    break;
  case 6:
    *(uint64_t*)0x20001280 = 0;
    *(uint32_t*)0x20001288 = 0;
    *(uint64_t*)0x20001290 = 0x20001240;
    *(uint64_t*)0x20001240 = 0x20000080;
    memcpy((void*)0x20000080, "\xb0\x0f\x00\x00", 4);
    *(uint16_t*)0x20000084 = 0;
    memcpy((void*)0x20000086, "\x00\x00\x00\x96\x52\x00\x03\x00", 8);
    *(uint32_t*)0x2000008e = 0;
    memcpy(
        (void*)0x20000092,
        "\x06\x00\x66\x00\x00\x00\x00\x00\x04\x00\x67\x00\x6a\x0f\x33\x00\x70"
        "\xb5\xa3\x6f\x21\x6e\x34\xd2\x33\x8f\x4e\xd9\xa7\x77\x4e\xac\x4e\x6f"
        "\xfc\xd3\xf6\xaa\xa3\xc3\x4f\xf3\x38\x9d\x2c\x41\x46\xf0\x78\xa5\xdc"
        "\xd1\xad\x60\x61\x13\x94\x19\xb2\xc3\xf8\xa1\x6f\x7e\x68\x20\xf5\x1f"
        "\x61\xb2\x5f\xe2\xf0\x33\x44\xe6\xac\x3c\x42\x2b\xb3\xe6\xc6\xb7\xb2"
        "\x36\xf2\x21\xca\xcf\xfa\xd5\x43\xa4\x91\x18\xc9\x4a\x01\x2a\xda\x60"
        "\xb6\x10\x27\x8b\xd3\x48\xb2\xc0\x12\x4f\x59\x80\xde\xc1\x6b\xd7\x0c"
        "\xff\xf5\xbe\xc9\x9c\x20\xb0\x7d\x91\xd3\xca\x14\xfe\x18\x79\x54\x20"
        "\x5d\xe7\xdb\x7f\xdc\x9f\x80\x6a\x20\x0f\x99\xaa\xf3\x0e\x36\xd3\x4c"
        "\x89\x86\x9c\xf1\xe9\x5f\x55\xb3\x83\x23\x5b\x5c\x07\xcf\xc7\xde\xc8"
        "\xea\xad\x71\x93\x46\xd1\xed\xd5\x61\xa8\x66\x1d\x57\xe6\x21\xf4\xd2"
        "\x56\x87\xfa\x5b\x24\x5a\xd1\xb8\x9b\x58\x73\x65\x35\x63\x54\x43\x48"
        "\x09\xd6\x40\x20\xa9\x49\x0f\xca\xed\x02\x76\x89\x0b\x55\xe7\x16\x0a"
        "\x60\x70\xab\xc8\xac\x01\xd1\x5b\x12\x0f\x0b\xd5\xf4\xff\x05\xc6\x3d"
        "\xcf\xc8\xba\x95\xd3\x6e\xd0\x8c\x5f\x29\x93\x7b\x29\x3b\x0d\x24\xce"
        "\x7c\xde\x1b\xeb\x2d\x65\x60\x36\x5b\x59\x0b\x29\x0a\x62\x81\x8b\x5a"
        "\x3b\x8d\x87\x45\x57\x81\x60\x71\x10\x45\x14\x7b\x2f\x54\xba\x95\xba"
        "\x71\x22\xf7\x65\xc5\xa5\xea\xa1\x53\x1c\xef\x8f\x67\x69\x9b\x47\x60"
        "\x65\xfd\x16\x75\xec\xe8\x67\x6e\xac\xa8\xc4\x53\x72\x59\x04\x3c\xa7"
        "\x9d\x8a\xb4\x77\xaf\xca\xe2\x65\x17\xd1\x4a\x99\xa4\x90\x8f\x67\x79"
        "\x45\x9d\x53\xf6\x71\x35\x91\xd8\x36\x15\x73\x1d\x46\x3c\xaa\xf7\x74"
        "\xa9\xd6\x6f\xb7\x83\x45\xc3\xd3\x0e\xe4\x29\xbb\xa1\xd4\xf6\xb6\x85"
        "\xcb\x98\x45\xef\x52\xc8\x6e\xb0\x89\x87\x17\x75\x60\x38\xa3\x2f\xab"
        "\x83\x50\xe6\xbf\xa5\xf6\x2d\x8f\xea\xd8\xe7\xc8\xf2\xf3\x06\xbd\x73"
        "\xcc\xda\x6b\xe2\x9f\xff\x8a\xe8\xee\x0c\x77\x9a\x35\x8e\x2c\xb4\x94"
        "\xef\xcd\x64\x59\xa6\x02\x36\x1e\x14\xca\x56\xf5\x51\x26\x8e\xd2\xbd"
        "\xc7\x03\x0f\xb6\xa4\x5e\xa9\x7a\x68\xec\x61\xb6\x47\x14\x53\x1f\x6b"
        "\x3b\x76\x08\x97\xb0\x0e\x81\xee\xd5\x9a\xb9\xd4\xa1\xc3\x5b\x5f\x1e"
        "\x76\x96\xcf\x71\xf3\x11\xbb\xb3\xdb\x1e\xfe\x24\x51\x95\x23\xdf\xb5"
        "\xf8\x27\xde\x6a\x78\xf7\x7d\x50\x91\x09\xce\x28\x37\xa6\x1e\x5b\x5b"
        "\xc8\x03\x39\x55\xaf\xfd\x68\xf9\x3d\x02\x67\xb7\x30\x82\xfe\x1f\xf1"
        "\xd3\x4d\xa9\xed\xf1\xe6\xd9\x6a\xd0\xa1\x84\xba\x41\x04\xf2\x7a\x55"
        "\x95\xa6\x3e\x30\x24\xe6\x7d\x68\xee\xf8\x79\x93\x47\x64\xb1\xf4\x39"
        "\xed\x57\xe4\x61\xab\x72\xe2\xa8\x2c\xa7\xf0\xc8\x9f\x74\x90\xb6\x5a"
        "\x1d\xfa\xc3\x6d\xff\xab\x60\x76\x5b\xdd\x0f\xe3\x55\x2a\xf2\x70\x3a"
        "\xb0\x72\xa9\xe8\xc3\xbd\x5e\xc6\x29\xb8\x48\x3b\xf9\x8c\x11\xfc\xe7"
        "\x90\x69\xe2\x86\x85\xb6\xe5\xdf\x08\xd4\xd7\xe4\xd3\xe9\xab\xcc\x65"
        "\xa9\x7b\x09\xfe\x06\x5d\xec\x3b\x22\xbb\x32\x99\xb7\x75\xcb\xa1\x5a"
        "\x48\x1b\x8c\x7d\x8d\x7f\x0a\xb4\xa0\x02\x2c\xf3\x47\x0e\x99\x05\x42"
        "\x3f\x9d\x02\xa9\xb7\x47\x66\xac\xf8\x0b\xce\x07\x2a\xa4\x53\xe8\xae"
        "\xa6\x11\xc0\x18\x1f\xe2\x2a\x3d\xcf\xb4\xc4\x29\xd0\x28\xb0\xf6\x83"
        "\x8e\xc4\x10\x08\xea\xbe\xcc\x80\x1f\x3e\xa5\xb7\xab\x61\x4e\x19\xa6"
        "\x06\x7a\xc4\x22\x05\xfe\xde\x87\x3c\xd4\xc3\x6d\xb5\x3d\xd2\x1d\xbc"
        "\x8c\xeb\xf3\x99\x6a\x7a\x22\x7f\x06\x4e\xef\xd9\x32\x89\x9f\xda\x3b"
        "\xbd\xf7\x00\xef\x81\x8d\xbb\xaf\xc2\x11\xf1\x4b\xad\xd7\x32\x14\xc9"
        "\x14\x4d\xda\x61\x8e\xd4\x53\x84\x20\x58\xe8\x96\x12\xe5\x18\xb1\xa1"
        "\x2d\xb6\xd8\x9a\xfd\x16\x1b\xc0\xe0\x5e\xf6\x3a\x0d\xb7\x7c\x5a\x44"
        "\x4c\xa5\x06\x45\x46\x64\x4b\x0e\xce\x24\xe5\xcd\xc4\x18\x44\x13\x23"
        "\x9e\xa5\x71\x2c\x33\x9b\x70\x89\xbe\xd5\x0e\x83\xf3\xb8\x10\x7a\xa1"
        "\x3a\xbf\xb2\xf6\x70\xb7\x48\xd7\xa7\x31\x6e\x2d\x98\x67\x2f\x26\x8a"
        "\xc8\xee\x86\x23\x97\x93\x27\x89\xb2\x69\x01\xde\x60\x33\x46\xdd\x9d"
        "\xa0\x11\xa8\x04\x42\xce\xe6\x69\x28\xab\x43\xa3\x76\x86\x14\x56\x54"
        "\x68\xf0\xda\xf2\xcb\x99\x32\x15\x34\xc2\x69\x91\x56\xf6\xe1\x10\x92"
        "\x0b\x93\x1d\x5d\x1b\xf5\xed\x54\xa4\xbd\x02\x92\xff\xb5\x8b\x40\x8d"
        "\x42\xb0\x8a\x39\x26\x51\xd2\x7d\x0e\xa5\x3e\xd4\x91\xd1\xc2\x11\xb4"
        "\x0d\x22\x02\x19\x7b\x02\xe0\x0a\x3c\x88\x72\x47\x4d\xc7\xf7\x4a\x5a"
        "\xe3\x38\xe9\xf6\x2d\x92\xff\x70\x7a\x7a\xac\x38\xd3\xa7\x97\x67\x7a"
        "\xc9\x52\x97\x1f\xec\x14\x7a\x6d\x55\x16\xf5\x2a\x82\x44\xa7\x3d\x6c"
        "\x6f\x74\x2a\x4d\xa9\x1c\x73\x3f\xc0\xe0\x04\x6e\xf1\x05\x23\x91\xc3"
        "\xb2\xb9\x7b\xde\x71\x52\x43\x14\x73\x0c\x0c\x62\xa6\x0c\xf4\x81\x7a"
        "\x3b\x48\xdf\xee\xfb\xf8\x27\xf1\x3d\xef\xc6\x86\x3a\x7a\x3b\x8b\xbb"
        "\xcc\x4b\x6c\xee\x2c\xe5\xc9\xa5\xab\xae\x2b\xa6\x5d\x16\xf4\xce\x55"
        "\x00\xd6\x31\x41\x87\x5f\xcc\x62\xd5\x33\x7e\x62\x76\x84\xc0\x14\x0a"
        "\x52\xcd\x93\xdb\xdf\x00\x39\xed\xd2\x29\x83\x03\x44\x5b\xe7\xb5\x06"
        "\x34\x5a\x58\x4d\x4a\x32\xbf\xc1\x1e\xe2\xb5\xc3\x7b\x1a\x96\x7a\xf2"
        "\x98\xb8\x10\x44\xff\x3e\xdf\x80\x7e\xc5\x8d\xc9\xf6\x9b\xa1\x84\xd5"
        "\x87\x31\xe8\xc4\x25\xff\x4a\x57\x0b\xb1\x88\xf3\xb2\xae\xdc\x34\xcf"
        "\x84\x79\x7c\x78\xa6\xc7\x89\x27\x6d\x1b\x25\x99\x38\x1c\xc5\x44\x7b"
        "\x3b\xe6\x59\xc8\xea\x44\x2c\x79\x79\xe5\xb7\xa3\x7f\xad\xda\x42\xbc"
        "\x05\xee\x3c\x49\x04\x0b\xbe\x8c\xc8\x34\x74\x5c\xb0\xfa\x79\xf9\x3f"
        "\x39\xa2\x26\xe5\x16\x36\x34\x27\xa8\x00\x15\x79\x85\x03\x7a\xab\xd4"
        "\x71\x6b\xe8\x2c\xd2\xc3\x3d\xd9\x97\x4d\x84\x5a\xd8\xb6\xaa\x7e\xcf"
        "\x11\xc8\x17\xf1\xca\x04\x76\xd3\x5a\x25\x46\x12\xe1\x4e\xb1\x96\x6d"
        "\xd7\xb5\x8e\xb9\xda\x1f\xae\x32\xe2\x20\xb7\x4f\x73\x9d\xd0\x24\xca"
        "\xeb\x79\x29\xcb\x50\xd6\x0f\xba\x4d\x5a\xe5\x46\xae\x8e\x08\xcd\x49"
        "\x0a\x18\x65\x73\xe9\x40\x59\x6e\xb9\xc7\xf5\xba\x08\xb7\xcc\x0c\xa0"
        "\x5c\x65\x86\xaf\xa3\xc0\x22\xf8\x19\xee\xe6\x4a\x15\xff\x8b\x43\x75"
        "\x7a\x98\xcc\x8b\x2d\xd1\x9c\x74\x07\xf9\xc8\xd2\xac\xd7\xda\x16\xd3"
        "\x53\xdf\x5e\x6e\x15\x4a\xcc\x82\x9f\xa7\x51\x3a\xef\x61\x77\x59\x7e"
        "\x85\x74\x4d\x9a\x2a\xce\xd9\x6b\x76\x71\x04\xbf\xc1\x3b\x5b\xd7\x36"
        "\x94\xf0\x09\x1b\x5f\x3c\x7d\x6d\x98\x51\xf7\x7f\xd8\xc9\xf4\xf7\x09"
        "\xc7\xec\x3d\x30\x9c\xe8\xd5\xf2\x65\x96\x76\x2f\xf6\xad\x74\x3c\x82"
        "\xec\xa3\xf2\xc8\x22\x1c\x39\x58\x0f\x88\x0b\xcc\x3f\x57\xb4\x21\x7b"
        "\x33\xcf\x88\x4a\x9c\x8b\x95\x4d\x40\xb1\x2e\x48\x37\xdd\x8e\xf1\x78"
        "\x16\xab\x6f\x17\x9e\xf9\xc8\x1f\x31\x89\x5f\x58\xdd\x3f\x5f\xd0\xb3"
        "\x74\x4d\xc5\xe4\x7b\x72\x39\x56\xf4\xf7\x5a\xfe\x25\x9f\x80\x9d\x90"
        "\xd2\x60\x2b\xd9\x4e\x18\x14\x94\x5a\xee\xde\xb3\x19\x96\xab\x7e\xee"
        "\xa3\x58\xd5\x11\xd7\x16\x66\xb3\x60\xab\xa4\xd9\xc2\xc8\x90\x9d\x49"
        "\x68\x43\xdd\x4c\xb4\x34\x96\xcd\xc9\x3b\x36\xd8\x1d\x75\x6b\x0e\xb1"
        "\x63\xc4\x0a\x32\x77\xdc\xb2\xad\xe8\x6d\x61\x45\x91\x23\x27\x09\x4b"
        "\x1c\xbd\xf6\x38\x71\x8f\xaa\xa1\x8b\x10\x6c\x9a\x53\xd0\x90\x22\x48"
        "\x4b\xeb\x23\xa2\xfc\x5c\x87\xa6\x36\xb1\x11\x50\x9d\x41\xa0\xc3\x39"
        "\x33\x5c\xd6\x00\x29\x4f\xf8\xf5\x9f\xb1\x55\x60\x13\x67\x97\xaa\x2f"
        "\x8a\xf4\x05\x22\xa2\x22\xfc\x1e\xb8\xae\x37\x34\x42\x21\x02\x28\xc0"
        "\xf0\xc6\x1f\x1e\x4e\x19\xb8\xbd\x88\xc2\x39\x14\xec\xa9\xdd\x55\xcb"
        "\xd1\xc9\x89\xc1\xa6\x36\x30\x80\x77\x6b\xf3\x7e\xfa\x09\x1b\x4a\xd3"
        "\x2f\xbc\xe1\x47\x0b\x25\xc2\xb8\xe7\xd4\xdc\xb2\x4b\x23\x35\xe3\x23"
        "\xef\x90\x91\xe9\xdf\xc8\x8e\x32\x08\x5a\x64\xcc\x30\xcd\xde\xb5\xc3"
        "\xe1\xb8\x05\xc7\xfc\xa0\x7e\xaf\xdc\xcf\x9c\x15\x1e\x0d\xeb\x8f\x01"
        "\x7b\x6c\xbc\x5a\x74\x20\x8f\xd9\xa8\x27\x67\x35\x7c\x91\x36\xdc\x3c"
        "\xaa\xd6\x6d\x8e\xf9\x54\xbc\x44\x3a\xc8\x04\x52\xf1\x96\x4f\xa4\xd0"
        "\xf2\x53\xcd\x43\xa9\x09\x3b\x2f\xf2\x96\x7c\x30\x86\x57\x87\x44\xc3"
        "\x90\x1d\xb8\x37\x8a\xd7\x0b\xc6\x42\x37\x23\xad\x37\x93\x12\x24\x25"
        "\xb5\x2c\x80\x42\x2c\xfc\xaf\x9f\xd0\x26\xf0\x20\x14\xa8\x21\x59\x88"
        "\x8e\x95\xbd\x56\x31\x6c\xd7\x15\x7a\xa3\x7e\xf2\xdd\x0b\xb1\xb5\x12"
        "\xbd\x21\x51\xd2\x67\xec\x39\x62\xa7\x61\x67\xa0\x7f\x34\x4a\x0d\x20"
        "\x81\x51\xe5\x9e\x38\x86\x08\x2f\xa4\x06\xbc\x40\x0c\xbf\xb9\xbd\xf4"
        "\x0c\x88\xc5\xbb\xf0\x80\xb6\x60\x5d\xe0\x2f\xe0\x61\x66\x58\x92\x8e"
        "\x3e\xaa\x2f\x30\x25\x6f\x61\xda\x44\x51\x47\xc3\x37\xc9\xf6\x50\x84"
        "\x4f\x38\x27\xe8\x42\x5a\x71\x6f\xbc\x89\xd3\x01\x1f\xbb\x4a\xce\x78"
        "\x96\x00\x81\x6c\x79\x33\x60\x67\x4c\x90\x90\x82\x1e\x05\x0e\x51\x3f"
        "\x0b\x2a\x37\x43\x88\x9c\xf1\x39\x37\x44\x78\x94\xdb\x60\x8f\xca\xed"
        "\x93\x52\xb4\xfb\x35\xf1\x6f\xd7\xb7\x73\x17\x52\x2d\xe4\xf0\xc1\x11"
        "\xf5\x8b\x04\x99\xe2\xa7\x9a\x3a\x0a\xe2\x71\x38\x20\x0f\x2b\x4c\xca"
        "\x0d\xae\x3b\xa4\x6f\x4d\xdb\x6d\x11\x54\x28\x1b\x9b\x93\x5b\x85\xba"
        "\xcf\x55\xd8\x33\xcc\x78\xe0\x51\x15\x04\xff\x9e\xc9\x6d\x9b\x52\x3b"
        "\x78\x28\x43\x06\xdf\xf1\xd3\x94\x0d\x2f\xbf\x0e\x19\xde\xd4\x80\x40"
        "\xb9\xc7\x46\x33\xd9\x20\x38\x32\xdc\x66\xd2\xff\x1d\x29\x4a\x50\x67"
        "\xdf\xca\x76\xc8\xbf\x96\xa4\x4b\x1e\xf7\x59\x9f\xe0\x07\x1b\x5c\xc1"
        "\xb0\x4b\x7a\xc0\xf4\x5b\x44\xe7\x7c\x5f\x17\x69\x36\xc0\x18\x39\xf8"
        "\x81\x56\xb5\xef\xe0\x91\xc5\xb5\x54\xda\x74\x31\x67\x2a\xd0\x96\x31"
        "\x85\xf6\x78\x46\xca\xc2\xeb\x5f\x01\x28\xff\xf8\x73\xdf\x60\x24\x5c"
        "\x38\x95\xe5\x40\x8d\x16\x3a\x22\xc6\x50\x85\xaa\xe3\x64\x95\xca\x94"
        "\x0b\x72\x3c\x25\x67\x95\x59\x78\x28\xee\x7d\x79\x49\xa4\x43\xa4\x47"
        "\xc2\xd4\x77\xf8\x6b\x5c\x48\x7f\x50\xb0\x5f\xf9\x27\xfc\xd3\x28\xc8"
        "\x60\x01\xe6\x0f\x56\xa4\x9f\x03\x93\xe4\x93\x1e\x1d\xdd\xb2\x83\x3f"
        "\x07\x41\xa1\x0c\x7d\xe5\xcd\xb4\x37\xdd\xb2\xd8\xcd\x0a\x28\xb1\x4d"
        "\x9b\x93\xd1\x6d\xf4\x9a\xec\x8e\x7a\xd8\x08\xb2\xc7\x37\x37\x90\xf3"
        "\x3d\x8c\x47\x54\xad\x8b\x51\x83\x3f\x4e\x1d\x47\xdc\x21\xea\x12\x64"
        "\x33\xf1\xaa\xbc\x47\x19\xdc\xdc\x5b\x0d\x0b\x31\x29\xe7\x2f\x4a\x9a"
        "\xe4\xc6\x6b\xc3\x67\xa8\x62\xc8\x16\xef\xaa\xf4\xec\x64\x1d\x1c\x2c"
        "\xf0\x00\xd5\x4f\x9e\xc0\xf6\xda\x5b\x30\x4f\xd2\xce\x86\x4e\xe2\x5d"
        "\x60\x3a\xe0\x2a\xda\xbb\xa3\x1f\xb8\x43\x13\xe3\xfd\xa9\x70\xbd\x5f"
        "\x09\x47\xfa\x4e\x8d\x3e\xed\x68\xff\xbb\x1b\x95\xe1\xd6\x2f\x11\x4c"
        "\x08\x5d\x79\x52\x97\x5c\xac\xc3\x58\x0a\x86\x5a\x1b\x4d\xc4\x76\xd0"
        "\xe1\x40\x24\x23\xe8\x5b\x90\xc1\x79\xb3\x3b\x53\x13\x0c\x2d\xa6\x33"
        "\x4c\x3e\x9b\xef\xb7\xe5\xe4\xb6\x4c\xa3\x7e\x45\x23\xbb\xf8\x14\x3f"
        "\x3a\xc1\x25\x6a\xdc\x47\xf3\x71\xb6\x89\x6d\xc2\x88\x91\x98\x97\xaf"
        "\x33\x44\x5c\x28\x77\xb1\xc4\x71\xb6\xd8\x74\xaa\x60\x10\xe4\xd7\xf7"
        "\x91\xba\x12\x0c\x4b\x57\xb5\x6e\x21\x4a\x48\x06\x07\xde\x62\x9a\xd6"
        "\xb3\x9a\xea\xf1\x9c\xea\x4f\x25\x8a\x02\x66\x72\xb6\x49\x4d\xa6\x90"
        "\xb5\x9a\x63\x8d\x70\xbd\x44\x1f\x38\xdd\xf2\xb3\x9f\x6e\xc2\x51\xbb"
        "\xc5\x10\x71\x45\x12\x39\x26\xd9\x74\xf0\x9a\xa7\xe4\x56\xcf\xe4\xa2"
        "\xce\x51\xf3\x2f\x4e\x24\x7b\x4d\xf5\xd3\x6b\x96\xf0\x72\x67\x67\xb6"
        "\x63\x9f\x66\x8c\xa6\xf4\x4f\x88\x4c\x35\x9f\xa2\x96\x15\x48\xd2\x55"
        "\xbe\xa5\x61\x67\x45\x1a\x1b\xfc\x98\xa6\xdb\x83\xc8\x29\x68\x9b\xc3"
        "\xce\x44\x2d\xb8\x53\xf7\x2a\x81\xcd\x25\x41\xe9\xff\x17\x9c\x60\xa5"
        "\xc8\xe1\x3a\x0d\x94\x16\x55\xd1\x3b\xeb\xfa\x3c\x65\x20\x3b\x4a\x17"
        "\x4e\xd0\x11\x98\x9e\xf1\x0d\xd1\x03\xa8\x05\x24\xd9\x89\xae\x09\xa0"
        "\x07\xa4\x81\x05\x31\xfe\x77\x0d\x78\x2b\x07\xe6\xa3\x8f\x11\xd3\x80"
        "\x9a\xe3\xcb\x8b\xa7\xc0\x77\x47\x57\xb3\x8f\x41\x2a\x70\x25\xf7\x43"
        "\x49\xc1\x91\x3a\x27\xe7\xfa\x5a\x2e\x14\x05\x76\xcc\x3c\x50\x34\x86"
        "\x24\x20\xd3\xa9\x1e\x77\xf4\xfe\x94\x20\x23\xbf\xfe\xeb\x01\x2b\xee"
        "\x60\x72\xf3\x02\xd0\xae\x46\x10\x9b\x00\xd9\xd4\xa1\x52\x07\x60\x57"
        "\x45\xd7\x8f\x7b\x20\xd0\x51\xa5\x78\xd2\x35\x35\x5b\xab\x8a\xa5\xcb"
        "\x52\xd6\xa1\x8e\x6b\x63\xc1\xeb\x2e\x3a\x84\x40\x87\x5e\x93\x51\x7b"
        "\x68\x79\x4f\x3e\xcb\x11\x1b\xb6\x50\x5b\x98\x97\xf8\xad\xc1\x93\x70"
        "\x53\xa0\xf9\x65\xb6\x70\xb5\xb8\xfe\x98\x01\x4e\xec\x35\x54\x02\xdf"
        "\xd2\x00\x32\x37\x96\x98\xe3\x09\x3d\x09\xbb\x2f\xb2\x57\x92\xf4\x4b"
        "\x37\x28\xca\x1c\x1d\xcc\x9d\xc4\xf2\x38\x78\xb2\x59\xc4\xe9\x60\xd3"
        "\x12\xab\x1e\xb6\x3c\x05\xee\x76\x84\x20\x5e\x68\x03\x97\xfb\x58\x29"
        "\xf9\x5d\xa9\xf5\x4c\xd8\x39\x39\xd9\x4b\x29\x1a\x8a\x0a\xca\x75\x18"
        "\xa9\x9b\xf9\x65\x95\x06\xd0\xf8\x8b\xf6\xd7\x8f\x84\x92\xe9\x09\x55"
        "\x40\xb5\x70\xb5\xe5\x9c\xc8\x1c\x09\x72\x7b\xee\x7a\xf2\x68\x90\xf7"
        "\xa2\x9f\x53\x50\xe6\xbb\x39\xea\xf2\xed\xbb\x69\x58\xcb\x33\x6b\x51"
        "\xba\xe3\x81\xdc\x12\x3c\x2f\xa0\xf4\xa7\x75\xa2\xcb\x6a\xfb\xd4\x3b"
        "\x29\xfc\xab\xe1\x7b\x51\xb4\x63\xac\x74\xd0\xbc\x31\x2a\x46\x75\x95"
        "\x03\x3f\x9c\xec\x6a\x76\xba\xc8\xe0\x7d\xf7\x62\xd6\x06\x1c\x7b\x0e"
        "\x41\xc8\x3f\x8b\xec\x32\x5a\xf1\xa8\x05\xde\x83\xc8\x45\xe5\x50\xb8"
        "\x2c\x4e\xdb\xf3\xbd\xec\x44\x28\xe6\xca\x12\x25\xdc\xff\xcc\x5a\xf7"
        "\xb3\xf1\x9b\xe8\x85\xa6\x2b\x6c\xf2\x3c\x48\x06\x17\x9b\xd0\xae\x8d"
        "\x27\xe3\x97\x1b\x41\xa6\xb7\xd4\xf8\x5c\xe0\xb7\x8c\xc5\xf9\x83\x08"
        "\x60\x4c\x2f\x9b\xe2\x4a\x57\x3f\x71\x49\x6f\xd8\xd1\xb9\xd2\x2b\x6f"
        "\xb8\x46\x12\xaf\xee\x23\x32\x3e\x49\x97\x30\xb0\x81\x31\x92\x7d\x7a"
        "\x91\x97\x71\xe9\x10\x0a\x42\x43\x1a\x22\xe0\xb2\xc0\x6d\x69\x9e\x40"
        "\xa3\xec\x97\xec\x66\x65\xd2\xa5\xa3\x54\xd2\x4f\x66\xf3\xae\xdb\xfc"
        "\xbd\x61\xd0\xa9\xb9\xa3\x78\xa5\xbf\xd2\xc6\xee\x1d\x9d\xfb\x89\xc9"
        "\xe6\x3e\x7c\xe1\xd2\x3e\x8c\xd9\xa8\x9c\x9c\xe5\xe0\x9a\x7b\x10\x88"
        "\x51\xcf\x75\x6d\x55\x30\xad\xb4\x4e\x5a\xda\x79\x5d\xd6\x86\x18\x57"
        "\x7a\xbb\x03\x65\x89\x50\x33\xb8\x8f\x70\xda\x31\x51\x1a\x58\xf5\x36"
        "\xec\xf1\x28\x9f\x85\x86\x5e\xcf\xff\x3b\xc0\x10\x27\xfd\x2c\x9a\xb1"
        "\x6c\xd6\xe0\x19\x82\xee\xc0\xf8\xc9\x5d\x43\x39\x70\x10\x98\xb3\x27"
        "\x33\x54\x7e\x79\x07\xbc\x3a\xb4\x7b\x48\xd5\x76\x85\x13\x65\x53\x0a"
        "\xab\x58\x11\xd8\xd8\x65\xb3\x20\xe9\x1c\xc8\x0e\xb0\xa6\x18\xb5\x19"
        "\x00\x4f\x99\x43\x34\xf2\x56\x33\x7b\x56\x0e\x7f\x8a\x6c\xc7\xeb\x81"
        "\xb7\xa3\x5b\x54\x1e\xc5\xd2\xe8\xb7\xf8\x9e\x92\xc0\xc0\xca\x58\x01"
        "\xb8\x2b\x38\xf2\x9c\x15\xe7\x55\xdc\x18\x33\x8c\x4f\xd9\x2d\x0a\xd6"
        "\x75\x21\x3e\xc6\x2f\x47\x1d\x83\x11\x48\x90\x67\xdc\xac\x98\xa2\xa2"
        "\x7f\x54\x4a\xc4\xbb\x06\x09\x55\x2a\x72\x75\x5d\x38\x44\xad\xd5\xfb"
        "\x1e\xff\x57\xad\xb0\xc8\xf8\xd3\x91\x50\xf7\x88\xf6\x59\xc0\x0e\x9d"
        "\x4b\xe2\x3c\xea\xbb\x45\x40\x9d\xef\xa5\xc8\xac\x05\x96\x0f\xf8\x0a"
        "\xa3\x64\xd8\xc5\x26\xeb\xa5\x59\x03\x36\xa5\x4d\xb8\xdf\x42\xed\xa7"
        "\x0e\xd9\x17\x21\x04\x51\x3a\x70\x52\x80\x66\x37\x42\xbd\x96\x61\x3f"
        "\x39\x9c\xf6\x7b\x7a\xd4\x8c\x79\x0f\xfd\x9f\x7c\xb7\xe0\xcd\xb3\xa6"
        "\x20\x35\xd9\xb3\x76\x11\x5f\x60\x0c\xe0\x4f\x25\xb0\x86\xa1\xa2\x36"
        "\x23\x17\x80\xf5\x9e\x02\x03\x8c\x7d\x78\xd0\xae\x43\xfc\x39\x0f\x32"
        "\x3b\xda\x68\xad\xbf\xe4\xee\x61\x67\xfa\x93\xe9\x09\x85\xa6\x21\xe7"
        "\x8a\xa5\x0f\xba\x64\xac\x08\x4a\x02\x94\xd2\x5c\xe2\x0b\x09\xfd\x49"
        "\x2e\x03\x4c\x09\x6f\xde\x4b\xbf\x46\x65\x51\x2b\xb7\x3a\x84\x1a\x38"
        "\xa7\x73\x75\x1b\x4b\x53\x53\x2e\x12\x51\xac\xbf\xb1\x50\xf9\x68\x9b"
        "\xd5\x39\x3a\x8c\x30\xef\x5b\xae\xff\xae\xc1\x60\x39\x6e\xee\xa4\xfd"
        "\xbe\xe6\x44\x50\xfb\x93\x67\x9d\x52\xb2\xcc\xda\xbc\xbd\x6b\xac\xd3"
        "\x52\x6d\x8f\xe5\x66\x6d\xd1\x19\x80\x22\x8e\xc9\x90\x8f\x76\x1a\x74"
        "\x4c\x30\xea\x1f\x44\x26\xa9\xf3\x89\x2e\xc3\x9e\xa8\xa5\xec\x24\x8f"
        "\xd4\xda\x46\xf2\xc9\x9a\x8a\xd2\x9b\x10\xc2\x2d\x8a\x86\x1f\xa1\x92"
        "\xe7\x63\x27\x3e\x78\xc2\xee\xd4\xd2\x5d\xaa\xef\x3c\xe7\xf4\xa9\x93"
        "\x44\x76\x4e\x2e\x27\x04\x4c\x57\x39\xbf\x53\x56\xd4\xe9\x2e\x2a\x3a"
        "\xb1\x5a\x39\xd0\x22\x07\xa2\x32\x1f\xfd\xba\xae\xae\x04\x0f\xde\xd0"
        "\xf1\x65\x19\x89\x6a\x73\x29\x0b\x29\x25\xac\xd5\xef\x89\xd0\x46\x50"
        "\x62\x6b\xdb\x3f\x12\xaa\x85\x0e\xaa\xa1\x7b\x1a\x0d\x4e\x52\xb6\xc3"
        "\xb1\x62\x79\xf3\xd5\x57\xc3\x1f\xce\x76\x7f\x90\xc8\x9c\x1d\x8d\x09"
        "\x9b\xf5\xbc\x23\x4a\x71\x51\x69\x7f\x2b\x54\xcd\x6c\xa5\xcd\xe2\xc8"
        "\xd5\xb3\x80\x28\x7f\x86\xd3\x0b\xfe\x63\x83\x5c\xd0\x47\x02\x87\xf4"
        "\x25\x16\x7e\xb8\x56\x01\x91\xe9\xe5\x7d\xa5\x5a\xae\xb1\x80\x76\x53"
        "\x79\xdc\xb8\x9b\x64\x4a\x58\xa9\x95\xcc\x7d\xa9\xa5\x72\x3d\xea\xa4"
        "\x88\xda\x2c\x04\x8b\x25\x84\xa4\x49\x6b\x46\xe2\xac\xa3\x79\x99\xee"
        "\x22\x33\x09\x39\xc8\xf1\x00\x83\xab\x2b\x49\x73\xcc\x10\x4e\x07\x27"
        "\xeb\x7e\xde\x0d\xad\x3f\xcc\xd7\xae\xeb\x66\xf4\x0d\x10\x3b\xe5\xb3"
        "\x8f\x8c\x21\x0a\x2d\x3b\xe3\x56\x2d\xe5\x75\x98\x74\xe9\xa1\x4e\xf2"
        "\xda\x00\x50\xd3\x3b\x0d\x35\x5a\x9a\xa1\xe3\xdc\x9f\xf8\x3e\x2f\x59"
        "\x81\x2a\x0e\xc1\x49\xd8\x19\x14\x3f\x63\xfb\x44\xe5\x2d\xe2\x73\xbf"
        "\x57\xba\x4b\xde\x36\x17\x55\xae\x94\x9c\xb5\xac\xa8\x67\x20\xe6\x45"
        "\xa9\x2c\xcb\x2b\x47\x5a\x90\x9b\x31\x66\xc9\x29\xcb\xf6\x27\xf5\x5a"
        "\x81\xce\xdd\x4a\x19\xb2\x6e\x7f\xdf\xb7\xa0\x5c\x2d\x03\x4f\xd4\x99"
        "\xd7\xfb\xbb\x44\xfc\xdf\xd1\x62\x50\xd7\xfa\xa8\xf4\xee\xe0\x7e\xf6"
        "\x18\x5a\x3e\x6c\x90\x0a\x33\x7b\x68\x6a\xb0\x18\x65\xef\x8b\x3c\x72"
        "\x09\xf1\xff\x68\x98\x9d\x04\xc9\xb6\xdd\x6f\x8a\x33\x2d\x74\xe2\xb4"
        "\xf4\x41\x4f\xc0\x5a\xf0\xbf\x09\x49\xc2\x19\xfb\xdc\x45\xbd\xee\xe7"
        "\x1a\x0f\x24\xd2\xa9\xe6\xbd\x15\xc9\x94\xc4\xc3\x96\x6d\x3e\x7e\x01"
        "\xc9\xb2\x8f\x8d\xb5\xf6\x7e\x49\xc9\xb3\x30\x4b\x7a\xe3\x8a\x33\xed"
        "\x64\xd8\x3c\xc5\x7b\xb3\x70\x08\x2f\x36\x6e\xb3\xcd\xbd\xf2\x33\x88"
        "\xa6\x8b\x67\xa9\x7d\x5c\xd4\xc8\xa1\x82\x7a\x78\x1e\x87\x0b\x26\x59"
        "\xd2\x10\x94\x71\x34\x5c\xdb\x09\x08\x52\x4d\x30\xfd\x71\x00\x00\x0a"
        "\x00\x06\x00\x08\x02\x11",
        3967);
    *(uint64_t*)0x20001248 = 0xfb0;
    *(uint64_t*)0x20001298 = 1;
    *(uint64_t*)0x200012a0 = 0;
    *(uint64_t*)0x200012a8 = 0;
    *(uint32_t*)0x200012b0 = 0;
    syscall(__NR_sendmsg, -1, 0x20001280ul, 0ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
