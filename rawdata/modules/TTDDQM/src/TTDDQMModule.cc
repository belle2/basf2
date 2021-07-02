/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
  // his DQM module can be run offline or online
  // if included in online DQM, binning should be reviewed (size!) and
  // adapted to the binning of the corresponding sub detector (or GDL) histograms
  // it may replace some of the histograms currently filled by sub-detectors
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

  hTriggersDeltaT = new TH1I("hTTDTriggersDeltaT",
                             "#delta Trigger Time since previous trigger;#delta t in #mus;Triggers/Time (0.5 #mus bins)", 100000, 0, 50000);
  hTriggersPerBunch = new TH1I("hTTDTriggerBunch", "Triggers per Bunch;Bunch(rel);Triggers per 4 Bunches)", 1280, 0, 1280 * 4);

  hBunchInjHER = new TH1I("hTTDBunchInjHER", "Last Injected Bunch HER;Bunch(rel);Counts per 4 Bunches", 1280, 0, 1280 * 4);
  hBunchInjLER = new TH1I("hTTDBunchInjLER", "Last Injected Bunch LER;Bunch(rel);Counts per 4 Bunches", 1280, 0, 1280 * 4);

  hTrigBunchInjLER =  new TH2I("hTTDTrigBunchInjLER",
                               "Correlation between triggered bunch and injected bunch in LER;Injected Bunch(rel);Triggered Bunch(rel)", 256, 0, 1280 * 4, 256, 0,
                               1280 * 4);
  hTrigBunchInjHER =  new TH2I("hTTDTrigBunchInjHer",
                               "Correlation between triggered bunch and injected bunch in HER;Injected Bunch(rel);Triggered Bunch(rel)", 256, 0, 1280 * 4, 256, 0,
                               1280 * 4);

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
  // all ptr are set in defineHisto, thus no need for nullptr check
  hTrigAfterInjLER->Reset();
  hTrigAfterInjHER->Reset();
  hTriggersDeltaT->Reset();
  hTriggersPerBunch->Reset();
  hBunchInjHER->Reset();
  hBunchInjLER->Reset();
  hTrigBunchInjLER->Reset();
  hTrigBunchInjHER->Reset();
}

void TTDDQMModule::event()
{

  if (m_EventLevelTriggerTimeInfo->isValid()) {
    // TODO conversion of clock ticks to time not yet done in EventLevelTriggerTimeInfo
    // all values are given in clock ticks of RF clock/4, thus one tick correspond to 4 bunches
    // ~ 508 MHz RF -> 127 = 508/4 (clock ticks)
    // 5120 possible bunch positions from RF and ring circumference -> 1280 = 5120/4 (in ticks)
    // use an int and not the real fraction number to avoid binning effects for getting inexact time
    // any bunch number is currently relative and not matching the SKB number (offset differs for LER, HER)
    // time after injection is time after prekick signal, real injection happens ~90us afterwards (offset differs for LER, HER)
    hTriggersDeltaT->Fill(m_EventLevelTriggerTimeInfo->getTimeSincePrevTrigger() / 127.);
    int triggered_bunch_in_ticks = m_EventLevelTriggerTimeInfo->getBunchNumber();
    hTriggersPerBunch->Fill(triggered_bunch_in_ticks * 4);


    if (m_EventLevelTriggerTimeInfo->hasInjection()) {
      auto time_since_inj_in_ticks = m_EventLevelTriggerTimeInfo->getTimeSinceLastInjection();// in clock ticks
      //  127MHz clock ticks to us, inexact rounding, use integer to avoid binning effects
      float time_since_inj_in_us = time_since_inj_in_ticks / 127.;
      // swapped? 1280-1-injected_bunch_in_ticks?
      int injected_bunch_in_ticks = ((time_since_inj_in_ticks - triggered_bunch_in_ticks + 1280) % 1280);

      if (m_EventLevelTriggerTimeInfo->isHER()) {
        hTrigAfterInjHER->Fill(time_since_inj_in_us, time_since_inj_in_us - int(time_since_inj_in_us / (5120 / 508.)) * (5120 / 508.));
        hBunchInjHER->Fill(injected_bunch_in_ticks * 4);
        hTrigBunchInjHER->Fill(injected_bunch_in_ticks * 4, triggered_bunch_in_ticks * 4);
      } else {
        hTrigAfterInjLER->Fill(time_since_inj_in_us, time_since_inj_in_us - int(time_since_inj_in_us / (5120 / 508.)) * (5120 / 508.));
        hBunchInjLER->Fill(injected_bunch_in_ticks * 4);
        hTrigBunchInjLER->Fill(injected_bunch_in_ticks * 4, triggered_bunch_in_ticks * 4);
      }
    }
  }
}
