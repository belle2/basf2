//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : DisplayRphi.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display CDC trigger information.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef TRGCDC_DISPLAY

#define TRGCDCDisplayRphi_INLINE_DEFINE_HERE

#include "trg/cdc/DisplayRphi.h"

namespace Belle2 {

// Gtk::Main * GtkMain = 0;
// bool TRGCDCDisplayRphi::_skipEvent = false;
// bool TRGCDCDisplayRphi::_endOfEvent = false;
// bool TRGCDCDisplayRphi::_endOfEventFlag = false;

TRGCDCDisplayRphi::TRGCDCDisplayRphi(const std::string & name,
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

TRGCDCDisplayRphi::~TRGCDCDisplayRphi() {
}

} // namespace Belle2

#endif
