#ifndef __ACCEL_CLASSIC_H_
#define __ACCEL_CLASSIC_H_

#include "../fixedptc.h"
#include "../math.h"

struct classic_curve_args {
  fpt accel;
  fpt power;
  fpt offset;
  fpt output_cap;
};

static inline fpt classic_base_fn(fpt x, struct classic_curve_args args) {

  fpt accel_raised = fpt_pow(args.accel, args.power);
  fpt _x = fpt_pow(x - args.offset, args.power);

  return fpt_div(fpt_mul(accel_raised, _x), x);
}

static inline fpt __classic_sens_fun(fpt input_speed,
                                     struct classic_curve_args args) {
  dbg("classic: accel             %s", fptoa(args.accel));
  dbg("classic: power             %s", fptoa(args.power));
  dbg("classic: offset            %s", fptoa(args.offset));
  dbg("classic: output_cap        %s", fptoa(args.output_cap));

  if (input_speed <= args.offset) {
    return FIXEDPT_ONE;
  }

  fpt sens = classic_base_fn(input_speed, args);
  dbg("classic: base_fn sens       %s", fptoa(args.accel));

  fpt sign = FIXEDPT_ONE;
  if (args.output_cap > 0) {
    fpt cap = fpt_sub(args.output_cap, FIXEDPT_ONE);
    if (cap < 0) {
      cap = -cap;
      sign = -sign;
    }
    sens = minsd(sens, cap);
  }

  return fpt_add(FIXEDPT_ONE, fpt_mul(sign, sens));
}
#endif // !__ACCEL_CLASSIC_H_
