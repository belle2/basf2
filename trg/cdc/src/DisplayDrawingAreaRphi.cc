//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : DisplayDrawingAreaRphi.cc
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

#include <cstdlib>
#include <iostream>
#include <pangomm/init.h>
#include "trg/trg/Constants.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/Link.h"
#include "trg/cdc/TrackSegment.h"
#include "trg/cdc/Circle.h"
#include "trg/cdc/Track.h"
#include "trg/cdc/FrontEnd.h"
#include "trg/cdc/Merger.h"
#include "trg/cdc/DisplayDrawingAreaRphi.h"

using namespace std;

namespace Belle2 {

TRGCDCDisplayDrawingAreaRphi::TRGCDCDisplayDrawingAreaRphi(TRGCDCDisplay & w,
                                                           int size,
                                                           double innerR,
                                                           double outerR)
    : TRGCDCDisplayDrawingArea(w, size, outerR),
      _scale(double(size) / outerR / 2),
      _axial(true),
      _stereo(false),
      _wireName(false),
      _oldCDC(false),
      _x(0),
      _y(0),
      _innerR(innerR),
      _outerR(outerR) {
    _blue = Gdk::Color("blue");
    _red = Gdk::Color("red");
    _green = Gdk::Color("green");
    _black = Gdk::Color("black");
    _white = Gdk::Color("white");
    _grey = Gdk::Color("grey");
    _yellow = Gdk::Color("yellow");
    _grey0 = Gdk::Color("gray90");

    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
    colormap->alloc_color(_blue);
    colormap->alloc_color(_red);
    colormap->alloc_color(_green);
    colormap->alloc_color(_black);
    colormap->alloc_color(_white);
    colormap->alloc_color(_grey);
    colormap->alloc_color(_yellow);

    Pango::init();
    Glib::ustring wn = "unknown";
    _pl = create_pango_layout(wn);

    add_events(Gdk::EXPOSURE_MASK | Gdk::BUTTON_PRESS_MASK);
}

TRGCDCDisplayDrawingAreaRphi::~TRGCDCDisplayDrawingAreaRphi() {
}

void
TRGCDCDisplayDrawingAreaRphi::on_realize() {
    Gtk::DrawingArea::on_realize();
    _window = get_window();
    _gc = Gdk::GC::create(_window);
    _window->set_background(_white);
    _window->clear();
}

bool
TRGCDCDisplayDrawingAreaRphi::on_expose_event(GdkEventExpose *) {
    Glib::RefPtr<Gdk::Window> window = get_window();
    window->get_geometry(_winx, _winy, _winw, _winh, _wind);
    window->clear();
    drawCDC();
    draw();
    return true;
}

bool
TRGCDCDisplayDrawingAreaRphi::on_button_press_event(GdkEventButton * e) {
    _x = xR(e->x);
    _y = yR(- e->y);
    on_expose_event((GdkEventExpose *) NULL);
    return true;
}

void
TRGCDCDisplayDrawingAreaRphi::drawCDC(void) {

    //...Axis...
    _gc->set_foreground(_grey);
    _gc->set_line_attributes(1,
                             Gdk::LINE_SOLID,
                             Gdk::CAP_NOT_LAST,
                             Gdk::JOIN_MITER);
    _window->draw_line(_gc,
                       x(- _outerR),
                       y(0),
                       x(_outerR),
                       y(0));
    _window->draw_line(_gc,
                       x(0),
                       y(- _outerR),
                       x(0),
                       y(_outerR));

    //...CDC...
    _gc->set_foreground(_grey);
    _gc->set_line_attributes(1,
                             Gdk::LINE_SOLID,
                             Gdk::CAP_NOT_LAST,
                             Gdk::JOIN_MITER);
    _window->draw_arc(_gc,
                      0,
                      x(- _innerR),
                      y(_innerR),
                       int(2 * _innerR * _scale),
                       int(2 * _innerR * _scale),
                      0,
                      360 * 64);
    _window->draw_arc(_gc,
                      0,
                      x(- _outerR),
                      y(_outerR),
                       int(2 * _outerR * _scale),
                       int(2 * _outerR * _scale),
                      0,
                      360 * 64);

    //...Belle CDC...
    if (_oldCDC) {
        const double innerR = 118;
        const double outerR = 880;
        _gc->set_foreground(_grey);
        _gc->set_line_attributes(1,
                                 Gdk::LINE_ON_OFF_DASH,
                                 Gdk::CAP_NOT_LAST,
                                 Gdk::JOIN_MITER);
        _window->draw_arc(_gc,
                          0,
                          x(- innerR),
                          y(innerR),
                          int(2 * innerR * _scale),
                          int(2 * innerR * _scale),
                          0,
                          360 * 64);
        _window->draw_arc(_gc,
                          0,
                          x(- outerR),
                          y(outerR),
                          int(2 * outerR * _scale),
                          int(2 * outerR * _scale),
                          0,
                          360 * 64);
    }
}

void
TRGCDCDisplayDrawingAreaRphi::draw(void) {
    drawHits();
    for (unsigned i = 0; i < _segments.size(); i++)
        drawTrackSegment(* _segments[i],
                         1,
                         _segmentsColor[i],
                         Gdk::LINE_SOLID);
    for (unsigned i = 0; i < _circles.size(); i++)
        drawCircle(* _circles[i], 1, _circlesColor[i], Gdk::LINE_SOLID);
    for (unsigned i = 0; i < _tracks.size(); i++)
        drawTrack(* _tracks[i], 1, _tracksColor[i], Gdk::LINE_SOLID);
    for (unsigned i = 0; i < _fronts.size(); i++)
        drawFrontEnd(* _fronts[i],
                     1,
                     _frontColors[i],
                     Gdk::LINE_SOLID);

    for (unsigned i = 0; i < _mergers.size(); i++)
        drawMerger(* _mergers[i],
                   1,
                   _mergerColors[i],
                   Gdk::LINE_SOLID);
    drawWires();
}

void
TRGCDCDisplayDrawingAreaRphi::drawWires(void) {
    for (unsigned i = 0; i < _wires.size(); i++)
	drawWire(* _wires[i], 1, _wiresColor[i], Gdk::LINE_SOLID);
}

void
TRGCDCDisplayDrawingAreaRphi::drawHits(void) {
    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
    const unsigned n = _hits.size();
    for (unsigned i = 0; i < n; i++) {
         if (! _stereo)
             if (_hits[i]->wire().stereo())
                 continue;
         if (! _axial)
             if (_hits[i]->wire().axial())
                 continue;

        //...Points...
        const TCWire & w = _hits[i]->wire();
        const HepGeom::Point3D<double> & p = w.backwardPosition();
        double radius = _hits[i]->drift();

        colormap->alloc_color(_hitsColor[i]);
        _gc->set_foreground(_hitsColor[i]);
        _gc->set_line_attributes(1,
                                 Gdk::LINE_SOLID,
                                 Gdk::CAP_NOT_LAST,
                                 Gdk::JOIN_MITER);
        _window->draw_arc(_gc,
                          0,
                          x((p.x() - radius) * 10),
                          y((p.y() + radius) * 10),
                          int(2 * radius * 10 * _scale),
                          int(2 * radius * 10 * _scale),
                          0,
                          360 * 64);
         if (_wireName) {
            Glib::ustring wn = _hits[i]->wire().name().c_str();
            _pl->set_text(wn);
            _window->draw_layout(_gc, x(p.x() * 10.), y(p.y() * 10.), _pl);
        }
        colormap->free_color(_hitsColor[i]);
    }
}

void
TRGCDCDisplayDrawingAreaRphi::drawWire(const TCWire & w,
                                       int lineWidth,
                                       Gdk::Color & c,
                                       Gdk::LineStyle s) {

    if (! _stereo)
        if (w.stereo())
            return;
    if (! _axial)
        if (w.axial())
            return;

    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
    colormap->alloc_color(c);
    _gc->set_foreground(c);
    _gc->set_line_attributes(lineWidth,
                             s,
                             Gdk::CAP_NOT_LAST,
                             Gdk::JOIN_MITER);

    //...Cell shape...
    Gdk::Point p0;
    std::vector<Gdk::Point> points;
    const unsigned nDivisions = 5;
    const float ri = w.layer().innerRadius();
    const float ro = w.layer().outerRadius();
    const float cPhi = w.backwardPosition().phi();
    const float dPhi = M_PI / w.layer().nWires();
    for (unsigned j = 0; j < nDivisions + 1; j++) {  // inner
        const float phi = cPhi - dPhi
            + (2 * dPhi / float(nDivisions)) * float(j);
        const float xx = ri * cos(phi) * 10;
        const float yy = ri * sin(phi) * 10;
        if (j == 0) {
            p0.set_x(x(xx));
            p0.set_y(y(yy));
        }
        points.push_back(Gdk::Point(x(xx), y(yy)));
    }
    for (unsigned j = 0; j < nDivisions + 1; j++) {  // outer
        const float phi = cPhi + dPhi
            - (2 * dPhi / float(nDivisions)) * float(j);
        const float xx = ro * cos(phi) * 10;
        const float yy = ro * sin(phi) * 10;
        points.push_back(Gdk::Point(x(xx), y(yy)));
    }
    points.push_back(p0);

    _gc->set_foreground(c);
    _window->draw_lines(_gc, points);
}

void
TRGCDCDisplayDrawingAreaRphi::drawTrackSegment(const TCTSegment & w,
                                               int lineWidth,
                                               Gdk::Color & c,
                                               Gdk::LineStyle s) {
    const std::vector<const TCWire *> wires = w.wires();
    const unsigned n = wires.size();
    for (unsigned i = 0; i < n; i++) {
        drawWire(* wires[i], lineWidth, c, s);
    }
}

void
TRGCDCDisplayDrawingAreaRphi::drawCircle(const TCCircle & t,
                                         int lineWidth,
                                         Gdk::Color & c,
                                         Gdk::LineStyle s) {

    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
    colormap->alloc_color(c);
    _gc->set_foreground(c);
    _gc->set_line_attributes(1,
                             Gdk::LINE_SOLID,
                             Gdk::CAP_NOT_LAST,
                             Gdk::JOIN_MITER);

    //...Draw a circle...
    const TRGPoint2D & h = t.center();
    double radius = fabs(t.radius());
    _window->draw_arc(_gc,
                      0,
                      x((h.x() - radius) * 10),
                      y((h.y() + radius) * 10),
                      int(2 * radius * 10 * _scale),
                      int(2 * radius * 10 * _scale),
                      0,
                      360 * 64);
    colormap->free_color(c);

    //...Draw text...
    //_window.draw_text(_xPositionText, _yPositionText, _text.c_str());
}

void
TRGCDCDisplayDrawingAreaRphi::drawTrack(const TCTrack & t,
                                        int lineWidth,
                                        Gdk::Color & c,
                                        Gdk::LineStyle s) {

    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
    colormap->alloc_color(c);
    _gc->set_foreground(c);
    _gc->set_line_attributes(1,
                             Gdk::LINE_SOLID,
                             Gdk::CAP_NOT_LAST,
                             Gdk::JOIN_MITER);

    const TRGCDC & cdc = * TRGCDC::getTRGCDC();

    //...Draw segments first...
    for (unsigned i = 0; i < cdc.nSuperLayers(); i++) {
        const vector<TCLink *> & links = t.links(i);
        for (unsigned j = 0; j < links.size(); j++) {
	    const TCLink & l = * links[j];
//             drawTrackSegment(* (TCTSegment *) l.wire(),
// 			     lineWidth,
// 			     c,
// 			     s);
	    const TCWire * w = l.wire();
	    const TCTSegment * t = dynamic_cast<const TCTSegment *>(w);

	    cout << "w,t=" << w << "," << t << endl;

	    if (t)
		drawTrackSegment(* t,
				 lineWidth,
				 c,
				 s);
	    _window->draw_line(_gc,
			       x(l.positionOnTrack().x() * 10),
			       y(l.positionOnTrack().y() * 10),
			       x(l.positionOnWire().x() * 10),
			       y(l.positionOnWire().y() * 10));
        }
    }

    //...Draw a track...
    TCHelix hIp = t.helix();
    hIp.pivot(ORIGIN);
    const HepGeom::Point3D<double> & h = hIp.center();
    const double radius = fabs(hIp.radius());
    const HepGeom::Point3D<double> pIn = t.links(0)[0]->positionOnTrack();
    const HepGeom::Point3D<double> pOut = t.links(8)[0]->positionOnTrack();

//     const HepGeom::Point3D<double> pIn = TLink::innerMost(t.cores())->positionOnTrack() - h;
//     const HepGeom::Point3D<double> pOut = TLink::outerMost(t.cores())->positionOnTrack() - h;

    double a0 = atan2(pIn.y(), pIn.x()) / M_PI * 180;
    double a1 = atan2(pOut.y(), pOut.x()) / M_PI * 180;
     std::cout << "h=" << h << ",r=" << radius
 				      << ",a0=" << a0 << ",a1=" << a1
 				      << std::endl;
    double d = a1 - a0;
    if (d > 180) d -= 360;
    else if (d < -180) d += 360;
    _window->draw_arc(_gc,
		      0,
		      x((h.x() - radius) * 10),
		      y((h.y() + radius) * 10),
		      int(2 * radius * 10 * _scale),
		      int(2 * radius * 10 * _scale),
//		      int(a0 * 64),
		      0,
		      360 * 64);
//		      int(d * 64));

//     //...Draw a circle...
//     const TRGPoint2D & h = t.helix().center();
//     double radius = fabs(t.helix().radius());
//     _window->draw_arc(_gc,
//                       0,
//                       x((h.x() - radius) * 10),
//                       y((h.y() + radius) * 10),
//                       int(2 * radius * 10 * _scale),
//                       int(2 * radius * 10 * _scale),
//                       0,
//                       360 * 64);

    colormap->free_color(c);
}

void
TRGCDCDisplayDrawingAreaRphi::drawFrontEnd(const TCFrontEnd & w,
                                           int lineWidth,
                                           Gdk::Color & c,
                                           Gdk::LineStyle s) {
    const unsigned n = w.size();
    for (unsigned i = 0; i < n; i++) {
        drawWire(* w[i], lineWidth, c, s);
    }
}

void
TRGCDCDisplayDrawingAreaRphi::drawMerger(const TCMerger & w,
                                         int lineWidth,
                                         Gdk::Color & c,
                                         Gdk::LineStyle s) {
    const unsigned n = w.size();
    for (unsigned i = 0; i < n; i++) {
        drawFrontEnd(* w[i], lineWidth, c, s);
    }
}

void
TRGCDCDisplayDrawingAreaRphi::resetPosition(void) {
    if (_winw < _winh)
        _scale = double(_winw) / _outerR / 2;
    else
        _scale = double(_winh) / _outerR / 2;

    _x = _y = 0;
    on_expose_event((GdkEventExpose *) NULL);
}

void
TRGCDCDisplayDrawingAreaRphi::append(const std::vector<const TCWire *> & l,
                                     Gdk::Color c) {
    const unsigned n = l.size();
    for (unsigned i = 0; i < n; i++) {
        _wires.push_back(l[i]);
        _wiresColor.push_back(c);
    }
    on_expose_event((GdkEventExpose *) NULL);
}

void
TRGCDCDisplayDrawingAreaRphi::append(const std::vector<const TCWHit *> & l,
                                     Gdk::Color c) {
    const unsigned n = l.size();
    for (unsigned i = 0; i < n; i++) {
        _hits.push_back(l[i]);
        _hitsColor.push_back(c);
    }
    on_expose_event((GdkEventExpose *) NULL);
}

void
TRGCDCDisplayDrawingAreaRphi::append(const TCTSegment & s,
                                     Gdk::Color c) {
    _segments.push_back(& s);
    _segmentsColor.push_back(c);
    on_expose_event((GdkEventExpose *) NULL);
}

void
TRGCDCDisplayDrawingAreaRphi::append(const std::vector<const TCTSegment *> & l,
                                     Gdk::Color c) {
    const unsigned n = l.size();
    for (unsigned i = 0; i < n; i++) {
        _segments.push_back(l[i]);
        _segmentsColor.push_back(c);
    }
    on_expose_event((GdkEventExpose *) NULL);
}

void
TRGCDCDisplayDrawingAreaRphi::append(const TCCircle & s,
                                     Gdk::Color c) {
    _circles.push_back(& s);
    _circlesColor.push_back(c);
    on_expose_event((GdkEventExpose *) NULL);
}

void
TRGCDCDisplayDrawingAreaRphi::append(const std::vector<const TCCircle *> & l,
                                     Gdk::Color c) {
    const unsigned n = l.size();
    for (unsigned i = 0; i < n; i++) {
        _circles.push_back(l[i]);
        _circlesColor.push_back(c);
    }
    on_expose_event((GdkEventExpose *) NULL);
}

void
TRGCDCDisplayDrawingAreaRphi::append(const TCTrack & s,
                                     Gdk::Color c) {
    _tracks.push_back(& s);
    _tracksColor.push_back(c);
    on_expose_event((GdkEventExpose *) NULL);
}

void
TRGCDCDisplayDrawingAreaRphi::append(const std::vector<const TCTrack *> & l,
                                     Gdk::Color c) {
    const unsigned n = l.size();
    for (unsigned i = 0; i < n; i++) {
        _tracks.push_back(l[i]);
        _tracksColor.push_back(c);
    }
    on_expose_event((GdkEventExpose *) NULL);
}

void
TRGCDCDisplayDrawingAreaRphi::append(const TCFrontEnd & s,
                                     Gdk::Color c) {
    _fronts.push_back(& s);
    _frontColors.push_back(c);
    on_expose_event((GdkEventExpose *) NULL);
}

void
TRGCDCDisplayDrawingAreaRphi::append(const TCMerger & s,
                                     Gdk::Color c) {
    _mergers.push_back(& s);
    _mergerColors.push_back(c);
    on_expose_event((GdkEventExpose *) NULL);
}

void
TRGCDCDisplayDrawingAreaRphi::clear(void) {
    _wires.clear();
    _wiresColor.clear();
    _hits.clear();
    _hitsColor.clear();
    _segments.clear();
    _segmentsColor.clear();
    _circles.clear();
    _circlesColor.clear();
    _tracks.clear();
    _tracksColor.clear();
    _fronts.clear();
    _frontColors.clear();
    _mergers.clear();
    _mergerColors.clear();
}

void
TRGCDCDisplayDrawingAreaRphi::oneShot(const vector<const TCTSegment *> & l,
                                      Gdk::Color color) {
    for (unsigned i = 0; i < l.size(); i++)
        drawTrackSegment(* l[i], 1, color, Gdk::LINE_SOLID);
}

} // namespace Belle2

#endif
