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
#include <vxd/geometry/GeoCache.h>
//#include <vxd/geometry/SensorInfoBase.h>

#include "TDirectory.h"
#include <TAxis.h>
#include <string>

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
  addParam("eachModule", m_eachModule, "creeate for each module", false);
}

void PXDInjectionDQMModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir(m_histogramDirectoryName.c_str());// do not rely on return value, might be ZERO
  oldDir->cd(m_histogramDirectoryName.c_str());

  hOccAfterInjLER  = new TH1F("PXDOccInjLER", "PXDOccInjLER/Time;;Count/Time", 1000, -500, 4500);
  hOccAfterInjHER  = new TH1F("PXDOccInjHER", "PXDOccInjHER/Time;;Count/Time", 1000, -500, 4500);

  if (m_eachModule) {
    std::vector<VxdID> sensors = m_vxdGeometry.getListOfSensors();
    for (VxdID& avxdid : sensors) {
      VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(avxdid);
      if (info.getType() != VXD::SensorInfoBase::PXD) continue;
      //Only interested in PXD sensors

      TString buff = (std::string)avxdid;
      TString bufful = buff;
      buff.ReplaceAll(".", "_");

      hOccModAfterInjLER[avxdid] = new TH1F("PXDOccInjLER_" + bufful, "PXDOccModInjLER " + buff + "/Time;;Count/Time", 1000, -500, 4500);
      hOccModAfterInjHER[avxdid] = new TH1F("PXDOccInjHER_" + bufful, "PXDOccModInjLER " + buff + "/Time;;Count/Time", 1000, -500, 4500);

    }
  }
  // cd back to root directory
  oldDir->cd();
}

void PXDInjectionDQMModule::initialize()
{
  REG_HISTOGRAM
//  m_storeDAQEvtStats.isRequired();
  m_rawTTD.isRequired();
  m_storeRawHits.isRequired(m_PXDRawHitsName);
}

void PXDInjectionDQMModule::beginRun()
{
  // Assume that everthing is non-yero ;-)
  hOccAfterInjLER->Reset();
  hOccAfterInjHER->Reset();
  for (auto& a : hOccModAfterInjLER) a.second->Reset();
  for (auto& a : hOccModAfterInjHER) a.second->Reset();
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
      // Should we use two histograms and normalize? Use maybe TEfficiency? Will this work with HistoModule?
      if (it.GetIsHER(0)) {
        hOccAfterInjHER->Fill(difference, all);
        for (auto& a : hOccModAfterInjHER) {
          a.second->Fill(difference, freq[a.first]);
        }
      } else {
        hOccAfterInjLER->Fill(difference, all);
        for (auto& a : hOccModAfterInjLER) {
          a.second->Fill(difference, freq[a.first]);
        }
      }
    }

    break;
  }
}
