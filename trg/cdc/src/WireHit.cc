//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : WireHit.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire hit in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include "framework/datastore/StoreArray.h"
#include "cdc/hitcdc/CDCSimHit.h"
#include "cdc/dataobjects/CDCHit.h"
#include "trg/trg/Utilities.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/WireHitMC.h"
#include "trg/cdc/TrackMC.h"

using namespace std;

namespace Belle2 {

TRGCDCWireHit::TRGCDCWireHit(const TRGCDCWire & w,
			     unsigned indexCDCHit,
			     unsigned indexCDCSimHit,
			     float driftLeft,
			     float driftLeftError,
			     float driftRight,
			     float driftRightError,
			     float fudgeFactor)
    :  _state(0),
       _wire(w),
       _xyPosition(w.xyPosition()),
       _track(0),
       _mc(0),
       _iCDCHit(indexCDCHit),
       _iCDCSimHit(indexCDCSimHit) {
//  w.hit(this); // set by TRGCDC
    _drift[0] = driftLeft;
    _drift[1] = driftRight;
    _driftError[0] = driftLeftError * fudgeFactor;
    _driftError[1] = driftRightError * fudgeFactor;
    if (w.axial()) _state |= WireHitAxial;
    else           _state |= WireHitStereo;
}

TRGCDCWireHit::~TRGCDCWireHit() {
}

void
TRGCDCWireHit::dump(const std::string & msg,
		    const std::string & pre) const {
    std::cout << pre;
    std::cout << _wire.name();
    if (msg.find("state") != std::string::npos ||
        msg.find("detail") != std::string::npos) {
        std::cout << ",state";
        TRGUtil::bitDisplay(_state);
        if (track()) std::cout << ",trk ";
    }
    if (msg.find("position") != std::string::npos ||
        msg.find("detail") != std::string::npos) {
	cout << ",xy=" << _xyPosition;
    }
    if (msg.find("drift") != std::string::npos ||
        msg.find("detail") != std::string::npos) {
        if (_state & WireHitLeftMask) std::cout << ", L";
        if (_state & WireHitRightMask) std::cout << ", R";
        std::cout << ",dl " << _drift[0] << "+-" << _driftError[0];
        std::cout << ",dr " << _drift[1] << "+-" << _driftError[1];
    }
    if (msg.find("mc") != std::string::npos ||
        msg.find("detail") != std::string::npos) {
        std::cout << ",hep ";
//         if (mc()) {
//             if (mc()->hep()) std::cout << mc()->hep()->id();
//             else std::cout << "0";
//         }
//         else {
//             std::cout << "0";
//         }
    }
    std::cout << std::endl;
}

Point3D
TRGCDCWireHit::position(unsigned lr) const {
    static const HepGeom::Vector3D<double> HepXHat(1.0, 0.0, 0.0);
    static const HepGeom::Vector3D<double> HepYHat(0.0, 1.0, 0.0);
    static const HepGeom::Vector3D<double> HepZHat(0.0, 0.0, 1.0);

    //...Left...
    if (lr == WireHitLeft) {
        return _xyPosition
            - _drift[WireHitLeft] * HepZHat.cross(_xyPosition.unit());
    }

    //...Right case...
    else {
        return _xyPosition
            + _drift[WireHitRight] * HepZHat.cross(_xyPosition.unit());
    }
}

int
TRGCDCWireHit::sortByWireId(const TRGCDCWireHit ** a,
                                const TRGCDCWireHit ** b) {
    if ((* a)->wire().id() > (* b)->wire().id())
        return 1;
    else if ((* a)->wire().id() == (* b)->wire().id())
        return 0;
    else
        return -1;
}

const CDCHit * const
TRGCDCWireHit::hit(void) const {
    StoreArray<CDCHit> CDCHits("CDCHits");
    CDCHits[_iCDCHit];
}

const CDCSimHit * const
TRGCDCWireHit::simHit(void) const {
    StoreArray<CDCSimHit> CDCHits("CDCSimHits");
    CDCHits[_iCDCSimHit];
}

} // namespace Belle2
