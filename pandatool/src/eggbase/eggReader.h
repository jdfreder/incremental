// Filename: eggReader.h
// Created by:  drose (14Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef EGGREADER_H
#define EGGREADER_H

#include <pandatoolbase.h>

#include "eggBase.h"


////////////////////////////////////////////////////////////////////
//       Class : EggReader
// Description : This is the base class for a program that reads egg
//               files, but doesn't write an egg file.
////////////////////////////////////////////////////////////////////
class EggReader : virtual public EggBase {
public:
  EggReader();

  virtual EggReader *as_reader();

protected:
  virtual bool handle_args(Args &args);

  bool _force_complete;
};

#endif


