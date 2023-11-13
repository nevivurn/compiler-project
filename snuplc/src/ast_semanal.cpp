//--------------------------------------------------------------------------------------------------
/// @brief SnuPL abstract syntax tree (methods related to semantic analysis)
/// @author Bernhard Egger <bernhard@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2023/09/26 Bernhard Egger created (move semantic analysis code from ast.cpp to this file)
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
const CType* CAstNode::GetType(void) const
{
  return CTypeManager::Get()->GetNull();
}


//------------------------------------------------------------------------------
// CAstScope
//
bool CAstScope::TypeCheck(CToken *t, string *msg) const
{
  bool result = true;

  // TODO(nevi): returns in all branches?

  CAstStatement *s = _statseq;
  while (s) {
    if (!s->TypeCheck(t, msg))
      return false;
    s = s->GetNext();
  }

  for (auto child : _children) {
    if (!child->TypeCheck(t, msg))
      return false;
  }

  return result;
}


//--------------------------------------------------------------------------------------------------
// CAstModule
//


//--------------------------------------------------------------------------------------------------
// CAstProcedure
//
const CType* CAstProcedure::GetType(void) const
{
  return GetSymbol()->GetDataType();
}


//--------------------------------------------------------------------------------------------------
// CAstType
//
const CType* CAstType::GetType(void) const
{
  return _type;
}


//--------------------------------------------------------------------------------------------------
// CAstStatement
//


//--------------------------------------------------------------------------------------------------
// CAstStatAssign
//
bool CAstStatAssign::TypeCheck(CToken *t, string *msg)
{
  CTypeManager *tm = CTypeManager::Get();

  // Typecheck either side
  CAstDesignator *lhs = GetLHS();
  if (!lhs->TypeCheck(t, msg))
    return false;
  CAstExpression *rhs = GetRHS();
  if (!rhs->TypeCheck(t, msg))
    return false;

  // Types must match
  if (!lhs->GetType()->Match(rhs->GetType())) {
    if (t) *t = GetToken();
    if (msg) *msg = "expected matching types";
    return false;
  }

  // Only scalar types
  if (tm->GetBool()->Match(lhs->GetType()) ||
    tm->GetInteger()->Match(lhs->GetType()) ||
    tm->GetChar()->Match(lhs->GetType())) {
    return true;
  }

  if (t) *t = GetToken();
  if (msg) *msg = "expected scalar type";
  return false;
}

const CType* CAstStatAssign::GetType(void) const
{
  return _lhs->GetType();
}


//--------------------------------------------------------------------------------------------------
// CAstStatCall
//
bool CAstStatCall::TypeCheck(CToken *t, string *msg)
{
  return GetCall()->TypeCheck(t, msg);
}


//--------------------------------------------------------------------------------------------------
// CAstStatReturn
//
bool CAstStatReturn::TypeCheck(CToken *t, string *msg)
{
  CTypeManager *tm = CTypeManager::Get();
  const CType *rt = GetScope()->GetType();
  CAstExpression *expr = GetExpression();

  // If return type is null, must return nothing
  if (rt->Match(tm->GetNull())) {
    if (expr != NULL) {
      if (t) *t = expr->GetToken();
      if (msg) *msg = "unexpected return expression";
      return false;
    }
    return true;
  }

  // If not returning anything, return type must be null
  if (expr == NULL) {
    if (t) *t = GetToken();
    if (msg) *msg = "expected return expression";
    return false;
  }

  if (!expr->TypeCheck(t, msg))
    return false;

  // Return types must match
  if (!rt->Match(expr->GetType())) {
    if (t) *t = expr->GetToken();
    if (msg) *msg = "expected matching return type";
    return false;
  }

  return true;
}

const CType* CAstStatReturn::GetType(void) const
{
  const CType *t = NULL;

  if (GetExpression() != NULL) {
    t = GetExpression()->GetType();
  } else {
    t = CTypeManager::Get()->GetNull();
  }

  return t;
}


//--------------------------------------------------------------------------------------------------
// CAstStatIf
//
bool CAstStatIf::TypeCheck(CToken *t, string *msg)
{
  CTypeManager *tm = CTypeManager::Get();

  CAstExpression *cond = GetCondition();
  if (!cond->TypeCheck(t, msg))
    return false;

  // Condition must be bool
  if (!tm->GetBool()->Match(cond->GetType())) {
    if (t) *t = cond->GetToken();
    if (msg) *msg = "expected boolean condition";
    return false;
  }

  // Typecheck body
  CAstStatement *stat = GetIfBody();
  while (stat) {
    if (!stat->TypeCheck(t, msg))
      return false;
    stat = stat->GetNext();
  }

  // Typecheck else
  stat = GetElseBody();
  while (stat) {
    if (!stat->TypeCheck(t, msg))
      return false;
    stat = stat->GetNext();
  }

  return true;
}


