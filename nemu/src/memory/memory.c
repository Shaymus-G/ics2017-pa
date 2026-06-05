#include "nemu.h"
#include "memory/mmu.h"

#ifdef HAS_IOE
#include "device/mmio.h"
#endif

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
#ifdef HAS_IOE
  int map_NO = is_mmio(addr);
  if (map_NO != -1) {
    return mmio_read(addr, len, map_NO);
  }
#endif

  return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
#ifdef HAS_IOE
  int map_NO = is_mmio(addr);
  if (map_NO != -1) {
    mmio_write(addr, len, data, map_NO);
    return;
  }
#endif

  memcpy(guest_to_host(addr), &data, len);
}

static paddr_t page_translate(vaddr_t addr, bool is_write) {
  uint32_t dir = (addr >> 22) & 0x3ff;
  uint32_t page = (addr >> 12) & 0x3ff;
  uint32_t offset = addr & PAGE_MASK;

  paddr_t pdir_base = cpu.cr3 & 0xfffff000;
  paddr_t pde_addr = pdir_base + dir * 4;

  PDE pde;
  pde.val = paddr_read(pde_addr, 4);

  Assert(pde.present, "Page directory entry not present: eip = 0x%08x, esp = 0x%08x, cr3 = 0x%08x, vaddr = 0x%08x, pde_addr = 0x%08x, pde = 0x%08x", cpu.eip, cpu.esp, cpu.cr3, addr, pde_addr, pde.val);

  if (!pde.accessed) {
    pde.accessed = 1;
    paddr_write(pde_addr, 4, pde.val);
  }

  paddr_t ptab_base = pde.page_frame << 12;
  paddr_t pte_addr = ptab_base + page * 4;

  PTE pte;
  pte.val = paddr_read(pte_addr, 4);

  Assert(pte.present, "Page table entry not present: eip = 0x%08x, esp = 0x%08x, cr3 = 0x%08x, vaddr = 0x%08x, pte_addr = 0x%08x, pte = 0x%08x", cpu.eip, cpu.esp, cpu.cr3, addr, pte_addr, pte.val);

  if (!pte.accessed || (is_write && !pte.dirty)) {
    pte.accessed = 1;
    if (is_write) {
      pte.dirty = 1;
    }
    paddr_write(pte_addr, 4, pte.val);
  }

  return (pte.page_frame << 12) | offset;
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  if (!cpu.cr0.PG) {
    return paddr_read(addr, len);
  }

  if ((addr & PAGE_MASK) + len <= PAGE_SIZE) {
    return paddr_read(page_translate(addr, false), len);
  }

  uint32_t ret = 0;
  int i;
  for (i = 0; i < len; i ++) {
    ret |= paddr_read(page_translate(addr + i, false), 1) << (i * 8);
  }
  return ret;
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if (!cpu.cr0.PG) {
    paddr_write(addr, len, data);
    return;
  }

  if ((addr & PAGE_MASK) + len <= PAGE_SIZE) {
    paddr_write(page_translate(addr, true), len, data);
    return;
  }

  int i;
  for (i = 0; i < len; i ++) {
    paddr_write(page_translate(addr + i, true), 1, data >> (i * 8));
  }
}
