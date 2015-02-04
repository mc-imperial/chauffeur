//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#ifndef ABSTRACTDRIVERREWRITEVISITOR_H
#define ABSTRACTDRIVERREWRITEVISITOR_H

#include "chauffeur/DriverInfo.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"


namespace chauffeur
{
  using namespace clang;

	class AbstractDriverRewriteVisitor : public RecursiveASTVisitor<AbstractDriverRewriteVisitor>
	{
	protected:
		ASTContext *Context;
		Rewriter RW;
		DriverInfo *DI;
    bool DoInline;

    explicit AbstractDriverRewriteVisitor(CompilerInstance *CI, bool doInline)
    : Context(&(CI->getASTContext()))
    {
      DoInline = doInline;
      RW.setSourceMgr(Context->getSourceManager(), Context->getLangOpts());
    }

    void InlineFunctions(FunctionDecl* FD, string fdFile);

    virtual void InstrumentEntryPoints(FunctionDecl* FD, string fdFile) = 0;
    virtual void CreateCheckerFunction(FunctionDecl* FD, string fdFile) = 0;
    virtual string GetSharedStructStr(CallExpr *callExpr) = 0;

	public:
		virtual bool VisitFunctionDecl(FunctionDecl* funcDecl);

    virtual ~AbstractDriverRewriteVisitor() = 0;

		void Finalise();
	};
}

#endif // ABSTRACTDRIVERREWRITEVISITOR_H
