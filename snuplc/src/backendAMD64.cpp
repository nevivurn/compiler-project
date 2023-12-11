//--------------------------------------------------------------------------------------------------
/// @brief SnuPL backend
/// @author Bernhard Egger <bernhard@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2020/08/05 Bernhard Egger created
///
/// @section license_section License
/// Copyright (c) 2020-2023, Computer Systems and Platforms Laboratory, SNU
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without modification, are permitted
/// provided that the following conditions are met:
///
/// - Redistributions of source code must retain the above copyright notice, this list of condi-
///   tions and the following disclaimer.
/// - Redistributions in binary form must reproduce the above copyright notice, this list of condi-
///   tions and the following disclaimer in the documentation and/or other materials provided with
///   the distribution.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
/// IMPLIED WARRANTIES,  INCLUDING, BUT NOT LIMITED TO,  THE IMPLIED WARRANTIES OF MERCHANTABILITY
/// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
/// CONTRIBUTORS BE LIABLE FOR ANY DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY, OR CONSE-
/// QUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
/// LOSS OF USE, DATA,  OR PROFITS;  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
/// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)
/// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
/// DAMAGE.
//--------------------------------------------------------------------------------------------------

#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "backendAMD64.h"
using namespace std;

//#define DEBUG


//--------------------------------------------------------------------------------------------------
// AMD64 registers
//
struct {
  char const *n64, *n32, *n16, *n8;     //          Calling convention
} EAMD64RegisterName[NUMREGS] = {       //          Function      Save
  { "rax",  "eax",  "ax",   "al"   },   // rAX       ret.val    caller
  { "rcx",  "ecx",  "cx",   "cl"   },   // rCX       arg #4     caller
  { "rdx",  "edx",  "dx",   "dl"   },   // rDX       arg #3     caller
  { "rbx",  "ebx",  "bx",   "bl"   },   // rBX                  callee
  { "rsi",  "esi",  "si",   NULL   },   // rSI       arg #2     caller
  { "rdi",  "edi",  "di",   NULL   },   // rDI       arg #1     caller
  { "rsp",  "esp",  "sp",   NULL   },   // rSP      stack ptr
  { "rbp",  "ebp",  "bp",   NULL   },   // rBP                  callee
  { "r8",   "r8d",  "r8w",  "r8b"  },   // r8        arg #5     caller
  { "r9",   "r9d",  "r9w",  "r9b"  },   // r9        arg #6     caller
  { "r10",  "r10d", "r10w", "r10b" },   // r10                  caller
  { "r11",  "r11d", "r11w", "r11b" },   // r11                  caller
  { "r12",  "r12d", "r12w", "r12b" },   // r12                  callee
  { "r13",  "r13d", "r13w", "r13b" },   // r13                  callee
  { "r14",  "r14d", "r14w", "r14b" },   // r14                  callee
  { "r15",  "r15d", "r15w", "r15b" },   // r15                  callee
};

//--------------------------------------------------------------------------------------------------
// CBackendAMD64
//
CBackendAMD64::CBackendAMD64(ostream &out)
  : CBackend(out), _curr_scope(NULL)
{
  _ind = string(4, ' ');
}

CBackendAMD64::~CBackendAMD64(void)
{
}

void CBackendAMD64::EmitHeader(void)
{
  _out << "##################################################" << endl
       << "# " << _m->GetName() << endl
       << "#" << endl
       << endl;
}

void CBackendAMD64::EmitCode(void)
{
  _out << _ind << "#-----------------------------------------" << endl
       << _ind << "# text section" << endl
       << _ind << "#" << endl
       << _ind << ".text" << endl
       << _ind << ".align 8" << endl
       << endl
       << _ind << "# entry point" << endl
       << _ind << ".global main" << endl
       << endl
       << _ind << "# external subroutines" << endl;

  // emit external function declarations
  CSymtab *st = _m->GetSymbolTable(); assert(st != NULL);
  vector<CSymbol*> sym = st->GetSymbols();
  auto symit = sym.cbegin();
  while (symit != sym.cend()) {
    CSymbol *s = *symit++;
    if (s->GetSymbolType() == stProcedure) {
      CSymProc *p = dynamic_cast<CSymProc*>(s); assert(p != NULL);

      if (p->IsExternal()) {
        _out << _ind << ".extern " << p->GetName() << endl;
      }
    }
  }

  _out << endl
       << endl;

  // emit scope & subscopes
  for (auto scope : _m->GetSubscopes())
    EmitScope(scope);
  EmitScope(_m);

  _out << _ind << "# end of text section" << endl
       << _ind << "#-----------------------------------------" << endl
       << endl;
}

