//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "chauffeur/Chauffeur.h"
#include "chauffeur/TestDriverRewriteVisitor.h"

namespace chauffeur
{
  using namespace std;

  void TestDriverRewriteVisitor::InstrumentEntryPoints(FunctionDecl* funcDecl, string fdFile)
  {
    if (funcDecl->getStorageClass() == SC_Static)
      RW.RemoveText(funcDecl->getInnerLocStart(), 7);
  }

  void TestDriverRewriteVisitor::CreateCheckerFunction(FunctionDecl* funcDecl, string fdFile)
  {
    string device_str;
    Stmt *body = funcDecl->getBody();

    for (auto i = body->child_begin(), e = body->child_end(); i != e; ++i)
    {
      if (!isa<DeclStmt>(*i))
        continue;

      DeclStmt *declStmt = cast<DeclStmt>(*i);
      if (!declStmt->isSingleDecl() && !isa<VarDecl>(declStmt->getSingleDecl()))
        continue;

      VarDecl *varDecl = cast<VarDecl>(declStmt->getSingleDecl());
      if (!isa<ValueDecl>(varDecl))
        continue;

      ValueDecl *value = cast<ValueDecl>(varDecl);

      if (value->getType().getAsString(Context->getPrintingPolicy()) != "struct test_device *")
        continue;
      if (!isa<NamedDecl>(varDecl))
        continue;

      NamedDecl *named = cast<NamedDecl>(varDecl);
      device_str = named->getNameAsString();
      break;
    }

    if (device_str.empty())
      return;

    string shared_struct_str;

    for (auto i = body->child_begin(), e = body->child_end(); i != e; ++i)
    {
      if (isa<DeclStmt>(*i))
      {
        DeclStmt *declStmt = cast<DeclStmt>(*i);
        if (!declStmt->isSingleDecl() && !isa<VarDecl>(declStmt->getSingleDecl()))
          continue;

        VarDecl *varDecl = cast<VarDecl>(declStmt->getSingleDecl());
        if (!isa<ValueDecl>(varDecl))
          continue;

        ValueDecl *value = cast<ValueDecl>(varDecl);

        if (value->getType().getAsString(Context->getPrintingPolicy()) != "struct test_device *")
          continue;
        if (!isa<NamedDecl>(varDecl))
          continue;

        if (varDecl->getInit() == 0 || !isa<CallExpr>(varDecl->getInit()))
          continue;

        CallExpr *callExpr = cast<CallExpr>(varDecl->getInit());
        shared_struct_str = GetSharedStructStr(callExpr);
        if (shared_struct_str != "")
          break;
      }
      else if (isa<BinaryOperator>(*i))
      {
        BinaryOperator *binOp = cast<BinaryOperator>(*i);
        if (!isa<CallExpr>(binOp->getRHS()))
          continue;

        CallExpr *callExpr = cast<CallExpr>(binOp->getRHS());
        shared_struct_str = GetSharedStructStr(callExpr);
        if (shared_struct_str != "")
          break;
      }
    }

    if (shared_struct_str.empty())
      return;

    FileID fileId = Context->getSourceManager().getFileID(funcDecl->getLocation());
    SourceLocation loc = Context->getSourceManager().getLocForEndOfFile(fileId);

    RW.InsertText(loc, "\n", true, true);
    RW.InsertText(loc, "void whoop$checker(", true, true);

    map<string, string> func_params;
    for (auto i = funcDecl->param_begin(), e = funcDecl->param_end(); i != e; ++i)
    {
      ValueDecl *paramVal = cast<ValueDecl>(*i);
      NamedDecl *paramNam = cast<NamedDecl>(*i);

      string paramType = paramVal->getType().getAsString(Context->getPrintingPolicy());
      string paramName = paramNam->getNameAsString();

      func_params[paramType] = paramName;

      if (i == funcDecl->param_begin())
        RW.InsertText(loc, paramType + " " + paramName + ", ", true, true);
      else
        RW.InsertText(loc, paramType + " " + paramName, true, true);
    }

    RW.InsertText(loc, ")\n", true, true);
    RW.InsertText(loc, "{\n", true, true);

    RW.InsertText(loc, "\tstruct test_device *dev;\n", true, true);
    RW.InsertText(loc, "\t" + shared_struct_str + "shared;\n", true, true);
    RW.InsertText(loc, "\tdev = alloc_testdev(sizeof(*shared));\n\n", true, true);

    auto entry_points = DI->getInstance().GetEntryPoints();
    for(auto i = entry_points.rbegin(); i != entry_points.rend(); i++)
    {
      string entry_point_call;
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

      RW.InsertText(loc, "\t" + entry_point_call + ");\n", true, true);
    }

    RW.InsertText(loc, "}", true, true);
  }

  string TestDriverRewriteVisitor::GetSharedStructStr(CallExpr *callExpr)
  {
    string shared_struct_str = "";

    Expr *callee = callExpr->getCallee();
    if (!isa<ImplicitCastExpr>(callee))
      return shared_struct_str;

    ImplicitCastExpr *calleeImplExpr = cast<ImplicitCastExpr>(callee);
    if (!isa<DeclRefExpr>(calleeImplExpr->getSubExpr()))
      return shared_struct_str;

    DeclRefExpr *calleeDeclExpr = cast<DeclRefExpr>(calleeImplExpr->getSubExpr());
    if (calleeDeclExpr->getNameInfo().getAsString() != "alloc_testdev")
      return shared_struct_str;

    for (auto i = callExpr->arg_begin(), e = callExpr->arg_end(); i != e; ++i)
    {
      if (!isa<ImplicitCastExpr>(*i))
        continue;

      ImplicitCastExpr *argImplExpr = cast<ImplicitCastExpr>(*i);
      if (!isa<UnaryExprOrTypeTraitExpr>(argImplExpr->getSubExpr()))
        continue;

      UnaryExprOrTypeTraitExpr *argExpr = cast<UnaryExprOrTypeTraitExpr>(argImplExpr->getSubExpr());
      ParenExpr *parenExpr = cast<ParenExpr>(argExpr->getArgumentExpr());
      UnaryOperator *uop = cast<UnaryOperator>(parenExpr->getSubExpr());
      ImplicitCastExpr *implExpr = cast<ImplicitCastExpr>(uop->getSubExpr());
      DeclRefExpr *declExpr = cast<DeclRefExpr>(implExpr->getSubExpr());
      ValueDecl *valueDecl = cast<ValueDecl>(declExpr->getDecl());

      shared_struct_str = valueDecl->getType().getAsString(Context->getPrintingPolicy());
      break;
    }

    return shared_struct_str;
  }
}
