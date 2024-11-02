// https://syzkaller.appspot.com/bug?id=44b1dd9a2c253efd99e0087b25f20d301e3defa7
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

#ifndef __NR_userfaultfd
#define __NR_userfaultfd 323
#endif

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
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  res = syscall(__NR_userfaultfd,
                /*flags=UFFD_USER_MODE_ONLY|O_CLOEXEC*/ 0x80001ul);
  if (res != -1)
    r[0] = res;
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0xb36000ul,
          /*prot=PROT_SEM|PROT_WRITE|PROT_READ|PROT_EXEC*/ 0xful,
          /*flags=MAP_UNINITIALIZED|MAP_POPULATE|MAP_FIXED|MAP_ANONYMOUS|0x2*/
          0x4008032ul, /*fd=*/-1, /*offset=*/0xffffc000ul);
  *(uint64_t*)0x20000000 = 0xaa;
  *(uint64_t*)0x20000008 = 0;
  *(uint64_t*)0x20000010 = 0;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0xc018aa3f, /*arg=*/0x20000000ul);
  *(uint64_t*)0x20000080 = 0x200e2000;
  *(uint64_t*)0x20000088 = 0xc00000;
  *(uint64_t*)0x20000090 = 2;
  *(uint64_t*)0x20000098 = 0;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0xc020aa00, /*arg=*/0x20000080ul);
  *(uint64_t*)0x200000c0 = 0x20497000;
  *(uint64_t*)0x200000c8 = 0x7fffdfb68000;
  *(uint64_t*)0x200000d0 = 1;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0xc018aa06, /*arg=*/0x200000c0ul);
  syscall(__NR_mremap, /*addr=*/0x20a96000ul, /*len=*/0x1000ul,
          /*newlen=*/0x800000ul, /*flags=MREMAP_FIXED|MREMAP_MAYMOVE*/ 3ul,
          /*newaddr=*/0x20130000ul);
  syscall(__NR_mlock, /*addr=*/0x20000000ul, /*size=*/0x800000ul);
  *(uint64_t*)0x20000040 = 0x7f;
  *(uint64_t*)0x20000300 = 0xa;
  syscall(__NR_migrate_pages, /*pid=*/0, /*maxnode=*/3ul, /*old=*/0x20000040ul,
          /*new=*/0x20000300ul);
  return 0;
}