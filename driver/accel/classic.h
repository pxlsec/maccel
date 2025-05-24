#ifndef __ACCEL_CLASSIC_H_
#define __ACCEL_CLASSIC_H_

#include "../fixedptc.h"
#include "../math.h"
#include <cmath>

struct classic_curve_args {
  fpt accel;
  fpt power;
  fpt offset;
  fpt output_cap;
};

static inline fpt classic_base_fn(fpt x, struct classic_curve_args args) {

  // float accel = fpt_tofloat(args.accel);
  // float power = fpt_tofloat(args.power);
  // float _x = fpt_tofloat(x - args.offset);
  // return std::pow(accel * _x, power) / _x;

  // (ax)^p/x
  // if (x < 1)
  //   return 1; // Hack to fix inaccuraccies with low values of x
  return fpt_div(fpt_pow(fpt_mul(args.accel, x - args.offset), args.power), x);

  // (ax)^p/x with dbg()
  // fpt ax = fpt_mul(args.accel, x - args.offset);
  // dbg("classic: ax                %s", fptoa(ax));
  // fpt axp = fpt_pow(ax, args.power);
  // dbg("classic: axp               %s", fptoa(axp));
  // fpt axpd = fpt_div(axp, x);
  // dbg("classic: axpd              %s", fptoa(axpd));
  // return axpd;

  // float version of (ax)^p/x with dbg()
  // float offset = fpt_tofloat(args.offset);
  // float accel = fpt_tofloat(args.accel);
  // float power = fpt_tofloat(args.power);
  //
  // float ax = x - offset * accel;
  // dbg("classic: ax                %f", ax);
  // float axp = std::pow(ax, power);
  // dbg("classic: axp               %f", axp);
  // float axpd = axp / x;
  // dbg("classic: axpd              %f", axpd);
  // return 0;

  // a^p*x^(p-1)
  // return fpt_mul(
  //     fpt_pow(args.accel, args.power),
  //     fpt_pow(x - args.offset, args.power - FIXEDPT_ONE));

  // a^p*x^p/x
  // return fpt_div(fpt_mul(fpt_pow(args.accel, args.power),
  //                        fpt_pow(x - args.offset, args.power)),
  //                x);
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
  dbg("classic: base_fn sens       %s", fptoa(sens));

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
