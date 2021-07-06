/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to display CDC trigger objects.
//-----------------------------------------------------------------------------

#ifdef TRGCDC_DISPLAY

#define TRGCDC_SHORT_NAMES

#include <iostream>
#include <pangomm/init.h>
#include "trg/trg/Utilities.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/Segment.h"
#include "trg/cdc/FrontEnd.h"
#include "trg/cdc/Merger.h"
#include "trg/cdc/HoughPlane.h"
#include "trg/cdc/HoughPlaneMulti2.h"
#include "trg/cdc/Display.h"
#include "trg/cdc/DisplayRphi.h"
#include "trg/cdc/DisplayDrawingAreaHough.h"

using namespace std;

namespace Belle2 {

  TRGCDCDisplayDrawingAreaHough::TRGCDCDisplayDrawingAreaHough(
    TRGCDCDisplay& w,
    int size)
    : TRGCDCDisplayDrawingArea(w, size, 10),
      _scale(1),
      _wireName(false),
      _oldCDC(false),
      _x(0),
      _y(0),
      _scaleX(0), _scaleY(0),
      _winx(0), _winy(0), _winw(0), _winh(0), _wind(0),
      _hp(0),

      _blue(Gdk::Color("blue")),
      _red(Gdk::Color("red")),
      _green(Gdk::Color("green")),
      _black(Gdk::Color("black")),
      _white(Gdk::Color("white")),
      _gray(Gdk::Color("gray")),
      _yellow(Gdk::Color("yellow")),

      _gray0(Gdk::Color("gray90")),
      _gray1(Gdk::Color("gray80")),
      _gray2(Gdk::Color("gray70")),
      _gray3(Gdk::Color("gray60"))
  {


    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
    colormap->alloc_color(_blue);
    colormap->alloc_color(_red);
    colormap->alloc_color(_green);
    colormap->alloc_color(_black);
    colormap->alloc_color(_white);
    colormap->alloc_color(_gray);
    colormap->alloc_color(_yellow);
    colormap->alloc_color(_gray0);
    colormap->alloc_color(_gray1);
    colormap->alloc_color(_gray2);
    colormap->alloc_color(_gray3);

    add_events(Gdk::EXPOSURE_MASK | Gdk::BUTTON_PRESS_MASK);
  }

  TRGCDCDisplayDrawingAreaHough::~TRGCDCDisplayDrawingAreaHough()
  {
  }

  void
  TRGCDCDisplayDrawingAreaHough::on_realize()
  {
    Gtk::DrawingArea::on_realize();
    _window = get_window();
    _window->get_geometry(_winx, _winy, _winw, _winh, _wind);
    if (_hp) {
      _x = _hp->nX() / 2;
      _y = _hp->nY() / 2;
    }
    _gc = Gdk::GC::create(_window);
    _window->set_background(_white);
    _window->clear();
  }

  bool
  TRGCDCDisplayDrawingAreaHough::on_expose_event(GdkEventExpose*)
  {
    Glib::RefPtr<Gdk::Window> window = get_window();
    window->get_geometry(_winx, _winy, _winw, _winh, _wind);
    window->clear();
    draw();
    return true;
  }

