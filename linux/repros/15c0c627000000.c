// https://syzkaller.appspot.com/bug?id=6bc52c9094f38b8dc7692f97892516fa0d16f0f6
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

static void test();

void loop()
{
  while (1) {
    test();
  }
}

long r[91];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    memcpy(
        (void*)0x206b9000,
        "\x2f\x64\x65\x76\x2f\x73\x65\x71\x75\x65\x6e\x63\x65\x72\x00",
        15);
    r[2] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x206b9000ul,
                   0x48002ul, 0x0ul);
    break;
  case 2:
    *(uint8_t*)0x20e7f000 = (uint8_t)0x0;
    *(uint8_t*)0x20e7f001 = (uint8_t)0x7;
    *(uint8_t*)0x20e7f002 = (uint8_t)0x0;
    *(uint8_t*)0x20e7f003 = (uint8_t)0x0;
    *(uint64_t*)0x20e7f008 = (uint64_t)0x0;
    *(uint64_t*)0x20e7f010 = (uint64_t)0x0;
    *(uint8_t*)0x20e7f018 = (uint8_t)0xfe;
    *(uint8_t*)0x20e7f019 = (uint8_t)0x100000000;
    *(uint8_t*)0x20e7f01a = (uint8_t)0x0;
    *(uint8_t*)0x20e7f01b = (uint8_t)0x0;
    *(uint8_t*)0x20e7f020 = (uint8_t)0x0;
    *(uint8_t*)0x20e7f021 = (uint8_t)0x401;
    *(uint8_t*)0x20e7f022 = (uint8_t)0x7fc;
    *(uint8_t*)0x20e7f023 = (uint8_t)0x0;
    *(uint8_t*)0x20e7f030 = (uint8_t)0x10000;
    *(uint8_t*)0x20e7f031 = (uint8_t)0x7;
    *(uint8_t*)0x20e7f032 = (uint8_t)0x3;
    *(uint8_t*)0x20e7f033 = (uint8_t)0x0;
    *(uint64_t*)0x20e7f038 = (uint64_t)0x0;
    *(uint64_t*)0x20e7f040 = (uint64_t)0x0;
    *(uint8_t*)0x20e7f048 = (uint8_t)0x0;
    *(uint8_t*)0x20e7f049 = (uint8_t)0x3;
    *(uint8_t*)0x20e7f04a = (uint8_t)0x0;
    *(uint8_t*)0x20e7f04b = (uint8_t)0x0;
    *(uint8_t*)0x20e7f050 = (uint8_t)0x5;
    *(uint8_t*)0x20e7f051 = (uint8_t)0x5f0;
    *(uint8_t*)0x20e7f060 = (uint8_t)0x0;
    *(uint8_t*)0x20e7f061 = (uint8_t)0x0;
    *(uint8_t*)0x20e7f062 = (uint8_t)0x0;
    *(uint8_t*)0x20e7f063 = (uint8_t)0x0;
    *(uint32_t*)0x20e7f068 = (uint32_t)0x7fff;
    *(uint8_t*)0x20e7f078 = (uint8_t)0x0;
    *(uint8_t*)0x20e7f079 = (uint8_t)0x0;
    *(uint8_t*)0x20e7f07a = (uint8_t)0x0;
    *(uint8_t*)0x20e7f07b = (uint8_t)0x0;
    *(uint32_t*)0x20e7f080 = (uint32_t)0x0;
    *(uint32_t*)0x20e7f084 = (uint32_t)0x0;
    *(uint32_t*)0x20e7f088 = (uint32_t)0x0;
    *(uint8_t*)0x20e7f090 = (uint8_t)0x39;
    *(uint8_t*)0x20e7f091 = (uint8_t)0x3;
    *(uint8_t*)0x20e7f092 = (uint8_t)0x80;
    *(uint8_t*)0x20e7f093 = (uint8_t)0x3;
    *(uint32_t*)0x20e7f098 = (uint32_t)0x1;
    *(uint8_t*)0x20e7f0a8 = (uint8_t)0xb13;
    *(uint8_t*)0x20e7f0a9 = (uint8_t)0x100000001;
    *(uint8_t*)0x20e7f0aa = (uint8_t)0x0;
    *(uint8_t*)0x20e7f0ab = (uint8_t)0x0;
    *(uint32_t*)0x20e7f0b0 = (uint32_t)0x5;
    *(uint32_t*)0x20e7f0b4 = (uint32_t)0x0;
    *(uint32_t*)0x20e7f0b8 = (uint32_t)0x0;
    *(uint8_t*)0x20e7f0c0 = (uint8_t)0x6;
    *(uint8_t*)0x20e7f0c1 = (uint8_t)0x0;
    *(uint8_t*)0x20e7f0c2 = (uint8_t)0x0;
    *(uint8_t*)0x20e7f0c3 = (uint8_t)0x0;
    *(uint64_t*)0x20e7f0c8 = (uint64_t)0x0;
    *(uint64_t*)0x20e7f0d0 = (uint64_t)0x989680;
    *(uint8_t*)0x20e7f0d8 = (uint8_t)0x0;
    *(uint8_t*)0x20e7f0d9 = (uint8_t)0x0;
    *(uint8_t*)0x20e7f0da = (uint8_t)0x0;
    *(uint8_t*)0x20e7f0db = (uint8_t)0x0;
    *(uint8_t*)0x20e7f0e0 = (uint8_t)0x0;
    *(uint32_t*)0x20e7f0e4 = (uint32_t)0x800;
    *(uint32_t*)0x20e7f0e8 = (uint32_t)0x5;
    *(uint8_t*)0x20e7f0f0 = (uint8_t)0x1;
    *(uint8_t*)0x20e7f0f1 = (uint8_t)0x0;
    *(uint8_t*)0x20e7f0f2 = (uint8_t)0xffffffffffffffc1;
    *(uint8_t*)0x20e7f0f3 = (uint8_t)0x0;
    *(uint64_t*)0x20e7f0f8 = (uint64_t)0x0;
    *(uint64_t*)0x20e7f100 = (uint64_t)0x989680;
    *(uint8_t*)0x20e7f108 = (uint8_t)0x7fffffff;
    *(uint8_t*)0x20e7f109 = (uint8_t)0x0;
    *(uint8_t*)0x20e7f10a = (uint8_t)0x0;
    *(uint8_t*)0x20e7f10b = (uint8_t)0x3ff;
    memcpy((void*)0x20e7f110,
           "\x84\xa4\x8f\x22\x90\x39\xe6\x42\xe0\xaa\xb8\xe5", 12);
    *(uint8_t*)0x20e7f120 = (uint8_t)0x800;
    *(uint8_t*)0x20e7f121 = (uint8_t)0x6;
    *(uint8_t*)0x20e7f122 = (uint8_t)0x8;
    *(uint8_t*)0x20e7f123 = (uint8_t)0x2;
    *(uint32_t*)0x20e7f128 = (uint32_t)0x800;
    *(uint8_t*)0x20e7f138 = (uint8_t)0x8;
    *(uint8_t*)0x20e7f139 = (uint8_t)0x0;
    *(uint8_t*)0x20e7f13a = (uint8_t)0xffffffff;
    *(uint8_t*)0x20e7f13b = (uint8_t)0x3;
    *(uint8_t*)0x20e7f140 = (uint8_t)0x5;
    *(uint8_t*)0x20e7f141 = (uint8_t)0xfffffffffffffff7;
    *(uint8_t*)0x20e7f142 = (uint8_t)0x0;
    *(uint8_t*)0x20e7f143 = (uint8_t)0x7f;
    r[90] = syscall(__NR_write, r[2], 0x20e7f000ul, 0x150ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[6];

  memset(r, -1, sizeof(r));
  srand(getpid());
  for (i = 0; i < 3; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  for (i = 0; i < 3; i++) {
    pthread_create(&th[3 + i], 0, thr, (void*)i);
    if (rand() % 2)
      usleep(rand() % 10000);
  }
  usleep(rand() % 100000);
}

int main()
{
  loop();
  return 0;
}