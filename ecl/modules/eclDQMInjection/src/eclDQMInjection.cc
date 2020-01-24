/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck, Dmitry Matvienko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclDQMInjection/eclDQMInjection.h>
#include "TDirectory.h"

using namespace std;
using namespace Belle2;
using namespace Belle2::ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLDQMInjection)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLDQMInjectionModule::ECLDQMInjectionModule()
  : HistoModule(),
    m_calibrationThrApsd("ECL_FPGA_StoreWaveform")
{
  //Set module properties
  setDescription("Monitor Occupancy after Injection");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("ECLINJ"));
  addParam("ECLDigitsName", m_ECLDigitsName, "Name of ECL hits", std::string(""));
}

void ECLDQMInjectionModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir(m_histogramDirectoryName.c_str());// do not rely on return value, might be ZERO
  oldDir->cd(m_histogramDirectoryName.c_str());//changing to the right directory

  hOccAfterInjLER  = new TH1F("ECLOccInjLER", "ECLOccInjLER/Time;Time in #mus;Count/Time (5 #mus bins)", 4000, 0, 20000);
  hOccAfterInjHER  = new TH1F("ECLOccInjHER", "ECLOccInjHER/Time;Time in #mus;Count/Time (5 #mus bins)", 4000, 0, 20000);
  hEOccAfterInjLER  = new TH1F("ECLEOccInjLER", "ECLEOccInjLER/Time;Time in #mus;Triggers/Time (5 #mus bins)", 4000, 0, 20000);
  hEOccAfterInjHER  = new TH1F("ECLEOccInjHER", "ECLEOccInjHER/Time;Time in #mus;Triggers/Time (5 #mus bins)", 4000, 0, 20000);
  hBurstsAfterInjLER = new TH1F("ECLBurstsInjLER", "ECLBurstsInjLER/Time;Time in #mus;Count/Time (1 #mus bins)", 20000, 0, 20000);
  hBurstsAfterInjHER = new TH1F("ECLBurstsInjHER", "ECLBurstsInjHER/Time;Time in #mus;Count/Time (1 #mus bins)", 20000, 0, 20000);
  hEBurstsAfterInjLER = new TH1F("ECLEBurstsInjLER", "ECLEBurstsInjLER/Time;Time in #mus;Triggers/Time (1 #mus bins)", 20000, 0,
                                 20000);
  hEBurstsAfterInjHER = new TH1F("ECLEBurstsInjHER", "ECLEBurstsInjHER/Time;Time in #mus;TRiggers/Time (1 #mus bins)", 20000, 0,
                                 20000);

  // cd back to root directory
  oldDir->cd();
}

void ECLDQMInjectionModule::initialize()
{
  REG_HISTOGRAM
  m_rawTTD.isOptional(); /// TODO better use isRequired(), but RawFTSW is not in sim, thus tests are failin
  m_storeHits.isRequired(m_ECLDigitsName);
  m_ECLTrigs.isOptional();
  m_l1Trigger.isOptional();

  if (!mapper.initFromDB()) B2FATAL("ECL Display:: Can't initialize eclChannelMapper");

  v_totalthrApsd.resize((m_calibrationThrApsd->getCalibVector()).size());
  for (size_t i = 0; i < v_totalthrApsd.size(); i++) v_totalthrApsd[i] = (int)(m_calibrationThrApsd->getCalibVector())[i];
}

void ECLDQMInjectionModule::beginRun()
{
  // Assume that everthing is non-yero ;-)
  hOccAfterInjLER->Reset();
  hOccAfterInjHER->Reset();
  hEOccAfterInjLER->Reset();
  hEOccAfterInjHER->Reset();
  hBurstsAfterInjLER->Reset();
  hBurstsAfterInjHER->Reset();
}

void ECLDQMInjectionModule::event()
{
  if (m_eventmetadata.isValid()) {
    m_iEvent = m_eventmetadata->getEvent();
  } else m_iEvent = -1;
  int discarded_wfs = 0;
  for (auto& aECLTrig : m_ECLTrigs) {
    int crate = aECLTrig.getTrigId();
    int suppress = aECLTrig.getBurstSuppressionMask();
    int shaper_pos = 0;
    while (suppress) {
      shaper_pos ++;
      bool shaper_bit = suppress & 1;
      if (shaper_bit) {
        if (m_iEvent % 1000 == 999 || (m_l1Trigger.isValid() &&  m_l1Trigger->getTimType() == TRGSummary::ETimingType::TTYP_RAND) ||
            (m_l1Trigger.isValid() &&  m_l1Trigger->getTimType() == TRGSummary::ETimingType::TTYP_DPHY)) {
          for (int channel_pos = 0; channel_pos < 16; channel_pos ++) {
            if (mapper.getCellId(crate, shaper_pos, channel_pos) > 0) discarded_wfs += 1;
          }
        } else {
          for (auto& aECLDigit : m_storeHits) {
            if (crate == mapper.getCrateID(aECLDigit.getCellId()) && shaper_pos == mapper.getShaperPosition(aECLDigit.getCellId()) &&
                aECLDigit.getAmp() >= (v_totalthrApsd[aECLDigit.getCellId() - 1] / 4 * 4)) discarded_wfs += 1;
          }
        }
      }
      suppress >>= 1;
    }
  }

  for (auto& it : m_rawTTD) {
    B2DEBUG(29, "TTD FTSW : " << hex << it.GetTTUtime(0) << " " << it.GetTTCtime(0) << " EvtNr " << it.GetEveNo(0)  << " Type " <<
            (it.GetTTCtimeTRGType(0) & 0xF) << " TimeSincePrev " << it.GetTimeSincePrevTrigger(0) << " TimeSinceInj " <<
            it.GetTimeSinceLastInjection(0) << " IsHER " << it.GetIsHER(0) << " Bunch " << it.GetBunchNumber(0));

    // get last injection time
    auto difference = it.GetTimeSinceLastInjection(0);
    // check time overflow, too long ago
    if (difference != 0x7FFFFFFF) {
      unsigned int all = m_storeHits.getEntries();
      float diff2 = difference / 127.; //  127MHz clock ticks to us, inexact rounding
      if (it.GetIsHER(0)) {
        hOccAfterInjHER->Fill(diff2, all);
        hEOccAfterInjHER->Fill(diff2);
        hBurstsAfterInjHER->Fill(diff2, discarded_wfs);
        hEBurstsAfterInjHER->Fill(diff2);
      } else {
        hOccAfterInjLER->Fill(diff2, all);
        hEOccAfterInjLER->Fill(diff2);
        hBurstsAfterInjLER->Fill(diff2, discarded_wfs);
        hEBurstsAfterInjLER->Fill(diff2);
      }
    }

    break;
  }
}
