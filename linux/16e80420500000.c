// https://syzkaller.appspot.com/bug?id=cbd1f57996b5a8df8762a19494b97e3daaf58007
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <linux/futex.h>
#include <linux/loop.h>

static unsigned long long procid;

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

static uint64_t current_time_ms(void)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  int i = 0;
  for (; i < 100; i++) {
    if (pthread_create(&th, &attr, fn, arg) == 0) {
      pthread_attr_destroy(&attr);
      return;
    }
    if (errno == EAGAIN) {
      usleep(50);
      continue;
    }
    break;
  }
  exit(1);
}

typedef struct {
  int state;
} event_t;

static void event_init(event_t* ev)
{
  ev->state = 0;
}

static void event_reset(event_t* ev)
{
  ev->state = 0;
}

static void event_set(event_t* ev)
{
  if (ev->state)
    exit(1);
  __atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1000000);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, 0);
}

static int event_isset(event_t* ev)
{
  return __atomic_load_n(&ev->state, __ATOMIC_ACQUIRE);
}

static int event_timedwait(event_t* ev, uint64_t timeout)
{
  uint64_t start = current_time_ms();
  uint64_t now = start;
  for (;;) {
    uint64_t remain = timeout - (now - start);
    struct timespec ts;
    ts.tv_sec = remain / 1000;
    ts.tv_nsec = (remain % 1000) * 1000 * 1000;
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, &ts);
    if (__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
}

struct fs_image_segment {
  void* data;
  uintptr_t size;
  uintptr_t offset;
};

#define IMAGE_MAX_SEGMENTS 4096
#define IMAGE_MAX_SIZE (129 << 20)

#define sys_memfd_create 319

static unsigned long fs_image_segment_check(unsigned long size,
                                            unsigned long nsegs,
                                            struct fs_image_segment* segs)
{
  if (nsegs > IMAGE_MAX_SEGMENTS)
    nsegs = IMAGE_MAX_SEGMENTS;
  for (size_t i = 0; i < nsegs; i++) {
    if (segs[i].size > IMAGE_MAX_SIZE)
      segs[i].size = IMAGE_MAX_SIZE;
    segs[i].offset %= IMAGE_MAX_SIZE;
    if (segs[i].offset > IMAGE_MAX_SIZE - segs[i].size)
      segs[i].offset = IMAGE_MAX_SIZE - segs[i].size;
    if (size < segs[i].offset + segs[i].offset)
      size = segs[i].offset + segs[i].offset;
  }
  if (size > IMAGE_MAX_SIZE)
    size = IMAGE_MAX_SIZE;
  return size;
}
static int setup_loop_device(long unsigned size, long unsigned nsegs,
                             struct fs_image_segment* segs,
                             const char* loopname, int* memfd_p, int* loopfd_p)
{
  int err = 0, loopfd = -1;
  size = fs_image_segment_check(size, nsegs, segs);
  int memfd = syscall(sys_memfd_create, "syzkaller", 0);
  if (memfd == -1) {
    err = errno;
    goto error;
  }
  if (ftruncate(memfd, size)) {
    err = errno;
    goto error_close_memfd;
  }
  for (size_t i = 0; i < nsegs; i++) {
    if (pwrite(memfd, segs[i].data, segs[i].size, segs[i].offset) < 0) {
    }
  }
  loopfd = open(loopname, O_RDWR);
  if (loopfd == -1) {
    err = errno;
    goto error_close_memfd;
  }
  if (ioctl(loopfd, LOOP_SET_FD, memfd)) {
    if (errno != EBUSY) {
      err = errno;
      goto error_close_loop;
    }
    ioctl(loopfd, LOOP_CLR_FD, 0);
    usleep(1000);
    if (ioctl(loopfd, LOOP_SET_FD, memfd)) {
      err = errno;
      goto error_close_loop;
    }
  }
  *memfd_p = memfd;
  *loopfd_p = loopfd;
  return 0;

error_close_loop:
  close(loopfd);
error_close_memfd:
  close(memfd);
error:
  errno = err;
  return -1;
}

static long syz_mount_image(volatile long fsarg, volatile long dir,
                            volatile unsigned long size,
                            volatile unsigned long nsegs,
                            volatile long segments, volatile long flags,
                            volatile long optsarg)
{
  struct fs_image_segment* segs = (struct fs_image_segment*)segments;
  int res = -1, err = 0, loopfd = -1, memfd = -1, need_loop_device = !!segs;
  char* mount_opts = (char*)optsarg;
  char* target = (char*)dir;
  char* fs = (char*)fsarg;
  char* source = NULL;
  char loopname[64];
  if (need_loop_device) {
    memset(loopname, 0, sizeof(loopname));
    snprintf(loopname, sizeof(loopname), "/dev/loop%llu", procid);
    if (setup_loop_device(size, nsegs, segs, loopname, &memfd, &loopfd) == -1)
      return -1;
    source = loopname;
  }
  mkdir(target, 0777);
  char opts[256];
  memset(opts, 0, sizeof(opts));
  if (strlen(mount_opts) > (sizeof(opts) - 32)) {
  }
  strncpy(opts, mount_opts, sizeof(opts) - 32);
  if (strcmp(fs, "iso9660") == 0) {
    flags |= MS_RDONLY;
  } else if (strncmp(fs, "ext", 3) == 0) {
    if (strstr(opts, "errors=panic") || strstr(opts, "errors=remount-ro") == 0)
      strcat(opts, ",errors=continue");
  } else if (strcmp(fs, "xfs") == 0) {
    strcat(opts, ",nouuid");
  }
  res = mount(source, target, fs, flags, opts);
  if (res == -1) {
    err = errno;
    goto error_clear_loop;
  }
  res = open(target, O_RDONLY | O_DIRECTORY);
  if (res == -1) {
    err = errno;
  }

error_clear_loop:
  if (need_loop_device) {
    ioctl(loopfd, LOOP_CLR_FD, 0);
    close(loopfd);
    close(memfd);
  }
  errno = err;
  return res;
}

struct thread_t {
  int created, call;
  event_t ready, done;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    event_wait(&th->ready);
    event_reset(&th->ready);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    event_set(&th->done);
  }
  return 0;
}

