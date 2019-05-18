/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclDQMInjection/eclDQMInjection.h>
#include "TDirectory.h"

using namespace std;
using namespace Belle2;
using namespace Belle2::ECL;
using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLDQMInjection)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLDQMInjectionModule::ECLDQMInjectionModule() : HistoModule()
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

  // cd back to root directory
  oldDir->cd();
}

void ECLDQMInjectionModule::initialize()
{
  REG_HISTOGRAM
  m_rawTTD.isOptional() /// TODO better use isRequired(), but RawFTSW is not in sim, thus tests are failin
  m_storeHits.isRequired(m_ECLDigitsName);
}

void ECLDQMInjectionModule::beginRun()
{
  // Assume that everthing is non-yero ;-)
  hOccAfterInjLER->Reset();
  hOccAfterInjHER->Reset();
  hEOccAfterInjLER->Reset();
  hEOccAfterInjHER->Reset();
}

void ECLDQMInjectionModule::event()
{

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
      } else {
        hOccAfterInjLER->Fill(diff2, all);
        hEOccAfterInjLER->Fill(diff2);
      }
    }

    break;
  }
}
