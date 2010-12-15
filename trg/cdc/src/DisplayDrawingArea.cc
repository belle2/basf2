//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : DisplayDrawingArea.cc
// Section  : Tracking CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display CDC trigger objects.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef CDCTRIGGER_DISPLAY

#define TRGCDC_SHORT_NAMES

#include <iostream>
#include <pangomm/init.h>
// #include "trg/cdc/Wire.h"
// #include "trg/cdc/WireHit.h"
// #include "trg/cdc/TrackSegment.h"
// #include "trg/cdc/FrontEnd.h"
// #include "trg/cdc/Merger.h"
#include "trg/cdc/DisplayDrawingAreaRphi.h"

using namespace std;

namespace Belle2 {

TRGCDCDisplayDrawingArea::TRGCDCDisplayDrawingArea(int size,
							   double innerR,
							   double outerR)
    : _scale(double(size) / outerR / 2),
      _axial(true),
      _stereo(false),
      _wireName(false),
      _oldCDC(false) {
}

TRGCDCDisplayDrawingArea::~TRGCDCDisplayDrawingArea() {
}

} // namespace Belle2

#endif