  bool
  TRGCDCDisplayDrawingAreaHough::on_button_press_event(GdkEventButton* e)
  {

    //...Clear window as a default...
    on_expose_event((GdkEventExpose*) NULL);

    //...HP there?...
    if (! _hp) return true;

    //...Get cell ID...
    const int cx0 = int(float(e->x) / _scaleX / _hp->xSize());
    const int cy0 = _hp->nY() - int(float(e->y) / _scaleY / _hp->ySize());
    const unsigned sid = _hp->serialId(cx0, cy0);

    //...Get entries...
    const unsigned n = _hp->entry(cx0, cy0);

    //...Information...
    display().information("Cell(" + TRGUtilities::itostring(cx0) + "," +
                          TRGUtilities::itostring(cy0) + ", sid" +
                          TRGUtilities::itostring(sid) + ") " +
                          TRGUtilities::itostring(n) + "hits");

    //...Draw cell...
    _gc->set_foreground(_red);
//  drawCell(cx0, cy0);
    vector<unsigned> r;
    r.push_back(sid);
    drawRegion(r);

    //...Draw TSs if rphi display exists...
    if (display().rphi()) {
      const TRGCDC& cdc = * TRGCDC::getTRGCDC();
      const TCHPlaneMulti2& hp =
        * dynamic_cast<const TCHPlaneMulti2*>(_hp);
      vector<const TCSegment*> list;
      string segs;
      for (unsigned i = 0; i < 5; i++) {
        const vector<unsigned>& l = hp.patternId(i, sid);
        for (unsigned j = 0; j < l.size(); j++) {
          list.push_back(& cdc.segment(i * 2, l[j]));
          if (list.size() == 1) {
            segs = "s" + cdc.segment(i * 2, l[j]).name();
          } else {
            segs += ",s" + cdc.segment(i * 2, l[j]).name();
          }
        }
      }
      display().rphi()->information(segs);
      display().rphi()->area().on_expose_event(0);
      display().rphi()->area().oneShot(list, _red);
    }

    //...Debug...
    _x = xR(e->x);
    _y = yR(e->y);
    cout << "***********************" << endl;
    cout << "x=" << e->x << ",y=" << e->y << endl;
    cout << "cx0=" << cx0 << ",cy0=" << cy0 << endl;
    cout << "************************" << endl;

    return true;
  }

  void
  TRGCDCDisplayDrawingAreaHough::draw(void)
  {
    if (! _hp) return;

    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
    _gc->set_line_attributes(1,
                             Gdk::LINE_SOLID,
                             Gdk::CAP_NOT_LAST,
                             Gdk::JOIN_MITER);

    //...Cal. scale for y coordinate...
    const float xMin = _hp->xMin();
    const float xMax = _hp->xMax();
    const float yMin = _hp->yMin();
    const float yMax = _hp->yMax();
//     const float scaleX = 1 / (xMax - xMin) * _winw;
//     const float scaleY = 1 / (yMax - yMin) * _winh;
    _scaleX = _scale / (xMax - xMin) * _winw;
    _scaleY = _scale / (yMax - yMin) * _winh;

    //...Search maximum...
    unsigned nMax = 0;
    for (unsigned i = 0; i < _hp->nX(); i++) {
      for (unsigned j = 0; j < _hp->nY(); j++) {
        const unsigned n = _hp->entry(i, j);
        if (n > nMax) nMax = n;
      }
    }

    if (nMax == 0)
      cout << "max entry=" << nMax << endl;

    //...Draw...
    for (unsigned i = 0; i < _hp->nX(); i++) {
      for (unsigned j = 0; j < _hp->nY(); j++) {
        const unsigned n = _hp->entry(i, j);
        if (! n) continue;

        //...Decide color...
        if (n == 1)
          _gc->set_foreground(_gray0);
        else if (n == 2)
          _gc->set_foreground(_gray1);
        else if (n == 3)
          _gc->set_foreground(_gray2);
        else if (n == 4)
          _gc->set_foreground(_blue);
        else
          _gc->set_foreground(_red);

        drawCell(i, j);
      }
    }

    //...Draw boundary...
    const int x0 = 0;
    const int y0 = 0;
    const int z0 = toY(y0);
    const int x1 = int(_hp->xSize() * _scaleX);
    const int y1 = int(_hp->ySize() * _scaleY);
    _gc->set_foreground(_gray0);
    _window->draw_rectangle(_gc, true, x0, z0, x1, y1);

//  cout << "TWHDArea ... xMin,xMax,yMin,yMax=" << xMin << "," << xMax
//            << "," << yMin << "," << yMax << endl;
//  cout << "TWHDArea ... winx,winy,winw,winh,wind=" << _winx << "," << _winy
//            << "," << _winw << "," << _winh << "," << _wind << endl;

    //...Draw regions...
    _gc->set_foreground(_green);
    _gc->set_line_attributes(2,
                             Gdk::LINE_SOLID,
                             Gdk::CAP_NOT_LAST,
                             Gdk::JOIN_MITER);
    const vector<vector<unsigned> *>& regions = _hp->regions();
    for (unsigned i = 0; i < (unsigned) regions.size(); i++)
      drawRegion(* regions[i]);

    //...Draw text...
//     _xPositionText = _hp->xMin() + _hp->xSize() * _hp->nX() * 0.1;
//     _yPositionText = _hp->yMin() + _hp->ySize() * _hp->nY() * 0.9 * scale;
//     _window.draw_text(_xPositionText, _yPositionText, _text.c_str());
//     string text = " y_scale=" + dtostring(1/scale);
//     text += " max.peak=" + itostring(_hp->maxEntry());
//     text += " #regions=" + itostring(_hp->regions().length());
//     _yPositionText = _hp->yMin() + _hp->ySize() * _hp->nY() * 0.85 * scale;
//     _window.draw_text(_xPositionText, _yPositionText, text.c_str());

    //...Draw reference line(CDC outer wall)...
//     const float outerWall = log10(88. / 2.);
//     const float yOuterWall = outerWall * scale;
//     _window.draw_segment(xMin, yOuterWall, xMax, yOuterWall, leda_gray2);

//    on_expose_event((GdkEventExpose *) NULL);
  }

