//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TWindowHoughDrawingArea.h
// Section  : Tracking CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display tracking objects in hough finder.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef TRASAN_WINDOW_GTK

#ifndef TWINDOWHOUGHDRAWINGAREA_FLAG_
#define TWINDOWHOUGHDRAWINGAREA_FLAG_


#include <gtkmm.h>
#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/ConstAList.h"

namespace Belle {

  class TPoint2D;
  class THoughPlane;
// class TRGCDCWireHit;
// class TLink;
// class TTrackBase;
// class TSegment;
// class TTrack;

/// Actual class to display tracking objects
  class TWindowHoughDrawingArea : public Gtk::DrawingArea {

  public:
    /// Default constructor
    TWindowHoughDrawingArea(int size);

    /// Destructor
    virtual ~TWindowHoughDrawingArea();

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
    void append(const THoughPlane* const);

    virtual bool on_expose_event(GdkEventExpose*);
    virtual bool on_button_press_event(GdkEventButton*);

  private:
    virtual void on_realize();
    void draw(void);
//     void draw(const THoughPlane &,
//        const AList<TPoint2D> & list,
//        float radius,
//        Gdk::Color c);

    /// Coordinate transformations.
    int toY(int y) const;

  private:
    double _scale;
    bool _axial;
    bool _stereo;
    bool _wireName;
    double _x, _y;
    const THoughPlane* _hp;

    Glib::RefPtr<Gdk::Window> _window;
    int _winx, _winy, _winw, _winh, _wind;

    Glib::RefPtr<Gdk::GC> _gc;
    Gdk::Color _blue, _red, _green, _black, _white, _gray, _yellow;
    Gdk::Color _gray0, _gray1, _gray2, _gray3;
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
  TWindowHoughDrawingArea::scale(double a)
  {
    return _scale = a;
  }

  inline
  double
  TWindowHoughDrawingArea::scale(void) const
  {
    return _scale;
  }

  inline
  bool
  TWindowHoughDrawingArea::axial(void) const
  {
    return _axial;
  }

  inline
  bool
  TWindowHoughDrawingArea::axial(bool a)
  {
    return _axial = a;
  }

  inline
  bool
  TWindowHoughDrawingArea::stereo(void) const
  {
    return _stereo;
  }

  inline
  bool
  TWindowHoughDrawingArea::stereo(bool a)
  {
    return _stereo = a;
  }

  inline
  bool
  TWindowHoughDrawingArea::wireName(void) const
  {
    return _wireName;
  }

  inline
  bool
  TWindowHoughDrawingArea::wireName(bool a)
  {
    return _wireName = a;
  }

  inline
  int
  TWindowHoughDrawingArea::toY(int a) const
  {
    return - a + _winh;
  }

  inline
  void
  TWindowHoughDrawingArea::clear(void)
  {
    _hp = 0;
  }

  inline
  void
  TWindowHoughDrawingArea::append(const THoughPlane* const hp)
  {
    _hp = hp;
    on_expose_event((GdkEventExpose*) NULL);
  }


#endif
#undef inline

} // namespace Belle

#endif // TWINDOWHOUGHDRAWINGAREA_FLAG_
#endif
