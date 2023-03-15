// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
// 物理页到数组中的 ID    
#define PA2PGREF_ID(p) (((p)-KERNBASE)/PGSIZE)
// MAX_PAGE最大页数
#define MAX_PAGE PA2PGREF_ID(PHYSTOP)
// 锁与索引数组
struct spinlock pageReflock;
int pageRef[MAX_PAGE];
// 指向页对应位置的宏
#define PAGEREF(p) (pageRef[PA2PGREF_ID(p)])

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&pageReflock, "pageRef");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // 判断是否这个页面没有被引用了
  acquire(&pageReflock);
  if(--pageRef[PA2PGREF_ID((uint64)pa)] > 0) {
    release(&pageReflock);
    return;
  }
  release(&pageReflock);
  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r) {
    memset((char*)r, 5, PGSIZE); // fill with junk
    pageRef[PA2PGREF_ID((uint64)r)] = 1; // referance ++
  }
  return (void*)r;
}

// 当引用已经小于等于 1 时，不创建和复制到新的物理页，而是直接返回该页本身
void *kCOWcopy(void *pa) {
  acquire(&pageReflock);

  if(PAGEREF((uint64)pa) <= 1) { // 只有 1 个引用，无需复制
    release(&pageReflock);
    return pa;
  }

  // 分配新的内存页，并复制
  uint64 newpage = (uint64)kalloc();
  if(newpage == 0) {
    release(&pageReflock);
    return 0; // out of memory
  }
  memmove((void*)newpage, (void*)pa, PGSIZE);

  // 旧页的引用减 1
  PAGEREF((uint64)pa)--;

  release(&pageReflock);
  return (void*)newpage;
}

void pageRefAdd(uint64 pa){
  acquire(&pageReflock);
  ++PAGEREF(pa);
  release(&pageReflock);
}
