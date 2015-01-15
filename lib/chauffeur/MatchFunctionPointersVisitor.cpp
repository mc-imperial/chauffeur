//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "chauffeur/Chauffeur.h"
#include "chauffeur/DriverType.h"
#include "chauffeur/MatchFunctionPointersVisitor.h"
#include "chauffeur/Utilities.h"
#include "clang/AST/ASTContext.h"

namespace chauffeur
{
  using namespace std;

  bool MatchFunctionPointersVisitor::VisitBinaryOperator(BinaryOperator* binOp)
  {
    string fdFileWithExt = Context->getSourceManager().getFilename(binOp->getOperatorLoc());
    string fdFile = fdFileWithExt.substr(0, fdFileWithExt.find_last_of("."));
    if (fdFile.find(FileName) == std::string::npos)
      return true;
    if (!binOp->isAssignmentOp())
      return true;

    Expr *lhs = binOp->getLHS();
    Expr *rhs = binOp->getRHS();
    if (!isa<MemberExpr>(lhs) || !isa<ImplicitCastExpr>(rhs))
      return true;

    MemberExpr *lhsExpr = cast<MemberExpr>(lhs);
    NamedDecl *lhsNamedDecl = lhsExpr->getFoundDecl();

    ImplicitCastExpr *rhsImplExpr = cast<ImplicitCastExpr>(rhs);
    if (rhsImplExpr->getCastKind() != 3)
      return true;
    if (!isa<ImplicitCastExpr>(lhsExpr->getBase()))
      return true;

    ImplicitCastExpr *lhsImplExpr = cast<ImplicitCastExpr>(lhsExpr->getBase());
    if (!isa<DeclRefExpr>(lhsImplExpr->getSubExpr()) || !isa<MemberExpr>(rhsImplExpr->getSubExpr()))
      return true;

    MemberExpr *rhsMemberExpr = cast<MemberExpr>(rhsImplExpr->getSubExpr());
    if (!isa<ImplicitCastExpr>(rhsMemberExpr->getBase()))
      return true;

    rhsImplExpr = cast<ImplicitCastExpr>(rhsMemberExpr->getBase());
    if (!isa<DeclRefExpr>(rhsImplExpr->getSubExpr()))
      return true;

    DeclRefExpr *lhsDeclExpr = cast<DeclRefExpr>(lhsImplExpr->getSubExpr());
    DeclRefExpr *rhsDeclExpr = cast<DeclRefExpr>(rhsImplExpr->getSubExpr());

    string lhsPrefix = GetPrefix(lhsDeclExpr->getDecl()->getType().getCanonicalType().getAsString());
    string rhsPrefix = GetPrefix(rhsDeclExpr->getDecl()->getType().getCanonicalType().getAsString());

    lhsPrefix = lhsPrefix + "." + lhsNamedDecl->getNameAsString();
    rhsPrefix = rhsPrefix + "." + lhsNamedDecl->getNameAsString();

    DI->getInstance().MergeFunctionPointerInformation(lhsPrefix, rhsPrefix);

    return true;
  }

  void MatchFunctionPointersVisitor::AnalyseMacros()
  {
    for (Preprocessor::macro_iterator i = PP->macro_begin(), e = PP->macro_end(); i != e; ++i)
    {
      if (!i->first->hasMacroDefinition())
        continue;

      MacroInfo *macroInfo = i->second->getMacroInfo();
      if (macroInfo == 0)
        continue;

      string fdFileWithExt = Context->getSourceManager().getFilename(macroInfo->getDefinitionLoc());
      string fdFile = fdFileWithExt.substr(0, fdFileWithExt.find_last_of("."));
      if (fdFile.find(FileName) == std::string::npos)
        continue;

      if (!macroInfo->isFunctionLike())
        continue;

      string name = "";
      string fp = "";
      string decl = "";
      string param = "";

      for (MacroInfo::tokens_iterator ti = macroInfo->tokens_begin(), te = macroInfo->tokens_end(); ti != te; ++ti)
      {
        string tempFp = DI->getInstance().GetFunctionPointerInformation(PP->getSpelling(*ti));
        string tempName = PP->getSpelling(*ti);
        if (tempFp.length() == 0)
          continue;

        ++ti;
        if (strcmp(ti->getName(), "equal") != 0)
          continue;

        ++ti;
        while((strcmp(ti->getName(), "identifier") == 0) || (strcmp(ti->getName(), "hashhash") == 0))
        {
          decl = decl + PP->getSpelling(*ti);
          ++ti;
        }

        fp = tempFp + "";
        name = tempName + "";
      }

      if (decl.length() == 0)
        continue;
      
      for (MacroInfo::tokens_iterator ti = macroInfo->tokens_begin(), te = macroInfo->tokens_end(); ti != te; ++ti)
      {
        if (strcmp(ti->getName(), "identifier") != 0)
          continue;
        if (PP->getSpelling(*ti) + "." + name != fp)
          continue;

        string temp = "";

        ++ti;
        if (strcmp(ti->getName(), "identifier") == 0)
          temp = PP->getSpelling(*ti);

        ++ti;
        if (strcmp(ti->getName(), "equal") != 0)
          continue;

        ++ti;
        if (strcmp(ti->getName(), "l_brace") != 0)
          continue;

        param = temp + "";
      }

      DI->getInstance().AddFunctionPointerInformation(fp, "macro", param + "::" + decl);
    }
  }
}
