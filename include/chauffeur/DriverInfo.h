//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#ifndef DRIVERINFO_H
#define DRIVERINFO_H

#include "chauffeur/DriverType.h"
#include "clang/AST/ASTContext.h"

namespace chauffeur
{
  using namespace clang;
  using namespace std;

	class DriverInfo
	{
	private:
    string init_function;
		map<string, list<string> > entry_points;
    map<string, map<string, string> > entry_point_pairs;
    map<string, list<map<string, string> > > function_pointer_information;
    DriverType driver_type;

		DriverInfo() {}
		DriverInfo(DriverInfo const&);
		void operator=(DriverInfo const&);

	public:
		static DriverInfo& getInstance()
    {
      static DriverInfo instance;
      return instance;
    }

    void SetType(DriverType type);
    void SetInitFunction(string entrypoint);

    void AddEntryPoint(string entrypoint, list<string> params);
		void AddEntryPointPair(string type, string funcname, string entrypoint);
    void AddSharedStructInformation(string type, string funcname);

    void AddFunctionPointerInformation(string fp, string type, string info);
    void AddFunctionPointerCallInformation(string fp, string type, string info);
    void MergeFunctionPointerInformation(string left, string right);
    string GetFunctionPointerInformation(string name);

    DriverType GetType();
    string GetInitFunction();
    map<string, list<string> > GetEntryPoints();

		bool ExistsEntryPointWithName(string name);
    bool IsDriverModule(string name);

		void PrintDriverInfo();
    void PrintFunctionPointerInfo();
	};
}

#endif // DRIVERINFO_H
