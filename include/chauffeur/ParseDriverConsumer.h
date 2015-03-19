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
#include "chauffeur/CharDriverRewriteVisitor.h"
#include "chauffeur/BlockDriverRewriteVisitor.h"
#include "chauffeur/NetworkDriverRewriteVisitor.h"
#include "chauffeur/FileSystemRewriteVisitor.h"
#include "chauffeur/NFCDriverRewriteVisitor.h"
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
    CharDriverRewriteVisitor *CDRV;
    BlockDriverRewriteVisitor *BDRV;
		NetworkDriverRewriteVisitor *NDRV;
    FileSystemRewriteVisitor *FSRV;
    NFCDriverRewriteVisitor *NFCDRV;
    TestDriverRewriteVisitor *TDRV;

	public:
	  explicit ParseDriverConsumer(CompilerInstance *CI, bool doInline)
	    : FEPV(new FindEntryPointsVisitor(CI)),
        FFPV(new FindFunctionPointersVisitor(CI)),
        FFPCV(new FindFunctionPointerCallsVisitor(CI)),
        MFPV(new MatchFunctionPointersVisitor(CI)),
        CDRV(new CharDriverRewriteVisitor(CI, doInline)),
        BDRV(new BlockDriverRewriteVisitor(CI, doInline)),
        NDRV(new NetworkDriverRewriteVisitor(CI, doInline)),
        FSRV(new FileSystemRewriteVisitor(CI, doInline)),
        NFCDRV(new NFCDriverRewriteVisitor(CI, doInline)),
        TDRV(new TestDriverRewriteVisitor(CI, doInline))
		{}

	  virtual void HandleTranslationUnit(ASTContext &Context);
	};
}

#endif // PARSEDRIVERCONSUMER_H
