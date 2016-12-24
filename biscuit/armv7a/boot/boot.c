#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "console.h"
#include "elf.h"

struct trapframe {
  uint32_t trapno;
  uint32_t lr;
  uint32_t sp;
  uint32_t r12;
  uint32_t fp;
  uint32_t r10;
  uint32_t r9;
  uint32_t r8;
  uint32_t r7;
  uint32_t r6;
  uint32_t r5;
  uint32_t r4;
  uint32_t r3;
  uint32_t r2;
  uint32_t r1;
  uint32_t r0;
};

const uint32_t RAM_START = 0x10000000;
const uint32_t RAM_SIZE = 0x80000000;
const uint32_t ONE_MEG = 0x00100000;

  //stack for go
uint32_t stacksize = 0x4000;

// go elf kernel
extern char gobin_start[], gobin_end[];

// go bin load address
uintptr_t go_load_addr=(uintptr_t)0x10000000;
//uintptr_t go_load_addr=(uintptr_t)0x40000000;


static void backtrace(struct trapframe *tf)
{
  uint32_t *fp = (uint32_t *)(tf->fp);
  unsigned i=0;
  cprintf("backtrace: \r\n");
  while (*fp)
  {
    uint32_t *lr = fp-1;
    cprintf("\t|stack frame at 0x%x, lr=0x%x\r\n", fp, *lr);
    fp = fp-3;
    ++i;
  }
  cprintf("backtrace done\r\n");
}

static void boot_memcpy(char *dst, char *src, size_t size)
{
  for (size_t i=0; i<size; ++i)
  {
    dst[i]=src[i];
  }
}

static void boot_memset(char *dst, char val, size_t size)
{
  for (size_t i=0; i<size; ++i)
  {
    dst[i]=val;
  }
}

void trap(struct trapframe *tf)
{
  switch (tf->trapno)
  {
    //undefined
    case 1:
      cprintf("undefined trap\r\n");
      break;
    //svc
    case 2:
      cprintf("svc from addr 0x%x\r\n", tf->lr-4);
      break;
    //prefetch abort
    case 3:
      cprintf("prefetch abort\r\n");
      break;
    //data abort
    case 4:
      cprintf("data abort from addr 0x%x\r\n", tf->lr-8);
      break;
    //irq
    case 5:
      cprintf("irq trap\r\n");
      break;
    //fiq
    case 6:
      cprintf("fiq trap\r\n");
      break;
    default:
      cprintf("unknown trap %d\r\n", tf->trapno);
  }
  uint32_t sctlr;
  asm volatile("MRC p15, 0, %[a], c1, c0, 0" : [a] "=r" (sctlr) :);
  cprintf("\t sctlr: 0x%x\r\n", sctlr);
  cprintf("\t r0: 0x%x\r\n", tf->r0);
  cprintf("\t r1: 0x%x\r\n", tf->r1);
  cprintf("\t r2: 0x%x\r\n", tf->r2);
  cprintf("\t r3: 0x%x\r\n", tf->r3);
  cprintf("\t r4: 0x%x\r\n", tf->r4);
  cprintf("\t r5: 0x%x\r\n", tf->r5);
  cprintf("\t r6: 0x%x\r\n", tf->r6);
  cprintf("\t r7: 0x%x\r\n", tf->r7);
  cprintf("\t r8: 0x%x\r\n", tf->r8);
  cprintf("\t r9: 0x%x\r\n", tf->r9);
  cprintf("\t r10: 0x%x\r\n", tf->r10);
  cprintf("\t fp: 0x%x\r\n", tf->fp);
  cprintf("\t lr: 0x%x\r\n", tf->lr);
  cprintf("\t sp: 0x%x\r\n", tf->sp);
 // backtrace(tf);
  volatile short stub=1;
  while (stub) {};
}


//static void load_go()
//{
//  size_t binsize = gobin_end - gobin_start;
//  cprintf("go bin size : 0x%x\r\n", binsize);
//  boot_memset((char *)(RAM_START + RAM_SIZE - ONE_MEG), 0, ONE_MEG);
//  boot_memset((char *)go_load_addr, 0, binsize+stacksize+ONE_MEG);
//  boot_memcpy((char *)go_load_addr, (char *)gobin_start, binsize);
//  cprintf("loaded at 0x%x\r\n", go_load_addr);
//  cprintf("first 10 words are:\r\n");
//  uint32_t *in = (uint32_t *)(gobin_start);
//  uint32_t *out = (uint32_t *)(go_load_addr);
//  for (unsigned i=0; i<10; ++i)
//  {
//    cprintf("\t0x%x vs 0x%x\r\n", out[i], in[i]);
//  }
//  uint32_t nwords = binsize /4;
//  cprintf("verifying: ");
//  for (unsigned i=0; i<nwords; ++i)
//  {
//    if (in[i] != out[i])
//    {
//      cprintf("mismatch at address 0x%x\r\n", &out[i]);
//      panic("verify failed");
//    }
//  }
//  cprintf(" complete!\r\n");
//}

static void load_go()
{
  struct Elf *elfhdr=(struct Elf*)gobin_start;
  struct Proghdr *ph, *eph;

  cprintf("elf lives at %x\r\n", gobin_start);

  //check if valid elf
  if (elfhdr->e_magic != ELF_MAGIC)
  {
      panic("bad elf header\r\n");
  }

  ph=(struct Proghdr *) ((uint8_t *) elfhdr + elfhdr->e_phoff);
  eph=ph+elfhdr->e_phnum;
  uint32_t elfentry=elfhdr->e_entry;
  cprintf("elf entry is %x\r\n", elfentry);
  go_load_addr = (uintptr_t)elfentry;
  for (; ph<eph; ph++)
  {
      if (ph->p_type==ELF_PROG_LOAD)
      {
          uint32_t offset=ph->p_offset;
          uint32_t progsize=ph->p_filesz; //the actual program size in bytes
          uint32_t totalsize=ph->p_memsz; //total space required in bytes
          uint32_t dst=ph->p_va;
          uint32_t pa=dst;
          cprintf("p_align is %d\r\n", ph->p_align);
          cprintf("putting va %x at pa %x with size %x\r\n", dst, (uint32_t)pa, totalsize);
          boot_memcpy((void*)pa, ((void*)elfhdr+offset), progsize); //copy data to pa
          boot_memset((void*)(pa+progsize), 0, totalsize-progsize); //zero out the rest
      }
  }
}

int main()
{
  //get a new stack
 // asm("mov sp,%[newsp]" : : [newsp]"r"(&stack[1023]));

  consoleinit();
  cprintf("---------------------------------------------\r\n");
  cprintf("Welcome to Biscuit-ARM Bootloader, 16 is %x hex!\r\n",16);

  float a=1.25;
  float b =a*3;
  cprintf("float multiplication %x\r\n", b);
  //load our kernel
  load_go();
  uint32_t kernel_start = gobin_start;
//  uint32_t kernel_size = gobin_end - gobin_start + stacksize;
//  asm volatile("mov r0, %0"
//      :
//      :"r"(kernel_start)
//      :"r0"
//      );
  asm volatile("mov r5, %0"
      :
      :"r"(kernel_start)
      :"r5"
      );
  asm volatile("mov sp, %0"
      :
      :"r"((RAM_START + RAM_SIZE - ONE_MEG + stacksize) & -16)
      :"sp"
      );
  cprintf("enter go\n");
  ((void (*)(void)) (go_load_addr))();
  panic("should not be here");
}
