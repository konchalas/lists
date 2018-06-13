/*
 *  linkedlist.cpp
 *
 *  Linked list data structure
 *
 *  Created by Vincent Gramoli on 1/12/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "linkedlist.h"

extern inline int is_marked_ref(long i);
extern inline long unset_mark(long i);
extern inline long set_mark(long i);
extern inline long get_unmarked_ref(long w);
extern inline long get_marked_ref(long w);

node_t *new_node(val_t key, val_t val, node_t *next)
{
  node_t *node;

  node = (node_t *)malloc(sizeof(node_t));
  if (node == NULL) {
    perror("malloc");
    exit(1);
  }

  node->key = key;
  node->val = val;
  node->next = next;
  node->flushed = false;
  _mm_clflush(&node);

  return node;
}

intset_t *set_new()
{
  intset_t *set;
  node_t *min, *max;

  if ((set = (intset_t *)malloc(sizeof(intset_t))) == NULL) {
    perror("malloc");
    exit(1);
  }
  max = new_node(VAL_MAX, 0, NULL);
  min = new_node(VAL_MIN, 0, max);
  set->head = min;
  _mm_clflush(&set);
  _mm_clflush(&set->head);
  _mm_clflush(&max);

  return set;
}

void set_delete(intset_t *set)
{
  node_t *node, *next;

  node = set->head;
  while (node != NULL) {
    next = node->next;
    free(node);
    node = next;
  }
  free(set);
}

int set_size(intset_t *set)
{
  int size = 0;
  node_t *node;

  /* We have at least 2 elements */
  node = set->head->next;
  if (is_marked_ref((long) node))
    node = (node_t*)get_unmarked_ref((long) node);
  while (node->next != NULL) {
    size++;
    node = node->next;
    if (is_marked_ref((long) node)) {
      node = (node_t*)get_unmarked_ref((long) node);
    }
  }

  return size;
}

int set_size_wmarked(intset_t *set)
{
  int size = 0;
  node_t *node;

  /* We have at least 2 elements */
  node = set->head->next;
  if (is_marked_ref((long) node)){
    node = (node_t*)get_unmarked_ref((long) node);
  }
  while (node->next != NULL) {
    node = node->next;
    if (!is_marked_ref((long) node)) {
      size++;
      continue;
    }
    node = (node_t*)get_unmarked_ref((long) node);
  }
  return size;
}
