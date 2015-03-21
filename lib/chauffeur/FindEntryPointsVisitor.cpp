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
        DI->getInstance().AddEntryPointPair("whoop_driver_ops", "probe",
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

        DI->getInstance().AddEntryPointPair("whoop_driver_ops", "remove",
          declExpr->getFoundDecl()->getNameAsString());
      }

      return true;
    }

    RecordDecl *baseRecDecl = varDecl->getType()->getAs<RecordType>()->getDecl();

    if (!DI->IsDriverModule(baseRecDecl->getNameAsString()))
    {
      return true;
    }

    if (baseRecDecl->getNameAsString() == "file_operations")
      DI->getInstance().SetType(CHAR_DRIVER);
    else if (baseRecDecl->getNameAsString() == "block_device_operations" ||
        baseRecDecl->getNameAsString() == "ps3_system_bus_driver")
      DI->getInstance().SetType(BLOCK_DRIVER);
    else if (baseRecDecl->getNameAsString() == "net_device_ops")
      DI->getInstance().SetType(NETWORK_DRIVER);
    else if (baseRecDecl->getNameAsString() == "file_system_type")
      DI->getInstance().SetType(FILE_SYSTEM);
    else if (baseRecDecl->getNameAsString() == "drm_i2c_encoder_driver")
      DI->getInstance().SetType(GPU_DRIVER);
    else if (baseRecDecl->getNameAsString() == "nfc_ops")
      DI->getInstance().SetType(NFC_DRIVER);
    else if (baseRecDecl->getNameAsString() == "test_driver")
      DI->getInstance().SetType(TEST_DRIVER);

    if (varDecl->getInit() == NULL)
    {
      return true;
    }

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
          /* file_operations */
          desExpr->getDesignator(0)->getFieldName()->getName() == "llseek" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "write" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "read" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "ioctl" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "unlocked_ioctl" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "mmap" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "poll" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "open" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "release" ||
          /* block_device_operations */
          desExpr->getDesignator(0)->getFieldName()->getName() == "getgeo" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "revalidate_disk" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "check_events" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "media_changed" ||
          /* cdrom_device_ops */
          desExpr->getDesignator(0)->getFieldName()->getName() == "drive_status" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "check_events" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "get_last_session" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "reset" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "audio_ioctl" ||
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
          /* nsc_gpio_ops */
          desExpr->getDesignator(0)->getFieldName()->getName() == "gpio_config" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "gpio_dump" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "gpio_get" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "gpio_set" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "gpio_change" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "gpio_current" ||
          /* drm_i2c_encoder_driver */
          desExpr->getDesignator(0)->getFieldName()->getName() == "encoder_init" ||
          /* drm_encoder_slave_funcs */
          desExpr->getDesignator(0)->getFieldName()->getName() == "set_config" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "destroy" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "dpms" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "save" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "restore" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "mode_fixup" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "mode_valid" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "mode_set" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "detect" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "get_modes" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "create_resources" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "set_property" ||
          /* drm_encoder_helper_funcs */
          desExpr->getDesignator(0)->getFieldName()->getName() == "prepare" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "commit" ||
          /* nfc_ops */
          desExpr->getDesignator(0)->getFieldName()->getName() == "dev_up" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "dev_down" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "dep_link_up" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "dep_link_down" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "start_poll" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "stop_poll" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "activate_target" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "deactivate_target" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "im_transceive" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "tm_send" ||
          /* super_operations */
          desExpr->getDesignator(0)->getFieldName()->getName() == "alloc_inode" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "destroy_inode" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "write_inode" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "evict_inode" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "put_super" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "statfs" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "remount_fs" ||
          /* address_space_operations */
          desExpr->getDesignator(0)->getFieldName()->getName() == "readpage" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "writepage" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "write_begin" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "write_end" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "minix_bmap" ||
          /* inode_operations */
          desExpr->getDesignator(0)->getFieldName()->getName() == "readlink" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "follow_link" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "put_link" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "getattr" ||
          /* file_system_type */
          desExpr->getDesignator(0)->getFieldName()->getName() == "mount" ||
          desExpr->getDesignator(0)->getFieldName()->getName() == "kill_sb" ||
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
