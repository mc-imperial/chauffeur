//
// Copyright (c) 2014 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#ifndef FINDENTRYPOINTSVISITOR_H
#define FINDENTRYPOINTSVISITOR_H

#include "chauffeur/DriverInfo.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"


namespace chauffeur
{
  using namespace clang;

	class FindEntryPointsVisitor : public RecursiveASTVisitor<FindEntryPointsVisitor> {
	private:
		ASTContext *Context;
		DriverInfo *DI;

	public:
	  explicit FindEntryPointsVisitor(CompilerInstance *CI)
			: Context(&(CI->getASTContext()))
		{}

    virtual ~FindEntryPointsVisitor() {}

		virtual bool VisitVarDecl(VarDecl* varDecl);
	};
}

#endif // FINDENTRYPOINTSVISITOR_H
