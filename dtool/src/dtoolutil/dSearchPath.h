// Filename: dSearchPath.h
// Created by:  drose (01Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PANDASEARCHPATH_H
#define PANDASEARCHPATH_H

#include <dtoolbase.h>

#include "filename.h"

#include <vector>

///////////////////////////////////////////////////////////////////
// 	 Class : DSearchPath
// Description : This class stores a list of directories that can be
//               searched, in order, to locate a particular file.  It
//               is normally constructed by passing it a traditional
//               searchpath-style string, e.g. a list of directory
//               names delimited by spaces or colons, but it can also
//               be built up explicitly.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL DSearchPath {
public:
  class Results {
  PUBLISHED:
    Results();
    Results(const Results &copy);
    void operator = (const Results &copy);
    ~Results();

    void clear();
    int get_num_files() const;
    Filename get_file(int n) const;

  private:
    typedef vector<Filename> Files;
    Files _files;
    friend class DSearchPath;
  };

PUBLISHED:
  DSearchPath();
  DSearchPath(const string &path, const string &delimiters = ": \t\n");
  DSearchPath(const DSearchPath &copy);
  void operator = (const DSearchPath &copy);
  ~DSearchPath();

  void clear();
  void append_directory(const Filename &directory);
  void prepend_directory(const Filename &directory);
  void append_path(const string &path,
		   const string &delimiters = ": \t\n");

  int get_num_directories() const;
  Filename get_directory(int n) const;

  Filename find_file(const Filename &filename) const;
  int find_all_files(const Filename &filename, Results &results) const;

  INLINE static Filename 
  search_path(const Filename &filename, const string &path,
	      const string &delimiters = ": \t\n");

  void output(ostream &out, const string &separator = ":") const;
  void write(ostream &out, int indent_level = 0) const;

private:
  typedef vector<Filename> Directories;
  Directories _directories;
};

INLINE ostream &operator << (ostream &out, const DSearchPath &sp) {
  sp.output(out);
  return out;
}

#include "dSearchPath.I"

#endif
