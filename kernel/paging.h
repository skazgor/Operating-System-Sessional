
 typedef struct 
{
    pagetable_t p;
    uint64 va;
    uint64 pa;
    uint64 serial;
    uint32 valid;
    uint32 last_used;
    uint16 flag;
} page_t;

#define MAXPHYPAGES 50