void CBackendAMD64::EmitData(void)
{
  _out << _ind << "#-----------------------------------------" << endl
       << _ind << "# global data section" << endl
       << _ind << "#" << endl
       << _ind << ".data" << endl
       << _ind << ".align 8" << endl
       << endl;

  EmitGlobalData(_m);

  _out << _ind << "# end of global data section" << endl
       << _ind << "#-----------------------------------------" << endl
       << endl;
}

void CBackendAMD64::EmitFooter(void)
{
  _out << _ind << "# identifier and stack options" << endl
       << _ind << ".ident \"SnuPL/2 (Fall 2023)\"" << endl
       << _ind << ".section .note.GNU-stack,\"\",@progbits" << endl
       << endl
       << _ind << ".end" << endl
       << "##################################################" << endl;
}

void CBackendAMD64::SetScope(CScope *scope)
{
  _curr_scope = scope;
}

CScope* CBackendAMD64::GetScope(void) const
{
  return _curr_scope;
}

void CBackendAMD64::EmitScope(CScope *scope)
{
  assert(scope != NULL);

  string label;

  if (scope->GetParent() == NULL) label = "main";
  else label = scope->GetName();

#ifdef DEBUG
  cout << endl << endl
       << "Encoding '" << label << "'..." << endl
       << endl;
#endif

  // label
  _out << _ind << "# scope " << scope->GetName() << endl
       << label << ":" << endl;

  SetScope(scope);

  // 1. compute the size of locals
  StackFrame paf = {
    .return_address   = 8,        // 1 * 8
    .saved_registers  = 6*8,      // number of saved registers * 8
    .padding          = 0,
    .saved_parameters = 0,
    .local_variables  = 0,
    .argument_build   = 0,
    .size             = 0
  };

  ComputeStackOffsets(scope, paf);

  // 2. emit function prologue
  //    - store saved registrs
  //    - adjust stack pointer to make room for PAF
  //    - save parameters to stack (not necessary if we do register allocation)
  //    - set argument build & local variable area to 0
  //    - initialize local arrays (EmitLocalData)

  _out << _ind << "# prologue" << endl;
  EmitInstruction("pushq", "%rbx", "save callee saved registers");
  EmitInstruction("pushq", "%r12", "");
  EmitInstruction("pushq", "%r13", "");
  EmitInstruction("pushq", "%r14", "");
  EmitInstruction("pushq", "%r15", "");
  EmitInstruction("pushq", "%rbp", "");
  // weird-looking, but this lets us access locals as rsp+offset, params as rbp+fixed+offset
  // plus we get to completely ignore alignment and padding
  EmitInstruction("movq", "%rsp, %rbp", "");
  EmitInstruction("subq", "$" + to_string(paf.size-paf.saved_registers-paf.return_address) + ", %rsp", "");
  EmitInstruction("andq", "$-16, %rsp", "align to 16 bytes"); // reference compiler seems to do it?

  if (auto *sym = dynamic_cast<CSymProc *>(scope->GetDeclaration())) {
    string cmt = "store parameters to stack";
    switch (sym->GetNParams()) {
      default: // fallthrough
      case 6: Store(new CTacName(sym->GetParam(5)), EAMD64Register::r9, cmt); cmt = "";
      case 5: Store(new CTacName(sym->GetParam(4)), EAMD64Register::r8, cmt); cmt = "";
      case 4: Store(new CTacName(sym->GetParam(3)), EAMD64Register::rCX, cmt); cmt = "";
      case 3: Store(new CTacName(sym->GetParam(2)), EAMD64Register::rDX, cmt); cmt = "";
      case 2: Store(new CTacName(sym->GetParam(1)), EAMD64Register::rSI, cmt); cmt = "";
      case 1: Store(new CTacName(sym->GetParam(0)), EAMD64Register::rDI, cmt); cmt = "";
      case 0: break;
    }
  }

  if (int size = paf.local_variables + paf.argument_build) {
    // Zero out all local variables, assume 8-byte alignment
    EmitInstruction("cld", "", "zero out local variables");
    EmitInstruction("xorq", "%rax, %rax", "");
    EmitInstruction("movl", "$" + to_string(size/8) + ", %ecx", "");
    EmitInstruction("movq", "%rsp, %rdi", "");
    EmitInstruction("rep", "stosq", "");
  }

  EmitLocalData(scope);
  _out << endl;

  // 3. emit code
  _out << _ind << "# function body" << endl;
  EmitCodeBlock(scope->GetCodeBlock(), paf);
  _out << endl;

  // 4. emit function epilogue
  _out << _ind << Label("exit") << ":" << endl;
  _out << _ind << "# epilogue" << endl;
  EmitInstruction("leave", "", "");
  EmitInstruction("popq", "%r15", "");
  EmitInstruction("popq", "%r14", "");
  EmitInstruction("popq", "%r13", "");
  EmitInstruction("popq", "%r12", "");
  EmitInstruction("popq", "%rbx", "");
  EmitInstruction("ret", "", "");

  _out << endl;
}

