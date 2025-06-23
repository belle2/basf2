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
      m_debugLevel(0),
      m_famMethod(1),
      m_binTimeInterval(125),
      m_SaveTCWaveForm(0),
      m_beambkgtag(0),
      m_famana(0),
      m_SetTCEThreshold(100.0),
      m_FADC(1),
      m_ConditionDB(0),
      m_SourceOfTC(3)
  {

    string desc = "TRGECLFAMModule(" + version() + ")";
    setDescription(desc);
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("DebugLevel", m_debugLevel, "TRGECL debug level", m_debugLevel);
    addParam("FAMFitMethod", m_famMethod, "TRGECLFAM fit method", m_famMethod);
    addParam("FAMBinTimeInterval", m_binTimeInterval, "TRGECLFAM binTimeInterval",
             m_binTimeInterval);
    addParam("TCWaveform", m_SaveTCWaveForm, "Output the TC waveform ",
             m_SaveTCWaveForm);
    addParam("FAMAnaTable", m_famana, "Save FAM ana table ",
             m_famana);
    addParam("BeamBkgTag", m_beambkgtag, "Save beambackground tag in TRGECLHit table ",
             m_beambkgtag);
    addParam("TCThreshold", m_SetTCEThreshold, "Set FAM TC threshold ",
             m_SetTCEThreshold);
    addParam("ShapingFunction", m_FADC, "Set function of shaper ",  m_FADC);
    addParam("ConditionDB", m_ConditionDB, "Use conditionDB ",  m_ConditionDB);
    addParam("SourceOfTC", m_SourceOfTC,
             "Select source of TC data(1:=ECLHit or 2:=ECLSimHit or 3:=ECLHit+TRGECLBGTCHit)",
             m_SourceOfTC);

    if (m_ConditionDB == 1) { //Use global tag
      m_FAMPara.addCallback(this, &TRGECLFAMModule::beginRun);
    }
    B2DEBUG(100, "TRGECLFAMModule ... created");
    m_TCEThreshold.clear();

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
    B2DEBUG(100, "TRGECLFAMModule::initialize> FAM Fit Method = " << m_famMethod
            << "  ; Bin of Time Interval = " << m_binTimeInterval
            << " ;output TC waveforml = " << m_SaveTCWaveForm);
    if (m_SourceOfTC <= 0 ||
        m_SourceOfTC >= 4) {
      B2FATAL("TRGECLFAMModule::initialize> SourceOfTC must be 1 or 2 or 3");
    }

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
    if (m_ConditionDB == 0) {
      m_TCEThreshold.resize(576, m_SetTCEThreshold);
    } else if (m_ConditionDB == 1) { //Use global tag
      m_TCEThreshold.resize(576, 0);
      for (const auto& para : m_FAMPara) {
        m_TCEThreshold[para.getTCId() - 1] = (int)((para.getThreshold()) * (para.getConversionFactor()));
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
    obj_trgeclDigi->setWaveform(m_SaveTCWaveForm);
    obj_trgeclDigi->setFADC(m_FADC);
    obj_trgeclDigi->setup(m_SourceOfTC);
    if (m_famMethod == 1 || m_famMethod == 2) {
      // no-fit method = backup method 1
      obj_trgeclDigi->digitization01(m_TCDigiE, m_TCDigiT);
    } else if (m_famMethod == 3) {
      // orignal method = backup method 2
      obj_trgeclDigi->digitization02(m_TCDigiE, m_TCDigiT);
    }
    obj_trgeclDigi->save(m_nEvent);


    // FAM Fitter
    TrgEclFAMFit* obj_trgeclfit = new TrgEclFAMFit();
    obj_trgeclfit->setBeamBkgTagFlag(m_beambkgtag);
    obj_trgeclfit->setAnaTagFlag(m_famana);
    obj_trgeclfit->setup(m_nEvent);
    obj_trgeclfit->setThreshold(m_TCEThreshold);

    if (m_famMethod == 1) { obj_trgeclfit->FAMFit01(m_TCDigiE, m_TCDigiT); }      // fitting method
    else if (m_famMethod == 2) { obj_trgeclfit->FAMFit02(m_TCDigiE, m_TCDigiT); } // no-fit method = backup method 1
    else if (m_famMethod == 3) { obj_trgeclfit->FAMFit03(m_TCDigiE, m_TCDigiT); } // orignal method = backup method 2
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
    m_nEvent++;
    delete obj_trgeclDigi;
    delete obj_trgeclfit;
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
