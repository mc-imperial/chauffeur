//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "chauffeur/Chauffeur.h"
#include "chauffeur/USBDriverRewriteVisitor.h"

namespace chauffeur
{
  using namespace std;

  void USBDriverRewriteVisitor::InstrumentEntryPoints(FunctionDecl* funcDecl, string fdFile)
  {
    if (funcDecl->getStorageClass() == SC_Static)
      RW.RemoveText(funcDecl->getInnerLocStart(), 7);
    return;
  }

  void USBDriverRewriteVisitor::CreateCheckerFunction(FunctionDecl* funcDecl, string fdFile)
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
      if (i != funcDecl->param_begin())
        RW.InsertText(loc, ", ", true, true);

      ValueDecl *paramVal = cast<ValueDecl>(*i);
      NamedDecl *paramNam = cast<NamedDecl>(*i);

      string paramType = paramVal->getType().getAsString(Context->getPrintingPolicy());
      string paramName = paramNam->getNameAsString();

      func_params[paramType] = paramName;

      RW.InsertText(loc, paramType + " " + paramName, true, true);
    }

    RW.InsertText(loc, ")\n", true, true);
    RW.InsertText(loc, "{\n", true, true);

    RW.InsertText(loc, "\tstruct tty_struct *whoop_tty_struct = (struct tty_struct *) malloc(sizeof(struct tty_struct *));\n", true, true);
    RW.InsertText(loc, "\tstruct usb_serial *whoop_usb_serial = (struct usb_serial *) malloc(sizeof(struct usb_serial *));\n", true, true);
    RW.InsertText(loc, "\tstruct usb_serial_port *whoop_usb_serial_port = (struct usb_serial_port *) malloc(sizeof(struct usb_serial_port *));\n", true, true);
    RW.InsertText(loc, "\tstruct ktermios *whoop_ktermios = (struct ktermios *) malloc(sizeof(struct ktermios *));\n", true, true);
    RW.InsertText(loc, "\tstruct urb *whoop_urb = (struct urb *) malloc(sizeof(struct urb *));\n", true, true);
    RW.InsertText(loc, "\tconst char *whoop_buf = (char *) malloc(sizeof(char *));\n\n", true, true);

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
        else if (*j == "struct tty_struct *")
          entry_point_call += "whoop_tty_struct, ";
        else if (*j == "struct usb_serial *")
          entry_point_call += "whoop_usb_serial, ";
        else if (*j == "struct usb_serial_port *")
          entry_point_call += "whoop_usb_serial_port, ";
        else if (*j == "struct urb *")
          entry_point_call += "whoop_ktermios, ";
        else if (*j == "struct ktermios *")
          entry_point_call += "whoop_urb, ";
        else if (*j == "char *")
          entry_point_call += "whoop_buf, ";
        else if (*j == "const char *")
          entry_point_call += "whoop_buf, ";
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
        else if (*j == "fmode_t")
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

  string USBDriverRewriteVisitor::GetSharedStructStr(CallExpr *callExpr)
  {
    return "";
  }
}
