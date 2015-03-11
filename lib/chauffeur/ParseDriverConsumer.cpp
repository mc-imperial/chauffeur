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
    FFPV->TraverseDecl(Context.getTranslationUnitDecl());
    FFPCV->TraverseDecl(Context.getTranslationUnitDecl());
    MFPV->TraverseDecl(Context.getTranslationUnitDecl());
    MFPV->AnalyseMacros();

    if (DI->getInstance().GetType() == CHAR_DRIVER)
    {
      CDRV->TraverseDecl(Context.getTranslationUnitDecl());
      CDRV->Finalise();
    }
    else if (DI->getInstance().GetType() == BLOCK_DRIVER)
    {
      BDRV->TraverseDecl(Context.getTranslationUnitDecl());
      BDRV->Finalise();
    }
    else if (DI->getInstance().GetType() == NETWORK_DRIVER)
    {
      NDRV->TraverseDecl(Context.getTranslationUnitDecl());
      NDRV->Finalise();
    }
    else if (DI->getInstance().GetType() == FILE_SYSTEM)
    {
      FSRV->TraverseDecl(Context.getTranslationUnitDecl());
      FSRV->Finalise();
    }
    else if (DI->getInstance().GetType() == TEST_DRIVER)
    {
      TDRV->TraverseDecl(Context.getTranslationUnitDecl());
      TDRV->Finalise();
    }

    DI->getInstance().PrintDriverInfo();
    DI->getInstance().PrintFunctionPointerInfo();
  }
}
