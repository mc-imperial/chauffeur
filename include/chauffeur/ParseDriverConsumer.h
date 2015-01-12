//
// Copyright (c) 2014 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#ifndef PARSEDRIVERCONSUMER_H
#define PARSEDRIVERCONSUMER_H

#include "chauffeur/FindEntryPointsVisitor.h"
#include "chauffeur/FindFunctionPointersVisitor.h"
#include "chauffeur/TestDriverRewriteVisitor.h"
#include "chauffeur/NetworkDriverRewriteVisitor.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"

namespace chauffeur
{
  using namespace clang;

	class ParseDriverConsumer : public ASTConsumer {
	private:
    DriverInfo *DI;
	  FindEntryPointsVisitor *FEPV;
    FindFunctionPointersVisitor *FFPV;
    TestDriverRewriteVisitor *TDRV;
		NetworkDriverRewriteVisitor *NDRV;

	public:
	  explicit ParseDriverConsumer(CompilerInstance *CI, bool doInline)
	    : FEPV(new FindEntryPointsVisitor(CI)),
        FFPV(new FindFunctionPointersVisitor(CI)),
        TDRV(new TestDriverRewriteVisitor(CI, doInline)),
        NDRV(new NetworkDriverRewriteVisitor(CI, doInline))
		{}

	  virtual void HandleTranslationUnit(ASTContext &Context);
	};
}

#endif // PARSEDRIVERCONSUMER_H
