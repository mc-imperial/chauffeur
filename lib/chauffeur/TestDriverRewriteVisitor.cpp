//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "chauffeur/Chauffeur.h"
#include "chauffeur/TestDriverRewriteVisitor.h"

namespace chauffeur
{
  using namespace std;

  bool TestDriverRewriteVisitor::VisitFunctionDecl(FunctionDecl* funcDecl)
  {
    string fdFileWithExt = Context->getSourceManager().getFilename(funcDecl->getLocation());
    string fdFile = fdFileWithExt.substr(0, fdFileWithExt.find_last_of("."));

    if (DoInline)
      InlineFunctions(funcDecl, fdFile);

    InstrumentEntryPoints(funcDecl, fdFile);
    InstrumentInitWithEntryPointCalls(funcDecl, fdFile);

    return true;
  }

  void TestDriverRewriteVisitor::InlineFunctions(FunctionDecl* funcDecl, string fdFile)
  {
    if (DI->getInstance().ExistsEntryPointWithName(funcDecl->getNameInfo().getName().getAsString()))
      return;

    if ((fdFile.size() > 0) && (fdFile.find(FileName) != string::npos))
    {
      if (funcDecl->getStorageClass() == SC_Static)
        RW.ReplaceText(funcDecl->getInnerLocStart(), 6, "static inline");
    }
  }

  void TestDriverRewriteVisitor::InstrumentEntryPoints(FunctionDecl* funcDecl, string fdFile)
  {
    if (!(DI->getInstance().ExistsEntryPointWithName(funcDecl->getNameInfo().getName().getAsString())))
      return;

    if (funcDecl->getStorageClass() == SC_Static)
      RW.RemoveText(funcDecl->getInnerLocStart(), 7);
  }

  void TestDriverRewriteVisitor::InstrumentInitWithEntryPointCalls(FunctionDecl* funcDecl, string fdFile)
  {
    if (funcDecl->getNameInfo().getName().getAsString() != DI->getInstance().GetInitFunction())
      return;

    string device_str;
    Stmt *body = funcDecl->getBody();

    for (auto i = body->child_begin(), e = body->child_end(); i != e; ++i)
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

      if (value->getType().getAsString(Context->getPrintingPolicy()) != "struct test_device *")
        continue;
      if (!isa<NamedDecl>(var))
        continue;

      NamedDecl *named = cast<NamedDecl>(var);
      device_str = named->getNameAsString();
      break;
    }

    if (device_str.empty())
      return;

    map<string, string> func_params;
    for (auto i = funcDecl->param_begin(), e = funcDecl->param_end(); i != e; ++i)
    {
      ValueDecl *paramVal = cast<ValueDecl>(*i);
      NamedDecl *paramNam = cast<NamedDecl>(*i);

      func_params[paramVal->getType().getAsString(Context->getPrintingPolicy())] = paramNam->getNameAsString();
    }

    ReturnStmt *returnStmt = nullptr;
    for (auto i = body->child_begin(), e = body->child_end(); i != e; ++i)
    {
      if (!isa<ReturnStmt>(*i))
        continue;
      returnStmt = cast<ReturnStmt>(*i);
      break;
    }

    LabelStmt *labelStmt = nullptr;
    for (auto i = body->child_begin(), e = body->child_end(); i != e; ++i)
    {
      if (!isa<LabelStmt>(*i))
        continue;
      labelStmt = cast<LabelStmt>(*i);
      break;
    }

    if (labelStmt)
      RW.InsertTextBefore(labelStmt->getIdentLoc(), "\n");
    else
      RW.InsertTextBefore(returnStmt->getReturnLoc(), "\n\t");

    auto entry_points = DI->getInstance().GetEntryPoints();
    for(auto i = entry_points.rbegin(); i != entry_points.rend(); i++)
    {
      string entry_point_call;
      if (labelStmt)
        entry_point_call = "\t" + i->first + "(";
      else
        entry_point_call = "" + i->first + "(";

      if (find(i->second.begin(), i->second.end(), "struct test_device *") == i->second.end())
        entry_point_call += device_str + ", ";

      for(auto j = i->second.begin(); j != i->second.end(); j++)
      {
        if (*j == "struct test_device *")
          entry_point_call += device_str + ", ";
        else
          entry_point_call += *j + ", ";
      }

      entry_point_call.resize(entry_point_call.size() - 2);

      if (labelStmt)
        RW.InsertTextBefore(labelStmt->getIdentLoc(), entry_point_call + ");\n");
      else
        RW.InsertTextBefore(returnStmt->getReturnLoc(), entry_point_call + ");\n\t");
    }
  }

  void TestDriverRewriteVisitor::Finalise()
  {
    string file = FileName;
    file.append(".re.c");

    string error_msg;
    llvm::raw_fd_ostream *fos = new llvm::raw_fd_ostream(file.c_str(), error_msg, llvm::sys::fs::F_None);
    if (!error_msg.empty())
    {
      if (llvm::errs().has_colors()) llvm::errs().changeColor(llvm::raw_ostream::RED);
      llvm::errs() << "error: " << error_msg << "\n";
      if (llvm::errs().has_colors()) llvm::errs().resetColor();
      exit(1);
    }

    fos->SetUnbuffered();
    fos->SetUseAtomicWrites(true);

    raw_ostream *ros = fos;

    RW.getEditBuffer(RW.getSourceMgr().getMainFileID()).write(*ros);
  }
}
