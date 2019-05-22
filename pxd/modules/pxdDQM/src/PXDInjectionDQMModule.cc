/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdDQM/PXDInjectionDQMModule.h>
#include "TDirectory.h"

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;
using namespace Belle2::VXD;
using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDInjectionDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDInjectionDQMModule::PXDInjectionDQMModule() : HistoModule() , m_vxdGeometry(VXD::GeoCache::getInstance())
{
  //Set module properties
  setDescription("Monitor Occupancy after Injection");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("PXDINJ"));
  addParam("PXDRawHitsName", m_PXDRawHitsName, "Name of PXD raw hits", std::string(""));
  addParam("eachModule", m_eachModule, "create for each module", false);
  addParam("offlineStudy", m_offlineStudy, "use finest binning and larger range", false);
}

void PXDInjectionDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir(m_histogramDirectoryName.c_str());// do not rely on return value, might be ZERO
  oldDir->cd(m_histogramDirectoryName.c_str());//changing to the right directory

  if (m_offlineStudy) {
    hOccAfterInjLER  = new TH1F("PXDOccInjLER", "PXDOccInjLER/Time;Time in #mus;Count/Time (0.5 #mus bins)", 100000, 0, 50000);
    hOccAfterInjHER  = new TH1F("PXDOccInjHER", "PXDOccInjHER/Time;Time in #mus;Count/Time (0.5 #mus bins)", 100000, 0, 50000);
    hEOccAfterInjLER  = new TH1F("PXDEOccInjLER", "PXDEOccInjLER/Time;Time in #mus;Triggers/Time (0.5 #mus bins)", 100000, 0, 50000);
    hEOccAfterInjHER  = new TH1F("PXDEOccInjHER", "PXDEOccInjHER/Time;Time in #mus;Triggers/Time (0.5 #mus bins)", 100000, 0, 50000);
    hMaxOccAfterInjLER  = new TH1F("PXDMaxOccInjLER", "PXDMaxOccInjLER/Time;Time in #mus;Triggers/Time (0.5 #mus bins)", 100000, 0,
                                   50000);
    hMaxOccAfterInjHER  = new TH1F("PXDMaxOccInjHER", "PXDMaxOccInjHER/Time;Time in #mus;Triggers/Time (0.5 #mus bins)", 100000, 0,
                                   50000);
  } else {
    hOccAfterInjLER  = new TH1F("PXDOccInjLER", "PXDOccInjLER/Time;Time in #mus;Count/Time (5 #mus bins)", 4000, 0, 20000);
    hOccAfterInjHER  = new TH1F("PXDOccInjHER", "PXDOccInjHER/Time;Time in #mus;Count/Time (5 #mus bins)", 4000, 0, 20000);
    hEOccAfterInjLER  = new TH1F("PXDEOccInjLER", "PXDEOccInjLER/Time;Time in #mus;Triggers/Time (5 #mus bins)", 4000, 0, 20000);
    hEOccAfterInjHER  = new TH1F("PXDEOccInjHER", "PXDEOccInjHER/Time;Time in #mus;Triggers/Time (5 #mus bins)", 4000, 0, 20000);
    hMaxOccAfterInjLER  = new TH1F("PXDMaxOccInjLER", "PXDMaxOccInjLER/Time;Time in #mus;Triggers/Time (5 #mus bins)", 4000, 0, 20000);
    hMaxOccAfterInjHER  = new TH1F("PXDMaxOccInjHER", "PXDMaxOccInjHER/Time;Time in #mus;Triggers/Time (5 #mus bins)", 4000, 0, 20000);
  }

  if (m_eachModule) {
    std::vector<VxdID> sensors = m_vxdGeometry.getListOfSensors();
    for (VxdID& avxdid : sensors) {
      VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(avxdid);
      if (info.getType() != VXD::SensorInfoBase::PXD) continue;
      // Only interested in PXD sensors

      TString buff = (std::string)avxdid;
      TString bufful = buff;
      bufful.ReplaceAll(".", "_");

      if (m_offlineStudy) {
        hOccModAfterInjLER[avxdid] = new TH1F("PXDOccInjLER_" + bufful,
                                              "PXDOccModInjLER " + buff + "/Time;Time in #mus;Count/Time (0.5 #mus bins)", 100000, 0, 50000);
        hOccModAfterInjHER[avxdid] = new TH1F("PXDOccInjHER_" + bufful,
                                              "PXDOccModInjHER " + buff + "/Time;Time in #mus;Count/Time (0.5 #mus bins)", 100000, 0, 50000);
        hEOccModAfterInjLER[avxdid] = new TH1F("PXDEOccInjLER_" + bufful,
                                               "PXDEOccModInjLER " + buff + "/Time;Time in #mus;Triggers/Time (0.5 #mus bins)", 100000,
                                               0, 50000);
        hEOccModAfterInjHER[avxdid] = new TH1F("PXDEOccInjHER_" + bufful,
                                               "PXDEOccModInjHER " + buff + "/Time;Time in #mus;Triggers/Time (0.5 #mus bins)", 100000,
                                               0, 50000);
        hMaxOccModAfterInjLER[avxdid] = new TH1F("PXDMaxOccInjLER_" + bufful,
                                                 "PXDMaxOccModInjLER " + buff + "/Time;Time in #mus;Count/Time (0.5 #mus bins)", 100000, 0, 50000);
        hMaxOccModAfterInjHER[avxdid] = new TH1F("PXDMaxOccInjHER_" + bufful,
                                                 "PXDMaxOccModInjHER " + buff + "/Time;Time in #mus;Count/Time (0.5 #mus bins)", 100000, 0, 50000);
      } else {
        hOccModAfterInjLER[avxdid] = new TH1F("PXDOccInjLER_" + bufful,
                                              "PXDOccModInjLER " + buff + "/Time;Time in #mus;Count/Time (5 #mus bins)", 4000, 0, 20000);
        hOccModAfterInjHER[avxdid] = new TH1F("PXDOccInjHER_" + bufful,
                                              "PXDOccModInjHER " + buff + "/Time;Time in #mus;Count/Time (5 #mus bins)", 4000, 0, 20000);
        hEOccModAfterInjLER[avxdid] = new TH1F("PXDEOccInjLER_" + bufful,
                                               "PXDEOccModInjLER " + buff + "/Time;Time in #mus;Triggers/Time (5 #mus bins)", 4000, 0, 20000);
        hEOccModAfterInjHER[avxdid] = new TH1F("PXDEOccInjHER_" + bufful,
                                               "PXDEOccModInjHER " + buff + "/Time;Time in #mus;Triggers/Time (5 #mus bins)", 4000, 0, 20000);
        hMaxOccModAfterInjLER[avxdid] = new TH1F("PXDMaxOccInjLER_" + bufful,
                                                 "PXDMaxOccModInjLER " + buff + "/Time;Time in #mus;Count/Time (5 #mus bins)", 4000, 0, 20000);
        hMaxOccModAfterInjHER[avxdid] = new TH1F("PXDMaxOccInjHER_" + bufful,
                                                 "PXDMaxOccModInjHER " + buff + "/Time;Time in #mus;Count/Time (5 #mus bins)", 4000, 0, 20000);
      }
    }
  }
  // cd back to root directory
  oldDir->cd();
}

