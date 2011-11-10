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

#include "trg/trg/Constants.h"
#include "trg/trg/Point2D.h"
#include "trg/cdc/Link.h"
#include "trg/cdc/Circle.h"
#include "trg/cdc/CircleFitter.h"
#include "trg/cdc/HoughPlane.h"

using namespace std;

namespace Belle2 {

const TRGCDCCircleFitter TRGCDCCircle::_fitter =
    TRGCDCCircleFitter("TRGCDCCircle Default Fitter");

TRGCDCCircle::TRGCDCCircle(const std::vector<TCLink *> links)
    : TCTBase("unknown", 0),
      _center(ORIGIN),
      _radius(0),
      _plane(0) {
    fitter(& _fitter);
    append(links);
}

TRGCDCCircle::TRGCDCCircle(float r,
                           float phi,
                           float charge,
                           const TCHPlane & plane)
    : TCTBase("unknown", charge),
      _center(r * cos(phi), r * sin(phi)),
      _radius(r),
      _plane(& plane) {
    fitter(& _fitter);
}

TRGCDCCircle::~TRGCDCCircle() {
}

void
TRGCDCCircle::dump(const string & cmd, const string & pre) const {
    cout << pre;
    cout << "c=" << _center;
    cout << ":r=" << _radius;
    if (_plane) cout << ":hp=" << _plane->name();
    cout << endl;
    if (cmd.find("detail") != string::npos)
        TRGCDCTrackBase::dump(cmd, pre);
}

} // namespace Belle2
