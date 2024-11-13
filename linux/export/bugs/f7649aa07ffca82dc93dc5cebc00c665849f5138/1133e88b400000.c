// https://syzkaller.appspot.com/bug?id=f7649aa07ffca82dc93dc5cebc00c665849f5138
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

uint64_t r[3] = {0x0, 0xffffffffffffffff, 0x0};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000100, "keyring", 8);
  memcpy((void*)0x20000140, "syz", 3);
  *(uint8_t*)0x20000143 = 0;
  *(uint8_t*)0x20000144 = 0;
  res = syscall(__NR_add_key, 0x20000100, 0x20000140, 0, 0, 0xfffffffb);
  if (res != -1)
    r[0] = res;
  syscall(__NR_keyctl, 5, r[0], 0x4042000);
  res = syscall(__NR_socketpair, 1, 3, 0, 0x200016c0);
  if (res != -1)
    r[1] = *(uint32_t*)0x200016c0;
  *(uint32_t*)0x20cab000 = 0xc;
  res = syscall(__NR_getsockopt, r[1], 1, 0x11, 0x20caaffb, 0x20cab000);
  if (res != -1)
    r[2] = *(uint32_t*)0x20caafff;
  syscall(__NR_setresuid, r[2], 0, 0);
  memcpy((void*)0x20000000, "user", 5);
  memcpy((void*)0x20000040, "syz", 3);
  *(uint8_t*)0x20000043 = 0x20;
  *(uint8_t*)0x20000044 = 0;
  memcpy((void*)0x20000200, "keyring", 8);
  syscall(__NR_request_key, 0x20000000, 0x20000040, 0x20000200, r[0]);
  return 0;
}