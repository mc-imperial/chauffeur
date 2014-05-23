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
		if (funcname == "probe")
			init_function = entrypoint;
		else
		  entry_points.push_back(entrypoint);

		entry_point_pairs[type][funcname] = entrypoint;
	}

	string DriverInfo::GetInitFunction()
	{
		return init_function;
	}

	list<string> DriverInfo::GetEntryPoints()
	{
		return entry_points;
	}

	bool DriverInfo::ExistsEntryPointWithName(string name)
	{
		bool exists = false;

		for (list<string>::iterator i = entry_points.begin(); i != entry_points.end(); ++i)
		{
			if (name == *i)
			{
				exists = true;
				break;
			}
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

		for(map<string, map<string, string> >::iterator i = entry_point_pairs.begin(); i != entry_point_pairs.end(); i++)
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
