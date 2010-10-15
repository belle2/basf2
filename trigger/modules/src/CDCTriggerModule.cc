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

#include <iostream>
#include "framework/core/ModuleManager.h"
#include "trigger/modules/CDCTriggerModule.h"

#ifdef CDCTRIGGER_DISPLAY
#undef ERROR
#include "trigger/cdc/CDCTriggerDisplay.h"
#endif

namespace Belle2_CDCTrigger {
//  Belle2::ModuleManager::Registrator<Belle2::CDCTriggerModule> REG;
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

// ModulePtr
// CDCTriggerModule::newModule() {
//     ModulePtr nm(new CDCTriggerModule("CDCTrigger"));
//     return nm;
// };

// CDCTriggerModule::CDCTriggerModule(const string & type)
//     : Module::Module(type),
//       _debugLevel(0),
//       _cdc(0),
//       _testParamInt(0) {

//     string desc = "CDCTriggerModule(" + version() + ")";
//     setDescription(desc);

//     addParam("testParamInt", _testParamInt, 20);

// #ifdef CDCTRIGGER_DEBUG
//     cout << "CDCTriggerModule ... created with " << type << endl;
// #endif
// }

CDCTriggerModule::CDCTriggerModule()
    : Module::Module(),
      _debugLevel(0),
      _cdc(0),
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
    const string versionCDC = "cdc2_test";
    _cdc = CDCTrigger::getCDCTrigger(versionCDC);
    _cdc->debugLevel(_debugLevel);

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
//  _cdc->dump("geometry superLayers layers wires detail");
    _cdc->dump("geometry superLayers layers detail");
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
#ifdef CDCTRIGGER_DEBUG
    cout << "CDCTriggerModule ... event called " << endl;
#endif
#ifdef CDCTRIGGER_DISPLAY
    _cdc->update(false);
//  _cdc->dump("hits");
    D->clear();
    D->beginEvent();
    D->area().append(_cdc->hits());
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
