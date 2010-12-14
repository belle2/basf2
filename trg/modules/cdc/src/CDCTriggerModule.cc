//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : CDCTriggerModule.cc
// Section  : Trigger CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A trigger module for CDC
//-----------------------------------------------------------------------------
// 0.00 : 2010/10/08 : First version
//-----------------------------------------------------------------------------

#define CDCTRIGGER_SHORT_NAMES

#include <iostream>
#include "framework/core/ModuleManager.h"
#include "trigger/gdl/GDLSignal.h"
#include "trigger/gdl/GDLClock.h"
#include "trigger/cdc/CDCTriggerTrackSegment.h"
#include "trigger/modules/cdc/CDCTriggerModule.h"

#ifdef CDCTRIGGER_DISPLAY
#undef ERROR
#include "trigger/cdc/CDCTriggerDisplay.h"
#endif

namespace Belle2_CDCTrigger {
#ifdef CDCTRIGGER_DISPLAY
    Belle2::CDCTriggerDisplay * D = 0;
#endif
}

using namespace std;
using namespace Belle2_CDCTrigger;

namespace Belle2 {

REG_MODULE(CDCTriggerModule, "CDCTrigger");

string
CDCTriggerModule::version() const {
    return "0.01";
}

CDCTriggerModule::CDCTriggerModule()
    : Module::Module(),
      _debugLevel(0),
//    _cdc(0),
      _testParamInt(0) {

    string desc = "CDCTriggerModule(" + version() + ")";
    setDescription(desc);

    addParam("testParamInt", _testParamInt, 20);

#ifdef CDCTRIGGER_DEBUG
    cout << "CDCTriggerModule ... created" << endl;
#endif
}

CDCTriggerModule::~CDCTriggerModule() {
#ifdef CDCTRIGGER_DISPLAY
    if (D)
	delete D;
    cout << "CDCTriggerModule ... display deleted" << endl;
#endif
#ifdef CDCTRIGGER_DEBUG
    cout << "CDCTriggerModule ... destructed " << endl;
#endif
}

void
CDCTriggerModule::initialize() {

    //...Do nothing here...

#ifdef CDCTRIGGER_DISPLAY
    int argc = 0;
    char ** argv = 0;
    Gtk::Main main_instance(argc, argv);
    if (! D)
	D = new CTDisplay();
    D->clear();
    D->show();
    cout << "CDCTriggerModule ... GTK initialized" << endl;
#endif
#ifdef CDCTRIGGER_DEBUG
    const CDCTrigger & cdc = * CDCTrigger::getCDCTrigger();
//  cdc.dump("geometry superLayers layers wires detail");
    cdc.dump("geometry superLayers layers detail");
#endif
}

void
CDCTriggerModule::beginRun() {
#ifdef CDCTRIGGER_DEBUG
    cout << "CDCTriggerModule ... beginRun called " << endl;
#endif
}

void
CDCTriggerModule::event() {

    //...CDC trigger...
    CDCTrigger & cdc = * CDCTrigger::getCDCTrigger();

    //...CDC clock...
//  const GDLClock & cdcClock = cdc.systemClock();

    //...CDC trigger simulation...
    cdc.update();
    cdc.simulate();

#ifdef CDCTRIGGER_DEBUG
    cout << "CDCTriggerModule ... event called " << endl;
    cout << "    TS hits" << endl;
    for (unsigned i = 0; i < cdc.nTrackSegments(); i++) {
	const CTTSegment & s = * cdc.trackSegment(i);
	if (s.triggerOutput())
	    s.dump("detail", "        ");
    }
#endif
#ifdef CDCTRIGGER_DISPLAY
//  cdc.dump("hits");
    D->clear();
    D->beginEvent();
    D->area().append(cdc.hits());
    D->area().append(cdc.tsHits(), Gdk::Color("#6600FF009900"));
    D->run();
#endif
}

void
CDCTriggerModule::endRun() {
#ifdef CDCTRIGGER_DEBUG
    cout << "CDCTriggerModule ... endRun called " << endl;
#endif
}

void
CDCTriggerModule::terminate() {
#ifdef CDCTRIGGER_DEBUG
    cout << "CDCTriggerModule ... terminate called " << endl;
#endif
}

} // namespace Belle2
