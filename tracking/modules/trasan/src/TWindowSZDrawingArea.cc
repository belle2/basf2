//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TWindowSZDrawingArea.cc
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
#include "tracking/modules/trasan/TWindowSZDrawingArea.h"
#include "tracking/modules/trasan/Trasan.h"
#include "trg/cdc/WireHit.h"
#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/TTrackBase.h"
#include "tracking/modules/trasan/TLine.h"


namespace Belle {

  TWindowSZDrawingArea::TWindowSZDrawingArea(int size, double outerR)
    : _scale(double(size) / outerR / 2),
      _axial(true),
      _stereo(false),
      _wireName(false),
      _x(0),
      _y(0), m_outerR(outerR)
  {
    _blue = Gdk::Color("blue");
    _red = Gdk::Color("red");
    _green = Gdk::Color("green");
    _black = Gdk::Color("black");
    _white = Gdk::Color("white");
    _grey = Gdk::Color("grey");
    _yellow = Gdk::Color("yellow");

    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
    colormap->alloc_color(_blue);
    colormap->alloc_color(_red);
    colormap->alloc_color(_green);
    colormap->alloc_color(_black);
    colormap->alloc_color(_white);
    colormap->alloc_color(_grey);
    colormap->alloc_color(_yellow);

// std::cout << "_scale=" << _scale << std::endl;

    add_events(Gdk::EXPOSURE_MASK | Gdk::BUTTON_PRESS_MASK);
  }

  TWindowSZDrawingArea::~TWindowSZDrawingArea()
  {
  }

  void
  TWindowSZDrawingArea::on_realize()
  {
    Gtk::DrawingArea::on_realize();
    _window = get_window();
    _gc = Gdk::GC::create(_window);
    _window->set_background(_white);
    _window->clear();
  }

  bool
  TWindowSZDrawingArea::on_expose_event(GdkEventExpose*)
  {
    Glib::RefPtr<Gdk::Window> window = get_window();
    window->get_geometry(_winx, _winy, _winw, _winh, _wind);
    window->clear();
    drawCDC();
    draw();
    return true;
  }

  bool
  TWindowSZDrawingArea::on_button_press_event(GdkEventButton* e)
  {
    _x = xR(e->x);
    _y = yR(- e->y);
    on_expose_event((GdkEventExpose*) NULL);
    return true;
  }

  void
  TWindowSZDrawingArea::drawCDC(void)
  {

    //...Axis...
    _gc->set_foreground(_grey);
    _gc->set_line_attributes(1,
                             Gdk::LINE_SOLID,
                             Gdk::CAP_NOT_LAST,
                             Gdk::JOIN_MITER);
    _window->draw_line(_gc,
                       x(- m_outerR),
                       y(0),
                       x(m_outerR),
                       y(0));
    _window->draw_line(_gc,
                       x(0),
                       y(- m_outerR),
                       x(0),
                       y(m_outerR));
  }

  void
  TWindowSZDrawingArea::draw(void)
  {
    unsigned n = _objects.length();
    for (unsigned i = 0; i < n; i++) {
      const TTrackBase& track = * _objects[i];
      if (track.objectType() == Line)
        drawLine((const TLine&) track, * _colors[i]);
      else if (track.objectType() == TrackBase)
        drawBase(track, * _colors[i]);
      else
        std::cout << "TWindowSZDrawingArea::draw !!! can't display"
                  << std::endl;
    }
  }

  void
  TWindowSZDrawingArea::drawBase(const TTrackBase& base, Gdk::Color& c)
  {
    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
    colormap->alloc_color(c);
    _gc->set_foreground(c);
    _gc->set_line_attributes(1,
                             Gdk::LINE_SOLID,
                             Gdk::CAP_NOT_LAST,
                             Gdk::JOIN_MITER);

    //...TLinks...
    const AList<TLink> & links = base.links();
    const unsigned n = links.length();

    for (unsigned i = 0; i < n; i++) {

      //...Points...
      const float radius = 1;
      const HepGeom::Point3D<double> & sz = links[i]->position();
//  _window->draw_point(_gc, x(sz.x()), y(sz.y()));
      _window->draw_arc(_gc,
                        0,
                        x(sz.x() - radius),
                        y(sz.y() + radius),
                        int(2 * radius * _scale),
                        int(2 * radius * _scale),
                        0,
                        360 * 64);

//    if (_wireName) {
//      Glib::ustring wn = links[i]->wire()->name().c_str();
//      Glib::RefPtr<Pango::Layout> pl = create_pango_layout(wn);
//      _window->draw_layout(_gc, x(p.x() * 10.), y(p.y() * 10.), pl);
//  }
    }

    colormap->free_color(c);
  }


  void
  TWindowSZDrawingArea::drawLine(const TLine& base, Gdk::Color& c)
  {
    Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
    colormap->alloc_color(c);
    _gc->set_foreground(c);
    _gc->set_line_attributes(1,
                             Gdk::LINE_SOLID,
                             Gdk::CAP_NOT_LAST,
                             Gdk::JOIN_MITER);

    //...Lines...
    const float xmin = -800;
    const float ymin = xmin * base.a() + base.b();
    const float xmax = 800;
    const float ymax = xmax * base.a() + base.b();
    _window->draw_line(_gc,
                       x(xmin),
                       y(ymin),
                       x(xmax),
                       y(ymax));

    //...TLinks...
    const AList<TLink> & links = base.links();
    const unsigned n = links.length();
    for (unsigned i = 0; i < n; i++) {
      if (links[i]->wire() == NULL) continue;
      if (! _stereo)
        if (links[i]->wire()->stereo())
          continue;
      if (! _axial)
        if (links[i]->wire()->axial())
          continue;

      //...Points...
      Point3D x = links[i]->positionOnTrack();
      _window->draw_point(_gc, x(x.x()), y(x.y()));

//    if (_wireName) {
//      Glib::ustring wn = links[i]->wire()->name().c_str();
//      Glib::RefPtr<Pango::Layout> pl = create_pango_layout(wn);
//      _window->draw_layout(_gc, x(p.x() * 10.), y(p.y() * 10.), pl);
//  }
    }

    colormap->free_color(c);
  }

  void
  TWindowSZDrawingArea::resetPosition(void)
  {
    if (_winw < _winh)
      _scale = double(_winw) / m_outerR / 2;
    else
      _scale = double(_winh) / m_outerR / 2;

    _x = _y = 0;
    on_expose_event((GdkEventExpose*) NULL);
  }

  void
  TWindowSZDrawingArea::append(const AList<TLine> & list, Gdk::Color c)
  {
    for (unsigned i = 0; i < list.length(); i++) {
      TLine* s = new TLine(* list[i]);
      _selfObjects.append(s);
      _objects.append(s);
      _colors.append(new Gdk::Color(c));
    }
    on_expose_event((GdkEventExpose*) NULL);
  }

  void
  TWindowSZDrawingArea::append(const TLine& line, Gdk::Color c)
  {
    TLine* s = new TLine(line);
    _selfObjects.append(s);
    _objects.append(s);
    _colors.append(new Gdk::Color(c));
    on_expose_event((GdkEventExpose*) NULL);
  }

  void
  TWindowSZDrawingArea::append(const AList<TLink> & list, Gdk::Color c)
  {
    TTrackBase* t = new TTrackBase(list);
    _selfObjects.append(t);
    _objects.append(t);
    _colors.append(new Gdk::Color(c));
    on_expose_event((GdkEventExpose*) NULL);
  }

} // namespace Belle

#endif
