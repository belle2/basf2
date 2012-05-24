//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TWindowSZDrawingArea.h
// Section  : Tracking CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display tracking objects in conformal finder.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef TRASAN_WINDOW_GTK

#ifndef TWINDOWSZDRAWINGAREA_FLAG_
#define TWINDOWSZDRAWINGAREA_FLAG_


#include <gtkmm.h>
#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/ConstAList.h"

namespace Belle2 {
  class TRGCDCWireHit;
}

namespace Belle {

  class TRGCDCWireHit;
  class TTrackBase;
  class TLine;
  class TLink;

/// Actual class to display tracking objects
  class TWindowSZDrawingArea : public Gtk::DrawingArea {

  public:
    /// Default constructor
    TWindowSZDrawingArea(int size, double outerR);

    /// Destructor
    virtual ~TWindowSZDrawingArea();

    double scale(double);
    double scale(void) const;
    void resetPosition(void);
    bool axial(void) const;
    bool axial(bool);
    bool stereo(void) const;
    bool stereo(bool);
    bool wireName(void) const;
    bool wireName(bool);

    void clear(void);
    void append(const AList<TLink> &,
                Gdk::Color color = Gdk::Color("grey"));
    void append(const AList<TLine> &,
                Gdk::Color color = Gdk::Color("grey"));
    void append(const TLine&,
                Gdk::Color color = Gdk::Color("grey"));

    virtual bool on_expose_event(GdkEventExpose*);
    virtual bool on_button_press_event(GdkEventButton*);

  private:
    virtual void on_realize();
    void drawCDC(void);
    void draw(void);
    void drawLine(const TLine& line, Gdk::Color& c);
    void drawBase(const TTrackBase& base, Gdk::Color& c);

    /// Coordinate transformations.
    int x(double x) const;
    int y(double y) const;
    int xR(double x) const;
    int yR(double y) const;

  private:
    double _scale;
    bool _axial;
    bool _stereo;
    bool _wireName;
    double _x, _y;
    double m_outerR;

    Gtk::Main* GtkMain;
    CAList<TTrackBase> _objects;
    AList<Gdk::Color> _colors;
    AList<TTrackBase> _selfObjects;
    AList<TLink> _selfTLinks;

    Glib::RefPtr<Gdk::Window> _window;
    int _winx, _winy, _winw, _winh, _wind;

    Glib::RefPtr<Gdk::GC> _gc;
    Gdk::Color _blue, _red, _green, _black, _white, _grey, _yellow;
  };

//-----------------------------------------------------------------------------

#ifdef TRASAN_NO_INLINE
#define inline
#else
#undef inline
#define TWINDOWGTK_INLINE_DEFINE_HERE
#endif
#ifdef TWINDOWGTK_INLINE_DEFINE_HERE

  inline
  double
  TWindowSZDrawingArea::scale(double a)
  {
    return _scale = a;
  }

  inline
  double
  TWindowSZDrawingArea::scale(void) const
  {
    return _scale;
  }

  inline
  bool
  TWindowSZDrawingArea::axial(void) const
  {
    return _axial;
  }

  inline
  bool
  TWindowSZDrawingArea::axial(bool a)
  {
    return _axial = a;
  }

  inline
  bool
  TWindowSZDrawingArea::stereo(void) const
  {
    return _stereo;
  }

  inline
  bool
  TWindowSZDrawingArea::stereo(bool a)
  {
    return _stereo = a;
  }

  inline
  bool
  TWindowSZDrawingArea::wireName(void) const
  {
    return _wireName;
  }

  inline
  bool
  TWindowSZDrawingArea::wireName(bool a)
  {
    return _wireName = a;
  }

  inline
  int
  TWindowSZDrawingArea::x(double a) const
  {
    /*     std::cout << "_x,_scale,_winw/2,a=" << _x << "," << _scale << "," */
    /*        << _winw/2 << "," << a << std::endl; */
    return int((a - _x) * _scale + _winw / 2);
  }

  inline
  int
  TWindowSZDrawingArea::y(double a) const
  {
    return int((- a - _y) * _scale + _winh / 2);
  }

  inline
  int
  TWindowSZDrawingArea::xR(double a) const
  {
    return int((a - _winw / 2) / _scale + _x);
  }

  inline
  int
  TWindowSZDrawingArea::yR(double a) const
  {
    return int((- a - _winh / 2) / _scale + _y);
  }

  inline
  void
  TWindowSZDrawingArea::clear(void)
  {
    _objects.removeAll();
    HepAListDeleteAll(_colors);
    HepAListDeleteAll(_selfObjects);
    HepAListDeleteAll(_selfTLinks);
  }

#endif
#undef inline

} // namespace Belle

#endif // TWINDOWSZDRAWINGAREA_FLAG_
#endif
