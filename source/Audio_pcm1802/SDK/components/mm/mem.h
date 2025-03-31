/****************************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

#ifndef __BFLB_MM_MEM_H
#define __BFLB_MM_MEM_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define MEM_ASSERT(x)                           \
    {                                           \
        if (!(x)) {                               \
            printf("[MEM] !!! assert " #x "\r\n"); \
            while (1)                           \
                ;                               \
        }                                       \
    }

#define MEM_LOG(fmt, ...)     //printf("[MEM] "fmt, __VA_ARGS__)

#define MEM_IS_VALID(heap)    ((heap) != NULL && (heap)->mem_impl != NULL)


#define KMEM_HEAP &g_memheap

/****************************************************************************
 * Public Types
 ****************************************************************************/

struct mem_heap_impl_s; /* Forward reference */
struct mem_heap_s {
    struct mem_heap_impl_s *mem_impl;
};

struct meminfo
{
    int total_size;    /* This is the total size of memory allocated
                        * for use by malloc in bytes. */
    int free_node;     /* This is the number of free (not in use) chunks */
    int used_node;     /* This is the number of allocated (in use) chunks */
    int max_free_size; /* Size of the largest free (not in use) chunk */
    int used_size;     /* This is the total size of memory occupied by
                        * chunks handed out by malloc. */
    int free_size;     /* This is the total size of memory occupied
                        * by free (not in use) chunks. */
};

/****************************************************************************
 * Public Data
 ****************************************************************************/

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif


EXTERN struct mem_heap_s g_memheap;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

void kmem_init(void *heapstart, size_t heapsize);

void umem_init(struct mem_heap_s *heap, void *heapstart, size_t heapsize);

/* private api for mm*/

void bflb_mem_init(struct mem_heap_s *heap, void *heapstart, size_t heapsize);

void *bflb_malloc(struct mem_heap_s *heap, size_t nbytes);

void bflb_free(struct mem_heap_s *heap, void *ptr);

void *bflb_realloc(struct mem_heap_s *heap, void *ptr, size_t nbytes);

void *bflb_calloc(struct mem_heap_s *heap, size_t count, size_t size);

void *bflb_malloc_align(struct mem_heap_s *heap, size_t align, size_t size);

void bflb_mem_usage(struct mem_heap_s *heap, struct meminfo *info);

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* __BFLB_MM_MEM_H */

