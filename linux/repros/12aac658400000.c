// https://syzkaller.appspot.com/bug?id=293f48c6a63935b5872fac5eafff89a15518864e
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <linux/futex.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
  }

static void execute_one();
extern unsigned long long procid;

void loop()
{
  while (1) {
    execute_one();
  }
}

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;
static int collide;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    while (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE))
      syscall(SYS_futex, &th->running, FUTEX_WAIT, 0, 0);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    __atomic_store_n(&th->running, 0, __ATOMIC_RELEASE);
    syscall(SYS_futex, &th->running, FUTEX_WAKE);
  }
  return 0;
}

static void execute(int num_calls)
{
  int call, thread;
  running = 0;
  for (call = 0; call < num_calls; call++) {
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]); thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 128 << 10);
        pthread_create(&th->th, &attr, thr, th);
      }
      if (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE)) {
        th->call = call;
        __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
        __atomic_store_n(&th->running, 1, __ATOMIC_RELEASE);
        syscall(SYS_futex, &th->running, FUTEX_WAKE);
        if (collide && call % 2)
          break;
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (__atomic_load_n(&running, __ATOMIC_RELAXED))
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};
unsigned long long procid;
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    res = syscall(__NR_socketpair, 0, 0, 0, 0x20000140);
    if (res != -1)
      r[0] = *(uint32_t*)0x20000144;
    break;
  case 1:
    syscall(__NR_socket, 0xa, 1, 0);
    break;
  case 2:
    *(uint32_t*)0x20000080 = 7;
    *(uint32_t*)0x20000084 = 0x70;
    *(uint8_t*)0x20000088 = -1;
    *(uint8_t*)0x20000089 = 7;
    *(uint8_t*)0x2000008a = 0x5a;
    *(uint8_t*)0x2000008b = -1;
    *(uint32_t*)0x2000008c = 0;
    *(uint64_t*)0x20000090 = 5;
    *(uint64_t*)0x20000098 = 0x400;
    *(uint64_t*)0x200000a0 = 1;
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 3, 0, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0x100, 1, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0xdee, 2, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 7, 3, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0x101, 4, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 8, 5, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0xbd13, 6, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0x81, 7, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0xf6b, 8, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 7, 9, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0xfff, 10, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0x3277320, 11, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0x100000001, 12, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 3, 13, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 4, 14, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 15, 2);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 5, 17, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0x7fffffff, 18, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0xff, 19, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0x3ff, 20, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 5, 21, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0x80000001, 22, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0x34ba39e8, 23, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0x11cb53a0, 24, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 7, 25, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0xffffffffffff010d, 26, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 27, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0xfffffffffffffffa, 28, 1);
    STORE_BY_BITMASK(uint64_t, 0x200000a8, 0, 29, 35);
    *(uint32_t*)0x200000b0 = 0x8000;
    *(uint32_t*)0x200000b4 = 0;
    *(uint64_t*)0x200000b8 = 0x20000040;
    *(uint64_t*)0x200000c0 = 5;
    *(uint64_t*)0x200000c8 = 0x10;
    *(uint64_t*)0x200000d0 = 5;
    *(uint32_t*)0x200000d8 = 6;
    *(uint32_t*)0x200000dc = 0;
    *(uint64_t*)0x200000e0 = 4;
    *(uint32_t*)0x200000e8 = 4;
    *(uint16_t*)0x200000ec = 0;
    *(uint16_t*)0x200000ee = 0;
    res = syscall(__NR_perf_event_open, 0x20000080, 0, 0xa, r[0], 1);
    if (res != -1)
      r[1] = res;
    break;
  case 3:
    syscall(__NR_ioctl, r[1], 0x2401, 2);
    break;
  case 4:
    *(uint32_t*)0x20000280 = 0x12;
    *(uint32_t*)0x20000284 = 0;
    *(uint32_t*)0x20000288 = 4;
    *(uint32_t*)0x2000028c = 7;
    *(uint32_t*)0x20000290 = 0;
    *(uint32_t*)0x20000294 = 1;
    *(uint32_t*)0x20000298 = 0;
    *(uint8_t*)0x2000029c = 0;
    *(uint8_t*)0x2000029d = 0;
    *(uint8_t*)0x2000029e = 0;
    *(uint8_t*)0x2000029f = 0;
    *(uint8_t*)0x200002a0 = 0;
    *(uint8_t*)0x200002a1 = 0;
    *(uint8_t*)0x200002a2 = 0;
    *(uint8_t*)0x200002a3 = 0;
    *(uint8_t*)0x200002a4 = 0;
    *(uint8_t*)0x200002a5 = 0;
    *(uint8_t*)0x200002a6 = 0;
    *(uint8_t*)0x200002a7 = 0;
    *(uint8_t*)0x200002a8 = 0;
    *(uint8_t*)0x200002a9 = 0;
    *(uint8_t*)0x200002aa = 0;
    *(uint8_t*)0x200002ab = 0;
    res = syscall(__NR_bpf, 0, 0x20000280, 0x2c);
    if (res != -1)
      r[2] = res;
    break;
  case 5:
    *(uint32_t*)0x20000180 = r[2];
    *(uint64_t*)0x20000188 = 0x20000000;
    *(uint64_t*)0x20000190 = 0x20000140;
    *(uint64_t*)0x20000198 = 0;
    syscall(__NR_bpf, 2, 0x20000180, 0x20);
    break;
  case 6:
    *(uint32_t*)0x20000000 = r[2];
    *(uint64_t*)0x20000008 = 0x20000080;
    *(uint64_t*)0x20000010 = 0x20000140;
    *(uint64_t*)0x20000018 = 1;
    syscall(__NR_bpf, 2, 0x20000000, 0x20);
    break;
  }
}

void execute_one()
{
  execute(7);
  collide = 1;
  execute(7);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        loop();
      }
    }
  }
  sleep(1000000);
  return 0;
}