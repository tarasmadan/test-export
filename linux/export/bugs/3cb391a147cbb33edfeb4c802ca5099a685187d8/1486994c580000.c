// https://syzkaller.appspot.com/bug?id=3cb391a147cbb33edfeb4c802ca5099a685187d8
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
  res = syscall(__NR_socket, /*domain=*/0xaul,
                /*type=SOCK_CLOEXEC|SOCK_DGRAM*/ 0x80002ul, /*proto=*/0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_setsockopt, /*fd=*/r[0], /*level=*/0x29,
          /*optname=IPV6_JOIN_ANYCAST*/ 0x1b, /*optval=*/0ul, /*optlen=*/0ul);
  res = syscall(__NR_socket, /*domain=*/0xaul, /*type=SOCK_STREAM*/ 1ul,
                /*proto=*/0x100);
  if (res != -1)
    r[1] = res;
  *(uint8_t*)0x200000000100 = 0xfe;
  *(uint8_t*)0x200000000101 = 0x80;
  memset((void*)0x200000000102, 0, 13);
  *(uint8_t*)0x20000000010f = 0xbb;
  *(uint32_t*)0x200000000110 = 0;
  syscall(__NR_setsockopt, /*fd=*/r[1], /*level=*/0x29,
          /*optname=IPV6_LEAVE_ANYCAST|0x2*/ 0x1e, /*optval=*/0x200000000100ul,
          /*optlen=*/0x14ul);
  *(uint8_t*)0x200000000000 = 0xfe;
  *(uint8_t*)0x200000000001 = 0x80;
  memset((void*)0x200000000002, 0, 13);
  *(uint8_t*)0x20000000000f = 0xbb;
  *(uint32_t*)0x200000000010 = 0;
  syscall(__NR_setsockopt, /*fd=*/r[1], /*level=*/0x29,
          /*optname=IPV6_JOIN_ANYCAST*/ 0x1b, /*optval=*/0x200000000000ul,
          /*optlen=*/0x14ul);
  return 0;
}
