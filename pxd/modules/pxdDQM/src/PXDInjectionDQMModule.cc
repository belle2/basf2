/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/modules/pxdDQM/PXDInjectionDQMModule.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <mdst/dataobjects/EventLevelTriggerTimeInfo.h>

#include "TDirectory.h"

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;
using namespace Belle2::VXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDInjectionDQM);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDInjectionDQMModule::PXDInjectionDQMModule() : HistoModule(), m_vxdGeometry(VXD::GeoCache::getInstance())
{
  //Set module properties
  setDescription("Monitor Occupancy after Injection");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("PXDINJ"));
  addParam("PXDRawHitsName", m_PXDRawHitsName, "Name of PXD raw hits", std::string(""));
  addParam("PXDClusterName", m_PXDClustersName, "Name of PXD clusters", std::string(""));
  addParam("eachModule", m_eachModule, "create for each module", false);
  addParam("offlineStudy", m_offlineStudy, "use finest binning and larger range", false);
  addParam("useClusters", m_useClusters, "use cluster instead of raw hits", false);
  addParam("createMaxHist", m_createMaxHist, "create histo with max occupancy (not mp save!!!)", false);
  addParam("createGateHist", m_createGateHist, "create 2d histo with gate against occupancy", false);

}

void PXDInjectionDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());// do not rely on return value, might be ZERO
    oldDir->cd(m_histogramDirectoryName.c_str());//changing to the right directory
  }

  if (m_offlineStudy) {
    hOccAfterInjLER  = new TH1F("PXDOccInjLER", "PXDOccInjLER/Time;Time in #mus;Count/Time (0.5 #mus bins)", 100000, 0, 50000);
    hOccAfterInjHER  = new TH1F("PXDOccInjHER", "PXDOccInjHER/Time;Time in #mus;Count/Time (0.5 #mus bins)", 100000, 0, 50000);
    hEOccAfterInjLER  = new TH1I("PXDEOccInjLER", "PXDEOccInjLER/Time;Time in #mus;Triggers/Time (0.5 #mus bins)", 100000, 0, 50000);
    hEOccAfterInjHER  = new TH1I("PXDEOccInjHER", "PXDEOccInjHER/Time;Time in #mus;Triggers/Time (0.5 #mus bins)", 100000, 0, 50000);
    if (m_createMaxHist) {
      hMaxOccAfterInjLER  = new TH1F("PXDMaxOccInjLER", "PXDMaxOccInjLER/Time;Time in #mus;Triggers/Time (0.5 #mus bins)", 100000, 0,
                                     50000);
      hMaxOccAfterInjHER  = new TH1F("PXDMaxOccInjHER", "PXDMaxOccInjHER/Time;Time in #mus;Triggers/Time (0.5 #mus bins)", 100000, 0,
                                     50000);
    }
    if (m_createGateHist) {
      hOccAfterInjLERGate  = new TH2F("PXDOccInjLERGate", "PXDOccInjLERGate;Time;Gate", 1000, 0, 10000, 192, 0, 192);
      hOccAfterInjHERGate  = new TH2F("PXDOccInjHERGate", "PXDOccInjHERGate;Time;Gate", 1000, 0, 10000, 192, 0, 192);
    }
  } else {
    hOccAfterInjLER  = new TH1F("PXDOccInjLER", "PXDOccInjLER/Time;Time in #mus;Count/Time (5 #mus bins)", 4000, 0, 20000);
    hOccAfterInjHER  = new TH1F("PXDOccInjHER", "PXDOccInjHER/Time;Time in #mus;Count/Time (5 #mus bins)", 4000, 0, 20000);
    hEOccAfterInjLER  = new TH1I("PXDEOccInjLER", "PXDEOccInjLER/Time;Time in #mus;Triggers/Time (5 #mus bins)", 4000, 0, 20000);
    hEOccAfterInjHER  = new TH1I("PXDEOccInjHER", "PXDEOccInjHER/Time;Time in #mus;Triggers/Time (5 #mus bins)", 4000, 0, 20000);
    if (m_createMaxHist) {
      hMaxOccAfterInjLER  = new TH1F("PXDMaxOccInjLER", "PXDMaxOccInjLER/Time;Time in #mus;Triggers/Time (5 #mus bins)", 4000, 0, 20000);
      hMaxOccAfterInjHER  = new TH1F("PXDMaxOccInjHER", "PXDMaxOccInjHER/Time;Time in #mus;Triggers/Time (5 #mus bins)", 4000, 0, 20000);
    }
    if (m_createGateHist) {
      hOccAfterInjLERGate  = new TH2F("PXDOccInjLERGate", "PXDOccInjLERGate;Time;Gate", 1000, 0, 10000, 192, 0, 192);
      hOccAfterInjHERGate  = new TH2F("PXDOccInjHERGate", "PXDOccInjHERGate;Time;Gate", 1000, 0, 10000, 192, 0, 192);
    }
  }

  if (m_eachModule) {
    std::vector<VxdID> sensors = m_vxdGeometry.getListOfSensors();
    for (VxdID& avxdid : sensors) {
      VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(avxdid);
      if (info.getType() != VXD::SensorInfoBase::PXD) continue;
      // Only interested in PXD sensors

      TString buff = (std::string)avxdid;
      TString buffus = buff;
      buffus.ReplaceAll(".", "_");

      /// The number of bins as well as the range MUST be the same here as in the histograms above and in the DQM analysis code!!!
      /// Reason: Number of entries histogram is shared by all histograms!
      if (m_offlineStudy) {
        hOccModAfterInjLER[avxdid] = new TH1F("PXDOccInjLER_" + buffus,
                                              "PXDOccModInjLER " + buff + "/Time;Time in #mus;Count/Time (0.5 #mus bins)", 100000, 0, 50000);
        hOccModAfterInjHER[avxdid] = new TH1F("PXDOccInjHER_" + buffus,
                                              "PXDOccModInjHER " + buff + "/Time;Time in #mus;Count/Time (0.5 #mus bins)", 100000, 0, 50000);
        if (m_createMaxHist) {
          hMaxOccModAfterInjLER[avxdid] = new TH1F("PXDMaxOccInjLER_" + buffus,
                                                   "PXDMaxOccModInjLER " + buff + "/Time;Time in #mus;Count/Time (0.5 #mus bins)", 100000, 0, 50000);
          hMaxOccModAfterInjHER[avxdid] = new TH1F("PXDMaxOccInjHER_" + buffus,
                                                   "PXDMaxOccModInjHER " + buff + "/Time;Time in #mus;Count/Time (0.5 #mus bins)", 100000, 0, 50000);
        }
        if (m_createGateHist) {
          hOccModAfterInjLERGate[avxdid]  = new TH2F("PXDOccInjLERGate_" + buffus, "PXDOccInjLERGate " + buff + ";Time;Gate", 1000, 0, 10000,
                                                     192, 0,
                                                     192);
          hOccModAfterInjHERGate[avxdid]  = new TH2F("PXDOccInjHERGate_" + buffus, "PXDOccInjHERGate " + buff + ";Time;Gate", 1000, 0, 10000,
                                                     192, 0,
                                                     192);
        }
      } else {
        hOccModAfterInjLER[avxdid] = new TH1F("PXDOccInjLER_" + buffus,
                                              "PXDOccModInjLER " + buff + "/Time;Time in #mus;Count/Time (5 #mus bins)", 4000, 0, 20000);
        hOccModAfterInjHER[avxdid] = new TH1F("PXDOccInjHER_" + buffus,
                                              "PXDOccModInjHER " + buff + "/Time;Time in #mus;Count/Time (5 #mus bins)", 4000, 0, 20000);
        if (m_createMaxHist) {
          hMaxOccModAfterInjLER[avxdid] = new TH1F("PXDMaxOccInjLER_" + buffus,
                                                   "PXDMaxOccModInjLER " + buff + "/Time;Time in #mus;Count/Time (5 #mus bins)", 4000, 0, 20000);
          hMaxOccModAfterInjHER[avxdid] = new TH1F("PXDMaxOccInjHER_" + buffus,
                                                   "PXDMaxOccModInjHER " + buff + "/Time;Time in #mus;Count/Time (5 #mus bins)", 4000, 0, 20000);
        }
        if (m_createGateHist) {
          hOccModAfterInjLERGate[avxdid]  = new TH2F("PXDOccInjLERGate_" + buffus, "PXDOccInjLERGate " + buff + ";Time;Gate", 1000, 0, 10000,
                                                     192, 0,
                                                     192);
          hOccModAfterInjHERGate[avxdid]  = new TH2F("PXDOccInjHERGate_" + buffus, "PXDOccInjHERGate " + buff + ";Time;Gate", 1000, 0, 10000,
                                                     192, 0,
                                                     192);
        }
      }
    }
  }

