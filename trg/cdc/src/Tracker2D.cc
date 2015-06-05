//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Tracker2D.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a CDC Tracker2D board
//-----------------------------------------------------------------------------
// $Log$
// 2013,1005 first working verion 0.00
//      1014 added the unpacker part
//-----------------------------------------------------------------------------

#define TRGCDC_SHORT_NAMES

#include <algorithm>
#include <limits>
#include <iostream>
#include <iomanip>
#include "trg/trg/Utilities.h"
#include "trg/trg/Debug.h"
#include "trg/trg/State.h"
#include "trg/trg/Channel.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Segment.h"
#include "trg/cdc/Tracker2D.h"
#include "trg/cdc/TrackSegmentFinder.h"

using namespace std;

namespace Belle2 {

unsigned
TRGCDCTracker2D::nTSFs(void) {
    return (160 + 160 + 192 + 224 + 256 + 288 + 320 + 352 + 384) / 2 ;
}

TRGCDCTracker2D::TRGCDCTracker2D(const std::string & name,
                                 const TRGClock & systemClock,
                                 const TRGClock & dataClock,
                                 const TRGClock & userClockInput,
                                 const TRGClock & userClockOutput)
    : TRGBoard(name, systemClock, dataClock, userClockInput, userClockOutput) {
}

TRGCDCTracker2D::~TRGCDCTracker2D() {
}

string
TRGCDCTracker2D::version(void) {
    return ("TRGCDCTracker2D version 0.00");
}

void
TRGCDCTracker2D::push_back(const TRGCDCTrackSegmentFinder * a) {
    std::vector<const TRGCDCTrackSegmentFinder *>::push_back(a);
}

void
TRGCDCTracker2D::dump(const string & message, const string & pre) const {
    TRGBoard::dump(message, pre);
}

void
TRGCDCTracker2D::simulate(void) {

    const string sn = "TRGCDC 2D simulate : " + name();
    TRGDebug::enterStage(sn);

    //...Delete old objects...
    if (nOutput())
        for (unsigned i = 0; i < nOutput(); i++)
            if (output(i)->signal())
                delete output(i)->signal();

    //...Create an input signal bundle from 5 TSF boards...
    TRGSignalBundle isb = TRGSignalBundle(name() +
                                          "-InputSignalBundle",
                                          clockData());
    for (unsigned i = 0; i < 5; i++) {
        TRGSignalBundle * b = input(i)->signal();
        if (! b)
            continue;
        for (unsigned j = 0; j < b->size(); j++) {
            isb.push_back((* b)[j]);
        }
    }

    if (TRGDebug::level())
        isb.dump("", TRGDebug::tab());

    //...Output signal bundle...
    const string no = name() + "OutputSignalBundle";
    TRGSignalBundle * osb = new TRGSignalBundle(no,
                                                clockData(),
                                                isb,
                                                256,
                                                nTSFs() * 16,
                                                TCTracker2D::packer);
    output(0)->signal(osb);

    TRGDebug::leaveStage(sn);
}

TRGState
TCTracker2D::packer(const TRGState & input,
                    TRGState & registers,
                    bool & logicStillActive) {

    //...Registers...
    //   Storing TSF hit history upto 16 clocks. #TSF is half of all TSF.
    //   reg[15 downto 0]      : TSF-0 history (SL0)
    //   reg[31 downto 16]     : TSF-1 history (SL0)
    //   ...
    //   reg[1279 downto 1264] : TSF-59 history (SL0)
    //   reg[1295 downto 1280] : TSF-? history (SL1)
    //   ...

    //...Shift registers (TSF hit history pipe)...
    for (unsigned i = 0; i < nTSFs(); i++) {
        TRGState s = registers.subset(i * 16, 16);
        s.shift(1);
        registers.set(i * 16, s);
    }

    //...Unpack input state...
    //   Get TSF hit information. The drift time information is ignored.
    unpacker(input, registers);

    //...Do core logic...

    //...Make output...
    logicStillActive = registers.active();

    return TRGState(256);
}

void
TCTracker2D:: unpacker(const TRGState & input,
                       TRGState & output) {

    const string sn = "TRGCDC 2D unpacker";
    TRGDebug::enterStage(sn);

    //...Constants...
//  const unsigned noHit = 0b11111111;

    //...Store hit TSF id...
    unsigned tsfId[5][22]; // Max. 22 TSF ID is sent
    for (unsigned i = 0; i < 5; i++) {
        for (unsigned j = 0; j < 22; j++) {
            tsfId[i][j] = input.subset(i * 141 + 22 * j + 13, 9);
            if (tsfId[i][j] != 0)
                output.set(tsfId[i][j] * 16, true);
        }
    }

    if (TRGDebug::level()) {
        input.dump("", TRGDebug::tab() + "input bits:");

        cout << TRGDebug::tab() << "TSF hit ID" << endl;
        for (unsigned i = 0; i < 5; i++) {
            cout << TRGDebug::tab() << "    ASL" << i << endl;
            cout << TRGDebug::tab() << "        ";
            for (unsigned j = 0; j < 22; j++) {
                cout << tsfId[i][j] << ",";
            }
            cout << endl;
        }
    }

    TRGDebug::leaveStage(sn);
}

} // namespace Belle2
