#include "intset.h"

int test() {
  //  insert(head, 1);
  //  insert(head, 2);
  //  insert(head, 3);
  //  insert(head, 4);
  //  insert(head, 5);
  //  insert(head, 6);
  //  remove(head, 5);
  //  printf("List size: %d", set_size(head));
  //  printf("List size without marked: %d", set_size_wmarked(head));
  return 0;
}


int table_insert(void *table, int64_t key, int64_t value){
  set_add_l(table, key, value);
  return 0;
}


int table_remove(void *table, int64_t key){
  set_remove_l(table, key);
  return 0;
}

int table_lookup(void *table, int64_t key, int64_t *value){
  return set_contains_l(table, key, value);
}


int table_lookup_safe(void *table, int64_t key, int64_t *value){
  return set_contains_l(table, key, value);
}

int table_print(void *table, int print_level){
  return 0;
}

void table_thread_init(void **table){
  intset_l_t* head = set_new_l();
  *table = head;
  return;
}
