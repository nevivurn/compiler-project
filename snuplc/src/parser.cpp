//--------------------------------------------------------------------------------------------------
/// @brief SnuPL parser
/// @author Bernhard Egger <bernhard@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2012/09/14 Bernhard Egger created
/// 2013/03/07 Bernhard Egger adapted to SnuPL/0
/// 2014/11/04 Bernhard Egger maintain unary '+' signs in the AST
/// 2016/04/01 Bernhard Egger adapted to SnuPL/1 (this is not a joke)
/// 2019/09/15 Bernhard Egger added support for constant expressions
/// 2020/07/31 Bernhard Egger adapted to SnuPL/2
/// 2020/09/27 Bernhard Egger assignment 2: parser for SnuPL/-1
///
/// @section license_section License
/// Copyright (c) 2012-2023, Computer Systems and Platforms Laboratory, SNU
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

#include <limits.h>
#include <cassert>
#include <errno.h>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <exception>

#include "parser.h"
using namespace std;

//--------------------------------------------------------------------------------------------------
// EBNF of SnuPL/-1
//   module        =  statSequence "."
//   digit         =  "0".."9".
//   letter        =  "a".."z".
//   factOp        =  "*" | "/".
//   termOp        =  "+" | "-".
//   relOp         =  "=" | "#".
//   factor        =  digit | "(" expression ")".
//   term          =  factor { factOp factor }.
//   simpleexpr    =  term { termOp term }.
//   expression    =  simpleexpr [ relOp simpleexpr ].
//   assignment    =  letter ":=" expression.
//   statement     =  assignment.
//   statSequence  =  [ statement { ";" statement } ].
//   whitespace    =  { " " | \n }+.

//--------------------------------------------------------------------------------------------------
// CParser
//
CParser::CParser(CScanner *scanner)
{
  _scanner = scanner;
  _module = NULL;
}

CAstNode* CParser::Parse(void)
{
  _abort = false;

  if (_module != NULL) { delete _module; _module = NULL; }

  try {
    if (_scanner != NULL) _module = module();
  } catch (...) {
    _module = NULL;
  }

  return _module;
}

const CToken* CParser::GetErrorToken(void) const
{
  if (_abort) return &_error_token;
  else return NULL;
}

string CParser::GetErrorMessage(void) const
{
  if (_abort) return _message;
  else return "";
}

void CParser::SetError(CToken t, const string message)
{
  _error_token = t;
  _message = message;
  _abort = true;
  throw message;
}

bool CParser::Consume(EToken type, CToken *token)
{
  if (_abort) return false;

  CToken t = _scanner->Get();

  if (t.GetType() != type) {
    SetError(t, "expected '" + CToken::Name(type) + "', got '" +
             t.GetName() + "'");
  }

  if (token != NULL) *token = t;

  return t.GetType() == type;
}

void CParser::InitSymbolTable(CSymtab *st)
{
  CTypeManager *tm = CTypeManager::Get();
  CSymbol *s;
  CSymProc *f;

  // reserved identifiers
  // such identifiers cannot be used as function/procedure/global variable names

  // 'main' is used to denote the module body in the generated assembly file
  s = new CSymbol("main", stReserved, tm->GetNull());
  st->AddSymbol(s);

  // predefined functions for open arrays
  f = new CSymProc("DIM", tm->GetInteger(), true);
  f->AddParam(new CSymParam(0, "array", tm->GetPointer(tm->GetNull())));
  f->AddParam(new CSymParam(1, "dim", tm->GetInteger()));
  st->AddSymbol(f);

  f = new CSymProc("DOFS", tm->GetInteger(), true);
  f->AddParam(new CSymParam(0, "array", tm->GetPointer(tm->GetNull())));
  st->AddSymbol(f);

  // predefined functions for I/O
  f = new CSymProc("ReadInt", tm->GetInteger(), true);
  st->AddSymbol(f);

  f = new CSymProc("ReadLong", tm->GetLongint(), true);
  st->AddSymbol(f);

  f = new CSymProc("WriteInt", tm->GetNull(), true);
  f->AddParam(new CSymParam(0, "v", tm->GetInteger()));
  st->AddSymbol(f);

  f = new CSymProc("WriteLong", tm->GetNull(), true);
  f->AddParam(new CSymParam(0, "v", tm->GetLongint()));
  st->AddSymbol(f);

  f = new CSymProc("WriteChar", tm->GetNull(), true);
  f->AddParam(new CSymParam(0, "v", tm->GetChar()));
  st->AddSymbol(f);

  f = new CSymProc("WriteStr", tm->GetNull(), true);
  f->AddParam(new CSymParam(0, "v", tm->GetPointer(tm->GetArray(CArrayType::OPEN, tm->GetChar()))));
  st->AddSymbol(f);

  f = new CSymProc("WriteLn", tm->GetNull(), true);
  st->AddSymbol(f);

  // TODO (phase 2)
  // add remaining predefined functions here
}

