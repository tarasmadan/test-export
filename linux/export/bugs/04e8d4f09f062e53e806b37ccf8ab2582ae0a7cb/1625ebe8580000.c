// https://syzkaller.appspot.com/bug?id=04e8d4f09f062e53e806b37ccf8ab2582ae0a7cb
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
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  memcpy((void*)0x20000000, "/sys/kernel/security/lockdown\000", 30);
  syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul, /*file=*/0x20000000ul,
          /*flags=O_TRUNC|O_NOFOLLOW|O_EXCL|O_APPEND|O_RDWR*/ 0x20682,
          /*mode=*/0);
  syscall(__NR_write, /*fd=*/3, /*buf=*/0ul, /*count=*/0xfffffdeful);
  return 0;
}
