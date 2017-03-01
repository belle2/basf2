//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGGDLModule.cc
// Section  : TRG GDL
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A trigger module for GDL
//-----------------------------------------------------------------------------
// 0.00 : 2013/12/13 : First version
//-----------------------------------------------------------------------------

#define TRGGDL_SHORT_NAMES

#include <stdlib.h>
#include <iostream>

#include "trg/trg/Debug.h"
#include "trg/gdl/modules/trggdl/TRGGDLModule.h"
//framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <trg/gdl/dataobjects/TRGGDLResults.h>
#include <trg/grl/dataobjects/TRGGRLInfo.h>

using namespace std;

namespace Belle2 {

  REG_MODULE(TRGGDL);

  TRGGDL*
  TRGGDLModule::_gdl = 0;

  string
  TRGGDLModule::version() const
  {
    return string("TRGGDLModule 0.00");
  }

  TRGGDLModule::TRGGDLModule()
    : Module::Module(),
      _debugLevel(0),
      _configFilename("TRGGDLConfig.dat"),
      _simulationMode(2),
      _fastSimulationMode(0),
      _firmwareSimulationMode(0)
  {

    string desc = "TRGGDLModule(" + version() + ")";
    setDescription(desc);
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("DebugLevel", _debugLevel, "TRGGDL debug level", _debugLevel);
    addParam("ConfigFile",
             _configFilename,
             "The filename of CDC trigger config file",
             _configFilename);
    addParam("SimulationMode",
             _simulationMode,
             "TRGGDL simulation switch",
             _simulationMode);
    addParam("FastSimulationMode",
             _fastSimulationMode,
             "TRGGDL fast simulation mode",
             _fastSimulationMode);
    addParam("FirmwareSimulationMode",
             _firmwareSimulationMode,
             "TRGGDL firmware simulation mode",
             _firmwareSimulationMode);

    if (TRGDebug::level())
      cout << "TRGGDLModule ... created" << endl;
  }

  TRGGDLModule::~TRGGDLModule()
  {

    if (_gdl)
      TRGGDL::getTRGGDL("good-bye");

    if (TRGDebug::level())
      cout << "TRGGDLModule ... destructed " << endl;
  }

  void
  TRGGDLModule::initialize()
  {

    TRGDebug::level(_debugLevel);

    if (TRGDebug::level()) {
      cout << "TRGGDLModule::initialize ... options" << endl;
      cout << TRGDebug::tab(4) << "debug level = " << TRGDebug::level()
           << endl;
    }
    StoreArray<TRGGDLResults>::registerPersistent();
  }

  void
  TRGGDLModule::beginRun()
  {

    //...GDL config. name...
    string cfn = _configFilename;

    //...GDL...
    if (_gdl == 0) {
      _gdl = TRGGDL::getTRGGDL(cfn,
                               _simulationMode,
                               _fastSimulationMode,
                               _firmwareSimulationMode);
    } else if (cfn != _gdl->configFile()) {
      _gdl = TRGGDL::getTRGGDL(cfn,
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
  TRGGDLModule::event()
  {
    TRGDebug::enterStage("TRGGDLModule event");

    //...GDL simulation...
    _gdl->update(true);
    _gdl->simulate();

    TRGDebug::leaveStage("TRGGDLModule event");
  }

  void
  TRGGDLModule::endRun()
  {
    if (TRGDebug::level())
      cout << "TRGGDLModule ... endRun called " << endl;
  }

  void
  TRGGDLModule::terminate()
  {

    _gdl->terminate();

    if (TRGDebug::level())
      cout << "TRGGDLModule ... terminate called " << endl;
  }

} // namespace Belle2
