//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TrasanModule.cc
// Section  : Trigger CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A trigger module for CDC
//-----------------------------------------------------------------------------
// 0.00 : 2010/10/15 : First version
//-----------------------------------------------------------------------------

#include <iostream>
#include "tracking/modules/trasan/TrasanModule.h"

using namespace std;

namespace Belle2 {

REG_MODULE(TrasanModule, "Trasan");

TrasanModule::TrasanModule()
    : Module::Module(),
      _debugLevel(0),
      _testParamInt(0),
      _tra(Belle::Trasan()) {

    string desc = "TrasanModule(" + _tra.version() + ")";
    setDescription(desc);

    addParam("testParamInt", _testParamInt, 20);

#ifdef TRASAN_DEBUG
    cout << "TrasanModule ... created" << endl;
#endif
}

TrasanModule::~TrasanModule() {
#ifdef TRASAN_DEBUG
    cout << "TrasanModule ... destructed " << endl;
#endif
}

void
TrasanModule::initialize() {
    _tra.initialize();
#ifdef TRASAN_DEBUG
//  _cdc->dump("geometry superLayers layers wires detail");
//  _cdc->dump("geometry superLayers layers detail");
    cout << "TrasanModule ... initialized" << endl;
#endif
}

void
TrasanModule::beginRun() {
    _tra.beginRun();
#ifdef TRASAN_DEBUG
    cout << "TrasanModule ... beginRun called " << endl;
#endif
}

void
TrasanModule::event() {
    _tra.event();
#ifdef TRASAN_DEBUG
    cout << "TrasanModule ... event called " << endl;
#endif
}

void
TrasanModule::endRun() {
    _tra.endRun();
#ifdef TRASAN_DEBUG
    cout << "TrasanModule ... endRun called " << endl;
#endif
}

void
TrasanModule::terminate() {
    _tra.terminate();
#ifdef TRASAN_DEBUG
    cout << "TrasanModule ... terminate called " << endl;
#endif
}

} // namespace Belle2
