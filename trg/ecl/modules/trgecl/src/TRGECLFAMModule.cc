/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#define TRGECLFAM_SHORT_NAMES

//framework headers
#include <framework/logging/Logger.h>

//trg package headers
#include "trg/ecl/modules/trgecl/TRGECLFAMModule.h"
#include "trg/ecl/TrgEclDigitizer.h"
#include "trg/ecl/TrgEclFAMFit.h"

#include "trg/ecl/dbobjects/TRGECLFAMPara.h"

#include <iostream>

using namespace std;

namespace Belle2 {
//
//
//! Register module name
  REG_MODULE(TRGECLFAM);
//
//
//
  string
  TRGECLFAMModule::version() const
  {
    return string("TRGECLFAMModule 1.00");
  }
//
//
//
  TRGECLFAMModule::TRGECLFAMModule()
    : Module::Module(),
      _debugLevel(0),
      _famMethod(1),
      _binTimeInterval(125),
      _waveform(0),
      _beambkgtag(0),
      _famana(0),
      _threshold(100.0),
      _FADC(1),
      _ConditionDB(0)

  {

    string desc = "TRGECLFAMModule(" + version() + ")";
    setDescription(desc);
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("DebugLevel", _debugLevel, "TRGECL debug level", _debugLevel);
    addParam("FAMFitMethod", _famMethod, "TRGECLFAM fit method", _famMethod);
    addParam("FAMBinTimeInterval", _binTimeInterval, "TRGECLFAM binTimeInterval",
             _binTimeInterval);
    addParam("TCWaveform", _waveform, "Output the TC waveform ",
             _waveform);
    addParam("FAMAnaTable", _famana, "Save FAM ana table ",
             _famana);
    addParam("BeamBkgTag", _beambkgtag, "Save beambackground tag in TRGECLHit table ",
             _beambkgtag);
    addParam("TCThreshold", _threshold, "Set FAM TC threshold ",
             _threshold);
    addParam("ShapingFunction", _FADC, "Set function of shaper ",  _FADC);
    addParam("ConditionDB", _ConditionDB, "Use conditionDB ",  _ConditionDB);

    if (_ConditionDB == 1) { //Use global tag
      m_FAMPara.addCallback(this, &TRGECLFAMModule::beginRun);
    }
    B2DEBUG(100, "TRGECLFAMModule ... created");
    Threshold.clear();
  }
//
//
//
  TRGECLFAMModule::~TRGECLFAMModule()
  {

    B2DEBUG(100, "TRGECLFAMModule ... destructed ");


  }
//
//
//
  void
  TRGECLFAMModule::initialize()
  {

    B2DEBUG(100, "TRGECLFAMModule::initialize ... options");
    B2DEBUG(100, "TRGECLFAMModule::initialize> FAM Fit Method = "
            << _famMethod << "  ; Bin of Time Interval = " << _binTimeInterval << " ;output TC waveforml = " << _waveform);

    m_nRun   = 0;
    m_nEvent = 1;

    m_TRGECLDigi0.registerInDataStore();
    m_TRGECLWaveform.registerInDataStore();
    m_TRGECLHit.registerInDataStore();
    m_TRGECLFAMAna.registerInDataStore();
    m_eventLevelClusteringInfo.registerInDataStore();

    //    m_FAMPara = new DBObjPtr<TRGECLFAMPara>;
  }
//
//
//
  void
  TRGECLFAMModule::beginRun()
  {
    if (_ConditionDB == 0) {
      Threshold.resize(576, _threshold);
    } else if (_ConditionDB == 1) { //Use global tag
      Threshold.resize(576, 0);
      for (const auto& para : m_FAMPara) {
        Threshold[para.getTCId() - 1] = (int)((para.getThreshold()) * (para.getConversionFactor()));
      }
    }

    B2DEBUG(200, "TRGECLFAMModule ... beginRun called ");

  }
//
//
//
  void
  TRGECLFAMModule::event()
  {

    B2DEBUG(200, "TRGECLFAMMoudle ... event called");
    //
    //
    if (m_nEvent < 1e2) {if (m_nEvent %    10 == 0) {B2DEBUG(200, "TRGECLFAMModule::event> evtno= " << m_nEvent);}}
    else if (m_nEvent < 1e3) {if (m_nEvent %   100 == 0) {B2DEBUG(200, "TRGECLFAMModule::event> evtno= " << m_nEvent);}}
    else if (m_nEvent < 1e4) {if (m_nEvent %  1000 == 0) {B2DEBUG(200, "TRGECLFAMModule::event> evtno= " << m_nEvent);}}
    else if (m_nEvent < 1e5) {if (m_nEvent % 10000 == 0) {B2DEBUG(200, "TRGECLFAMModule::event> evtno= " << m_nEvent);}}
    else if (m_nEvent < 1e6) {if (m_nEvent % 100000 == 0) {B2DEBUG(200, "TRGECLFAMModule::event> evtno= " << m_nEvent);}}


    //
    //
    //
    // FAM Digitizer
    TrgEclDigitizer* obj_trgeclDigi = new TrgEclDigitizer();
    obj_trgeclDigi-> setWaveform(_waveform);
    obj_trgeclDigi-> setFADC(_FADC);
    obj_trgeclDigi-> setup();
    if (_famMethod == 2 || _famMethod == 1) {obj_trgeclDigi->  digitization01(TCDigiE, TCDigiT); } // no-fit method = backup method 1
    else if (_famMethod == 3) { obj_trgeclDigi-> digitization02(TCDigiE, TCDigiT); } // orignal method = backup method 2
    obj_trgeclDigi-> save(m_nEvent);


    // FAM Fitter
    TrgEclFAMFit* obj_trgeclfit = new TrgEclFAMFit();
    obj_trgeclfit-> SetBeamBkgTagFlag(_beambkgtag);
    obj_trgeclfit-> SetAnaTagFlag(_famana);
    obj_trgeclfit-> setup(m_nEvent);
    obj_trgeclfit-> SetThreshold(Threshold);

    if (_famMethod == 1) {obj_trgeclfit->  FAMFit01(TCDigiE, TCDigiT); } // fitting method
    else if (_famMethod == 2) {obj_trgeclfit->  FAMFit02(TCDigiE, TCDigiT); } // no-fit method = backup method 1
    else if (_famMethod == 3) { obj_trgeclfit-> FAMFit03(TCDigiE, TCDigiT); } // orignal method = backup method 2
    obj_trgeclfit-> save(m_nEvent);


    // Count number of trigger cells in each ECL region for EventLevelClusteringInfo
    uint16_t nTCsPerRegion[3] = {};
    const double absTimeRequirement = 9999.; // Selection on time to reproduce data
    const int firstBarrelId = 81; // First TCId in the barrel
    const int lastBarrelId = 512; // Last TCId in the barrel
    for (auto& trgeclhit : m_TRGECLHit) {
      const int tcId = trgeclhit.getTCId();
      const double tcTime = trgeclhit.getTimeAve();
      if (std::abs(tcTime) < absTimeRequirement) {
        if (tcId < firstBarrelId) {
          nTCsPerRegion[0]++;
        } else if (tcId > lastBarrelId) {
          nTCsPerRegion[2]++;
        } else {
          nTCsPerRegion[1]++;
        }
      }
    }

    // Store
    if (!m_eventLevelClusteringInfo) { m_eventLevelClusteringInfo.create();}
    m_eventLevelClusteringInfo->setNECLTriggerCellsFWD(nTCsPerRegion[0]);
    m_eventLevelClusteringInfo->setNECLTriggerCellsBarrel(nTCsPerRegion[1]);
    m_eventLevelClusteringInfo->setNECLTriggerCellsBWD(nTCsPerRegion[2]);


    //
    //
    //
    m_nEvent++;
    delete obj_trgeclDigi;
    delete obj_trgeclfit;
    //
    //
    //
  }
//
//
//
  void
  TRGECLFAMModule::endRun()
  {
    B2DEBUG(200, "TRGECLFAMModule ... endRun called ");
  }
//
//
//
  void
  TRGECLFAMModule::terminate()
  {
    B2DEBUG(100, "TRGECLFAMModule ... terminate called ");
  }
//
//
//
} // namespace Belle2
