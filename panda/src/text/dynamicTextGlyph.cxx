// Filename: dynamicTextGlyph.cxx
// Created by:  drose (09Feb02)
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

#include "dynamicTextGlyph.h"

#ifdef HAVE_FREETYPE

#include "dynamicTextPage.h"
#include "geomTextGlyph.h"
#include "qpgeomTextGlyph.h"
#include "qpgeomVertexData.h"
#include "qpgeomVertexFormat.h"
#include "qpgeomTristrips.h"
#include "qpgeomVertexWriter.h"
#include "textureAttrib.h"
#include "transparencyAttrib.h"
#include "renderState.h"
#include "config_gobj.h"

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DynamicTextGlyph::
~DynamicTextGlyph() {
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::get_row
//       Access: Public
//  Description: Returns a pointer to the first byte in the pixel
//               buffer associated with the leftmost pixel in the
//               indicated row, where 0 is the topmost row and _y_size
//               - _margin * 2 - 1 is the bottommost row.
////////////////////////////////////////////////////////////////////
unsigned char *DynamicTextGlyph::
get_row(int y) {
  nassertr(y >= 0 && y < _y_size - _margin * 2, (unsigned char *)NULL);
  nassertr(_page != (DynamicTextPage *)NULL, (unsigned char *)NULL);

  // First, offset y by the glyph's start.
  y += _y + _margin;
  // Also, get the x start.
  int x = _x + _margin;

  // Invert y.
  y = _page->get_y_size() - 1 - y;

  int offset = (y * _page->get_x_size()) + x;
  return _page->modify_ram_image() + offset; 
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::erase
//       Access: Public
//  Description: Erases the glyph from the texture map.
////////////////////////////////////////////////////////////////////
void DynamicTextGlyph::
erase() {
  nassertv(_page != (DynamicTextPage *)NULL);
  nassertv(_page->has_ram_image());

  int ysizetop = _page->get_y_size() - 1;
  int width = _page->get_x_size();
  unsigned char *buffer = _page->modify_ram_image();

  for (int y = _y; y < _y + _y_size; y++) {
    int offset = (ysizetop - y) * width + _x;
    memset(buffer + offset, 0, _x_size);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextGlyph::make_geom
//       Access: Public
//  Description: Creates the actual geometry for the glyph.  The
//               parameters bitmap_top and bitmap_left are from
//               FreeType, and indicate the position of the top left
//               corner of the bitmap relative to the glyph's origin.
//               The advance number represents the number of pixels
//               the pen should be advanced after drawing this glyph.
////////////////////////////////////////////////////////////////////
void DynamicTextGlyph::
make_geom(int bitmap_top, int bitmap_left, float advance, float poly_margin, 
          float tex_x_size, float tex_y_size,
          float font_pixels_per_unit, float tex_pixels_per_unit) {
  nassertv(_page != (DynamicTextPage *)NULL);

  // This function should not be called twice.
  nassertv(_geom_count == 0);

  tex_x_size += _margin * 2;
  tex_y_size += _margin * 2;

  // Determine the corners of the rectangle in geometric units.
  float tex_poly_margin = poly_margin / tex_pixels_per_unit;
  float origin_y = bitmap_top / font_pixels_per_unit;
  float origin_x = bitmap_left / font_pixels_per_unit;
  float top = origin_y + tex_poly_margin;
  float left = origin_x - tex_poly_margin;
  float bottom = origin_y - tex_y_size / tex_pixels_per_unit - tex_poly_margin;
  float right = origin_x + tex_x_size / tex_pixels_per_unit + tex_poly_margin;

  // And the corresponding corners in UV units.
  float uv_top = 1.0f - (float)(_y - poly_margin) / _page->get_y_size();
  float uv_left = (float)(_x - poly_margin) / _page->get_x_size();
  float uv_bottom = 1.0f - (float)(_y + poly_margin + tex_y_size) / _page->get_y_size();
  float uv_right = (float)(_x + poly_margin + tex_x_size) / _page->get_x_size();
  // Create a corresponding tristrip.
  if (use_qpgeom) {
    PT(qpGeomVertexData) vdata = new qpGeomVertexData
      (string(), qpGeomVertexFormat::get_v3t2(),
       qpGeomUsageHint::UH_static);
    qpGeomVertexWriter vertex(vdata, InternalName::get_vertex());
    qpGeomVertexWriter texcoord(vdata, InternalName::get_texcoord());

    vertex.add_data3f(left, 0, top);
    vertex.add_data3f(left, 0, bottom);
    vertex.add_data3f(right, 0, top);
    vertex.add_data3f(right, 0, bottom);
    
    texcoord.add_data2f(uv_left, uv_top);
    texcoord.add_data2f(uv_left, uv_bottom);
    texcoord.add_data2f(uv_right, uv_top);
    texcoord.add_data2f(uv_right, uv_bottom);
    
    PT(qpGeomTristrips) strip = new qpGeomTristrips(qpGeomUsageHint::UH_static);
    strip->add_consecutive_vertices(0, 4);
    strip->close_primitive();
    
    PT(qpGeom) geom = new qpGeomTextGlyph(this);
    geom->set_vertex_data(vdata);
    geom->add_primitive(strip);
    
    _geom = geom;
    
    // The above will increment our _geom_count to 1.  Reset it back
    // down to 0, since our own internal Geom doesn't count.
    nassertv(_geom_count == 1);
    _geom_count--;

  } else {
    PT(Geom) geom = new GeomTextGlyph(this);
    _geom = geom;
    
    // The above will increment our _geom_count to 1.  Reset it back
    // down to 0, since our own internal Geom doesn't count.
    nassertv(_geom_count == 1);
    _geom_count--;
    
    PTA_Vertexf coords;
    coords.push_back(Vertexf(left, 0, top));
    coords.push_back(Vertexf(left, 0, bottom));
    coords.push_back(Vertexf(right, 0, top));
    coords.push_back(Vertexf(right, 0, bottom));
    
    PTA_TexCoordf texcoords;
    texcoords.push_back(TexCoordf(uv_left, uv_top));
    texcoords.push_back(TexCoordf(uv_left, uv_bottom));
    texcoords.push_back(TexCoordf(uv_right, uv_top));
    texcoords.push_back(TexCoordf(uv_right, uv_bottom));
    
    PTA_Colorf colors;
    colors.push_back(Colorf(1.0f, 1.0f, 1.0f, 1.0f));
    
    PTA_int lengths;
    lengths.push_back(4);
    
    geom->set_coords(coords);
    geom->set_texcoords(texcoords, G_PER_VERTEX);
    geom->set_colors(colors, G_OVERALL);
    geom->set_lengths(lengths);
    geom->set_num_prims(1);
  }
    
  _state = RenderState::make(TextureAttrib::make(_page),
                             TransparencyAttrib::make(TransparencyAttrib::M_alpha));

  _advance = advance / font_pixels_per_unit;
}


#endif  // HAVE_FREETYPE