  void
  TRGCDCDisplayDrawingAreaHough::resetPosition(void)
  {
    _x = _winw / 2;
    _y = _winh / 2;
  }

  int
  TRGCDCDisplayDrawingAreaHough::xT(double x) const
  {
    const double xx = x - _x + double(_hp->nX()) / 2;
    return int(xx * _hp->xSize() * _scaleX);
  }

  int
  TRGCDCDisplayDrawingAreaHough::yT(double y) const
  {
    const double yy = y - _y + double(_hp->nY()) / 2;
    return - int(yy * _hp->ySize() * _scaleY) + _winh;
  }

  int
  TRGCDCDisplayDrawingAreaHough::xR(double x) const
  {
    return int(x / _hp->xSize() * _scaleX);
  }

  int
  TRGCDCDisplayDrawingAreaHough::yR(double y) const
  {
    return - int((y - double(_winh)) / _hp->ySize() / _scaleY);
  }

  int
  TRGCDCDisplayDrawingAreaHough::drawCell(unsigned xCell, unsigned yCell)
  {
    const float x = float(xCell) * _hp->xSize() * _scaleX;
    const float y = float(yCell) * _hp->ySize() * _scaleY;

    const int x0 = int(x);
    const int y0 = int(y);
    const int z0 = toY(y0);
    const int x1 = int(x + _hp->xSize() * _scaleX) - x0;
    const int y1 = int(y + _hp->ySize() * _scaleY) - y0;

    _window->draw_rectangle(_gc, true, x0, z0, x1, y1);

//     cout << "x0,y0,z0,x1,y1=" << x0 << ","
//          << y0 << "," << z0 << "," << x1 << ","
//          << y1 << endl;

    return 0;
  }

  int
  TRGCDCDisplayDrawingAreaHough:: drawRegion(const std::vector<unsigned>& r)
  {
    for (unsigned i = 0; i < r.size(); i++) {
      const unsigned id = r[i];
      unsigned ix = 0;
      unsigned iy = 0;
      _hp->id(id, ix, iy);

      const float x = float(ix) * _hp->xSize() * _scaleX;
      const float y = float(iy) * _hp->ySize() * _scaleY;

      const int x0 = int(x);
      const int y0 = int(y);
      const int z0 = toY(y0);

      const int x1 = int(x + _hp->xSize() * _scaleX);
      const int y1 = int(y + _hp->ySize() * _scaleY) - y0;
      const int z1 = z0 + y1;

      for (unsigned k = 0; k < 8; k++) {
        if (k % 2) continue;
        unsigned idx = _hp->neighbor(id, k);

        if (idx == id) continue;
        bool found = false;
        for (unsigned l = 0; l < (unsigned) r.size(); l++) {
          if (idx == r[l]) {
            found = true;
            break;
          }
        }
        if (found) continue;
        if (k == 0)
          _window->draw_line(_gc, x0, z0, x1, z0);
        else if (k == 2)
          _window->draw_line(_gc, x1, z0, x1, z1);
        else if (k == 4)
          _window->draw_line(_gc, x0, z1, x1, z1);
        else if (k == 6)
          _window->draw_line(_gc, x0, z0, x0, z1);
      }
    }

    return 0;
  }

} // namespace Belle2

#endif