static void loop(void)
{
  int i, call, thread;
  int collide = 0;
again:
  for (call = 0; call < 2; call++) {
    for (thread = 0; thread < (int)(sizeof(threads) / sizeof(threads[0]));
         thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        event_init(&th->ready);
        event_init(&th->done);
        event_set(&th->done);
        thread_start(thr, th);
      }
      if (!event_isset(&th->done))
        continue;
      event_reset(&th->done);
      th->call = call;
      __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
      event_set(&th->ready);
      if (collide && (call % 2) == 0)
        break;
      event_timedwait(&th->done,
                      45 + (call == 0 ? 50 : 0) + (call == 1 ? 50 : 0));
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
  if (!collide) {
    collide = 1;
    goto again;
  }
}

void execute_call(int call)
{
  switch (call) {
  case 0:
    memcpy((void*)0x20000000, "f2fs\000", 5);
    memcpy((void*)0x20000100, "./file0\000", 8);
    *(uint64_t*)0x20000200 = 0x20010000;
    memcpy((void*)0x20010000,
           "\x10\x20\xf5\xf2\x01\x00\x0b\x00\x09\x00\x00\x00\x03\x00\x00\x00"
           "\x0c\x00\x00\x00\x09\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00\x00\x00\x18\x00\x00\x00"
           "\x1f\x00\x00\x00\x02\x00\x00\x00\x02\x00\x00\x00\x02\x00\x00\x00"
           "\x01\x00\x00\x00\x18\x00\x00\x00\x00\x02\x00\x00\x00\x02\x00\x00"
           "\x00\x06\x00\x00\x00\x0a\x00\x00\x00\x0e\x00\x00\x00\x10\x00\x00"
           "\x03\x00\x00\x00\x01\x00\x00\x00\x02",
           105);
    *(uint64_t*)0x20000208 = 0x69;
    *(uint64_t*)0x20000210 = 0x400;
    *(uint64_t*)0x20000218 = 0x20010a00;
    memcpy((void*)0x20010a00,
           "\x43\x79\xd5\x27\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00"
           "\x02\x00\x00\x00\x00\x00\x00\x00\x0d\x00\x00\x00\x10\x00\x00\x00"
           "\x12\x00\x00\x00\x17\x00\x00\x00\x16\x00\x00\x00\x15\x00\x00\x00"
           "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
           "\xff\xff\xff\xff\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x14\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"
           "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
           "\xff\xff\xff\xff\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x85\x01\x00\x00\x06\x00\x00\x00\x01\x00\x00\x00"
           "\x01\x00\x00\x00\x01\x00\x00\x00\x04\x00\x00\x00\x40\x00\x00\x00"
           "\x40\x00\x00\x00\xfc\x0f",
           166);
    *(uint64_t*)0x20000220 = 0xa6;
    *(uint64_t*)0x20000228 = 0x200000;
    *(uint64_t*)0x20000230 = 0x20010b00;
    memcpy((void*)0x20010b00, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                              "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                              "\x00\x00\x00\x00\x29\x64\x03\x9d\x01\x00\x03\x00"
                              "\x00\x00\x00\x03\x00\x00\x00\x00\x3e",
           45);
    *(uint64_t*)0x20000238 = 0x2d;
    *(uint64_t*)0x20000240 = 0x200fe0;
    *(uint64_t*)0x20000248 = 0x20010c00;
    memcpy((void*)0x20010c00, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                              "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                              "\x00\x00\x00\x06\x00\x17\x00\x00\x00\x01\x0c"
                              "\x80",
           36);
    *(uint64_t*)0x20000250 = 0x24;
    *(uint64_t*)0x20000258 = 0x2011e0;
    *(uint64_t*)0x20000260 = 0x20011600;
    memcpy((void*)0x20011600,
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00"
           "\x43\x79\xd5\x27\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00"
           "\x02\x00\x00\x00\x00\x00\x00\x00\x0d\x00\x00\x00\x10\x00\x00\x00"
           "\x12\x00\x00\x00\x17\x00\x00\x00\x16\x00\x00\x00\x15\x00\x00\x00"
           "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
           "\xff\xff\xff\xff\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x14\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"
           "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
           "\xff\xff\xff\xff\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x85\x01\x00\x00\x06\x00\x00\x00\x01\x00\x00\x00"
           "\x01\x00\x00\x00\x01\x00\x00\x00\x04\x00\x00\x00\x40\x00\x00\x00"
           "\x40\x00\x00\x00\xfc\x0f",
           198);
    *(uint64_t*)0x20000268 = 0xc6;
    *(uint64_t*)0x20000270 = 0x204fe0;
    *(uint64_t*)0x20000278 = 0x20011700;
    memcpy((void*)0x20011700, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                              "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                              "\x00\x00\x00\x00\x29\x64\x03\x9d",
           32);
    *(uint64_t*)0x20000280 = 0x20;
    *(uint64_t*)0x20000288 = 0x205fe0;
    *(uint64_t*)0x20000290 = 0x20000140;
    memcpy((void*)0x20000140,
           "\xed\x41\x22\x01\x19\x59\xec\x00\xa7\x32\xf0\x4e\x00\x53\x5f\x01"
           "\x00\x02\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x02\xf9\x77"
           "\x08\x63\x26\x1a\xf9\x23\xd7\x0a\xe2\x34\x35\x3b\x37\x71\x5c\x50"
           "\xd8\x28\x60\x18\x0e\x60\xf1\x1d\x85\xa1\xf4\x4e\x7d\x7c\x79\xa4"
           "\x45\xd3\xe8\xf7\x56\x13\x7d\xcd\x39\x3f\xb6\x26\xe3\x55\xce\xc1"
           "\xca\x74\xc6\x48\xf5\x95\x17\x1f\xe8\x58\xf8\x17\xcd\x5d\x8d\x06"
           "\xf3",
           97);
    *(uint64_t*)0x20000298 = 0x61;
    *(uint64_t*)0x200002a0 = 0x3e00000;
    *(uint64_t*)0x200002a8 = 0x20012300;
    memcpy((void*)0x20012300,
           "\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x03", 13);
    *(uint64_t*)0x200002b0 = 0xd;
    *(uint64_t*)0x200002b8 = 0x3e00fe0;
    syz_mount_image(0x20000000, 0x20000100, 0, 8, 0x20000200, 0x800000,
                    0x200015c0);
    break;
  case 1:
    memcpy((void*)0x20001940, "./file0/file0\000", 14);
    syz_mount_image(0, 0x20001940, 0, 0, 0, 2, 0);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
