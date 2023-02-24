#include "types.h"
#include "riscv.h"
#include "fs.h"
#include "spinlock.h"
#include "defs.h"
#include "param.h"
#include "sleeplock.h"
#include "buf.h"
#include "swap.h"

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} swapmem;

// Initialize swapmem
void
swapinit(void)
{
  initlock(&swapmem.lock, "swapmem");
  swapmem.freelist = 0;
}

// Allocate one swap struct.
// Returns a pointer to the swap struct.
// Returns 0 if the memory cannot be allocated.
struct swap *
swapalloc(void)
{
  struct run *r;
  struct swap *s;

  acquire(&swapmem.lock);
  r = swapmem.freelist;
  if(!r){
    release(&swapmem.lock);
    char *mem = kalloc();
    char *mem_end = mem + PGSIZE;
    for(; mem + sizeof(struct swap) <= mem_end; mem += sizeof(struct swap)){
      r = (struct run*)mem;

      acquire(&swapmem.lock);
      r->next = swapmem.freelist;
      swapmem.freelist = r;
      release(&swapmem.lock);
    }
    acquire(&swapmem.lock);
    r = swapmem.freelist;
  }
  swapmem.freelist = r->next;
  release(&swapmem.lock);
  
  s = (struct swap*)r;
  if(s)
    {
      memset((char*)s->blocknos, 0, sizeof(s->blocknos));
      s->ref_count = 1;  
    } // fill with zeros
  
  return s;
}

// Free the swap struct pointed by s, and the blocks
// contained in s, which normally should have been returned
// by a call to swapalloc() and swapout().
void
swapfree(struct swap *s)
{
  uint *blockno;
  struct run *r;

  if(!s)
    panic("swapfree");
  
  s->ref_count=0;
  begin_op();
  for(blockno = s->blocknos; blockno < &s->blocknos[NBLOCKPERPAGE]; blockno++){
    if(*blockno)
      bfree(ROOTDEV, *blockno);
  }
  end_op();

  r = (struct run*)s;

  acquire(&swapmem.lock);
  r->next = swapmem.freelist;
  swapmem.freelist = r;
  release(&swapmem.lock);
}

// Swap out a given physical page src_pa to disk.
// The metadata for retriving src_pa will be saved
// to dst_pa which normally should have been returned
// by a call to swapalloc().
void
swapout(struct swap *dst_sp, char *src_pa)
{
  uint *blockno;
  struct buf *bp;
  
  begin_op();
  for(blockno = dst_sp->blocknos; blockno < &dst_sp->blocknos[NBLOCKPERPAGE]; blockno++, src_pa += BSIZE){
    *blockno = balloc(ROOTDEV);
    if(*blockno == 0)
      panic("swapout");
    bp = bread(ROOTDEV, *blockno);
    memmove(bp->data, src_pa, BSIZE);
    log_write(bp);
    brelse(bp);
  }
  end_op();
}

// Swap in a page into dst_pa from disk using src_sp.
// src_sp should normally be updated with metadata
// for retriving the page by a call to swapout().
void
swapin(char *dst_pa, struct swap *src_sp)
{
  uint *blockno;
  struct buf *bp;
  
  if(!dst_pa)
    panic("swapin");
  for(blockno = src_sp->blocknos; blockno < &src_sp->blocknos[NBLOCKPERPAGE]; blockno++, dst_pa += BSIZE){
    bp = bread(ROOTDEV, *blockno);
    memmove(dst_pa, bp->data, BSIZE);
    brelse(bp);
  }
}
