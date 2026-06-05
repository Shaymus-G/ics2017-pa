#include "proc.h"

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC];
static int nr_proc = 0;
PCB *current = NULL;
static int current_game = 0;
//static bool game_switch_pending = false;
static bool force_game_schedule = false;

uintptr_t loader(_Protect *as, const char *filename);

void load_prog(const char *filename) {
  int i = nr_proc ++;
  _protect(&pcb[i].as);

  uintptr_t entry = loader(&pcb[i].as, filename);

  // TODO: remove the following three lines after you have implemented _umake()

  _Area stack;
  stack.start = pcb[i].stack;
  stack.end = stack.start + sizeof(pcb[i].stack);

  pcb[i].tf = _umake(&pcb[i].as, stack, stack, (void *)entry, NULL, NULL);

  Log("load process %d: filename = %s, entry = 0x%x, tf = %p", i, filename, entry, pcb[i].tf);
}

void switch_game(void) {
  if (nr_proc < 3) {
    return;
  }

  if (current_game == 0) {
    current_game = 2;
    Log("switch game: /bin/videotest");
  } else {
    current_game = 0;
    Log("switch game: /bin/pal");
  }

  force_game_schedule = true;
}

//bool consume_game_switch_pending(void) {
  //if (game_switch_pending) {
    //game_switch_pending = false;
    //force_game_schedule = true;
    //return true;
  //}

  //return false;
//}

_RegSet* schedule(_RegSet *prev) {
  static int sched_cnt = 0;

  int old = -1;
  if (current != NULL) {
    old = current - pcb;
  }

  if (current != NULL && prev != NULL) {
    current->tf = prev;
  }

  if (nr_proc == 0) {
    return NULL;
  }

  int next = 0;

  if (force_game_schedule) {
    force_game_schedule = false;
    next = current_game;
  } else if (nr_proc >= 3) {
    sched_cnt ++;

    if (sched_cnt % 20 == 0) {
      next = 1;
    } else {
      next = current_game;
    }
  } else if (nr_proc >= 2) {
    sched_cnt ++;

    if (sched_cnt % 20 == 0) {
      next = 1;
    } else {
      next = 0;
    }
  }else {
    next = 0;
  }

  //if (current != NULL) {
    //int cur = current - pcb;
    //next = (cur + 1) % nr_proc;
  //}

  current = &pcb[next];

  Log("schedule: old = %d, next = %d, current_game = %d, prev = %p, prev_eip = 0x%x, prev_esp = 0x%x, next_tf = %p, next_eip = 0x%x, next_esp = 0x%x, next_as = %p", old, next, current_game, prev, prev ? prev->eip : 0, prev ? prev->esp : 0, current->tf, current->tf ? current->tf->eip : 0, current->tf ? current->tf->esp : 0, current->as.ptr);

  _switch(&current->as);

  return current->tf;
}

void run_first_proc(void) {
  _RegSet *tf = schedule(NULL);

  asm volatile(
    "movl %0, %%esp;"
    "popal;"
    "addl $8, %%esp;"
    "iret"
    :
    : "r"(tf)
    : "memory"
  );

  panic("Should not reach here");
}
