//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TWindowHoughDrawingArea.cc
// Section  : Tracking CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display tracking object.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef TRASAN_WINDOW_GTK

#include <iostream>
#include "tracking/modules/trasan/TWindowHoughDrawingArea.h"
#include "tracking/modules/trasan/Trasan.h"
#include "trg/cdc/WireHit.h"
#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/TTrackBase.h"
#include "tracking/modules/trasan/TSegment.h"
#include "tracking/modules/trasan/TTrack.h"

#include "tracking/modules/trasan/THoughPlane.h"

namespace Belle {

  TWindowHoughDrawingArea::TWindowHoughDrawingArea(int size)
    : _scale(double(size) / 2),
      _axial(true),
      _stereo(false),
      _wireName(false),
      _x(0),
      _y(0),
      _hp(0)
  {
    _blue = Gdk::Color("blue");
    _red = Gdk::Color("red");
    _green = Gdk::Color("green");
    _black = Gdk::Color("black");
    _white = Gdk::Color("white");
    _gray = Gdk::Color("gray");
    _yellow = Gdk::Color("yellow");

//     _gray0 = Gdk::Color("dcdddd");
//     _gray1 = Gdk::Color("afafb0");
//     _gray2 = Gdk::Color("949495");
//     _gray3 = Gdk::Color("595857");
    _gray0 = Gdk::Color("gray90");
    _gray1 = Gdk::Color("gray80");
    _gray2 = Gdk::Color("gray70");
    _gray3 = Gdk::Color("gray60");

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

  TWindowHoughDrawingArea::~TWindowHoughDrawingArea()
  {
  }

  void
  TWindowHoughDrawingArea::on_realize()
  {
    Gtk::DrawingArea::on_realize();
    _window = get_window();
    _gc = Gdk::GC::create(_window);
    _window->set_background(_white);
    _window->clear();
  }

  bool
  TWindowHoughDrawingArea::on_expose_event(GdkEventExpose*)
  {
    Glib::RefPtr<Gdk::Window> window = get_window();
    window->get_geometry(_winx, _winy, _winw, _winh, _wind);
    window->clear();
    draw();
    return true;
  }

bool
TWindowHoughDrawingArea::on_button_press_event(GdkEventButton *) {
//     _x = xR(e->x);
//     _y = yR(- e->y);
    on_expose_event((GdkEventExpose*) NULL);
    return true;
}

  void
  TWindowHoughDrawingArea::resetPosition(void)
  {
    if (_winw < _winh)
      _scale = double(_winw) / 2;
    else
      _scale = double(_winh) / 2;

    _x = _y = 0;
    on_expose_event((GdkEventExpose*) NULL);
  }

  void
  TWindowHoughDrawingArea::draw(void)
  {
    if (! _hp) return;

    std::cout << "HP name=" << _hp->name() << std::endl;

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
    const float scaleX = 1 / (xMax - xMin) * _winw;
    const float scaleY = 1 / (yMax - yMin) * _winh;

    //...Search maximum...
    unsigned nMax = 0;
    for (unsigned i = 0; i < _hp->nX(); i++) {
      for (unsigned j = 0; j < _hp->nY(); j++) {
        const unsigned n = _hp->entry(i, j);
        if (n > nMax) nMax = n;
      }
    }

    if (nMax == 0)
      std::cout << "max entry=" << nMax << std::endl;

    //...Draw...
    for (unsigned i = 0; i < _hp->nX(); i++) {
      for (unsigned j = 0; j < _hp->nY(); j++) {
        const unsigned n = _hp->entry(i, j);
        if (n) {
          const float x = float(i) * _hp->xSize() * scaleX;
          const float y = float(j) * _hp->ySize() * scaleY;

          const int x0 = int(x);
          const int y0 = int(y);
          const int z0 = toY(y0);
          const int x1 = int(x + _hp->xSize() * scaleX) - x0;
          const int y1 = int(y + _hp->ySize() * scaleY) - y0;
          const float level = float(n) / float(nMax);

//    if (n == nMax)
//        std::cout << "y=" << (_hp->yMin() + float(j) * _hp->ySize())
//         << " ";

          if (level < 0.25)
            _gc->set_foreground(_gray0);
          else if (level < 0.5)
            _gc->set_foreground(_gray1);
          else if (level < 0.75)
            _gc->set_foreground(_gray2);
          else if (level > 0.90)
            _gc->set_foreground(_red);
          else
            _gc->set_foreground(_gray3);

          _window->draw_rectangle(_gc, true, x0, z0, x1, y1);

//    if (n == nMax) std::cout << "x0,y0,z0,x1,y1=" << x0 << ","
//              << y0 << "," << z0 << "," << x1 << ","
//              << y1 << ",level=" << level << std::endl;
        }
      }
    }

//  std::cout << "TWHDArea ... xMin,xMax,yMin,yMax=" << xMin << "," << xMax
//     << "," << yMin << "," << yMax << std::endl;
//  std::cout << "TWHDArea ... winx,winy,winw,winh,wind=" << _winx << "," << _winy
//     << "," << _winw << "," << _winh << "," << _wind << std::endl;

    //...Draw region...
    _gc->set_foreground(_green);
    const AList<CList<unsigned> > & regions = _hp->regions();
    for (unsigned i = 0; i < (unsigned) regions.length(); i++) {

//  std::cout << "TWH ... region " << i << std::endl;

      for (unsigned j = 0; j < (unsigned) regions[i]->length(); j++) {
        const unsigned id = * (* regions[i])[j];
        unsigned ix = 0;
        unsigned iy = 0;
        _hp->id(id, ix, iy);

        const float x = float(ix) * _hp->xSize() * scaleX;
        const float y = float(iy) * _hp->ySize() * scaleY;

        const int x0 = int(x);
        const int y0 = int(y);
        const int z0 = toY(y0);

        const int x1 = int(x + _hp->xSize() * scaleX);
        const int y1 = int(y + _hp->ySize() * scaleY) - y0;
        const int z1 = z0 + y1;

//      std::cout << "TWH ... id=" << id
//            << std::endl;

        for (unsigned k = 0; k < 8; k++) {
          if (k % 2) continue;
          unsigned idx = _hp->neighbor(id, k);

          if (idx == id) continue;
          bool found = false;
          for (unsigned l = 0;
               l < (unsigned) regions[i]->length();
               l++) {
            if (idx == * (* regions[i])[l]) {
//      std::cout << "        " << idx << " is neighbor " << k << std::endl;
              found = true;
              break;
            }
//        std::cout << "        " << idx << " is not neighbor " << k << std::endl;
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
    }

    //...Draw text...
//     _xPositionText = _hp->xMin() + _hp->xSize() * _hp->nX() * 0.1;
//     _yPositionText = _hp->yMin() + _hp->ySize() * _hp->nY() * 0.9 * scale;
//     _window.draw_text(_xPositionText, _yPositionText, _text.c_str());
//     std::string text = " y_scale=" + dtostring(1/scale);
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

// void
// TWindowHoughDrawingArea::draw(const THoughPlane & hp,
//            const AList<TPoint2D> & list,
//            float radius,
//            leda_color c) {

//     draw(hp);

//     //...Cal. scale for y coordinate...
//     const float xMin = _hp->xMin();
//     const float xMax = _hp->xMax();
//     const float yMin = _hp->yMin();
//     const float scale = (xMax - xMin) / (_hp->yMax() * 1.15 - _hp->yMin());

//     //...y=0 line...
//     if (yMin < 0) {
//  const float z = (0 - yMin) * scale + yMin;
//  _window.draw_segment(xMin, z, xMax, z, leda_gray2);
//     }

//     //...
//     if (list.length()) {
//  for (unsigned i = 0; i < (unsigned) list.length(); i++) {
//      _window.draw_circle(list[i]->x(),
//        (list[i]->y() - yMin) * scale + yMin,
//        radius,
//        c);

// //       std::cout << "TWH ... region " << i << ":x=" << list[i]->x()
// //           << ",y=" << list[i]->y() << std::endl;
//  }
//     }
// }

} // namespace Belle

#endif
