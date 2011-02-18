//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : DisplayHough.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display CDC trigger information.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef TRGCDC_DISPLAY

#ifndef TRGCDCDisplayHough_FLAG_
#define TRGCDCDisplayHough_FLAG_

#include <gtkmm.h>
#include "trg/cdc/Display.h"
#include "trg/cdc/DisplayDrawingAreaHough.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCDisplayHough TRGCDCDisplayHough
#endif

namespace Belle2 {

/// A class to display CDC trigger information.
class TRGCDCDisplayHough : public TRGCDCDisplay {
    
  public:
    /// Default constructor
    TRGCDCDisplayHough(const std::string & name = "TRGCDCDisplayHough",
                       int size = 600);

    /// Destructor
    virtual ~TRGCDCDisplayHough();

  public: // display control

    /// returns scaler value.
    double scale(void) const;

    /// sets and returns scaler value.
    double scale(double);

  private: // Actions
    virtual void on_scale_value_changed(void);
    virtual void on_positionReset(void);

  public: // Access to drawing area.

    /// returns drawing area.
    TRGCDCDisplayDrawingAreaHough & area(void);

  private: // GTK stuff
    TRGCDCDisplayDrawingAreaHough _w;
    Gtk::Adjustment _adjustment;
    Gtk::HScale _scaler;
};

//-----------------------------------------------------------------------------

inline
TRGCDCDisplayDrawingAreaHough &
TRGCDCDisplayHough::area(void) {
    return _w;
}

inline
double
TRGCDCDisplayHough::scale(void) const {
    return _scaler.get_value();
}

inline
double
TRGCDCDisplayHough::scale(double a) {
    _scaler.set_value(a);
    return _scaler.get_value();
}

} // namespace Belle2

#endif // TRGCDCDisplayHough_FLAG_
#endif
