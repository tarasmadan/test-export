// https://syzkaller.appspot.com/bug?id=ce59fcac840fa844bc2dc839738779c0a0145445
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
  syscall(
      __NR_mmap, /*addr=*/0x20000000ul, /*len=*/0xff5000ul,
      /*prot=PROT_GROWSDOWN|PROT_WRITE*/ 0x1000002ul,
      /*flags=MAP_POPULATE|MAP_NORESERVE|MAP_NONBLOCK|MAP_HUGETLB|MAP_FIXED|0x2000000000821*/
      0x200000005c831ul, /*fd=*/-1, /*offset=*/0ul);
  memcpy((void*)0x20000040, "/proc/sys/vm/drop_caches\000", 25);
  res = syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul, /*file=*/0x20000040ul,
                /*flags=*/1, /*mode=*/0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x200000c0 = 0x200002c0;
  memset((void*)0x200002c0, 51, 1);
  *(uint64_t*)0x200000c8 = 1;
  syscall(__NR_writev, /*fd=*/r[0], /*vec=*/0x200000c0ul, /*vlen=*/1ul);
  return 0;
}