void CBackendAMD64::EmitGlobalData(CScope *scope)
{
  assert(scope != NULL);

  // emit the globals for the current scope
  CSymtab *st = scope->GetSymbolTable();
  assert(st != NULL);

  bool header = false;

  vector<CSymbol*> slist = st->GetSymbols();

  _out << dec;

  size_t size = 0;

  for (size_t i=0; i<slist.size(); i++) {
    CSymbol *s = slist[i];

    // filter out constant symbols
    if (dynamic_cast<CSymConstant*>(s) != NULL) continue;

    const CType *t = s->GetDataType();

    if (s->GetSymbolType() == stGlobal) {
      if (!header) {
        _out << _ind << "# scope: " << scope->GetName() << endl;
        header = true;
      }

      // insert alignment only when necessary
      if ((t->GetAlign() > 1) && (size % t->GetAlign() != 0)) {
        size += t->GetAlign() - size % t->GetAlign();
        _out << setw(4) << " " << ".align "
             << right << setw(3) << t->GetAlign() << endl;
      }

      _out << left << setw(36) << s->GetName() + ":" << "# " << t << endl;

      if (t->IsArray()) {
        const CArrayType *a = dynamic_cast<const CArrayType*>(t);
        assert(a != NULL);
        int dim = a->GetNDim();

        _out << dec << right;

        _out << setw(4) << " "
          << ".long " << setw(4) << dim
          << setw(22) << " " << "#   dimensions"
          << endl;

        for (int d=0; d<dim; d++) {
          assert(a != NULL);

          _out << setw(4) << " "
            << ".long " << setw(4) << a->GetNElem()
            << setw(22) << " " << "#     dimension " << d+1
            << endl;

          a = dynamic_cast<const CArrayType*>(a->GetInnerType());
        }

        if (dim % 2 == 0) {
          // on AMD64, the array data is aligned a 8-byte boundaries
          // i.e., we have to pad 4 bytes if the array dimension is even

          _out << setw(4) << " "
            << ".skip " << setw(4) << 4
            << setw(22) << " " << "#   pad"
            << endl;
        }
      }

      const CDataInitializer *di = s->GetData();
      if (di != NULL) {
        const CDataInitString *sdi = dynamic_cast<const CDataInitString*>(di);
        assert(sdi != NULL);  // only support string data initializers for now

       //cout << "data initializer: " << sdi->GetData() << endl;
        _out << left << setw(4) << " "
          << ".asciz " << '"' << sdi->GetData() << '"' << endl;
      } else {
        _out  << left << setw(4) << " "
          << ".skip " << dec << right << setw(4) << t->GetDataSize()
          << endl;
      }

      size += t->GetSize();
    }
  }

  _out << endl;

  // emit globals in subscopes (necessary if we support static local variables)
  vector<CScope*>::const_iterator sit = scope->GetSubscopes().begin();
  while (sit != scope->GetSubscopes().end()) EmitGlobalData(*sit++);
}

