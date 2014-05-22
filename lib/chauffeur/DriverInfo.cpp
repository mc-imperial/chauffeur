//
// Copyright (c) 2014 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "chauffeur/DriverInfo.h"
#include "clang/Tooling/Tooling.h"

namespace chauffeur
{
	std::string FileName;

	void DriverInfo::AddEntryPoint(string type, string funcname, string entrypoint)
	{
		entry_points[type][funcname] = entrypoint;
	}

	bool DriverInfo::ExistsEntryPointWithName(string name)
	{
		bool exists = false;

		for(map<string, map<string, string> >::iterator i = entry_points.begin(); i != entry_points.end(); i++)
		{
			for(map<string, string>::iterator j = i->second.begin(); j != i->second.end(); j++)
			{
				if (name == j->second)
				{
					exists = true;
					break;
				}
			}

			if (exists) break;
		}

		return exists;
	}

	void DriverInfo::PrintDriverInfo()
	{
		string file = FileName;
		file.append(".info");
		string error_msg;
		llvm::raw_fd_ostream *FOS = new llvm::raw_fd_ostream(file.c_str(), error_msg, llvm::sys::fs::F_None);
		if (!error_msg.empty())
		{
			if (llvm::errs().has_colors()) llvm::errs().changeColor(llvm::raw_ostream::RED);
			llvm::errs() << "error: " << error_msg << "\n";
			if (llvm::errs().has_colors()) llvm::errs().resetColor();
			exit(1);
		}

		FOS->SetUnbuffered();
		FOS->SetUseAtomicWrites(true);

		string output = "";

		for(map<string, map<string, string> >::iterator i = entry_points.begin(); i != entry_points.end(); i++)
		{
			output.append("<" + i->first + ">\n");

			for(map<string, string>::iterator j = i->second.begin(); j != i->second.end(); j++)
			{
				output.append(j->first + "::" +j->second + "\n");
			}

			output.append("</>\n");
		}

		llvm::raw_ostream *ros = FOS;

		ros->write(output.data(), output.size());
	}
}
