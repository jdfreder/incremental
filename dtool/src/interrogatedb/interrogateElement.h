// Filename: interrogateElement.h
// Created by:  drose (11Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef INTERROGATEELEMENT_H
#define INTERROGATEELEMENT_H

#include <dtoolbase.h>

#include "interrogateComponent.h"

class IndexRemapper;

////////////////////////////////////////////////////////////////////
//       Class : InterrogateElement
// Description : An internal representation of a data element, like a
//               data member or a global variable.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG InterrogateElement : public InterrogateComponent {
public:
  INLINE InterrogateElement(InterrogateModuleDef *def = NULL);
  INLINE InterrogateElement(const InterrogateElement &copy);
  INLINE void operator = (const InterrogateElement &copy);

  INLINE bool is_global() const;

  INLINE bool has_scoped_name() const;
  INLINE const string &get_scoped_name() const;

  INLINE TypeIndex get_type() const;
  INLINE bool has_getter() const;
  INLINE FunctionIndex get_getter() const;
  INLINE bool has_setter() const;
  INLINE FunctionIndex get_setter() const;

  void output(ostream &out) const;
  void input(istream &in);

  void remap_indices(const IndexRemapper &remap);

private:
  enum Flags {
    F_global          = 0x0001,
    F_has_getter      = 0x0002,
    F_has_setter      = 0x0004
  };

  int _flags;
  string _scoped_name;
  TypeIndex _type;
  FunctionIndex _getter;
  FunctionIndex _setter;

  friend class InterrogateBuilder;
};

INLINE ostream &operator << (ostream &out, const InterrogateElement &element);
INLINE istream &operator >> (istream &in, InterrogateElement &element);

#include "interrogateElement.I"

#endif
