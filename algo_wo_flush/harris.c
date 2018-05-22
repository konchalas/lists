/*
 * File:
 *   harris.c
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 * Description:
 *   Lock-free linkedlist implementation of Harris' algorithm
 *   "A Pragmatic Implementation of Non-Blocking Linked Lists"
 *   T. Harris, p. 300-314, DISC 2001.
 *
 * Copyright (c) 2009-2010.
 *
 * harris.c is part of Synchrobench
 *
 * Synchrobench is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "harris.h"

int count_success = 0;
int count = 0;

/*
 * harris_search looks for value key, it
 *  - returns right_node owning key(if present) or its immediately higher
 *    value present in the list (otherwise) and
 *  - sets the left_node to the node owning the value immediately lower than key.
 * Encountered nodes that are marked as logically deleted are physically removed
 * from the list, yet not garbage collected.
 */
node_t *search(intset_t *set, val_t key, node_t **left_node) {
  node_t *pred = NULL, *curr = NULL, *succ = NULL;
  pred = set->head;
  curr = (node_t*)get_unmarked_ref((long) pred->next);
  while (true) {
    succ = (node_t*)get_unmarked_ref((long) curr->next);
    if (curr->key >= key) {
      (*left_node) = pred;
      return curr;
    }
    pred = curr;
    curr = succ;
  }
}

/*
 * harris_contains returns whether there is a node in the list owning value key.
 */
int contains(intset_t *set, val_t key, val_t *value) {
  val_t _key = key;
  node_t *curr = set->head;
  node_t *pred = curr;
  while (curr->key < _key) {
    pred = curr;
    curr = (node_t*)get_unmarked_ref((long) curr->next);
  }
  if (curr->key == key) {
    *value = curr->val;
    return !is_marked_ref((long) curr->next);
  }

  return false;
}

int count;
/*
 * harris_contains inserts a new node with the given value key in the list
 * (if the value was absent) or does nothing (if the value is already present).
 */
int insert(intset_t *set, val_t key, val_t value) {
	node_t *newnode, *right_node, *left_node;
	left_node = set->head;
  count++;
	do {
		right_node = search(set, key, &left_node);
		if (right_node->key == key)
			return 0;
		newnode = new_node(key, value, right_node);
		/* mem-bar between node creation and insertion */
		AO_nop_full();
		if (ATOMIC_CAS_MB(&left_node->next, right_node, newnode)) {
      count_success++;
			return 1;
    }
	} while(1);
}

/*
 * harris_contains deletes a node with the given value key(if the value is present)
 * or does nothing (if the value is already present).
 * The deletion is logical and consists of setting the node mark bit to 1.
 */
int remove_(intset_t *set, val_t key) {
	node_t *right_node, *right_node_next, *left_node;
	left_node = set->head;

	do {
		right_node = search(set, key, &left_node);
		if (right_node->key != key)
			return 0;
		right_node_next = right_node->next;
		if (!is_marked_ref((long) right_node_next)){
			if (ATOMIC_CAS_MB(&right_node->next,
							  right_node_next,
                        get_marked_ref((long) right_node_next))) {
				break;
      }
    }
	} while(1);
	return 1;
}



