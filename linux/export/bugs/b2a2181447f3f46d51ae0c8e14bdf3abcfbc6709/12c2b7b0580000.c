// https://syzkaller.appspot.com/bug?id=b2a2181447f3f46d51ae0c8e14bdf3abcfbc6709
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

static long syz_open_procfs(volatile long a0, volatile long a1)
{
  char buf[128];
  memset(buf, 0, sizeof(buf));
  if (a0 == 0) {
    snprintf(buf, sizeof(buf), "/proc/self/%s", (char*)a1);
  } else if (a0 == -1) {
    snprintf(buf, sizeof(buf), "/proc/thread-self/%s", (char*)a1);
  } else {
    snprintf(buf, sizeof(buf), "/proc/self/task/%d/%s", (int)a0, (char*)a1);
  }
  int fd = open(buf, O_RDWR);
  if (fd == -1)
    fd = open(buf, O_RDONLY);
  return fd;
}

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
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  *(uint32_t*)0x200000000400 = 0xe;
  *(uint32_t*)0x200000000404 = 4;
  *(uint32_t*)0x200000000408 = 4;
  *(uint32_t*)0x20000000040c = 3;
  *(uint32_t*)0x200000000410 = 0;
  *(uint32_t*)0x200000000414 = -1;
  *(uint32_t*)0x200000000418 = 0;
  memset((void*)0x20000000041c, 0, 16);
  *(uint32_t*)0x20000000042c = 0;
  *(uint32_t*)0x200000000430 = -1;
  *(uint32_t*)0x200000000434 = 0;
  *(uint32_t*)0x200000000438 = 0;
  *(uint32_t*)0x20000000043c = 0;
  *(uint64_t*)0x200000000440 = 0;
  *(uint32_t*)0x200000000448 = 0;
  *(uint32_t*)0x20000000044c = 0;
  syscall(__NR_bpf, /*cmd=*/0ul, /*arg=*/0x200000000400ul, /*size=*/0x48ul);
  memcpy((void*)0x2000000001c0, "fd/3\000", 5);
  syz_open_procfs(/*pid=*/0, /*file=*/0x2000000001c0);
  return 0;
}
