
#include <sys/ipc.h>
#include <sys/shm.h>

// standard C/C++ headers
#include <cstdio>
#include <cstdlib>
#include <getopt.h>
#include <string>
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <cassert>

// rapidjson headers
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
using namespace rapidjson;

#include "test.h"
#include <mc_scverify.h>
CCS_MAIN(int argc, char **argv) {
  std::cout << "[INFO] Initialize input buffers...\n";

    FILE *f = fopen("../inputs.json", "r");
    char readBuffer[65536];
    FileReadStream is(f, readBuffer, sizeof(readBuffer));

    Document document;
    document.ParseStream(is);
    fclose(f);
    assert(document.HasMember("mode"));
  const Value& mode_d = document["mode"];
  assert(mode_d.IsArray());
  ac_int<32, true> mode[10];
  for (size_t i0 = 0; i0 < 10; i0++) {
    mode[i0] = (mode_d[i0].GetInt());
  }

  assert(document.HasMember("a"));
  const Value& a_d = document["a"];
  assert(a_d.IsArray());
  ac_int<32, true> a[5];
  for (size_t i0 = 0; i0 < 5; i0++) {
    a[i0] = (a_d[i0].GetInt());
  }

  assert(document.HasMember("b"));
  const Value& b_d = document["b"];
  assert(b_d.IsArray());
  ac_int<32, true> b[5];
  for (size_t i0 = 0; i0 < 5; i0++) {
    b[i0] = (b_d[i0].GetInt());
  }

  assert(document.HasMember("key"));
  const Value& key_d = document["key"];
  assert(key_d.IsArray());
  ac_int<32, true> key[10];
  for (size_t i0 = 0; i0 < 10; i0++) {
    key[i0] = (key_d[i0].GetInt());
  }

  assert(document.HasMember("result"));
  const Value& result_d = document["result"];
  assert(result_d.IsArray());
  ac_int<32, true> result[10];
  for (size_t i0 = 0; i0 < 10; i0++) {
    result[i0] = (result_d[i0].GetInt());
  }

  std::cout << "[INFO] Initialize RTE...\n";

  // Compute and kernel call from host
  

static ac_channel< ac_int<32, true> > key_;
for (unsigned int i0 = 0; i0 < 10; i0++ ){
  key_.write( (ac_int<32, true>)key[i0] );
}
static ac_channel< ac_int<32, true> > result_;
CCS_DESIGN(test) (mode, key_, a, b, result_);
for (unsigned int i0 = 0; i0 < 10; i0++ ){
  result[i0] = result_.read();
}

  rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
  document["mode"].Clear();
  rapidjson::Value v_mode(rapidjson::kArrayType);
  for (size_t i0 = 0; i0 < 10; i0++) {
    v_mode.PushBack(rapidjson::Value().SetInt(mode[i0]), allocator);
  }
  document["mode"] = v_mode;
  document["a"].Clear();
  rapidjson::Value v_a(rapidjson::kArrayType);
  for (size_t i0 = 0; i0 < 5; i0++) {
    v_a.PushBack(rapidjson::Value().SetInt(a[i0]), allocator);
  }
  document["a"] = v_a;
  document["b"].Clear();
  rapidjson::Value v_b(rapidjson::kArrayType);
  for (size_t i0 = 0; i0 < 5; i0++) {
    v_b.PushBack(rapidjson::Value().SetInt(b[i0]), allocator);
  }
  document["b"] = v_b;
  document["key"].Clear();
  rapidjson::Value v_key(rapidjson::kArrayType);
  for (size_t i0 = 0; i0 < 10; i0++) {
    v_key.PushBack(rapidjson::Value().SetInt(key[i0]), allocator);
  }
  document["key"] = v_key;
  document["result"].Clear();
  rapidjson::Value v_result(rapidjson::kArrayType);
  for (size_t i0 = 0; i0 < 10; i0++) {
    v_result.PushBack(rapidjson::Value().SetInt(result[i0]), allocator);
  }
  document["result"] = v_result;

    FILE* fp = fopen("../inputs.json", "w"); 
  
    char writeBuffer[65536];
    FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
  
    Writer<FileWriteStream> writer(os);
    document.Accept(writer);
    fclose(fp);

    CCS_RETURN(0);

  }
