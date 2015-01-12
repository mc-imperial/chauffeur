//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#ifndef FINDFUNCTIONPOINTERCALLSVISITOR_H
#define FINDFUNCTIONPOINTERCALLSVISITOR_H

#include "chauffeur/DriverInfo.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"


namespace chauffeur
{
  using namespace clang;

	class FindFunctionPointerCallsVisitor : public RecursiveASTVisitor<FindFunctionPointerCallsVisitor> {
	private:
		ASTContext *Context;
		DriverInfo *DI;

	public:
	  explicit FindFunctionPointerCallsVisitor(CompilerInstance *CI)
			: Context(&(CI->getASTContext()))
		{}

    virtual ~FindFunctionPointerCallsVisitor() {}

    virtual bool VisitCallExpr(CallExpr *callExpr);
	};
}

#endif // FINDFUNCTIONPOINTERCALLSVISITOR_H
