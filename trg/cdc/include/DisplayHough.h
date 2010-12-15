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

#ifdef CDCTRIGGER_DISPLAY

#ifndef TRGCDCDisplayHough_FLAG_
#define TRGCDCDisplayHough_FLAG_

#include <gtkmm.h>
#include "trg/cdc/Display.h"
#include "trg/cdc/DisplayDrawingAreaHough.h"

#ifdef TRGCDC_SHORT_NAMES
#define CTDisplayHough TRGCDCDisplayHough
#endif

namespace Belle2 {

class TRGCDCWireHit;

/// A class to display CDC trigger information.
class TRGCDCDisplayHough : public TRGCDCDisplay {
    
  public:
    /// Default constructor
    TRGCDCDisplayHough(const std::string & name = "TRGCDCDisplayHough",
			   double innerR = 160,
			   double outerR = 1137,
			   int size = 600);

    /// Destructor
    virtual ~TRGCDCDisplayHough();

  public: // Access to drawing area.

    /// returns drawing area.
    TRGCDCDisplayDrawingAreaHough & area(void);

  private: // Objects to display and control
    std::vector<TRGCDCWireHit *> _hits;

  private: // GTK stuff
    TRGCDCDisplayDrawingAreaHough _w;
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
TRGCDCDisplayDrawingAreaHough &
TRGCDCDisplayHough::area(void) {
    return _w;
}

#endif
#undef inline

} // namespace Belle2

#endif // TRGCDCDisplayHough_FLAG_
#endif