//   hTrigAfterInjLER = new TH2F("TrigAfterInjLER",
//                               "Triggers for LER veto tuning;Time since last injection in #mus;Time within beam cycle in #mus", 500, 0, 30000, 100, 0,
//                               5120 / 508.);
//   hTrigAfterInjHER = new TH2F("TrigAfterInjHER",
//                               "Triggers for HER veto tuning;Time since last injection in #mus;Time within beam cycle in #mus", 500, 0, 30000, 100, 0,
//                               5120 / 508.);

  hTriggersAfterTrigger = new TH1I("PXDTriggersAfterLast",
                                   "PXD Trigger after Last Trigger;Time diff in #mus;Count/Time (0.5 #mus bins)", 100000, 0, 50000);
  hTriggersPerBunch = new TH1I("PXDTriggerBunch", "PXD Trigger per Bunch;Bunch/4;Triggers", 1280, 0, 1280);


  // cd back to root directory
  oldDir->cd();
}

void PXDInjectionDQMModule::initialize()
{
  REG_HISTOGRAM
  m_EventLevelTriggerTimeInfo.isRequired();

  if (m_useClusters) {
    m_storeClusters.isRequired(m_PXDClustersName);
  } else {
    m_storeRawHits.isRequired(m_PXDRawHitsName);
  }
}