void CBackendAMD64::EmitLocalData(CScope *scope)
{
  assert(scope != NULL);

  // emit the variables for the current scope
  //

  CSymtab *st = scope->GetSymbolTable();
  for (auto *sym : st->GetSymbols()) {
    // non-local variables don't need to be initialized
    if (sym->GetSymbolType() != stLocal) continue;
    // non-arrays have already been zeroed
    if (!sym->GetDataType()->IsArray()) continue;

    const CArrayType *aty = (const CArrayType *) sym->GetDataType();
    unsigned int ndim = aty->GetNDim();
    EmitInstruction("movl", "$" + to_string(ndim) + ", " + Location(sym), "");
    for (unsigned int i = 0; i < ndim; i++) {
      EmitInstruction("movl", "$" + to_string(aty->GetNElem()) + ", " + Location(sym, 4*(i+1)), "");
      aty = (const CArrayType *) aty->GetInnerType();
    }
  }
}

void CBackendAMD64::EmitCodeBlock(CCodeBlock *cb, StackFrame &paf)
{
  assert(cb != NULL);

  const list<CTacInstr*> &instr = cb->GetInstr();
  list<CTacInstr*>::const_iterator it = instr.begin();

  while (it != instr.end()) EmitInstruction(*it++, paf);
}

void CBackendAMD64::EmitInstruction(CTacInstr *i, StackFrame &paf)
{
  assert(i != NULL);

  ostringstream cmt;
  string mnm, mnm2, mnmpf;
  cmt << i;

  EOperation op = i->GetOperation();

  switch (op) {
    // binary operators
    // dst = src1 op src2
    case opAdd:
      Load(EAMD64Register::rAX, i->GetSrc(1), cmt.str());
      Load(EAMD64Register::rBX, i->GetSrc(2), "");
      EmitInstruction("addq", "%rbx, %rax", "");
      Store(i->GetDest(), EAMD64Register::rAX, "");
      break;
    case opSub:
      Load(EAMD64Register::rAX, i->GetSrc(1), cmt.str());
      Load(EAMD64Register::rBX, i->GetSrc(2), "");
      EmitInstruction("subq", "%rbx, %rax", "");
      Store(i->GetDest(), EAMD64Register::rAX, "");
      break;
    case opMul:
      Load(EAMD64Register::rAX, i->GetSrc(1), cmt.str());
      Load(EAMD64Register::rBX, i->GetSrc(2), "");
      EmitInstruction("imulq", "%rbx", "");
      Store(i->GetDest(), EAMD64Register::rAX, "");
      break;
    case opDiv:
      Load(EAMD64Register::rAX, i->GetSrc(1), cmt.str());
      Load(EAMD64Register::rBX, i->GetSrc(2), "");
      EmitInstruction("cdq", "", "");
      EmitInstruction("idivq", "%rbx", "");
      Store(i->GetDest(), EAMD64Register::rAX, "");
      break;
    // opAnd and opOr never appear in TAC

    // unary operators
    // dst = src1
    // opNot never appears in TAC
    case opNeg:
      Load(EAMD64Register::rAX, i->GetSrc(1), cmt.str());
      EmitInstruction("negq", "%rax", "");
      Store(i->GetDest(), EAMD64Register::rAX, "");
      break;
    case opPos:
      // notably, bug on reference compiler
      // fallthrough to opAssign

    // memory operations
    // dst = src1
    case opAssign:
      Load(EAMD64Register::rAX, i->GetSrc(1), cmt.str());
      Store(i->GetDest(), EAMD64Register::rAX, "");
      break;

    // pointer operations
    // dst = &src1
    // dst = *src1
    // only opAddress ever appears in TAC
    case opAddress:
      EmitInstruction("leaq", Operand(i->GetSrc(1)) + ", %rax", cmt.str());
      Store(i->GetDest(), EAMD64Register::rAX, "");
      break;

    // unconditional branching
    // goto dst
    case opGoto:
      EmitInstruction("jmp", Operand(i->GetDest()), cmt.str());
      break;

    // conditional branching
    // if src1 relOp src2 then goto dst
    case opEqual:
    case opNotEqual:
    case opLessThan:
    case opLessEqual:
    case opBiggerThan:
    case opBiggerEqual:
      Load(EAMD64Register::rAX, i->GetSrc(1), cmt.str());
      Load(EAMD64Register::rBX, i->GetSrc(2), "");
      EmitInstruction("cmp", "%rbx, %rax", "");
      EmitInstruction("j" + Condition(op), Operand(i->GetDest()), "");
      break;

    // function call-related operations
    case opCall:
      EmitInstruction("call", Operand(i->GetSrc(1)), cmt.str());
      if (i->GetDest())
        Store(i->GetDest(), EAMD64Register::rAX, "");
      break;
    case opReturn:
      if (i->GetSrc(1))
        Load(EAMD64Register::rAX, i->GetSrc(1), cmt.str());
      EmitInstruction("jmp", Label("exit"), "");
      break;
    case opParam:
      switch (long long int index = ((CTacConst *) i->GetDest())->GetValue()) {
        case 0: Load(EAMD64Register::rDI, i->GetSrc(1), cmt.str()); break;
        case 1: Load(EAMD64Register::rSI, i->GetSrc(1), cmt.str()); break;
        case 2: Load(EAMD64Register::rDX, i->GetSrc(1), cmt.str()); break;
        case 3: Load(EAMD64Register::rCX, i->GetSrc(1), cmt.str()); break;
        case 4: Load(EAMD64Register::r8, i->GetSrc(1), cmt.str()); break;
        case 5: Load(EAMD64Register::r9, i->GetSrc(1), cmt.str()); break;
        default:
          Load(EAMD64Register::rAX, i->GetSrc(1), cmt.str());
          Store(paf.argbuild[index-6], EAMD64Register::rAX, "");
          break;
      }
      break;

    // special
    case opLabel:
      _out << Label(dynamic_cast<CTacLabel*>(i)) << ":" << endl;
      break;

    case opNop:
      EmitInstruction("nop", "", cmt.str());
      break;


    default:
      EmitInstruction("# ???", "not implemented", cmt.str());
  }
}

