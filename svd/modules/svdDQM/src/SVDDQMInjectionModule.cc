/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdDQM/SVDDQMInjectionModule.h>
#include <framework/dataobjects/EventMetaData.h>
#include "TDirectory.h"

using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;
using namespace Belle2::VXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDDQMInjection)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDDQMInjectionModule::SVDDQMInjectionModule() : HistoModule() , m_vxdGeometry(VXD::GeoCache::getInstance())
{
  //Set module properties
  setDescription("Monitor SVD Occupancy after Injection.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed.",
           std::string("SVDInjection"));
  addParam("ShaperDigits", m_SVDShaperDigitsName, "Name of SVD ShaperDigits to count occupancy - usually ZS5 strips.",
           std::string(""));
}

void SVDDQMInjectionModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());// do not rely on return value, might be ZERO
    oldDir->cd(m_histogramDirectoryName.c_str());//changing to the right directory
  }

  m_hOccAfterInjLER  = new TH1F("SVDOccInjLER", Form("SVDOccInjLER/Time;Time in #mus;Count/Time (5 #mus bins) ~ Exp%d Run%d",
                                                     m_expNumber, m_runNumber), 4000, 0, 20000);
  m_hOccAfterInjHER  = new TH1F("SVDOccInjHER", Form("SVDOccInjHER/Time;Time in #mus;Count/Time (5 #mus bins) ~ Exp%d Run%d",
                                                     m_expNumber, m_runNumber), 4000, 0, 20000);
  m_hTrgOccAfterInjLER  = new TH1F("SVDTrgOccInjLER",
                                   Form("SVDTrgOccInjLER/Time;Time in #mus;Triggers/Time (5 #mus bins) ~ Exp%d Run%d", m_expNumber, m_runNumber), 4000, 0,
                                   20000);
  m_hTrgOccAfterInjHER  = new TH1F("SVDTrgOccInjHER",
                                   Form("SVDTrgOccInjHER/Time;Time in #mus;Triggers/Time (5 #mus bins) ~ Exp%d Run%d", m_expNumber, m_runNumber), 4000, 0,
                                   20000);
  m_hMaxOccAfterInjLER  = new TH1F("SVDMaxOccInjLER",
                                   Form("SVDMaxOccInjLER/Time;Time in #mus;Triggers/Time (5 #mus bins) ~ Exp%d Run%d", m_expNumber, m_runNumber), 4000, 0,
                                   20000);
  m_hMaxOccAfterInjHER  = new TH1F("SVDMaxOccInjHER",
                                   Form("SVDMaxOccInjHER/Time;Time in #mus;Triggers/Time (5 #mus bins) ~ Exp%d Run%d", m_expNumber, m_runNumber), 4000, 0,
                                   20000);
  m_hBunchNumVSNStrips  = new TH2F("SVDBunchNumVSNStrips", Form("SVDBunchNumVSNStrips;Bunch No.;Number of fired strips ~ Exp%d Run%d",
                                   m_expNumber, m_runNumber), 1280, 0, 1280, 10,
                                   0,
                                   10000);

  // cd back to root directory
  oldDir->cd();
}

void SVDDQMInjectionModule::initialize()
{
  REG_HISTOGRAM

  m_rawTTD.isOptional();
  m_digits.isOptional(m_SVDShaperDigitsName);
}

void SVDDQMInjectionModule::beginRun()
{
  StoreObjPtr<EventMetaData> evtMetaData;
  m_expNumber = evtMetaData->getExperiment();
  m_runNumber = evtMetaData->getRun();

  // Assume that everthing is non-zero ;-)
  m_hOccAfterInjLER->Reset();
  m_hOccAfterInjHER->Reset();
  m_hTrgOccAfterInjLER->Reset();
  m_hTrgOccAfterInjHER->Reset();
  m_hMaxOccAfterInjLER->Reset();
  m_hMaxOccAfterInjHER->Reset();
  m_hBunchNumVSNStrips->Reset();
}

void SVDDQMInjectionModule::event()
{

  if (!m_rawTTD.isValid()) {
    B2WARNING("Missing RawFTSW, SVDDQMInjection is skipped.");
    return;
  }

  if (!m_digits.isValid()) {
    B2WARNING("Missing " << m_SVDShaperDigitsName << ", SVDDQMInjection is skipped.");
    return;
  }


  for (auto& it : m_rawTTD) {
    B2DEBUG(29, "TTD FTSW : " << hex << it.GetTTUtime(0) << " " << it.GetTTCtime(0) << " EvtNr " << it.GetEveNo(0)  << " Type " <<
            (it.GetTTCtimeTRGType(0) & 0xF) << " TimeSincePrev " << it.GetTimeSincePrevTrigger(0) << " TimeSinceInj " <<
            it.GetTimeSinceLastInjection(0) << " IsHER " << it.GetIsHER(0) << " Bunch " << it.GetBunchNumber(0));

    // get last injection time
    auto difference = it.GetTimeSinceLastInjection(0);
    // check time overflow, too long ago
    if (difference != 0x7FFFFFFF) {
      unsigned int allV = 0;
      unsigned int allU = 0;
      unsigned int nHitsL3v = 0;
      unsigned int nHitsL3u = 0;
      for (auto& p : m_digits) {
        if (p.isUStrip()) allU++;
        else allV++;

        if (p.getSensorID().getLayerNumber() != 3) continue;
        if (p.isUStrip())
          nHitsL3u++;
        else nHitsL3v++;
      }

      B2DEBUG(29, "L3 V =  " << nHitsL3v << ", L3 U =  " << nHitsL3u << ", all V =  " << allV << ", all U =  " << allU);

      //choose your counter
      unsigned int counter = nHitsL3u;

      float diff2 = difference / 127.; //  127MHz clock ticks to us, inexact rounding
      if (it.GetIsHER(0)) {
        m_hOccAfterInjHER->Fill(diff2, counter);
        m_hTrgOccAfterInjHER->Fill(diff2);
        auto bin = m_hMaxOccAfterInjHER->FindBin(diff2);
        auto value = m_hMaxOccAfterInjHER->GetBinContent(bin);
        if (counter > value) m_hMaxOccAfterInjHER->SetBinContent(bin, counter);
      } else {
        m_hOccAfterInjLER->Fill(diff2, counter);
        m_hTrgOccAfterInjLER->Fill(diff2);
        auto bin = m_hMaxOccAfterInjLER->FindBin(diff2);
        auto value = m_hMaxOccAfterInjLER->GetBinContent(bin);
        if (counter > value) m_hMaxOccAfterInjLER->SetBinContent(bin, counter);

      }
      m_hBunchNumVSNStrips->Fill(it.GetBunchNumber(0), allU + allV);
    }

    break;
  }
}
