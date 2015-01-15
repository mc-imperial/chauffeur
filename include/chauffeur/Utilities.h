//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#ifndef UTILITIES_H
#define UTILITIES_H

#include <sstream>

namespace chauffeur
{
  static string GetPrefix(string str)
  {
    istringstream iss(str);
    vector<string> tokens{istream_iterator<string>{iss}, istream_iterator<string>{}};

    string prefix = "";
    if (tokens.size() == 3)
    {
      prefix = tokens[1];
    }
    else if (tokens.size() == 4)
  {
    prefix = tokens[2];
  }

  return prefix;
}
}

#endif // UTILITIES_H
