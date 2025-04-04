// https://syzkaller.appspot.com/bug?id=8fc4ac2bd63ac049e7a369676b65f047f65d7736
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

#ifndef __NR_close_range
#define __NR_close_range 436
#endif
#ifndef __NR_seccomp
#define __NR_seccomp 317
#endif

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x3ffffffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x400000000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x400001000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  const char* reason;
  (void)reason;
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  *(uint16_t*)0x400000000040 = 1;
  *(uint64_t*)0x400000000048 = 0x400000000000;
  *(uint16_t*)0x400000000000 = 6;
  *(uint8_t*)0x400000000002 = 0;
  *(uint8_t*)0x400000000003 = 0;
  *(uint32_t*)0x400000000004 = 0x7fff7ffc;
  res = syscall(__NR_seccomp, /*op=*/1ul, /*flags=*/0ul,
                /*arg=*/0x400000000040ul);
  if (res != -1)
    r[0] = res;
  syscall(__NR_close_range, /*fd=*/r[0], /*max_fd=*/-1, /*flags=*/0ul);
  memcpy((void*)0x400000000040, "/dev/kvm\000", 9);
  res = syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul,
                /*file=*/0x400000000040ul, /*flags=*/0, /*mode=*/0);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_ioctl, /*fd=*/r[1], /*cmd=*/0xae01, /*type=*/0ul);
  if (res != -1)
    r[2] = res;
  memcpy((void*)0x400000000000, "/dev/ocfs2_control\000", 19);
  res = syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul,
                /*file=*/0x400000000000ul, /*flags=O_LARGEFILE*/ 0x8000,
                /*mode=*/0);
  if (res != -1)
    r[3] = res;
  syscall(__NR_ioctl, /*fd=*/r[2], /*cmd=*/0xae41, /*id=*/2ul);
  *(uint32_t*)0x400000000180 = 2;
  *(uint32_t*)0x400000000184 = r[3];
  *(uint32_t*)0x400000000188 = -1;
  memset((void*)0x40000000018c, 0, 12);
  syscall(__NR_ioctl, /*fd=*/r[2], /*cmd=*/0x400caed0,
          /*arg=*/0x400000000180ul);
  return 0;
}
