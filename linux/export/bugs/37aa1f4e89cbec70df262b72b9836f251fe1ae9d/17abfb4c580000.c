// https://syzkaller.appspot.com/bug?id=37aa1f4e89cbec70df262b72b9836f251fe1ae9d
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
  res = syscall(__NR_socket, /*domain=*/0xaul, /*type=SOCK_DGRAM*/ 2ul,
                /*proto=*/0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x200000000080 = 0xa40;
  syscall(__NR_setsockopt, /*fd=*/r[0], /*level=*/0x11, /*optname=*/0x68,
          /*optval=*/0x200000000080ul, /*optlen=*/4ul);
  *(uint32_t*)0x200000000000 = 2;
  syscall(__NR_setsockopt, /*fd=*/r[0], /*level=*/0x11, /*optname=*/0x64,
          /*optval=*/0x200000000000ul, /*optlen=*/4ul);
  return 0;
}