CAstModule* CParser::module(void)
{
  // module ::= "module" ident ";"
  //            { constDeclaration | varDeclaration | subroutineDecl }
  //            [ "begin" statSequence ] "end" ident ".".

  CToken t;
  EToken tt;

  Consume(tModule);
  Consume(tIdent, &t);
  Consume(tSemicolon);

  CAstModule *m = new CAstModule(t, t.GetValue());
  InitSymbolTable(m->GetSymbolTable());

  tt = _scanner->Peek().GetType();
  while (!_abort && tt != tBegin && tt != tEnd) {
    switch (tt) {
      case tConstDecl:
        constDeclarations(m);
        break;
      case tVarDecl:
        varDeclarations(m);
        break;

      case tFunction: // fallthrough
      case tProcedure:
        subroutineDecl(m); // TODO(nevi): check if we need to do stuff with the procedure
        break;

      default:
        SetError(_scanner->Peek(), "");
        break;
    }
    tt = _scanner->Peek().GetType();
  }

  if (tt == tBegin) {
    Consume(tBegin);
    CAstStatement *statseq = statSequence(m);
    m->SetStatementSequence(statseq);
  }

  Consume(tEnd);
  Consume(tIdent, &t);

  if (t.GetValue() != m->GetName()) {
    SetError(t, "mismatched module closing ident.");
  }

  Consume(tDot);

  return m;
}

void CParser::varDeclarations(CAstScope *s)
{
  //
  // varDeclaration ::= [ "var" varDeclSequence ";" ].
  // varDeclSequence ::= varDecl { ";" varDecl }.
  //
  CToken t;
  CSymtab *st = s->GetSymbolTable();

  Consume(tVarDecl, &t);

  do {
    auto decls = varDecl(s);
    for (string ident : decls.first) {
      CSymbol *sym = s->CreateVar(ident, decls.second);
      if (!st->AddSymbol(sym)) {
        SetError(t, "variable redeclared.");
        break;
      }
    }
    Consume(tSemicolon, &t);

    if (_scanner->Peek().GetType() != tIdent) {
      break;
    }
  } while (!_abort);
}

void CParser::constDeclarations(CAstScope *s)
{
  //
  // constDeclaration ::= [ "const" constDeclSequence ].
  // constDeclSequence ::= constDecl ";" { constDecl ";" }.
  // constDecl ::= varDecl "=" expression.
  //
  CToken t;
  CSymtab *st = s->GetSymbolTable();

  Consume(tConstDecl, &t);

  do {
    auto decls = varDecl(s);
    Consume(tAssign, &t);
    expression(s); // TODO(nevi)

    for (string ident : decls.first) {
      // TODO(nevi)
      CSymbol *sym = s->CreateConst(ident, decls.second, NULL);
      if (!st->AddSymbol(sym)) {
        SetError(t, "constant redeclared.");
        break;
      }
    }
    Consume(tSemicolon, &t);

    if (_scanner->Peek().GetType() != tIdent) {
      break;
    }
  } while (!_abort);
}

pair<vector<string>,const CType*> CParser::varDecl(CAstScope *s)
{
  //
  // varDecl ::= ident { "," ident } ":" type.
  //
  CToken t;
  EToken tt;

  vector<string> idents;

  do {
    Consume(tIdent, &t);
    idents.push_back(t.GetValue());

    tt = _scanner->Peek().GetType();
    if (tt == tComma) {
      Consume(tComma);
    } else if (tt == tColon) {
      Consume(tColon);
      break;
    }
  } while (!_abort);

  const CType *ty = cctype(s);
  return make_pair(idents, ty);
}

