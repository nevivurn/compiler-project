//--------------------------------------------------------------------------------------------------
/// @brief SnuPL abstract syntax tree (methods related to three-address-code generation)
/// @author Bernhard Egger <bernhard@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2023/09/26 Bernhard Egger created (move TAC generation code from ast.cpp to this file)
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

#include <iostream>
#include <cassert>
#include <cstring>

#include <typeinfo>

#include "ast.h"
using namespace std;


//--------------------------------------------------------------------------------------------------
// CAstNode
//


//------------------------------------------------------------------------------
// CAstScope
//
CTacAddr* CAstScope::ToTac(CCodeBlock *cb)
{
  assert(cb != NULL);

  CAstStatement *s = GetStatementSequence();
  while (s) {
    CTacLabel *next = cb->CreateLabel();
    s->ToTac(cb, next);
    cb->AddInstr(next);
    s = s->GetNext();
  }

  cb->CleanupControlFlow();

  return NULL;
}


//--------------------------------------------------------------------------------------------------
// CAstModule
//


//--------------------------------------------------------------------------------------------------
// CAstProcedure
//


//--------------------------------------------------------------------------------------------------
// CAstType
//


//--------------------------------------------------------------------------------------------------
// CAstStatement
//
CTacAddr* CAstStatement::ToTac(CCodeBlock *cb, CTacLabel *next)
{
  assert(false);
  return NULL;
}


//--------------------------------------------------------------------------------------------------
// CAstStatAssign
//
CTacAddr* CAstStatAssign::ToTac(CCodeBlock *cb, CTacLabel *next)
{
  CTacAddr *rhs = GetRHS()->ToTac(cb);
  CTacAddr *lhs = GetLHS()->ToTac(cb);
  cb->AddInstr(new CTacInstr(opAssign, lhs, rhs));
  cb->AddInstr(new CTacInstr(opGoto, next));
  return NULL;
}


//--------------------------------------------------------------------------------------------------
// CAstStatCall
//
CTacAddr* CAstStatCall::ToTac(CCodeBlock *cb, CTacLabel *next)
{
  GetCall()->ToTac(cb);
  cb->AddInstr(new CTacInstr(opGoto, next));
  return NULL;
}


//--------------------------------------------------------------------------------------------------
// CAstStatReturn
//
CTacAddr* CAstStatReturn::ToTac(CCodeBlock *cb, CTacLabel *next)
{
  CTacAddr *val = NULL;
  if (GetExpression())
     val = GetExpression()->ToTac(cb);
  cb->AddInstr(new CTacInstr(opReturn, NULL, val));
  return NULL;
}


//--------------------------------------------------------------------------------------------------
// CAstStatIf
//
CTacAddr* CAstStatIf::ToTac(CCodeBlock *cb, CTacLabel *next)
{
  CTacLabel *ltrue = cb->CreateLabel("if_true");
  CTacLabel *lfalse = cb->CreateLabel("if_false");

  GetCondition()->ToTac(cb, ltrue, lfalse);

  cb->AddInstr(ltrue);
  CAstStatement *ifbody = GetIfBody();
  while (ifbody) {
    CTacLabel *next = cb->CreateLabel();
    ifbody->ToTac(cb, next);
    cb->AddInstr(next);
    ifbody = ifbody->GetNext();
  }
  cb->AddInstr(new CTacInstr(opGoto, next));

  cb->AddInstr(lfalse);
  CAstStatement *elsebody = GetElseBody();
  while (elsebody) {
    CTacLabel *next = cb->CreateLabel();
    elsebody->ToTac(cb, next);
    cb->AddInstr(next);
    elsebody = elsebody->GetNext();
  }
  cb->AddInstr(new CTacInstr(opGoto, next));

  return NULL;
}


//--------------------------------------------------------------------------------------------------
// CAstStatWhile
//
CTacAddr* CAstStatWhile::ToTac(CCodeBlock *cb, CTacLabel *next)
{
  CTacLabel *lcond = cb->CreateLabel("while_cond");
  CTacLabel *lbody = cb->CreateLabel("while_body");

  cb->AddInstr(lcond);
  GetCondition()->ToTac(cb, lbody, next);
  cb->AddInstr(lbody);

  CAstStatement *body = GetBody();
  while (body) {
    CTacLabel *next = cb->CreateLabel();
    body->ToTac(cb, next);
    cb->AddInstr(next);
    body = body->GetNext();
  }
  cb->AddInstr(new CTacInstr(opGoto, lcond));

  return NULL;
}


//--------------------------------------------------------------------------------------------------
// CAstExpression
//
CTacAddr* CAstExpression::ToTac(CCodeBlock *cb)
{
  return NULL;
}

CTacAddr* CAstExpression::ToTac(CCodeBlock *cb,
                                CTacLabel *ltrue, CTacLabel *lfalse)
{
  return NULL;
}


