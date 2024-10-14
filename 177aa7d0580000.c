// https://syzkaller.appspot.com/bug?id=eb0aa4a75a41bff452007f9646b91005ece85bc3
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
#ifndef __NR_userfaultfd
#define __NR_userfaultfd 282
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
  syscall(
      __NR_mmap, /*addr=*/0x20400000ul, /*len=*/0xc00000ul, /*prot=*/0ul,
      /*flags=MAP_POPULATE|MAP_NORESERVE|MAP_NONBLOCK|MAP_HUGETLB|MAP_FIXED|0x1021*/
      0x5d031ul, /*fd=*/-1, /*offset=*/0ul);
  res = syscall(__NR_userfaultfd,
                /*flags=UFFD_USER_MODE_ONLY|O_NONBLOCK*/ 0x801ul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000100 = 0xaa;
  *(uint64_t*)0x20000108 = 0;
  *(uint64_t*)0x20000110 = 0;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0xc018aa3f, /*arg=*/0x20000100ul);
  *(uint64_t*)0x20000040 = 0x20400000;
  *(uint64_t*)0x20000048 = 0xc00000;
  *(uint64_t*)0x20000050 = 5;
  *(uint64_t*)0x20000058 = 0;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0xc020aa00, /*arg=*/0x20000040ul);
  syscall(
      __NR_mmap, /*addr=*/0x20400000ul, /*len=*/0xc00000ul, /*prot=*/0ul,
      /*flags=MAP_POPULATE|MAP_NORESERVE|MAP_NONBLOCK|MAP_HUGETLB|MAP_FIXED|0x1021*/
      0x5d031ul, /*fd=*/-1, /*offset=*/0ul);
  return 0;
}
