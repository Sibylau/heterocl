/*!
 *  Copyright (c) 2017 by Contributors
 * \file codegen_systemc.cc
 */
#include <tvm/runtime/config.h>
#include <tvm/packed_func_ext.h>
#include <tvm/ir_pass.h>
#include <vector>
#include <string>
#include <tuple>
#include <regex>
#include "./codegen_systemc.h"
#include "../../runtime/thread_storage_scope.h"

namespace TVM {
namespace codegen {

struct argInfo {
  std::string     name;
  StorageType     mem_type;
  int             mem_port;
  StreamType      stream_type;
  int             channel_depth;
  bool            is_written;
};

CodeGenSystemC::CodeGenSystemC() {
  restrict_keyword_ = "restrict"; // FIXME: Check if this is useful
  return ;
}

// assign "global" to all array types?
void CodeGenSystemC::InitFuncState(LoweredFunc f) {
  CodeGenC::InitFuncState(f);
  for (Var arg : f->args) {
    if (arg.type().is_handle()) {
      alloc_storage_scope_[arg.get()] = "global";
    }
  }
  return ;
}

void CodeGenSystemC::AddFunction(LoweredFunc f,
        str2tupleMap<std::string, Type> map_arg_type) {
  // Clear previous generated state
  this->InitFuncState(f);

  // Skip the first underscore, so SSA variable starts from _1
  GetUniqueName("_");

  // Register alloc buffer type
  for (const auto & kv : f->handle_data_type) {
    RegisterHandleType(kv.first.get(), kv.second.type());
  }

  // Write header files
  this->stream << "#ifndef DUT_H\n";
  this->stream << "#define DUT_H\n\n";
  this->stream << "#include <cynw_p2p.h>\n";
  this->stream << "#include \"defines.h\"\n";

  // Write entry function name
  this->stream << "SC_MODULE( " << f->name << " ) \n{\n";
  this->stream << "public:\n";
  this->PrintIndent();
  this->stream << "sc_in< bool > clk;\n";
  this->PrintIndent();
  this->stream << "sc_in< bool > rst;\n";

  // Write arguments
  // All args are Cadence Stratus Streaming dtype
  for (size_t i = 0; i < f->args.size(); ++i) {
    Var v = f->args[i];
    // auto attr = f->attributes[i];
    std::string vid = AllocVarID(v.get());
    // if (i != 0) this->stream << ", ";
    if (map_arg_type.find(vid) == map_arg_type.end()) {
      LOG(WARNING) << vid << " type not found\n";
      PrintType(v.type(), this->stream);
      this->stream << ' ' << vid;
    }
    else {
      auto arg = map_arg_type[vid];
      this->stream << "cynw_p2p < ";
      PrintType(v.type(), this->stream);
      this->stream << " >::in ";
      this->stream << std::get<0>(arg);
      this->stream << ";\n";
      top_args.insert(std::get<0>(arg));
    }
  }
  this->stream << "\nSC_CTOR( " << f->name << " ): \n";
  this->stream << "clk ( \"clk\" ), \n";
  this->stream << "rst ( \"rst\" ), \n";
  this->stream << "\n{";
  int func_scope = this->BeginScope();
  this->PrintStmt(f->body);
  this->EndScope(func_scope);
  this->PrintIndent();
  this->stream << "}\n\n";
}

std::string CodeGenSystemC::Finish() {
  return CodeGenC::Finish();
}

void CodeGenSystemC::VisitStmt_(const KernelStmt *op) {
  PrintIndent();
  this->stream << "entered KernelDef\n";
  stream << op->name << "(";

  // Extract annotation values
  std::vector<argInfo> args_info;
  for (size_t k = 0; k < op->annotate_keys.size(); k++) {
    auto key = op->annotate_values[k].as<StringImm>(); CHECK(key);
  }
  // Print kernel function arguments
  for (size_t i = 0; i < op->args.size(); i++) {
    std::string arg_name = PrintExpr(op->args[i]);
    stream << arg_name;
    if (i < op->args.size() - 1) stream << ", ";
  }
  stream << ");\n";
}

void CodeGenSystemC::VisitStmt_(const KernelDef* op) {
  LoweredFunc f;
  // save func states
  this->stream << "entered KernelDef\n";
  CodeGenC::SaveFuncState(f);
  CodeGenC::InitFuncState(f);
  std::ostringstream save;
  std::ostringstream pragma;
  save << this->stream.str();
  this->stream.str("");
  this->stream.clear();

  // skip the first underscore
  GetUniqueName("_");
  // add to alloc buffer : type.
  for (const auto & k : op->args) {
    RegisterHandleType(k.get(), k.get()->type);
  }

  // collect argument information
  std::vector<argInfo> args_info;
  bool is_kernel_func = false;
  for (size_t i = 0; i < op->attributes.size(); i++) {
    auto info = op->attributes[i];
    CHECK(info.size() >=2);
    auto arg_name = info[0].as<StringImm>()->value;
    for (size_t i = 0; i < arg_name.size(); ++i) {
      if (arg_name[i] == '.') arg_name[i] = '_';
    }

    if (info.size() > 2) { 
        is_kernel_func = true;
        CHECK(info.size() == 6);
        auto mem_dev = static_cast<StorageType>(info[1].as<IntImm>()->value);
        int mem_port = info[2].as<IntImm>()->value;
        auto stream_type = static_cast<StreamType>(info[3].as<IntImm>()->value);
        int channel_depth = info[4].as<IntImm>()->value;
        bool is_written = info[5].as<IntImm>()->value == 1 ? true : false;
        argInfo arg_info = {arg_name, mem_dev, mem_port, stream_type, channel_depth, is_written};
        args_info.push_back(arg_info);

    } else {
        bool is_written = info[1].as<IntImm>()->value == 1 ? true : false;
        argInfo arg_info;
        arg_info.is_written = is_written;
        args_info.push_back(arg_info);
    }
  }

  // print top-level kernel function
  if (is_kernel_func) {

    int extern_scope = -1;
    if (extern_mode) {
      extern_scope  = BeginScope();
      stream << "extern \"C\" {\n";
    }

    stream << "void " << op->name << "(";
    for (size_t i = 0; i < op->args.size(); ++i) {
      VarExpr v = op->args[i];
      var_shape_map_[v.get()] = op->arg_shapes[i];
      std::string vid = AllocVarID(v.get());

      if (i != 0) stream << ", ";
      std::string str = PrintExpr(op->arg_types[i]);
      Type type = String2Type(str);

      // pass-by-value arguments
      if (var_shape_map_[v.get()].size() == 1 &&
          var_shape_map_[v.get()][0].as<IntImm>()->value == 1) {
        PrintType(type, stream);
        this->stream << " " << vid;

      // pass-by-pointer arguments
      } else {
        CHECK(args_info.size() > i) << i << ":" << args_info.size();
        auto info = args_info[i];

        if (info.stream_type == StreamType::FIFO) {
          auto bits = type.bits();
          if (decl_stream.str().find("typedef qdma_axis<" + 
                  std::to_string(bits)) == std::string::npos) {
            decl_stream << "typedef qdma_axis<" << bits 
                        << ", 0, 0, 0> pkt_b" << bits << ";\n";
          }
          stream << "hls::stream<pkt_b" << bits << "> &" << vid;

        // Memory-mapped pointers
        } else {
          PrintType(type, stream);
          auto size = var_shape_map_[v.get()];
          stream << " " << vid;
          for (auto& s : size) {
            stream << "[" << s << "]";
          }
        }
      }
    }
    stream << ") {\n";

    if (extern_mode) {
      // Port-level protocol interface
      CHECK(op->args.size() == op->args.size());
      for (size_t i = 0; i < op->args.size(); i++) {
        if (op->arg_shapes[i].size() == 1 &&
            op->arg_shapes[i][0].as<IntImm>()->value == 1) {
          continue;
        } else {
          PrintIndent();
          auto info = args_info[i];

          if (info.stream_type == StreamType::FIFO) {
            stream << "#pragma HLS INTERFACE axis port="
                   << info.name << "\n";
          } else {
            stream << "#pragma HLS INTERFACE m_axi port="
                   << info.name << " "
                   << "offset=slave bundle=gmem" << info.mem_port << "\n";
          }
        }
      }

      // Block-level control interface 
      for (size_t i = 0; i < op->args.size(); i++) {
        auto info = args_info[i];
        if (info.stream_type == StreamType::FIFO) continue;
        PrintIndent();
        stream << "#pragma HLS INTERFACE s_axilite port="
               << info.name << " "
               << "bundle=control\n";
      }
      PrintIndent();
      stream << "#pragma HLS INTERFACE s_axilite"
             << " port=return bundle=control\n";
    }

    // function body
    int func_scope = BeginScope();
    range_ = CollectIterRange(op->body);
    PrintStmt(op->body);

    EndScope(func_scope);
    PrintIndent();
    stream << "}\n";

    if (extern_mode) {
        stream << "}\n\n";
        EndScope(extern_scope);
    }

  // Non-top kernel function 
  } else {

    auto const_size = [&](Array<Expr> shape) -> int32_t {
      int32_t res = 1;
      for (auto s : shape) {
          CHECK(s.as<IntImm>());
          auto v = s.as<IntImm>()->value;
          res = res * v;
      }
      return res;
    };
    std::ostringstream func_os;
    func_os << "static void " << op->name << "(";
    for (size_t i = 0; i < op->args.size(); ++i) {
      VarExpr v = op->args[i];
      var_shape_map_[v.get()] = op->arg_shapes[i];

      int32_t constant_size = const_size(op->arg_shapes[i]);
      CHECK_GT(constant_size, 0)
          << "Input arg size must be greater than 0...";
      buf_length_map_[v.get()] = constant_size;
      std::string vid = AllocVarID(v.get());
      if (i != 0) func_os << ", ";
      std::string str = PrintExpr(op->arg_types[i]);
      Type type = String2Type(str);

      // Scalar input
      CHECK_GT(op->arg_shapes[i].size(), 0);
      if (op->arg_shapes[i].size() == 1) {
        auto dim = op->arg_shapes[i][0].as<IntImm>();
        CHECK(dim);
        if (dim->value == 1 || dim->value == 0) {
            PrintType(type, func_os);
            auto info = args_info[i];
            if (info.is_written) func_os << "&";
            func_os << " " << vid;
            continue;
        }
      }

      if (op->arg_shapes[i].size() > 0) {
        auto shape = op->arg_shapes[i]; 
        PrintType(type, func_os);
        func_os << " " << vid;
        func_os << "[";
        for (size_t k = 0; k < shape.size(); k++) {
          if (k != shape.size() - 1) func_os << "][";
          func_os << shape[k];
        }
        func_os << "]";
      }
    }
    decl_stream << func_os.str() << ");\n";
    stream << func_os.str() << ") {\n";
    
    PrintIndent();
    stream << "#pragma HLS inline off\n";

    // function body
    int func_scope = BeginScope();
    range_ = CollectIterRange(op->body);
    PrintStmt(op->body);
    EndScope(func_scope);
    PrintIndent();
    stream << "}\n\n";

  }

  // restore default stream
  module_stream << this->stream.str();
  this->stream.str("");
  this->stream.clear();
  this->stream << save.str();
  RestoreFuncState(f);
}

void CodeGenSystemC::BindThreadIndex(const IterVar& iv) {
  LOG(FATAL) << "Merlin doesn't support thread binding";
  return ;
}

void CodeGenSystemC::PrintType(Type t, std::ostream& os) {  // NOLINT(*)
  int lanes = t.lanes();
  if (t.is_uint()) {
    os << "sc_uint <";
    os << t.bits();
    os << ">";
  }
  else if (t.is_int()) {
    os << "sc_int <";
    os << t.bits();
    os << ">";
  }
  else
    LOG(ERROR) << "Unsupported Data Type " << t << "!" ;
  if (lanes > 1)
    LOG(ERROR) << "SystemC does not support array interface" ;
  return;
  
  // if (t.is_handle()) {
  //   //LOG(FATAL) << "The buffer shouldn't call PrintType for printing type";
  //   os << "void*";
  //   return ;
  // }
  // bool fail = false;
  // if (t.is_float()) {
  //   switch (t.bits()) {
  //     case 16: os << "half"; break;
  //     case 32: os << "float"; break;
  //     case 64: os << "double"; break;
  //     case 128: os << "double double"; break;
  //     default: fail = true; break;
  //   }
  //   if (!fail && lanes == 1) return;
  //   if (!fail && (lanes >= 2 && lanes <= 16)) {
  //     os << lanes; return;
  //   }
  // } else if (t.is_uint() || t.is_int()) {
  //   if (t.is_uint()) {
  //     os << "unsigned ";
  //   }
  //   if (t.bits() == 8 && t.lanes() == 4) {
  //     // directly 4 8 bit int in integer.
  //     os << "int"; return;
  //   }

  //   int target_bit = 1;
  //   while (target_bit < t.bits())
  //     target_bit <<= 1;

  //   switch (target_bit) {
  //     case 1: os << "int"; break;
  //     case 2: os << "char"; break;
  //     case 4: os << "char"; break;
  //     case 8: os << "char"; break;
  //     case 16: os << "short"; break;
  //     case 32: os << "int"; break;
  //     case 64: os << "long"; break;
  //     case 128: os << "long"; break; // FIXME: Should use long long
  //     default: fail = true; break;
  //   }
  //   if (!fail && lanes == 1) return;
  //   // FIXME: Not yet support multiple lanes
  //   //if (!fail && (lanes >= 2 && lanes <= 16)) {
  //   //  os << lanes; return;
  //   //}
  // }
  // os << t;
  // LOG(WARNING) << "Cannot convert type " << t ;
  return ;
}

void CodeGenSystemC::PrintVecAddr(const Variable* buffer, Type t,
                                 Expr base, std::ostream& os) {  // NOLINT(*)
  // FIXME: What's this node for?
  if (!HandleTypeMatch(buffer, t.element_of())) {
    os << '(';
    auto it = alloc_storage_scope_.find(buffer);
    if (it != alloc_storage_scope_.end()) {
      PrintStorageScope(it->second, os);
    }
    os << ' ';
    PrintType(t.element_of(), os);
    os << "*)";
  }
  os << GetVarID(buffer) << " + ";
  PrintExpr(base, os);
  return ;
}

void CodeGenSystemC::PrintVecStore(const Variable* buffer,
                                  Type t, Expr base,
                                  const std::string& value) {
  // FIXME: What's this node for?
  this->PrintIndent();
  stream << "vstore" << t.lanes() << "(" << value << ", 0, ";
  PrintVecAddr(buffer, t, base, stream);
  stream << ");\n";
  return ;
}

void CodeGenSystemC::PrintStorageSync(const Call* op) {
  const std::string& sync = op->args[0].as<StringImm>()->value;
  if (sync == "warp") {
    LOG(FATAL) << "warp sync not supported in Merlin";
  } else if (sync == "shared") {
    LOG(FATAL) << "shared sync not supported in Merlin";
  } else if (sync == "global") {
    LOG(FATAL) << "global sync not supported in Merlin";
  }
  return ;
}

void CodeGenSystemC::VisitExpr_(const Load* op, std::ostream& os) {
  std::string vid = GetVarID(op->buffer_var.get());
  // TODO: find a betetr way to track streaming channels 
  if (top_args.find(vid) != top_args.end()) {
    PrintIndent(); 
    stream << vid << "_temp = " << vid << ".read_nb();\n";
    os << vid << "_temp.get_data()";
  } else {
    CodeGenC::VisitExpr_(op, os);
  }
}

void CodeGenSystemC::VisitStmt_(const Store* op) {
  std::string vid = GetVarID(op->buffer_var.get());
  if (top_args.find(vid) != top_args.end()) {
    auto value = PrintExpr(op->value);
    auto bits = handle_data_type_[op->buffer_var.get()].bits();
    PrintIndent(); 
    stream << "pkt_b" << bits << " " << vid <<  "_temp;\n";
    PrintIndent(); 
    stream << vid <<  "_temp.set_data(" << value << ");\n";
    PrintIndent(); 
    stream << vid <<  "_temp.set_keep(-1);\n";
    PrintIndent(); 
    stream << vid << ".write(" << vid << "_temp);\n";
    return;
  }

  // handle SetSlice
  if (const SetSlice* ss = op->value.as<SetSlice>()) {
    Type t = op->value.type();
    Expr new_index_left = ir::Simplify(ss->index_left - 1);
    std::string ref = this->GetBufferRef(t, op->buffer_var.get(), op->index);
    std::string rhs = PrintExpr(ss->value);
    PrintIndent();
    this->stream << ref
                 << "(" << PrintExpr(new_index_left) << ", " << PrintExpr(ss->index_right)
                 << ") = " << rhs << ";\n";
  } else if (const SetBit* sb = op->value.as<SetBit>()) {
    Type t = op->value.type();
    std::string ref = this->GetBufferRef(t, op->buffer_var.get(), op->index);
    PrintIndent();
    this->stream << ref
                 << "[" << PrintExpr(sb->index)
                 << "] = " << PrintExpr(sb->value) << ";\n";
  } else if (auto expr_op = op->value.as<Select>()) {
    Type t = op->value.type();
    std::string ref = this->GetBufferRef(t, op->buffer_var.get(), op->index);
    PrintIndent();
    this->stream << "if (" << PrintExpr(expr_op->condition) << ") { \n";
    PrintIndent();
    this->stream << "  " << ref 
        << " = " << PrintExpr(expr_op->true_value) << ";\n";
    PrintIndent();
    this->stream << "} else { \n";
    PrintIndent();
    this->stream << "  " << ref 
        << " = " << PrintExpr(expr_op->false_value) << ";\n";
    PrintIndent();
    this->stream << "}\n";
  } else {
    CodeGenC::VisitStmt_(op);
  }
}

void CodeGenSystemC::PrintStorageScope(
    const std::string& scope, std::ostream& os) { // NOLINT(*)
    return ;
}

void CodeGenSystemC::VisitExpr_(const Broadcast* op, std::ostream& os) { // NOLINT(*)
  std::string v = PrintExpr(op->value);
  os << "((";
  PrintType(op->type, os);
  os << ")(";
  for (int i = 0; i < op->lanes; ++i) {
    if (i != 0) os << ", ";
    os << v;
  }
  os << "))";
  return ;
}

void CodeGenSystemC::VisitStmt_(const LetStmt* op) {
  std::string value = PrintExpr(op->value);
  // Skip the argument retrieving assign statement
  std::string vid = AllocVarID(op->var.get());
  if (op->var.type() != Handle() &&
      value.find("TVMArray") == std::string::npos &&
      value.find("arg") != 0) {
    PrintIndent();
    PrintType(op->var.type(), this->stream);
    this->stream << ' '
                 << vid
                 << " = " << value << ";\n";
  }
  PrintStmt(op->body);
}

void CodeGenSystemC::GenForStmt(const For* op, std::string pragma, bool before) {
  std::string extent = PrintExpr(op->extent);
  std::string vid = AllocVarID(op->loop_var.get());
  CHECK(is_zero(op->min));
  if (before && pragma.length() > 0) {
    PrintIndent();
    stream << pragma;
  }
  PrintIndent();

  // print loop labels
  bool loop_stage_name = false;
  for (unsigned int i = 0; i < op->annotate_keys.size(); i++) {
    if (auto str = op->annotate_keys[i].as<StringImm>()) {
      if (str->value == "stage_name") {
        loop_stage_name = true;
        auto label = op->annotate_values[i].as<StringImm>();
        std::string output_label;
        if (label->value == "") {
          output_label = vid;
        } else {
          output_label = label->value + "_" + vid;
        }
        for (size_t i = 0; i < output_label.size(); ++i) {
          if (output_label[i] == '.') output_label[i] = '_';
        }
        stream << output_label << ": ";
        break;
      }
    }
  }
  if (!loop_stage_name)
    stream << vid << ": ";

  stream << "for (";
  PrintType(op->loop_var.type(), stream);
  stream << ' ' << vid << " = 0; "
            << vid << " < " << extent
            << "; ++" << vid << ") {\n";
  if (!before && pragma.length() > 0) {
    PrintIndent();
    stream << pragma;
  }
  int for_scope = BeginScope();
  PrintStmt(op->body);
  this->EndScope(for_scope);
  PrintIndent();
  stream << "}\n";
}

void CodeGenSystemC::VisitStmt_(const For* op) {
  std::ostringstream os;

  Stmt stmt = op->body;
  while (const For* for_op = stmt.as<For>())
    stmt = for_op->body;

  // Skip for-loops for all 0 assignment 
  if (auto st = stmt.as<Store>()) {
    auto value = st->value;
    if (auto c = value.as<Cast>()) value = c->value;
    if (auto v = value.as<IntImm>()) {
      if (v->value == 0) return;
    } else if (auto v = value.as<FloatImm>()) {
      if (v->value == 0) return;
    } else if (auto v = value.as<UIntImm>()) {
      if (v->value == 0) return;
    }
  }

  if (op->for_type == ForType::Unrolled) {
    int unroll_factor = 0, i = 0;
    for (auto key : op->annotate_keys) {
      if (auto str = key.as<StringImm>()) {
        auto factor = op->annotate_values[i].as<IntImm>();
        if (str->value == "factor" && factor != nullptr && factor->value > 1) {
          unroll_factor = factor->value;
          break;
        }
      }
      i++;
    }
    os << "#pragma HLS unroll";
    if (unroll_factor > 0) os << " factor=" << unroll_factor << "\n";
    else                   os << "\n";
  }
  else if (op->for_type == ForType::Pipelined) {
    int II = 0, i = 0;
    for (auto key : op->annotate_keys) {
      if (auto str = key.as<StringImm>()) {
        auto initiation_interval = op->annotate_values[i].as<IntImm>();
        if (str->value == "initiation_interval" &&
            initiation_interval != nullptr &&
            initiation_interval->value > 1) {
          II = initiation_interval->value;
          break;
        }
      }
      i++;
    }
    os << "#pragma HLS pipeline";
    if (II > 0) os << " II=" << II << "\n";
    else        os << "\n";
  }
  GenForStmt(op, os.str(), false);
}

void CodeGenSystemC::VisitStmt_(const IfThenElse* op) {
  std::string cond = PrintExpr(op->condition);

  // Skip the buffer data checking
  if (std::regex_match(cond, std::regex("!\\((arg)(.+)(== NULL)\\)")))
      return ;

  PrintIndent();
  if (cond[0] == '(' && cond[cond.length() - 1] == ')') {
    stream << "if " << cond << " {\n";
  } else {
    stream << "if (" << cond << ") {\n";
  }
  int then_scope = BeginScope();
  PrintStmt(op->then_case);
  this->EndScope(then_scope);

  if (op->else_case.defined()) {
    PrintIndent();
    stream << "} else {\n";
    int else_scope = BeginScope();
    PrintStmt(op->else_case);
    this->EndScope(else_scope);
  }
  PrintIndent();
  stream << "}\n";
}
}  // namespace codegen
}  // namespace TVM