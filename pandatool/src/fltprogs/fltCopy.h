// Filename: fltCopy.h
// Created by:  drose (01Nov00)
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

#ifndef FLTCOPY_H
#define FLTCOPY_H

#include <pandatoolbase.h>

#include "cvsCopy.h"

#include <dSearchPath.h>
#include <pointerTo.h>

#include <set>

class FltRecord;
class FltTexture;
class FltExternalReference;

////////////////////////////////////////////////////////////////////
//       Class : FltCopy
// Description : A program to copy Multigen .flt files into the cvs
//               tree.  It copies the base file plus all externally
//               referenced files as well as all textures.
////////////////////////////////////////////////////////////////////
class FltCopy : public CVSCopy {
public:
  FltCopy();

  void run();

protected:
  virtual bool copy_file(const Filename &source, const Filename &dest,
                         CVSSourceDirectory *dir, void *extra_data,
                         bool new_file);

private:
  enum FileType {
    FT_flt,
    FT_texture
  };

  class ExtraData {
  public:
    FileType _type;
    FltTexture *_texture;
  };

  bool copy_flt_file(const Filename &source, const Filename &dest,
                     CVSSourceDirectory *dir);
  bool copy_texture(const Filename &source, const Filename &dest,
                    CVSSourceDirectory *dir, FltTexture *tex,
                    bool new_file);


  typedef set<PT(FltExternalReference)> Refs;
  typedef set<PT(FltTexture)> Textures;

  void scan_flt(FltRecord *record, Refs &refs, Textures &textures);

  DSearchPath _search_path;
};

#endif
