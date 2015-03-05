//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#ifndef PARSEDRIVERCONSUMER_H
#define PARSEDRIVERCONSUMER_H

#include "chauffeur/FindEntryPointsVisitor.h"
#include "chauffeur/FindFunctionPointersVisitor.h"
#include "chauffeur/FindFunctionPointerCallsVisitor.h"
#include "chauffeur/MatchFunctionPointersVisitor.h"
#include "chauffeur/NetworkDriverRewriteVisitor.h"
#include "chauffeur/CharDriverRewriteVisitor.h"
#include "chauffeur/TestDriverRewriteVisitor.h"
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
    FindFunctionPointerCallsVisitor *FFPCV;
    MatchFunctionPointersVisitor *MFPV;
		NetworkDriverRewriteVisitor *NDRV;
    CharDriverRewriteVisitor *CDRV;
    TestDriverRewriteVisitor *TDRV;

	public:
	  explicit ParseDriverConsumer(CompilerInstance *CI, bool doInline)
	    : FEPV(new FindEntryPointsVisitor(CI)),
        FFPV(new FindFunctionPointersVisitor(CI)),
        FFPCV(new FindFunctionPointerCallsVisitor(CI)),
        MFPV(new MatchFunctionPointersVisitor(CI)),
        NDRV(new NetworkDriverRewriteVisitor(CI, doInline)),
        CDRV(new CharDriverRewriteVisitor(CI, doInline)),
        TDRV(new TestDriverRewriteVisitor(CI, doInline))
		{}

	  virtual void HandleTranslationUnit(ASTContext &Context);
	};
}

#endif // PARSEDRIVERCONSUMER_H
