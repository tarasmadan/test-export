// https://syzkaller.appspot.com/bug?id=8e030beda49b8e549cfeb17ed369414ed27331e6
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
  memcpy((void*)0x20000040, "/proc/self/maps\000", 16);
  res =
      syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul, /*file=*/0x20000040ul,
              /*flags=O_TRUNC|O_NOCTTY|O_NOATIME|O_RDWR*/ 0x40302, /*mode=*/0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_mmap, /*addr=*/0ul, /*len=*/0x20009ul, /*prot=*/0xdful,
          /*flags=*/0xeb1ul, /*fd=*/0x401, /*off=*/0x8000ul);
  *(uint64_t*)0x20000080 = 0x101;
  *(uint64_t*)0x20000088 = 0x3c;
  *(uint64_t*)0x20000090 = 0x7fff;
  *(uint64_t*)0x20000098 = 5;
  *(uint64_t*)0x200000a0 = 0x80000000009;
  *(uint64_t*)0x200000a8 = 1;
  *(uint64_t*)0x200000b0 = 0x800;
  *(uint64_t*)0x200000b8 = 0x101;
  *(uint64_t*)0x200000c0 = 5;
  *(uint32_t*)0x200000c8 = 0x7f93;
  *(uint32_t*)0x200000cc = 0xfffffffe;
  *(uint32_t*)0x200000d0 = 0x7ffffffd;
  *(uint32_t*)0x200000d4 = 0x7ff;
  *(uint64_t*)0x200000d8 = 7;
  *(uint64_t*)0x200000e0 = 9;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0xc0686611, /*arg=*/0x20000080ul);
  return 0;
}
