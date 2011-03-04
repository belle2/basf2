//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Fitter3D.cc
// Section  : TRG CDC
// Owner    :
// Email    :
//-----------------------------------------------------------------------------
// Description : A class to fit tracks in 3D
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include <iostream>
#include "trg/trg/Debug.h"
#include "trg/cdc/Fitter3D.h"
#include "trg/cdc/TrackSegment.h"
#include "trg/cdc/Track.h"

using namespace std;

namespace Belle2 {

string
TRGCDCFitter3D::version(void) const {
    return string("TRGCDCFitter3D 5.04");
}

TRGCDCFitter3D::TRGCDCFitter3D(const string & name,
                               const TRGCDC & TRGCDC)
    : _name(name),
      _cdc(TRGCDC) {

    //...Initialization...
}

TRGCDCFitter3D::~TRGCDCFitter3D() {
}

int
TRGCDCFitter3D::doit(vector<TCTrack *> & trackListIn,
                     vector<TCTrack *> & trackListOut) {

    TRGDebug::enterStage("Fitter 3D");

    //...Loop over track list...
    const unsigned nInput = trackListIn.size();
    for (unsigned i = 0; i < nInput; i++) {

        //...Access to a track...
        const TCTrack & t = * trackListIn[i];

        //...Super layer loop...
        for (unsigned i = 0; i < _cdc.nSuperLayers(); i++) {

            //...Access to track segment list in this super layer...
            const vector<const TCTSegment *> & segments = t.trackSegments(i);
            const unsigned nSegments = segments.size();

            //...Presently nSegments should be 1...
            if (nSegments != 1) {
                cout << name() << " !!! multiple TS assigned" << endl;
            }

            //...Access to a track segment...
            const TCTSegment & s = * segments[0];
            s.name();
        }

        //...Do fitting job here (or call a fitting function)...
    }

    //...Termination...
    TRGDebug::leaveStage("Fitter 3D");
    return 0;
}

} // namespace Belle2
