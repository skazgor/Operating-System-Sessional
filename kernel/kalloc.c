// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.
uint16 ref_count[REF_COUNT_INDEX(PHYSTOP)+1];
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
  for(int i = 0; i < REF_COUNT_INDEX(PHYSTOP)+1; i++)
    ref_count[i] = 0;
  // printf("%p", end);
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

void
increase_ref_count(uint64 pa)
{
  if((char*)pa < end || (uint64)pa >= PHYSTOP)
    return;

  ref_count[REF_COUNT_INDEX(pa)]++;
  // printf("Increasing ref count to %d for page %p\n ", ref_count[REF_COUNT_INDEX(pa)], pa);
}

void
decrease_ref_count(uint64 pa)
{
  if((char*)pa < end || (uint64)pa >= PHYSTOP)
    return;
  
  // printf("Decreasing ref count to %d for page %p\n ", ref_count[REF_COUNT_INDEX(pa)], pa);
  if(ref_count[REF_COUNT_INDEX(pa)] == 1)
  {
    // printf("Freeing Page\n");
    kfree((void*)PGROUNDDOWN(pa));
  }
  else
    ref_count[REF_COUNT_INDEX(pa)]--;
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
  {
    panic("kfree");
  }
  // Fill with junk to catch dangling refs.
  if(ref_count[REF_COUNT_INDEX(pa)] > 1)
  {
   ref_count[REF_COUNT_INDEX(pa)]--;
    return;
  }
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
  ref_count[REF_COUNT_INDEX(pa)] = 0;
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

  if(r)
  {
    memset((char*)r, 5, PGSIZE); // fill with junk
    if ((char*)r>=end && (uint64)r < PHYSTOP)
    ref_count[REF_COUNT_INDEX(r)] = 1;
  }
  return (void*)r;
}