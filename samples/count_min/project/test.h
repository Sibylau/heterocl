#ifndef TEST_H__
#define TEST_H__
#include <ac_int.h>
#include <ac_channel.h>

void test(
ac_int<32, true> mode[1], 
ac_int<32, true> a[5], 
ac_int<32, true> data_key[10], 
ac_int<32, true> b[5], 
ac_int<32, true> sketch[5][10], 
ac_int<32, true> query_key[5], 
ac_int<32, true> result[5]);

#endif
