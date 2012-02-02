//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGCDCTSStreamModule.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A trigger module for CDC
//-----------------------------------------------------------------------------
// 0.00 : 2012/02/02 : First version
//-----------------------------------------------------------------------------

#define TRGCDC_SHORT_NAMES

#include <stdlib.h>
#include <iostream>
#include "framework/core/ModuleManager.h"
#include "trg/modules/tsstream/TRGCDCTSStreamModule.h"
#include "trg/trg/Debug.h"
#include "trg/trg/BitStream.h"
#include "trg/cdc/Layer.h"
#include "trg/cdc/TrackSegment.h"

using namespace std;

namespace Belle2 {

REG_MODULE(TRGCDCTSStream);

string
TRGCDCTSStreamModule::version() const {
    return string("TRGCDCTSStreamModule 0.00");
}

TRGCDCTSStreamModule::TRGCDCTSStreamModule()
    : Module::Module(),
      _debugLevel(0),
      _mode(0),
      _streamFilename("unknown"),
      _cdc(0) {

    string desc = "TRGCDCTSStreamModule(" + version() + ")";
    setDescription(desc);

    addParam("DebugLevel",
	     _debugLevel,
	     "TRGCDCTSStream debug level",
	     _debugLevel);

    addParam("Mode", _mode, "TRGCDCTSStream mode", _mode);

    addParam("OutputStreamFile",
             _streamFilename,
             "The filename of bit stream",
             _streamFilename);

    if (TRGDebug::level())
        cout << "TRGCDCTSStreamModule ... created" << endl;
}

TRGCDCTSStreamModule::~TRGCDCTSStreamModule() {

    if (_cdc)
        TRGCDC::getTRGCDC("good-bye");

    if (TRGDebug::level())
        cout << "TRGCDCTSStreamModule ... destructed " << endl;
}

void
TRGCDCTSStreamModule::initialize() {

    if (TRGDebug::level()) {
        cout << "TRGCDCTSStreamModule::initialize ... options" << endl;
        cout << TRGDebug::tab(4) << "debug level = " << TRGDebug::level()
             << endl;
    }
}

void
TRGCDCTSStreamModule::beginRun() {

    _cdc = TRGCDC::getTRGCDC();

    //...Super layer loop...
    for (unsigned l = 0; l < _cdc->nTrackSegmentLayers(); l++) {
	const Belle2::TRGCDCLayer * lyr = _cdc->trackSegmentLayer(l);
	const unsigned nWires = lyr->nWires();

	//...Clear old pointers...
	_wires[l].clear();

	//...TS loop...
	for (unsigned i = 0; i < nWires; i++) {
	      const TCTSegment & s = (TCTSegment &) * (* lyr)[i];
	      _wires[l].push_back(s.wires()[5]);
	}
    }

    if (TRGDebug::level())
        cout << "TRGCDCTSStreamModule ... beginRun called. TRGCDC version="
	     << _cdc->version() << endl;
}

void
TRGCDCTSStreamModule::event() {

    //...To dump wire hits...
    _cdc->dump("trgWireCentralHits");

    //...Clock loop (from 0 to 99 cycles, about 800 ns)...
    for (unsigned c = 0; c < 100; c++) {

	//...Super layer loop...
	for (unsigned l = 0; l < 9; l++) {

	    //...Bit stream for this super layer...
	    TRGBitStream stream;

	    //...Wire loop...
	    for (unsigned i = 0; i < _wires[l].size(); i++) {
		const TRGSignal & s = _wires[l][i]->triggerOutput();
		bool hit = s.active(c);
		stream.append(hit);
	    }

	    cout << "Super layer " << l << ", clock " << c << endl;
	    stream.dump("test");
	}
    }
}

void
TRGCDCTSStreamModule::endRun() {
    if (TRGDebug::level())
        cout << "TRGCDCTSStreamModule ... endRun called " << endl;
}

void
TRGCDCTSStreamModule::terminate() {
    if (TRGDebug::level())
        cout << "TRGCDCTSStreamModule ... terminate called " << endl;
}

} // namespace Belle2
