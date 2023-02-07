#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
typedef char ALIGN[16];
typedef union header
{
    struct
    {
        size_t size;
        unsigned is_free;
        union header *next;
    } s;
    ALIGN stub;
} header_t;

header_t *head, *tail;
pthread_mutex_t global_malloc_lock;

void *malloc(size_t size)
{
    if (size == 0)
    {
        return NULL;
    }
    size_t totalSize = size + sizeof(header_t);
    header_t *header = NULL;
    void *block = NULL;

    pthread_mutex_lock(&global_malloc_lock); // lock
    header = find_free_block(size);
    if (header)
    {
        // the linked list has free block.
        header->s.is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock); // unlock
        return (void *)(header + 1);
    }
    else
    {
        block = sbrk(totalSize);
        if (block == (void *)-1)
        {
            pthread_mutex_unlock(&global_malloc_lock); // unlock
            return NULL;
        }
        else
        {
            header = block;
            header->s.is_free = 0;
            header->s.size = size;
            header->s.next = NULL;
            if (head == NULL)
            {
                head = header;
            }
            if (tail)
            {
                tail->s.next = header;
            }
            tail = header;
            pthread_mutex_unlock(&global_malloc_lock); // unlock
            return (void *)(header + 1);
        }
    }
}

header_t *get_free_block(size_t size)
{
    header_t *curr = head;
    while (curr)
    {
        if (curr->s.is_free && curr->s.size >= size)
            return curr;
        curr = curr->s.next;
    }
    return NULL;
}

void free(void *block)
{
    if (!block)
    {
        return;
    }
    header_t *header;
    void *programbreak;
    pthread_mutex_lock(&global_malloc_lock);
    header = (header_t *)block - 1;
    programbreak = sbrk(0);
    if (programbreak == (char *)block + header->s.size)
    {
        if (head == tail)
        {
            head = NULL;
            tail = NULL;
        }
        else
        {
            header_t *curr = head;
            while (curr->s.next != header)
            {
                curr = curr->s.next;
            }
            curr->s.next = NULL;
            tail = curr;
        }
        sbrk(0 - sizeof(header_t) - header->s.size);
    }
    else
    {
        header->s.is_free = 1;
    }
    pthread_mutex_unlock(&global_malloc_lock);
}

void *calloc(size_t num, size_t nsize)
{
    size_t totalSize = num * nsize;
    if (totalSize == 0 || nsize != totalSize / num)
    {
        return NULL;
    }
    void *block = malloc(totalSize);
    if (block == NULL)
    {
        return NULL;
    }
    else
    {
        memset(block, 0, totalSize);
        return block;
    }
}

void *realloc(void *block, size_t size)
{
    if (!block)
    {
        return NULL;
    }
    header_t *header = (header_t *)block - 1;
    if (header->s.size >= size)
    {
        return block;
    }
    void *newBlock = malloc(size);
    if (newBlock)
    {
        memcpy(newBlock, block, header->s.size);
        free(block);
    }
    return newBlock;
}
