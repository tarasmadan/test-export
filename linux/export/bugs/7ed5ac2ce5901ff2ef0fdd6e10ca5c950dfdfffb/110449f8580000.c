// https://syzkaller.appspot.com/bug?id=7ed5ac2ce5901ff2ef0fdd6e10ca5c950dfdfffb
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

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
  memcpy((void*)0x20000340, "/dev/kvm\000", 9);
  res = syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul, /*file=*/0x20000340ul,
                /*flags=*/0, /*mode=*/0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0xae01, /*type=*/0ul);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20000240 = 0;
  *(uint32_t*)0x20000244 = 0xda0;
  *(uint64_t*)0x20000248 = 0x20000080;
  memcpy((void*)0x20000080, "\x14\x2f\x91\xb1\xf9", 5);
  *(uint64_t*)0x20000250 = 0;
  *(uint8_t*)0x20000258 = 5;
  *(uint8_t*)0x20000259 = 0;
  memset((void*)0x2000025a, 0, 30);
  syscall(__NR_ioctl, /*fd=*/r[1], /*cmd=*/0x4038ae7a, /*arg=*/0x20000240ul);
  syscall(__NR_ioctl, /*fd=*/r[1], /*cmd=*/0xae41, /*id=*/0ul);
  return 0;
}
