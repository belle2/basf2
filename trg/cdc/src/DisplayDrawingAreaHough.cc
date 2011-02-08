//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : DisplayDrawingAreaHough.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display CDC trigger objects.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef TRGCDC_DISPLAY

#define TRGCDC_SHORT_NAMES
#define TRGCDCDisplayDrawingAreaHough_INLINE_DEFINE_HERE

#include <iostream>
#include <pangomm/init.h>
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/TrackSegment.h"
#include "trg/cdc/FrontEnd.h"
#include "trg/cdc/Merger.h"
#include "trg/cdc/HoughPlane.h"
#include "trg/cdc/DisplayDrawingAreaHough.h"

using namespace std;

namespace Belle2 {

TRGCDCDisplayDrawingAreaHough::TRGCDCDisplayDrawingAreaHough(int size)
    : TRGCDCDisplayDrawingArea(size, 10),
      _scale(1),
      _wireName(false),
      _oldCDC(false),
      _x(0),
      _y(0),
      _hp(0) {

    _blue = Gdk::Color("blue");
    _red = Gdk::Color("red");
    _green = Gdk::Color("green");
    _black = Gdk::Color("black");
    _white = Gdk::Color("white");
    _gray = Gdk::Color("gray");
    _yellow = Gdk::Color("yellow");

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

TRGCDCDisplayDrawingAreaHough::~TRGCDCDisplayDrawingAreaHough() {
}

void
TRGCDCDisplayDrawingAreaHough::on_realize() {
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
TRGCDCDisplayDrawingAreaHough::on_expose_event(GdkEventExpose *) {
    Glib::RefPtr<Gdk::Window> window = get_window();
    window->get_geometry(_winx, _winy, _winw, _winh, _wind);
    window->clear();
    draw();
    return true;
}

bool
TRGCDCDisplayDrawingAreaHough::on_button_press_event(GdkEventButton * e) {
    cout << "x=" << e->x << ",y=" << e->y << endl;
    _x = xR(e->x);
    _y = yR(e->y);
    on_expose_event((GdkEventExpose *) NULL);
    return true;
}

void
TRGCDCDisplayDrawingAreaHough::draw(void) {
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
        std::cout << "max entry=" << nMax << std::endl;

    //...Draw...
    for (unsigned i = 0; i < _hp->nX(); i++) {
        for (unsigned j = 0; j < _hp->nY(); j++) {
            const unsigned n = _hp->entry(i, j);
            if (n) {
                const float x = float(i) * _hp->xSize() * _scaleX;
                const float y = float(j) * _hp->ySize() * _scaleY;

                const int x0 = int(x);
                const int y0 = int(y);
                const int z0 = toY(y0);
                const int x1 = int(x + _hp->xSize() * _scaleX) - x0;
                const int y1 = int(y + _hp->ySize() * _scaleY) - y0;
                const float level = float(n) / float(nMax);

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

//                 const int x2 = x0 - int(_x - _winw / 2);
//                 const int y2 = z0 - int(_y - _winh / 2);
//                 const int x2 = xT(i);
//                 const int y2 = yT(j);
//                 _gc->set_foreground(_green);
//                 _window->draw_rectangle(_gc, true, x2, y2, x1, y1);

//                  std::cout << "x0,y0,z0,x1,y1=" << x0 << ","
//                           << y0 << "," << z0 << "," << x1 << ","
//                           << y1 << ",level=" << level << std::endl;
//                 std::cout << "x2,y2,x3,y3=" << x2 << ","
//                           << y2 << "," << ",level=" << level << std::endl;
                
            }
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

//  std::cout << "TWHDArea ... xMin,xMax,yMin,yMax=" << xMin << "," << xMax
//            << "," << yMin << "," << yMax << std::endl;
//  std::cout << "TWHDArea ... winx,winy,winw,winh,wind=" << _winx << "," << _winy
//            << "," << _winw << "," << _winh << "," << _wind << std::endl;

    //...Draw region...
    _gc->set_foreground(_green);
    const std::vector<std::vector<unsigned> *> & regions = _hp->regions();
    for (unsigned i = 0; i < (unsigned) regions.size(); i++) {

//        std::cout << "TWH ... region " << i << std::endl;

        for (unsigned j = 0; j < (unsigned) regions[i]->size(); j++) {
//            const unsigned id = * (* regions[i])[j];
            const unsigned id = (* regions[i])[j];
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

//             std::cout << "TWH ... id=" << id
//                                                 << std::endl;

            for (unsigned k = 0; k < 8; k++) {
                if (k % 2) continue;
                unsigned idx = _hp->neighbor(id, k);

                if (idx == id) continue;
                bool found = false;
                for (unsigned l = 0;
                     l < (unsigned) regions[i]->size();
                     l++) {
//                    if (idx == * (* regions[i])[l]) {
                    if (idx == (* regions[i])[l]) {
//                        std::cout << "        " << idx << " is neighbor " << k << std::endl;
                        found = true;
                        break;
                    }
//                    std::cout << "        " << idx << " is not neighbor " << k << std::endl;
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

void
TRGCDCDisplayDrawingAreaHough::resetPosition(void) {
    _x = _winw / 2;
    _y = _winh / 2;
}

int
TRGCDCDisplayDrawingAreaHough::xT(double x) const {
    const double xx = x - _x + double(_hp->nX()) / 2;
    return int(xx * _hp->xSize() * _scaleX);
}

int
TRGCDCDisplayDrawingAreaHough::yT(double y) const {
    const double yy = y - _y + double(_hp->nY()) / 2;
    return - int(yy * _hp->ySize() * _scaleY) + _winh;
}

int
TRGCDCDisplayDrawingAreaHough::xR(double x) const {
    return int(x / _hp->xSize() * _scaleX);
}

int
TRGCDCDisplayDrawingAreaHough::yR(double y) const {
    return - int((y - double(_winh)) / _hp->ySize() / _scaleY);
}

} // namespace Belle2

#endif
