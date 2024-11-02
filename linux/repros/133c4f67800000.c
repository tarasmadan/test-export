// https://syzkaller.appspot.com/bug?id=ce3346fdcc9b5d1f0324a705f14f90f6d33abd29
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/loop.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

extern unsigned long long procid;

struct fs_image_segment {
  void* data;
  uintptr_t size;
  uintptr_t offset;
};

#define IMAGE_MAX_SEGMENTS 4096
#define IMAGE_MAX_SIZE (129 << 20)

#define SYZ_memfd_create 319

static uintptr_t syz_mount_image(uintptr_t fs, uintptr_t dir, uintptr_t size,
                                 uintptr_t nsegs, uintptr_t segments,
                                 uintptr_t flags, uintptr_t opts)
{
  char loopname[64];
  int loopfd, err = 0, res = -1;
  uintptr_t i;
  struct fs_image_segment* segs = (struct fs_image_segment*)segments;

  if (nsegs > IMAGE_MAX_SEGMENTS)
    nsegs = IMAGE_MAX_SEGMENTS;
  for (i = 0; i < nsegs; i++) {
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
  int memfd = syscall(SYZ_memfd_create, "syz_mount_image", 0);
  if (memfd == -1) {
    err = errno;
    goto error;
  }
  if (ftruncate(memfd, size)) {
    err = errno;
    goto error_close_memfd;
  }
  for (i = 0; i < nsegs; i++) {
    if (pwrite(memfd, segs[i].data, segs[i].size, segs[i].offset) < 0) {
    }
  }
  snprintf(loopname, sizeof(loopname), "/dev/loop%llu", procid);
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
  mkdir((char*)dir, 0777);
  if (strcmp((char*)fs, "iso9660") == 0)
    flags |= MS_RDONLY;
  if (mount(loopname, (char*)dir, (char*)fs, flags, (char*)opts)) {
    err = errno;
    goto error_clear_loop;
  }
  res = 0;
error_clear_loop:
  ioctl(loopfd, LOOP_CLR_FD, 0);
error_close_loop:
  close(loopfd);
error_close_memfd:
  close(memfd);
error:
  errno = err;
  return res;
}

unsigned long long procid;
void loop()
{
  memcpy((void*)0x20000000, "f2fs", 5);
  memcpy((void*)0x20000100, "./file0", 8);
  *(uint64_t*)0x20000200 = 0x20010000;
  *(uint64_t*)0x20000208 = 0;
  *(uint64_t*)0x20000210 = 0x400;
  *(uint64_t*)0x20000218 = 0x20010100;
  *(uint64_t*)0x20000220 = 0;
  *(uint64_t*)0x20000228 = 0x860;
  *(uint64_t*)0x20000230 = 0x20010200;
  *(uint64_t*)0x20000238 = 0;
  *(uint64_t*)0x20000240 = 0xa80;
  *(uint64_t*)0x20000248 = 0x20010300;
  *(uint64_t*)0x20000250 = 0;
  *(uint64_t*)0x20000258 = 0xb80;
  *(uint64_t*)0x20000260 = 0x20010400;
  memcpy((void*)0x20010400,
         "\x10\x20\xf5\xf2\x01\x00\x07\x00\x09\x00\x00\x00\x03\x00\x00\x00\x0c"
         "\x00\x00\x00\x09\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x28\x00\x00\x00\x00\x00\x00\x0c\x00\x00\x00\x13\x00\x00"
         "\x00\x02\x00\x00\x00\x02\x00\x00\x00\x02\x00\x00\x00\x01\x00\x00\x00"
         "\x0c\x00\x00\x00\x00\x02\x00\x00\x00\x02\x00\x00\x00\x06\x00\x00\x00"
         "\x0a\x00\x00\x00\x0e\x00\x00\x00\x10\x00\x00\x03\x00\x00\x00\x01\x00"
         "\x00\x00\x02\x00\x00\x00\x04\x73\x6e\xab\x53\x3c\x49\xb2\x89\x56\x4f"
         "\x22\xbf\x9c\xfd\x23\x73\x00\x79\x00\x7a\x00\x30\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00",
         160);
  *(uint64_t*)0x20000268 = 0xa0;
  *(uint64_t*)0x20000270 = 0x1400;
  *(uint64_t*)0x20000278 = 0x20010500;
  *(uint64_t*)0x20000280 = 0;
  *(uint64_t*)0x20000288 = 0x1860;
  *(uint64_t*)0x20000290 = 0x20010600;
  *(uint64_t*)0x20000298 = 0;
  *(uint64_t*)0x200002a0 = 0x1a80;
  *(uint64_t*)0x200002a8 = 0x20010700;
  *(uint64_t*)0x200002b0 = 0;
  *(uint64_t*)0x200002b8 = 0x1b80;
  *(uint64_t*)0x200002c0 = 0x20010800;
  memcpy((void*)0x20010800,
         "\x0d\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x0c"
         "\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x00\x00\x0b\x00\x00\x00\x06\x00"
         "\x00\x00\x0b\x00\x00\x00\x0a\x00\x00\x00\x09\x00\x00\x00\xff\xff\xff"
         "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
         "\x0d\x00\x0b\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08"
         "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff"
         "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x09\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x45\x00\x00\x00"
         "\x06\x00\x00\x00\x01\x00\x00\x00\x0b\x00\x00\x00\x0b\x00\x00\x00\x0e"
         "\x00\x00\x00\x40\x00\x00\x00\x40\x00\x00\x00\xfc\x0f\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00",
         192);
  *(uint64_t*)0x200002c8 = 0xc0;
  *(uint64_t*)0x200002d0 = 0x200000;
  *(uint64_t*)0x200002d8 = 0x20010900;
  memcpy((void*)0x20010900,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x7d\x96\x92\x6f\x0b\x00"
         "\x03\x00\x00\x00\x00\x03\x00\x00\x00\x0c\x26\x00\x00\x04\x00\x00\x00"
         "\x00\x04\x00\x00\x00\x01\x24\x00\x00\x05\x00\x00\x00\x00\x05\x00\x00"
         "\x00\x04\x26\x00\x00\x06\x00\x00\x00\x00\x06\x00\x00\x00\x03\x24\x00"
         "\x00\x07\x00\x00\x00\x00\x07\x00\x00\x00\x04\x24\x00\x00\x08\x00\x00"
         "\x00\x00\x08\x00\x00\x00\x05\x24\x00\x00\x09\x00\x00\x00\x00\x09\x00"
         "\x00\x00\x06\x24\x00\x00\x0a\x00\x00\x00\x00\x0a\x00\x00\x00\x07\x24"
         "\x00\x00\x0b\x00\x00\x00\x00\x0b\x00\x00\x00\x0b\x26\x00\x00\x0c\x00"
         "\x00\x00\x00\x0c\x00\x00\x00\x08\x24\x00\x00\x0d\x00\x00\x00\x00\x0d"
         "\x00\x00\x00\x0a\x24\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00",
         192);
  *(uint64_t*)0x200002e0 = 0xc0;
  *(uint64_t*)0x200002e8 = 0x200fe0;
  *(uint64_t*)0x200002f0 = 0x20010a00;
  *(uint64_t*)0x200002f8 = 0;
  *(uint64_t*)0x20000300 = 0x2011e0;
  *(uint64_t*)0x20000308 = 0x20010b00;
  *(uint64_t*)0x20000310 = 0;
  *(uint64_t*)0x20000318 = 0x201240;
  *(uint64_t*)0x20000320 = 0x20010c00;
  *(uint64_t*)0x20000328 = 0;
  *(uint64_t*)0x20000330 = 0x201280;
  *(uint64_t*)0x20000338 = 0x20010d00;
  *(uint64_t*)0x20000340 = 0;
  *(uint64_t*)0x20000348 = 0x2012e0;
  *(uint64_t*)0x20000350 = 0x20010e00;
  *(uint64_t*)0x20000358 = 0;
  *(uint64_t*)0x20000360 = 0x201320;
  *(uint64_t*)0x20000368 = 0x20010f00;
  *(uint64_t*)0x20000370 = 0;
  *(uint64_t*)0x20000378 = 0x201380;
  *(uint64_t*)0x20000380 = 0x20011000;
  *(uint64_t*)0x20000388 = 0;
  *(uint64_t*)0x20000390 = 0x2013e0;
  *(uint64_t*)0x20000398 = 0x20011100;
  *(uint64_t*)0x200003a0 = 0;
  *(uint64_t*)0x200003a8 = 0x202000;
  *(uint64_t*)0x200003b0 = 0x20011200;
  *(uint64_t*)0x200003b8 = 0;
  *(uint64_t*)0x200003c0 = 0x202e00;
  *(uint64_t*)0x200003c8 = 0x20011300;
  *(uint64_t*)0x200003d0 = 0;
  *(uint64_t*)0x200003d8 = 0x202fe0;
  *(uint64_t*)0x200003e0 = 0x20011400;
  *(uint64_t*)0x200003e8 = 0;
  *(uint64_t*)0x200003f0 = 0x203fe0;
  *(uint64_t*)0x200003f8 = 0x20011500;
  memcpy((void*)0x20011500,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x0d\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x0c\x00\x00"
         "\x00\x00\x00\x00\x00\x0a\x00\x00\x00\x0b\x00\x00\x00\x06\x00\x00\x00"
         "\x0b\x00\x00\x00\x0a\x00\x00\x00\x09\x00\x00\x00\xff\xff\xff\xff\xff"
         "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x0d\x00"
         "\x0b\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00"
         "\x00\x01\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff"
         "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x09\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x45\x00\x00\x00\x06\x00"
         "\x00\x00\x01\x00\x00\x00\x0b\x00\x00\x00\x0b\x00\x00\x00\x0e\x00\x00"
         "\x00\x40\x00\x00\x00\x40\x00\x00\x00\xfc\x0f\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00",
         224);
  *(uint64_t*)0x20000400 = 0xe0;
  *(uint64_t*)0x20000408 = 0x204fe0;
  *(uint64_t*)0x20000410 = 0x20011600;
  memcpy((void*)0x20011600, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x7d\x96\x92\x6f",
         32);
  *(uint64_t*)0x20000418 = 0x20;
  *(uint64_t*)0x20000420 = 0x205fe0;
  *(uint64_t*)0x20000428 = 0x20011700;
  *(uint64_t*)0x20000430 = 0;
  *(uint64_t*)0x20000438 = 0x400000;
  *(uint64_t*)0x20000440 = 0x20011800;
  *(uint64_t*)0x20000448 = 0;
  *(uint64_t*)0x20000450 = 0x400fe0;
  *(uint64_t*)0x20000458 = 0x20011900;
  *(uint64_t*)0x20000460 = 0;
  *(uint64_t*)0x20000468 = 0x4011e0;
  *(uint64_t*)0x20000470 = 0x20011a00;
  *(uint64_t*)0x20000478 = 0;
  *(uint64_t*)0x20000480 = 0x401240;
  *(uint64_t*)0x20000488 = 0x20011b00;
  *(uint64_t*)0x20000490 = 0;
  *(uint64_t*)0x20000498 = 0x401280;
  *(uint64_t*)0x200004a0 = 0x20011c00;
  *(uint64_t*)0x200004a8 = 0;
  *(uint64_t*)0x200004b0 = 0x4012e0;
  *(uint64_t*)0x200004b8 = 0x20011d00;
  *(uint64_t*)0x200004c0 = 0;
  *(uint64_t*)0x200004c8 = 0x401320;
  *(uint64_t*)0x200004d0 = 0x20000040;
  *(uint64_t*)0x200004d8 = 0;
  *(uint64_t*)0x200004e0 = 0x401380;
  *(uint64_t*)0x200004e8 = 0x20011f00;
  *(uint64_t*)0x200004f0 = 0;
  *(uint64_t*)0x200004f8 = 0x4013e0;
  *(uint64_t*)0x20000500 = 0x20012000;
  *(uint64_t*)0x20000508 = 0;
  *(uint64_t*)0x20000510 = 0x402000;
  *(uint64_t*)0x20000518 = 0x20012100;
  *(uint64_t*)0x20000520 = 0;
  *(uint64_t*)0x20000528 = 0x402e00;
  *(uint64_t*)0x20000530 = 0x20012200;
  *(uint64_t*)0x20000538 = 0;
  *(uint64_t*)0x20000540 = 0x402fe0;
  *(uint64_t*)0x20000548 = 0x20012300;
  *(uint64_t*)0x20000550 = 0;
  *(uint64_t*)0x20000558 = 0x403fe0;
  *(uint64_t*)0x20000560 = 0x20012400;
  *(uint64_t*)0x20000568 = 0;
  *(uint64_t*)0x20000570 = 0x404fe0;
  *(uint64_t*)0x20000578 = 0x20012500;
  *(uint64_t*)0x20000580 = 0;
  *(uint64_t*)0x20000588 = 0x405fe0;
  *(uint64_t*)0x20000590 = 0x20012600;
  *(uint64_t*)0x20000598 = 0;
  *(uint64_t*)0x200005a0 = 0xa00000;
  *(uint64_t*)0x200005a8 = 0x20012700;
  *(uint64_t*)0x200005b0 = 0;
  *(uint64_t*)0x200005b8 = 0x2000000;
  *(uint64_t*)0x200005c0 = 0x20012800;
  *(uint64_t*)0x200005c8 = 0;
  *(uint64_t*)0x200005d0 = 0x2000940;
  *(uint64_t*)0x200005d8 = 0x20012900;
  *(uint64_t*)0x200005e0 = 0;
  *(uint64_t*)0x200005e8 = 0x2001000;
  *(uint64_t*)0x200005f0 = 0x20012a00;
  *(uint64_t*)0x200005f8 = 0;
  *(uint64_t*)0x20000600 = 0x2001940;
  *(uint64_t*)0x20000608 = 0x20012b00;
  *(uint64_t*)0x20000610 = 0;
  *(uint64_t*)0x20000618 = 0x2002000;
  *(uint64_t*)0x20000620 = 0x20012c00;
  *(uint64_t*)0x20000628 = 0;
  *(uint64_t*)0x20000630 = 0x2002940;
  *(uint64_t*)0x20000638 = 0x20012d00;
  *(uint64_t*)0x20000640 = 0;
  *(uint64_t*)0x20000648 = 0x2003000;
  *(uint64_t*)0x20000650 = 0x20012e00;
  *(uint64_t*)0x20000658 = 0;
  *(uint64_t*)0x20000660 = 0x2003940;
  *(uint64_t*)0x20000668 = 0x20012f00;
  *(uint64_t*)0x20000670 = 0;
  *(uint64_t*)0x20000678 = 0x2004000;
  *(uint64_t*)0x20000680 = 0x20013000;
  *(uint64_t*)0x20000688 = 0;
  *(uint64_t*)0x20000690 = 0x2004940;
  *(uint64_t*)0x20000698 = 0x20013100;
  *(uint64_t*)0x200006a0 = 0;
  *(uint64_t*)0x200006a8 = 0x2005000;
  *(uint64_t*)0x200006b0 = 0x20013200;
  *(uint64_t*)0x200006b8 = 0;
  *(uint64_t*)0x200006c0 = 0x2005940;
  *(uint64_t*)0x200006c8 = 0x20013300;
  *(uint64_t*)0x200006d0 = 0;
  *(uint64_t*)0x200006d8 = 0x2006000;
  *(uint64_t*)0x200006e0 = 0x20013400;
  *(uint64_t*)0x200006e8 = 0;
  *(uint64_t*)0x200006f0 = 0x2006940;
  *(uint64_t*)0x200006f8 = 0x20013500;
  *(uint64_t*)0x20000700 = 0;
  *(uint64_t*)0x20000708 = 0x2007000;
  *(uint64_t*)0x20000710 = 0x20013600;
  *(uint64_t*)0x20000718 = 0;
  *(uint64_t*)0x20000720 = 0x2007940;
  *(uint64_t*)0x20000728 = 0x20013700;
  *(uint64_t*)0x20000730 = 0;
  *(uint64_t*)0x20000738 = 0x2008000;
  *(uint64_t*)0x20000740 = 0x20013800;
  *(uint64_t*)0x20000748 = 0;
  *(uint64_t*)0x20000750 = 0x2008940;
  *(uint64_t*)0x20000758 = 0x20013900;
  *(uint64_t*)0x20000760 = 0;
  *(uint64_t*)0x20000768 = 0x2400000;
  *(uint64_t*)0x20000770 = 0x20013a00;
  *(uint64_t*)0x20000778 = 0;
  *(uint64_t*)0x20000780 = 0x2400160;
  *(uint64_t*)0x20000788 = 0x20013b00;
  *(uint64_t*)0x20000790 = 0;
  *(uint64_t*)0x20000798 = 0x2400fe0;
  *(uint64_t*)0x200007a0 = 0x20013c00;
  *(uint64_t*)0x200007a8 = 0;
  *(uint64_t*)0x200007b0 = 0x2401160;
  *(uint64_t*)0x200007b8 = 0x20013d00;
  *(uint64_t*)0x200007c0 = 0;
  *(uint64_t*)0x200007c8 = 0x2401fe0;
  *(uint64_t*)0x200007d0 = 0x20013e00;
  *(uint64_t*)0x200007d8 = 0;
  *(uint64_t*)0x200007e0 = 0x2402160;
  *(uint64_t*)0x200007e8 = 0x20013f00;
  *(uint64_t*)0x200007f0 = 0;
  *(uint64_t*)0x200007f8 = 0x2402fe0;
  *(uint64_t*)0x20000800 = 0x20014000;
  *(uint64_t*)0x20000808 = 0;
  *(uint64_t*)0x20000810 = 0x2403160;
  *(uint64_t*)0x20000818 = 0x20014100;
  *(uint64_t*)0x20000820 = 0;
  *(uint64_t*)0x20000828 = 0x2403fe0;
  *(uint64_t*)0x20000830 = 0x20014200;
  *(uint64_t*)0x20000838 = 0;
  *(uint64_t*)0x20000840 = 0x2404fe0;
  *(uint64_t*)0x20000848 = 0x20014300;
  *(uint64_t*)0x20000850 = 0;
  *(uint64_t*)0x20000858 = 0x2405fe0;
  *(uint64_t*)0x20000860 = 0x20014400;
  *(uint64_t*)0x20000868 = 0;
  *(uint64_t*)0x20000870 = 0x2406fe0;
  *(uint64_t*)0x20000878 = 0x20014500;
  *(uint64_t*)0x20000880 = 0;
  *(uint64_t*)0x20000888 = 0x2407fe0;
  *(uint64_t*)0x20000890 = 0x20014600;
  *(uint64_t*)0x20000898 = 0;
  *(uint64_t*)0x200008a0 = 0x2408fe0;
  *(uint64_t*)0x200008a8 = 0x20014700;
  *(uint64_t*)0x200008b0 = 0;
  *(uint64_t*)0x200008b8 = 0x2409160;
  *(uint64_t*)0x200008c0 = 0x20014800;
  *(uint64_t*)0x200008c8 = 0;
  *(uint64_t*)0x200008d0 = 0x2409fe0;
  *(uint64_t*)0x200008d8 = 0x20014900;
  *(uint64_t*)0x200008e0 = 0;
  *(uint64_t*)0x200008e8 = 0x240a160;
  *(uint64_t*)0x200008f0 = 0x20014a00;
  *(uint64_t*)0x200008f8 = 0;
  *(uint64_t*)0x20000900 = 0x240afe0;
  *(uint64_t*)0x20000908 = 0x20014b00;
  *(uint64_t*)0x20000910 = 0;
  *(uint64_t*)0x20000918 = 0x2600000;
  *(uint64_t*)0x20000920 = 0x20014c00;
  *(uint64_t*)0x20000928 = 0;
  *(uint64_t*)0x20000930 = 0x2600160;
  *(uint64_t*)0x20000938 = 0x20014d00;
  *(uint64_t*)0x20000940 = 0;
  *(uint64_t*)0x20000948 = 0x2600fe0;
  *(uint64_t*)0x20000950 = 0x20014e00;
  *(uint64_t*)0x20000958 = 0;
  *(uint64_t*)0x20000960 = 0x2601160;
  *(uint64_t*)0x20000968 = 0x20014f00;
  *(uint64_t*)0x20000970 = 0;
  *(uint64_t*)0x20000978 = 0x2601fe0;
  *(uint64_t*)0x20000980 = 0x20015000;
  *(uint64_t*)0x20000988 = 0;
  *(uint64_t*)0x20000990 = 0x2602160;
  *(uint64_t*)0x20000998 = 0x20015100;
  *(uint64_t*)0x200009a0 = 0;
  *(uint64_t*)0x200009a8 = 0x2602fe0;
  *(uint64_t*)0x200009b0 = 0x20015200;
  *(uint64_t*)0x200009b8 = 0;
  *(uint64_t*)0x200009c0 = 0x2603160;
  *(uint64_t*)0x200009c8 = 0x20015300;
  *(uint64_t*)0x200009d0 = 0;
  *(uint64_t*)0x200009d8 = 0x2603940;
  *(uint64_t*)0x200009e0 = 0x20015400;
  *(uint64_t*)0x200009e8 = 0;
  *(uint64_t*)0x200009f0 = 0x2603fe0;
  *(uint64_t*)0x200009f8 = 0x20015500;
  *(uint64_t*)0x20000a00 = 0;
  *(uint64_t*)0x20000a08 = 0x2604160;
  *(uint64_t*)0x20000a10 = 0x20015600;
  *(uint64_t*)0x20000a18 = 0;
  *(uint64_t*)0x20000a20 = 0x2604940;
  *(uint64_t*)0x20000a28 = 0x20015700;
  *(uint64_t*)0x20000a30 = 0;
  *(uint64_t*)0x20000a38 = 0x2604fe0;
  *(uint64_t*)0x20000a40 = 0x20015800;
  *(uint64_t*)0x20000a48 = 0;
  *(uint64_t*)0x20000a50 = 0x2605160;
  *(uint64_t*)0x20000a58 = 0x20015900;
  *(uint64_t*)0x20000a60 = 0;
  *(uint64_t*)0x20000a68 = 0x2605fe0;
  *(uint64_t*)0x20000a70 = 0x20015a00;
  *(uint64_t*)0x20000a78 = 0;
  *(uint64_t*)0x20000a80 = 0x2606160;
  *(uint64_t*)0x20000a88 = 0x20015b00;
  *(uint64_t*)0x20000a90 = 0;
  *(uint64_t*)0x20000a98 = 0x2606fe0;
  *(uint64_t*)0x20000aa0 = 0x20015c00;
  *(uint64_t*)0x20000aa8 = 0;
  *(uint64_t*)0x20000ab0 = 0x2607160;
  *(uint64_t*)0x20000ab8 = 0x20015d00;
  *(uint64_t*)0x20000ac0 = 0;
  *(uint64_t*)0x20000ac8 = 0x2607fe0;
  *(uint64_t*)0x20000ad0 = 0x20015e00;
  *(uint64_t*)0x20000ad8 = 0;
  *(uint64_t*)0x20000ae0 = 0x2608160;
  *(uint64_t*)0x20000ae8 = 0x20015f00;
  *(uint64_t*)0x20000af0 = 0;
  *(uint64_t*)0x20000af8 = 0x2608fe0;
  *(uint64_t*)0x20000b00 = 0x20016000;
  *(uint64_t*)0x20000b08 = 0;
  *(uint64_t*)0x20000b10 = 0x2609160;
  *(uint64_t*)0x20000b18 = 0x20016100;
  *(uint64_t*)0x20000b20 = 0;
  *(uint64_t*)0x20000b28 = 0x2609fe0;
  *(uint64_t*)0x20000b30 = 0x20016200;
  *(uint64_t*)0x20000b38 = 0;
  *(uint64_t*)0x20000b40 = 0x260a160;
  *(uint64_t*)0x20000b48 = 0x20016300;
  *(uint64_t*)0x20000b50 = 0;
  *(uint64_t*)0x20000b58 = 0x260a940;
  *(uint64_t*)0x20000b60 = 0x20016400;
  *(uint64_t*)0x20000b68 = 0;
  *(uint64_t*)0x20000b70 = 0x260afe0;
  *(uint64_t*)0x20000b78 = 0x20016500;
  *(uint64_t*)0x20000b80 = 0;
  *(uint64_t*)0x20000b88 = 0x260b160;
  *(uint64_t*)0x20000b90 = 0x20016600;
  *(uint64_t*)0x20000b98 = 0;
  *(uint64_t*)0x20000ba0 = 0x260b940;
  *(uint64_t*)0x20000ba8 = 0x20016700;
  memcpy((void*)0x20016700,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x0b\x00\x00\x00\x0b\x00\x00\x00\x00"
         "\x00\x00\x00\x09\x00\x00\x00\x29\xdb\xda\x7f\x0c\x26\x00\x00\xed\x41"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x10\x00"
         "\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\xa6\xa0\xd8\x5a"
         "\x00\x00\x00\x00\xa6\xa0\xd8\x5a\x00\x00\x00\x00\xa6\xa0\xd8\x5a\x00"
         "\x00\x00\x00\x63\x90\x6e\x14\x96\x83\xde\x1a\x96\x83\xde\x1a\x00\x00"
         "\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         128);
  *(uint64_t*)0x20000bb0 = 0x80;
  *(uint64_t*)0x20000bb8 = 0x260bfe4;
  *(uint64_t*)0x20000bc0 = 0x20016800;
  *(uint64_t*)0x20000bc8 = 0;
  *(uint64_t*)0x20000bd0 = 0x260c160;
  *(uint64_t*)0x20000bd8 = 0x20016900;
  memcpy((void*)0x20016900, "\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00"
                            "\x03\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x00\x00"
                            "\x5a\xab\xd9\x50\x0d\x26\x00\x00",
         32);
  *(uint64_t*)0x20000be0 = 0x20;
  *(uint64_t*)0x20000be8 = 0x260cfe0;
  *(uint8_t*)0x20016a00 = 0;
  syz_mount_image(0x20000000, 0x20000100, 0x2800000, 0x6a, 0x20000200, 0,
                  0x20016a00);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}