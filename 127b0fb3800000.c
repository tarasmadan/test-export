// https://syzkaller.appspot.com/bug?id=d8f431b776f2fb2347b2ad84fbb7f6a4388f211b
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <fcntl.h>
#include <linux/capability.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

static void fail(const char* msg, ...)
{
  int e = errno;
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit((e == ENOMEM || e == EAGAIN) ? kRetryStatus : kFailStatus);
}

static void use_temporary_dir()
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    fail("failed to mkdtemp");
  if (chmod(tmpdir, 0777))
    fail("failed to chmod");
  if (chdir(tmpdir))
    fail("failed to chdir");
}

static void vsnprintf_check(char* str, size_t size, const char* format,
                            va_list args)
{
  int rv;

  rv = vsnprintf(str, size, format, args);
  if (rv < 0)
    fail("tun: snprintf failed");
  if ((size_t)rv >= size)
    fail("tun: string '%s...' doesn't fit into buffer", str);
}

static void snprintf_check(char* str, size_t size, const char* format, ...)
{
  va_list args;

  va_start(args, format);
  vsnprintf_check(str, size, format, args);
  va_end(args);
}

#define COMMAND_MAX_LEN 128
#define PATH_PREFIX                                                            \
  "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin "
#define PATH_PREFIX_LEN (sizeof(PATH_PREFIX) - 1)

static void execute_command(bool panic, const char* format, ...)
{
  va_list args;
  char command[PATH_PREFIX_LEN + COMMAND_MAX_LEN];
  int rv;

  va_start(args, format);
  memcpy(command, PATH_PREFIX, PATH_PREFIX_LEN);
  vsnprintf_check(command + PATH_PREFIX_LEN, COMMAND_MAX_LEN, format, args);
  rv = system(command);
  if (panic && rv != 0)
    fail("tun: command \"%s\" failed with code %d", &command[0], rv);

  va_end(args);
}

static int tunfd = -1;
static int tun_frags_enabled;

#define SYZ_TUN_MAX_PACKET_SIZE 1000

#define MAX_PIDS 32
#define ADDR_MAX_LEN 32

#define LOCAL_MAC "aa:aa:aa:aa:%02hx:aa"
#define REMOTE_MAC "aa:aa:aa:aa:%02hx:bb"

#define LOCAL_IPV4 "172.20.%d.170"
#define REMOTE_IPV4 "172.20.%d.187"

#define LOCAL_IPV6 "fe80::%02hx:aa"
#define REMOTE_IPV6 "fe80::%02hx:bb"

#define IFF_NAPI 0x0010
#define IFF_NAPI_FRAGS 0x0020

static void initialize_tun(int id)
{
  if (id >= MAX_PIDS)
    fail("tun: no more than %d executors", MAX_PIDS);

  tunfd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
  if (tunfd == -1) {
    printf("tun: can't open /dev/net/tun: please enable CONFIG_TUN=y\n");
    printf("otherwise fuzzing or reproducing might not work as intended\n");
    return;
  }

  char iface[IFNAMSIZ];
  snprintf_check(iface, sizeof(iface), "syz%d", id);

  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, iface, IFNAMSIZ);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI | IFF_NAPI | IFF_NAPI_FRAGS;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0) {
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0)
      fail("tun: ioctl(TUNSETIFF) failed");
  }
  if (ioctl(tunfd, TUNGETIFF, (void*)&ifr) < 0)
    fail("tun: ioctl(TUNGETIFF) failed");
  tun_frags_enabled = (ifr.ifr_flags & IFF_NAPI_FRAGS) != 0;

  char local_mac[ADDR_MAX_LEN];
  snprintf_check(local_mac, sizeof(local_mac), LOCAL_MAC, id);
  char remote_mac[ADDR_MAX_LEN];
  snprintf_check(remote_mac, sizeof(remote_mac), REMOTE_MAC, id);

  char local_ipv4[ADDR_MAX_LEN];
  snprintf_check(local_ipv4, sizeof(local_ipv4), LOCAL_IPV4, id);
  char remote_ipv4[ADDR_MAX_LEN];
  snprintf_check(remote_ipv4, sizeof(remote_ipv4), REMOTE_IPV4, id);

  char local_ipv6[ADDR_MAX_LEN];
  snprintf_check(local_ipv6, sizeof(local_ipv6), LOCAL_IPV6, id);
  char remote_ipv6[ADDR_MAX_LEN];
  snprintf_check(remote_ipv6, sizeof(remote_ipv6), REMOTE_IPV6, id);

  execute_command(1, "sysctl -w net.ipv6.conf.%s.accept_dad=0", iface);

  execute_command(1, "sysctl -w net.ipv6.conf.%s.router_solicitations=0",
                  iface);

  execute_command(1, "ip link set dev %s address %s", iface, local_mac);
  execute_command(1, "ip addr add %s/24 dev %s", local_ipv4, iface);
  execute_command(1, "ip -6 addr add %s/120 dev %s", local_ipv6, iface);
  execute_command(1, "ip neigh add %s lladdr %s dev %s nud permanent",
                  remote_ipv4, remote_mac, iface);
  execute_command(1, "ip -6 neigh add %s lladdr %s dev %s nud permanent",
                  remote_ipv6, remote_mac, iface);
  execute_command(1, "ip link set dev %s up", iface);
}

#define DEV_IPV4 "172.20.%d.%d"
#define DEV_IPV6 "fe80::%02hx:%02hx"
#define DEV_MAC "aa:aa:aa:aa:%02hx:%02hx"