void CBackendAMD64::EmitInstruction(string mnemonic, string args, string comment)
{
  // goes to some lengths to avoid trailing spaces
  bool hasArgs    = args != "";
  bool hasComment = comment != "";

  _out << left << _ind << setw(hasArgs || hasComment ? 7 : 0)
       << mnemonic
       << (hasArgs || hasComment ? " " : "")
       << setw(hasComment ? 23 : 0) << args;
  if (hasComment) _out << " # " << comment;
  _out << endl;
}

void CBackendAMD64::Load(EAMD64Register dst, CTacAddr *src, string comment)
{
  assert(src != NULL);
  int size = OperandSize(src);
  string mnm = "mov", mod, reg = Reg(dst);

  // set operator modifier based on operand size
  switch (size) {
    case 1: mod = "zbq"; break;
    case 2: mod = "zwq"; break;
    case 4: mod = "slq"; break;
    case 8: mod = "q"; break;
    default: SetError("Data type not supported by this backend.");
  }

  // emit a load instruction
  EmitInstruction(mnm + mod, Operand(src) + ", " + reg, comment);
}

void CBackendAMD64::Store(CTac *dst, EAMD64Register src, string comment)
{
  assert(dst != NULL);
  int size = OperandSize(dst);
  string mod, reg = Reg(src, size);

  // set operator modifier based on operand size
  switch (size) {
    case 1: mod = "b"; break;
    case 2: mod = "w"; break;
    case 4: mod = "l"; break;
    case 8: mod = "q"; break;
    default: SetError("Data type not supported by this backend.");
  }

  // emit a store instruction
  EmitInstruction("mov" + mod, reg + ", " + Operand(dst), comment);
}

