// Filename: cppTemplateScope.h
// Created by:  drose (28Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef CPPTEMPLATESCOPE_H
#define CPPTEMPLATESCOPE_H

#include <dtoolbase.h>

#include "cppScope.h"
#include "cppTemplateParameterList.h"

///////////////////////////////////////////////////////////////////
// 	 Class : CPPTemplateScope
// Description : This is an implicit scope that is created following
//               the appearance of a "template<class x, class y>" or
//               some such line in a C++ file.  It simply defines the
//               template parameters.
////////////////////////////////////////////////////////////////////
class CPPTemplateScope : public CPPScope {
public:
  CPPTemplateScope(CPPScope *parent_scope);

  void add_template_parameter(CPPDeclaration *param);
 
  virtual void add_declaration(CPPDeclaration *decl, CPPScope *global_scope,
			       CPPPreprocessor *preprocessor,
			       const cppyyltype &pos);
  virtual void add_enum_value(CPPInstance *inst);
  virtual void define_extension_type(CPPExtensionType *type);
  virtual void define_namespace(CPPNamespace *scope);
  virtual void add_using(CPPUsing *using_decl, CPPScope *global_scope,
			 CPPPreprocessor *error_sink = NULL);

  virtual bool is_fully_specified() const;

  virtual string get_simple_name() const;
  virtual string get_local_name(CPPScope *scope = NULL) const;
  virtual string get_fully_scoped_name() const;

  virtual void output(ostream &out, CPPScope *scope) const;

  virtual CPPTemplateScope *as_template_scope();

  CPPTemplateParameterList _parameters;
};

#endif
