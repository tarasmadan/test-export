// https://syzkaller.appspot.com/bug?id=3408a94500d4068ba34af9682f84a54d45e82ec3
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef __NR_mmap
#define __NR_mmap 90
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#ifndef __NR_setsockopt
#define __NR_setsockopt 366
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[4];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] = syscall(__NR_socket, 0xaul, 0x1ul, 0x0ul);
  memcpy((void*)0x20000000, "\x8f\xd5\xcd\x37\x43\xb0\xd8\xe1\xdf\x04"
                            "\x15\xd0\x5b\x20\x53\x6a\x59\x64\xab\x8c"
                            "\x33\xda\x05\x87\x11\xc5\x15\xf5\x3d\x55"
                            "\x8d\x8a\xbd\xd8\x30\x96\x17\x73\x5e\x86"
                            "\x28\xfa\xaf\xba\xc0\xfa\x4a\x81",
         48);
  r[3] = syscall(__NR_setsockopt, r[1], 0x29ul, 0x41ul, 0x20000000ul,
                 0x30ul);
}

int main()
{
  loop();
  return 0;
}
