//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : DisplayDrawingAreaHough.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display CDC trigger objects.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef TRGCDC_DISPLAY

#ifndef TRGCDCDisplayDrawingAreaHough_FLAG_
#define TRGCDCDisplayDrawingAreaHough_FLAG_

#include "trg/cdc/DisplayDrawingArea.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCDDAreaHough TRGCDCDisplayDrawingAreaHough
#endif

namespace Belle2 {

class TRGCDCHoughPlane;

/// Actual class to display trigger objects
class TRGCDCDisplayDrawingAreaHough : public TRGCDCDisplayDrawingArea {

  public:
    /// Default constructor
    TRGCDCDisplayDrawingAreaHough(int size, double innerR, double outerR);
    
    /// Destructor
    virtual ~TRGCDCDisplayDrawingAreaHough();

    double scale(double);
    double scale(void) const;
    void resetPosition(void);
    bool axial(void) const;
    bool axial(bool);
    bool stereo(void) const;
    bool stereo(bool);
    bool wireName(void) const;
    bool wireName(bool);
    bool oldCDC(void) const;
    bool oldCDC(bool);

    void clear(void);

    virtual bool on_expose_event(GdkEventExpose *);
    virtual bool on_button_press_event(GdkEventButton *);

    void append(const TRGCDCHoughPlane * const);

  private:
    virtual void on_realize();
    void draw(void);

    /// Coordinate transformations.
    int toY(int y) const;

  private:
    double _scale;
    bool _axial;
    bool _stereo;
    bool _wireName;
    bool _oldCDC;
    double _x, _y;
    double _innerR;
    double _outerR;
    const TRGCDCHoughPlane * _hp;

    Glib::RefPtr<Gdk::Window> _window;
    int _winx, _winy, _winw, _winh, _wind;

    Glib::RefPtr<Gdk::GC> _gc;
    Gdk::Color _blue, _red, _green, _black, _white, _gray, _yellow;
    Gdk::Color _gray0, _gray1, _gray2, _gray3;
};

//-----------------------------------------------------------------------------

#ifdef TRGCDC_NO_INLINE
#define inline
#else
#undef inline
#define TRGCDCDisplayDrawingAreaHough_INLINE_DEFINE_HERE
#endif
#ifdef TRGCDCDisplayDrawingAreaHough_INLINE_DEFINE_HERE

inline
double
TRGCDCDisplayDrawingAreaHough::scale(double a) {
    return _scale = a;
}

inline
double
TRGCDCDisplayDrawingAreaHough::scale(void) const {
    return _scale;
}

inline
bool
TRGCDCDisplayDrawingAreaHough::axial(void) const {
    return _axial;
}

inline
bool
TRGCDCDisplayDrawingAreaHough::axial(bool a) {
    return _axial = a;
}

inline
bool
TRGCDCDisplayDrawingAreaHough::stereo(void) const {
    return _stereo;
}

inline
bool
TRGCDCDisplayDrawingAreaHough::stereo(bool a) {
    return _stereo = a;
}

inline
bool
TRGCDCDisplayDrawingAreaHough::wireName(void) const {
    return _wireName;
}

inline
bool
TRGCDCDisplayDrawingAreaHough::wireName(bool a) {
    return _wireName = a;
}

inline
int
TRGCDCDisplayDrawingAreaHough::toY(int a) const {
    return - a + _winh;
}

inline
void
TRGCDCDisplayDrawingAreaHough::clear(void) {
    _hp = 0;
}

inline
void
TRGCDCDisplayDrawingAreaHough::append(const TRGCDCHoughPlane * const hp) {
    _hp = hp;
    on_expose_event((GdkEventExpose *) NULL);
}

#endif
#undef inline

} // namespace Belle2

#endif // TRGCDCDisplayDrawingAreaHough_FLAG_
#endif
