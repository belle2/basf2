/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <rawdata/modules/TTDDQM/TTDDQMModule.h>
#include <TDirectory.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TTDDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TTDDQMModule::TTDDQMModule() : HistoModule(),  m_EventLevelTriggerTimeInfo()
{
  //Set module properties
  setDescription("Monitor TTD Trigger and Injection");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("TTD"));

}

void TTDDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());// do not rely on return value, might be ZERO
    oldDir->cd(m_histogramDirectoryName.c_str());//changing to the right directory
  }

  hTrigAfterInjLER = new TH2I("hTTDTrigAfterInjLER",
                              "Triggers for LER veto tuning;Time since last injection in #mus;Time within beam cycle in #mus", 500, 0, 30000, 100, 0,
                              5120 / 508.);
  hTrigAfterInjHER = new TH2I("hTTDTrigAfterInjHER",
                              "Triggers for HER veto tuning;Time since last injection in #mus;Time within beam cycle in #mus", 500, 0, 30000, 100, 0,
                              5120 / 508.);

  hTriggersAfterTrigger = new TH1I("hTTDTriggersAfterLast",
                                   "Trigger after Last Trigger;Time diff in #mus;Count/Time (0.5 #mus bins)", 100000, 0, 50000);
  hTriggersPerBunch = new TH1I("hTTDTriggerBunch", "Trigger per Bunch;Bunch/4;Triggers", 1280, 0, 1280);

  hBunchInjHER = new TH1I("hTTDBunchInjHER", "Last Inj Bunch HER;Counts;BunchNr/4", 1280, 0, 1280);
  hBunchInjLER = new TH1I("hTTDBunchInjLER", "Last Inj Bunch LER;Counts;BunchNr/4", 1280, 0, 1280);

  hTrigBunchInjLER =  new TH2I("hTTDTrigBunchInjLER", "TrigBunchInjLER", 256, 0, 1280, 256, 0, 1280);
  hTrigBunchInjHER =  new TH2I("hTTDTrigBunchInjHer", "TrigBunchInjHer", 256, 0, 1280, 256, 0, 1280);

  // cd back to root directory
  oldDir->cd();
}

void TTDDQMModule::initialize()
{
  REG_HISTOGRAM
  m_EventLevelTriggerTimeInfo.isRequired();
}

void TTDDQMModule::beginRun()
{
  // Do not assume that everthing is non-zero, e.g. Max might be nullptr
  hTrigAfterInjLER->Reset();
  hTrigAfterInjHER->Reset();
  hTriggersAfterTrigger->Reset();
  hTriggersPerBunch->Reset();
  hBunchInjHER->Reset();
  hBunchInjLER->Reset();
  hTrigBunchInjLER->Reset();
  hTrigBunchInjHER->Reset();
}

void TTDDQMModule::event()
{

  if (m_EventLevelTriggerTimeInfo->isValid()) {
    hTriggersAfterTrigger->Fill(m_EventLevelTriggerTimeInfo->getTimeSincePrevTrigger() / 127.);
    int bunch_trg = m_EventLevelTriggerTimeInfo->getBunchNumber();
    hTriggersPerBunch->Fill(bunch_trg);


    if (m_EventLevelTriggerTimeInfo->hasInjection()) {
      auto difference = m_EventLevelTriggerTimeInfo->getTimeSinceLastInjection();
      int bunch_inj = ((difference - bunch_trg + 1280) % 1280); // swapped? 1280-1-bunch_inj?

      float diff2 = difference / 127.; //  127MHz clock ticks to us, inexact rounding
      if (m_EventLevelTriggerTimeInfo->isHER()) {
        hTrigAfterInjHER->Fill(diff2, diff2 - int(diff2 / (5120 / 508.)) * (5120 / 508.));
        hBunchInjHER->Fill(bunch_inj);
        hTrigBunchInjHER->Fill(bunch_inj, bunch_trg);
      } else {
        hTrigAfterInjLER->Fill(diff2, diff2 - int(diff2 / (5120 / 508.)) * (5120 / 508.));
        hBunchInjLER->Fill(bunch_inj);
        hTrigBunchInjLER->Fill(bunch_inj, bunch_trg);
      }
    }
  }
}
