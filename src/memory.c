#include "common.h"

#include "string.h"
#include "system.h"

#include "memory.h"

#define USED        1

// end of bss segment --> start of heap
extern uint32_t _bend;

 // forward
static uint16_t* pack(uint16_t nsize);

static uint16_t* free;
static uint16_t* heap;

void MEM_init()
{
    uint32_t h;
    uint32_t len;

    // point to end of bss (start of heap)
    h = (uint32_t)&_bend;
    // 2 bytes aligned
    h += 1;
    h >>= 1;
    h <<= 1;

    // define available memory (sizeof(uint16_t) is the memory reserved to indicate heap end)
    len = MEMORY_HIGH - (h + sizeof(uint16_t));

    // define heap
    heap = (uint16_t*) h;
    // and its size
    *heap = len;

    // free memory : whole heap
    free = heap;

    // mark end of heap memory
    heap[len >> 1] = 0;
}

void MEM_free(void *ptr)
{
    // valid block --> mark block as no more used
    if (ptr)
        ((uint16_t*)ptr)[-1] &= ~USED;
}

void* MEM_alloc(uint16_t size)
{
    uint16_t* p;
    uint16_t adjsize;
    uint16_t remaining;

    if (size == 0)
        return 0;

    // 2 bytes aligned
    adjsize = (size + sizeof(uint16_t) + 1) & 0xFFFE;

    if (adjsize > *free)
    {
        p = pack(adjsize);

        // no enough memory
        if (p == NULL)
        {
#if (LIB_DEBUG != 0)
            KDebug_Alert("MEM_alloc failed: no enough memory !");
#endif

            return NULL;
        }

        free = p;
    }
    else
        // at this point we can allocate memory
        p = free;

    // set free to next free block
    free += adjsize >> 1;

    // get remaining (old - allocated)
    remaining = *p - adjsize;
    // adjust remaining free space
    if (remaining > 0) *free = remaining;
    else
    {
        // no more space in bloc so we have to find the next free bloc
        uint16_t *newfree = free;
        uint16_t bloc;

        while((bloc = *newfree) & USED)
            newfree += bloc >> 1;

        free = newfree;
    }

    // set block size, mark as used and point to free region
    *p++ = adjsize | USED;

    // return block
    return p;
}

/*
 * Pack free block and return first matching free block.
 */
static uint16_t* pack(uint16_t nsize)
{
    uint16_t *b;
    uint16_t *best;
    uint16_t bsize, psize;

    b = heap;
    best = b;
    bsize = 0;

    while ((psize = *b))
    {
        if (psize & USED)
        {
            if (bsize != 0)
            {
                *best = bsize;

                if (bsize >= nsize)
                    return best;

                bsize = 0;
            }

            b += psize >> 1;
            best = b;
        }
        else
        {
            bsize += psize;
            b += psize >> 1;
        }
    }

    if (bsize != 0)
    {
        *best = bsize;

        if (bsize >= nsize)
            return best;
    }

    return NULL;
}
