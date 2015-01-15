//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "chauffeur/DriverType.h"
#include "chauffeur/ParseDriverConsumer.h"

namespace chauffeur
{
  void ParseDriverConsumer::HandleTranslationUnit(ASTContext &Context)
  {
    FEPV->TraverseDecl(Context.getTranslationUnitDecl());
    DI->getInstance().PrintDriverInfo();

    FFPV->TraverseDecl(Context.getTranslationUnitDecl());
    FFPCV->TraverseDecl(Context.getTranslationUnitDecl());
    MFPV->TraverseDecl(Context.getTranslationUnitDecl());
    MFPV->AnalyseMacros();
    DI->getInstance().PrintFunctionPointerInfo();

    if (DI->getInstance().GetType() == TEST_DRIVER)
    {
      TDRV->TraverseDecl(Context.getTranslationUnitDecl());
      TDRV->Finalise();
    }
    else if (DI->getInstance().GetType() == NETWORK_DRIVER)
    {
      NDRV->TraverseDecl(Context.getTranslationUnitDecl());
      NDRV->Finalise();
    }
  }
}
