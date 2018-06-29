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

/*
 * harris_search looks for value val, it
 *  - returns right_node owning val (if present) or its immediately higher 
 *    value present in the list (otherwise) and 
 *  - sets the left_node to the node owning the value immediately lower than val. 
 * ιστρoφή Χιμένεθ στον ΠΑΟΚEncountered nodes that are marked as logically deleted are physically removed
 * from the list, yet not garbage collected.
 */
node_t *search(intset_t *set, val_t val, node_t **left_node) {
	node_t *left_node_next, *right_node;
	left_node_next = set->head;
search_again:
	do {
		node_t *t = set->head;
		node_t *t_next = set->head->next;
		
		/* Find left_node and right_node */
		do {
			if (!is_marked_ref((long) t_next)) {
				(*left_node) = t;
				left_node_next = t_next;
			}
			t = (node_t *) get_unmarked_ref((long) t_next);
			if (!t->next) break;
			if (!is_marked_flag((long)t_next)) {
				_mm_clflush(&t->next);
        ATOMIC_CAS_MB(&t->next, t_next , get_marked_ref((long) t_next));
			} 
			t_next = t->next;
		} while (is_marked_ref((long) t_next) || (t->val < val));
		right_node = t;
		
		/* Check that nodes are adjacent */
		if (left_node_next == right_node) {
			if (right_node->next && is_marked_ref((long) right_node->next))
				goto search_again;
			else return right_node;
		}
		
		/* Remove one or more marked nodes */
    set_flag((long) (*left_node)->next);
		if (ATOMIC_CAS_MB(&(*left_node)->next, 
						  left_node_next, 
						  right_node)) {
			_mm_clflush(&(*left_node)->next);
      ATOMIC_CAS_MB(&(*left_node)->next, left_node_next, get_flagged_ref((long) left_node_next));
			if (right_node->next && is_marked_ref((long) right_node->next))
				goto search_again;
			else return right_node;
		} 
		
	} while (1);
}

/*
 * harris_find returns whether there is a node in the list owning value val.
 */
int contains(intset_t *set, val_t key, val_t *value) {
#ifdef WAIT_FREE_CONTAINS
  val_t _key = key;
  node_t *curr = set->head;
  node_t *curr_next;
  node_t *pred = curr;
  while (curr->key < _key) {
    pred = curr;
    curr = (node_t*)get_unmarked_ref((long) curr->next);
  }
  if (curr->key == key) {
    *value = curr->val;
    if (!is_marked_flag((long) curr->next)) {
      curr_next = curr->next;
      _mm_clflush(&curr->next);
    }
    ATOMIC_CAS_MB(&curr->next, curr_next, get_flagged_ref((long) curr_next));
    return !is_marked_ref((long) curr->next);
  }

  return false;
  #else
	node_t *right_node, *left_node;
	node_t *right_node_next;
	left_node = set->head;
	
	right_node = search(set, key, &left_node);
	if ((!right_node->next) || right_node->key != key)
		return 0;
	else {
    if (!is_marked_flag((long) right_node->next)){
      right_node_next = right_node->next;
      _mm_clflush(&right_node->next);
      ATOMIC_CAS_MB(&right_node->next, right_node_next, get_flagged_ref((long) right_node_next));
    }
		return 1;
  }
  #endif 
}

/*
 * harris_find inserts a new node with the given value val in the list
 * (if the value was absent) or does nothing (if the value is already present).
 */
int insert(intset_t *set, val_t key, val_t val) {
	node_t *newnode, *right_node, *left_node;
	left_node = set->head;
	
	do {
		right_node = search(set, key, &left_node);
		if (right_node->key == key)
			return 0;
		newnode = new_node(key, val, right_node);
		_mm_clflush(new_node);
		/* mem-bar between node creation and insertion */
		AO_nop_full(); 
		if (ATOMIC_CAS_MB(&left_node->next, right_node, newnode)) {
			_mm_clflush(&left_node->next);
      ATOMIC_CAS_MB(&left_node->next, newnode, get_flagged_ref((long) newnode));
			return 1;
		}
	} while(1);
}

/*
 * harris_find deletes a node with the given value val (if the value is present) 
 * or does nothing (if the value is already present).
 * The deletion is logical and consists of setting the node mark bit to 1.
 */
int remove_(intset_t *set, val_t val) {
	node_t *right_node, *right_node_next, *left_node;
	left_node = set->head;
	
	do {
		right_node = search(set, val, &left_node);
		if (right_node->val != val)
			return 0;
		right_node_next = right_node->next;
		if (!is_marked_ref((long) right_node_next)){
			if (ATOMIC_CAS_MB(&right_node->next, 
					right_node_next, 
					get_marked_ref((long) right_node_next))) {
				_mm_clflush(&right_node->next);
        ATOMIC_CAS_MB(&right_node->next, right_node_next, get_flagged_ref((long) right_node_next));
				break;
			}
    }
	} while(1);
  set_flag((long) left_node->next);
	if (!ATOMIC_CAS_MB(&left_node->next, right_node, right_node_next)) {
    _mm_clflush(&left_node->next);
    ATOMIC_CAS_MB(&left_node->next, right_node_next, get_flagged_ref((long) right_node_next));
		right_node = search(set, right_node->val, &left_node);
  }
	return 1;
}



