//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "chauffeur/Chauffeur.h"
#include "chauffeur/DriverType.h"
#include "chauffeur/FindFunctionPointerCallsVisitor.h"
#include "clang/AST/ASTContext.h"
#include <sstream>

namespace chauffeur
{
  using namespace std;

  bool FindFunctionPointerCallsVisitor::VisitCallExpr(CallExpr *callExpr)
  {
    string fdFileWithExt = Context->getSourceManager().getFilename(callExpr->getRParenLoc());
    string fdFile = fdFileWithExt.substr(0, fdFileWithExt.find_last_of("."));
    if (fdFile.find(FileName) == std::string::npos)
    {
      return true;
    }

    // llvm::errs() << "call:" << "\n";
    // callExpr->dumpColor();

    Expr *callee = callExpr->getCallee();
    if (!isa<ImplicitCastExpr>(callee))
    {
      return true;
    }

    ImplicitCastExpr *calleeImplExpr = cast<ImplicitCastExpr>(callee);
    NamedDecl *calleeDecl;
    map<string, string> funcPointers;

    if (isa<DeclRefExpr>(calleeImplExpr->getSubExpr()))
    {
      DeclRefExpr *calleeDeclExpr = cast<DeclRefExpr>(calleeImplExpr->getSubExpr());
      calleeDecl = calleeDeclExpr->getDecl();
    }
    else if (isa<MemberExpr>(calleeImplExpr->getSubExpr()))
    {
      MemberExpr *calleeMemExpr = cast<MemberExpr>(calleeImplExpr->getSubExpr());
      MemberExpr *calleeSubMemExpr;
      if (isa<ImplicitCastExpr>(calleeMemExpr->getBase()))
      {
        calleeImplExpr = cast<ImplicitCastExpr>(calleeMemExpr->getBase());
        if (!isa<DeclRefExpr>(calleeImplExpr->getSubExpr()))
        {
          return true;
        }

        DeclRefExpr *calleeDeclExpr = cast<DeclRefExpr>(calleeImplExpr->getSubExpr());
        istringstream iss(calleeDeclExpr->getDecl()->getType().getCanonicalType().getAsString());
        vector<string> tokens{istream_iterator<string>{iss}, istream_iterator<string>{}};
        calleeDecl = calleeMemExpr->getMemberDecl();
        funcPointers["0"] = tokens[1] + "." + calleeDecl->getNameAsString();
      }
      else if (isa<MemberExpr>(calleeMemExpr->getBase()))
      {
        calleeSubMemExpr = cast<MemberExpr>(calleeMemExpr->getBase());
        NamedDecl *calleeSubDecl = calleeSubMemExpr->getMemberDecl();
        calleeDecl = calleeMemExpr->getMemberDecl();
        funcPointers["0"] = calleeSubDecl->getNameAsString() + "." + calleeDecl->getNameAsString();
      }
      else
      {
        return true;
      }
    }
    else
    {
      return true;
    }

    if (funcPointers.empty())
    {
      int count = 0;
      for (auto i = callExpr->arg_begin(), e = callExpr->arg_end(); i != e; ++i)
      {
        count++;

        if (!isa<ImplicitCastExpr>(*i))
        {
          continue;
        }

        ImplicitCastExpr *argImplExpr = cast<ImplicitCastExpr>(*i);
        if (!isa<MemberExpr>(argImplExpr->getSubExpr()))
        {
          continue;
        }

        MemberExpr *argMemExpr = cast<MemberExpr>(argImplExpr->getSubExpr());
        NamedDecl *argDecl = argMemExpr->getMemberDecl();
        if (!isa<MemberExpr>(argMemExpr->getBase()))
        {
          continue;
        }

        argMemExpr = cast<MemberExpr>(argMemExpr->getBase());
        funcPointers[to_string(count)] = argMemExpr->getMemberDecl()->getNameAsString() + "." + argDecl->getNameAsString();
      }
    }

    if (funcPointers.empty())
    {
      return true;
    }

    int line = Context->getSourceManager().getPresumedLineNumber(callExpr->getRParenLoc());
    for(auto i = funcPointers.begin(); i != funcPointers.end(); i++)
    {
      string info = calleeDecl->getNameAsString() + "::" + to_string(line) + "::" + i->first;
      DI->getInstance().AddFunctionPointerCallInformation(i->second, "call", info);
    }

    return true;
  }
}