//--------------------------------------------------------------------------------------------------
// CAstOperation
//


//--------------------------------------------------------------------------------------------------
// CAstBinaryOp
//
CTacAddr* CAstBinaryOp::ToTac(CCodeBlock *cb)
{
  if (GetType()->IsBoolean()) {
    CTacLabel *ltrue = cb->CreateLabel();
    CTacLabel *lfalse = cb->CreateLabel();
    CTacLabel *next = cb->CreateLabel();
    ToTac(cb, ltrue, lfalse);

    CTacTemp *dst = cb->CreateTemp(GetType());

    cb->AddInstr(ltrue);
    cb->AddInstr(new CTacInstr(opAssign, dst, new CTacConst(1, GetType())));
    cb->AddInstr(new CTacInstr(opGoto, next));
    cb->AddInstr(lfalse);
    cb->AddInstr(new CTacInstr(opAssign, dst, new CTacConst(0, GetType())));
    cb->AddInstr(new CTacInstr(opGoto, next));
    cb->AddInstr(next);

    return dst;
  }

  CTacAddr *left = GetLeft()->ToTac(cb);
  CTacAddr *right = GetRight()->ToTac(cb);
  CTacTemp *dst = cb->CreateTemp(GetType());
  cb->AddInstr(new CTacInstr(GetOperation(), dst, left, right));

  return dst;
}

CTacAddr* CAstBinaryOp::ToTac(CCodeBlock *cb, CTacLabel *ltrue, CTacLabel *lfalse)
{
  CAstExpression *left = GetLeft();
  CAstExpression *right = GetRight();
  CTacLabel *next;

  switch (GetOperation()) {
    case opAnd:
      next = cb->CreateLabel();
      left->ToTac(cb, next, lfalse);
      cb->AddInstr(next);
      right->ToTac(cb, ltrue, lfalse);
      break;
    case opOr:
      next = cb->CreateLabel();
      left->ToTac(cb, ltrue, next);
      cb->AddInstr(next);
      right->ToTac(cb, ltrue, lfalse);
      break;

    default:
      CTacAddr *laddr = left->ToTac(cb);
      CTacAddr *raddr = right->ToTac(cb);
      cb->AddInstr(new CTacInstr(GetOperation(), ltrue, laddr, raddr));
      cb->AddInstr(new CTacInstr(opGoto, lfalse));
  }

  return NULL;
}


//--------------------------------------------------------------------------------------------------
// CAstUnaryOp
//
CTacAddr* CAstUnaryOp::ToTac(CCodeBlock *cb)
{
  if (GetType()->IsBoolean()) {
    CTacLabel *ltrue = cb->CreateLabel();
    CTacLabel *lfalse = cb->CreateLabel();
    ToTac(cb, ltrue, lfalse);

    CTacTemp *dst = cb->CreateTemp(GetType());
    CTacLabel *next = cb->CreateLabel();

    cb->AddInstr(ltrue);
    cb->AddInstr(new CTacInstr(opAssign, dst, new CTacConst(1, GetType())));
    cb->AddInstr(new CTacInstr(opGoto, next));
    cb->AddInstr(lfalse);
    cb->AddInstr(new CTacInstr(opAssign, dst, new CTacConst(0, GetType())));
    cb->AddInstr(new CTacInstr(opGoto, next));
    cb->AddInstr(next);

    return dst;
  }

  CTacAddr *val = GetOperand()->ToTac(cb);
  CTacTemp *dst = cb->CreateTemp(GetType());
  cb->AddInstr(new CTacInstr(GetOperation(), dst, val));
  return dst;
}

CTacAddr* CAstUnaryOp::ToTac(CCodeBlock *cb,
                             CTacLabel *ltrue, CTacLabel *lfalse)
{
  // always opNot
  GetOperand()->ToTac(cb, lfalse, ltrue);
  return NULL;
}


//--------------------------------------------------------------------------------------------------
// CAstSpecialOp
//
CTacAddr* CAstSpecialOp::ToTac(CCodeBlock *cb)
{
  CTacAddr *val = GetOperand()->ToTac(cb);
  CTacTemp *dst = cb->CreateTemp(GetType());
  cb->AddInstr(new CTacInstr(opAddress, dst, val));
  return dst;
}


//--------------------------------------------------------------------------------------------------
// CAstFunctionCall
//
CTacAddr* CAstFunctionCall::ToTac(CCodeBlock *cb)
{
  vector<CTacAddr *> params;

  for (unsigned int i = 0; i < GetNArgs(); i++)
    params.push_back(GetArg(i)->ToTac(cb));

  for (unsigned int i = 0; i < GetNArgs(); i++) {
    CTacConst *index = new CTacConst(GetNArgs()-i-1, CTypeManager::Get()->GetInteger());
    cb->AddInstr(new CTacInstr(opParam, index, params[GetNArgs()-i-1]));
  }

  CTacTemp *dst = NULL;
  if (!GetType()->IsNull())
    dst = cb->CreateTemp(GetType());

  CTacAddr *func = new CTacName(GetSymbol());
  cb->AddInstr(new CTacInstr(opCall, dst, func));
  return dst;
}