void PXDInjectionDQMModule::beginRun()
{
  // Do not assume that everything is non-zero, e.g. Max might be nullptr
  if (hOccAfterInjLER) hOccAfterInjLER->Reset();
  if (hOccAfterInjHER) hOccAfterInjHER->Reset();
  if (hEOccAfterInjLER) hEOccAfterInjLER->Reset();
  if (hEOccAfterInjHER) hEOccAfterInjHER->Reset();
  if (hMaxOccAfterInjLER) hMaxOccAfterInjLER->Reset();
  if (hMaxOccAfterInjHER) hMaxOccAfterInjHER->Reset();
  if (hOccAfterInjLERGate) hOccAfterInjLERGate->Reset();
  if (hOccAfterInjHERGate) hOccAfterInjHERGate->Reset();
  for (auto& a : hOccModAfterInjLER) if (a.second) a.second->Reset();
  for (auto& a : hOccModAfterInjHER) if (a.second) a.second->Reset();
  for (auto& a : hMaxOccModAfterInjLER) if (a.second) a.second->Reset();
  for (auto& a : hMaxOccModAfterInjHER) if (a.second) a.second->Reset();
  for (auto& a : hOccModAfterInjLERGate) if (a.second) a.second->Reset();
  for (auto& a : hOccModAfterInjHERGate) if (a.second) a.second->Reset();
//   hTrigAfterInjLER->Reset();
//   hTrigAfterInjHER->Reset();
  hTriggersAfterTrigger->Reset();
  hTriggersPerBunch->Reset();
}

