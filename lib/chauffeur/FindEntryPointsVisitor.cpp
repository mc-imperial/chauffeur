//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "chauffeur/Chauffeur.h"
#include "chauffeur/DriverType.h"
#include "chauffeur/FindEntryPointsVisitor.h"
#include "clang/AST/ASTContext.h"

namespace chauffeur
{
  using namespace std;

  bool FindEntryPointsVisitor::VisitVarDecl(VarDecl* varDecl)
  {
    if (!varDecl->getType()->isRecordType())
    {
      if (varDecl->getNameAsString() == "_whoop_init")
      {
        ImplicitCastExpr *implExpr = cast<ImplicitCastExpr>(varDecl->getInit());
        if (implExpr == NULL)
        {
          return true;
        }

        ImplicitCastExpr *implExpr2 = cast<ImplicitCastExpr>(implExpr->getSubExpr());
        DeclRefExpr *declExpr = cast<DeclRefExpr>(implExpr2->getSubExpr());
        DI->getInstance().AddEntryPointPair("char_driver", "probe",
          declExpr->getFoundDecl()->getNameAsString());
        DI->getInstance().SetInitFunction(declExpr->getFoundDecl()->getNameAsString());
      }
      else if (varDecl->getNameAsString() == "_whoop_exit")
      {
        ImplicitCastExpr *implExpr = cast<ImplicitCastExpr>(varDecl->getInit());
        if (implExpr == NULL)
        {
          return true;
        }

        DeclRefExpr *declExpr = cast<DeclRefExpr>(implExpr->getSubExpr());

        list<string> func_params;
        func_params.push_back("void");
        DI->getInstance().AddEntryPoint(declExpr->getFoundDecl()->getNameAsString(), func_params);

        DI->getInstance().AddEntryPointPair("char_driver", "remove",
          declExpr->getFoundDecl()->getNameAsString());
      }

      return true;
    }

    RecordDecl *baseRecDecl = varDecl->getType()->getAs<RecordType>()->getDecl();

    if (!DI->IsDriverModule(baseRecDecl->getNameAsString()))
    {
      return true;
    }

    if (baseRecDecl->getNameAsString() == "net_device_ops")
      DI->getInstance().SetType(NETWORK_DRIVER);
    else if (baseRecDecl->getNameAsString() == "test_driver")
      DI->getInstance().SetType(TEST_DRIVER);
    else if (baseRecDecl->getNameAsString() == "file_operations")
      DI->getInstance().SetType(CHAR_DRIVER);

    InitListExpr *initListExpr = cast<InitListExpr>(varDecl->getInit())->getSyntacticForm();

    for (auto range = initListExpr->children(); range; ++range)
    {
      DesignatedInitExpr *desExpr = cast<DesignatedInitExpr>(*range);
      if (desExpr->size() != 1) continue;

      string funcname;

      if (/* pci_driver */
          desExpr->getDesignator(0)->getFieldName()->getName() == "probe" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "remove" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "shutdown" ||
          /* dev_pm_ops */
          desExpr->getDesignator(0)->getFieldName()->getName() == "suspend" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "resume" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "freeze" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "thaw" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "poweroff" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "restore" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "runtime_suspend" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "runtime_resume" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "runtime_idle" ||
          /* net_device_ops */
          desExpr->getDesignator(0)->getFieldName()->getName() == "ndo_open" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "ndo_stop" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "ndo_get_stats64" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "ndo_start_xmit" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "ndo_tx_timeout" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "ndo_validate_addr" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "ndo_change_mtu" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "ndo_fix_features" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "ndo_set_features" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "ndo_set_mac_address" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "ndo_do_ioctl" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "ndo_set_rx_mode" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "ndo_poll_controller" ||
          /* ethtool_ops */
          desExpr->getDesignator(0)->getFieldName()->getName() == "get_drvinfo" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "get_regs_len" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "get_link" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "get_settings" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "set_settings" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "get_msglevel" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "set_msglevel" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "get_regs" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "get_wol" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "set_wol" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "get_strings" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "get_sset_count" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "get_ethtool_stats" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "get_ts_info" ||
          /* file_operations */
          desExpr->getDesignator(0)->getFieldName()->getName() == "llseek" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "write" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "read" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "ioctl" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "unlocked_ioctl" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "open" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "release" ||
          /* test_driver */
          desExpr->getDesignator(0)->getFieldName()->getName() == "ep1" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "ep2" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "ep3" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "ep4" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "ep5")
        funcname = desExpr->getDesignator(0)->getFieldName()->getName();
      else
        continue;

      Expr *expr = cast<ImplicitCastExpr>(desExpr->getInit())->getSubExpr();
      while (!isa<DeclRefExpr>(expr))
        expr = cast<ImplicitCastExpr>(expr)->getSubExpr();
      DeclRefExpr *declExpr = cast<DeclRefExpr>(expr);

      string fdFileWithExt = Context->getSourceManager().getFilename(declExpr->getDecl()->getLocation());
      string fdFile = fdFileWithExt.substr(0, fdFileWithExt.find_last_of("."));

      if ((fdFile.size() > 0) && (fdFile.find(FileName) != string::npos))
      {
        ValueDecl *value = declExpr->getDecl();
        if (!isa<FunctionDecl>(value))
          continue;

        if (funcname == "probe")
        {
          DI->getInstance().SetInitFunction(declExpr->getNameInfo().getName().getAsString());
        }
        else
        {
          list<string> func_params;
          FunctionDecl *func = cast<FunctionDecl>(value);
          for (auto i = func->param_begin(), e = func->param_end(); i != e; ++i)
          {
            ValueDecl *paramVal = cast<ValueDecl>(*i);
            func_params.push_back(paramVal->getType().getAsString(Context->getPrintingPolicy()));
          }

          DI->getInstance().AddEntryPoint(declExpr->getNameInfo().getName().getAsString(), func_params);
        }

        DI->getInstance().AddEntryPointPair(baseRecDecl->getNameAsString(),
          funcname, declExpr->getNameInfo().getName().getAsString());
      }
    }

    return true;
  }
}