string CBackendAMD64::Operand(const CTac *op)
{
  // return a string representing op
  // Hint: references (op of type CTacReference) require special care

  if (auto *c = dynamic_cast<const CTacConst *>(op)) {
    // const
    return Imm(c->GetValue());
  } else if (auto *r = dynamic_cast<const CTacReference *>(op)) {
    // reference
    EmitInstruction("movq", Location(r->GetSymbol(), 0), Reg(EAMD64Register::r15, 8));
    return "(%" + Reg(EAMD64Register::r15, 8) + ")";
  } else if (auto *n = dynamic_cast<const CTacName *>(op)) {
    // named (temporary) variables
    return Location(n->GetSymbol(), 0);
  } else if (auto *l = dynamic_cast<const CTacLabel *>(op)) {
    // label, mainly for jumps
    return Label(l->GetLabel());
  }

  return "?";
}

string CBackendAMD64::Imm(int value) const
{
  ostringstream o;
  o << "$" << dec << value;
  return o.str();
}

string CBackendAMD64::Label(const CTacLabel *label) const
{
  CScope *cs = GetScope();
  assert(cs != NULL);

  ostringstream o;
  o << "l_" << cs->GetName() << "_" << label->GetLabel();
  return o.str();
}

string CBackendAMD64::Label(string label) const
{
  CScope *cs = GetScope();
  assert(cs != NULL);

  return "l_" + cs->GetName() + "_" + label;
}

string CBackendAMD64::Condition(EOperation cond) const
{
  // return condition postfix in dependence of cond
  //
  switch (cond) {
    case opEqual: return "e";
    case opNotEqual: return "ne";
    case opLessThan: return "l";
    case opLessEqual: return "le";
    case opBiggerThan: return "g";
    case opBiggerEqual: return "ge";
    default:
      assert(false);
  }

  return "?";
}

int CBackendAMD64::OperandSize(CTac *t) const
{
  // compute the size for operand t of type CTacName
  // Hint: also here references (incl. references to pointers) and arrays need special care.
  //       Compare your output to that of the reference implementation if you are unsure.
  //

  if (dynamic_cast<CTacConst *>(t)) {
    // pretend all consts are 8 bytes
    return 8;
  }

  if (auto *tref = dynamic_cast<CTacReference *>(t)) {
    // arrays, and only arrays are CTacReference
    // we want the size of a single element
    auto *aty = (const CArrayType *) tref->GetDerefSymbol()->GetDataType();
    return aty->GetBaseType()->GetDataSize();
  }
  CTacAddr *addr = (CTacAddr *) t;
  return addr->GetType()->GetSize();
}

string CBackendAMD64::Location(const CSymbol *s, long long ofs)
{
  // return a string denoting the location of a symbol
  //
  const CStorage *st = s->GetLocation();
  switch (st->GetLocation()) {
    case slUndefined:
      assert(false); // should not happen
    case slMemoryAbs:
      return st->GetBase();
    case slMemoryRel:
      if (st->GetOffset() + ofs)
        return to_string(st->GetOffset() + ofs) + "(%" + st->GetBase() + ")";
      return "(%" + st->GetBase() + ")";
    case slRegister:
      return "%" + st->GetBase();
    case slLabel:
      return st->GetBase() + "(%rip)";
  }

  assert(false); // should not happen
}

string CBackendAMD64::Reg(EAMD64Register reg, int size)
{
  // return the full register name for base register @a base and a given data @a size
  // Hint: this is a simple lookup into EAMD64RegisterName
  auto names = EAMD64RegisterName[reg];
  string rn;
  switch (size) {
    case 1: rn = names.n8; break;
    case 2: rn = names.n16; break;
    case 4: rn = names.n32; break;
    case 8: rn = names.n64; break;
    default: assert(false); // should not happen
  }
  return "%" + rn;
}