void PXDInjectionDQMModule::event()
{
  // And check if the stored data is valid
  if (m_EventLevelTriggerTimeInfo.isValid() and m_EventLevelTriggerTimeInfo->isValid()) {
    // get last injection time
    hTriggersAfterTrigger->Fill(m_EventLevelTriggerTimeInfo->getTimeSincePrevTrigger() / 127.);
    // hTriggersAfterTrigger->Fill(m_EventLevelTriggerTimeInfo->getTimeSincePrevTrigger() / 64.);
    hTriggersPerBunch->Fill(m_EventLevelTriggerTimeInfo->getBunchNumber());

    // check time overflow, too long ago
    if (m_EventLevelTriggerTimeInfo->hasInjection()) {
      // count raw pixel hits or clusters per module, only if necessary
      unsigned int all = 0;
      std::map <VxdID, int> freq;// count the number of RawHits per sensor
      if (m_useClusters) {
        for (auto& p : m_storeClusters) {
          freq[p.getSensorID()]++;
          all++;
        }
      } else {
        for (auto& p : m_storeRawHits) {
          freq[p.getSensorID()]++;
          all++;
        }
      }
      float difference = m_EventLevelTriggerTimeInfo->getTimeSinceLastInjection() / 127.; //  127MHz clock ticks to us, inexact rounding
      if (m_EventLevelTriggerTimeInfo->isHER()) {
        hOccAfterInjHER->Fill(difference, all);
        hEOccAfterInjHER->Fill(difference);
        //         hTrigAfterInjHER->Fill(difference, difference - int(difference / (5120 / 508.)) * (5120 / 508.));
        if (m_createMaxHist) {
          auto bin = hMaxOccAfterInjHER->FindBin(difference);
          auto value = hMaxOccAfterInjHER->GetBinContent(bin);
          if (all > value) hMaxOccAfterInjHER->SetBinContent(bin, all);
        }
        for (auto& a : hOccModAfterInjHER) {
          if (a.second) a.second->Fill(difference, freq[a.first]);
        }
        if (m_createMaxHist) {
          for (auto& a : hMaxOccModAfterInjHER) {
            if (a.second) {
              auto bin = a.second->FindBin(difference);
              auto value = a.second->GetBinContent(bin);
              if (freq[a.first] > value) a.second->SetBinContent(bin, freq[a.first]);
            }
          }
        }
        if (hOccAfterInjHERGate) {
          if (m_useClusters) {
            // Cluster does not contain VCellID, need to change histogramm completely
            // -> doesn't work with clusters!
            //             for (auto& p : m_storeClusters) {
            //               hOccAfterInjHERGate->Fill(difference, p.getVCellID() / 4);
            //             }
          } else {
            for (auto& p : m_storeRawHits) {
              hOccAfterInjHERGate->Fill(difference, p.getRow() / 4);
              hOccModAfterInjHERGate[p.getSensorID()]->Fill(difference, p.getRow() / 4);
            }
          }
        }
      } else {
        hOccAfterInjLER->Fill(difference, all);
        hEOccAfterInjLER->Fill(difference);
        //         hTrigAfterInjLER->Fill(difference, difference - int(difference / (5120 / 508.)) * (5120 / 508.));
        if (m_createMaxHist) {
          auto bin = hMaxOccAfterInjLER->FindBin(difference);
          auto value = hMaxOccAfterInjLER->GetBinContent(bin);
          if (all > value) hMaxOccAfterInjLER->SetBinContent(bin, all);
        }
        for (auto& a : hOccModAfterInjLER) {
          if (a.second) a.second->Fill(difference, freq[a.first]);
        }
        if (m_createMaxHist) {
          for (auto& a : hMaxOccModAfterInjLER) {
            if (a.second) {
              auto bin = a.second->FindBin(difference);
              auto value = a.second->GetBinContent(bin);
              if (freq[a.first] > value) a.second->SetBinContent(bin, freq[a.first]);
            }
          }

        }
        if (hOccAfterInjLERGate) {
          if (m_useClusters) {
            // Cluster does not contain VCellID, need to change histogramm completely
            // -> doesn't work with clusters!
            //             for (auto& p : m_storeClusters) {
            //               hOccAfterInjLERGate->Fill(difference, p.getVCellID() / 4);
            //             }
          } else {
            for (auto& p : m_storeRawHits) {
              hOccAfterInjLERGate->Fill(difference, p.getRow() / 4);
              hOccModAfterInjLERGate[p.getSensorID()]->Fill(difference, p.getRow() / 4);
            }
          }
        }
      }
    }
  }
}
