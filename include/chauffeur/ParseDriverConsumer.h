//
// Copyright (c) 2014 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#ifndef PARSEDRIVERCONSUMER_H
#define PARSEDRIVERCONSUMER_H

#include "chauffeur/FindEntryPointsVisitor.h"
#include "chauffeur/RewriteVisitor.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"

namespace chauffeur
{
  using namespace clang;

	class ParseDriverConsumer : public ASTConsumer {
	private:
	  FindEntryPointsVisitor *FEPV;
		RewriteVisitor *RV;

	public:
	  explicit ParseDriverConsumer(CompilerInstance *CI)
	    : FEPV(new FindEntryPointsVisitor(CI)), RV(new RewriteVisitor(CI))
		{}

	  virtual void HandleTranslationUnit(ASTContext &Context);
	};
}

#endif // PARSEDRIVERCONSUMER_H
