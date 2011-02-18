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

#include <stdlib.h>
#include <iostream>
#include "G4RunManager.hh"
#include "framework/core/ModuleManager.h"
#include "trg/modules/trgcdc/TRGCDCModule.h"
#include "trg/cdc/SteppingAction.h"

using namespace std;

namespace Belle2 {

#ifdef BUILD_2010_12_13
REG_XXX_MODULE(TRGCDCModule, "TRGCDC");
#else
REG_MODULE(TRGCDC);
#endif

string
TRGCDCModule::version() const {
    return "0.02";
}

TRGCDCModule::TRGCDCModule()
    : Module::Module(),
      _debugLevel(0),
      _testParamInt(0),
      _configFilename("TRGCDCConfig.dat") {

    string desc = "TRGCDCModule(" + version() + ")";
    setDescription(desc);

#ifdef BUILD_2010_12_13
    addParam("testParamInt", _testParamInt, 20);
    addParam("ConfigFile",
             _configFilename,
             string("TRGCDCConfig.dat"),
             "The filename of CDC trigger config file");
#else
    addParam("testParamInt", _testParamInt, "Test Parameter", 20);
    addParam("ConfigFile",
             _configFilename,
             "The filename of CDC trigger config file",
             string("TRGCDCConfig.dat"));
#endif

#ifdef TRGCDC_DEBUG
    cout << "TRGCDCModule ... created" << endl;
#endif
}

TRGCDCModule::~TRGCDCModule() {
#ifdef TRGCDC_DEBUG
    cout << "TRGCDCModule ... destructed " << endl;
#endif
}

void
TRGCDCModule::initialize() {

    //...Stop curl buck...
    G4RunManager * g4rm = G4RunManager::GetRunManager();
    static TCSAction * sa = new TCSAction();
    g4rm->SetUserAction(sa);

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

#ifdef TRGCDC_DEBUG
//  cdc.dump("geometry superLayers layers wires detail");
//  cdc.dump("geometry superLayers layers detail");
#endif

    //...CDC trigger simulation...
    cdc.update();
    cdc.simulate();
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
