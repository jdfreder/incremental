// Filename: dcPackerInterface.cxx
// Created by:  drose (15Jun04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "dcPackerInterface.h"
#include "dcPackerCatalog.h"

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DCPackerInterface::
DCPackerInterface(const string &name) :
  _name(name)
{
  _has_fixed_byte_size = false;
  _fixed_byte_size = 0;
  _num_length_bytes = 0;
  _has_nested_fields = false;
  _num_nested_fields = -1;
  _pack_type = PT_invalid;
  _catalog = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DCPackerInterface::
DCPackerInterface(const DCPackerInterface &copy) :
  _name(copy._name),
  _has_fixed_byte_size(copy._has_fixed_byte_size),
  _fixed_byte_size(copy._fixed_byte_size),
  _num_length_bytes(copy._num_length_bytes),
  _has_nested_fields(copy._has_nested_fields),
  _num_nested_fields(copy._num_nested_fields),
  _pack_type(copy._pack_type)
{
  _catalog = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DCPackerInterface::
~DCPackerInterface() {
  if (_catalog != (DCPackerCatalog *)NULL) {
    delete _catalog;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::calc_num_nested_fields
//       Access: Public, Virtual
//  Description: This flavor of get_num_nested_fields is used during
//               unpacking.  It returns the number of nested fields to
//               expect, given a certain length in bytes (as read from
//               the _num_length_bytes stored in the stream on the
//               push).  This will only be called if _num_length_bytes
//               is nonzero.
////////////////////////////////////////////////////////////////////
int DCPackerInterface::
calc_num_nested_fields(size_t length_bytes) const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::get_nested_field
//       Access: Public, Virtual
//  Description: Returns the DCPackerInterface object that represents
//               the nth nested field.  This may return NULL if there
//               is no such field (but it shouldn't do this if n is in
//               the range 0 <= n < get_num_nested_fields()).
////////////////////////////////////////////////////////////////////
DCPackerInterface *DCPackerInterface::
get_nested_field(int n) const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::pack_double
//       Access: Public, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
pack_double(DCPackData &, double) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::pack_int
//       Access: Public, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
pack_int(DCPackData &, int) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::pack_uint
//       Access: Public, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
pack_uint(DCPackData &, unsigned int) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::pack_int64
//       Access: Public, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
pack_int64(DCPackData &, PN_int64) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::pack_uint64
//       Access: Public, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
pack_uint64(DCPackData &, PN_uint64) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::pack_string
//       Access: Public, Virtual
//  Description: Packs the indicated numeric or string value into the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
pack_string(DCPackData &, const string &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::unpack_double
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
unpack_double(const char *, size_t, size_t &, double &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::unpack_int
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
unpack_int(const char *, size_t, size_t &, int &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::unpack_uint
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
unpack_uint(const char *, size_t, size_t &, unsigned int &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::unpack_int64
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
unpack_int64(const char *, size_t, size_t &, PN_int64 &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::unpack_uint64
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
unpack_uint64(const char *, size_t, size_t &, PN_uint64 &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::unpack_string
//       Access: Public, Virtual
//  Description: Unpacks the current numeric or string value from the
//               stream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
unpack_string(const char *, size_t, size_t &, string &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::unpack_skip
//       Access: Public, Virtual
//  Description: Increments p to the end of the current field without
//               actually unpacking any data.  Returns true on
//               success, false on failure.
////////////////////////////////////////////////////////////////////
bool DCPackerInterface::
unpack_skip(const char *, size_t, size_t &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::get_catalog
//       Access: Public
//  Description: Returns the DCPackerCatalog associated with this
//               field, listing all of the nested fields by name.
////////////////////////////////////////////////////////////////////
const DCPackerCatalog *DCPackerInterface::
get_catalog() const {
  if (_catalog == (DCPackerCatalog *)NULL) {
    ((DCPackerInterface *)this)->make_catalog();
  }
  return _catalog;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::make_catalog
//       Access: Private
//  Description: Called internally to create a new DCPackerCatalog
//               object.
////////////////////////////////////////////////////////////////////
void DCPackerInterface::
make_catalog() {
  nassertv(_catalog == (DCPackerCatalog *)NULL);
  _catalog = new DCPackerCatalog(this);

  if (has_nested_fields()) {
    int num_nested = get_num_nested_fields();
    // num_nested might be -1, indicating there are a dynamic number
    // of fields (e.g. an array).  But in that case, none of the
    // fields will be named anyway, so we don't care about them, so
    // it's ok that the following loop will not visit any fields.
    for (int i = 0; i < num_nested; i++) {
      DCPackerInterface *nested = get_nested_field(i);
      if (nested != (DCPackerInterface *)NULL) {
        nested->r_fill_catalog(_catalog, "", this, i);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerInterface::r_fill_catalog
//       Access: Private
//  Description: Called internally to recursively fill up the new
//               DCPackerCatalog object.
////////////////////////////////////////////////////////////////////
void DCPackerInterface::
r_fill_catalog(DCPackerCatalog *catalog, const string &name_prefix,
               DCPackerInterface *parent, int field_index) {
  string next_name_prefix = name_prefix;

  if (!get_name().empty()) {
    // Record this entry in the catalog.
    next_name_prefix += get_name();
    catalog->add_entry(next_name_prefix, this, parent, field_index);

    next_name_prefix += ".";
  }

  // Add any children.
  if (has_nested_fields()) {
    int num_nested = get_num_nested_fields();
    // As above, it's ok if num_nested is -1.
    for (int i = 0; i < num_nested; i++) {
      DCPackerInterface *nested = get_nested_field(i);
      if (nested != (DCPackerInterface *)NULL) {
        nested->r_fill_catalog(catalog, next_name_prefix, this, i);
      }
    }
  }
}
