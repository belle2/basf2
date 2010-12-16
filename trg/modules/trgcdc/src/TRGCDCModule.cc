//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Module.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A trigger module for CDC
//-----------------------------------------------------------------------------
// 0.00 : 2010/10/08 : First version
//-----------------------------------------------------------------------------

#define TRGCDC_SHORT_NAMES

#include <iostream>
#include "framework/core/ModuleManager.h"
#include "trg/trg/Signal.h"
#include "trg/trg/Clock.h"
#include "trg/modules/trgcdc/TRGCDCModule.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/Layer.h"
#include "trg/cdc/TrackSegment.h"
#include "trg/cdc/HoughPlaneMulti2.h"
#include "trg/cdc/HoughTransformationCircle.h"

#ifdef TRGCDC_DISPLAY
#undef ERROR
#include "trg/cdc/DisplayRphi.h"
#include "trg/cdc/DisplayHough.h"
#include "trg/cdc/FrontEnd.h"
#include "trg/cdc/Merger.h"
#endif

namespace Belle2_TRGCDC {
#ifdef TRGCDC_DISPLAY
    Belle2::TCDisplayRphi * D = 0;
    Belle2::TCDisplayHough * H0 = 0;
    Belle2::TCDisplayHough * H1 = 0;
#endif
}

using namespace std;
using namespace Belle2_TRGCDC;

