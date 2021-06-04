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

#include <iostream>

#include "trg/trg/Debug.h"
#include "trg/gdl/modules/trggdl/TRGGDLModule.h"
//framework
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

  TRGGDLModule::TRGGDLModule() : HistoModule(),
//TRGGDLModule::TRGGDLModule()
//  : Module::Module(),
    _debugLevel(0),
    _configFilename("TRGGDLConfig.dat"),
    _simulationMode(1),
    _fastSimulationMode(0),
    _firmwareSimulationMode(0),
    _Phase("Phase2"),
    _algFilePath("ftd.alg")
  {

    string desc = "TRGGDLModule(" + version() + ")";
    setDescription(desc);
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("debugLevel", _debugLevel, "Debug Level", _debugLevel);
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
             _algFromDB,
             "Set false when alg is taken from local file.",
             true);

    addParam("algFilePath",
             _algFilePath,
             ".alg file path",
             _algFilePath);

    addParam("timquality_threshold_sfin",
             _timquality_threshold_sfin,
             "Threshold to determine timing quality flag with MC truth: superfine",
             -1.0);

    addParam("timquality_threshold_fine",
             _timquality_threshold_fine,
             "Threshold to determine timing quality flag with MC truth: fine",
             10.0);

    addParam("simulateT0jitter",
             m_simulateT0Jitter,
             "if True, L1 jitter is simulated by EventT0Generator.",
             false);

    B2DEBUG(100, "TRGGDLModule ... created");
  }

  TRGGDLModule::~TRGGDLModule()
  {

    if (_gdl)
      B2DEBUG(100, "good-bye");

    B2DEBUG(100,  "TRGGDLModule ... destructed ");
  }

  void TRGGDLModule::defineHisto()
  {

    oldDir = gDirectory;
    newDir = gDirectory;
    oldDir->mkdir("TRGGDLModule");
    newDir->cd("TRGGDLModule");

    h_inp  = new TH1I("hTRGGDL_inp", "input bits from TRGGDLModule", 200, 0, 200);
    h_ftd  = new TH1I("hTRGGDL_ftd", "ftdl  bits from TRGGDLModule", 200, 0, 200);
    h_psn  = new TH1I("hTRGGDL_psn", "psnm  bits from TRGGDLModule", 200, 0, 200);

    oldDir->cd();

  }

  void
  TRGGDLModule::initialize()
  {

    REG_HISTOGRAM
    defineHisto();

//  m_TRGSummary.isRequired();
    TRGDebug::level(_debugLevel);

    B2INFO("TRGGDLModule::initialize. simulationMode=" << _simulationMode);
    if (_simulationMode != 3) {
      m_TRGGRLInfo.isRequired("TRGGRLObjects");
    }

    if (m_simulateT0Jitter)
      m_simClockState.isRequired();

    m_TRGSummary.registerInDataStore();
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
                               _algFromDB,
                               _algFilePath,
                               _debugLevel,
                               _timquality_threshold_sfin,
                               _timquality_threshold_fine);
    } else if (cfn != _gdl->configFile()) {
      _gdl = TRGGDL::getTRGGDL(cfn,
                               _simulationMode,
                               _fastSimulationMode,
                               _firmwareSimulationMode,
                               _Phase,
                               _algFromDB,
                               _algFilePath,
                               _debugLevel,
                               _timquality_threshold_sfin,
                               _timquality_threshold_fine);
    }
    if (_debugLevel > 9) printf("TRGGDLModule::beginRun() ends.\n");

    _gdl->checkDatabase();

    B2DEBUG(100, "TRGGDLModule ... beginRun called  configFile = " << cfn);
  }

  void
  TRGGDLModule::event()
  {

    if (_debugLevel > 9) printf("TRGGDLModule::event() starts.\n");

    newDir->cd();

    //...GDL simulation...
    _gdl->update(true);
    _gdl->simulate();
    _gdl->accumulateInp(h_inp);
    _gdl->accumulateFtd(h_ftd);
    _gdl->accumulatePsn(h_psn);

    //StoreObjPtr<TRGSummary> m_TRGSummary; /**< output for TRGSummary */
    int result_summary = 0;
    if (m_TRGSummary) {
      result_summary = m_TRGSummary->getTRGSummary(0);
    } else {
      B2WARNING("TRGGDLModule.cc: TRGSummary not found. Check it!!!!");
    }
    setReturnValue(result_summary);

    oldDir->cd();
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