//--------------------------------------------------------------------------------------------------
// CAstStatWhile
//
bool CAstStatWhile::TypeCheck(CToken *t, string *msg)
{
  CTypeManager *tm = CTypeManager::Get();

  // Condition must be bool
  CAstExpression *cond = GetCondition();
  if (!cond->TypeCheck(t, msg))
    return false;
  if (!tm->GetBool()->Match(cond->GetType())) {
    if (t) *t = cond->GetToken();
    if (msg) *msg = "expected boolean condition";
    return false;
  }

  // Typecheck body
  CAstStatement *stat = GetBody();
  while (stat) {
    if (!stat->TypeCheck(t, msg))
      return false;
    stat = stat->GetNext();
  }

  return true;
}


//--------------------------------------------------------------------------------------------------
// CAstExpression
//
const CDataInitializer* CAstExpression::Evaluate(void) const
{
  return NULL;
}


//--------------------------------------------------------------------------------------------------
// CAstOperation
//


//--------------------------------------------------------------------------------------------------
// CAstBinaryOp
//
bool CAstBinaryOp::TypeCheck(CToken *t, string *msg)
{
  CTypeManager *tm = CTypeManager::Get();

  // Typecheck operands
  CAstExpression *left = GetLeft();
  CAstExpression *right = GetRight();
  if (!left->TypeCheck(t, msg) || !right->TypeCheck(t, msg))
    return false;

  // All binary operations require matching types
  if (!left->GetType()->Match(right->GetType())) {
    if (t) *t = GetToken();
    if (msg) *msg = "expected matching operand types";
    return false;
  }

  // Rules for each operand
  switch (GetOperation()) {
    case opAdd:
    case opSub:
    case opMul:
    case opDiv:
      // numeric
      if (!tm->GetInteger()->Match(left->GetType())) {
        if (t) *t = GetToken();
        if (msg) *msg = "expected integer type";
        return false;
      }
      break;

    case opAnd:
    case opOr:
      // boolean
      if (!tm->GetBool()->Match(left->GetType())) {
        if (t) *t = GetToken();
        if (msg) *msg = "expected boolean type";
        return false;
      }
      break;

    case opEqual:
    case opNotEqual:
      // any scalar
      if (tm->GetBool()->Match(left->GetType()) ||
          tm->GetInteger()->Match(left->GetType()) ||
          tm->GetChar()->Match(left->GetType()))
        return true;
      if (t) *t = GetToken();
      if (msg) *msg = "expected scalar type";
      return false;

    case opLessThan:
    case opLessEqual:
    case opBiggerThan:
    case opBiggerEqual:
      // int or char
      // any primitives
      if (tm->GetInteger()->Match(left->GetType()) ||
          tm->GetChar()->Match(left->GetType()))
        return true;
      if (t) *t = GetToken();
      if (msg) *msg = "expected integer or character type";
      return false;

    default:
      // Only above three operators are binary
      // TODO(nevi): throw?
      return false;
  }

  return true;
}

const CType* CAstBinaryOp::GetType(void) const
{
  CTypeManager *tm = CTypeManager::Get();

  switch (GetOperation()) {
    case opAdd:
    case opSub:
    case opMul:
    case opDiv:
      if (GetLeft()->GetType()->IsInteger())
        return GetRight()->GetType();
      return GetLeft()->GetType();

    case opAnd:
    case opOr:
    case opEqual:
    case opNotEqual:
    case opLessThan:
    case opLessEqual:
    case opBiggerThan:
    case opBiggerEqual:
      return tm->GetBool();

    default:
      // TODO(nevi): throw?
      return NULL;
  }
}

