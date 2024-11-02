// https://syzkaller.appspot.com/bug?id=34f977f6e45547b7385a93f051f23a3bdba01f76
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef __NR_ioctl
#define __NR_ioctl 29
#endif
#ifndef __NR_mmap
#define __NR_mmap 222
#endif

const int kInitNetNsFd = 201;

static long syz_init_net_socket(volatile long domain, volatile long type,
                                volatile long proto)
{
  return syscall(__NR_socket, domain, type, proto);
}

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul, /*prot=*/7ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  intptr_t res = 0;
  res = -1;
  res = syz_init_net_socket(/*domain=*/0x10, /*type=*/3, /*proto=*/0x10);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000000, "bond0\000\000\000\000\000\000\000\000\000\000\000",
         16);
  memcpy((void*)0x20000010, "ip6gre0\000\000\000\000\000\000\000\000\000", 16);
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x8990, /*arg=*/0x20000000ul);
  return 0;
}