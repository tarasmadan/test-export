// https://syzkaller.appspot.com/bug?id=f8175f1fbf2be8dad746d907eabe71d8e5ba6de7
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/futex.h>

#ifndef __NR_io_uring_enter
#define __NR_io_uring_enter 426
#endif
#ifndef __NR_io_uring_setup
#define __NR_io_uring_setup 425
#endif

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

static bool write_file(const char* file, const char* what, ...)
{
  char buf[1024];
  va_list args;
  va_start(args, what);
  vsnprintf(buf, sizeof(buf), what, args);
  va_end(args);
  buf[sizeof(buf) - 1] = 0;
  int len = strlen(buf);
  int fd = open(file, O_WRONLY | O_CLOEXEC);
  if (fd == -1)
    return false;
  if (write(fd, buf, len) != len) {
    int err = errno;
    close(fd);
    errno = err;
    return false;
  }
  close(fd);
  return true;
}

#define SIZEOF_IO_URING_SQE 64
#define SIZEOF_IO_URING_CQE 16
#define SQ_HEAD_OFFSET 0
#define SQ_TAIL_OFFSET 64
#define SQ_RING_MASK_OFFSET 256
#define SQ_RING_ENTRIES_OFFSET 264
#define SQ_FLAGS_OFFSET 276
#define SQ_DROPPED_OFFSET 272
#define CQ_HEAD_OFFSET 128
#define CQ_TAIL_OFFSET 192
#define CQ_RING_MASK_OFFSET 260
#define CQ_RING_ENTRIES_OFFSET 268
#define CQ_RING_OVERFLOW_OFFSET 284
#define CQ_FLAGS_OFFSET 280
#define CQ_CQES_OFFSET 320

struct io_sqring_offsets {
  uint32_t head;
  uint32_t tail;
  uint32_t ring_mask;
  uint32_t ring_entries;
  uint32_t flags;
  uint32_t dropped;
  uint32_t array;
  uint32_t resv1;
  uint64_t resv2;
};

struct io_cqring_offsets {
  uint32_t head;
  uint32_t tail;
  uint32_t ring_mask;
  uint32_t ring_entries;
  uint32_t overflow;
  uint32_t cqes;
  uint64_t resv[2];
};

struct io_uring_params {
  uint32_t sq_entries;
  uint32_t cq_entries;
  uint32_t flags;
  uint32_t sq_thread_cpu;
  uint32_t sq_thread_idle;
  uint32_t features;
  uint32_t resv[4];
  struct io_sqring_offsets sq_off;
  struct io_cqring_offsets cq_off;
};

#define IORING_OFF_SQ_RING 0
#define IORING_OFF_SQES 0x10000000ULL
#define IORING_SETUP_SQE128 (1U << 10)
#define IORING_SETUP_CQE32 (1U << 11)

static long syz_io_uring_setup(volatile long a0, volatile long a1,
                               volatile long a2, volatile long a3)
{
  uint32_t entries = (uint32_t)a0;
  struct io_uring_params* setup_params = (struct io_uring_params*)a1;
  void** ring_ptr_out = (void**)a2;
  void** sqes_ptr_out = (void**)a3;
  setup_params->flags &= ~(IORING_SETUP_CQE32 | IORING_SETUP_SQE128);
  uint32_t fd_io_uring = syscall(__NR_io_uring_setup, entries, setup_params);
  uint32_t sq_ring_sz =
      setup_params->sq_off.array + setup_params->sq_entries * sizeof(uint32_t);
  uint32_t cq_ring_sz = setup_params->cq_off.cqes +
                        setup_params->cq_entries * SIZEOF_IO_URING_CQE;
  uint32_t ring_sz = sq_ring_sz > cq_ring_sz ? sq_ring_sz : cq_ring_sz;
  *ring_ptr_out =
      mmap(0, ring_sz, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE,
           fd_io_uring, IORING_OFF_SQ_RING);
  uint32_t sqes_sz = setup_params->sq_entries * SIZEOF_IO_URING_SQE;
  *sqes_ptr_out = mmap(0, sqes_sz, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_POPULATE, fd_io_uring, IORING_OFF_SQES);
  uint32_t* array =
      (uint32_t*)((uintptr_t)*ring_ptr_out + setup_params->sq_off.array);
  for (uint32_t index = 0; index < entries; index++)
    array[index] = index;
  return fd_io_uring;
}

static long syz_io_uring_submit(volatile long a0, volatile long a1,
                                volatile long a2)
{
  char* ring_ptr = (char*)a0;
  char* sqes_ptr = (char*)a1;
  char* sqe = (char*)a2;
  uint32_t sq_ring_mask = *(uint32_t*)(ring_ptr + SQ_RING_MASK_OFFSET);
  uint32_t* sq_tail_ptr = (uint32_t*)(ring_ptr + SQ_TAIL_OFFSET);
  uint32_t sq_tail = *sq_tail_ptr & sq_ring_mask;
  char* sqe_dest = sqes_ptr + sq_tail * SIZEOF_IO_URING_SQE;
  memcpy(sqe_dest, sqe, SIZEOF_IO_URING_SQE);
  uint32_t sq_tail_next = *sq_tail_ptr + 1;
  __atomic_store_n(sq_tail_ptr, sq_tail_next, __ATOMIC_RELEASE);
  return 0;
}

