#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>

static uint64_t udiv64_u32(uint64_t n, uint32_t d) {
  assert(d != 0);

  uint64_t q = 0;
  uint64_t r = 0;

  for (int i = 63; i >= 0; i --) {
    r = (r << 1) | ((n >> i) & 1);
    if (r >= d) {
      r -= d;
      q |= (1ULL << i);
    }
  }

  return q;
}

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  return (FLOAT)(((int64_t)a * (int64_t)b) >> F_SHIFT);
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
  assert(b != 0);

  int sign = 0;
  int64_t x = a;
  int64_t y = b;

  if (x < 0) {
    x = -x;
    sign ^= 1;
  }

  if (y < 0) {
    y = -y;
    sign ^= 1;
  }

  uint64_t numerator = ((uint64_t)x) << F_SHIFT;
  uint64_t quotient = udiv64_u32(numerator, (uint32_t)y);

  int64_t result = sign ? -(int64_t)quotient : (int64_t)quotient;
  return (FLOAT)result;
}

FLOAT f2F(float a) {
  /* You should figure out how to convert `a' into FLOAT without
   * introducing x87 floating point instructions. Else you can
   * not run this code in NEMU before implementing x87 floating
   * point instructions, which is contrary to our expectation.
   *
   * Hint: The bit representation of `a' is already on the
   * stack. How do you retrieve it to another variable without
   * performing arithmetic operations on it directly?
   */
  union {
    float f;
    uint32_t u;
  } conv;

  conv.f = a;
  uint32_t bits = conv.u;

  int sign = bits >> 31;
  int exp = (bits >> 23) & 0xff;
  uint32_t frac = bits & 0x7fffff;

  if (exp == 0 && frac == 0) {
    return 0;
  }

  int64_t mantissa;
  int shift;

  if (exp == 0) {
    mantissa = frac;
    shift = -126 - 23 + F_SHIFT;
  } else {
    mantissa = (1u << 23) | frac;
    shift = exp - 127 - 23 + F_SHIFT;
  }

  int64_t result;
  if (shift >= 0) {
    result = mantissa << shift;
  } else {
    result = mantissa >> (-shift);
  }

  if (sign) {
    result = -result;
  }

  return (FLOAT)result;
}

FLOAT Fabs(FLOAT a) {
  return a < 0 ? -a : a;
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x) {
  FLOAT dt, t = int2F(2);

  do {
    dt = F_div_int((F_div_F(x, t) - t), 2);
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
  /* we only compute x^0.333 */
  FLOAT t2, dt, t = int2F(2);

  do {
    t2 = F_mul_F(t, t);
    dt = (F_div_F(x, t2) - t) / 3;
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}
