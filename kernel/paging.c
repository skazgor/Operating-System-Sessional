#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "elf.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "paging.h"
#include "spinlock.h"
#include "proc.h"

page_t used_pages[MAXPHYPAGES];
uint16 live_page_count=0;
uint64 serial = 0;

void init_paging() {
  for (int i=0; i<MAXPHYPAGES; i++) {
    used_pages[i].valid = 0;
    used_pages[i].pa = 0;
    used_pages[i].flag = 1;
  }
  live_page_count = 0;
  swapinit();
}
void page_swap_fifo();
void 
add_page(pagetable_t pagetable, uint64 va, uint64 pa) {
  if (live_page_count >= MAXPHYPAGES) {
    page_swap_fifo();
  }
  
  int i;
  for (i=0; i<MAXPHYPAGES; i++) {
    if (used_pages[i].valid  == 0) {
        used_pages[i].valid = 1;
        used_pages[i].flag = 1;
        used_pages[i].pa = pa;
        used_pages[i].va = va;
        used_pages[i].serial = serial++;
        used_pages[i].p  = pagetable;
        break;
    }
  }
  live_page_count++;
   printf("Added page.Now %d at index %d va %d\n",live_page_count ,i, va);
}

void remove_page(uint64 pa) {
  for (int i=0; i<live_page_count; i++) {
    if (used_pages[i].valid && used_pages[i].pa == pa) {
      used_pages[i].valid = 0;
      used_pages[i].pa = 0;
      live_page_count--;
      struct proc *p = myproc();
      printf("Removed page %d by %d.Now %d\n",i,p->pid,live_page_count);
      return;
    }
  }
}
void page_swap_fifo(){
    uint64 min= -1;
    int min_index = -1;
    for (int i=0; i<MAXPHYPAGES; i++) {
      if (used_pages[i].valid && used_pages[i].serial < min && used_pages[i].flag  ) {
        pte_t *pte = walk(used_pages[i].p, used_pages[i].va, 0);
        if(pte == 0){
            // printf("pagetable %p index %d\n", used_pages[i].p, i);
            // printf("va %p\n", used_pages[i].va);
          panic("pte is null");
        }
        if (PTE_FLAGS(*pte) & PTE_COW) {
          continue;
        }
        min = used_pages[i].serial;
        min_index = i;
      }
    }
   
    if (min_index == -1) {
      panic("No page to swap out");
    }
    // printf("reached here\n");
    struct proc *p = myproc();
    int f=0;
    if(holding(&p->lock)){
      release(&p->lock);
      f=1;
    }
    used_pages[min_index].flag = 0;
    struct swap *s = swapalloc();
    printf("Swapping out page %p\n", used_pages[min_index].pa);
    swapout(s, (char*)used_pages[min_index].pa);
    if(f){
      acquire(&p->lock);
    }

    used_pages[min_index].valid = 0;
    used_pages[min_index].flag = 1;
    live_page_count--;
    pte_t *pte = walk(used_pages[min_index].p, used_pages[min_index].va, 0);
    uint16 flags = PTE_FLAGS(*pte);
    flags &= ~PTE_V;
    flags |= PTE_PG;
     kfree((void*)PGROUNDDOWN(used_pages[min_index].pa));
    uint64 new_pte = (((uint64)s)<< 10) | flags;
    *pte = new_pte;
    sfence_vma();
}
void remove_pagetable(pagetable_t pagetable) {
  for (int i=0; i<MAXPHYPAGES; i++) {
    if (used_pages[i].valid && used_pages[i].p == pagetable) {
      used_pages[i].valid = 0;
      used_pages[i].pa = 0;
      live_page_count--;
    }
  }
}