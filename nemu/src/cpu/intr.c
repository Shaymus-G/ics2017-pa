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

  Log("raise_intr: NO=0x%x, ret_addr=0x%x, idtr.base=0x%x, gate=0x%x, low=0x%x, high=0x%x, handler=0x%x", NO, ret_addr, cpu.idtr.base, gate_addr, desc_low, desc_high, handler);

  push32(cpu.eflags);
  push32(cpu.cs);
  push32(ret_addr);

  cpu.eip = handler;
}

void dev_raise_intr() {
}
