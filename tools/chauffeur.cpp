//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "chauffeur/Chauffeur.h"
#include "chauffeur/ParseDriverConsumer.h"
#include "clang/Driver/Options.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Option/OptTable.h"

using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

static std::unique_ptr<llvm::opt::OptTable> Options(createDriverOptTable());

static cl::opt<bool> ASTDump("ast-dump",
    cl::desc(Options->getOptionHelpText(options::OPT_ast_dump)));
static cl::opt<bool> ASTList("ast-list",
    cl::desc(Options->getOptionHelpText(options::OPT_ast_list)));
static cl::opt<bool> ASTPrint("ast-print",
    cl::desc(Options->getOptionHelpText(options::OPT_ast_print)));
static cl::opt<std::string> ASTDumpFilter("ast-dump-filter",
    cl::desc(Options->getOptionHelpText(options::OPT_ast_dump_filter)));
static cl::opt<bool> ASTInline("inline",
    cl::desc("Inline all non entry point device driver functions"));

static cl::OptionCategory ToolCategory("Chauffeur options");

namespace chauffeur
{
  class ParseDriverASTAction : public ASTFrontendAction
  {
  public:
      virtual ASTConsumer *CreateASTConsumer(CompilerInstance &CI, StringRef file)
      {
        if (ASTList)
          return clang::CreateASTDeclNodeLister();
        if (ASTDump)
          return clang::CreateASTDumper(ASTDumpFilter);
        if (ASTPrint)
          return clang::CreateASTPrinter(&llvm::outs(), ASTDumpFilter);
        if (ASTInline)
          llvm::errs() << "test" << "\n";
        return new ParseDriverConsumer(&CI, ASTInline);
      }
  };
}

int main(int argc, const char **argv)
{
	CommonOptionsParser op(argc, argv, ToolCategory);
	ClangTool Tool(op.getCompilations(), op.getSourcePathList());
	chauffeur::FileName = op.getSourcePathList()[0].substr(0, op.getSourcePathList()[0].find_last_of("."));
	return Tool.run(newFrontendActionFactory<chauffeur::ParseDriverASTAction>().get());
}
