//--------------------------------------------------------------------------------------------------
/// @brief SnuPL parser
/// @author Bernhard Egger <bernhard@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2012/09/14 Bernhard Egger created
/// 2013/03/07 Bernhard Egger adapted to SnuPL/0
/// 2016/03/09 Bernhard Egger adapted to SnuPL/1
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

#ifndef __SnuPL_PARSER_H__
#define __SnuPL_PARSER_H__

#include "scanner.h"
#include "symtab.h"
#include "ast.h"


//--------------------------------------------------------------------------------------------------
/// @brief operational modes for type scanning
///
enum EType {
  mConstant=0,                      ///< constant definition
  mVariable,                        ///< variable definition
  mFormalPar,                       ///< formal parameter definition
};

//--------------------------------------------------------------------------------------------------
/// @brief parser
///
/// parses a module
///
class CParser {
  public:
    /// @brief constructor
    ///
    /// @param scanner  CScanner from which the input stream is read
    CParser(CScanner *scanner);

    /// @brief parse a module
    /// @retval CAstNode program node
    CAstNode* Parse(void);

    /// @name error handling
    ///@{

    /// @brief indicates whether there was an error while parsing the source
    /// @retval true if the parser detected an error
    /// @retval false otherwise
    bool HasError(void) const { return _abort; };

    /// @brief returns the token that caused the error
    /// @retval CToken containing the error token
    const CToken* GetErrorToken(void) const;

    /// @brief returns a human-readable error message
    /// @retval error message
    string GetErrorMessage(void) const;
    ///@}

  private:
    /// @brief sets the token causing a parse error along with a message
    /// @param t token causing the error
    /// @param message human-readable error message
    void SetError(CToken t, const string message);

    /// @brief consume a token given type and optionally store the token
    /// @param type expected token type
    /// @param token If not null, the consumed token is stored in 'token'
    /// @retval true if a token has been consumed
    /// @retval false otherwise
    bool Consume(EToken type, CToken *token=NULL);


    /// @brief initialize symbol table @a s with predefined procedures and
    ///        global variables
    void InitSymbolTable(CSymtab *s);

    /// @name methods for recursive-descent parsing
    /// @{

    /// @brief parse the top-level module
    /// @retval parsed module
    CAstModule*             module(void);

    /// @brief parse variable declarations, only if variables are actually declared
    /// @param s scope, variable will be added to this scope
    void                    varDeclarations(CAstScope *s);
    /// @brief parse const declarations, only if consts are actually declared
    /// @param s scope, const will be added to this scope
    void                    constDeclarations(CAstScope *s);
    /// @brief parse variable declarations
    /// @param s scope
    /// @retval list of identifier(s), type pairs
    pair<vector<string>,const CType*> varDecl(CAstScope *s);

    /// @brief parse subroutine declaration
    /// @param s scope, subroutine will be added to this scope
    /// @retval parsed subroutine
    CAstProcedure*          subroutineDecl(CAstScope *s);
    /// @brief parse subroutine body
    /// @param s scope
    /// @retval parsed subroutine body
    CAstStatement*          subroutineBody(CAstScope *s);
    /// @brief parse procedure declaration
    /// @param s scope, params are added to this scope
    /// @retval parsed procedure declaration
    CAstProcedure*          procedureDecl(CAstScope *s);
    /// @brief parse function declaration
    /// @param s scope, params are added to this scope
    /// @retval parsed function declaration
    CAstProcedure*          functionDecl(CAstScope *s);
    /// @brief parse procedure parameters
    /// @param s scope
    /// @retval list of parameters
    vector<CSymParam*>      formalParam(CAstScope *s);

    /// @brief parse statement sequence
    /// @param s scope
    /// @retval parsed subroutine body
    CAstStatement*          statSequence(CAstScope *s);

    /// @brief parse assignment statement
    /// @param s scope
    /// @retval parsed assignment statement
    CAstStatAssign*         assignment(CAstScope *s, CAstDesignator *lhs);
    /// @brief parse if statement
    /// @param s scope
    /// @retval parsed if statement
    CAstStatIf*             ifStatement(CAstScope *s);
    /// @brief parse while statement
    /// @param s scope
    /// @retval parsed while statement
    CAstStatWhile*          whileStatement(CAstScope *s);
    /// @brief parse return statement
    /// @param s scope
    /// @retval parsed return statement
    CAstStatReturn*         returnStatement(CAstScope *s);

    /// @brief parse expression
    /// @param s scope
    /// @retval parsed expression
    CAstExpression*         expression(CAstScope *s);
    /// @brief parse simple expression
    /// @param s scope
    /// @retval parsed simple expression
    CAstExpression*         simpleexpr(CAstScope *s);
    /// @brief parse term
    /// @param s scope
    /// @retval parsed term
    CAstExpression*         term(CAstScope *s);
    /// @brief parse factor
    /// @param s scope
    /// @retval parsed factor
    CAstExpression*         factor(CAstScope *s);

    /// @brief parse either a qualified identifier or a function call
    /// @param s scope
    /// @retval parsed node, always either CAstDesignator* or CAstFunctionCall*
    CAstExpression*         identOrCall(CAstScope *s);

    /// @brief parse an previously-declared identifier
    /// @param s scope, identifier must be in scope
    /// @retval parsed identifier
    CAstDesignator*         ident(CAstScope *s);
    /// @brief parse boolean constant
    /// @param s scope
    /// @retval parsed boolean constant
    CAstConstant*           boolConst(void);
    /// @brief parse character constant
    /// @param s scope
    /// @retval parsed character constant
    CAstConstant*           charConst(void);
    /// @brief parse string constant
    /// @param s scope, string data will be added to global scope
    /// @retval parsed string constant
    CAstStringConstant*     stringConst(CAstScope *s);
    /// @brief parse numeric constant
    /// @param s scope
    /// @retval parsed numeric constant
    CAstConstant*           number(void);

    /// @brief parse type specification
    /// @param s scope, required to evaluate array size expressions
    /// @retval parsed type specification
    const CType*            cctype(CAstScope *s);

    /// @}


    CScanner     *_scanner;       ///< CScanner instance
    CAstModule   *_module;        ///< root node of the program
    CToken        _token;         ///< current token

    /// @name error handling
    CToken        _error_token;   ///< error token
    string        _message;       ///< error message
    bool          _abort;         ///< error flag

};

#endif // __SnuPL_PARSER_H__
