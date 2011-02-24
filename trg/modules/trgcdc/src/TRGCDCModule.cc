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

using namespace std;

namespace Belle2 {

#ifdef BUILD_2010_12_13
REG_XXX_MODULE(TRGCDCModule, "TRGCDC");
#else
REG_MODULE(TRGCDC);
#endif

string
TRGCDCModule::version() const {
    return "0.03";
}

TRGCDCModule::TRGCDCModule()
    : Module::Module(),
      _debugLevel(0),
      _configFilename("TRGCDCConfig.dat"),
      _curlBackStop(0),
      _cdc(0),
      _sa(0) {

    string desc = "TRGCDCModule(" + version() + ")";
    setDescription(desc);

#ifdef BUILD_2010_12_13
    addParam("testParamInt", _testParamInt, 20);
    addParam("ConfigFile",
             _configFilename,
             string("TRGCDCConfig.dat"),
             "The filename of CDC trigger config file");
#else
    addParam("ConfigFile",
             _configFilename,
             "The filename of CDC trigger config file",
             string("TRGCDCConfig.dat"));
    addParam("CurlBackStop", _curlBackStop, "Curl back stop parameter", 0);
#endif

#ifdef TRGCDC_DEBUG
    cout << "TRGCDCModule ... created" << endl;
#endif
}

TRGCDCModule::~TRGCDCModule() {

    if (_cdc)
	TRGCDC::getTRGCDC("good-bye");

    //...Maybe G4RunManager delete it, so don't delete _sa.
//  if (_sa)
// 	delete _sa;

#ifdef TRGCDC_DEBUG
    cout << "TRGCDCModule ... destructed " << endl;
#endif
}

void
TRGCDCModule::initialize() {

    //...Stop curl buck...
    if (_curlBackStop) {
	G4RunManager * g4rm = G4RunManager::GetRunManager();
	_sa = new TCSAction();
	g4rm->SetUserAction(_sa);
    }
}

void
TRGCDCModule::beginRun() {

    //...CDC trigger config. name...
    static string cfn = _configFilename;

    //...CDC trigger...
    if ((cfn != _configFilename) || (_cdc == 0))
	_cdc = TRGCDC::getTRGCDC(_configFilename);

#ifdef TRGCDC_DEBUG
    cout << "TRGCDCModule ... beginRun called " << endl;
#endif
}

void
TRGCDCModule::event() {

#ifdef TRGCDC_DEBUG
//  _cdc->dump("geometry superLayers layers wires detail");
//  _cdc->dump("geometry superLayers layers detail");
#endif

    //...CDC trigger simulation...
    _cdc->update();
    _cdc->simulate();
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
