//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : DisplayRphi.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display CDC trigger information.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef TRGCDC_DISPLAY

#ifndef TRGCDCDisplayRphi_FLAG_
#define TRGCDCDisplayRphi_FLAG_

#include <gtkmm.h>
#include "trg/cdc/Display.h"
#include "trg/cdc/DisplayDrawingAreaRphi.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCDisplayRphi TRGCDCDisplayRphi
#endif

namespace Belle2 {

class TRGCDCWireHit;

/// A class to display CDC trigger information.
class TRGCDCDisplayRphi : public TRGCDCDisplay {
    
  public:
    /// Default constructor
    TRGCDCDisplayRphi(const std::string & name = "TRGCDCDisplayRphi",
			  double innerR = 160,
			  double outerR = 1137,
			  int size = 600);

    /// Destructor
    virtual ~TRGCDCDisplayRphi();

  public: // Access to drawing area.

    /// returns drawing area.
    TRGCDCDisplayDrawingAreaRphi & area(void);

  private: // Objects to display and control
    std::vector<TRGCDCWireHit *> _hits;

  private: // GTK stuff
    TRGCDCDisplayDrawingAreaRphi _w;
};

//-----------------------------------------------------------------------------

#ifdef TRGCDC_NO_INLINE
#define inline
#else
#undef inline
#define TRGCDCDisplayRphi_INLINE_DEFINE_HERE
#endif
#ifdef TRGCDCDisplayRphi_INLINE_DEFINE_HERE

inline
TRGCDCDisplayDrawingAreaRphi &
TRGCDCDisplayRphi::area(void) {
    return _w;
}

#endif
#undef inline

} // namespace Belle2

#endif // TRGCDCDisplayRphi_FLAG_
#endif
