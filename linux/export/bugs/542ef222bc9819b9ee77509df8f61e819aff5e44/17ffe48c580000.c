// https://syzkaller.appspot.com/bug?id=542ef222bc9819b9ee77509df8f61e819aff5e44
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
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  memcpy((void*)0x200000001380, "./file0\000", 8);
  syscall(__NR_creat, /*file=*/0x200000001380ul, /*mode=S_IROTH*/ 4ul);
  memcpy((void*)0x200000001440, "./file0\000", 8);
  memcpy((void*)0x200000000000, "rpc_pipefs\000", 11);
  syscall(__NR_mount, /*src=*/0ul, /*dst=*/0x200000001440ul,
          /*type=*/0x200000000000ul, /*flags=*/0ul, /*data=*/0ul);
  return 0;
}
