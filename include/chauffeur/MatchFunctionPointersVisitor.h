//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#ifndef MATCHFUNCTIONPOINTERSVISITOR_H
#define MATCHFUNCTIONPOINTERSVISITOR_H

#include "chauffeur/DriverInfo.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"


namespace chauffeur
{
  using namespace clang;

	class MatchFunctionPointersVisitor : public RecursiveASTVisitor<MatchFunctionPointersVisitor> {
	private:
		ASTContext *Context;
    Preprocessor *PP;
		DriverInfo *DI;

	public:
	  explicit MatchFunctionPointersVisitor(CompilerInstance *CI)
			: Context(&(CI->getASTContext()))
		{
      PP = &CI->getPreprocessor();
    }

    virtual ~MatchFunctionPointersVisitor() {}

    virtual bool VisitBinaryOperator(BinaryOperator* binOp);

    void AnalyseMacros();
	};
}

#endif // MATCHFUNCTIONPOINTERSVISITOR_H
