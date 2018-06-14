/*
 *  linkedlist.h
 *
 *
 *  Created by Vincent Gramoli on 1/12/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <x86intrin.h>
#include <stdbool.h>

#include <atomic_ops.h>

#ifdef DEBUG
#define IO_FLUSH                        fflush(NULL)
/* Note: stdio is thread-safe */
#endif

#define XSTR(s)                         STR(s)
#define STR(s)                          #s

#define ATOMIC_CAS_MB(a, e, v)          (AO_compare_and_swap_full((volatile AO_t *)(a), (AO_t)(e), (AO_t)(v)))

static volatile AO_t stop;

typedef intptr_t val_t;
#define VAL_MIN                         INT_MIN
#define VAL_MAX                         INT_MAX

typedef struct node {
	val_t val;
	val_t key;
	bool flushed;
    double padding[4];
	struct node *next;
} node_t;

typedef struct intset {
	node_t *head;
} intset_t;

node_t *new_node(val_t key, val_t val, node_t *next);
intset_t *set_new();
void set_delete(intset_t *set);
int set_size(intset_t *set);
int set_size_wmarked(intset_t *set);

/*
 * The five following functions handle the low-order mark bit that indicates
 * whether a node is logically deleted (1) or not (0).
 *  - is_marked_ref returns whether it is marked,
 *  - (un)set_marked changes the mark,
 *  - get_(un)marked_ref sets the mark before returning the node.
 */
inline int is_marked_ref(long i) {
  return (int) (i & (LONG_MIN+1));
}

inline long unset_mark(long i) {
	i &= LONG_MAX-1;
	return i;
}

inline long set_mark(long i) {
	i = unset_mark(i);
	i += 1;
	return i;
}

inline long get_unmarked_ref(long w) {
	return unset_mark(w);
}

inline long get_marked_ref(long w) {
	return set_mark(w);
}
