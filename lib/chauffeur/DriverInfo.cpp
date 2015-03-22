//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "chauffeur/DriverInfo.h"
#include "clang/Tooling/Tooling.h"

namespace chauffeur
{
	std::string FileName;

	void DriverInfo::SetType(DriverType type)
	{
		driver_type = type;
	}

  void DriverInfo::SetInitFunction(string entrypoint)
	{
		init_function = entrypoint;
	}

	void DriverInfo::AddEntryPoint(string entrypoint, list<string> params)
	{
		entry_points[entrypoint] = params;
	}

	void DriverInfo::AddEntryPointPair(string type, string funcname, string entrypoint)
	{
		entry_point_pairs[type][funcname] = entrypoint;
	}

	void DriverInfo::AddSharedStructInformation(string type, string funcname)
	{
		entry_point_pairs[type][""] = funcname;
	}

	void DriverInfo::AddFunctionPointerInformation(string fp, string type, string info)
	{
		map<string, string> fpInfo;
		fpInfo[type] = info;
		function_pointer_information[fp].push_back(fpInfo);
	}

	void DriverInfo::AddFunctionPointerCallInformation(string fp, string type, string info)
	{
		map<string, string> fpInfo;
		fpInfo[type] = info;
		function_pointer_information[fp].push_back(fpInfo);
	}

	void DriverInfo::MergeFunctionPointerInformation(string left, string right)
	{
		if ((function_pointer_information.find(left) == function_pointer_information.end()) ||
			(function_pointer_information.find(right) == function_pointer_information.end()))
		{
			return;
		}

		function_pointer_information[left].splice(function_pointer_information[left].begin(),
		  function_pointer_information[right]);
		function_pointer_information.erase(right);
	}

	string DriverInfo::GetFunctionPointerInformation(string name)
	{
		for(auto i = function_pointer_information.begin(); i != function_pointer_information.end(); i++)
		{
			for(auto j = i->second.begin(); j != i->second.end(); j++)
			{
				for(auto z = j->begin(); z != j->end(); z++)
				{
					if (z->first != "call")
						continue;
					if (z->second.find(name + "::") != 0)
						continue;

					return i->first;
				}
			}
		}

		return "";
	}

	DriverType DriverInfo::GetType()
	{
		return driver_type;
	}

	string DriverInfo::GetInitFunction()
	{
		return init_function;
	}

	map<string, list<string> > DriverInfo::GetEntryPoints()
	{
		return entry_points;
	}

	bool DriverInfo::ExistsEntryPointWithName(string name)
	{
		bool exists = false;

		for(auto i = entry_points.begin(); i != entry_points.end(); i++)
		{
			if (name == i->first)
			{
				exists = true;
				break;
			}
		}

		if (name == init_function)
		  exists = true;

		return exists;
	}

	bool DriverInfo::IsDriverModule(string name)
	{
		if (name == "pci_driver" || name == "cx_drv" || name == "platform_driver" ||
			name == "dev_pm_ops" ||
			name == "file_operations" || name == "block_device_operations" ||
			name == "cdrom_device_ops" ||
			name == "file_system_type" || name == "inode_operations" ||
			name == "address_space_operations" || name == "super_operations" ||
			name == "net_device_ops" || name == "ethtool_ops" ||
			name == "nfc_ops" ||
			name == "usb_serial_driver" ||
			name == "ps3_system_bus_driver" ||
			name == "nsc_gpio_ops" ||
			name == "drm_i2c_encoder_driver" ||
			name == "drm_encoder_slave_funcs" ||
			name == "drm_encoder_helper_funcs" ||
			name == "drm_encoder_funcs" ||
			name == "drm_connector_helper_funcs" ||
			name == "ps3_system_btda998x_opsus_driver" ||
			name == "test_driver")
		{
			return true;
		}

		return false;
	}

	void DriverInfo::PrintDriverInfo()
	{
		string file = FileName;
		file.append(".info");
		string error_msg;
		llvm::raw_fd_ostream *fos = new llvm::raw_fd_ostream(file.c_str(), error_msg, llvm::sys::fs::F_None);
		if (!error_msg.empty())
		{
			if (llvm::errs().has_colors()) llvm::errs().changeColor(llvm::raw_ostream::RED);
			llvm::errs() << "error: " << error_msg << "\n";
			if (llvm::errs().has_colors()) llvm::errs().resetColor();
			exit(1);
		}

		fos->SetUnbuffered();
		fos->SetUseAtomicWrites(true);

		string output = "";

		for(auto i = entry_point_pairs.begin(); i != entry_point_pairs.end(); i++)
		{
			output.append("<" + i->first + ">\n");

			for(auto j = i->second.begin(); j != i->second.end(); j++)
			{
				output.append(j->first + "::" + j->second + "\n");
			}

			output.append("</>\n");
		}

		llvm::raw_ostream *ros = fos;

		ros->write(output.data(), output.size());
	}

	void DriverInfo::PrintFunctionPointerInfo()
	{
		string file = FileName;
		file.append(".fp.info");
		string error_msg;
		llvm::raw_fd_ostream *fos = new llvm::raw_fd_ostream(file.c_str(), error_msg, llvm::sys::fs::F_None);
		if (!error_msg.empty())
		{
			if (llvm::errs().has_colors()) llvm::errs().changeColor(llvm::raw_ostream::RED);
			llvm::errs() << "error: " << error_msg << "\n";
			if (llvm::errs().has_colors()) llvm::errs().resetColor();
			exit(1);
		}

		fos->SetUnbuffered();
		fos->SetUseAtomicWrites(true);

		string output = "";

		for(auto i = function_pointer_information.begin(); i != function_pointer_information.end(); i++)
		{
			output.append("<" + i->first + ">\n");

			for(auto j = i->second.begin(); j != i->second.end(); j++)
			{
				for(auto z = j->begin(); z != j->end(); z++)
				{
					output.append(z->first + "::" + z->second + "\n");
				}
			}

			output.append("</>\n");
		}

		llvm::raw_ostream *ros = fos;

		ros->write(output.data(), output.size());
	}
}