static long syz_open_dev(volatile long a0, volatile long a1, volatile long a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  for (int i = 0; i < 100; i++) {
    if (waitpid(-1, status, WNOHANG | __WALL) == pid)
      return;
    usleep(1000);
  }
  DIR* dir = opendir("/sys/fs/fuse/connections");
  if (dir) {
    for (;;) {
      struct dirent* ent = readdir(dir);
      if (!ent)
        break;
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        continue;
      char abort[300];
      snprintf(abort, sizeof(abort), "/sys/fs/fuse/connections/%s/abort",
               ent->d_name);
      int fd = open(abort, O_WRONLY);
      if (fd == -1) {
        continue;
      }
      if (write(fd, abort, 1) < 0) {
      }
      close(fd);
    }
    closedir(dir);
  } else {
  }
  while (waitpid(-1, status, __WALL) != pid) {
  }
}

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  write_file("/proc/self/oom_score_adj", "1000");
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

static void execute_one(void)
{
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  int i, call, thread;
  for (call = 0; call < 11; call++) {
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
      event_timedwait(&th->done, 50);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter = 0;
  for (;; iter++) {
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      execute_one();
      exit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      sleep_ms(10);
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      if (current_time_ms() - start < 5000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[5] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0x0, 0x0};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    syscall(__NR_prctl, /*option=*/0x3eul, /*cmd=*/1ul, /*pid=*/0,
            /*type=PIDTYPE_PGID*/ 2ul, /*uaddr=*/0ul);
    break;
  case 1:
    memcpy((void*)0x20000040, "/dev/snd/midiC#D#\000", 18);
    res = -1;
    res = syz_open_dev(/*dev=*/0x20000040, /*id=*/2,
                       /*flags=O_SYNC|O_NOCTTY|O_NOATIME|O_WRONLY*/ 0x141101);
    if (res != -1)
      r[0] = res;
    break;
  case 2:
    res = syscall(__NR_dup, /*oldfd=*/r[0]);
    if (res != -1)
      r[1] = res;
    break;
  case 3:
    *(uint64_t*)0x20000140 = 8;
    *(uint64_t*)0x20000148 = 0x8b;
    syscall(__NR_prlimit64, /*pid=*/0, /*res=RLIMIT_RTPRIO*/ 0xeul,
            /*new=*/0x20000140ul, /*old=*/0ul);
    break;
  case 4:
    *(uint32_t*)0x20000080 = 7;
    syscall(__NR_sched_setscheduler, /*pid=*/0, /*policy=SCHED_FIFO*/ 1ul,
            /*prio=*/0x20000080ul);
    break;
  case 5:
    *(uint32_t*)0x20000200 = 6;
    syscall(__NR_sched_setscheduler, /*pid=*/0, /*policy=SCHED_RR*/ 2ul,
            /*prio=*/0x20000200ul);
    break;
  case 6:
    memset((void*)0x20000000, 48, 1);
    syscall(__NR_write, /*fd=*/r[1], /*data=*/0x20000000ul,
            /*len=*/0xfffffd2cul);
    break;
  case 7:
    *(uint32_t*)0x200002c4 = 0x73eb;
    *(uint32_t*)0x200002c8 = 0x10100;
    *(uint32_t*)0x200002cc = 0;
    *(uint32_t*)0x200002d0 = 0x2b1;
    *(uint32_t*)0x200002d8 = -1;
    memset((void*)0x200002dc, 0, 12);
    res = -1;
    res = syz_io_uring_setup(/*entries=*/0x2235, /*params=*/0x200002c0,
                             /*ring_ptr=*/0x20000180, /*sqes_ptr=*/0x20000340);
    if (res != -1) {
      r[2] = res;
      r[3] = *(uint64_t*)0x20000180;
      r[4] = *(uint64_t*)0x20000340;
    }
    break;
  case 8:
    *(uint8_t*)0x20000040 = 6;
    *(uint8_t*)0x20000041 = 2;
    *(uint16_t*)0x20000042 = 0;
    *(uint32_t*)0x20000044 = 4;
    *(uint64_t*)0x20000048 = 0;
    *(uint64_t*)0x20000050 = 0;
    *(uint32_t*)0x20000058 = 0;
    *(uint16_t*)0x2000005c = 0x201;
    *(uint16_t*)0x2000005e = 0;
    *(uint64_t*)0x20000060 = 1;
    *(uint16_t*)0x20000068 = 0;
    *(uint16_t*)0x2000006a = 0;
    memset((void*)0x2000006c, 0, 20);
    syz_io_uring_submit(/*ring_ptr=*/r[3], /*sqes_ptr=*/r[4],
                        /*sqe=*/0x20000040);
    break;
  case 9:
    syscall(__NR_io_uring_enter, /*fd=*/r[2], /*to_submit=*/0x2ded,
            /*min_complete=*/0x4000, /*flags=*/0ul, /*sigmask=*/0ul,
            /*size=*/0ul);
    break;
  case 10:
    syscall(__NR_ioctl, /*fd=*/-1, /*cmd=*/0xc0502100, /*arg=*/0ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  const char* reason;
  (void)reason;
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}