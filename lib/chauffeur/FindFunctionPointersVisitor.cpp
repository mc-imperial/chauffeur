//
// Copyright (c) 2014 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "chauffeur/Chauffeur.h"
#include "chauffeur/DriverType.h"
#include "chauffeur/FindFunctionPointersVisitor.h"
#include "clang/AST/ASTContext.h"
#include <sstream>

namespace chauffeur
{
  using namespace std;

  bool FindFunctionPointersVisitor::VisitBinaryOperator(BinaryOperator* binOp)
  {
    if (!binOp->isAssignmentOp())
    {
      return true;
    }

    Expr *lhs = binOp->getLHS();
    Expr *rhs = binOp->getRHS();
    if (!isa<MemberExpr>(lhs) || !isa<ImplicitCastExpr>(rhs))
    {
      return true;
    }

    MemberExpr *lhsExpr = cast<MemberExpr>(lhs);
    NamedDecl *lhsNamedDecl = lhsExpr->getFoundDecl();

    string fdFileWithExt = Context->getSourceManager().getFilename(lhsNamedDecl->getLocation());
    string fdFile = fdFileWithExt.substr(0, fdFileWithExt.find_last_of("."));
    if (fdFile.find(FileName) == std::string::npos)
    {
      return true;
    }

    ImplicitCastExpr *rhsImplExpr = cast<ImplicitCastExpr>(rhs);
    if (rhsImplExpr->getCastKind() != 11)
    {
      return true;
    }

    llvm::errs() << "binOp: " << "\n";
    binOp->dumpColor();

    if (!isa<ImplicitCastExpr>(lhsExpr->getBase()))
    {
      return true;
    }

    ImplicitCastExpr *lhsImplExpr = cast<ImplicitCastExpr>(lhsExpr->getBase());
    if (!isa<DeclRefExpr>(lhsImplExpr->getSubExpr()) || !isa<DeclRefExpr>(rhsImplExpr->getSubExpr()))
    {
      return true;
    }

    DeclRefExpr *lhsDeclExpr = cast<DeclRefExpr>(lhsImplExpr->getSubExpr());
    DeclRefExpr *rhsDeclExpr = cast<DeclRefExpr>(rhsImplExpr->getSubExpr());

    istringstream iss(lhsDeclExpr->getDecl()->getType().getCanonicalType().getAsString());
    vector<string> tokens{istream_iterator<string>{iss}, istream_iterator<string>{}};

    llvm::errs() << "test: " << tokens[1] << "\n";
    llvm::errs() << "test: " << lhsNamedDecl->getNameAsString() << "\n";
    llvm::errs() << "test: " << rhsDeclExpr->getDecl()->getNameAsString() << "\n";

    DI->getInstance().AddFunctionPointerInformation(tokens[1] + "." + lhsNamedDecl->getNameAsString(),
      "fp", rhsDeclExpr->getDecl()->getNameAsString());

    return true;
  }

  void FindFunctionPointersVisitor::PrintFunctionPointers()
  {
    DI->getInstance().PrintFunctionPointerInfo();
  }
}