const CDataInitializer* CAstBinaryOp::Evaluate(void) const
{
  const CDataInitializer *left = GetLeft()->Evaluate();
  const CDataInitializer *right = GetRight()->Evaluate();
  if (!left || !right)
    return NULL;

  // pre-cast all possible values, nonmatching combinations will be NULL
  const CDataInitBoolean *boolLeft = dynamic_cast<const CDataInitBoolean *>(left);
  const CDataInitBoolean *boolRight = dynamic_cast<const CDataInitBoolean *>(right);
  const CDataInitInteger *intLeft = dynamic_cast<const CDataInitInteger *>(left);
  const CDataInitInteger *intRight = dynamic_cast<const CDataInitInteger *>(right);
  const CDataInitLongint *longLeft = dynamic_cast<const CDataInitLongint *>(left);
  const CDataInitLongint *longRight = dynamic_cast<const CDataInitLongint *>(right);
  const CDataInitChar *charLeft = dynamic_cast<const CDataInitChar *>(left);
  const CDataInitChar *charRight = dynamic_cast<const CDataInitChar *>(right);

  long leftv, rightv;
  if (intLeft) leftv = intLeft->GetData();
  if (longLeft) leftv = longLeft->GetData();
  if (intRight) rightv = intRight->GetData();
  if (longRight) rightv = longRight->GetData();

  switch (GetOperation()) {
    case opAdd:
      if (longLeft || longRight)
        return new CDataInitLongint(leftv+rightv);
      return new CDataInitInteger(leftv+rightv);
    case opSub:
      if (longLeft || longRight)
        return new CDataInitLongint(leftv-rightv);
      return new CDataInitInteger(leftv-rightv);
    case opMul:
      if (longLeft || longRight)
        return new CDataInitLongint(leftv*rightv);
      return new CDataInitInteger(leftv*rightv);
    case opDiv:
      if (longLeft || longRight)
        return new CDataInitLongint(leftv/rightv);
      return new CDataInitInteger(leftv/rightv);

    case opAnd:
      return new CDataInitBoolean(boolLeft->GetData() && boolRight->GetData());
    case opOr:
      return new CDataInitBoolean(boolLeft->GetData() || boolRight->GetData());

    case opEqual:
      if (intLeft || longLeft)
        return new CDataInitBoolean(leftv == rightv);
      if (boolLeft)
        return new CDataInitBoolean(boolLeft->GetData() == boolRight->GetData());
      if (charLeft)
        return new CDataInitBoolean(charLeft->GetData() == charRight->GetData());

    case opNotEqual:
      if (intLeft || longLeft)
        return new CDataInitBoolean(leftv != rightv);
      if (boolLeft)
        return new CDataInitBoolean(boolLeft->GetData() != boolRight->GetData());
      if (charLeft)
        return new CDataInitBoolean(charLeft->GetData() != charRight->GetData());
      break;

    case opLessThan:
      if (intLeft || longLeft)
        return new CDataInitBoolean(leftv < rightv);
      if (charLeft)
        return new CDataInitBoolean(charLeft->GetData() < charRight->GetData());
      break;
    case opLessEqual:
      if (intLeft || longLeft)
        return new CDataInitBoolean(leftv <= rightv);
      if (charLeft)
        return new CDataInitBoolean(charLeft->GetData() <= charRight->GetData());
      break;
    case opBiggerThan:
      if (intLeft || longLeft)
        return new CDataInitBoolean(leftv > rightv);
      if (charLeft)
        return new CDataInitBoolean(charLeft->GetData() > charRight->GetData());
      break;
    case opBiggerEqual:
      if (intLeft || longLeft)
        return new CDataInitBoolean(leftv >= rightv);
      if (charLeft)
        return new CDataInitBoolean(charLeft->GetData() >= charRight->GetData());
      break;

    default:
      // Only above three operators are binary
      break;
  }
  // TODO(nevi): throw?
  return NULL;
}


//--------------------------------------------------------------------------------------------------
// CAstUnaryOp
//
bool CAstUnaryOp::TypeCheck(CToken *t, string *msg)
{
  CTypeManager *tm = CTypeManager::Get();

  CAstExpression *expr = GetOperand();
  if (!expr->TypeCheck(t, msg))
    return false;

  switch (GetOperation()) {
    case opNeg:
    case opPos:
      if (!tm->GetInteger()->Match(expr->GetType())) {
        if (t) *t = GetToken();
        if (msg) *msg = "expected integer type";
        return false;
      }
      break;
    case opNot:
      if (!tm->GetBool()->Match(expr->GetType())) {
        if (t) *t = GetToken();
        if (msg) *msg = "expected boolean type";
        return false;
      }
      break;

    default:
      // Only above three operators are unary
      // TODO(nevi): throw?
      return false;
  }

  return true;
}

const CType* CAstUnaryOp::GetType(void) const
{
  return GetOperand()->GetType();
}

const CDataInitializer* CAstUnaryOp::Evaluate(void) const
{
  const CDataInitializer *v = GetOperand()->Evaluate();
  if (!v)
    return NULL;

  switch (GetOperation()) {
    case opPos:
      return v;
    case opNeg:
      if (GetType()->IsInteger()) {
        const CDataInitInteger *tv = dynamic_cast<const CDataInitInteger *>(v);
        return new CDataInitInteger(-tv->GetData());
      }
      if (GetType()->IsLongint()) {
        const CDataInitLongint *tv = dynamic_cast<const CDataInitLongint *>(v);
        return new CDataInitLongint(-tv->GetData());
      }
      break;

    case opNot: {
      const CDataInitBoolean *bv = dynamic_cast<const CDataInitBoolean *>(v);
      return new CDataInitBoolean(!bv->GetData());
    }

    default:
      break;
  }

  // TODO(nevi): throw?
  return NULL;
}


