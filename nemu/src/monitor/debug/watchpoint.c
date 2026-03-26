#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(){
  if(free_==NULL){
    printf("No free watchpoints!\n");
    assert(0);
  }
  WP *wp=free_;
  free_=free_->next;
  wp->next=head;
  head=wp;
  return wp;
}

void free_wp(WP *wp){
  if(wp==NULL||head==NULL) return;
  if(head==wp){
    head=head->next;
  }
  else{
    WP *curr=head;
    while(curr->next!=NULL&&curr->next!=wp){
      curr=curr->next;
    }
    if(curr->next==wp){
      curr->next=wp->next;
    }
  }
  wp->next=free_;
  free_=wp;
  wp->expr[0]='\0';
  wp->old_val=0;
}

void print_wp(){
  printf("Num\tType\t\tWhat\n");
  WP *curr=head;
  while(curr!=NULL){
    printf("%d\twatchpoint\t%s\n",curr->NO,curr->expr);
    curr=curr->next;
  }
}

void delete_wp(int no){
  WP *curr=head;
  while(curr!=NULL){
    if(curr->NO==no){
      free_wp(curr);
      printf("Watchpoint %d deleted\n",no);
      return;
    }
    curr=curr->next;
  }
  printf("Watchpoint %d not found\n",no);
}

bool check_watchpoint(){
  bool changed=false;
  WP *curr=head;
  while(curr!=NULL){
    bool success=false;
    uint32_t new_val=expr(curr->expr,&success);
    if(new_val!=curr->old_val){
      printf("Watchpoint %d: %s\n",curr->NO,curr->expr);
      printf("Old value=%u (0x%08x)\n",curr->old_val,curr->old_val);
      printf("New value=%u (0x%08x)\n",new_val,new_val);
      curr->old_val=new_val;
      changed=true;
    }
    curr=curr->next;
  }
  return changed;
}

