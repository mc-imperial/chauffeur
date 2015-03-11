//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "chauffeur/Chauffeur.h"
#include "chauffeur/FileSystemRewriteVisitor.h"

namespace chauffeur
{
  using namespace std;

  void FileSystemRewriteVisitor::InstrumentEntryPoints(FunctionDecl* funcDecl, string fdFile)
  {
    return;
  }

  void FileSystemRewriteVisitor::CreateCheckerFunction(FunctionDecl* funcDecl, string fdFile)
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

    RW.InsertText(loc, "\tstruct inode *whoop_inode = (struct inode *) malloc(sizeof(struct inode *));\n", true, true);
    RW.InsertText(loc, "\tstruct super_block *whoop_super_block = (struct super_block *) malloc(sizeof(struct super_block *));\n", true, true);
    RW.InsertText(loc, "\tstruct vfsmount *whoop_vfsmount = (struct vfsmount *) malloc(sizeof(struct vfsmount *));\n", true, true);
    RW.InsertText(loc, "\tstruct file *whoop_file = (struct file *) malloc(sizeof(struct file *));\n", true, true);
    RW.InsertText(loc, "\tstruct page *whoop_page = (struct page *) malloc(sizeof(struct page *));\n", true, true);
    RW.InsertText(loc, "\tstruct page **whoop_vpage = (struct page **) malloc(sizeof(struct page *) * 32);\n", true, true);
    RW.InsertText(loc, "\tstruct writeback_control *whoop_writeback_control = (struct writeback_control *) malloc(sizeof(struct writeback_control *));\n", true, true);
    RW.InsertText(loc, "\tstruct address_space *whoop_address_space = (struct address_space *) malloc(sizeof(struct address_space *));\n", true, true);
    RW.InsertText(loc, "\tstruct dentry *whoop_dentry = (struct dentry *) malloc(sizeof(struct dentry *));\n", true, true);
    RW.InsertText(loc, "\tstruct kstatfs *whoop_kstatfs = (struct kstatfs *) malloc(sizeof(struct kstatfs *));\n", true, true);
    RW.InsertText(loc, "\tstruct kstat *whoop_kstat = (struct kstat *) malloc(sizeof(struct kstat *));\n", true, true);
    RW.InsertText(loc, "\tstruct loff_t *whoop_loff_t = (struct loff_t *) malloc(sizeof(struct loff_t *));\n", true, true);
    RW.InsertText(loc, "\tconst char *whoop_buf = (char *) malloc(sizeof(char *));\n", true, true);
    RW.InsertText(loc, "\tconst void **whoop_vbuf = (void **) malloc(sizeof(char *) * 32);\n", true, true);
    RW.InsertText(loc, "\tconst int *whoop_int = (int *) malloc(sizeof(int *));\n\n", true, true);

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
        else if (*j == "void **")
          entry_point_call += "whoop_vbuf, ";
        else if (*j == "u64 *")
          entry_point_call += "NULL, ";
        else if (*j == "u8 *")
          entry_point_call += "NULL, ";
        else if (*j == "struct inode *")
          entry_point_call += "whoop_inode, ";
        else if (*j == "struct super_block *")
          entry_point_call += "whoop_super_block, ";
        else if (*j == "struct vfsmount *")
          entry_point_call += "whoop_vfsmount, ";
        else if (*j == "struct file *")
          entry_point_call += "whoop_file, ";
        else if (*j == "struct page **")
          entry_point_call += "whoop_vpage, ";
        else if (*j == "struct page *")
          entry_point_call += "whoop_page, ";
        else if (*j == "struct writeback_control *")
          entry_point_call += "whoop_writeback_control, ";
        else if (*j == "struct address_space *")
          entry_point_call += "whoop_address_space, ";
        else if (*j == "struct dentry *")
          entry_point_call += "whoop_dentry, ";
        else if (*j == "struct kstatfs *")
          entry_point_call += "whoop_kstatfs, ";
        else if (*j == "struct kstat *")
          entry_point_call += "whoop_kstat, ";
        else if (*j == "loff_t *")
          entry_point_call += "whoop_loff_t, ";
        else if (*j == "loff_t")
          entry_point_call += "&whoop_loff_t, ";
        else if (*j == "char *")
          entry_point_call += "whoop_buf, ";
        else if (*j == "const char *")
          entry_point_call += "whoop_buf, ";
        else if (*j == "size_t")
          entry_point_call += "0, ";
        else if (*j == "int *")
          entry_point_call += "whoop_int, ";
        else if (*j == "int")
          entry_point_call += "0, ";
        else if (*j == "unsigned int")
          entry_point_call += "0, ";
        else if (*j == "long")
          entry_point_call += "0, ";
        else if (*j == "unsigned long")
          entry_point_call += "0, ";
        else if (*j == "u32")
          entry_point_call += "0, ";
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

  string FileSystemRewriteVisitor::GetSharedStructStr(CallExpr *callExpr)
  {
    return "";
  }
}
