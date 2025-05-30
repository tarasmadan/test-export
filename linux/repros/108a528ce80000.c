// https://syzkaller.appspot.com/bug?id=d235582be5214054da8aea1b32a063a2e8a5d97d
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

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul, /*prot=*/7ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=*/0x32ul, /*fd=*/-1, /*offset=*/0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, /*domain=*/1ul, /*type=*/1ul, /*proto=*/0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000200 = 0xf;
  *(uint32_t*)0x20000204 = 4;
  *(uint32_t*)0x20000208 = 4;
  *(uint32_t*)0x2000020c = 0x12;
  *(uint32_t*)0x20000210 = 0;
  *(uint32_t*)0x20000214 = -1;
  *(uint32_t*)0x20000218 = 0;
  memset((void*)0x2000021c, 0, 16);
  *(uint32_t*)0x2000022c = 0;
  *(uint32_t*)0x20000230 = -1;
  *(uint32_t*)0x20000234 = 0;
  *(uint32_t*)0x20000238 = 0;
  *(uint32_t*)0x2000023c = 0;
  *(uint64_t*)0x20000240 = 0;
  res = syscall(__NR_bpf, /*cmd=*/0ul, /*arg=*/0x20000200ul, /*size=*/0x48ul);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20000140 = r[1];
  *(uint64_t*)0x20000148 = 0x20000000;
  *(uint64_t*)0x20000150 = 0x20000100;
  *(uint32_t*)0x20000100 = r[0];
  *(uint64_t*)0x20000158 = 0;
  syscall(__NR_bpf, /*cmd=*/2ul, /*arg=*/0x20000140ul, /*size=*/0x20ul);
  return 0;
}
