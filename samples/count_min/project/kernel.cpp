// HASH:3328172645
#include <ac_int.h>
#include <ac_float.h>
#include <ac_channel.h>
#include <mc_scverify.h>
#include "test.h"

#pragma design top
void CCS_BLOCK(test) (
ac_int<32, true> mode[1], 
ac_int<32, true> a[5], 
ac_int<32, true> data_key[10], 
ac_int<32, true> b[5], 
ac_int<32, true> sketch[5][10], 
ac_int<32, true> query_key[5], 
ac_int<32, true> result[5]) {
        if (mode[0] == 1) {
      i: for (unsigned i = 0; i < 10; ++i) {
        ac_int<32, true> compute0[5];
        compute0_i1: for (unsigned i1 = 0; i1 < 5; ++i1) {
          compute0[i1] = ((ac_int<32, true>)(((ac_int<65, true>)(((ac_int<64, true>)a[i1]) * ((ac_int<64, true>)data_key[i]))) + ((ac_int<65, true>)b[i1])));
        }
        i2: for (unsigned i2 = 0; i2 < 5; ++i2) {
          sketch[((compute0[i2] / 10) + i2)][(compute0[i2] % 10)] = (sketch[((compute0[i2] / 10) + i2)][(compute0[i2] % 10)] + 1);
        }
      }
    }
    if (mode[0] == 2) {
      i3: for (unsigned i3 = 0; i3 < 5; ++i3) {
        ac_int<32, true> compute1[5];
        compute1_i4: for (unsigned i4 = 0; i4 < 5; ++i4) {
          compute1[i4] = ((ac_int<32, true>)(((ac_int<65, true>)(((ac_int<64, true>)a[i4]) * ((ac_int<64, true>)query_key[i3]))) + ((ac_int<65, true>)b[i4])));
        }
        result[i3] = sketch[(compute1[0] / 10)][(compute1[0] % 10)];
        i5: for (unsigned i5 = 0; i5 < 4; ++i5) {
          if (sketch[(((compute1[(i5 + 1)] / 10) + i5) + 1)][(compute1[(i5 + 1)] % 10)] < result[i3]) {
            result[i3] = sketch[(((compute1[(i5 + 1)] / 10) + i5) + 1)][(compute1[(i5 + 1)] % 10)];
          }
        }
      }
    }
    if (mode[0] == 3) {
      i6: for (unsigned i6 = 0; i6 < 10; ++i6) {
        ac_int<32, true> compute2[5];
        compute2_i7: for (unsigned i7 = 0; i7 < 5; ++i7) {
          compute2[i7] = ((ac_int<32, true>)((((ac_int<65, true>)(((ac_int<64, true>)a[i7]) * ((ac_int<64, true>)data_key[i6]))) + ((ac_int<65, true>)b[i7])) % (ac_int<65, true>)10));
        }
        i8: for (unsigned i8 = 0; i8 < 5; ++i8) {
          sketch[((compute2[i8] / 10) + i8)][(compute2[i8] % 10)] = (sketch[((compute2[i8] / 10) + i8)][(compute2[i8] % 10)] + 1);
        }
      }
      i9: for (unsigned i9 = 0; i9 < 5; ++i9) {
        ac_int<32, true> compute3[5];
        compute3_i10: for (unsigned i10 = 0; i10 < 5; ++i10) {
          compute3[i10] = ((ac_int<32, true>)((((ac_int<65, true>)(((ac_int<64, true>)a[i10]) * ((ac_int<64, true>)query_key[i9]))) + ((ac_int<65, true>)b[i10])) % (ac_int<65, true>)10));
        }
        result[i9] = sketch[(compute3[0] / 10)][(compute3[0] % 10)];
        i11: for (unsigned i11 = 0; i11 < 4; ++i11) {
          if (sketch[(((compute3[(i11 + 1)] / 10) + i11) + 1)][(compute3[(i11 + 1)] % 10)] < result[i9]) {
            result[i9] = sketch[(((compute3[(i11 + 1)] / 10) + i11) + 1)][(compute3[(i11 + 1)] % 10)];
          }
        }
      }
    }
}

