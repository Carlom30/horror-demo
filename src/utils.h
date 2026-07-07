#pragma once

#include <stdlib.h>

typedef struct {
	int cnt;
	int cap;
} header;

#define DA(type) type *
#define DA_ALLOC(ptr)								\
	do {									\
		header *h = malloc(sizeof(header) + sizeof(*(ptr)) * 2);	\
		h->cap = 2;							\
		h->cnt = 0;							\
		(ptr) = (void *)(h + 1);					\
	} while (0)
#define DA_APPEND(ptr, item)								\
	do {										\
		header *h = (((header *)(ptr)) - 1);					\
		if (h->cnt >= h->cap) {						\
			h->cap *= 2;							\
			h = realloc(h, sizeof(header) + sizeof(*(ptr)) * h->cap);	\
			(ptr) = (void *)(h + 1);					\
		}									\
		(ptr)[h->cnt++] = item;						\
	} while (0)
#define DA_COUNT(ptr) (((header *)(ptr)) - 1)->cnt
