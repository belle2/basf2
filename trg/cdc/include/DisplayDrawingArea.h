//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : DisplayDrawingArea.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display CDC trigger objects.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef TRGCDC_DISPLAY

#ifndef TRGCDCDisplayDrawingArea_FLAG_
#define TRGCDCDisplayDrawingArea_FLAG_

#include <vector>
#include <gtkmm.h>

#ifdef TRGCDC_SHORT_NAMES
#define TCDDArea TRGCDCDisplayDrawingArea
#endif

namespace Belle2 {

/// Actual class to display trigger objects
class TRGCDCDisplayDrawingArea : public Gtk::DrawingArea {

  public:
    /// Default constructor
    TRGCDCDisplayDrawingArea(int size, double outerR);
    
    /// Destructor
    virtual ~TRGCDCDisplayDrawingArea();

    double scale(double);
    double scale(void) const;
    virtual void resetPosition(void) = 0;
    bool axial(void) const;
    bool axial(bool);
    bool stereo(void) const;
    bool stereo(bool);
    bool wireName(void) const;
    bool wireName(bool);
    bool oldCDC(void) const;
    bool oldCDC(bool);

    virtual void clear(void) = 0;

    virtual bool on_expose_event(GdkEventExpose *) = 0;
    virtual bool on_button_press_event(GdkEventButton *) = 0;

  private:
    virtual void on_realize() = 0;

  private:
    double _scale;
    bool _axial;
    bool _stereo;
    bool _wireName;
    bool _oldCDC;
};

//-----------------------------------------------------------------------------

inline
double
TRGCDCDisplayDrawingArea::scale(double a) {
    return _scale = a;
}

inline
double
TRGCDCDisplayDrawingArea::scale(void) const {
    return _scale;
}

inline
bool
TRGCDCDisplayDrawingArea::axial(void) const {
    return _axial;
}

inline
bool
TRGCDCDisplayDrawingArea::axial(bool a) {
    return _axial = a;
}

inline
bool
TRGCDCDisplayDrawingArea::stereo(void) const {
    return _stereo;
}

inline
bool
TRGCDCDisplayDrawingArea::stereo(bool a) {
    return _stereo = a;
}

inline
bool
TRGCDCDisplayDrawingArea::wireName(void) const {
    return _wireName;
}

inline
bool
TRGCDCDisplayDrawingArea::wireName(bool a) {
    return _wireName = a;
}

inline
bool
TRGCDCDisplayDrawingArea::oldCDC(void) const {
    return _oldCDC;
}

inline
bool
TRGCDCDisplayDrawingArea::oldCDC(bool a) {
    return _oldCDC = a;
}

} // namespace Belle2

#endif // TRGCDCDisplayDrawingArea_FLAG_
#endif
