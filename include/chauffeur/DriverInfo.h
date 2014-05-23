//
// Copyright (c) 2014 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#ifndef DRIVERINFO_H
#define DRIVERINFO_H

#include <list>
#include <map>
#include <string>

namespace chauffeur
{
  using std::list;
  using std::map;
  using std::string;

	class DriverInfo
	{
	private:
    string init_function;
    list<string> entry_points;
		map<string, map<string, string> > entry_point_pairs;

		DriverInfo() {}
		DriverInfo(DriverInfo const&);
		void operator=(DriverInfo const&);

	public:
		static DriverInfo& getInstance()
    {
      static DriverInfo instance;
      return instance;
    }

		void AddEntryPoint(string type, string funcname, string entrypoint);

    string GetInitFunction();

    list<string> GetEntryPoints();

		bool ExistsEntryPointWithName(string name);

		void PrintDriverInfo();
	};
}

#endif // DRIVERINFO_H
