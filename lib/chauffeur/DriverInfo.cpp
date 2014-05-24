//
// Copyright (c) 2014 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "chauffeur/DriverInfo.h"
#include "clang/Tooling/Tooling.h"

namespace chauffeur
{
	std::string FileName;

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
				output.append(j->first + "::" +j->second + "\n");
			}

			output.append("</>\n");
		}

		llvm::raw_ostream *ros = fos;

		ros->write(output.data(), output.size());
	}
}
