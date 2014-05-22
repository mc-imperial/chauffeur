//
// Copyright (c) 2014 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "chauffeur/ParseDriverConsumer.h"

namespace chauffeur
{
  void ParseDriverConsumer::HandleTranslationUnit(ASTContext &Context)
  {
    FEPV->TraverseDecl(Context.getTranslationUnitDecl());
    FEPV->PrintEntryPoints();
    RV->TraverseDecl(Context.getTranslationUnitDecl());
    RV->Finalise();
  }
}
