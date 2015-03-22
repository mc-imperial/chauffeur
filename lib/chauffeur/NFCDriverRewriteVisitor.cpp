//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "chauffeur/Chauffeur.h"
#include "chauffeur/NFCDriverRewriteVisitor.h"

namespace chauffeur
{
  using namespace std;

  void NFCDriverRewriteVisitor::InstrumentEntryPoints(FunctionDecl* funcDecl, string fdFile)
  {
    if (funcDecl->getStorageClass() == SC_Static)
      RW.RemoveText(funcDecl->getInnerLocStart(), 7);
    return;
  }

  void NFCDriverRewriteVisitor::CreateCheckerFunction(FunctionDecl* funcDecl, string fdFile)
  {
    string device_str;
    string shared_struct_str;

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

    RW.InsertText(loc, "\tstruct nfc_dev *whoop_nfc_dev = (struct nfc_dev *) malloc(sizeof(struct nfc_dev));\n", true, true);
    RW.InsertText(loc, "\tstruct nfc_target *whoop_nfc_target = (struct nfc_target *) malloc(sizeof(struct nfc_target));\n", true, true);
    RW.InsertText(loc, "\tstruct sk_buff *whoop_skb = (struct sk_buff *) malloc(sizeof(struct sk_buff));\n", true, true);
    RW.InsertText(loc, "\tdata_exchange_cb_t whoop_de;\n\n", true, true);

    RW.InsertText(loc, "\tint whoop_int = __SMACK_nondet();\n", true, true);
    RW.InsertText(loc, "\t__SMACK_code(\"assume @ >= @;\", whoop_int, 0);\n\n", true, true);

    auto entry_points = DI->getInstance().GetEntryPoints();
    for(auto i = entry_points.rbegin(); i != entry_points.rend(); i++)
    {
      string entry_point_call;
      entry_point_call = "" + i->first + "(";

      for(auto j = i->second.begin(); j != i->second.end(); j++)
      {
        if (*j == "void")
          entry_point_call += "";
        else if (*j == "void *")
          entry_point_call += "NULL, ";
        else if (*j == "u64 *")
          entry_point_call += "NULL, ";
        else if (*j == "u8 *")
          entry_point_call += "NULL, ";
        else if (*j == "struct nfc_dev *")
          entry_point_call += "whoop_nfc_dev, ";
        else if (*j == "struct nfc_target *")
          entry_point_call += "whoop_nfc_target, ";
        else if (*j == "struct sk_buff *")
          entry_point_call += "whoop_skb, ";
        else if (*j == "data_exchange_cb_t")
          entry_point_call += "whoop_de, ";
        else if (*j == "size_t")
          entry_point_call += "whoop_int, ";
        else if (*j == "int")
          entry_point_call += "whoop_int, ";
        else if (*j == "unsigned int")
          entry_point_call += "whoop_int, ";
        else if (*j == "long")
          entry_point_call += "whoop_int, ";
        else if (*j == "unsigned long")
          entry_point_call += "whoop_int, ";
        else if (*j == "u32")
          entry_point_call += "whoop_int, ";
        else if (*j == "u8")
          entry_point_call += "whoop_int, ";
        else
          entry_point_call += *j + ", ";
      }

      if (entry_point_call != i->first + "(")
      {
        entry_point_call.resize(entry_point_call.size() - 2);
      }

      RW.InsertText(loc, "\t" + entry_point_call + ");\n", true, true);
    }

    RW.InsertText(loc, "}", true, true);
  }

  string NFCDriverRewriteVisitor::GetSharedStructStr(CallExpr *callExpr)
  {
    return "";
  }
}