static void initialize_netdevices(int id)
{
  unsigned i;
  const char* devtypes[] = {"ip6gretap", "bridge", "vcan"};
  const char* devnames[] = {"lo",       "sit0",    "bridge0", "vcan0",
                            "tunl0",    "gre0",    "gretap0", "ip_vti0",
                            "ip6_vti0", "ip6tnl0", "ip6gre0", "ip6gretap0",
                            "erspan0"};

  for (i = 0; i < sizeof(devtypes) / (sizeof(devtypes[0])); i++)
    execute_command(0, "ip link add dev %s0 type %s", devtypes[i], devtypes[i]);
  for (i = 0; i < sizeof(devnames) / (sizeof(devnames[0])); i++) {
    char addr[ADDR_MAX_LEN];
    snprintf_check(addr, sizeof(addr), DEV_IPV4, id, id + 10);
    execute_command(0, "ip -4 addr add %s/24 dev %s", addr, devnames[i]);
    snprintf_check(addr, sizeof(addr), DEV_IPV6, id, id + 10);
    execute_command(0, "ip -6 addr add %s/120 dev %s", addr, devnames[i]);
    snprintf_check(addr, sizeof(addr), DEV_MAC, id, id + 10);
    execute_command(0, "ip link set dev %s address %s", devnames[i], addr);
    execute_command(0, "ip link set dev %s up", devnames[i]);
  }
}

static void setup_tun(uint64_t pid, bool enable_tun)
{
  if (enable_tun) {
    initialize_tun(pid);
    initialize_netdevices(pid);
  }
}

#define MAX_FRAGS 4
struct vnet_fragmentation {
  uint32_t full;
  uint32_t count;
  uint32_t frags[MAX_FRAGS];
};

static uintptr_t syz_emit_ethernet(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (tunfd < 0)
    return (uintptr_t)-1;

  uint32_t length = a0;
  char* data = (char*)a1;

  struct vnet_fragmentation* frags = (struct vnet_fragmentation*)a2;
  struct iovec vecs[MAX_FRAGS + 1];
  uint32_t nfrags = 0;
  if (!tun_frags_enabled || frags == NULL) {
    vecs[nfrags].iov_base = data;
    vecs[nfrags].iov_len = length;
    nfrags++;
  } else {
    bool full = true;
    uint32_t i, count = 0;
    full = frags->full;
    count = frags->count;
    if (count > MAX_FRAGS)
      count = MAX_FRAGS;
    for (i = 0; i < count && length != 0; i++) {
      uint32_t size = 0;
      size = frags->frags[i];
      if (size > length)
        size = length;
      vecs[nfrags].iov_base = data;
      vecs[nfrags].iov_len = size;
      nfrags++;
      data += size;
      length -= size;
    }
    if (length != 0 && (full || nfrags == 0)) {
      vecs[nfrags].iov_base = data;
      vecs[nfrags].iov_len = length;
      nfrags++;
    }
  }
  return writev(tunfd, vecs, nfrags);
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();

  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 128 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);

#define CLONE_NEWCGROUP 0x02000000

  if (unshare(CLONE_NEWNS)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(CLONE_NEWCGROUP)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
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
    close(fd);
    return false;
  }
  close(fd);
  return true;
}

static int real_uid;
static int real_gid;
__attribute__((aligned(64 << 10))) static char sandbox_stack[1 << 20];

static int namespace_sandbox_proc(void* arg)
{
  sandbox_common();

  write_file("/proc/self/setgroups", "deny");
  if (!write_file("/proc/self/uid_map", "0 %d 1\n", real_uid))
    fail("write of /proc/self/uid_map failed");
  if (!write_file("/proc/self/gid_map", "0 %d 1\n", real_gid))
    fail("write of /proc/self/gid_map failed");

  if (unshare(CLONE_NEWNET))
    fail("unshare(CLONE_NEWNET)");
  setup_tun((long)arg >> 1, (long)arg & 1);

  if (mkdir("./syz-tmp", 0777))
    fail("mkdir(syz-tmp) failed");
  if (mount("", "./syz-tmp", "tmpfs", 0, NULL))
    fail("mount(tmpfs) failed");
  if (mkdir("./syz-tmp/newroot", 0777))
    fail("mkdir failed");
  if (mkdir("./syz-tmp/newroot/dev", 0700))
    fail("mkdir failed");
  unsigned mount_flags = MS_BIND | MS_REC | MS_PRIVATE;
  if (mount("/dev", "./syz-tmp/newroot/dev", NULL, mount_flags, NULL))
    fail("mount(dev) failed");
  if (mkdir("./syz-tmp/newroot/proc", 0700))
    fail("mkdir failed");
  if (mount(NULL, "./syz-tmp/newroot/proc", "proc", 0, NULL))
    fail("mount(proc) failed");
  if (mkdir("./syz-tmp/newroot/selinux", 0700))
    fail("mkdir failed");
  const char* selinux_path = "./syz-tmp/newroot/selinux";
  if (mount("/selinux", selinux_path, NULL, mount_flags, NULL) &&
      mount("/sys/fs/selinux", selinux_path, NULL, mount_flags, NULL))
    fail("mount(selinuxfs) failed");
  if (mkdir("./syz-tmp/pivot", 0777))
    fail("mkdir failed");
  if (syscall(SYS_pivot_root, "./syz-tmp", "./syz-tmp/pivot")) {
    if (chdir("./syz-tmp"))
      fail("chdir failed");
  } else {
    if (chdir("/"))
      fail("chdir failed");
    if (umount2("./pivot", MNT_DETACH))
      fail("umount failed");
  }
  if (chroot("./newroot"))
    fail("chroot failed");
  if (chdir("/"))
    fail("chdir failed");

  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    fail("capget failed");
  cap_data[0].effective &= ~(1 << CAP_SYS_PTRACE);
  cap_data[0].permitted &= ~(1 << CAP_SYS_PTRACE);
  cap_data[0].inheritable &= ~(1 << CAP_SYS_PTRACE);
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    fail("capset failed");

  loop();
  doexit(1);
}

