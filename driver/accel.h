#ifndef _ACCEL_H_
#define _ACCEL_H_

#include "dbg.h"
#include "fixedptc.h"
#include "speed.h"
#include "vector.h"

static inline fixedpt minsd(fixedpt a, fixedpt b) { return (a < b) ? a : b; }

static inline fixedpt base_fn(fixedpt x, fixedpt accel, fixedpt input_offset) {
  fixedpt _x = x - input_offset;
  fixedpt _x_square = fixedpt_mul(
      _x, _x); // because linear in rawaccel is classic with exponent = 2
  return fixedpt_mul(accel, fixedpt_div(_x_square, x));
}

/**
 * Sensitivity Function for Linear Acceleration
 */
static inline fixedpt __sensitivity(fixedpt input_speed, fixedpt param_accel,
                                    fixedpt param_offset,
                                    fixedpt param_output_cap) {
  if (input_speed <= param_offset) {
    return FIXEDPT_ONE;
  }

  fixedpt sens = base_fn(input_speed, param_accel, param_offset);
  dbg("base_fn sens               %s", fptoa(param_accel));

  fixedpt sign = FIXEDPT_ONE;
  if (param_output_cap > 0) {
    fixedpt cap = fixedpt_sub(param_output_cap, FIXEDPT_ONE);
    if (cap < 0) {
      cap = -cap;
      sign = -sign;
    }
    sens = minsd(sens, cap);
  }

  return fixedpt_add(FIXEDPT_ONE, fixedpt_mul(sign, sens));
}

/**
 * Calculate the factor by which to multiply the input vector
 * in order to get the desired output speed.
 *
 */
static inline struct vector
sensitivity(fixedpt input_speed, fixedpt param_sens_mult,
            fixedpt param_yx_ratio, fixedpt param_accel, fixedpt param_offset,

            fixedpt param_output_cap) {
  fixedpt sens =
      __sensitivity(input_speed, param_accel, param_offset, param_output_cap);
  sens = fixedpt_mul(sens, param_sens_mult);
  return (struct vector){sens, fixedpt_mul(sens, param_yx_ratio)};
}

static inline void f_accelerate(int *x, int *y, fixedpt time_interval_ms,
                                fixedpt param_sens_mult, fixedpt param_yx_ratio,
                                fixedpt param_accel, fixedpt param_offset,
                                fixedpt param_output_cap) {
  /* AccelResult result = {.x = 0, .y = 0}; */

  static fixedpt carry_x = 0;
  static fixedpt carry_y = 0;

  fixedpt dx = fixedpt_fromint(*x);
  fixedpt dy = fixedpt_fromint(*y);

  dbg("in                        (%d, %d)", *x, *y);
  dbg("in: x (fixedpt conversion) %s", fptoa(dx));
  dbg("in: y (fixedpt conversion) %s", fptoa(dy));

  fixedpt speed_in = input_speed(dx, dy, time_interval_ms);
  struct vector sens = sensitivity(speed_in, param_sens_mult, param_yx_ratio,
                                   param_accel, param_offset, param_output_cap);
  dbg("scale x                    %s", fptoa(sens.x));
  dbg("scale y                    %s", fptoa(sens.y));

  fixedpt dx_out = fixedpt_mul(dx, sens.x);
  fixedpt dy_out = fixedpt_mul(dy, sens.y);

  dx_out = fixedpt_add(dx_out, carry_x);
  dy_out = fixedpt_add(dy_out, carry_y);

  dbg("out: x                     %s", fptoa(dx_out));
  dbg("out: y                     %s", fptoa(dy_out));

  *x = fixedpt_toint(dx_out);
  *y = fixedpt_toint(dy_out);

  dbg("out (int conversion)      (%d, %d)", *x, *y);

  carry_x = fixedpt_sub(dx_out, fixedpt_fromint(*x));
  carry_y = fixedpt_sub(dy_out, fixedpt_fromint(*y));

  dbg("carry                     (%s, %s)", fptoa(carry_x), fptoa(carry_x));
}

#endif
