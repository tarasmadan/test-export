// https://syzkaller.appspot.com/bug?id=ce9e03bcb92d5f6c328a814bc5a74bde9e8e885e
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
  syscall(__NR_mmap, /*addr=*/0x1ffffffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200000000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x200001000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  const char* reason;
  (void)reason;
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  memcpy((void*)0x200000000040,
         "/sys/devices/virtual/block/loop12/queue/nr_requests\000", 52);
  res = syscall(
      __NR_openat, /*fd=*/0xffffffffffffff9cul, /*file=*/0x200000000040ul,
      /*flags=O_TRUNC|O_NOCTTY|O_CLOEXEC|O_RDWR*/ 0x80302, /*mode=*/0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_sendfile, /*out_fd=*/r[0], /*in_fd=*/r[0], /*offset=*/0ul,
          /*count=*/2ul);
  return 0;
}
