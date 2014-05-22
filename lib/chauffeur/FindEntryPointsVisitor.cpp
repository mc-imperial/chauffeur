//
// Copyright (c) 2014 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "chauffeur/Chauffeur.h"
#include "chauffeur/FindEntryPointsVisitor.h"
#include "clang/AST/ASTContext.h"

namespace chauffeur
{
  using namespace std;

  bool FindEntryPointsVisitor::VisitVarDecl(VarDecl* VD)
  {
    if (!VD->getType()->isRecordType()) return true;

    RecordDecl *BaseRD = VD->getType()->getAs<RecordType>()->getDecl();

    if (!(BaseRD->getNameAsString() == "pci_driver" ||
        BaseRD->getNameAsString() == "dev_pm_ops" ||
        BaseRD->getNameAsString() == "net_device_ops" ||
        BaseRD->getNameAsString() == "ethtool_ops" ||
        BaseRD->getNameAsString() == "test_driver"))
    {
      return true;
    }

    InitListExpr *ILE = cast<InitListExpr>(VD->getInit())->getSyntacticForm();

    for (Stmt::child_range range = ILE->children(); range; ++range)
    {
      DesignatedInitExpr *DIE = cast<DesignatedInitExpr>(*range);
      if (DIE->size() != 1) continue;

      string funcname;

      if (/* pci_driver */
          DIE->getDesignator(0)->getFieldName()->getName() == "probe" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "remove" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "shutdown" ||
          /* dev_pm_ops */
          DIE->getDesignator(0)->getFieldName()->getName() == "suspend" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "resume" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "freeze" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "thaw" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "poweroff" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "restore" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "runtime_suspend" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "runtime_resume" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "runtime_idle" ||
          /* net_device_ops */
          DIE->getDesignator(0)->getFieldName()->getName() == "ndo_open" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "ndo_stop" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "ndo_get_stats64" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "ndo_start_xmit" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "ndo_tx_timeout" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "ndo_validate_addr" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "ndo_change_mtu" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "ndo_fix_features" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "ndo_set_features" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "ndo_set_mac_address" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "ndo_do_ioctl" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "ndo_set_rx_mode" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "ndo_poll_controller" ||
          /* ethtool_ops */
          DIE->getDesignator(0)->getFieldName()->getName() == "get_drvinfo" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "get_regs_len" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "get_link" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "get_settings" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "set_settings" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "get_msglevel" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "set_msglevel" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "get_regs" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "get_wol" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "set_wol" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "get_strings" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "get_sset_count" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "get_ethtool_stats" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "get_ts_info" ||
          /* test_driver */
          DIE->getDesignator(0)->getFieldName()->getName() == "ep1" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "ep2" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "ep3" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "ep4" ||
          DIE->getDesignator(0)->getFieldName()->getName() == "ep5")
        funcname = DIE->getDesignator(0)->getFieldName()->getName();
      else
        continue;

      Expr *expr = cast<ImplicitCastExpr>(DIE->getInit())->getSubExpr();
      while (!isa<DeclRefExpr>(expr))
        expr = cast<ImplicitCastExpr>(expr)->getSubExpr();
      DeclRefExpr *DRE = cast<DeclRefExpr>(expr);

      string fdFileWithExt = Context->getSourceManager().getFilename(DRE->getDecl()->getLocation());
      string fdFile = fdFileWithExt.substr(0, fdFileWithExt.find_last_of("."));

      if ((fdFile.size() > 0) && (fdFile.find(FileName) != string::npos))
      {
        DI->getInstance().AddEntryPoint(BaseRD->getNameAsString(), funcname, DRE->getNameInfo().getName().getAsString());
      }
    }

    return true;
  }

  void FindEntryPointsVisitor::PrintEntryPoints()
  {
    DI->getInstance().PrintDriverInfo();
  }
}
