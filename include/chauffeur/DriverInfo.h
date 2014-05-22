//
// Copyright (c) 2014 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#ifndef DRIVERINFO_H
#define DRIVERINFO_H

#include <string>
#include <map>

namespace chauffeur
{
  using std::string;
  using std::map;

	class DriverInfo
	{
	private:
		map<string, map<string, string> > entry_points;

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

		bool ExistsEntryPointWithName(string name);

		void PrintDriverInfo();
	};
}

#endif // DRIVERINFO_H
