//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "chauffeur/Chauffeur.h"
#include "chauffeur/NetworkDriverRewriteVisitor.h"

namespace chauffeur
{
  using namespace std;

  void NetworkDriverRewriteVisitor::InstrumentEntryPoints(FunctionDecl* funcDecl, string fdFile)
  {
    if (funcDecl->getStorageClass() == SC_Static)
      RW.RemoveText(funcDecl->getInnerLocStart(), 7);

    if (DI->getInstance().GetInitFunction() == funcDecl->getNameInfo().getName().getAsString())
      return;

    if (funcDecl->getParamDecl(0)->getOriginalType().getAsString() != "struct device *" &&
      funcDecl->getParamDecl(0)->getOriginalType().getAsString() != "struct pci_dev *")
      return;

    SourceRange sr = funcDecl->getParamDecl(0)->getSourceRange();

    RW.InsertTextBefore(sr.getBegin(), "struct net_device *dev, ");

    Stmt *body = funcDecl->getBody();
    list<DeclStmt*> stmtsToRewrite;

    for (auto i = body->child_begin(), e = body->child_end(); i != e; ++i)
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
      RW.RemoveText(stmt->getSourceRange());
      stmtsToRewrite.pop_back();
    }
  }

  void NetworkDriverRewriteVisitor::CreateCheckerFunction(FunctionDecl* funcDecl, string fdFile)
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

      if (value->getType().getAsString(Context->getPrintingPolicy()) != "struct net_device *")
        continue;
      if (!isa<NamedDecl>(varDecl))
        continue;

      NamedDecl *named = cast<NamedDecl>(varDecl);
      device_str = named->getNameAsString();
      break;
    }

    if (device_str.empty())
      return;

    string shared_struct_str = GetSharedStructStrInFunctionBody(body);
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

    RW.InsertText(loc, "\tstruct net_device *dev;\n", true, true);
    RW.InsertText(loc, "\t" + shared_struct_str + "shared;\n", true, true);
    RW.InsertText(loc, "\tdev = alloc_etherdev(sizeof(*shared));\n\n", true, true);

    RW.InsertText(loc, "\tstruct sk_buff *whoop_skb = (struct sk_buff *) malloc(sizeof(struct sk_buff *));\n", true, true);
    RW.InsertText(loc, "\tstruct ethtool_wolinfo *whoop_wolinfo = (struct ethtool_wolinfo *) malloc(sizeof(struct ethtool_wolinfo *));\n", true, true);
    RW.InsertText(loc, "\tstruct ethtool_cmd *whoop_ecmd = (struct ethtool_cmd *) malloc(sizeof(struct ethtool_cmd *));\n", true, true);
    RW.InsertText(loc, "\tstruct ifreq *whoop_ifreq = (struct ifreq *) malloc(sizeof(struct ifreq *));\n", true, true);
    RW.InsertText(loc, "\tstruct rtnl_link_stats64 *whoop_rtnlsts64 = (struct rtnl_link_stats64 *) malloc(sizeof(struct rtnl_link_stats64 *));\n", true, true);
    RW.InsertText(loc, "\tstruct ethtool_regs *whoop_ethtoolregs = (struct ethtool_regs *) malloc(sizeof(struct ethtool_regs *));\n", true, true);
    RW.InsertText(loc, "\tstruct ethtool_stats *whoop_ethtoolsts = (struct ethtool_stats *) malloc(sizeof(struct ethtool_stats *));\n", true, true);
    RW.InsertText(loc, "\tstruct ethtool_drvinfo *whoop_ethtooldrvinfo = (struct ethtool_drvinfo *) malloc(sizeof(struct ethtool_drvinfo *));\n", true, true);
    RW.InsertText(loc, "\tnetdev_features_t whoop_netdevfeat = NETIF_F_RXCSUM;\n\n", true, true);

    auto entry_points = DI->getInstance().GetEntryPoints();
    for(auto i = entry_points.rbegin(); i != entry_points.rend(); i++)
    {
      string entry_point_call;
      entry_point_call = "" + i->first + "(";

      if (find(i->second.begin(), i->second.end(), "struct net_device *") == i->second.end())
        entry_point_call += device_str + ", ";

      for(auto j = i->second.begin(); j != i->second.end(); j++)
      {
        if (*j == "struct net_device *")
          entry_point_call += device_str + ", ";
        else if (*j == "struct pci_dev *")
          entry_point_call += func_params["struct pci_dev *"] + ", ";
        else if (*j == "struct device *")
          entry_point_call += "&" + func_params["struct pci_dev *"] + "->dev, ";
        else if (*j == "void *")
          entry_point_call += "NULL, ";
        else if (*j == "u64 *")
          entry_point_call += "NULL, ";
        else if (*j == "u8 *")
          entry_point_call += "NULL, ";
        else if (*j == "struct sk_buff *")
          entry_point_call += "whoop_skb, ";
        else if (*j == "struct ethtool_wolinfo *")
          entry_point_call += "whoop_wolinfo, ";
        else if (*j == "struct ethtool_cmd *")
          entry_point_call += "whoop_ecmd, ";
        else if (*j == "struct ifreq *")
          entry_point_call += "whoop_ifreq, ";
        else if (*j == "struct rtnl_link_stats64 *")
          entry_point_call += "whoop_rtnlsts64, ";
        else if (*j == "struct ethtool_regs *")
          entry_point_call += "whoop_ethtoolregs, ";
        else if (*j == "struct ethtool_stats *")
          entry_point_call += "whoop_ethtoolsts, ";
        else if (*j == "struct ethtool_drvinfo *")
          entry_point_call += "whoop_ethtooldrvinfo, ";
        else if (*j == "netdev_features_t")
          entry_point_call += "whoop_netdevfeat, ";
        else if (*j == "int")
          entry_point_call += "0, ";
        else if (*j == "u32")
          entry_point_call += "0, ";
        else
          entry_point_call += *j + ", ";
      }

      entry_point_call.resize(entry_point_call.size() - 2);

      RW.InsertText(loc, "\t" + entry_point_call + ");\n", true, true);
    }

    RW.InsertText(loc, "}", true, true);
  }

  string NetworkDriverRewriteVisitor::GetSharedStructStrInFunctionBody(Stmt *body)
  {
    string shared_struct_str = "";

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

        if (value->getType().getAsString(Context->getPrintingPolicy()) != "struct net_device *")
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

    return shared_struct_str;
  }

  string NetworkDriverRewriteVisitor::GetSharedStructStr(CallExpr *callExpr)
  {
    string shared_struct_str = "";

    Expr *callee = callExpr->getCallee();
    if (!isa<ImplicitCastExpr>(callee))
      return shared_struct_str;

    ImplicitCastExpr *calleeImplExpr = cast<ImplicitCastExpr>(callee);
    if (!isa<DeclRefExpr>(calleeImplExpr->getSubExpr()))
      return shared_struct_str;

    DeclRefExpr *calleeDeclExpr = cast<DeclRefExpr>(calleeImplExpr->getSubExpr());
    Stmt *body = callExpr->getCalleeDecl()->getBody();

    if (calleeDeclExpr->getNameInfo().getAsString() != "alloc_etherdev")
      shared_struct_str = GetSharedStructStrInFunctionBody(body);
    if (calleeDeclExpr->getNameInfo().getAsString() != "alloc_etherdev")
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
