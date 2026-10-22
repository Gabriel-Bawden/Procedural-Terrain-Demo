#include "dynarray.h"

#include <stddef.h>

#include "alloc.h"
#include "os.h"

void dynarray_create(struct dynarray *v, size_t size_element, void (*destroy)(void *))
{
	v->size_array = 1;
	v->size_element = size_element;
	v->current = 0;
	v->destroy = destroy;
	v->array = alloc_malloc(v->size_array * v->size_element);
}

void *dynarray_get(struct dynarray *v, size_t index)
{
	if (index < v->current)
	{
		return (v->array + index * v->size_element);
	}
	else
	{
		return NULL;
	}
}

void dynarray_destroy_element(struct dynarray *v, size_t index)
{
	if (v->destroy && index < v->current)
	{
		(v->destroy)(v->array + index * v->size_element);
	}
}

void dynarray_clear(struct dynarray *v)
{
	for (size_t i = 0; i < v->current; i++)
	{
		dynarray_destroy_element(v, i);
	}

	v->current = 0;
}

void dynarray_destroy(void *ptr)
{
	struct dynarray *v = ptr;

	dynarray_clear(v);

	alloc_free(v->array);
}

void dynarray_size_check(struct dynarray *v)
{
	if (v->current >= v->size_array)
	{
		v->size_array *= 2; //Should change so this doesn't happen if realloc fails
		v->array = alloc_realloc(v->array, v->size_array * v->size_element);
	}
}

void dynarray_set(struct dynarray *v, size_t index, void *value)
{
	if (index < v->current)
	{
		dynarray_destroy_element(v, index);
		memcpy(v->array + index * v->size_element, value, v->size_element);
	}
}

void dynarray_push(struct dynarray *v, void *value)
{
	dynarray_size_check(v);

	memcpy(v->array + v->current * v->size_element, value, v->size_element);

	v->current++;
}

void dynarray_pop(struct dynarray *v, void *pop)
{
	if (v->current)
	{
		v->current--;

		if (pop)
		{
			memcpy(pop, v->array + v->current * v->size_element, v->size_element);
		}

		dynarray_destroy_element(v, v->current);
	}
}

void dynarray_insert(struct dynarray *v, size_t index, void *value)
{
	if (index < v->current || !index)
	{
		dynarray_size_check(v);

		memmove(v->array + (index + 1) * v->size_element, v->array + index * v->size_element, (v->current - index) * v->size_element);

		memcpy(v->array + index * v->size_element, value, v->size_element); //Not using dynarray_set() because this would destroy an element that is simply being moved

		v->current++;
	}
}

void dynarray_remove(struct dynarray *v, size_t index)
{
	if (index < v->current)
	{
		dynarray_destroy_element(v, index);

		if (index != v->current - 1)
		{
			memmove(v->array + index * v->size_element, v->array + (index + 1) * v->size_element, (v->current - index - 1) * v->size_element);
		}

		v->current--;
	}
}

size_t dynarray_len(struct dynarray *v)
{
	return v->current;
}