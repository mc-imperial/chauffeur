//
// Copyright (c) 2014 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#ifndef NETWORKDRIVERREWRITEVISITOR_H
#define NETWORKDRIVERREWRITEVISITOR_H

#include "chauffeur/DriverInfo.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"


namespace chauffeur
{
  using namespace clang;

	class NetworkDriverRewriteVisitor : public RecursiveASTVisitor<NetworkDriverRewriteVisitor>
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
	  explicit NetworkDriverRewriteVisitor(CompilerInstance *CI, bool doInline)
      : Context(&(CI->getASTContext()))
    {
      DoInline = doInline;
      RW.setSourceMgr(Context->getSourceManager(), Context->getLangOpts());
    }

		virtual bool VisitFunctionDecl(FunctionDecl* FD);

		void Finalise();
	};
}

#endif // NETWORKDRIVERREWRITEVISITOR_H
