//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : DisplayHough.cc
// Section  : Tracking CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display CDC trigger information.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef CDCTRIGGER_DISPLAY

#include "trg/cdc/DisplayHough.h"

namespace Belle2 {

// Gtk::Main * GtkMain = 0;
// bool TRGCDCDisplayHough::_skipEvent = false;
// bool TRGCDCDisplayHough::_endOfEvent = false;
// bool TRGCDCDisplayHough::_endOfEventFlag = false;

TRGCDCDisplayHough::TRGCDCDisplayHough(const std::string & name,
					     double innerR,
					     double outerR,
					     int size)
    :
// _adjustment(double(size) / outerR / 2,
//		  double(size) / outerR / 2,
//		  10.0,
//		  0.1),
    TRGCDCDisplay(name, size, int(outerR)),
    _w(size, innerR, outerR) {
    _w.set_size_request(size, size);
    initialize(_w, size);
}

TRGCDCDisplayHough::~TRGCDCDisplayHough() {
}

} // namespace Belle2

#endif
