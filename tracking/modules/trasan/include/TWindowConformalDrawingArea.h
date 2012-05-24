//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TWindowConformalDrawingArea.h
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

#ifndef TWINDOWCONFORMALDRAWINGAREA_FLAG_
#define TWINDOWCONFORMALDRAWINGAREA_FLAG_


#include <gtkmm.h>
#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/ConstAList.h"

namespace Belle2 {
  class TRGCDCWireHit;
}

namespace Belle {

  class TLink;
  class TTrackBase;
  class TSegment;
  class TTrack;
  class TCircle;

/// Actual class to display tracking objects
  class TWindowConformalDrawingArea : public Gtk::DrawingArea {

  public:
    /// Default constructor
    TWindowConformalDrawingArea(int size, double innerR, double outerR);

    /// Destructor
    virtual ~TWindowConformalDrawingArea();

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
    void append(const CAList<Belle2::TRGCDCWireHit> &,
                Gdk::Color color = Gdk::Color("grey"));
    void append(const AList<TLink> &,
                Gdk::Color color = Gdk::Color("grey"));
    void append(const AList<TSegment> &,
                Gdk::Color color = Gdk::Color("grey"));
    void append(const AList<TTrack> &,
                Gdk::Color color = Gdk::Color("grey"));
    void append(const AList<TCircle> &,
                Gdk::Color color = Gdk::Color("grey"));

    virtual bool on_expose_event(GdkEventExpose*);
    virtual bool on_button_press_event(GdkEventButton*);

  private:
    virtual void on_realize();
    void drawCDC(void);
    void draw(void);
    void drawBase(const TTrackBase&, Gdk::Color& c);
    void drawSegment(const TSegment&, Gdk::Color& c);
    void drawTrack(const TTrack&, Gdk::Color& c);
//     void drawLine(const TLine &, Gdk::Color & c);
    void drawCircle(const TCircle&, Gdk::Color& c);

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
    double _innerR;
    double _outerR;

    Gtk::Main* GtkMain;
    CAList<TTrackBase> _objects;
    AList<Gdk::Color> _colors;
    AList<TTrackBase> _selfObjects;
    AList<TLink> _selfTLinks;

    Glib::RefPtr<Gdk::Window> _window;
    int _winx, _winy, _winw, _winh, _wind;

    Glib::RefPtr<Gdk::GC> _gc;
    Gdk::Color _blue, _red, _green, _black, _white, _grey, _yellow, _grey0;

    Glib::RefPtr<Pango::Layout> _pl;
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
  TWindowConformalDrawingArea::scale(double a)
  {
    return _scale = a;
  }

  inline
  double
  TWindowConformalDrawingArea::scale(void) const
  {
    return _scale;
  }

  inline
  bool
  TWindowConformalDrawingArea::axial(void) const
  {
    return _axial;
  }

  inline
  bool
  TWindowConformalDrawingArea::axial(bool a)
  {
    return _axial = a;
  }

  inline
  bool
  TWindowConformalDrawingArea::stereo(void) const
  {
    return _stereo;
  }

  inline
  bool
  TWindowConformalDrawingArea::stereo(bool a)
  {
    return _stereo = a;
  }

  inline
  bool
  TWindowConformalDrawingArea::wireName(void) const
  {
    return _wireName;
  }

  inline
  bool
  TWindowConformalDrawingArea::wireName(bool a)
  {
    return _wireName = a;
  }

  inline
  int
  TWindowConformalDrawingArea::x(double a) const
  {
    /*     std::cout << "_x,_scale,_winw/2,a=" << _x << "," << _scale << "," */
    /*        << _winw/2 << "," << a << std::endl; */
    return int((a - _x) * _scale + _winw / 2);
  }

  inline
  int
  TWindowConformalDrawingArea::y(double a) const
  {
    return int((- a - _y) * _scale + _winh / 2);
  }

  inline
  int
  TWindowConformalDrawingArea::xR(double a) const
  {
    return int((a - _winw / 2) / _scale + _x);
  }

  inline
  int
  TWindowConformalDrawingArea::yR(double a) const
  {
    return int((- a - _winh / 2) / _scale + _y);
  }

  inline
  void
  TWindowConformalDrawingArea::clear(void)
  {
    _objects.removeAll();
    HepAListDeleteAll(_colors);
    HepAListDeleteAll(_selfObjects);
    HepAListDeleteAll(_selfTLinks);
  }

#endif
#undef inline

} // namespace Belle

#endif // TWINDOWCONFORMALDRAWINGAREA_FLAG_
#endif
