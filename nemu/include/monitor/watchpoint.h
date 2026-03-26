#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char expr[64];
  uint32_t old_val;

} WP;
WP* new_wp();
void free_wp(WP *wp);
void print_wp();
void delete_wp(int no);
bool check_watchpoint();

#endif
