// https://syzkaller.appspot.com/bug?id=2482dbdc75808e95f27907b6a48676dab2f56417
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
  memcpy((void*)0x400000000200, "/sys/module/sg/parameters/scatter_elem_sz\000",
         42);
  res = syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul,
                /*file=*/0x400000000200ul,
                /*flags=O_SYNC|O_NOCTTY|O_CREAT|O_RDWR*/ 0x101142, /*mode=*/0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x400000000100,
         "0\000\246\314\r\221QU\235I\332\033\255\261\236\310Tt\250\224\234\212"
         "\342\307cOM\266\243,!o\236\260\255T\373R\241Y\224V[8\004c\337:]"
         "\331\224\370F\273\242\273>\255e\030\275\342\034\211OO]e["
         "\273\371\315\300\311\000\332\254\335\032\335\335\271o\032\253\325\357"
         "\300\004z\320I>\217\000\345\034*\355`"
         "\375\025\210\017\232\325\247\024\f};\253t\321ak\345\230\352\343}"
         "\020\253\f_\031\233\021\2625VUK\223\315d\027\344\313A\245[\b\270;"
         "\002tcf\006\373D\221\312G\332a:k["
         "r\006\353\360\304\313\020\256\310\351u\237\336K\245\216\326\217\320UV"
         "\021\313\335\201\276\336L/"
         "\006(\035\245\305\233\262\226\005`\347\325Y\a\301\351("
         "\225\337H\364\v\363C",
         213);
  syscall(__NR_write, /*fd=*/r[0], /*buf=*/0x400000000100ul, /*count=*/4ul);
  memcpy((void*)0x4000000000c0, "/dev/sg0\000", 9);
  syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul, /*file=*/0x4000000000c0ul,
          /*flags=O_LARGEFILE|O_CLOEXEC|FASYNC|O_RDWR*/ 0x8a002, /*mode=*/0);
  return 0;
}
