// /*!
//  *  Copyright (c) 2017 by Contributors
//  * \file codegen_opencl.h
//  * \brief Generate OpenCL device code.
//  */
// #ifndef TVM_CODEGEN_SYSTEMC_H_
// #define TVM_CODEGEN_SYSTEMC_H_

// #include <fstream>
// #include <tvm/codegen.h>
// #include <tvm/packed_func_ext.h>
// #include <string>
// #include "../merlinc/codeanalys_merlinc.h"
// #include "../codegen_c.h"

// namespace TVM {
// namespace codegen {

// class CodeGenSystemC final : public CodeGenC {
//  public:
//   CodeGenSystemC();
//   void AddFunction(LoweredFunc f, str2tupleMap<std::string, Type> map_arg_type);
//   std::string Finish();

//   // override print thread tag.
//   void InitFuncState(LoweredFunc f) final;
//   void BindThreadIndex(const IterVar& iv) final;  // NOLINT(*)
//   void PrintStorageScope(const std::string& scope, std::ostream& os) final; // NOLINT(*)
//   void PrintStorageSync(const Call* op) final;  // NOLINT(*)
//   void PrintType(Type t, std::ostream& os) final; // NOLINT(*)
//   void PrintVecStore(const Variable* buffer,
//                      Type t, Expr base,
//                      const std::string& value) final;  // NOLINT(*)
//   // the address of load/store
//   void PrintVecAddr(const Variable* buffer, Type t,
//                     Expr base, std::ostream& os);  // NOLINT(*)
//   void GenForStmt(const For* op, std::string pragma, bool before);
//   // overload visitor
//   void VisitExpr_(const Load* op, std::ostream& os) final; // NOLINT(*)
//   void VisitExpr_(const Broadcast* op, std::ostream& os) final; // NOLINT(*)
  
//   void VisitStmt_(const For* op) override;
//   void VisitStmt_(const LetStmt* op) final; // NOLINT(*)
//   void VisitStmt_(const IfThenElse* op) final; // NOLINT(*)
//   void VisitStmt_(const Store* op) final; // NOLINT(*)
//   void VisitStmt_(const KernelDef* op) override;
//   void VisitStmt_(const KernelStmt* op) override;
//  private:
//   bool extern_mode{false};
//   std::unordered_set<std::string> top_args;
// };

// }  // namespace codegen
// }  // namespace TVM

// #endif  // TVM_CODEGEN_SYSTEMC_H_

/*!
 *  Copyright (c) 2018 by Contributors
 * \file codegen_vhls.h
 * \brief Generate Vivado HLS kernel code.
 */
#ifndef TVM_CODEGEN_SYSTEMC_H_
#define TVM_CODEGEN_SYSTEMC_H_

#include <fstream>
#include <tvm/codegen.h>
#include <tvm/packed_func_ext.h>
#include <string>
#include "../hlsc/codegen_hlsc.h"
#include "../merlinc/codeanalys_merlinc.h"

namespace TVM {
namespace codegen {

class CodeGenSystemC final : public CodeGenHLSC {
 public:
  void AddFunction(LoweredFunc f, str2tupleMap<std::string, Type> map_arg_type);
  void PrintType(Type t, std::ostream& os) override;
  
  void VisitExpr_(const Min* op, std::ostream& os) override;
  void VisitExpr_(const Max* op, std::ostream& os) override;
  void VisitExpr_(const GetBit* op, std::ostream& os) override;
  void VisitExpr_(const GetSlice* op, std::ostream& os) override;
  void VisitExpr_(const StreamExpr* op, std::ostream& os) override;
  void VisitExpr_(const Call *op, std::ostream& os) override;
  void VisitExpr_(const Load *op, std::ostream& os) override;

  void VisitStmt_(const Allocate* op) override;
  void VisitStmt_(const Store* op) override;
  void VisitStmt_(const For* op) override;
  void VisitStmt_(const Partition* op) override;
  void VisitStmt_(const Stencil* op) override;
  void VisitStmt_(const StreamStmt* op) override;
  void VisitStmt_(const KernelDef* op) override;
  void VisitStmt_(const KernelStmt* op) override;
  void VisitStmt_(const ExternModule* op) override;

 private:
  std::ofstream soda_header_;
  bool sdsoc_mode{false};
  bool extern_mode{false};
  std::unordered_set<std::string> stream_vars;
};

}  // namespace codegen
}  // namespace TVM

#endif  // TVM_CODEGEN_SYSTEMC_H_
