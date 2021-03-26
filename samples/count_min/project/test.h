#ifndef TEST_H__
#define TEST_H__
#include <ac_int.h>
#include <ac_channel.h>

void test(
ac_int<32, true> mode[10], 
ac_channel< ac_int<32, true> > &key, 
ac_int<32, true> a[5], 
ac_int<32, true> b[5], 
ac_channel< ac_int<32, true> > &result);

#endif
