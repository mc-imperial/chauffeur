//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#ifndef DRIVERTYPE_H
#define DRIVERTYPE_H

namespace chauffeur
{
  enum DriverType
  {
    TEST_DRIVER,
    CHAR_DRIVER,
    BLOCK_DRIVER,
    NETWORK_DRIVER,
    GPU_DRIVER,
    NFC_DRIVER,
    USB_DRIVER,
    FILE_SYSTEM
  };
}

#endif // DRIVERTYPE_H
