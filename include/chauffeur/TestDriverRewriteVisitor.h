//
// Copyright (c) 2014 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#ifndef TESTDRIVERREWRITEVISITOR_H
#define TESTDRIVERREWRITEVISITOR_H

#include "chauffeur/DriverInfo.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"


namespace chauffeur
{
  using namespace clang;

	class TestDriverRewriteVisitor : public RecursiveASTVisitor<TestDriverRewriteVisitor>
	{
	private:
		ASTContext *Context;
		Rewriter RW;
		DriverInfo *DI;
    bool DoInline;

    void InlineFunctions(FunctionDecl* FD, string fdFile);
    void InstrumentEntryPoints(FunctionDecl* FD, string fdFile);
    void InstrumentInitWithEntryPointCalls(FunctionDecl* FD, string fdFile);

	public:
	  explicit TestDriverRewriteVisitor(CompilerInstance *CI, bool doInline)
      : Context(&(CI->getASTContext()))
    {
      DoInline = doInline;
      RW.setSourceMgr(Context->getSourceManager(), Context->getLangOpts());
    }

    virtual ~TestDriverRewriteVisitor() {}

		virtual bool VisitFunctionDecl(FunctionDecl* funcDecl);

		void Finalise();
	};
}

#endif // TESTDRIVERREWRITEVISITOR_H
