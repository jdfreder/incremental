// Filename: interrogateFunction.h
// Created by:  drose (01Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef INTERROGATEFUNCTION_H
#define INTERROGATEFUNCTION_H

#include <dtoolbase.h>

#include "interrogateComponent.h"

#include <vector>

class IndexRemapper;

////////////////////////////////////////////////////////////////////
// 	 Class : InterrogateFunction
// Description : An internal representation of a function.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL InterrogateFunction : public InterrogateComponent {
public:
  INLINE InterrogateFunction(InterrogateModuleDef *def = NULL);
  INLINE InterrogateFunction(const InterrogateFunction &copy);
  INLINE void operator = (const InterrogateFunction &copy);

  INLINE bool is_global() const;
  INLINE bool is_virtual() const;
  INLINE bool is_method() const;
  INLINE TypeIndex get_class() const;

  INLINE bool has_scoped_name() const;
  INLINE const string &get_scoped_name() const;

  INLINE bool has_comment() const;
  INLINE const string &get_comment() const;

  INLINE bool has_prototype() const;
  INLINE const string &get_prototype() const;

  INLINE int number_of_c_wrappers() const;
  INLINE FunctionWrapperIndex get_c_wrapper(int n) const;

  INLINE int number_of_python_wrappers() const;
  INLINE FunctionWrapperIndex get_python_wrapper(int n) const;

  void output(ostream &out) const;
  void input(istream &in);

  void remap_indices(const IndexRemapper &remap);

private:
  enum Flags {
    F_global          = 0x0001,
    F_virtual         = 0x0002,
    F_method          = 0x0004
  };

  int _flags;
  string _scoped_name;
  string _comment;
  string _prototype;
  TypeIndex _class;

  typedef vector<FunctionWrapperIndex> Wrappers;
  Wrappers _c_wrappers;
  Wrappers _python_wrappers;
  friend class InterrogateBuilder;
};

INLINE ostream &operator << (ostream &out, const InterrogateFunction &function);
INLINE istream &operator >> (istream &in, InterrogateFunction &function);

#include "interrogateFunction.I"

#endif
