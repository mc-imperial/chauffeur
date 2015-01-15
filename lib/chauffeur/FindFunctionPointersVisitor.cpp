//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "chauffeur/Chauffeur.h"
#include "chauffeur/DriverType.h"
#include "chauffeur/FindFunctionPointersVisitor.h"
#include "chauffeur/Utilities.h"
#include "clang/AST/ASTContext.h"

namespace chauffeur
{
  using namespace std;

  bool FindFunctionPointersVisitor::VisitBinaryOperator(BinaryOperator* binOp)
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

    // llvm::errs() << "binOp:" << "\n";
    // binOp->dumpColor();

    ImplicitCastExpr *rhsImplExpr = cast<ImplicitCastExpr>(rhs);
    if (rhsImplExpr->getCastKind() != 11)
      return true;

    if (!isa<ImplicitCastExpr>(lhsExpr->getBase()))
      return true;

    ImplicitCastExpr *lhsImplExpr = cast<ImplicitCastExpr>(lhsExpr->getBase());
    if (!isa<DeclRefExpr>(lhsImplExpr->getSubExpr()) || !isa<DeclRefExpr>(rhsImplExpr->getSubExpr()))
      return true;

    DeclRefExpr *lhsDeclExpr = cast<DeclRefExpr>(lhsImplExpr->getSubExpr());
    DeclRefExpr *rhsDeclExpr = cast<DeclRefExpr>(rhsImplExpr->getSubExpr());

    string prefix = GetPrefix(lhsDeclExpr->getDecl()->getType().getCanonicalType().getAsString());

    DI->getInstance().AddFunctionPointerInformation(prefix + "." + lhsNamedDecl->getNameAsString(),
      "decl", rhsDeclExpr->getDecl()->getNameAsString());

    return true;
  }

  bool FindFunctionPointersVisitor::VisitVarDecl(VarDecl* varDecl)
  {
    string fdFileWithExt = Context->getSourceManager().getFilename(varDecl->getLocation());
    string fdFile = fdFileWithExt.substr(0, fdFileWithExt.find_last_of("."));
    if (fdFile.find(FileName) == std::string::npos)
      return true;

    if (varDecl->getType()->isRecordType())
    {
      RecordDecl *baseRecDecl = varDecl->getType()->getAs<RecordType>()->getDecl();
      if (DI->IsDriverModule(baseRecDecl->getNameAsString()))
        return true;
    }

    if (varDecl->getInit() == 0 || !isa<InitListExpr>(varDecl->getInit()))
      return true;

    InitListExpr *initListExpr = cast<InitListExpr>(varDecl->getInit())->getSyntacticForm();
    if (initListExpr == 0)
      return true;

    string prefix = GetPrefix(varDecl->getType().getCanonicalType().getAsString());
    if (prefix == "")
      return true;

    this->AnalyseInitListExpr(initListExpr, prefix);

    return true;
  }

  void FindFunctionPointersVisitor::AnalyseInitListExpr(InitListExpr* initListExpr, string prefix)
  {
    for (auto range = initListExpr->children(); range; ++range)
    {
      DesignatedInitExpr *desExpr = cast<DesignatedInitExpr>(*range);
      if (desExpr->size() > 1)
        continue;

      if (desExpr->getDesignator(0)->isArrayDesignator())
      {
        if(!isa<InitListExpr>(desExpr->getSubExpr(0)))
          continue;

        InitListExpr *expr = cast<InitListExpr>(desExpr->getSubExpr(0));
        if (initListExpr == 0)
          continue;

        this->AnalyseInitListExpr(expr, prefix);
      }
      else if (desExpr->getDesignator(0)->isFieldDesignator())
      {
        if(!isa<ImplicitCastExpr>(desExpr->getSubExpr(0)))
          continue;

        ImplicitCastExpr *implExpr = cast<ImplicitCastExpr>(desExpr->getSubExpr(0));
        if (implExpr->getCastKind() != 11 || !isa<DeclRefExpr>(implExpr->getSubExpr()))
          continue;

        DeclRefExpr *declExpr = cast<DeclRefExpr>(implExpr->getSubExpr());
        string fp = prefix + "." + desExpr->getDesignator(0)->getFieldName()->getName().str();
        DI->getInstance().AddFunctionPointerInformation(fp, "decl", declExpr->getDecl()->getNameAsString());
      }
    }
  }
}
