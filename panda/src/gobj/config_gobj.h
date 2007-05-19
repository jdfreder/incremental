// Filename: config_gobj.h
// Created by:  drose (01Oct99)
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

#ifndef CONFIG_GOBJ_H
#define CONFIG_GOBJ_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableInt.h"
#include "configVariableEnum.h"
#include "configVariableDouble.h"
#include "configVariableFilename.h"
#include "configVariableString.h"

NotifyCategoryDecl(gobj, EXPCL_PANDA, EXPTP_PANDA);

enum AutoTextureScale {
  ATS_none,
  ATS_down,
  ATS_up
};
EXPCL_PANDA ostream &operator << (ostream &out, AutoTextureScale ats);
EXPCL_PANDA istream &operator >> (istream &in, AutoTextureScale &ats);

// Configure variables for gobj package.
extern EXPCL_PANDA ConfigVariableInt max_texture_dimension;
extern EXPCL_PANDA ConfigVariableDouble texture_scale;
extern EXPCL_PANDA ConfigVariableList exclude_texture_scale;


extern EXPCL_PANDA ConfigVariableBool keep_texture_ram;
extern EXPCL_PANDA ConfigVariableBool preload_textures;
extern EXPCL_PANDA ConfigVariableBool compressed_textures;
extern EXPCL_PANDA ConfigVariableBool vertex_buffers;
extern EXPCL_PANDA ConfigVariableBool vertex_arrays;
extern EXPCL_PANDA ConfigVariableBool display_lists;
extern EXPCL_PANDA ConfigVariableBool hardware_animated_vertices;
extern EXPCL_PANDA ConfigVariableBool hardware_point_sprites;
extern EXPCL_PANDA ConfigVariableBool matrix_palette;
extern EXPCL_PANDA ConfigVariableBool display_list_animation;
extern EXPCL_PANDA ConfigVariableBool connect_triangle_strips;
extern EXPCL_PANDA ConfigVariableBool preserve_triangle_strips;

extern EXPCL_PANDA ConfigVariableEnum<AutoTextureScale> textures_power_2;
extern EXPCL_PANDA ConfigVariableEnum<AutoTextureScale> textures_square;
extern EXPCL_PANDA ConfigVariableBool textures_header_only;

extern EXPCL_PANDA ConfigVariableInt geom_cache_size;
extern EXPCL_PANDA ConfigVariableInt geom_cache_min_frames;

extern EXPCL_PANDA ConfigVariableDouble default_near;
extern EXPCL_PANDA ConfigVariableDouble default_far;
extern EXPCL_PANDA ConfigVariableDouble default_fov;
extern EXPCL_PANDA ConfigVariableDouble default_iod;
extern EXPCL_PANDA ConfigVariableDouble default_converge;
extern EXPCL_PANDA ConfigVariableDouble default_keystone;

extern EXPCL_PANDA ConfigVariableFilename vertex_save_file_directory;
extern EXPCL_PANDA ConfigVariableString vertex_save_file_prefix;
extern EXPCL_PANDA ConfigVariableInt vertex_data_small_size;

#endif