static int do_sandbox_namespace(int executor_pid, bool enable_tun)
{
  int pid;

  real_uid = getuid();
  real_gid = getgid();
  mprotect(sandbox_stack, 4096, PROT_NONE);
  void* arg = (void*)(long)((executor_pid << 1) | enable_tun);
  pid =
      clone(namespace_sandbox_proc, &sandbox_stack[sizeof(sandbox_stack) - 64],
            CLONE_NEWUSER | CLONE_NEWPID, arg);
  if (pid < 0)
    fail("sandbox clone failed");
  return pid;
}

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_socket, 2, 3, 0xb);
  memcpy((void*)0x20000400, "\x66\x69\x6c\x74\x65\x72\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20000420 = 7;
  *(uint32_t*)0x20000424 = 4;
  *(uint32_t*)0x20000428 = 0x3a0;
  *(uint32_t*)0x2000042c = 0;
  *(uint32_t*)0x20000430 = 0xe8;
  *(uint32_t*)0x20000434 = 0x1d0;
  *(uint32_t*)0x20000438 = 0x2b8;
  *(uint32_t*)0x2000043c = 0x2b8;
  *(uint32_t*)0x20000440 = 0x2b8;
  *(uint32_t*)0x20000444 = 4;
  *(uint64_t*)0x20000448 = 0x20000080;
  *(uint8_t*)0x20000450 = 0;
  *(uint8_t*)0x20000451 = 0;
  *(uint8_t*)0x20000452 = 0;
  *(uint8_t*)0x20000453 = 0;
  *(uint8_t*)0x20000454 = 0;
  *(uint8_t*)0x20000455 = 0;
  *(uint8_t*)0x20000456 = 0;
  *(uint8_t*)0x20000457 = 0;
  *(uint8_t*)0x20000458 = 0;
  *(uint8_t*)0x20000459 = 0;
  *(uint8_t*)0x2000045a = 0;
  *(uint8_t*)0x2000045b = 0;
  *(uint8_t*)0x2000045c = 0;
  *(uint8_t*)0x2000045d = 0;
  *(uint8_t*)0x2000045e = 0;
  *(uint8_t*)0x2000045f = 0;
  *(uint8_t*)0x20000460 = 0;
  *(uint8_t*)0x20000461 = 0;
  *(uint8_t*)0x20000462 = 0;
  *(uint8_t*)0x20000463 = 0;
  *(uint8_t*)0x20000464 = 0;
  *(uint8_t*)0x20000465 = 0;
  *(uint8_t*)0x20000466 = 0;
  *(uint8_t*)0x20000467 = 0;
  *(uint8_t*)0x20000468 = 0;
  *(uint8_t*)0x20000469 = 0;
  *(uint8_t*)0x2000046a = 0;
  *(uint8_t*)0x2000046b = 0;
  *(uint8_t*)0x2000046c = 0;
  *(uint8_t*)0x2000046d = 0;
  *(uint8_t*)0x2000046e = 0;
  *(uint8_t*)0x2000046f = 0;
  *(uint8_t*)0x20000470 = 0;
  *(uint8_t*)0x20000471 = 0;
  *(uint8_t*)0x20000472 = 0;
  *(uint8_t*)0x20000473 = 0;
  *(uint8_t*)0x20000474 = 0;
  *(uint8_t*)0x20000475 = 0;
  *(uint8_t*)0x20000476 = 0;
  *(uint8_t*)0x20000477 = 0;
  *(uint8_t*)0x20000478 = 0;
  *(uint8_t*)0x20000479 = 0;
  *(uint8_t*)0x2000047a = 0;
  *(uint8_t*)0x2000047b = 0;
  *(uint8_t*)0x2000047c = 0;
  *(uint8_t*)0x2000047d = 0;
  *(uint8_t*)0x2000047e = 0;
  *(uint8_t*)0x2000047f = 0;
  *(uint8_t*)0x20000480 = 0;
  *(uint8_t*)0x20000481 = 0;
  *(uint8_t*)0x20000482 = 0;
  *(uint8_t*)0x20000483 = 0;
  *(uint8_t*)0x20000484 = 0;
  *(uint8_t*)0x20000485 = 0;
  *(uint8_t*)0x20000486 = 0;
  *(uint8_t*)0x20000487 = 0;
  *(uint8_t*)0x20000488 = 0;
  *(uint8_t*)0x20000489 = 0;
  *(uint8_t*)0x2000048a = 0;
  *(uint8_t*)0x2000048b = 0;
  *(uint8_t*)0x2000048c = 0;
  *(uint8_t*)0x2000048d = 0;
  *(uint8_t*)0x2000048e = 0;
  *(uint8_t*)0x2000048f = 0;
  *(uint8_t*)0x20000490 = 0;
  *(uint8_t*)0x20000491 = 0;
  *(uint8_t*)0x20000492 = 0;
  *(uint8_t*)0x20000493 = 0;
  *(uint8_t*)0x20000494 = 0;
  *(uint8_t*)0x20000495 = 0;
  *(uint8_t*)0x20000496 = 0;
  *(uint8_t*)0x20000497 = 0;
  *(uint8_t*)0x20000498 = 0;
  *(uint8_t*)0x20000499 = 0;
  *(uint8_t*)0x2000049a = 0;
  *(uint8_t*)0x2000049b = 0;
  *(uint8_t*)0x2000049c = 0;
  *(uint8_t*)0x2000049d = 0;
  *(uint8_t*)0x2000049e = 0;
  *(uint8_t*)0x2000049f = 0;
  *(uint8_t*)0x200004a0 = 0;
  *(uint8_t*)0x200004a1 = 0;
  *(uint8_t*)0x200004a2 = 0;
  *(uint8_t*)0x200004a3 = 0;
  *(uint8_t*)0x200004a4 = 0;
  *(uint8_t*)0x200004a5 = 0;
  *(uint8_t*)0x200004a6 = 0;
  *(uint8_t*)0x200004a7 = 0;
  *(uint8_t*)0x200004a8 = 0;
  *(uint8_t*)0x200004a9 = 0;
  *(uint8_t*)0x200004aa = 0;
  *(uint8_t*)0x200004ab = 0;
  *(uint8_t*)0x200004ac = 0;
  *(uint8_t*)0x200004ad = 0;
  *(uint8_t*)0x200004ae = 0;
  *(uint8_t*)0x200004af = 0;
  *(uint8_t*)0x200004b0 = 0;
  *(uint8_t*)0x200004b1 = 0;
  *(uint8_t*)0x200004b2 = 0;
  *(uint8_t*)0x200004b3 = 0;
  *(uint8_t*)0x200004b4 = 0;
  *(uint8_t*)0x200004b5 = 0;
  *(uint8_t*)0x200004b6 = 0;
  *(uint8_t*)0x200004b7 = 0;
  *(uint8_t*)0x200004b8 = 0;
  *(uint8_t*)0x200004b9 = 0;
  *(uint8_t*)0x200004ba = 0;
  *(uint8_t*)0x200004bb = 0;
  *(uint8_t*)0x200004bc = 0;
  *(uint8_t*)0x200004bd = 0;
  *(uint8_t*)0x200004be = 0;
  *(uint8_t*)0x200004bf = 0;
  *(uint8_t*)0x200004c0 = 0;
  *(uint8_t*)0x200004c1 = 0;
  *(uint8_t*)0x200004c2 = 0;
  *(uint8_t*)0x200004c3 = 0;
  *(uint8_t*)0x200004c4 = 0;
  *(uint8_t*)0x200004c5 = 0;
  *(uint8_t*)0x200004c6 = 0;
  *(uint8_t*)0x200004c7 = 0;
  *(uint8_t*)0x200004c8 = 0;
  *(uint8_t*)0x200004c9 = 0;
  *(uint8_t*)0x200004ca = 0;
  *(uint8_t*)0x200004cb = 0;
  *(uint8_t*)0x200004cc = 0;
  *(uint8_t*)0x200004cd = 0;
  *(uint8_t*)0x200004ce = 0;
  *(uint8_t*)0x200004cf = 0;
  *(uint8_t*)0x200004d0 = 0;
  *(uint8_t*)0x200004d1 = 0;
  *(uint8_t*)0x200004d2 = 0;
  *(uint8_t*)0x200004d3 = 0;
  *(uint8_t*)0x200004d4 = 0;
  *(uint8_t*)0x200004d5 = 0;
  *(uint8_t*)0x200004d6 = 0;
  *(uint8_t*)0x200004d7 = 0;
  *(uint8_t*)0x200004d8 = 0;
  *(uint8_t*)0x200004d9 = 0;
  *(uint8_t*)0x200004da = 0;
  *(uint8_t*)0x200004db = 0;
  *(uint8_t*)0x200004dc = 0;
  *(uint8_t*)0x200004dd = 0;
  *(uint8_t*)0x200004de = 0;
  *(uint8_t*)0x200004df = 0;
  *(uint8_t*)0x200004e0 = 0;
  *(uint8_t*)0x200004e1 = 0;
  *(uint8_t*)0x200004e2 = 0;
  *(uint8_t*)0x200004e3 = 0;
  *(uint8_t*)0x200004e4 = 0;
  *(uint8_t*)0x200004e5 = 0;
  *(uint8_t*)0x200004e6 = 0;
  *(uint8_t*)0x200004e7 = 0;
  *(uint8_t*)0x200004e8 = 0;
  *(uint8_t*)0x200004e9 = 0;
  *(uint8_t*)0x200004ea = 0;
  *(uint8_t*)0x200004eb = 0;
  *(uint8_t*)0x200004ec = 0;
  *(uint8_t*)0x200004ed = 0;
  *(uint8_t*)0x200004ee = 0;
  *(uint8_t*)0x200004ef = 0;
  *(uint8_t*)0x200004f0 = 0;
  *(uint8_t*)0x200004f1 = 0;
  *(uint8_t*)0x200004f2 = 0;
  *(uint8_t*)0x200004f3 = 0;
  *(uint16_t*)0x200004f4 = 0xc0;
  *(uint16_t*)0x200004f6 = 0xe8;
  *(uint32_t*)0x200004f8 = 0;
  *(uint64_t*)0x20000500 = 0;
  *(uint64_t*)0x20000508 = 0;
  *(uint16_t*)0x20000510 = 0x28;
  memcpy((void*)0x20000512, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x2000052f = 0;
  *(uint32_t*)0x20000530 = 0x1d0;
  *(uint8_t*)0x20000538 = 0;
  *(uint8_t*)0x20000539 = 0;
  *(uint8_t*)0x2000053a = 0;
  *(uint8_t*)0x2000053b = 0;
  *(uint8_t*)0x2000053c = 0;
  *(uint8_t*)0x2000053d = 0;
  *(uint8_t*)0x2000053e = 0;
  *(uint8_t*)0x2000053f = 0;
  *(uint8_t*)0x20000540 = 0;
  *(uint8_t*)0x20000541 = 0;
  *(uint8_t*)0x20000542 = 0;
  *(uint8_t*)0x20000543 = 0;
  *(uint8_t*)0x20000544 = 0;
  *(uint8_t*)0x20000545 = 0;
  *(uint8_t*)0x20000546 = 0;
  *(uint8_t*)0x20000547 = 0;
  *(uint8_t*)0x20000548 = 0;
  *(uint8_t*)0x20000549 = 0;
  *(uint8_t*)0x2000054a = 0;
  *(uint8_t*)0x2000054b = 0;
  *(uint8_t*)0x2000054c = 0;
  *(uint8_t*)0x2000054d = 0;
  *(uint8_t*)0x2000054e = 0;
  *(uint8_t*)0x2000054f = 0;
  *(uint8_t*)0x20000550 = 0;
  *(uint8_t*)0x20000551 = 0;
  *(uint8_t*)0x20000552 = 0;
  *(uint8_t*)0x20000553 = 0;
  *(uint8_t*)0x20000554 = 0;
  *(uint8_t*)0x20000555 = 0;
  *(uint8_t*)0x20000556 = 0;
  *(uint8_t*)0x20000557 = 0;
  *(uint8_t*)0x20000558 = 0;
  *(uint8_t*)0x20000559 = 0;
  *(uint8_t*)0x2000055a = 0;
  *(uint8_t*)0x2000055b = 0;
  *(uint8_t*)0x2000055c = 0;
  *(uint8_t*)0x2000055d = 0;
  *(uint8_t*)0x2000055e = 0;
  *(uint8_t*)0x2000055f = 0;
  *(uint8_t*)0x20000560 = 0;
  *(uint8_t*)0x20000561 = 0;
  *(uint8_t*)0x20000562 = 0;
  *(uint8_t*)0x20000563 = 0;
  *(uint8_t*)0x20000564 = 0;
  *(uint8_t*)0x20000565 = 0;
  *(uint8_t*)0x20000566 = 0;
  *(uint8_t*)0x20000567 = 0;
  *(uint8_t*)0x20000568 = 0;
  *(uint8_t*)0x20000569 = 0;
  *(uint8_t*)0x2000056a = 0;
  *(uint8_t*)0x2000056b = 0;
  *(uint8_t*)0x2000056c = 0;
  *(uint8_t*)0x2000056d = 0;
  *(uint8_t*)0x2000056e = 0;
  *(uint8_t*)0x2000056f = 0;
  *(uint8_t*)0x20000570 = 0;
  *(uint8_t*)0x20000571 = 0;
  *(uint8_t*)0x20000572 = 0;
  *(uint8_t*)0x20000573 = 0;
  *(uint8_t*)0x20000574 = 0;
  *(uint8_t*)0x20000575 = 0;
  *(uint8_t*)0x20000576 = 0;
  *(uint8_t*)0x20000577 = 0;
  *(uint8_t*)0x20000578 = 0;
  *(uint8_t*)0x20000579 = 0;
  *(uint8_t*)0x2000057a = 0;
  *(uint8_t*)0x2000057b = 0;
  *(uint8_t*)0x2000057c = 0;
  *(uint8_t*)0x2000057d = 0;
  *(uint8_t*)0x2000057e = 0;
  *(uint8_t*)0x2000057f = 0;
  *(uint8_t*)0x20000580 = 0;
  *(uint8_t*)0x20000581 = 0;
  *(uint8_t*)0x20000582 = 0;
  *(uint8_t*)0x20000583 = 0;
  *(uint8_t*)0x20000584 = 0;
  *(uint8_t*)0x20000585 = 0;
  *(uint8_t*)0x20000586 = 0;
  *(uint8_t*)0x20000587 = 0;
  *(uint8_t*)0x20000588 = 0;
  *(uint8_t*)0x20000589 = 0;
  *(uint8_t*)0x2000058a = 0;
  *(uint8_t*)0x2000058b = 0;
  *(uint8_t*)0x2000058c = 0;
  *(uint8_t*)0x2000058d = 0;
  *(uint8_t*)0x2000058e = 0;
  *(uint8_t*)0x2000058f = 0;
  *(uint8_t*)0x20000590 = 0;
  *(uint8_t*)0x20000591 = 0;
  *(uint8_t*)0x20000592 = 0;
  *(uint8_t*)0x20000593 = 0;
  *(uint8_t*)0x20000594 = 0;
  *(uint8_t*)0x20000595 = 0;
  *(uint8_t*)0x20000596 = 0;
  *(uint8_t*)0x20000597 = 0;
  *(uint8_t*)0x20000598 = 0;
  *(uint8_t*)0x20000599 = 0;
  *(uint8_t*)0x2000059a = 0;
  *(uint8_t*)0x2000059b = 0;
  *(uint8_t*)0x2000059c = 0;
  *(uint8_t*)0x2000059d = 0;
  *(uint8_t*)0x2000059e = 0;
  *(uint8_t*)0x2000059f = 0;
  *(uint8_t*)0x200005a0 = 0;
  *(uint8_t*)0x200005a1 = 0;
  *(uint8_t*)0x200005a2 = 0;
  *(uint8_t*)0x200005a3 = 0;
  *(uint8_t*)0x200005a4 = 0;
  *(uint8_t*)0x200005a5 = 0;
  *(uint8_t*)0x200005a6 = 0;
  *(uint8_t*)0x200005a7 = 0;
  *(uint8_t*)0x200005a8 = 0;
  *(uint8_t*)0x200005a9 = 0;
  *(uint8_t*)0x200005aa = 0;
  *(uint8_t*)0x200005ab = 0;
  *(uint8_t*)0x200005ac = 0;
  *(uint8_t*)0x200005ad = 0;
  *(uint8_t*)0x200005ae = 0;
  *(uint8_t*)0x200005af = 0;
  *(uint8_t*)0x200005b0 = 0;
  *(uint8_t*)0x200005b1 = 0;
  *(uint8_t*)0x200005b2 = 0;
  *(uint8_t*)0x200005b3 = 0;
  *(uint8_t*)0x200005b4 = 0;
  *(uint8_t*)0x200005b5 = 0;
  *(uint8_t*)0x200005b6 = 0;
  *(uint8_t*)0x200005b7 = 0;
  *(uint8_t*)0x200005b8 = 0;
  *(uint8_t*)0x200005b9 = 0;
  *(uint8_t*)0x200005ba = 0;
  *(uint8_t*)0x200005bb = 0;
  *(uint8_t*)0x200005bc = 0;
  *(uint8_t*)0x200005bd = 0;
  *(uint8_t*)0x200005be = 0;
  *(uint8_t*)0x200005bf = 0;
  *(uint8_t*)0x200005c0 = 0;
  *(uint8_t*)0x200005c1 = 0;
  *(uint8_t*)0x200005c2 = 0;
  *(uint8_t*)0x200005c3 = 0;
  *(uint8_t*)0x200005c4 = 0;
  *(uint8_t*)0x200005c5 = 0;
  *(uint8_t*)0x200005c6 = 0;
  *(uint8_t*)0x200005c7 = 0;
  *(uint8_t*)0x200005c8 = 0;
  *(uint8_t*)0x200005c9 = 0;
  *(uint8_t*)0x200005ca = 0;
  *(uint8_t*)0x200005cb = 0;
  *(uint8_t*)0x200005cc = 0;
  *(uint8_t*)0x200005cd = 0;
  *(uint8_t*)0x200005ce = 0;
  *(uint8_t*)0x200005cf = 0;
  *(uint8_t*)0x200005d0 = 0;
  *(uint8_t*)0x200005d1 = 0;
  *(uint8_t*)0x200005d2 = 0;
  *(uint8_t*)0x200005d3 = 0;
  *(uint8_t*)0x200005d4 = 0;
  *(uint8_t*)0x200005d5 = 0;
  *(uint8_t*)0x200005d6 = 0;
  *(uint8_t*)0x200005d7 = 0;
  *(uint8_t*)0x200005d8 = 0;
  *(uint8_t*)0x200005d9 = 0;
  *(uint8_t*)0x200005da = 0;
  *(uint8_t*)0x200005db = 0;
  *(uint16_t*)0x200005dc = 0xc0;
  *(uint16_t*)0x200005de = 0xe8;
  *(uint32_t*)0x200005e0 = 0;
  *(uint64_t*)0x200005e8 = 0;
  *(uint64_t*)0x200005f0 = 0;
  *(uint16_t*)0x200005f8 = 0x28;
  memcpy((void*)0x200005fa, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20000617 = 0;
  *(uint32_t*)0x20000618 = 0xfffffffe;
  *(uint32_t*)0x20000620 = htobe32(0x7f000001);
  *(uint32_t*)0x20000624 = htobe32(0xe0000002);
  *(uint32_t*)0x20000628 = htobe32(-1);
  *(uint32_t*)0x2000062c = htobe32(-1);
  *(uint8_t*)0x20000630 = 0;
  *(uint8_t*)0x20000631 = 0;
  *(uint8_t*)0x20000632 = 0;
  *(uint8_t*)0x20000633 = 0;
  *(uint8_t*)0x20000634 = 0;
  *(uint8_t*)0x20000635 = 0;
  *(uint8_t*)0x20000636 = 0;
  *(uint8_t*)0x20000637 = 0;
  *(uint8_t*)0x20000638 = 0;
  *(uint8_t*)0x20000639 = 0;
  *(uint8_t*)0x2000063a = 0;
  *(uint8_t*)0x2000063b = 0;
  *(uint8_t*)0x2000063c = 0;
  *(uint8_t*)0x2000063d = 0;
  *(uint8_t*)0x2000063e = 0;
  *(uint8_t*)0x2000063f = 0;
  *(uint8_t*)0x20000640 = 0;
  *(uint8_t*)0x20000641 = 0;
  *(uint8_t*)0x20000642 = -1;
  *(uint8_t*)0x20000643 = -1;
  *(uint8_t*)0x20000644 = 0;
  *(uint8_t*)0x20000645 = -1;
  *(uint8_t*)0x20000646 = 0;
  *(uint8_t*)0x20000647 = 0;
  *(uint8_t*)0x20000648 = 0;
  *(uint8_t*)0x20000649 = 0;
  *(uint8_t*)0x2000064a = 0;
  *(uint8_t*)0x2000064b = 0;
  *(uint8_t*)0x2000064c = 0;
  *(uint8_t*)0x2000064d = 0;
  *(uint8_t*)0x2000064e = 0;
  *(uint8_t*)0x2000064f = 0;
  *(uint8_t*)0x20000650 = 0;
  *(uint8_t*)0x20000651 = 0;
  *(uint8_t*)0x20000652 = 0;
  *(uint8_t*)0x20000653 = 0;
  *(uint8_t*)0x20000654 = 0;
  *(uint8_t*)0x20000655 = 0;
  *(uint8_t*)0x20000656 = 0;
  *(uint8_t*)0x20000657 = 0;
  *(uint8_t*)0x20000658 = 0;
  *(uint8_t*)0x20000659 = 0;
  *(uint8_t*)0x2000065a = 0;
  *(uint8_t*)0x2000065b = 0;
  *(uint8_t*)0x2000065c = 0;
  *(uint8_t*)0x2000065d = 0;
  *(uint8_t*)0x2000065e = 0;
  *(uint8_t*)0x2000065f = 0;
  *(uint8_t*)0x20000660 = -1;
  *(uint8_t*)0x20000661 = -1;
  *(uint8_t*)0x20000662 = -1;
  *(uint8_t*)0x20000663 = -1;
  *(uint8_t*)0x20000664 = 0;
  *(uint8_t*)0x20000665 = -1;
  *(uint8_t*)0x20000666 = 0;
  *(uint8_t*)0x20000667 = 0;
  *(uint8_t*)0x20000668 = 0;
  *(uint8_t*)0x20000669 = 0;
  *(uint8_t*)0x2000066a = 0;
  *(uint8_t*)0x2000066b = 0;
  *(uint8_t*)0x2000066c = 0;
  *(uint8_t*)0x2000066d = 0;
  *(uint8_t*)0x2000066e = 0;
  *(uint8_t*)0x2000066f = 0;
  *(uint16_t*)0x20000670 = htobe16(2);
  *(uint16_t*)0x20000672 = htobe16(7);
  *(uint16_t*)0x20000674 = htobe16(3);
  *(uint16_t*)0x20000676 = htobe16(0);
  *(uint16_t*)0x20000678 = htobe16(1);
  *(uint16_t*)0x2000067a = htobe16(7);
  *(uint8_t*)0x2000067c = 0x73;
  *(uint8_t*)0x2000067d = 0x79;
  *(uint8_t*)0x2000067e = 0x7a;
  *(uint8_t*)0x2000067f = 0x30;
  *(uint8_t*)0x20000680 = 0;
  memcpy((void*)0x2000068c,
         "\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x31\x00\x00\x00\x00\x00\x00",
         16);
  *(uint8_t*)0x2000069c = -1;
  *(uint8_t*)0x2000069d = 0;
  *(uint8_t*)0x2000069e = 0;
  *(uint8_t*)0x2000069f = 0;
  *(uint8_t*)0x200006a0 = 0;
  *(uint8_t*)0x200006a1 = 0;
  *(uint8_t*)0x200006a2 = 0;
  *(uint8_t*)0x200006a3 = 0;
  *(uint8_t*)0x200006a4 = 0;
  *(uint8_t*)0x200006a5 = 0;
  *(uint8_t*)0x200006a6 = 0;
  *(uint8_t*)0x200006a7 = 0;
  *(uint8_t*)0x200006a8 = 0;
  *(uint8_t*)0x200006a9 = 0;
  *(uint8_t*)0x200006aa = 0;
  *(uint8_t*)0x200006ab = 0;
  *(uint8_t*)0x200006ac = 0;
  *(uint8_t*)0x200006ad = 0;
  *(uint8_t*)0x200006ae = 0;
  *(uint8_t*)0x200006af = 0;
  *(uint8_t*)0x200006b0 = 0;
  *(uint8_t*)0x200006b1 = 0;
  *(uint8_t*)0x200006b2 = 0;
  *(uint8_t*)0x200006b3 = 0;
  *(uint8_t*)0x200006b4 = 0;
  *(uint8_t*)0x200006b5 = 0;
  *(uint8_t*)0x200006b6 = 0;
  *(uint8_t*)0x200006b7 = 0;
  *(uint8_t*)0x200006b8 = 0;
  *(uint8_t*)0x200006b9 = 0;
  *(uint8_t*)0x200006ba = 0;
  *(uint8_t*)0x200006bb = 0;
  *(uint8_t*)0x200006bc = 0;
  *(uint16_t*)0x200006be = 0x200;
  *(uint16_t*)0x200006c4 = 0xc0;
  *(uint16_t*)0x200006c6 = 0xe8;
  *(uint32_t*)0x200006c8 = 0;
  *(uint64_t*)0x200006d0 = 0;
  *(uint64_t*)0x200006d8 = 0;
  *(uint16_t*)0x200006e0 = 0x28;
  memcpy((void*)0x200006e2, "\x41\x55\x44\x49\x54\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x200006ff = 0;
  *(uint8_t*)0x20000700 = 0;
  *(uint8_t*)0x20000708 = 0;
  *(uint8_t*)0x20000709 = 0;
  *(uint8_t*)0x2000070a = 0;
  *(uint8_t*)0x2000070b = 0;
  *(uint8_t*)0x2000070c = 0;
  *(uint8_t*)0x2000070d = 0;
  *(uint8_t*)0x2000070e = 0;
  *(uint8_t*)0x2000070f = 0;
  *(uint8_t*)0x20000710 = 0;
  *(uint8_t*)0x20000711 = 0;
  *(uint8_t*)0x20000712 = 0;
  *(uint8_t*)0x20000713 = 0;
  *(uint8_t*)0x20000714 = 0;
  *(uint8_t*)0x20000715 = 0;
  *(uint8_t*)0x20000716 = 0;
  *(uint8_t*)0x20000717 = 0;
  *(uint8_t*)0x20000718 = 0;
  *(uint8_t*)0x20000719 = 0;
  *(uint8_t*)0x2000071a = 0;
  *(uint8_t*)0x2000071b = 0;
  *(uint8_t*)0x2000071c = 0;
  *(uint8_t*)0x2000071d = 0;
  *(uint8_t*)0x2000071e = 0;
  *(uint8_t*)0x2000071f = 0;
  *(uint8_t*)0x20000720 = 0;
  *(uint8_t*)0x20000721 = 0;
  *(uint8_t*)0x20000722 = 0;
  *(uint8_t*)0x20000723 = 0;
  *(uint8_t*)0x20000724 = 0;
  *(uint8_t*)0x20000725 = 0;
  *(uint8_t*)0x20000726 = 0;
  *(uint8_t*)0x20000727 = 0;
  *(uint8_t*)0x20000728 = 0;
  *(uint8_t*)0x20000729 = 0;
  *(uint8_t*)0x2000072a = 0;
  *(uint8_t*)0x2000072b = 0;
  *(uint8_t*)0x2000072c = 0;
  *(uint8_t*)0x2000072d = 0;
  *(uint8_t*)0x2000072e = 0;
  *(uint8_t*)0x2000072f = 0;
  *(uint8_t*)0x20000730 = 0;
  *(uint8_t*)0x20000731 = 0;
  *(uint8_t*)0x20000732 = 0;
  *(uint8_t*)0x20000733 = 0;
  *(uint8_t*)0x20000734 = 0;
  *(uint8_t*)0x20000735 = 0;
  *(uint8_t*)0x20000736 = 0;
  *(uint8_t*)0x20000737 = 0;
  *(uint8_t*)0x20000738 = 0;
  *(uint8_t*)0x20000739 = 0;
  *(uint8_t*)0x2000073a = 0;
  *(uint8_t*)0x2000073b = 0;
  *(uint8_t*)0x2000073c = 0;
  *(uint8_t*)0x2000073d = 0;
  *(uint8_t*)0x2000073e = 0;
  *(uint8_t*)0x2000073f = 0;
  *(uint8_t*)0x20000740 = 0;
  *(uint8_t*)0x20000741 = 0;
  *(uint8_t*)0x20000742 = 0;
  *(uint8_t*)0x20000743 = 0;
  *(uint8_t*)0x20000744 = 0;
  *(uint8_t*)0x20000745 = 0;
  *(uint8_t*)0x20000746 = 0;
  *(uint8_t*)0x20000747 = 0;
  *(uint8_t*)0x20000748 = 0;
  *(uint8_t*)0x20000749 = 0;
  *(uint8_t*)0x2000074a = 0;
  *(uint8_t*)0x2000074b = 0;
  *(uint8_t*)0x2000074c = 0;
  *(uint8_t*)0x2000074d = 0;
  *(uint8_t*)0x2000074e = 0;
  *(uint8_t*)0x2000074f = 0;
  *(uint8_t*)0x20000750 = 0;
  *(uint8_t*)0x20000751 = 0;
  *(uint8_t*)0x20000752 = 0;
  *(uint8_t*)0x20000753 = 0;
  *(uint8_t*)0x20000754 = 0;
  *(uint8_t*)0x20000755 = 0;
  *(uint8_t*)0x20000756 = 0;
  *(uint8_t*)0x20000757 = 0;
  *(uint8_t*)0x20000758 = 0;
  *(uint8_t*)0x20000759 = 0;
  *(uint8_t*)0x2000075a = 0;
  *(uint8_t*)0x2000075b = 0;
  *(uint8_t*)0x2000075c = 0;
  *(uint8_t*)0x2000075d = 0;
  *(uint8_t*)0x2000075e = 0;
  *(uint8_t*)0x2000075f = 0;
  *(uint8_t*)0x20000760 = 0;
  *(uint8_t*)0x20000761 = 0;
  *(uint8_t*)0x20000762 = 0;
  *(uint8_t*)0x20000763 = 0;
  *(uint8_t*)0x20000764 = 0;
  *(uint8_t*)0x20000765 = 0;
  *(uint8_t*)0x20000766 = 0;
  *(uint8_t*)0x20000767 = 0;
  *(uint8_t*)0x20000768 = 0;
  *(uint8_t*)0x20000769 = 0;
  *(uint8_t*)0x2000076a = 0;
  *(uint8_t*)0x2000076b = 0;
  *(uint8_t*)0x2000076c = 0;
  *(uint8_t*)0x2000076d = 0;
  *(uint8_t*)0x2000076e = 0;
  *(uint8_t*)0x2000076f = 0;
  *(uint8_t*)0x20000770 = 0;
  *(uint8_t*)0x20000771 = 0;
  *(uint8_t*)0x20000772 = 0;
  *(uint8_t*)0x20000773 = 0;
  *(uint8_t*)0x20000774 = 0;
  *(uint8_t*)0x20000775 = 0;
  *(uint8_t*)0x20000776 = 0;
  *(uint8_t*)0x20000777 = 0;
  *(uint8_t*)0x20000778 = 0;
  *(uint8_t*)0x20000779 = 0;
  *(uint8_t*)0x2000077a = 0;
  *(uint8_t*)0x2000077b = 0;
  *(uint8_t*)0x2000077c = 0;
  *(uint8_t*)0x2000077d = 0;
  *(uint8_t*)0x2000077e = 0;
  *(uint8_t*)0x2000077f = 0;
  *(uint8_t*)0x20000780 = 0;
  *(uint8_t*)0x20000781 = 0;
  *(uint8_t*)0x20000782 = 0;
  *(uint8_t*)0x20000783 = 0;
  *(uint8_t*)0x20000784 = 0;
  *(uint8_t*)0x20000785 = 0;
  *(uint8_t*)0x20000786 = 0;
  *(uint8_t*)0x20000787 = 0;
  *(uint8_t*)0x20000788 = 0;
  *(uint8_t*)0x20000789 = 0;
  *(uint8_t*)0x2000078a = 0;
  *(uint8_t*)0x2000078b = 0;
  *(uint8_t*)0x2000078c = 0;
  *(uint8_t*)0x2000078d = 0;
  *(uint8_t*)0x2000078e = 0;
  *(uint8_t*)0x2000078f = 0;
  *(uint8_t*)0x20000790 = 0;
  *(uint8_t*)0x20000791 = 0;
  *(uint8_t*)0x20000792 = 0;
  *(uint8_t*)0x20000793 = 0;
  *(uint8_t*)0x20000794 = 0;
  *(uint8_t*)0x20000795 = 0;
  *(uint8_t*)0x20000796 = 0;
  *(uint8_t*)0x20000797 = 0;
  *(uint8_t*)0x20000798 = 0;
  *(uint8_t*)0x20000799 = 0;
  *(uint8_t*)0x2000079a = 0;
  *(uint8_t*)0x2000079b = 0;
  *(uint8_t*)0x2000079c = 0;
  *(uint8_t*)0x2000079d = 0;
  *(uint8_t*)0x2000079e = 0;
  *(uint8_t*)0x2000079f = 0;
  *(uint8_t*)0x200007a0 = 0;
  *(uint8_t*)0x200007a1 = 0;
  *(uint8_t*)0x200007a2 = 0;
  *(uint8_t*)0x200007a3 = 0;
  *(uint8_t*)0x200007a4 = 0;
  *(uint8_t*)0x200007a5 = 0;
  *(uint8_t*)0x200007a6 = 0;
  *(uint8_t*)0x200007a7 = 0;
  *(uint8_t*)0x200007a8 = 0;
  *(uint8_t*)0x200007a9 = 0;
  *(uint8_t*)0x200007aa = 0;
  *(uint8_t*)0x200007ab = 0;
  *(uint16_t*)0x200007ac = 0xc0;
  *(uint16_t*)0x200007ae = 0xe8;
  *(uint32_t*)0x200007b0 = 0;
  *(uint64_t*)0x200007b8 = 0;
  *(uint64_t*)0x200007c0 = 0;
  *(uint16_t*)0x200007c8 = 0x28;
  memcpy((void*)0x200007ca, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x200007e7 = 0;
  *(uint32_t*)0x200007e8 = 0xfffffffe;
  syscall(__NR_setsockopt, r[0], 0, 0x60, 0x20000400, 0x3f0);
  *(uint8_t*)0x20010000 = -1;
  *(uint8_t*)0x20010001 = -1;
  *(uint8_t*)0x20010002 = -1;
  *(uint8_t*)0x20010003 = -1;
  *(uint8_t*)0x20010004 = -1;
  *(uint8_t*)0x20010005 = -1;
  *(uint8_t*)0x20010006 = 0;
  *(uint8_t*)0x20010007 = 0;
  *(uint8_t*)0x20010008 = 0;
  *(uint8_t*)0x20010009 = 0;
  *(uint8_t*)0x2001000a = 0;
  *(uint8_t*)0x2001000b = 0;
  *(uint16_t*)0x2001000c = htobe16(0x806);
  *(uint16_t*)0x2001000e = htobe16(1);
  *(uint16_t*)0x20010010 = htobe16(0x800);
  *(uint8_t*)0x20010012 = 6;
  *(uint8_t*)0x20010013 = 4;
  *(uint16_t*)0x20010014 = htobe16(0);
  *(uint8_t*)0x20010016 = -1;
  *(uint8_t*)0x20010017 = -1;
  *(uint8_t*)0x20010018 = -1;
  *(uint8_t*)0x20010019 = -1;
  *(uint8_t*)0x2001001a = -1;
  *(uint8_t*)0x2001001b = -1;
  *(uint8_t*)0x2001001c = 0xac;
  *(uint8_t*)0x2001001d = 0x14;
  *(uint8_t*)0x2001001e = 0;
  *(uint8_t*)0x2001001f = 0xbb;
  memcpy((void*)0x20010020, "\x87\x7c\x02\x00\x00\xa5", 6);
  *(uint8_t*)0x20010026 = 0xac;
  *(uint8_t*)0x20010027 = 0x14;
  *(uint8_t*)0x20010028 = 0;
  *(uint8_t*)0x20010029 = 0xaa;
  *(uint32_t*)0x20002000 = 0;
  *(uint32_t*)0x20002004 = 1;
  *(uint32_t*)0x20002008 = 0;
  syz_emit_ethernet(0x2a, 0x20010000, 0x20002000);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  use_temporary_dir();
  int pid = do_sandbox_namespace(0, true);
  int status = 0;
  while (waitpid(pid, &status, __WALL) != pid) {
  }
  return 0;
}