//--------------------------------------------------------------------------------------------------
// CAstSpecialOp
//
bool CAstSpecialOp::TypeCheck(CToken *t, string *msg)
{
  // We only implemented opAddress
  if (GetOperand()->GetType()->IsNull()) {
    if (t) *t = GetToken();
    if (msg) *msg = "taking address of null";
    return false;
  }
  return true;
}

const CType* CAstSpecialOp::GetType(void) const
{
  return (CTypeManager::Get())->GetPointer(GetOperand()->GetType());
}

const CDataInitializer* CAstSpecialOp::Evaluate(void) const
{
  return NULL;
}


//--------------------------------------------------------------------------------------------------
// CAstFunctionCall
//
bool CAstFunctionCall::TypeCheck(CToken *t, string *msg)
{
  // - ensure symbol is procedure
  // - ensure argument count matches params
  // - typecheck arguments
  // - ensure arguments match parameter types

  const CSymProc *sym =dynamic_cast<const CSymProc *>(GetSymbol());
  if (!sym) {
    if (t) *t = GetToken();
    if (msg) *msg = "symbol is not a procedure";
    return false;
  }

  if (GetNArgs() != sym->GetNParams()) {
    if (t) *t = GetToken();
    if (msg) *msg = "mismatched argument count";
    return false;
  }

  for (size_t i = 0; i < GetNArgs(); i++) {
    CAstExpression *arg = GetArg(i);
    const CSymParam *param = sym->GetParam(i);
    if (!arg->TypeCheck(t, msg))
      return false;
    if (!param->GetDataType()->Match(arg->GetType())) {
      if (t) *t = arg->GetToken();
      if (msg) *msg = "procedure call type mistmatch";
      return false;
    }
  }

  return true;
}

const CType* CAstFunctionCall::GetType(void) const
{
  return GetSymbol()->GetDataType();
}


//--------------------------------------------------------------------------------------------------
// CAstOperand
//


//--------------------------------------------------------------------------------------------------
// CAstDesignator
//
bool CAstDesignator::TypeCheck(CToken *t, string *msg)
{
  return true;
}

const CType* CAstDesignator::GetType(void) const
{
  return GetSymbol()->GetDataType();
}

const CDataInitializer* CAstDesignator::Evaluate(void) const
{
  return GetSymbol()->GetData();
}


//--------------------------------------------------------------------------------------------------
// CAstArrayDesignator
//
bool CAstArrayDesignator::TypeCheck(CToken *t, string *msg)
{
  assert(_done);

  const CType *dt = GetSymbol()->GetDataType();
  CTypeManager *tm = CTypeManager::Get();

  // - typecheck index expressions
  // - ensure all indices are numeric
  // - ensure array has at least len(idx) dimensions

  for (auto idx : _idx) {
    if (!dt->IsArray()) {
      if (t) *t = idx->GetToken();
      if (msg) *msg = "not an array";
      return false;
    }

    if (!idx->TypeCheck(t, msg))
      return false;
    if (tm->GetInteger()->Match(idx->GetType())) {
      if (t) *t = idx->GetToken();
      if (msg) *msg = "must be numeric";
    }

    const CArrayType *at = dynamic_cast<const CArrayType*>(dt);
    dt = at->GetInnerType();
  }

  return true;
}

const CType* CAstArrayDesignator::GetType(void) const
{
  const CType *dt = GetSymbol()->GetDataType();

  for (size_t i = 0; i < CAstArrayDesignator::GetNIndices(); i++) {
    const CArrayType *at = dynamic_cast<const CArrayType*>(dt);
    dt = at->GetInnerType();
  }

  return dt;
}


//--------------------------------------------------------------------------------------------------
// CAstConstant
//
bool CAstConstant::TypeCheck(CToken *t, string *msg)
{
  return true;
}

const CType* CAstConstant::GetType(void) const
{
  return _type;
}

const CDataInitializer* CAstConstant::Evaluate(void) const
{
  if (_type->IsBoolean())
    return new CDataInitBoolean(_value);
  if (_type->IsChar())
    return new CDataInitChar(_value);
  if (_type->IsInteger())
    return new CDataInitInteger(_value);
  if (_type->IsLongint())
    return new CDataInitLongint(_value);
  return NULL;
}


//--------------------------------------------------------------------------------------------------
// CAstStringConstant
//
bool CAstStringConstant::TypeCheck(CToken *t, string *msg)
{
  return true;
}

const CType* CAstStringConstant::GetType(void) const
{
  return _type;
}

const CDataInitializer* CAstStringConstant::Evaluate(void) const
{
  return _value;
}
