// https://syzkaller.appspot.com/bug?id=0d259373da8be7356652213543e1efc254a5abf0
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

long r[3];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
          0xfffffffffffffffful, 0x0ul);
  memcpy((void*)0x2028bff7, "/dev/kvm", 9);
  r[0] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x2028bff7ul, 0x0ul,
                 0x0ul);
  r[1] = syscall(__NR_ioctl, r[0], 0xae01ul, 0x0ul);
  r[2] = syscall(__NR_ioctl, r[1], 0xae41ul, 0x2ul);
  *(uint64_t*)0x20c87000 = (uint64_t)0xf000;
  *(uint32_t*)0x20c87008 = (uint32_t)0x1000;
  *(uint16_t*)0x20c8700c = (uint16_t)0xd;
  *(uint8_t*)0x20c8700e = (uint8_t)0x8001;
  *(uint8_t*)0x20c8700f = (uint8_t)0xffff;
  *(uint8_t*)0x20c87010 = (uint8_t)0x7639;
  *(uint8_t*)0x20c87011 = (uint8_t)0x2;
  *(uint8_t*)0x20c87012 = (uint8_t)0x53b000000000000;
  *(uint8_t*)0x20c87013 = (uint8_t)0x9;
  *(uint8_t*)0x20c87014 = (uint8_t)0x5;
  *(uint8_t*)0x20c87015 = (uint8_t)0x2;
  *(uint8_t*)0x20c87016 = (uint8_t)0x80;
  *(uint8_t*)0x20c87017 = (uint8_t)0x0;
  *(uint64_t*)0x20c87018 = (uint64_t)0x3000;
  *(uint32_t*)0x20c87020 = (uint32_t)0x2;
  *(uint16_t*)0x20c87024 = (uint16_t)0x0;
  *(uint8_t*)0x20c87026 = (uint8_t)0x1000;
  *(uint8_t*)0x20c87027 = (uint8_t)0xe57;
  *(uint8_t*)0x20c87028 = (uint8_t)0x6;
  *(uint8_t*)0x20c87029 = (uint8_t)0x4;
  *(uint8_t*)0x20c8702a = (uint8_t)0x5;
  *(uint8_t*)0x20c8702b = (uint8_t)0x5;
  *(uint8_t*)0x20c8702c = (uint8_t)0x8001;
  *(uint8_t*)0x20c8702d = (uint8_t)0x1;
  *(uint8_t*)0x20c8702e = (uint8_t)0x2;
  *(uint8_t*)0x20c8702f = (uint8_t)0x0;
  *(uint64_t*)0x20c87030 = (uint64_t)0x0;
  *(uint32_t*)0x20c87038 = (uint32_t)0x9232958014386b78;
  *(uint16_t*)0x20c8703c = (uint16_t)0xe;
  *(uint8_t*)0x20c8703e = (uint8_t)0x1c4d;
  *(uint8_t*)0x20c8703f = (uint8_t)0x80000;
  *(uint8_t*)0x20c87040 = (uint8_t)0x9;
  *(uint8_t*)0x20c87041 = (uint8_t)0x81;
  *(uint8_t*)0x20c87042 = (uint8_t)0x9;
  *(uint8_t*)0x20c87043 = (uint8_t)0x80;
  *(uint8_t*)0x20c87044 = (uint8_t)0x1;
  *(uint8_t*)0x20c87045 = (uint8_t)0x1;
  *(uint8_t*)0x20c87046 = (uint8_t)0x5;
  *(uint8_t*)0x20c87047 = (uint8_t)0x0;
  *(uint64_t*)0x20c87048 = (uint64_t)0x104005;
  *(uint32_t*)0x20c87050 = (uint32_t)0x5000;
  *(uint16_t*)0x20c87054 = (uint16_t)0x1a;
  *(uint8_t*)0x20c87056 = (uint8_t)0x1;
  *(uint8_t*)0x20c87057 = (uint8_t)0x9;
  *(uint8_t*)0x20c87058 = (uint8_t)0x4;
  *(uint8_t*)0x20c87059 = (uint8_t)0xf5f;
  *(uint8_t*)0x20c8705a = (uint8_t)0x2e;
  *(uint8_t*)0x20c8705b = (uint8_t)0x0;
  *(uint8_t*)0x20c8705c = (uint8_t)0x3;
  *(uint8_t*)0x20c8705d = (uint8_t)0x0;
  *(uint8_t*)0x20c8705e = (uint8_t)0x0;
  *(uint8_t*)0x20c8705f = (uint8_t)0x0;
  *(uint64_t*)0x20c87060 = (uint64_t)0x10d000;
  *(uint32_t*)0x20c87068 = (uint32_t)0x0;
  *(uint16_t*)0x20c8706c = (uint16_t)0xb;
  *(uint8_t*)0x20c8706e = (uint8_t)0x100000000;
  *(uint8_t*)0x20c8706f = (uint8_t)0x1;
  *(uint8_t*)0x20c87070 = (uint8_t)0xfffffffffffffffb;
  *(uint8_t*)0x20c87071 = (uint8_t)0x80000000;
  *(uint8_t*)0x20c87072 = (uint8_t)0x1;
  *(uint8_t*)0x20c87073 = (uint8_t)0xffffffff80000000;
  *(uint8_t*)0x20c87074 = (uint8_t)0x4;
  *(uint8_t*)0x20c87075 = (uint8_t)0x8;
  *(uint8_t*)0x20c87076 = (uint8_t)0x3ff;
  *(uint8_t*)0x20c87077 = (uint8_t)0x0;
  *(uint64_t*)0x20c87078 = (uint64_t)0x4;
  *(uint32_t*)0x20c87080 = (uint32_t)0x2002;
  *(uint16_t*)0x20c87084 = (uint16_t)0x0;
  *(uint8_t*)0x20c87086 = (uint8_t)0x8;
  *(uint8_t*)0x20c87087 = (uint8_t)0x8;
  *(uint8_t*)0x20c87088 = (uint8_t)0xfffffffffffffff9;
  *(uint8_t*)0x20c87089 = (uint8_t)0x4;
  *(uint8_t*)0x20c8708a = (uint8_t)0x25;
  *(uint8_t*)0x20c8708b = (uint8_t)0x4;
  *(uint8_t*)0x20c8708c = (uint8_t)0x4;
  *(uint8_t*)0x20c8708d = (uint8_t)0x0;
  *(uint8_t*)0x20c8708e = (uint8_t)0x7;
  *(uint8_t*)0x20c8708f = (uint8_t)0x0;
  *(uint64_t*)0x20c87090 = (uint64_t)0xf000;
  *(uint32_t*)0x20c87098 = (uint32_t)0x5000;
  *(uint16_t*)0x20c8709c = (uint16_t)0x1c;
  *(uint8_t*)0x20c8709e = (uint8_t)0x4;
  *(uint8_t*)0x20c8709f = (uint8_t)0x6;
  *(uint8_t*)0x20c870a0 = (uint8_t)0x5;
  *(uint8_t*)0x20c870a1 = (uint8_t)0x7;
  *(uint8_t*)0x20c870a2 = (uint8_t)0x7ff;
  *(uint8_t*)0x20c870a3 = (uint8_t)0xffff;
  *(uint8_t*)0x20c870a4 = (uint8_t)0x4;
  *(uint8_t*)0x20c870a5 = (uint8_t)0x3f;
  *(uint8_t*)0x20c870a6 = (uint8_t)0x3;
  *(uint8_t*)0x20c870a7 = (uint8_t)0x0;
  *(uint64_t*)0x20c870a8 = (uint64_t)0x2;
  *(uint32_t*)0x20c870b0 = (uint32_t)0x3000;
  *(uint16_t*)0x20c870b4 = (uint16_t)0xf;
  *(uint8_t*)0x20c870b6 = (uint8_t)0x5;
  *(uint8_t*)0x20c870b7 = (uint8_t)0x7;
  *(uint8_t*)0x20c870b8 = (uint8_t)0x7f;
  *(uint8_t*)0x20c870b9 = (uint8_t)0x9;
  *(uint8_t*)0x20c870ba = (uint8_t)0x3;
  *(uint8_t*)0x20c870bb = (uint8_t)0x6;
  *(uint8_t*)0x20c870bc = (uint8_t)0xffff;
  *(uint8_t*)0x20c870bd = (uint8_t)0x5;
  *(uint8_t*)0x20c870be = (uint8_t)0xfffffffffffffeff;
  *(uint8_t*)0x20c870bf = (uint8_t)0x0;
  *(uint64_t*)0x20c870c0 = (uint64_t)0x2000;
  *(uint16_t*)0x20c870c8 = (uint16_t)0x6;
  *(uint16_t*)0x20c870ca = (uint16_t)0x0;
  *(uint16_t*)0x20c870cc = (uint16_t)0x0;
  *(uint16_t*)0x20c870ce = (uint16_t)0x0;
  *(uint64_t*)0x20c870d0 = (uint64_t)0x1f000;
  *(uint16_t*)0x20c870d8 = (uint16_t)0x5000;
  *(uint16_t*)0x20c870da = (uint16_t)0x0;
  *(uint16_t*)0x20c870dc = (uint16_t)0x0;
  *(uint16_t*)0x20c870de = (uint16_t)0x0;
  *(uint64_t*)0x20c870e0 = (uint64_t)0x80000004;
  *(uint64_t*)0x20c870e8 = (uint64_t)0x0;
  *(uint64_t*)0x20c870f0 = (uint64_t)0x10f000;
  *(uint64_t*)0x20c870f8 = (uint64_t)0x400004;
  *(uint64_t*)0x20c87100 = (uint64_t)0x4;
  *(uint64_t*)0x20c87108 = (uint64_t)0x2000;
  *(uint64_t*)0x20c87110 = (uint64_t)0x4;
  *(uint64_t*)0x20c87118 = (uint64_t)0x5;
  *(uint64_t*)0x20c87120 = (uint64_t)0x8001;
  *(uint64_t*)0x20c87128 = (uint64_t)0x0;
  *(uint64_t*)0x20c87130 = (uint64_t)0x80000000;
  syscall(__NR_ioctl, r[2], 0x4138ae84ul, 0x20c87000ul);
}

int main()
{
  loop();
  return 0;
}