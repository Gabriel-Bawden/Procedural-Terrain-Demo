#include "alloc.h"

#include <stdint.h>
#include <stddef.h>

#include "os.h"

#define ALIGNMENT 8
#define BLOCK_HEADER_SIZE_ALIGNED (sizeof(struct block) + ((ALIGNMENT - (sizeof(struct block) % ALIGNMENT)) % ALIGNMENT))
#define BLOCK_SIZE_MINIMUM (BLOCK_HEADER_SIZE_ALIGNED + ALIGNMENT * sizeof(char))
#define ALLOC_EXTRA (BLOCK_SIZE_MINIMUM + 1000 * ALIGNMENT)

//Should maybe add linked list to keep track of all VirtualAlloc allocations
//For realloc, consider merging with following blocks
//Consider making coalesce and merge separate functions (merge could be reused in realloc)

struct block
{
	size_t size;
	unsigned char is_free;
	struct block *next;
};

static struct block *base = NULL;

static size_t alloc_get_alignment_padding(size_t size)
{
	return (ALIGNMENT - (size % ALIGNMENT)) % ALIGNMENT;
}

static struct block *alloc_create_blocks(size_t block_size_aligned)
{
	size_t block_extra_size_aligned = 0;

	//Checking for overflow before allocating extra block
	if (block_size_aligned <= (SIZE_MAX - ALLOC_EXTRA))
	{
		block_extra_size_aligned = ALLOC_EXTRA;
	}
	else if ((SIZE_MAX - block_size_aligned) >= BLOCK_SIZE_MINIMUM)
	{
		size_t block_extra_size = SIZE_MAX - block_size_aligned;
		block_extra_size_aligned = block_extra_size - (block_extra_size % ALIGNMENT);
	}

	struct block *block = os_alloc_request_memory(block_size_aligned + block_extra_size_aligned);

	block->size = block_size_aligned;
	block->is_free = 0;
	block->next = NULL;

	if (block_extra_size_aligned)
	{
		struct block *block_extra = (struct block *)((unsigned char *)block + block_size_aligned);

		block_extra->size = block_extra_size_aligned;
		block_extra->is_free = 1;
		block_extra->next = NULL;

		block->next = block_extra;
	}

	return block;
}

void *alloc_malloc(size_t size)
{
	size_t padding = alloc_get_alignment_padding(size);

	//Checking that size is valid and will not overflow
	if (size && (size <= (SIZE_MAX - BLOCK_HEADER_SIZE_ALIGNED - padding)))
	{
		size_t size_aligned = size + padding;
		size_t block_new_size_aligned = BLOCK_HEADER_SIZE_ALIGNED + size_aligned;

		if (base)
		{
			struct block *prev = NULL;
			struct block *current = base;

			while (current)
			{
				if ((current->is_free) && (current->size >= block_new_size_aligned))
				{
					struct block *block_new = current;

					//Checking whether the remainder is large enough to be split into its own block
					if ((block_new->size - block_new_size_aligned) >= BLOCK_SIZE_MINIMUM)
					{
						struct block *block_extra = (struct block *)((unsigned char *)block_new + block_new_size_aligned);

						block_extra->is_free = 1;
						block_extra->size = block_new->size - block_new_size_aligned;
						block_extra->next = block_new->next;

						//Updating the size of block new now that block_extra has been split off
						block_new->size = block_new_size_aligned;
						block_new->next = block_extra;
					}

					block_new->is_free = 0;

					return ((unsigned char *)block_new + BLOCK_HEADER_SIZE_ALIGNED);
				}

				prev = current;
				current = current->next;
			}

			//No free block found, creating new blocks
			struct block *block_new = alloc_create_blocks(block_new_size_aligned);

			prev->next = block_new;

			return ((unsigned char *)block_new + BLOCK_HEADER_SIZE_ALIGNED);
		}
		else
		{
			//No blocks exist, creating blocks
			struct block *block_new = alloc_create_blocks(block_new_size_aligned);

			base = block_new;

			return ((unsigned char *)block_new + BLOCK_HEADER_SIZE_ALIGNED);
		}
	}
	else
	{
		return NULL;
	}
}

void *alloc_calloc(size_t count, size_t size)
{
	//Checking that count and size are valid and will not overflow
	if (count && size && (size <= (SIZE_MAX / count)))
	{
		void *ptr = alloc_malloc(count * size);

		if (ptr)
		{
			memset(ptr, 0, count * size);
		}

		return ptr;
	}
	else
	{
		return NULL;
	}
}

void *alloc_realloc(void *src, size_t size)
{
	if (!src)
	{
		return alloc_malloc(size);
	}

	if (!size)
	{
		alloc_free(src);

		return NULL;
	}

	size_t padding = alloc_get_alignment_padding(size);

	//Checking that size will not overflow
	if (size <= (SIZE_MAX - BLOCK_HEADER_SIZE_ALIGNED - padding))
	{
		struct block *block_old = (struct block *)((unsigned char *)src - BLOCK_HEADER_SIZE_ALIGNED);

		size_t size_aligned = size + padding;
		size_t block_new_size_aligned = BLOCK_HEADER_SIZE_ALIGNED + size_aligned;

		//Leaving as is if src is already big enough
		if (block_new_size_aligned <= block_old->size)
		{
			return src;
		}

		void *ptr = alloc_malloc(size);

		if (ptr)
		{
			memcpy(ptr, src, block_old->size - BLOCK_HEADER_SIZE_ALIGNED);

			alloc_free(src);

			return ptr;
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}

static void alloc_merge_blocks()
{
	struct block *current = base;

	while (current && current->next)
	{
		if (current->is_free && current->next->is_free)
		{
			if ((unsigned char *)current + current->size == (unsigned char *)current->next)
			{
				current->size += current->next->size;
				current->next = current->next->next;
			}
			else
			{
				current = current->next;
			}
		}
		else
		{
			current = current->next;
		}
	}
}

void alloc_free(void *src)
{
	if (src)
	{
		struct block *block = (struct block *)((unsigned char *)src - BLOCK_HEADER_SIZE_ALIGNED);

		block->is_free = 1;

		alloc_merge_blocks();
	}
}