void PXDInjectionDQMModule::initialize()
{
  REG_HISTOGRAM
  m_rawTTD.isOptional(); /// TODO better use isRequired(), but RawFTSW is not in sim, thus tests are failing
  m_storeRawHits.isRequired(m_PXDRawHitsName);
}

void PXDInjectionDQMModule::beginRun()
{
  // Assume that everthing is non-yero ;-)
  hOccAfterInjLER->Reset();
  hOccAfterInjHER->Reset();
  hEOccAfterInjLER->Reset();
  hEOccAfterInjHER->Reset();
  hMaxOccAfterInjLER->Reset();
  hMaxOccAfterInjHER->Reset();
  for (auto& a : hOccModAfterInjLER) a.second->Reset();
  for (auto& a : hOccModAfterInjHER) a.second->Reset();
  for (auto& a : hEOccModAfterInjLER) a.second->Reset();
  for (auto& a : hEOccModAfterInjHER) a.second->Reset();
  for (auto& a : hMaxOccModAfterInjLER) a.second->Reset();
  for (auto& a : hMaxOccModAfterInjHER) a.second->Reset();
}

void PXDInjectionDQMModule::event()
{

  for (auto& it : m_rawTTD) {
    B2DEBUG(29, "TTD FTSW : " << hex << it.GetTTUtime(0) << " " << it.GetTTCtime(0) << " EvtNr " << it.GetEveNo(0)  << " Type " <<
            (it.GetTTCtimeTRGType(0) & 0xF) << " TimeSincePrev " << it.GetTimeSincePrevTrigger(0) << " TimeSinceInj " <<
            it.GetTimeSinceLastInjection(0) << " IsHER " << it.GetIsHER(0) << " Bunch " << it.GetBunchNumber(0));

    // get last injection time
    auto difference = it.GetTimeSinceLastInjection(0);
    // check time overflow, too long ago
    if (difference != 0x7FFFFFFF) {
      // count raw pixel hits per module, only if necessary
      unsigned int all = 0;
      std::map <VxdID, int> freq;// count the number of RawHits per sensor
      for (auto& p : m_storeRawHits) {
        freq[p.getSensorID()]++;
        all++;
      }
      float diff2 = difference / 127.; //  127MHz clock ticks to us, inexact rounding
      if (it.GetIsHER(0)) {
        hOccAfterInjHER->Fill(diff2, all);
        hEOccAfterInjHER->Fill(diff2);
        auto bin = hMaxOccAfterInjHER->FindBin(diff2);
        auto value = hMaxOccAfterInjHER->GetBinContent(bin);
        if (all > value) hMaxOccAfterInjHER->SetBinContent(bin, all);
        for (auto& a : hOccModAfterInjHER) {
          if (a.second) a.second->Fill(diff2, freq[a.first]);
        }
        for (auto& a : hEOccModAfterInjHER) {
          if (a.second) a.second->Fill(diff2);
        }
        for (auto& a : hMaxOccModAfterInjHER) {
          if (a.second) {
            bin = a.second->FindBin(diff2);
            value = a.second->GetBinContent(bin);
            if (freq[a.first] > value) a.second->SetBinContent(bin, freq[a.first]);
          }
        }
      } else {
        hOccAfterInjLER->Fill(diff2, all);
        hEOccAfterInjLER->Fill(diff2);
        auto bin = hMaxOccAfterInjLER->FindBin(diff2);
        auto value = hMaxOccAfterInjLER->GetBinContent(bin);
        if (all > value) hMaxOccAfterInjLER->SetBinContent(bin, all);
        for (auto& a : hOccModAfterInjLER) {
          if (a.second) a.second->Fill(diff2, freq[a.first]);
        }
        for (auto& a : hEOccModAfterInjLER) {
          if (a.second) a.second->Fill(diff2);
        }
        for (auto& a : hMaxOccModAfterInjLER) {
          if (a.second) {
            bin = a.second->FindBin(diff2);
            value = a.second->GetBinContent(bin);
            if (freq[a.first] > value) a.second->SetBinContent(bin, freq[a.first]);
          }
        }
      }
    }

    break;
  }
}
