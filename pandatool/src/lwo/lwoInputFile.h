// Filename: lwoInputFile.h
// Created by:  drose (24Apr01)
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

#ifndef LWOINPUTFILE_H
#define LWOINPUTFILE_H

#include <pandatoolbase.h>

#include "iffInputFile.h"

#include <luse.h>

////////////////////////////////////////////////////////////////////
//       Class : LwoInputFile
// Description : A specialization of IffInputFile to handle reading a
//               Lightwave Object file.
////////////////////////////////////////////////////////////////////
class LwoInputFile : public IffInputFile {
public:
  LwoInputFile();
  ~LwoInputFile();

  INLINE double get_lwo_version() const;
  INLINE void set_lwo_version(double version);

  int get_vx();
  LVecBase3f get_vec3();
  Filename get_filename();

protected:
  virtual IffChunk *make_new_chunk(IffId id);

private:
  double _lwo_version;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    IffInputFile::init_type();
    register_type(_type_handle, "LwoInputFile",
                  IffInputFile::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "lwoInputFile.I"

#endif