CAstProcedure* CParser::subroutineDecl(CAstScope *s)
{
  // subroutineDecl ::= (procedureDecl | functionDecl)
  //                    ( "extern" | subroutineBody ident ) ";".

  CToken t;
  EToken tt = _scanner->Peek().GetType();
  CSymtab *st = s->GetSymbolTable();

  CAstProcedure *n = NULL;

  if (tt == tProcedure) {
    n = procedureDecl(s);
  } else if (tt == tFunction) {
    n = functionDecl(s);
  } else {
    SetError(_scanner->Peek(), "subroutine declaration expected.");
    return n;
  }

  if (_scanner->Peek().GetType() == tExtern) {
    Consume(tExtern);
    Consume(tSemicolon);
    n->GetSymbol()->SetExternal(true);
    return n;
  }

  CAstStatement *body = subroutineBody(n);
  n->SetStatementSequence(body);

  Consume(tIdent, &t);
  if (t.GetValue() != n->GetName()) {
    SetError(t, "mismatched subroutine closing ident.");
  }

  Consume(tSemicolon);
  st->AddSymbol(n->GetSymbol());

  return n;
}

CAstStatement* CParser::subroutineBody(CAstScope *s)
{
  // subroutineBody ::= constDeclaration varDeclaration
  //                    "begin" statSequence "end".

  if (_scanner->Peek().GetType() == tConstDecl) {
    constDeclarations(s);
  }
  if (_scanner->Peek().GetType() == tVarDecl) {
    varDeclarations(s);
  }

  Consume(tBegin);
  CAstStatement* stat = statSequence(s);
  Consume(tEnd);
  return stat;
}

CAstProcedure* CParser::procedureDecl(CAstScope *s)
{
  //
  // procedureDecl ::= "procedure" ident [ formalParam ] ";".
  //
  CToken t;

  Consume(tProcedure);
  Consume(tIdent, &t);

  CSymProc *sym = new CSymProc(t.GetValue(), CTypeManager::Get()->GetNull(), false);
  CAstProcedure *f = new CAstProcedure(t, t.GetValue(), s, sym);
  CSymtab *st = f->GetSymbolTable();

  if (_scanner->Peek().GetType() == tLParens) {
    auto params = formalParam(s);
    for (auto *param : params) {
      sym->AddParam(param);
      st->AddSymbol(param);
    }
  }

  Consume(tSemicolon);

  return f;
}

CAstProcedure* CParser::functionDecl(CAstScope *s)
{
  //
  // functionDecl ::= "function" ident [ formalParam ] ":" type ";".
  //
  CToken t;

  Consume(tFunction);
  Consume(tIdent, &t);

  vector<CSymParam*> params;
  if (_scanner->Peek().GetType() == tLParens) {
    params = formalParam(s);
  }

  Consume(tColon);
  const CType *ty = cctype(s);

  CSymProc *sym = new CSymProc(t.GetValue(), ty, false);
  CAstProcedure *f = new CAstProcedure(t, t.GetValue(), s, sym);
  CSymtab *st = f->GetSymbolTable();

  for (auto *param : params) {
    sym->AddParam(param);
    st->AddSymbol(param);
  }

  Consume(tSemicolon);
  return f;
}

vector<CSymParam*> CParser::formalParam(CAstScope *s)
{
  //
  // formalParam ::= "(" [ varDeclSequence ] ")".
  //
  CToken t;
  Consume(tLParens);

  vector<CSymParam*> params;
  if (_scanner->Peek().GetType() == tRParens) {
    Consume(tRParens);
    return params;
  }

  do {
    auto decls = varDecl(s);

    int index = 0;
    for (string ident : decls.first) {
      params.push_back(new CSymParam(index++, ident, decls.second));
    }

    if (_scanner->Peek().GetType() != tSemicolon) {
      break;
    }
    Consume(tSemicolon, &t);
  } while (!_abort);

  Consume(tRParens);

  return params;
}

