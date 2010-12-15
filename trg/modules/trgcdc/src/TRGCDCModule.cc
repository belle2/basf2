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
#include "trg/modules/cdc/TRGCDCModule.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/Layer.h"
#include "trg/cdc/TrackSegment.h"
#include "trg/cdc/HoughPlaneMulti2.h"
#include "trg/cdc/HoughTransformationCircle.h"

#ifdef CDCTRIGGER_DISPLAY
#undef ERROR
#include "trg/cdc/DisplayRphi.h"
#include "trg/cdc/DisplayHough.h"
#include "trg/cdc/FrontEnd.h"
#include "trg/cdc/Merger.h"
#endif

namespace Belle2_TRGCDC {
#ifdef CDCTRIGGER_DISPLAY
    Belle2::CTDisplayRphi * D = 0;
    Belle2::CTDisplayHough * H0 = 0;
    Belle2::CTDisplayHough * H1 = 0;
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

#ifdef CDCTRIGGER_DEBUG
    cout << "TRGCDCModule ... created" << endl;
#endif
}

TRGCDCModule::~TRGCDCModule() {
#ifdef CDCTRIGGER_DISPLAY
    if (D)
	delete D;
    if (H0)
	delete H0;
    if (H1)
	delete H1;
    cout << "TRGCDCModule ... display deleted" << endl;
#endif
#ifdef CDCTRIGGER_DEBUG
    cout << "TRGCDCModule ... destructed " << endl;
#endif
}

void
TRGCDCModule::initialize() {

#ifdef CDCTRIGGER_DISPLAY
    int argc = 0;
    char ** argv = 0;
    Gtk::Main main_instance(argc, argv);
    if (! D)
	D = new CTDisplayRphi();
    D->clear();
    D->show();
    if (! H0)
	H0 = new CTDisplayHough("Plus");
    H0->clear();
    H0->show();
    if (! H1)
	H1 = new CTDisplayHough("Minus");
    H1->clear();
    H1->show();
    cout << "TRGCDCModule ... GTK initialized" << endl;
#endif
}

void
TRGCDCModule::beginRun() {
#ifdef CDCTRIGGER_DEBUG
    cout << "TRGCDCModule ... beginRun called " << endl;
#endif
}

void
TRGCDCModule::event() {

    //...CDC trigger...
    TRGCDC & cdc = * TRGCDC::getTRGCDC(_configFilename);

    //...Make Hough planes...
    static bool first = true;
    static CTHPlaneMulti2 * plane[2] = {0, 0};
    static CTHTransformationCircle circleH("CircleHough");
    if (first) {
	plane[0] = new CTHPlaneMulti2("circle hough plus",
				      350,
				      0,
				      2 * M_PI,
				      100,
				      0.7,
				      3,
				      5);
	plane[1] = new CTHPlaneMulti2("circle hough minus",
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

#ifdef CDCTRIGGER_DISPLAY
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

#ifdef CDCTRIGGER_DEBUG
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
	    const CTTSegment & s = (CTTSegment &) * (* l)[j];

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

#ifdef CDCTRIGGER_DEBUG
    cout << "TRGCDCModule ... event called " << endl;
    cout << "    TS hits" << endl;
    for (unsigned i = 0; i < cdc.nTrackSegments(); i++) {
	const CTTSegment & s = * cdc.trackSegment(i);
	if (s.triggerOutput().active())
	    s.dump("detail", "        ");
    }
#endif
#ifdef CDCTRIGGER_DISPLAY
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
#ifdef CDCTRIGGER_DISPLAY
//     unsigned iFront = 0;
//     while (const CTFrontEnd * f = cdc.frontEnd(iFront++)) {
// 	D->clear();
// 	D->beginEvent();
// 	D->area().append(* f);
// 	D->run();
//     }
//     unsigned iMerger = 0;
//     while (const CTMerger * f = cdc.merger(iMerger++)) {
// 	D->clear();
// 	D->beginEvent();
// 	D->area().append(* f);
// 	D->run();
//     }
#endif
}

void
TRGCDCModule::endRun() {
#ifdef CDCTRIGGER_DEBUG
    cout << "TRGCDCModule ... endRun called " << endl;
#endif
}

void
TRGCDCModule::terminate() {
#ifdef CDCTRIGGER_DEBUG
    cout << "TRGCDCModule ... terminate called " << endl;
#endif
}

} // namespace Belle2
