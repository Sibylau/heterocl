// HASH:2509525458
#include <ac_int.h>
#include <ac_float.h>
#include <ac_channel.h>
#include <mc_scverify.h>
#include "test.h"

#pragma design top
void CCS_BLOCK(test) (
  ac_int<32, true> mode[10]  , 
  ac_channel< ac_int<32, true> > &key  , 
  ac_int<32, true> a[5]  , 
  ac_int<32, true> b[5]  , 
  ac_channel< ac_int<32, true> > &result) {
        ac_int<32, true> sketch[5][10];
    sketch_x: for (unsigned x = 0; x < 5; ++x) {
      sketch_y: for (unsigned y = 0; y < 10; ++y) {
        sketch[x][y] = 0;
      }
    }
    i: for (unsigned i = 0; i < 10; ++i) {
      if (mode[i] == 1) {
        ac_int<32, true> key_local;
        key_local_x1: for (unsigned x1 = 0; x1 < 1; ++x1) {
          key_local = key.read();
        }
        ac_int<32, true> index[5];
        index_x2: for (unsigned x2 = 0; x2 < 5; ++x2) {
          index[x2] = ((ac_int<32, true>)((((ac_int<65, true>)(((ac_int<64, true>)a[x2]) * ((ac_int<64, true>)key_local))) + ((ac_int<65, true>)b[x2])) % (ac_int<65, true>)10));
        }
        i1: for (unsigned i1 = 0; i1 < 5; ++i1) {
          sketch[((index[i1] / 10) + i1)][(index[i1] % 10)] = (sketch[((index[i1] / 10) + i1)][(index[i1] % 10)] + 1);
        }
        ac_int<32, true> min_val;
        min_val_x3: for (unsigned x3 = 0; x3 < 1; ++x3) {
          min_val = sketch[(index[0] / 10)][(index[0] % 10)];
        }
        i2: for (unsigned i2 = 0; i2 < 4; ++i2) {
          if (sketch[(((index[(i2 + 1)] / 10) + i2) + 1)][(index[(i2 + 1)] % 10)] < min_val) {
            min_val = sketch[(((index[(i2 + 1)] / 10) + i2) + 1)][(index[(i2 + 1)] % 10)];
          }
        }
        result.write(min_val);
      }
      if (mode[i] == 2) {
        ac_int<32, true> key_local_;
        key_local_x4: for (unsigned x4 = 0; x4 < 1; ++x4) {
          key_local_ = key.read();
        }
        ac_int<32, true> index_[5];
        index_x5: for (unsigned x5 = 0; x5 < 5; ++x5) {
          index_[x5] = ((ac_int<32, true>)((((ac_int<65, true>)(((ac_int<64, true>)a[x5]) * ((ac_int<64, true>)key_local_))) + ((ac_int<65, true>)b[x5])) % (ac_int<65, true>)10));
        }
        ac_int<32, true> min_val_;
        min_val_x6: for (unsigned x6 = 0; x6 < 1; ++x6) {
          min_val_ = sketch[(index_[0] / 10)][(index_[0] % 10)];
        }
        i3: for (unsigned i3 = 0; i3 < 4; ++i3) {
          if (sketch[(((index_[(i3 + 1)] / 10) + i3) + 1)][(index_[(i3 + 1)] % 10)] < min_val_) {
            min_val_ = sketch[(((index_[(i3 + 1)] / 10) + i3) + 1)][(index_[(i3 + 1)] % 10)];
          }
        }
        result.write(min_val_);
      }
      if (mode[i] == 3) {
        result.write(-1);
        break;
      }
    }
  }

