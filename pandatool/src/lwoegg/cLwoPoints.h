// Filename: cLwoPoints.h
// Created by:  drose (25Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef CLWOPOINTS_H
#define CLWOPOINTS_H

#include <pandatoolbase.h>

#include <lwoPoints.h>
#include <eggVertexPool.h>
#include <pointerTo.h>

#include <map>

class LwoToEggConverter;
class LwoVertexMap;
class CLwoLayer;

////////////////////////////////////////////////////////////////////
//       Class : CLwoPoints
// Description : This class is a wrapper around LwoPoints and stores
//               additional information useful during the
//               conversion-to-egg process.
////////////////////////////////////////////////////////////////////
class CLwoPoints {
public:
  INLINE CLwoPoints(LwoToEggConverter *converter, const LwoPoints *points,
                    CLwoLayer *layer);

  void add_vmap(const LwoVertexMap *lwo_vmap);
  bool get_uv(const string &uv_name, int n, LPoint2f &uv) const;

  void make_egg();
  void connect_egg();

  LwoToEggConverter *_converter;
  CPT(LwoPoints) _points;
  CLwoLayer *_layer;
  PT(EggVertexPool) _egg_vpool;

  // A number of vertex maps of different types may be associated, but
  // we only care about some of the types here.
  typedef map<string, const LwoVertexMap *> VMap;
  VMap _txuv;
  VMap _pick;
};

#include "cLwoPoints.I"

#endif


