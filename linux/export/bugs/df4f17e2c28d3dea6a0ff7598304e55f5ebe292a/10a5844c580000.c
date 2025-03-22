// https://syzkaller.appspot.com/bug?id=df4f17e2c28d3dea6a0ff7598304e55f5ebe292a
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
  syscall(__NR_mmap, /*addr=*/0x3ffffffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x400000000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x400001000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  const char* reason;
  (void)reason;
  intptr_t res = 0;
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  *(uint32_t*)0x400000000180 = 0;
  *(uint32_t*)0x400000000184 = 0xc;
  *(uint64_t*)0x400000000188 = 0x400000000440;
  memcpy((void*)0x400000000440,
         "\x18\x00\x00\x00\x00\x04\x47\x00\x00\x00\x00\x00\x00\x00\x00\x00\x18"
         "\x11\x00\x00",
         20);
  *(uint32_t*)0x400000000454 = -1;
  memcpy((void*)0x400000000458,
         "\x00\x00\x00\x00\x00\x00\x00\x00\xb7\x08\x00\x00\x02\x00\x00\x00\x7b"
         "\x8a\xf8\xff\x00\x00\x00\x00\xbf\xa2\x00\x00\x00\x00\x00\x00\x07\x02"
         "\x00\x00\xf8\xff\xff\xff\xb7\x03\x00\x00\x08\x00\x00\x00\xb7\x04\x00"
         "\x00\x00\x00\x00\x00\x85\x00\x00\x00\x03\x00\x00\x00\x95",
         65);
  *(uint64_t*)0x400000000190 = 0;
  *(uint32_t*)0x400000000198 = 0;
  *(uint32_t*)0x40000000019c = 0;
  *(uint64_t*)0x4000000001a0 = 0;
  *(uint32_t*)0x4000000001a8 = 0;
  *(uint32_t*)0x4000000001ac = 0;
  memset((void*)0x4000000001b0, 0, 16);
  *(uint32_t*)0x4000000001c0 = 0;
  *(uint32_t*)0x4000000001c4 = 0;
  *(uint32_t*)0x4000000001c8 = -1;
  *(uint32_t*)0x4000000001cc = 0;
  *(uint64_t*)0x4000000001d0 = 0;
  *(uint32_t*)0x4000000001d8 = 0;
  *(uint32_t*)0x4000000001dc = 0;
  *(uint64_t*)0x4000000001e0 = 0;
  *(uint32_t*)0x4000000001e8 = 0;
  *(uint32_t*)0x4000000001ec = 0;
  *(uint32_t*)0x4000000001f0 = 0;
  *(uint32_t*)0x4000000001f4 = 0;
  *(uint64_t*)0x4000000001f8 = 0;
  *(uint64_t*)0x400000000200 = 0;
  *(uint32_t*)0x400000000208 = 0;
  *(uint32_t*)0x40000000020c = 0;
  *(uint32_t*)0x400000000210 = 0;
  syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x400000000180ul, /*size=*/0x90ul);
  *(uint32_t*)0x4000000000c0 = 0;
  *(uint32_t*)0x4000000000c4 = 0xc;
  *(uint64_t*)0x4000000000c8 = 0x400000000440;
  memcpy((void*)0x400000000440,
         "\x18\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x18"
         "\x12\x00\x00",
         20);
  *(uint32_t*)0x400000000454 = -1;
  memcpy((void*)0x400000000458,
         "\x00\x00\x00\x00\x00\x00\x00\x00\xb7\x08\x00\x00\x00\x00\x00\x00\x7b"
         "\x8a\xf8\xff\x00\x00\x00\x00\xbf\xa2\x00\x00\x00\x00\x00\x00\x07\x02"
         "\x00\x00\xf8\xff\xff\xff\xb7\x03\x00\x00\x08\x00\x00\x00\xb7\x04\x00"
         "\x00\xf6\x00\x00\x00\x85\x00\x00\x00\x43",
         61);
  *(uint64_t*)0x4000000000d0 = 0;
  *(uint32_t*)0x4000000000d8 = 0;
  *(uint32_t*)0x4000000000dc = 0;
  *(uint64_t*)0x4000000000e0 = 0;
  *(uint32_t*)0x4000000000e8 = 0;
  *(uint32_t*)0x4000000000ec = 0;
  memset((void*)0x4000000000f0, 0, 16);
  *(uint32_t*)0x400000000100 = 0;
  *(uint32_t*)0x400000000104 = 0;
  *(uint32_t*)0x400000000108 = -1;
  *(uint32_t*)0x40000000010c = 0;
  *(uint64_t*)0x400000000110 = 0;
  *(uint32_t*)0x400000000118 = 0;
  *(uint32_t*)0x40000000011c = 0;
  *(uint64_t*)0x400000000120 = 0;
  *(uint32_t*)0x400000000128 = 0;
  *(uint32_t*)0x40000000012c = 0;
  *(uint32_t*)0x400000000130 = -1;
  *(uint32_t*)0x400000000134 = 0;
  *(uint64_t*)0x400000000138 = 0;
  *(uint64_t*)0x400000000140 = 0;
  *(uint32_t*)0x400000000148 = 0;
  *(uint32_t*)0x40000000014c = 0;
  *(uint32_t*)0x400000000150 = 0;
  syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x4000000000c0ul, /*size=*/0x90ul);
  *(uint32_t*)0x4000000000c0 = 0x1b;
  *(uint32_t*)0x4000000000c4 = 0;
  *(uint32_t*)0x4000000000c8 = 0;
  *(uint32_t*)0x4000000000cc = 0x8000;
  *(uint32_t*)0x4000000000d0 = 0;
  *(uint32_t*)0x4000000000d4 = -1;
  *(uint32_t*)0x4000000000d8 = 0;
  memset((void*)0x4000000000dc, 0, 16);
  *(uint32_t*)0x4000000000ec = 0;
  *(uint32_t*)0x4000000000f0 = -1;
  *(uint32_t*)0x4000000000f4 = 0;
  *(uint32_t*)0x4000000000f8 = 0;
  *(uint32_t*)0x4000000000fc = 0;
  *(uint64_t*)0x400000000100 = 0;
  *(uint32_t*)0x400000000108 = 0;
  *(uint32_t*)0x40000000010c = 0;
  res =
      syscall(__NR_bpf, /*cmd=*/0ul, /*arg=*/0x4000000000c0ul, /*size=*/0x48ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x4000000000c0 = 0;
  *(uint32_t*)0x4000000000c4 = 0xc;
  *(uint64_t*)0x4000000000c8 = 0x400000000440;
  memcpy((void*)0x400000000440,
         "\x18\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x18"
         "\x12\x00\x00",
         20);
  *(uint32_t*)0x400000000454 = r[0];
  *(uint64_t*)0x4000000000d0 = 0;
  *(uint32_t*)0x4000000000d8 = 0;
  *(uint32_t*)0x4000000000dc = 0;
  *(uint64_t*)0x4000000000e0 = 0;
  *(uint32_t*)0x4000000000e8 = 0;
  *(uint32_t*)0x4000000000ec = 0;
  memset((void*)0x4000000000f0, 0, 16);
  *(uint32_t*)0x400000000100 = 0;
  *(uint32_t*)0x400000000104 = 0;
  *(uint32_t*)0x400000000108 = -1;
  *(uint32_t*)0x40000000010c = 0;
  *(uint64_t*)0x400000000110 = 0;
  *(uint32_t*)0x400000000118 = 0;
  *(uint32_t*)0x40000000011c = 0;
  *(uint64_t*)0x400000000120 = 0;
  *(uint32_t*)0x400000000128 = 0;
  *(uint32_t*)0x40000000012c = 0;
  *(uint32_t*)0x400000000130 = 0;
  *(uint32_t*)0x400000000134 = 0;
  *(uint64_t*)0x400000000138 = 0;
  *(uint64_t*)0x400000000140 = 0;
  *(uint32_t*)0x400000000148 = 0;
  *(uint32_t*)0x40000000014c = 0;
  *(uint32_t*)0x400000000150 = 0;
  syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x4000000000c0ul, /*size=*/0x90ul);
  *(uint32_t*)0x4000000000c0 = 0x1f;
  *(uint32_t*)0x4000000000c4 = 0xc;
  *(uint64_t*)0x4000000000c8 = 0x400000000440;
  *(uint64_t*)0x4000000000d0 = 0x400000000240;
  memcpy((void*)0x400000000240, "GPL\000", 4);
  *(uint32_t*)0x4000000000d8 = 0;
  *(uint32_t*)0x4000000000dc = 0;
  *(uint64_t*)0x4000000000e0 = 0;
  *(uint32_t*)0x4000000000e8 = 0;
  *(uint32_t*)0x4000000000ec = 0x12;
  memset((void*)0x4000000000f0, 0, 16);
  *(uint32_t*)0x400000000100 = 0;
  *(uint32_t*)0x400000000104 = 0;
  *(uint32_t*)0x400000000108 = -1;
  *(uint32_t*)0x40000000010c = 0;
  *(uint64_t*)0x400000000110 = 0;
  *(uint32_t*)0x400000000118 = 0;
  *(uint32_t*)0x40000000011c = 0;
  *(uint64_t*)0x400000000120 = 0;
  *(uint32_t*)0x400000000128 = 0;
  *(uint32_t*)0x40000000012c = 0;
  *(uint32_t*)0x400000000130 = 0;
  *(uint32_t*)0x400000000134 = 0x3b;
  *(uint64_t*)0x400000000138 = 0;
  *(uint64_t*)0x400000000140 = 0;
  *(uint32_t*)0x400000000148 = 0;
  *(uint32_t*)0x40000000014c = 0;
  *(uint32_t*)0x400000000150 = 0;
  res =
      syscall(__NR_bpf, /*cmd=*/5ul, /*arg=*/0x4000000000c0ul, /*size=*/0x90ul);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x400000000740 = r[1];
  *(uint32_t*)0x400000000744 = 0;
  *(uint64_t*)0x400000000748 = 0;
  syscall(__NR_bpf, /*cmd=*/0xaul, /*arg=*/0x400000000740ul, /*size=*/0x10ul);
  return 0;
}
