//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : CellHit.cc
// Section  : CDC tracking trasan
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a cell hit in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include "framework/datastore/StoreArray.h"
#include "generators/dataobjects/MCParticle.h"
#include "cdc/dataobjects/CDCSimHit.h"
#include "cdc/dataobjects/CDCHit.h"

#include "tracking/modules/trasan/TDebug.h"
#include "tracking/modules/trasan/TWire.h"
#include "tracking/modules/trasan/TCellHit.h"
#include "tracking/modules/trasan/TTrackMC.h"
#include "tracking/modules/trasan/TWireHitMC.h"

using namespace std;

namespace Belle {

TCellHit::TCellHit(const TCell & w,
                   unsigned indexCDCHit,
                   unsigned indexCDCSimHit,
                   unsigned indexMCParticle,
                   float driftLeft,
                   float driftLeftError,
                   float driftRight,
                   float driftRightError,
                   int mcLRflag,
                   float fudgeFactor)
    :  _state(0),
       _cell(w),
       _xyPosition(w.xyPosition()),
       _track(0),
       _iCDCHit(indexCDCHit),
       _iCDCSimHit(indexCDCSimHit),
       _iMCParticle(indexMCParticle) {
    _mcLR=mcLRflag;
    _drift[0] = driftLeft;
    _drift[1] = driftRight;
    _driftError[0] = driftLeftError * fudgeFactor;
    _driftError[1] = driftRightError * fudgeFactor;
    if (w.axial()) _state |= CellHitAxial;
    else           _state |= CellHitStereo;
}

TCellHit::~TCellHit() {
}

// [FIXME] Crashes when there is no MC information
void
TCellHit::dump(const std::string & msg,
               const std::string & pre) const {
    std::cout << pre;
    std::cout << _cell.name();
    if (msg.find("state") != std::string::npos ||
        msg.find("detail") != std::string::npos) {
        std::cout << ",cout";
        TDebugUtilities::bitDisplay(_state);
        if (track()) std::cout << ",trk ";
    }
    if (msg.find("position") != std::string::npos ||
        msg.find("detail") != std::string::npos) {
	cout << ",xy=" << _xyPosition;
    }
    if (msg.find("drift") != std::string::npos ||
        msg.find("detail") != std::string::npos) {
        if (_state & CellHitLeftMask) std::cout << ", L";
        if (_state & CellHitRightMask) std::cout << ", R";
        std::cout << ",dl " << _drift[0] << "+-" << _driftError[0];
        std::cout << ",dr " << _drift[1] << "+-" << _driftError[1];
    }
    if (msg.find("mc") != std::string::npos ||
        msg.find("detail") != std::string::npos) {
        cout << ",mcpart=" << iMCParticle();
	cout << ",flightTime=" << simHit()->getFlightTime();
	cout << ",driftLength=" << simHit()->getDriftLength() << endl;
//	_cell.signal().dump("detail", pre + "    ");
    }
    std::cout << std::endl;
}

Point3D
TCellHit::position(unsigned lr) const {
    static const HepGeom::Vector3D<double> HepXHat(1.0, 0.0, 0.0);
    static const HepGeom::Vector3D<double> HepYHat(0.0, 1.0, 0.0);
    static const HepGeom::Vector3D<double> HepZHat(0.0, 0.0, 1.0);

    //...Left...
    if (lr == CellHitLeft) {
        return _xyPosition
            - _drift[CellHitLeft] * HepZHat.cross(_xyPosition.unit());
    }

    //...Right case...
    else {
        return _xyPosition
            + _drift[CellHitRight] * HepZHat.cross(_xyPosition.unit());
    }
}

int
TCellHit::sortById(const TCellHit ** a, const TCellHit ** b) {
    if ((* a)->_cell.id() > (* b)->_cell.id())
        return 1;
    else if ((* a)->_cell.id() == (* b)->_cell.id())
        return 0;
    else
        return -1;
}

const Belle2::CDCHit *
TCellHit::hit(void) const {
    Belle2::StoreArray<Belle2::CDCHit> CDCHits("CDCHits");
    return CDCHits[_iCDCHit];
}

const Belle2::CDCSimHit *
TCellHit::simHit(void) const {
    Belle2::StoreArray<Belle2::CDCSimHit> CDCHits("CDCSimHits");
    return CDCHits[_iCDCSimHit];
}

const Belle2::MCParticle *
TCellHit::mcParticle(void) const {
    Belle2::StoreArray<Belle2::MCParticle> mcParticles("MCParticles");
    return mcParticles[_iMCParticle];
}

} // namespace Belle2
