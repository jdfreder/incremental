// Filename: displayRegion.h
// Created by:  mike (09Jan97)
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
#ifndef DISPLAYREGION_H
#define DISPLAYREGION_H

#include "pandabase.h"

#include "referenceCount.h"
#include "camera.h"
#include "nodeChain.h"

#include "plist.h"

class GraphicsLayer;
class GraphicsChannel;
class GraphicsWindow;
class GraphicsPipe;
class CullHandler;
class qpCamera;

////////////////////////////////////////////////////////////////////
//       Class : DisplayRegion
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DisplayRegion : public ReferenceCount {
public:
  DisplayRegion(GraphicsLayer *);
  DisplayRegion(GraphicsLayer *,
                const float l, const float r,
                const float b, const float t);
  DisplayRegion(int xsize, int ysize);
private:
  DisplayRegion(const DisplayRegion &);
  void operator = (const DisplayRegion &);

public:
  ~DisplayRegion();

PUBLISHED:
  INLINE void get_dimensions(float &l, float &r, float &b, float &t) const;
  INLINE float get_left() const;
  INLINE float get_right() const;
  INLINE float get_bottom() const;
  INLINE float get_top() const;
  void set_dimensions(float l, float r, float b, float t);

  INLINE GraphicsLayer *get_layer() const;
  GraphicsChannel *get_channel() const;
  GraphicsWindow *get_window() const;
  GraphicsPipe *get_pipe() const;

  void set_camera(Camera *camera);
  INLINE Camera *get_camera() const;

  void set_qpcamera(const NodeChain &camera);
  INLINE const NodeChain &get_qpcamera() const;

  INLINE void set_cull_frustum(LensNode *cull_frustum);
  INLINE LensNode *get_cull_frustum() const;

  INLINE void set_active(bool active);
  INLINE bool is_active() const;

  INLINE void compute_pixels(const int x, const int y);
  INLINE void get_pixels(int &pl, int &pr, int &pb, int &pt) const;
  INLINE void get_region_pixels(int &xo, int &yo, int &w, int &h) const;

  INLINE int get_pixel_width() const;
  INLINE int get_pixel_height() const;

  void output(ostream &out) const;

public:
  void win_display_regions_changed();

protected:

  float _l;
  float _r;
  float _b;
  float _t;

  int _pl;
  int _pr;
  int _pb;
  int _pt;

  GraphicsLayer *_layer;
  PT(Camera) _camera;
  NodeChain _qpcamera;
  qpCamera *_camera_node;
  PT(LensNode) _cull_frustum;

  bool _active;

  friend class GraphicsLayer;
};

INLINE ostream &operator << (ostream &out, const DisplayRegion &dr);

#include "displayRegion.I"

#endif /* DISPLAYREGION_H */
