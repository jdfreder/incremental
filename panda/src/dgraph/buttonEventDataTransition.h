// Filename: buttonEventDataTransition.h
// Created by:  drose (27Mar00)
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

#ifndef BUTTONEVENTDATATRANSITION_H
#define BUTTONEVENTDATATRANSITION_H

#include "pandabase.h"

#include "nodeTransition.h"
#include "buttonEvent.h"

#include "pvector.h"

class ModifierButtons;

////////////////////////////////////////////////////////////////////
//       Class : ButtonEventDataTransition
// Description : This data graph transition stores a collection of
//               button events that have been generated by the user
//               since the last time the data graph pulsed, in the
//               order they were generated.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ButtonEventDataTransition : public NodeTransition {
private:
  typedef pvector<ButtonEvent> Buttons;

public:
  INLINE ButtonEventDataTransition();
  INLINE ButtonEventDataTransition(const ButtonEventDataTransition &copy);
  INLINE void operator = (const ButtonEventDataTransition &copy);

public:
  // Functions to access and manipulate the set of buttons.
  typedef Buttons::const_iterator const_iterator;
  typedef Buttons::const_iterator iterator;

  INLINE const_iterator begin() const;
  INLINE const_iterator end() const;

  INLINE void clear();
  INLINE void push_back(const ButtonEvent &event);

  void update_mods(ModifierButtons &mods) const;

public:
  virtual NodeTransition *make_copy() const;

  virtual NodeTransition *compose(const NodeTransition *other) const;
  virtual NodeTransition *invert() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual int internal_compare_to(const NodeTransition *other) const;

private:
  Buttons _buttons;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    NodeTransition::init_type();
    register_type(_type_handle, "ButtonEventDataTransition",
                  NodeTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "buttonEventDataTransition.I"

#endif
