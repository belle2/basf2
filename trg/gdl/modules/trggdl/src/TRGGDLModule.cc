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
#include <framework/logging/Logger.h>

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
      _simulationMode(1),
      _fastSimulationMode(0),
      _firmwareSimulationMode(0),
      _Phase("Phase2")
  {

    string desc = "TRGGDLModule(" + version() + ")";
    setDescription(desc);
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("DebugLevel", _debugLevel, "TRGGDL debug level", _debugLevel);
    addParam("Belle2Phase", _Phase, "Phase2 or Phase3", _Phase);
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

    addParam("algFromDB",
             _alg_from_db,
             "Set false when alg is taken from local file.",
             true);

    B2DEBUG(100, "TRGGDLModule ... created");
  }

  TRGGDLModule::~TRGGDLModule()
  {

    if (_gdl)
      B2DEBUG(100, "good-bye");

    B2DEBUG(100,  "TRGGDLModule ... destructed ");
  }

  void
  TRGGDLModule::initialize()
  {

//  m_TRGSummary.isRequired();
    TRGDebug::level(_debugLevel);

    B2DEBUG(100, "TRGGDLModule::initialize ... options");
    if (_simulationMode != 3) {
      m_TRGGRLInfo.isRequired("TRGGRLObjects");
    }
//  m_TRGSummary.registerInDataStore();
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
                               _firmwareSimulationMode,
                               _Phase,
                               _alg_from_db);
    } else if (cfn != _gdl->configFile()) {
      _gdl = TRGGDL::getTRGGDL(cfn,
                               _simulationMode,
                               _fastSimulationMode,
                               _firmwareSimulationMode,
                               _Phase,
                               _alg_from_db);
    }

    B2DEBUG(100, "TRGGDLModule ... beginRun called  configFile = " << cfn);
  }

  void
  TRGGDLModule::event()
  {
    /*
    StoreObjPtr<EventMetaData> bevt;
    unsigned _exp = bevt->getExperiment();
    unsigned _run = bevt->getRun();
    unsigned _evt = bevt->getEvent();
    std::cout << "evt(" << _evt << ") " << std::endl;
    */

    TRGDebug::enterStage("TRGGDLModule event");
    //...GDL simulation...
    _gdl->update(true);
    _gdl->simulate();

    StoreObjPtr<TRGSummary> m_TRGSummary; /**< output for TRGSummary */
    int result_summary = 0;
    if (m_TRGSummary) {
      result_summary = m_TRGSummary->getTRGSummary(0);
    } else {
      B2WARNING("TRGGDLModule.cc: TRGSummary not found. Check it!!!!");
    }
    setReturnValue(result_summary);

    TRGDebug::leaveStage("TRGGDLModule event");

  }

  void
  TRGGDLModule::endRun()
  {
    B2DEBUG(200, "TRGGDLModule ... endRun called ");
  }

  void
  TRGGDLModule::terminate()
  {

    _gdl->terminate();

    B2DEBUG(100, "TRGGDLModule ... terminate called ");
  }

} // namespace Belle2
