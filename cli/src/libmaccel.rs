pub mod fixedptc {
    use std::{
        ffi::{CStr, CString},
        str::FromStr,
    };

    use anyhow::Context;

    use super::c_libmaccel::{self, str_to_fixedpt};

    #[derive(Debug, Default, Clone, Copy, PartialEq)]
    #[repr(transparent)]
    pub struct Fixedpt(pub i64);

    impl From<Fixedpt> for f64 {
        fn from(value: Fixedpt) -> Self {
            unsafe { c_libmaccel::fixedpt_to_float(value) }
        }
    }

    impl From<f64> for Fixedpt {
        fn from(value: f64) -> Self {
            unsafe { c_libmaccel::fixedpt_from_float(value) }
        }
    }

    impl<'a> TryFrom<&'a Fixedpt> for &'a str {
        type Error = anyhow::Error;

        fn try_from(value: &'a Fixedpt) -> Result<Self, Self::Error> {
            unsafe {
                let s = CStr::from_ptr(c_libmaccel::fixedpt_to_str(*value));
                let s = core::str::from_utf8(s.to_bytes())?;
                Ok(s)
            }
        }
    }

    impl FromStr for Fixedpt {
        type Err = anyhow::Error;

        fn from_str(s: &str) -> Result<Self, Self::Err> {
            let cstr = CString::new(s).context("Failed to convert to a C string")?;
            let f = unsafe { str_to_fixedpt(cstr.as_ptr()) };
            Ok(f)
        }
    }
}

pub mod c_libmaccel {
    use super::fixedptc;
    use std::ffi::c_char;

    #[repr(C)]
    pub struct Vector {
        pub x: i64,
        pub y: i64,
    }

    #[repr(C)]
    pub struct AccelArgs {
        pub param_sens_mult: fixedptc::Fixedpt,
        pub param_yx_ratio: fixedptc::Fixedpt,
        pub args: AccelArgsChoice,
    }

    #[repr(C)]
    pub struct LinearCurveArgs {
        pub accel: fixedptc::Fixedpt,
        pub offset: fixedptc::Fixedpt,
        pub output_cap: fixedptc::Fixedpt,
    }

    #[repr(C)]
    pub struct NaturalCurveArgs {
        pub decay_rate: fixedptc::Fixedpt,
        pub offset: fixedptc::Fixedpt,
        pub limit: fixedptc::Fixedpt,
    }

    #[repr(C, u8)]
    #[allow(dead_code)]
    pub enum AccelArgsChoice {
        Linear(LinearCurveArgs),
        Natural(NaturalCurveArgs),
    }

    extern "C" {
        pub fn sensitivity_rs(speed_in: fixedptc::Fixedpt, args: AccelArgs) -> Vector;
    }

    extern "C" {
        pub fn fixedpt_to_str(num: fixedptc::Fixedpt) -> *const c_char;
        pub fn str_to_fixedpt(string: *const c_char) -> fixedptc::Fixedpt;
        pub fn fixedpt_from_float(value: f64) -> fixedptc::Fixedpt;
        pub fn fixedpt_to_float(value: fixedptc::Fixedpt) -> f64;
    }
}