CAstStatement* CParser::statSequence(CAstScope *s)
{
  //
  // statSequence ::= [ statement { ";" statement } ].
  // statement ::= assignment.
  //
  // FIRST(statSequence) = { tLetter }
  // FOLLOW(statSequence) = { tDot }
  //
  // FIRST(statement) = { tLetter }
  // FOLLOW(statement) = { tSemicolon, tDot }
  //

  // statement ::= assignment | subroutineCall | ifStatement
  //               | whileStatement | returnStatement.

  // The linking of statement sequences is a bit akward here because
  // we implement statSequence as a loop and not recursively.
  // We keep a 'head' that points to the first statement and is finally
  // returned at the end of the function. Head can be NULL if no statement
  // is present.
  // In the loop, we track the end of the linked list using 'tail' and
  // attach new statements to that tail.
  CAstStatement *head = NULL;

  CToken t = _scanner->Peek();
  EToken tt = t.GetType();
  if (tt == tEnd || tt == tElse) {
    return head;
  }

  CAstStatement *tail = NULL;
  do {
    CAstStatement *st = NULL;

    // For disambiguating ident
    CAstExpression *expr;
    CAstDesignator *ident;
    CAstFunctionCall *call;

    switch (tt) {
      // statement ::= assignment | subroutineCall
      case tIdent:
        expr = identOrCall(s);
        if ((ident = dynamic_cast<CAstDesignator*>(expr))) {
          st = assignment(s, ident);
        } else if ((call = dynamic_cast<CAstFunctionCall*>(expr))) {
          st = new CAstStatCall(t, call);
        } else {
          // TODO(nevi)
        }
        break;

      // statement ::= ifStatement | whileStatement | returnStatement
      case tIf:
        st = ifStatement(s);
        break;
      case tWhile:
        st = whileStatement(s);
        break;
      case tReturn:
        st = returnStatement(s);
        break;

      default:
        SetError(_scanner->Peek(), "statement expected.");
        break;
    }

    assert(st != NULL);
    if (head == NULL) head = st;
    else tail->SetNext(st);
    tail = st;

    if (_scanner->Peek().GetType() != tSemicolon) break;
    Consume(tSemicolon);

    tt = _scanner->Peek().GetType();
  } while (!_abort);

  return head;
}

CAstStatAssign* CParser::assignment(CAstScope *s, CAstDesignator *lhs)
{
  //
  // assignment ::= qualident ":=" expression.
  //

  // We receive the identifier from the caller, it is more convenient to let
  // the caller consume the ident.
  CToken t;
  Consume(tAssign, &t);
  CAstExpression *rhs = expression(s);
  return new CAstStatAssign(t, lhs, rhs);
}

CAstStatIf* CParser::ifStatement(CAstScope *s) {
  //
  // ifStatement ::= "if" "(" expression ")" "then" statSequence
  //                 [ "else" statSequence ] "end".
  //
  CToken t;
  Consume(tIf, &t);

  Consume(tLParens);
  CAstExpression *cond = expression(s);
  Consume(tRParens);

  Consume(tThen);
  CAstStatement *ifBody = statSequence(s);

  CAstStatement *elseBody = NULL;
  if (_scanner->Peek().GetType() == tElse) {
    Consume(tElse);
    elseBody = statSequence(s);
  }

  Consume(tEnd);
  return new CAstStatIf(t, cond, ifBody, elseBody);
}

CAstStatWhile* CParser::whileStatement(CAstScope *s)
{
  //
  // whileStatement ::= "while" "(" expression ")" "do" statSequence "end".
  //
  CToken t;
  Consume(tWhile, &t);

  Consume(tLParens);
  CAstExpression *cond = expression(s);
  Consume(tRParens);

  Consume(tDo);
  CAstStatement *body = statSequence(s);
  Consume(tEnd);

  return new CAstStatWhile(t, cond, body);
}

CAstStatReturn* CParser::returnStatement(CAstScope *s)
{
  //
  // returnStatement ::= "return" [ expression ].
  //
  CToken t;
  Consume(tReturn, &t);

  return new CAstStatReturn(t, s, expression(s));
}