namespace Belle2 {

REG_MODULE(TRGCDCModule, "TRGCDC");

string
TRGCDCModule::version() const {
    return "0.01";
}

TRGCDCModule::TRGCDCModule()
    : Module::Module(),
      _debugLevel(0),
      _testParamInt(0),
      _configFilename("TRGCDCConfig.dat") {

    string desc = "TRGCDCModule(" + version() + ")";
    setDescription(desc);

    addParam("testParamInt", _testParamInt, 20);
    addParam("ConfigFile",
	     _configFilename,
	     string("TRGCDCConfig.dat"),
	     "The filename of CDC trigger config file");

#ifdef TRGCDC_DEBUG
    cout << "TRGCDCModule ... created" << endl;
#endif
}

TRGCDCModule::~TRGCDCModule() {
#ifdef TRGCDC_DISPLAY
    if (D)
	delete D;
    if (H0)
	delete H0;
    if (H1)
	delete H1;
    cout << "TRGCDCModule ... display deleted" << endl;
#endif
#ifdef TRGCDC_DEBUG
    cout << "TRGCDCModule ... destructed " << endl;
#endif
}

void
TRGCDCModule::initialize() {

#ifdef TRGCDC_DISPLAY
    int argc = 0;
    char ** argv = 0;
    Gtk::Main main_instance(argc, argv);
    if (! D)
	D = new TCDisplayRphi();
    D->clear();
    D->show();
    if (! H0)
	H0 = new TCDisplayHough("Plus");
    H0->clear();
    H0->show();
    if (! H1)
	H1 = new TCDisplayHough("Minus");
    H1->clear();
    H1->show();
    cout << "TRGCDCModule ... GTK initialized" << endl;
#endif
}

void
TRGCDCModule::beginRun() {
#ifdef TRGCDC_DEBUG
    cout << "TRGCDCModule ... beginRun called " << endl;
#endif
}

void
TRGCDCModule::event() {

    //...CDC trigger...
    TRGCDC & cdc = * TRGCDC::getTRGCDC(_configFilename);

    //...Make Hough planes...
    static bool first = true;
    static TCHPlaneMulti2 * plane[2] = {0, 0};
    static TCHTransformationCircle circleH("CircleHough");
    if (first) {
	plane[0] = new TCHPlaneMulti2("circle hough plus",
				      350,
				      0,
				      2 * M_PI,
				      100,
				      0.7,
				      3,
				      5);
	plane[1] = new TCHPlaneMulti2("circle hough minus",
				      350,
				      0,
				      2 * M_PI,
				      100,
				      0.7,
				      3,
				      5);

	//...Create patterns...
	unsigned axialSuperLayerId = 0;
	for (unsigned i = 0; i < cdc.nTrackSegmentLayers(); i++) {
	    const Belle2::TRGCDCLayer * l = cdc.trackSegmentLayer(i);
	    const unsigned nWires = l->nWires();
	    
	    if (! nWires) continue;
	    if ((* l)[0]->stereo()) continue;
	    
	    plane[0]->preparePatterns(axialSuperLayerId, nWires);
	    plane[1]->preparePatterns(axialSuperLayerId, nWires);
	    for (unsigned j = 0; j < nWires; j++) {
		const Belle2::TRGCDCWire & w = * (* l)[j];
		const float x = w.xyPosition().x();
		const float y = w.xyPosition().y();
		
		plane[0]->clear();
		plane[0]->vote(x, y, +1, circleH, axialSuperLayerId, 1);
		plane[0]->registerPattern(axialSuperLayerId, j);
		
		plane[1]->clear();
		plane[1]->vote(x, y, -1, circleH, axialSuperLayerId, 1);
		plane[1]->registerPattern(axialSuperLayerId, j);

#ifdef TRGCDC_DISPLAY
		string stg = "Hough Pattern Regstration";
		string inf = "   ";

		plane[0]->dump();
 		plane[0]->merge();
		plane[1]->dump();
 		plane[1]->merge();
		H0->stage(stg);
		H0->information(inf);
		H0->clear();
		H0->area().append(plane[0]);
		H0->show();
		H1->stage(stg);
		H1->information(inf);
		H1->clear();
		H1->area().append(plane[1]);
		H1->show();
		H1->run();
#endif
	    }
	    ++axialSuperLayerId;
	}
	first = false;
    }

#ifdef TRGCDC_DEBUG
//  cdc.dump("geometry superLayers layers wires detail");
    cdc.dump("geometry superLayers layers detail");
#endif

    //...CDC clock...
//  const TRGClock & cdcClock = cdc.systemClock();

    //...CDC trigger simulation...
    cdc.update();
    cdc.simulate();

    //...Hough studies : voting...
    plane[0]->clear();
    plane[1]->clear();
    unsigned axialSuperLayerId = 0;
    for (unsigned i = 0; i < cdc.nTrackSegmentLayers(); i++) {
	const Belle2::TRGCDCLayer * l = cdc.trackSegmentLayer(i);
	const unsigned nWires = l->nWires();
	if (! nWires) continue;
	if ((* l)[0]->stereo()) continue;

	for (unsigned j = 0; j < nWires; j++) {
	    const TCTSegment & s = (TCTSegment &) * (* l)[j];

	    //...Select hit TS only...
	    if (s.triggerOutput().active()) {
		plane[0]->vote(axialSuperLayerId, j);
		plane[1]->vote(axialSuperLayerId, j);
	    }
	}
	++axialSuperLayerId;
    }
    plane[0]->merge();
    plane[1]->merge();

#ifdef TRGCDC_DEBUG
    cout << "TRGCDCModule ... event called " << endl;
    cout << "    TS hits" << endl;
    for (unsigned i = 0; i < cdc.nTrackSegments(); i++) {
	const TCTSegment & s = * cdc.trackSegment(i);
	if (s.triggerOutput().active())
	    s.dump("detail", "        ");
    }
#endif
#ifdef TRGCDC_DISPLAY
    plane[0]->dump();
    plane[1]->dump();

    string stg = "2D : Peak Finding";
    string inf = "   ";
//  cdc.dump("hits");
    D->stage(stg);
    D->information(inf);
    D->clear();
    D->beginEvent();
    D->area().append(cdc.hits());
    D->area().append(cdc.tsHits(), Gdk::Color("#6600FF009900"));
    H0->stage(stg);
    H0->information(inf);
    H0->clear();
    H0->area().append(plane[0]);
    H0->show();
    H1->stage(stg);
    H1->information(inf);
    H1->clear();
    H1->area().append(plane[1]);
    H1->show();
    H1->run();
#endif
#ifdef TRGCDC_DISPLAY
//     unsigned iFront = 0;
//     while (const TCFrontEnd * f = cdc.frontEnd(iFront++)) {
// 	D->clear();
// 	D->beginEvent();
// 	D->area().append(* f);
// 	D->run();
//     }
//     unsigned iMerger = 0;
//     while (const TCMerger * f = cdc.merger(iMerger++)) {
// 	D->clear();
// 	D->beginEvent();
// 	D->area().append(* f);
// 	D->run();
//     }
#endif
}

void
TRGCDCModule::endRun() {
#ifdef TRGCDC_DEBUG
    cout << "TRGCDCModule ... endRun called " << endl;
#endif
}

void
TRGCDCModule::terminate() {
#ifdef TRGCDC_DEBUG
    cout << "TRGCDCModule ... terminate called " << endl;
#endif
}

} // namespace Belle2
