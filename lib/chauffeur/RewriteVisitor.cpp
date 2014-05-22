//
// Copyright (c) 2014 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "chauffeur/Chauffeur.h"
#include "chauffeur/RewriteVisitor.h"

namespace chauffeur
{
  using namespace std;

  bool RewriteVisitor::VisitFunctionDecl(FunctionDecl* FD)
  {
    string fdFileWithExt = Context->getSourceManager().getFilename(FD->getLocation());
    string fdFile = fdFileWithExt.substr(0, fdFileWithExt.find_last_of("."));

    if (DI->getInstance().ExistsEntryPointWithName(FD->getNameInfo().getName().getAsString()))
    {
      if (FD->getStorageClass() == SC_Static)
      {
        RW.RemoveText(FD->getInnerLocStart(), 7);
      }
    }
    else if ((fdFile.size() > 0) && (FileName.find(fdFile) != string::npos))
    {
      if (FD->getStorageClass() == SC_Static)
        RW.ReplaceText(FD->getInnerLocStart(), 6, "static inline");
    }

    if ((fdFile.size() > 0) && (FileName.find(fdFile) != string::npos))
    {
      if (FD->getNumParams() == 1 &&
        FD->getParamDecl(0)->getOriginalType().getAsString() == "struct device *")
      {
        SourceRange sr = FD->getParamDecl(0)->getSourceRange();

        RW.InsertTextBefore(sr.getBegin(), "struct net_device *dev, ");

        Stmt *body = FD->getBody();
        for (StmtIterator i = body->child_begin(), e = body->child_end(); i != e; ++i)
        {
          if (!isa<DeclStmt>(i->IgnoreImplicit()))
            continue;

          DeclStmt *declStmt = cast<DeclStmt>(i->IgnoreImplicit());
          if (!declStmt->isSingleDecl() && !isa<VarDecl>(declStmt->getSingleDecl()))
            continue;

          VarDecl *var = cast<VarDecl>(declStmt->getSingleDecl());
          if (!var->hasInit())
            continue;

          Expr *expr = var->getInit();
          if (!isa<ImplicitCastExpr>(expr))
            continue;

          ImplicitCastExpr *implicit = cast<ImplicitCastExpr>(expr);
          if (!isa<CallExpr>(implicit->getSubExpr()))
           continue;

          CallExpr *call = cast<CallExpr>(implicit->getSubExpr());
          DeclRefExpr *callee = cast<DeclRefExpr>(cast<ImplicitCastExpr>(call->getCallee()));

          // DRE->getNameInfo().getName().getAsString()

          call->dump();
          llvm::errs() << "decl: " << "true" << "\n";
        }
      }
    }

    return true;
  }

  void RewriteVisitor::Finalise() {
    string file = FileName;
    file.append(".re.c");

    string error_msg;
    llvm::raw_fd_ostream *FOS = new llvm::raw_fd_ostream(file.c_str(), error_msg, llvm::sys::fs::F_None);
    if (!error_msg.empty())
    {
      if (llvm::errs().has_colors()) llvm::errs().changeColor(llvm::raw_ostream::RED);
      llvm::errs() << "error: " << error_msg << "\n";
      if (llvm::errs().has_colors()) llvm::errs().resetColor();
      exit(1);
    }

    FOS->SetUnbuffered();
    FOS->SetUseAtomicWrites(true);

    raw_ostream *ros = FOS;

    RW.getEditBuffer(RW.getSourceMgr().getMainFileID()).write(*ros);
  }
}