CAstExpression* CParser::expression(CAstScope* s)
{
  //
  // expression ::= simpleexpr [ relOp simpleexpr ].
  //
  CToken t;
  EOperation relop;
  CAstExpression *left = NULL, *right = NULL;

  left = simpleexpr(s);

  if (_scanner->Peek().GetType() == tRelOp) {
    Consume(tRelOp, &t);
    right = simpleexpr(s);

    if (t.GetValue() == "=") {
      relop = opEqual;
    } else if (t.GetValue() == "#") {
      relop = opNotEqual;
    } else if (t.GetValue() == "<") {
      relop = opLessThan;
    } else if (t.GetValue() == "<=") {
      relop = opLessEqual;
    } else if (t.GetValue() == ">") {
      relop = opBiggerThan;
    } else if (t.GetValue() == ">=") {
      relop = opBiggerEqual;
    } else {
        SetError(t, "invalid relation.");
    }
    return new CAstBinaryOp(t, relop, left, right);
  } else {
    return left;
  }
}

CAstExpression* CParser::simpleexpr(CAstScope *s)
{
  //
  // simpleexpr ::= ["+"|"-"] term { termOp term }.
  //
  CAstExpression *n = NULL;

  CToken t;
  if (_scanner->Peek().GetType() == tPlusMinus) {
    Consume(tPlusMinus, &t);
    n = term(s);
    n = new CAstUnaryOp(t, t.GetValue() == "+" ? opPos : opNeg, n);
  } else {
    n = term(s);
  }

  EToken tt = _scanner->Peek().GetType();
  while (tt == tPlusMinus || tt == tOr) {
    CAstExpression *l = n, *r;
    EOperation termop;

    if (tt == tPlusMinus) {
      Consume(tPlusMinus, &t);
      termop = t.GetValue() == "+" ? opAdd : opSub;
    } else {
      Consume(tOr, &t);
      termop = opOr;
    }

    r = term(s);
    n = new CAstBinaryOp(t, termop, l, r);
    tt = _scanner->Peek().GetType();
  }

  return n;
}

CAstExpression* CParser::term(CAstScope *s)
{
  //
  // term ::= factor { factOp factor }.
  //
  CAstExpression *n = factor(s);

  EToken tt = _scanner->Peek().GetType();
  while (tt == tMulDiv || tt == tAnd) {
    CToken t;
    CAstExpression *l = n, *r;
    EOperation factop;

    if (tt == tMulDiv) {
      Consume(tMulDiv, &t);
      factop = t.GetValue() == "*" ? opMul : opDiv;
    } else {
      Consume(tAnd, &t);
      factop = opAnd;
    }

    r = factor(s);
    n = new CAstBinaryOp(t, factop, l, r);
    tt = _scanner->Peek().GetType();
  }

  return n;
}

CAstExpression* CParser::factor(CAstScope *s)
{
  //
  // factor ::= qualident | number | boolean | char | string |
  //            "(" expression ")" | subroutineCall | "!" factor.
  //
  CToken t;
  CAstExpression *n = NULL;

  switch (_scanner->Peek().GetType()) {
    // factor ::= number | boolean | char | string
    case tNumber:
      n = number();
      break;
    case tBoolConst:
      n = boolConst();
      break;
    case tCharConst:
      n = charConst();
      break;
    case tStringConst:
      n = stringConst(s);
      break;

    // factor ::= "(" expression ")"
    case tLParens:
      Consume(tLParens);
      n = expression(s);
      Consume(tRParens);
      break;

    // factor ::= "!" factor
    case tNot:
      Consume(tNot, &t);
      n = factor(s);
      n = new CAstUnaryOp(t, opNot, n);
      break;

    // factor ::= qualident | subroutineCall
    case tIdent:
      n = identOrCall(s);
      break;

    default:
      SetError(_scanner->Peek(), "factor expected.");
      break;
  }

  return n;
}

