//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Circle.cc
// Section  : Circleing
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a circle
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRGCDC_SHORT_NAMES

#include "trg/trg/Point2D.h"
#include "trg/cdc/Circle.h"
#include "trg/cdc/HoughPlane.h"

using namespace std;

namespace Belle2 {

TRGCDCCircle::TRGCDCCircle(float r,
                           float phi,
                           float charge,
                           const TCHPlane & plane)
    : TCTBase("unknown", charge),
      _center(r * cos(phi), r * sin(phi)),
      _radius(r),
      _plane(& plane) {
}

TRGCDCCircle::~TRGCDCCircle() {
}

void
TRGCDCCircle::dump(const string & cmd, const string & pre) const {
    cout << pre;
    cout << "c=" << _center << ":r=" << _radius << ":hp=" << _plane->name()
         << endl;
    if (cmd.find("detail") != string::npos)
        TRGCDCTrackBase::dump(cmd, pre);
}

} // namespace Belle2
