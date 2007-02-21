// Filename: sceneGraphAnalyzer.h
// Created by:  drose (02Jul00)
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

#ifndef SCENEGRAPHANALYZER_H
#define SCENEGRAPHANALYZER_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"
#include "pmap.h"
#include "pset.h"

class PandaNode;
class GeomNode;
class Geom;
class GeomVertexData;
class Texture;

////////////////////////////////////////////////////////////////////
//       Class : SceneGraphAnalyzer
// Description : A handy class that can scrub over a scene graph and
//               collect interesting statistics on it.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA SceneGraphAnalyzer {
PUBLISHED:
  SceneGraphAnalyzer();
  ~SceneGraphAnalyzer();

  void clear();
  void add_node(PandaNode *node);

  void write(ostream &out, int indent_level = 0) const;

  INLINE int get_num_nodes() const;
  INLINE int get_num_instances() const;
  INLINE int get_num_transforms() const;
  INLINE int get_num_nodes_with_attribs() const;
  INLINE int get_num_geom_nodes() const;
  INLINE int get_num_geoms() const;
  INLINE int get_num_geom_vertex_datas() const;

  INLINE int get_num_vertices() const;
  INLINE int get_num_normals() const;
  INLINE int get_num_colors() const;
  INLINE int get_num_texcoords() const;
  INLINE int get_num_tris() const;
  INLINE int get_num_lines() const;
  INLINE int get_num_points() const;

  INLINE int get_num_individual_tris() const;
  INLINE int get_num_tristrips() const;
  INLINE int get_num_triangles_in_strips() const;
  INLINE int get_num_trifans() const;
  INLINE int get_num_triangles_in_fans() const;

  INLINE int get_texture_bytes() const;

  INLINE int get_num_long_normals() const;
  INLINE int get_num_short_normals() const;
  INLINE float get_total_normal_length() const;

private:
  void collect_statistics(PandaNode *node, bool under_instance);
  void collect_statistics(GeomNode *geom_node);
  void collect_statistics(const Geom *geom);
  void collect_statistics(Texture *texture);

  typedef pmap<PandaNode *, int> Nodes;
  typedef pset<const GeomVertexData *> VDatas;
  typedef pmap<Texture *, int> Textures;

  Nodes _nodes;
  VDatas _vdatas;
  Textures _textures;

private:
  int _num_nodes;
  int _num_instances;
  int _num_transforms;
  int _num_nodes_with_attribs;
  int _num_geom_nodes;
  int _num_geoms;
  int _num_geom_vertex_datas;

  int _num_vertices;
  int _num_normals;
  int _num_colors;
  int _num_texcoords;
  int _num_tris;
  int _num_lines;
  int _num_points;

  int _num_individual_tris;
  int _num_tristrips;
  int _num_triangles_in_strips;
  int _num_trifans;
  int _num_triangles_in_fans;

  int _texture_bytes;

  int _num_long_normals;
  int _num_short_normals;
  float _total_normal_length;
};

#include "sceneGraphAnalyzer.I"

#endif