CAstExpression* CParser::identOrCall(CAstScope *s)
{
  //
  // ident or call ::= ident
  //                   ( { "[" simpleexpr "]" } |
  //                   "(" [ expression {"," expression} ] ")" ).
  //
  CAstDesignator *n = ident(s);

  EToken tt = _scanner->Peek().GetType();
  if (tt == tLBrak) {
    CAstArrayDesignator *nn = new CAstArrayDesignator(_scanner->Peek(), n->GetSymbol());
    while (tt == tLBrak) {
      Consume(tLBrak);
      nn->AddIndex(simpleexpr(s));
      Consume(tRBrak);
      tt = _scanner->Peek().GetType();
    }
    return nn;
  } else if (tt == tLParens) {
    CToken t;
    Consume(tLParens, &t);

    const CSymProc *sym = dynamic_cast<const CSymProc*>(n->GetSymbol());
    if (!sym) {
      SetError(t, "not a procedure.");
      return NULL;
    }
    CAstFunctionCall *nn = new CAstFunctionCall(t, sym);

    if (_scanner->Peek().GetType() == tRParens) {
      Consume(tRParens);
      return nn;
    }

    while (true) {
      nn->AddArg(expression(s));
      switch (_scanner->Peek().GetType()) {
        case tRParens:
          Consume(tRParens);
          return nn;
        case tComma:
          Consume(tComma);
          break;
        default:
          SetError(_scanner->Peek(), "comma or rparens expected.");
          return NULL;
      }
    }
  }

  return n;
}

CAstDesignator* CParser::ident(CAstScope *s)
{
  // ident only handles already-declared identifiers.
  // parsing ident during declaration is handled at each declaration site.
  CToken t;
  CSymtab *st = s->GetSymbolTable();

  Consume(tIdent, &t);

  const CSymbol *sym = st->FindSymbol(t.GetValue(), sLocal);
  if (sym == NULL) {
    SetError(t, "undeclared identifier");
  }

  return new CAstDesignator(t, sym);
}

CAstConstant* CParser::boolConst(void)
{
  CToken t;

  Consume(tBoolConst, &t);

  if (t.GetValue() == "true") {
    return new CAstConstant(t, CTypeManager::Get()->GetBool(), true);
  } else {
    return new CAstConstant(t, CTypeManager::Get()->GetBool(), false);
  }
}

CAstConstant* CParser::charConst(void) {
  CToken t;
  Consume(tCharConst, &t);
  return new CAstConstant(t, CTypeManager::Get()->GetChar(), t.unescape(t.GetValue()).front());
}

CAstStringConstant* CParser::stringConst(CAstScope *s) {
  CToken t;
  Consume(tStringConst, &t);
  return new CAstStringConstant(t, t.unescape(t.GetValue()), s);
}

CAstConstant* CParser::number(void)
{
  CToken t;

  Consume(tNumber, &t);
  string s = t.GetValue();

  if (s.back() == 'L') {
    s.pop_back();

    errno = 0;
    long long v = strtoll(s.c_str(), NULL, 10);
    if (errno != 0) SetError(t, "invalid number.");

    return new CAstConstant(t, CTypeManager::Get()->GetLongint(), v);
  } else {
    errno = 0;
    long long v = strtoll(s.c_str(), NULL, 10);
    if (errno != 0) SetError(t, "invalid number.");

    return new CAstConstant(t, CTypeManager::Get()->GetInteger(), v);
  }

}

const CType* CParser::cctype(CAstScope *s)
{
  CToken t;

  const CType *ct;
  switch (_scanner->Peek().GetType()) {
    case tBoolean:
      Consume(tBoolean, &t);
      ct = CTypeManager::Get()->GetBool();
      break;
    case tChar:
      Consume(tChar, &t);
      ct = CTypeManager::Get()->GetChar();
      break;
    case tInteger:
      Consume(tInteger, &t);
      ct = CTypeManager::Get()->GetInteger();
      break;
    case tLongint:
      Consume(tLongint, &t);
      ct = CTypeManager::Get()->GetLongint();
      break;
    default:
      SetError(_scanner->Peek(), "expected type.");
      break;
  }

  while (_scanner->Peek().GetType() == tLBrak) {
    Consume(tLBrak, &t);

    if (_scanner->Peek().GetType() != tRBrak) {
      // TODO(nevi): evaluate
      simpleexpr(s);
    }
    ct = CTypeManager::Get()->GetArray(CArrayType::OPEN, ct); // TODO(nevi): use evaluated value
    Consume(tRBrak);
  }

  return ct;
}
