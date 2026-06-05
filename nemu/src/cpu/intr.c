#include "cpu/exec.h"
#include "memory/mmu.h"

static inline void push32(uint32_t val) {
  cpu.esp -= 4;
  vaddr_write(cpu.esp, 4, val);
}

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  uint32_t gate_addr = cpu.idtr.base + NO * 8;

  uint32_t desc_low = vaddr_read(gate_addr, 4);
  uint32_t desc_high = vaddr_read(gate_addr + 4, 4);

  uint32_t handler = (desc_low & 0xffff) | (desc_high & 0xffff0000);

  push32(cpu.eflags);
  push32(cpu.cs);
  push32(ret_addr);

  cpu.eip = handler;
}

static volatile bool intr_pending = false;

void dev_raise_intr() {
  intr_pending = true;
}

bool query_intr(void) {
  if (intr_pending && cpu.IF) {
    intr_pending = false;
    return true;
  }
  return false;
}