CTacAddr* CAstFunctionCall::ToTac(CCodeBlock *cb,
                                  CTacLabel *ltrue, CTacLabel *lfalse)
{
  CTacAddr *ret = ToTac(cb);
  cb->AddInstr(new CTacInstr(opEqual, ltrue, ret, new CTacConst(true, ret->GetType())));
  cb->AddInstr(new CTacInstr(opGoto, lfalse));
  return NULL;
}


//--------------------------------------------------------------------------------------------------
// CAstOperand
//


//--------------------------------------------------------------------------------------------------
// CAstDesignator
//
CTacAddr* CAstDesignator::ToTac(CCodeBlock *cb)
{
  return new CTacName(GetSymbol());
}

CTacAddr* CAstDesignator::ToTac(CCodeBlock *cb,
                                CTacLabel *ltrue, CTacLabel *lfalse)
{
  CTacAddr *val = ToTac(cb);
  cb->AddInstr(new CTacInstr(opEqual, ltrue, val, new CTacConst(true, val->GetType())));
  cb->AddInstr(new CTacInstr(opGoto, lfalse));
  return NULL;
}


//--------------------------------------------------------------------------------------------------
// CAstArrayDesignator
//
CTacAddr* CAstArrayDesignator::ToTac(CCodeBlock *cb)
{
  CTypeManager *cm = CTypeManager::Get();
  CToken tok = GetToken();

  // Array pointer
  CAstExpression *array = new CAstDesignator(tok, GetSymbol());
  if (!GetSymbol()->GetDataType()->IsPointer())
    array = new CAstSpecialOp(tok, opAddress, array, NULL);
  //array = new CAstSpecialOp(tok, opCast, array, cm->GetVoidPtr());

  // Number of elements, from the last index forward
  CAstExpression *elem = GetIndex(0);
  for (unsigned int i = 1; i < GetNIndices(); i++) {
    CAstFunctionCall *dim = new CAstFunctionCall(
      tok,
      (const CSymProc *) GetSymbol()->GetSymbolTable()->FindSymbol("DIM"));
    dim->AddArg(array);
    dim->AddArg(new CAstConstant(tok, cm->GetInteger(), i+1));

    elem = new CAstBinaryOp(
      tok, opMul,
      elem, dim
    );
    elem = new CAstBinaryOp(
      tok, opAdd,
      elem, GetIndex(i));
  }

  // Calculate final offset: elem * size + dofs
  CAstExpression *offset = new CAstBinaryOp(
    tok, opMul,
    elem,
    new CAstConstant(tok, cm->GetInteger(), GetType()->GetSize()));

  CAstFunctionCall *dofs = new CAstFunctionCall(
    tok,
    (const CSymProc *) GetSymbol()->GetSymbolTable()->FindSymbol("DOFS"));
  dofs->AddArg(array);

  offset = new CAstBinaryOp(
    tok, opAdd,
    offset,
    dofs);

  CAstExpression *val = new CAstBinaryOp(
    tok, opAdd,
    array, offset);

  CTacName *ref = (CTacName *) val->ToTac(cb);
  return new CTacReference(ref->GetSymbol(), GetSymbol());
}

CTacAddr* CAstArrayDesignator::ToTac(CCodeBlock *cb,
                                     CTacLabel *ltrue, CTacLabel *lfalse)
{
  CTacAddr *val = ToTac(cb);
  cb->AddInstr(new CTacInstr(opEqual, ltrue, val, new CTacConst(true, val->GetType())));
  cb->AddInstr(new CTacInstr(opGoto, lfalse));
  return NULL;
}


//--------------------------------------------------------------------------------------------------
// CAstConstant
//
CTacAddr* CAstConstant::ToTac(CCodeBlock *cb)
{
  return new CTacConst(GetValue(), GetType());
}
CTacAddr* CAstConstant::ToTac(CCodeBlock *cb,
                                CTacLabel *ltrue, CTacLabel *lfalse)
{
  if (GetValue())
    cb->AddInstr(new CTacInstr(opGoto, ltrue));
  else
    cb->AddInstr(new CTacInstr(opGoto, lfalse));
  return NULL;
}


//--------------------------------------------------------------------------------------------------
// CAstStringConstant
//
CTacAddr* CAstStringConstant::ToTac(CCodeBlock *cb)
{
  return new CTacName(_sym);
}

CTacAddr* CAstStringConstant::ToTac(CCodeBlock *cb,
                                CTacLabel *ltrue, CTacLabel *lfalse)
{
  // Should never happen
  assert(false);
  return NULL;
}
