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

    RemoveStatic(FD, fdFile);
    RefactorFuncWithoutNetDeviceParam(FD, fdFile);
    InstrumentInitWithEntryPointCalls(FD, fdFile);

    return true;
  }

  void RewriteVisitor::RemoveStatic(FunctionDecl* FD, string fdFile)
  {
    if (DI->getInstance().ExistsEntryPointWithName(FD->getNameInfo().getName().getAsString()))
    {
      if (FD->getStorageClass() == SC_Static)
        RW.RemoveText(FD->getInnerLocStart(), 7);
    }
    else if ((fdFile.size() > 0) && (FileName.find(fdFile) != string::npos))
    {
      if (FD->getStorageClass() == SC_Static)
        RW.ReplaceText(FD->getInnerLocStart(), 6, "static inline");
    }
  }

  void RewriteVisitor::RefactorFuncWithoutNetDeviceParam(FunctionDecl* FD, string fdFile)
  {
    if (fdFile.size() == 0)
      return;
    if (FileName.find(fdFile) == string::npos)
      return;
    if (FD->getNumParams() != 1)
      return;
    if (FD->getParamDecl(0)->getOriginalType().getAsString() != "struct device *")
      return;

    SourceRange sr = FD->getParamDecl(0)->getSourceRange();

    RW.InsertTextBefore(sr.getBegin(), "struct net_device *dev, ");

    Stmt *body = FD->getBody();
    std::list<DeclStmt*> stmtsToRewrite;

    for (StmtIterator i = body->child_begin(), e = body->child_end(); i != e; ++i)
    {
      if (!isa<DeclStmt>(*i))
        continue;

      DeclStmt *declStmt = cast<DeclStmt>(*i);
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
      DeclRefExpr *callee = cast<DeclRefExpr>(cast<ImplicitCastExpr>(call->getCallee())->getSubExpr());

      if (callee->getNameInfo().getName().getAsString() == "to_pci_dev" ||
          callee->getNameInfo().getName().getAsString() == "pci_get_drvdata")
      {
        stmtsToRewrite.push_back(declStmt);
      }
    }

    while (!stmtsToRewrite.empty())
    {
      DeclStmt *stmt = stmtsToRewrite.back();
      RW.RemoveText(stmt->getSourceRange(), *RWO);
      stmtsToRewrite.pop_back();
    }
  }

  void RewriteVisitor::InstrumentInitWithEntryPointCalls(FunctionDecl* FD, string fdFile)
  {
    if (FD->getNameInfo().getName().getAsString() != DI->getInstance().GetInitFunction())
      return;

    string net_device_str;
    Stmt *body = FD->getBody();

    for (StmtIterator i = body->child_begin(), e = body->child_end(); i != e; ++i)
    {
      if (!isa<DeclStmt>(*i))
        continue;

      DeclStmt *declStmt = cast<DeclStmt>(*i);
      if (!declStmt->isSingleDecl() && !isa<VarDecl>(declStmt->getSingleDecl()))
        continue;

      VarDecl *var = cast<VarDecl>(declStmt->getSingleDecl());
      if (!isa<ValueDecl>(var))
        continue;

      ValueDecl *value = cast<ValueDecl>(var);

      if (value->getType().getAsString(Context->getPrintingPolicy()) != "struct net_device *")
        continue;
      if (!isa<NamedDecl>(var))
        continue;

      NamedDecl *named = cast<NamedDecl>(var);
      net_device_str = named->getNameAsString();
      break;
    }

    if (net_device_str.empty())
      return;

    for (StmtIterator i = body->child_begin(), e = body->child_end(); i != e; ++i)
    {
      if (!isa<LabelStmt>(*i))
        continue;

      LabelStmt *labelStmt = cast<LabelStmt>(*i);

      RW.InsertTextBefore(labelStmt->getIdentLoc(), "\n");

      list<string> entry_points = DI->getInstance().GetEntryPoints();
      for (list<string>::reverse_iterator i = entry_points.rbegin(); i != entry_points.rend(); ++i)
      {
        RW.InsertTextBefore(labelStmt->getIdentLoc(), "	" + *i + "(" + net_device_str + ");\n");
      }
    }
  }
  
  void RewriteVisitor::Finalise()
  {
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
