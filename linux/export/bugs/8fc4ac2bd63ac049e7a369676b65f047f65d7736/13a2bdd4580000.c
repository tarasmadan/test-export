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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffffffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul,
          /*fd=*/(intptr_t)-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200000000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul,
          /*fd=*/(intptr_t)-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200001000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul,
          /*fd=*/(intptr_t)-1, /*offset=*/0ul);
  const char* reason;
  (void)reason;
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  res = syscall(__NR_socket, /*domain=*/2ul, /*type=*/1ul, /*proto=*/0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x200000000000, "/dev/kvm\000", 9);
  res = syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul,
                /*file=*/0x200000000000ul, /*flags=O_NOFOLLOW*/ 0x20000,
                /*mode=*/0);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_ioctl, /*fd=*/r[1], /*cmd=*/0xae01, /*type=*/0ul);
  if (res != -1)
    r[2] = res;
  syscall(__NR_ioctl, /*fd=*/r[2], /*cmd=*/0xae41, /*id=*/3ul);
  *(uint32_t*)0x200000000400 = 3;
  *(uint32_t*)0x200000000404 = r[0];
  *(uint32_t*)0x200000000408 = -1;
  memset((void*)0x20000000040c, 0, 12);
  syscall(__NR_ioctl, /*fd=*/r[2], /*cmd=*/0x400caed0,
          /*arg=*/0x200000000400ul);
  return 0;
}