void CBackendAMD64::ComputeStackOffsets(CScope *scope, StackFrame &paf)
{
  // compute the location of local variables, temporaries and arguments on the stack
  //
  // Hint: this is the most complicated part of this phase. The reference implementation is
  // ~270 lines long (~100 of which are comments or empty lines)
  //
  // Study the details of the AMD64 PAF, then decide whether you are going to use base pointer-based
  // or stack pointer-based addressing. Depending on that, addressing of variables is relative to
  // the base pointer (rbp) or the stack pointer (rsp).
  // Iterate over all variables in this scope (locals, parameters), compute their locations, then
  // store that location in the symbol (CSymbol->SetLocation).
  //
  // The outputs of this function are
  // - the sizes of the differnt section in the PAF (store in provided 'paf' parameter)
  // - CStorage() class instances assigned to all local variables and parameters that indicate
  //   the location of the variable/parameter on the stack
  //

  // Stack layout:
  // param[...8]
  // param[7]
  // --- previous stack frame ---
  // ret
  // saved registers (6), rbp last <- rbp
  // param[1]
  // param[2...]
  // [padding]
  // local variables
  // arg[...8]
  // arg[7]

  unsigned int maxParams = 0;

  // Handle all non-local symbols. locals "float" between low params and argbuild,
  // so it is more convenient to handle it after argbuild is known
  for (auto sym : scope->GetSymbolTable()->GetSymbols()) {
    switch (sym->GetSymbolType()) {
      case stGlobal: // fallthrough
      case stConstant:
        // globals are rip relative
        sym->SetLocation(new CStorage(EStorageLocation::slLabel, sym->GetName(), 0));
        break;
      case stLocal:
        // locals are rsp relative, ordered in whatever order we encounter them
        sym->SetLocation(new CStorage(EStorageLocation::slMemoryRel, "rsp", paf.local_variables));
        paf.local_variables += sym->GetDataType()->GetSize();
        paf.local_variables += (8-paf.local_variables%8)%8; // just align everything to 8 bytes
        break;
      case stParam: {
        int index = ((CSymParam *) sym)->GetIndex() + 1; // 0->1-indexed
        if (index <= 6) {
          // unspilled params are rbp - offset
          sym->SetLocation(new CStorage(EStorageLocation::slMemoryRel, "rbp", -index*8));
          paf.saved_parameters += 8; // all parameters are padded to 8 bytes
        } else {
          // spilled params are rbp + (7*8) + offset
          // conveniently, index starts at 7
          sym->SetLocation(new CStorage(EStorageLocation::slMemoryRel, "rbp", index*8));
        }
        break;
      }
      case stProcedure: {
        maxParams = max(maxParams, ((CSymProc *) sym)->GetNParams());
        // procedures are absolute
        sym->SetLocation(new CStorage(EStorageLocation::slMemoryAbs, sym->GetName(), 0));
        break;
      }
      case stReserved:
        // reserved (main) is invalid
        break;
    }
  }

  // compute argument_build. argbuild is initialized later, to avoid offsetting locals
  if (maxParams > 6)
    paf.argument_build = (maxParams - 6) * 8;

  // handle locals
  for (auto sym : scope->GetSymbolTable()->GetSymbols()) {
    if (sym->GetSymbolType() != stLocal) continue;

    // locals are rsp relative, ordered in whatever order we encounter them
    sym->SetLocation(new CStorage(EStorageLocation::slMemoryRel, "rsp", paf.argument_build+paf.local_variables));
    paf.local_variables += sym->GetDataType()->GetSize();
    paf.local_variables += (8-paf.local_variables%8)%8; // just align everything to 8 bytes
  }

  // finally, initialize argbuild
  if (maxParams > 6) {
    paf.argbuild = vector<CTacTemp *>(maxParams - 6);
    for (unsigned int i = 0; i < maxParams-6; i++) {
      // make it 8 bytes large, opParam handles proper sizing
      paf.argbuild[i] = scope->CreateTemp(CTypeManager::Get()->GetLongint(), "ab", new CStorage(EStorageLocation::slMemoryRel, "rsp", i*8));
    }
  }

  paf.size =
    paf.return_address + paf.saved_registers +
    paf.padding + paf.saved_parameters +
    paf.local_variables + paf.argument_build;
}
