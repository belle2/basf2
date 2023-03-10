/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/modules/pxdDQM/PXDGatedDHCDQMModule.h>
#include "TDirectory.h"

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDGatedDHCDQM);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDGatedDHCDQMModule::PXDGatedDHCDQMModule() : HistoModule()
{
  //Set module properties
  setDescription("Monitor Gating after Injection");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("PXDINJ"));
}

void PXDGatedDHCDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir(m_histogramDirectoryName.c_str());// do not rely on return value, might be ZERO
  oldDir->cd(m_histogramDirectoryName.c_str());//changing to the right directory

  hGateAfterInjLER  = new TH2F("PXDGateingInjLER", "PXDGateingInjLER/Time;Time in #mus;Flags", 100000, 0, 50000, 64, 0, 64);
  hGateAfterInjHER  = new TH2F("PXDGateingInjHER", "PXDGateingInjHER/Time;Time in #mus;Flags", 100000, 0, 50000, 64, 0, 64);
  // cd back to root directory
  oldDir->cd();
}

void PXDGatedDHCDQMModule::initialize()
{
  REG_HISTOGRAM
  m_storeDAQEvtStats.isRequired();
  m_EventLevelTriggerTimeInfo.isRequired();
}

void PXDGatedDHCDQMModule::beginRun()
{
  // Assume that everthing is non-zero ;-)
  hGateAfterInjLER->Reset();
  hGateAfterInjHER->Reset();
}

void PXDGatedDHCDQMModule::event()
{

  // And check if the stored data is valid
  if (m_EventLevelTriggerTimeInfo->isValid()) {

    // check time overflow, too long ago
    if (m_EventLevelTriggerTimeInfo->hasInjection()) {
      // get last injection time
      auto difference = m_EventLevelTriggerTimeInfo->getTimeSinceLastInjection();
      float diff2 = difference / 127.; //  127MHz clock ticks to us, inexact rounding
      bool isher = m_EventLevelTriggerTimeInfo->isHER();
      for (auto& pkt : *m_storeDAQEvtStats) {
        for (auto& dhc : pkt) {
          int value = dhc.getDHCID() * 4 + dhc.getGatedFlag() * 2 + dhc.getGatedHER();
          if (isher) {
            hGateAfterInjHER->Fill(diff2, value);
          } else {
            hGateAfterInjLER->Fill(diff2, value);
          }
        }
      }
    }
  }
}
