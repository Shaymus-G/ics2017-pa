#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  rtl_push(&id_dest->val);

  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&t2);
  operand_write(id_dest, &t2);

  print_asm_template1(pop);
}

make_EHelper(pusha) {
  rtlreg_t temp = cpu.esp;

  rtl_push(&cpu.eax);
  rtl_push(&cpu.ecx);
  rtl_push(&cpu.edx);
  rtl_push(&cpu.ebx);
  rtl_push(&temp);
  rtl_push(&cpu.ebp);
  rtl_push(&cpu.esi);
  rtl_push(&cpu.edi);

  print_asm("pusha");
}

make_EHelper(popa) {
  rtl_pop(&cpu.edi);
  rtl_pop(&cpu.esi);
  rtl_pop(&cpu.ebp);

  rtl_pop(&t0);

  rtl_pop(&cpu.ebx);
  rtl_pop(&cpu.edx);
  rtl_pop(&cpu.ecx);
  rtl_pop(&cpu.eax);

  print_asm("popa");
}

make_EHelper(leave) {
  rtl_mv(&cpu.esp, &cpu.ebp);
  rtl_pop(&cpu.ebp);

  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    rtl_lr_w(&t0, R_AX);
    rtl_msb(&t1, &t0, 2);
    rtl_sub(&t1, &tzero, &t1);
    rtl_sr_w(R_DX, &t1);
  }
  else {
    rtl_lr_l(&t0, R_EAX);
    rtl_msb(&t1, &t0, 4);
    rtl_sub(&t1, &tzero, &t1);
    rtl_sr_l(R_EDX, &t1);
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    rtl_lr_b(&t0, R_AL);
    rtl_sext(&t1, &t0, 1);
    rtl_sr_w(R_AX, &t1);
  }
  else {
    rtl_lr_w(&t0, R_AX);
    rtl_sext(&t1, &t0, 2);
    rtl_sr_l(R_EAX, &t1);
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}

make_EHelper(bsr) {
  uint32_t src = id_src->val;
  rtlreg_t result = 0;

  if (id_src->width == 2) {
    src &= 0xffff;
  }

  if (src == 0) {
    cpu.ZF = 1;
  } else {
    cpu.ZF = 0;

    for (int i = id_src->width * 8 - 1; i >= 0; i--) {
      if (src & (1u << i)) {
        result = i;
	break;
      }
    }

    operand_write(id_dest, &result);
  }

  print_asm_template2(bsr);
}
