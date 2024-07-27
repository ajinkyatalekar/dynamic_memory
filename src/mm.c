#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

/* The standard allocator interface from stdlib.h.  These are the
 * functions you must implement, more information on each function is
 * found below. They are declared here in case you want to use one
 * function in the implementation of another. */
void *malloc(size_t rsize);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);

/* When requesting memory from the OS using sbrk(), request it in
 * increments of CHUNK_SIZE. */
#define CHUNK_SIZE (1 << 12)

/*
 * This function, defined in bulk.c, allocates a contiguous memory
 * region of at least size bytes.  It MAY NOT BE USED as the allocator
 * for pool-allocated regions.  Memory allocated using bulk_alloc()
 * must be freed by bulk_free().
 *
 * This function will return NULL on failure.
 */
extern void *bulk_alloc(size_t size);

/*
 * This function is also defined in bulk.c, and it frees an allocation
 * created with bulk_alloc().  Note that the pointer passed to this
 * function MUST have been returned by bulk_alloc(), and the size MUST
 * be the same as the size passed to bulk_alloc() when that memory was
 * allocated.  Any other usage is likely to fail, and may crash your
 * program.
 *
 * Passing incorrect arguments to this function will result in an
 * error message notifying you of this mistake.
 */
extern void bulk_free(void *ptr, size_t size);

/*
 * This function computes the log base 2 of the allocation block size
 * for a given allocation.  To find the allocation block size from the
 * result of this function, use 1 << block_index(x).
 *
 * This function ALREADY ACCOUNTS FOR both padding and the size of the
 * header.
 *
 * Note that its results are NOT meaningful for any
 * size > 4088!
 *
 * You do NOT need to understand how this function works.  If you are
 * curious, see the gcc info page and search for __builtin_clz; it
 * basically counts the number of leading binary zeroes in the value
 * passed as its argument.
 */
static inline __attribute__((unused)) int block_index(size_t x)
{
    if (x <= 8)
    {
        return 5;
    }
    else
    {
        return 32 - __builtin_clz((unsigned int)x + 7);
    }
}

typedef struct Block
{
    size_t size;
    bool f;
    struct Block *next;
} Block;

static Block *freep[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

static int sfi(int index)
{
    int size = 32;
    for (int i = 0; i < index; i++)
    {
        size = size * 2;
    }

    return size;
}

static void bulk_req(int index)
{
    // fprintf(stderr, "running bulk_req.. \n");

    int size = sfi(index);

    int n = CHUNK_SIZE / size;

    Block *p = sbrk(CHUNK_SIZE);
    p->size = size;
    p->f = true;
    p->next = NULL;

    Block *current = p;
    for (int i = 1; i < n; i++)
    {
        Block *newp = (void *)current + size;
        newp->size = size;
        p->f = true;
        newp->next = NULL;
        current->next = newp;
        current = newp;
    }

    freep[index] = p;
}

static Block *rem_p(size_t index)
{

    // fprintf(stderr, "running rem_p.. \n");

    if (freep[index] == NULL) // No elements left
    {
        bulk_req(index);
    }

    Block *p = freep[index];
    p->f = false;
    freep[index] = freep[index]->next;
    return p;
}

/*
 * You must implement malloc().  Your implementation of malloc() must be
 * the multi-pool allocator described in the project handout.
 */
void *malloc(size_t size)
{
    // fprintf(stderr, "running malloc.. \n");

    if (size == 0)
    {
        size = 1;
    }

    if (size > 4088)
    {
        void *ptr = bulk_alloc(size + 8);
        size_t tot_size = size + 8;
        memcpy(ptr, &tot_size, sizeof(size_t));
        return ptr + 8;
    }

    int index = block_index(size) - 5;

    Block *p = rem_p(index);

    return (void *)p + 8;
}

/*
 * You must also implement calloc().  It should create allocations
 * compatible with those created by malloc().  In particular, any
 * allocations of a total size <= 4088 bytes must be pool allocated,
 * while larger allocations must use the bulk allocator.
 *
 * calloc() (see man 3 calloc) returns a cleared allocation large enough
 * to hold nmemb elements of size size.  It is cleared by setting every
 * byte of the allocation to 0.  You should use the function memset()
 * for this (see man 3 memset).
 */
void *calloc(size_t nmemb, size_t size)
{
    // fprintf(stderr, "running calloc.. \n");

    void *ptr = malloc(nmemb * size);
    memset(ptr, 0, nmemb*size);
    return ptr;
}

/*
 * You must also implement realloc().  It should create allocations
 * compatible with those created by malloc(), honoring the pool
 * alocation and bulk allocation rules.  It must move data from the
 * previously-allocated block to the newly-allocated block if it cannot
 * resize the given block directly.  See man 3 realloc for more
 * information on what this means.
 *
 * It is not possible to implement realloc() using bulk_alloc() without
 * additional metadata, so the given code is NOT a working
 * implementation!
 */
void *realloc(void *ptr, size_t size)
{

    // fprintf(stderr, "running realloc.. \n");

    if (ptr == NULL)
    {
        // fprintf(stderr, "    NULL POINTER.. \n");
        return malloc(size);
    }

    if (size == 0 && ptr != NULL)
    {
        // fprintf(stderr, "    SIZE ZERO.. \n");
        free(ptr);
        return NULL;
    }

    size_t curr_size = *(size_t *)(ptr - 8);
    if (curr_size >= size + 8) // No new allocation
    {
        // fprintf(stderr, "    NO NEW REALLOC.. \n");
        return ptr;
    }

    // fprintf(stderr, "    NEW REALLOC.. \n");
    void *new = malloc(size);

    if (new == NULL)
    {
        return NULL;
    }
    memcpy(new, ptr, curr_size - 8);
    free(ptr);
    return new;
}

/*
 * You should implement a free() that can successfully free a region of
 * memory allocated by any of the above allocation routines, whether it
 * is a pool- or bulk-allocated region.
 *
 * The given implementation does nothing.
 */
void free(void *ptr)
{
    // fprintf(stderr, "running free.. \n");

    if (ptr == NULL)
    {
        return;
    }

    size_t size = *(size_t *)(ptr - 8);

    if (size > 4096) // Bulk free
    {
        bulk_free(ptr - 8, size);
        return;
    }

    Block *curr = (Block *)(ptr - 8);
    int index = block_index(size - 8) - 5; // size - 8 to get size without metadata
    curr->f = true;
    if (freep[index] == NULL) {
    	curr->next = NULL;
	freep[index] = curr; // THIS IS THE LINE THAT (USED TO) BREAK VIM!
    } else {
	curr->next = freep[index];
    	freep[index] = curr;
    }

    return;
}
