//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGGRLModule.cc
// Section  : TRG GRL
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A trigger module for GRL
//-----------------------------------------------------------------------------
// 0.00 : 2013/12/13 : First version
//-----------------------------------------------------------------------------

#define TRGGRL_SHORT_NAMES

#include <stdlib.h>
#include <iostream>
#include "framework/core/ModuleManager.h"
#include "trg/trg/Debug.h"
#include "trg/grl/modules/trggrl/TRGGRLModule.h"

using namespace std;

namespace Belle2 {

REG_MODULE(TRGGRL);

TRGGRL *
TRGGRLModule::_grl = 0;

string
TRGGRLModule::version() const {
    return string("TRGGRLModule 0.00");
}

TRGGRLModule::TRGGRLModule()
    : Module::Module(),
      _debugLevel(0),
      _configFilename("TRGGDLConfig.dat"),
      _simulationMode(2),
      _fastSimulationMode(0),
      _firmwareSimulationMode(0) {
    
    string desc = "TRGGRLModule(" + version() + ")";
    setDescription(desc);
    setPropertyFlags(c_ParallelProcessingCertified);
    
    addParam("DebugLevel", _debugLevel, "TRGGRL debug level", _debugLevel);
    addParam("ConfigFile",
             _configFilename,
             "The filename of CDC trigger config file",
             _configFilename);
    addParam("SimulationMode",
             _simulationMode,
             "TRGGRL simulation switch",
             _simulationMode);
    addParam("FastSimulationMode",
             _fastSimulationMode,
             "TRGGRL fast simulation mode",
             _fastSimulationMode);
    addParam("FirmwareSimulationMode",
             _firmwareSimulationMode,
             "TRGGRL firmware simulation mode",
             _firmwareSimulationMode);
    
    if (TRGDebug::level())
        cout << "TRGGRLModule ... created" << endl;
}

TRGGRLModule::~TRGGRLModule() {

    if (_grl)
        TRGGRL::getTRGGRL("good-bye");

    if (TRGDebug::level())
        cout << "TRGGRLModule ... destructed " << endl;
}

void
TRGGRLModule::initialize() {

//  TRGDebug::level(_debugLevel);

    if (TRGDebug::level()) {
	cout << "TRGGRLModule::initialize ... options" << endl;
	cout << TRGDebug::tab(4) << "debug level = " << TRGDebug::level()
	     << endl;
    }
}

void
TRGGRLModule::beginRun() {

    //...GDL config. name...
    string cfn = _configFilename;

    //...GRL...
    if (_grl == 0) { 
	_grl = TRGGRL::getTRGGRL(cfn,
				 _simulationMode,
				 _fastSimulationMode,
				 _firmwareSimulationMode);
    }
    else if (cfn != _grl->configFile()) {
	_grl = TRGGRL::getTRGGRL(cfn,
				 _simulationMode,
				 _fastSimulationMode,
				 _firmwareSimulationMode);
    }

    if (TRGDebug::level()) {
        cout << "TRGGDLModule ... beginRun called " << endl;
        cout << "                 configFile = " << cfn << endl;
    }
}

void
TRGGRLModule::event() {
    TRGDebug::enterStage("TRGGRLModule event");

    //...GRL simulation...
    _grl->update(true);
    _grl->simulate();

    TRGDebug::leaveStage("TRGGRLModule event");
}

void
TRGGRLModule::endRun() {
    if (TRGDebug::level())
        cout << "TRGGRLModule ... endRun called " << endl;
}


void
TRGGRLModule::terminate() {
    _grl->terminate();
    if (TRGDebug::level())
	cout << "TRGGRLModule ... terminate called " << endl;
}

} // namespace Belle2
