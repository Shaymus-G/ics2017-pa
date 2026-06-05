#include "common.h"

extern _RegSet* do_syscall(_RegSet *r);
extern _RegSet* schedule(_RegSet *prev);
extern bool consume_game_switch_pending(void);

static _RegSet* do_event(_Event e, _RegSet* r) {
  switch (e.event) {
    case _EVENT_SYSCALL:
      do_syscall(r);
      if (consume_game_switch_pending()) {
        return schedule(r);
      }
      return NULL;

    case _EVENT_IRQ_TIME:
      return schedule(r);

    default: panic("Unhandled event ID = %d", e.event);
  }

  return NULL;
}

void init_irq(void) {
  _asye_init(do_event);
}
