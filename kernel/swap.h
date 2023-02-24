#define NBLOCKPERPAGE (PGSIZE / BSIZE)
struct swap {
  uint blocknos[NBLOCKPERPAGE];
  uint32 ref_count;
};