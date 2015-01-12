//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#ifndef FINDFUNCTIONPOINTERSVISITOR_H
#define FINDFUNCTIONPOINTERSVISITOR_H

#include "chauffeur/DriverInfo.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"


namespace chauffeur
{
  using namespace clang;

	class FindFunctionPointersVisitor : public RecursiveASTVisitor<FindFunctionPointersVisitor> {
	private:
		ASTContext *Context;
		DriverInfo *DI;

	public:
	  explicit FindFunctionPointersVisitor(CompilerInstance *CI)
			: Context(&(CI->getASTContext()))
		{}

    virtual ~FindFunctionPointersVisitor() {}

    virtual bool VisitBinaryOperator(BinaryOperator* binOp);
	};
}

#endif // FINDFUNCTIONPOINTERSVISITOR_H
