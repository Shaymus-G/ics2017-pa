#include "cpu/exec.h"

make_EHelper(test) {
  rtl_and(&t2, &id_dest->val, &id_src->val);

  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_li(&t0, 0);
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);

  print_asm_template2(test);
}

make_EHelper(and) {
  rtl_and(&t2, &id_dest->val, &id_src->val);

  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_li(&t0, 0);
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);

  print_asm_template2(and);
}

make_EHelper(xor) {
  rtl_xor(&t2, &id_dest->val, &id_src->val);

  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_li(&t0, 0);
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);

  print_asm_template2(xor);
}

make_EHelper(or) {
  rtl_or(&t2, &id_dest->val, &id_src->val);

  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_li(&t0, 0);
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);

  print_asm_template2(or);
}

make_EHelper(sar) {
  rtl_andi(&t1, &id_src->val, 0x1f);

  if (id_dest->width == 4) {
    rtl_sar(&t2, &id_dest->val, &t1);
  }
  else {
    rtl_sext(&t0, &id_dest->val, id_dest->width);
    rtl_sar(&t2, &t0, &t1);
  }

  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);

  // unnecessary to update CF and OF in NEMU

  print_asm_template2(sar);
}

make_EHelper(shl) {
  rtl_andi(&t1, &id_src->val, 0x1f);

  rtl_shl(&t2, &id_dest->val, &t1);

  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);

  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shl);
}

make_EHelper(shr) {
  rtl_andi(&t1, &id_src->val, 0x1f);

  rtl_shr(&t2, &id_dest->val, &t1);

  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);

  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shr);
}

make_EHelper(shld) {
  assert(id_dest->width == 2 || id_dest->width == 4);

  uint32_t width_bits = id_dest->width * 8;
  uint32_t count = id_src->val & 0x1f;

  uint32_t mask;
  if (id_dest->width == 2) {
    mask = 0xffff;
  } else {
    mask = 0xffffffffu;
  }

  uint32_t dest = id_dest->val & mask;
  uint32_t src = id_src2->val & mask;

  if (count != 0) {
    if (count < width_bits) {
      t2 = ((dest << count) | (src >> (width_bits - count))) & mask;
      operand_write(id_dest, &t2);
      rtl_update_ZFSF(&t2, id_dest->width);

      cpu.CF = (dest >> (width_bits - count)) & 0x1;
    } else {
    }
  }

  print_asm("shld %s,%s,%s", id_src->str, id_src2->str, id_dest->str);
}

make_EHelper(shrd) {
  assert(id_dest->width == 2 || id_dest->width == 4);

  uint32_t width_bits = id_dest->width * 8;
  uint32_t count = id_src->val & 0x1f;

  uint32_t mask;
  if (id_dest->width == 2) {
    mask = 0xffff;
  } else {
    mask = 0xffffffffu;
  }

  uint32_t dest = id_dest->val & mask;
  uint32_t src = id_src2->val & mask;

  if (count != 0) {
    if (count < width_bits) {
      t2 = ((dest >> count) | (src << (width_bits - count))) & mask;
      operand_write(id_dest, &t2);
      rtl_update_ZFSF(&t2, id_dest->width);

      cpu.CF = (dest >> (count - 1)) & 0x1;
    } else {
    }
  }

  print_asm("shrd %s,%s,%s", id_src->str, id_src2->str, id_dest->str);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  rtl_not(&id_dest->val);
  operand_write(id_dest, &id_dest->val);

  print_asm_template1(not);
}

make_EHelper(rol) {
  rtl_andi(&t1, &id_src->val, 0x1f);

  uint32_t width_bits = id_dest->width * 8;
  uint32_t count = t1 % width_bits;

  if (count == 0) {
    t2 = id_dest->val;
  }
  else {
    uint32_t mask;
    switch (id_dest->width) {
      case 1:
        mask = 0xff;
	break;
      case 2:
	mask = 0xffff;
	break;
      case 4:
	mask = 0xffffffffu;
	break;
      default:
	assert(0);
    }

    uint32_t val = id_dest->val & mask;
    t2 = ((val << count) | (val >> (width_bits - count))) & mask;
  }

  operand_write(id_dest, &t2);

  print_asm_template2(rol);
}
