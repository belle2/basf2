/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "svd/modules/svdDQM/SVDUnpackerDQMModule.h"
#include <svd/online/SVDOnlineToOfflineMap.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <svd/dataobjects/SVDDAQDiagnostic.h>

#include <TDirectory.h>
#include <TLine.h>
#include <TStyle.h>

#include <algorithm>
#include <string>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDUnpackerDQM)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

std::string SVDUnpackerDQMModule::m_xmlFileName = std::string("SVDChannelMapping.xml");

SVDUnpackerDQMModule::SVDUnpackerDQMModule() : HistoModule(), m_mapping(m_xmlFileName), m_FADCs(nullptr)
{
  //Set module properties
  setDescription("DQM Histogram of the SVD Raw Data format.");

  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed.",
           std::string("SVDUnpacker"));
  addParam("DAQDiagnostics", m_SVDDAQDiagnosticsName, "Name of DAQDiagnostics Store Array.", std::string(""));
  addParam("EventInfo", m_SVDEventInfoName, "Name of SVDEventInfo object.", std::string(""));

  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
}


SVDUnpackerDQMModule::~SVDUnpackerDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void SVDUnpackerDQMModule::defineHisto()
{

  // Create a separate histogram directories and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());// do not use return value with ->cd(), its ZERO if dir already exists
    oldDir->cd(m_histogramDirectoryName.c_str());
  }

  const unsigned short Bins_FTBFlags = 5;
  const unsigned short Bins_FTBError = 4;
  const unsigned short Bins_APVError = 4;
  const unsigned short Bins_APVMatch = 1;
  const unsigned short Bins_FADCMatch = 1;
  const unsigned short Bins_UpsetAPV = 1;
  const unsigned short Bins_BadMapping = 1;
  const unsigned short Bins_BadHeader = 1;
  const unsigned short Bins_MissedTrailer = 1;
  const unsigned short Bins_MissedHeader = 1;


  const unsigned short nBits = Bins_FTBFlags + Bins_FTBError + Bins_APVError + Bins_APVMatch + Bins_FADCMatch + Bins_UpsetAPV +
                               Bins_BadMapping + Bins_BadHeader + Bins_MissedTrailer + Bins_MissedHeader;

  m_DQMUnpackerHisto = new TH2F("DQMUnpackerHisto", "SVD Data Format Monitor", nBits, 1, nBits + 1, 52, 1, 53);
  m_DQMEventFractionHisto = new TH1F("DQMEventFractionHisto", "SVD Error Fraction Event Counter", 2, 0, 2);
  m_DQMnSamplesHisto = new TH2F("DQMnSamplesHisto", "nAPVsamples VS DAQMode", 3, 1, 4, 2, 1, 3);
  m_DQMnSamplesHisto2 = new TH2F("DQMnSamplesHisto2", "nAPVsamples VS DAQMode", 2, 1, 3, 2, 1, 3);
  m_DQMtrgQuality = new TH2F("DQMtrgQuality", "nAPVsamples VS trgQuality", 4, 1, 5, 2, 1, 3);

  m_DQMUnpackerHisto->GetYaxis()->SetTitle("FADC board");
  m_DQMUnpackerHisto->GetYaxis()->SetTitleOffset(1.2);

  m_DQMEventFractionHisto->GetYaxis()->SetTitle("# of Events");
  m_DQMEventFractionHisto->GetYaxis()->SetTitleOffset(1.5);
  m_DQMEventFractionHisto->SetMinimum(0);

  m_DQMnSamplesHisto->GetXaxis()->SetTitle("DAQ Mode");
  m_DQMnSamplesHisto->GetYaxis()->SetTitle("number of APV samples");

  m_DQMnSamplesHisto2->GetXaxis()->SetTitle("DAQ Mode");
  m_DQMnSamplesHisto2->GetYaxis()->SetTitle("number of APV samples");

  m_DQMtrgQuality->GetXaxis()->SetTitle("TRG Quality");
  m_DQMtrgQuality->GetYaxis()->SetTitle("number of APV samples");

  TString Xlabels[nBits] = {"EvTooLong", "TimeOut", "doubleHead", "badEvt", "errCRC", "badFADC", "badTTD", "badFTB", "badALL", "errAPV", "errDET", "errFrame", "errFIFO", "APVmatch", "FADCmatch", "upsetAPV", "EVTmatch", "missHead", "missTrail", "badMapping"};

  TString Ysamples[2] = {"3", "6"};
  TString Xsamples[3] = {"3 samples", "6 samples", "3/6 mixed"};

  TString Xquality[4] = {"coarse", "fine", "super fine", "no TRGSummary"};

  //preparing X axis of the DQMUnpacker histograms
  for (unsigned short i = 0; i < nBits; i++) m_DQMUnpackerHisto->GetXaxis()->SetBinLabel(i + 1, Xlabels[i].Data());

  //preparing X and Y axis of the DQMnSamples histograms
  for (unsigned short i = 0; i < 3; i++) m_DQMnSamplesHisto->GetXaxis()->SetBinLabel(i + 1, Xsamples[i].Data());
  for (unsigned short i = 0; i < 2; i++) m_DQMnSamplesHisto2->GetXaxis()->SetBinLabel(i + 1, Xsamples[i].Data());

  for (unsigned short i = 0; i < 2; i++) m_DQMnSamplesHisto->GetYaxis()->SetBinLabel(i + 1, Ysamples[i].Data());
  for (unsigned short i = 0; i < 2; i++) m_DQMnSamplesHisto2->GetYaxis()->SetBinLabel(i + 1, Ysamples[i].Data());

  //preparing X and Y axis of the DQMtrgQuality histograms
  for (unsigned short i = 0; i < 4; i++) m_DQMtrgQuality->GetXaxis()->SetBinLabel(i + 1, Xquality[i].Data());
  for (unsigned short i = 0; i < 2; i++) m_DQMtrgQuality->GetYaxis()->SetBinLabel(i + 1, Ysamples[i].Data());

  m_DQMEventFractionHisto->GetXaxis()->SetBinLabel(1, "OK");
  m_DQMEventFractionHisto->GetXaxis()->SetBinLabel(2, "Error(s)");


  oldDir->cd();
}

void SVDUnpackerDQMModule::initialize()
{
  m_eventMetaData.isRequired();
  m_svdDAQDiagnostics.isOptional(m_SVDDAQDiagnosticsName);
  m_svdEventInfo.isOptional(m_SVDEventInfoName);
  m_objTrgSummary.isOptional();

  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM
}


void SVDUnpackerDQMModule::beginRun()
{

  StoreObjPtr<EventMetaData> evtMetaData;
  m_expNumber = evtMetaData->getExperiment();
  m_runNumber = evtMetaData->getRun();
  m_errorFraction = 0;

  TString histoTitle = TString::Format("SVD Data Format Monitor, Exp %d Run %d", m_expNumber, m_runNumber);

  if (m_DQMUnpackerHisto != nullptr) {
    m_DQMUnpackerHisto->Reset();
    m_DQMUnpackerHisto->SetTitle(histoTitle.Data());
  }

  if (m_DQMEventFractionHisto != nullptr) {
    m_DQMEventFractionHisto->Reset();
  }

  if (m_DQMnSamplesHisto != nullptr) {
    m_DQMnSamplesHisto->Reset();
  }

  if (m_DQMnSamplesHisto2 != nullptr) {
    m_DQMnSamplesHisto2->Reset();
  }

  if (m_DQMtrgQuality != nullptr) {
    m_DQMtrgQuality->Reset();
  }

  m_shutUpNoData = false;

  if (m_mapping.hasChanged()) { m_map = std::make_unique<SVDOnlineToOfflineMap>(m_mapping->getFileName()); }

  m_changeFADCaxis = false;

  //getting fadc numbers from the mapping
  m_FADCs = &(m_map->FADCnumbers);

  //copy FADC numbers to vector and sort them
  m_vec_fadc.insert(m_vec_fadc.end(), m_FADCs->begin(), m_FADCs->end());
  std::sort(m_vec_fadc.begin(), m_vec_fadc.end());

  unsigned short ifadc = 0;
  for (const auto& fadc : m_vec_fadc) {
    m_fadc_map.insert(make_pair(fadc, ++ifadc));
    m_DQMUnpackerHisto->GetYaxis()->SetBinLabel(ifadc, to_string(fadc).c_str());
  }

  m_nEvents = 0;
  m_nBadEvents = 0;
}


void SVDUnpackerDQMModule::event()
{
  if (!m_svdDAQDiagnostics.isValid()) {
    if (!m_shutUpNoData)
      B2WARNING("There are no SVDDAQDiagnostic objects saved by the Unpacker! SVD Data Format Monitoring disabled!");
    m_shutUpNoData = true;
    return;
  }

  if (!m_svdEventInfo.isValid()) {
    if (!m_shutUpNoData)
      B2WARNING("There is no SVDEventInfo object saved by the Unpacker! SVD Data Format Monitoring disabled!");
    m_shutUpNoData = true;
    return;
  }


  m_badEvent = 0;
  m_nEvents++;

  // filling nSamplesHisto
  int daqMode = m_svdEventInfo->getModeByte().getDAQMode();
  int nSamples = m_svdEventInfo->getNSamples();

  m_DQMnSamplesHisto->Fill(daqMode, nSamples / 3);
  if (daqMode < 3) m_DQMnSamplesHisto2->Fill(daqMode, nSamples / 3);


  //filling TRGqualityHisto
  if (m_objTrgSummary.isValid()) {
    int trgQuality = m_objTrgSummary->getTimQuality();
    m_DQMtrgQuality->Fill(trgQuality, nSamples / 3);
  } else m_DQMtrgQuality->Fill(4, nSamples / 3);


  //filling m_DQMUnpackerHisto
  unsigned int nDiagnostics = m_svdDAQDiagnostics.getEntries();

  unsigned short bin_no = 0;
  gStyle->SetOptStat(0);

  for (unsigned short i = 0; i < nDiagnostics; i++) {

    m_ftbFlags = m_svdDAQDiagnostics[i]->getFTBFlags();
    m_ftbError = m_svdDAQDiagnostics[i]->getFTBError();
    m_apvError = m_svdDAQDiagnostics[i]->getAPVError();
    m_apvMatch = m_svdDAQDiagnostics[i]->getAPVMatch();
    m_fadcMatch = m_svdDAQDiagnostics[i]->getFADCMatch();
    m_upsetAPV = m_svdDAQDiagnostics[i]->getUpsetAPV();
    m_badMapping = m_svdDAQDiagnostics[i]->getBadMapping();
    m_badHeader = m_svdDAQDiagnostics[i]->getBadHeader();
    m_badTrailer = m_svdDAQDiagnostics[i]->getBadTrailer();
    m_missedHeader = m_svdDAQDiagnostics[i]->getMissedHeader();
    m_missedTrailer = m_svdDAQDiagnostics[i]->getMissedTrailer();

    m_fadcNo = m_svdDAQDiagnostics[i]->getFADCNumber();
    //apvNo = m_svdDAQDiagnostics[i]->getAPVNumber();

    // insert FADCnumber into the map (if not already there) and assign the next bin to it.
    if (m_changeFADCaxis) {
      if (m_fadc_map.find(m_fadcNo) == m_fadc_map.end())   m_fadc_map.insert(make_pair(m_fadcNo, ++bin_no));
    }

    if (m_ftbFlags != 0 or m_ftbError != 240 or m_apvError != 0 or !m_apvMatch or !m_fadcMatch or m_upsetAPV or m_badMapping
        or m_badHeader
        or m_badTrailer or m_missedHeader or m_missedTrailer) {

      m_badEvent = 1;

      auto ybin = m_fadc_map.find(m_fadcNo);

      if (m_badMapping)  {
        if (!m_changeFADCaxis) {
          m_changeFADCaxis = true;
          m_fadc_map.clear();
          break;
        } else {
          m_DQMUnpackerHisto->Fill(20, ybin->second);
        }
      }

      if (m_badHeader) m_DQMUnpackerHisto->Fill(17, ybin->second);
      if (m_missedHeader) m_DQMUnpackerHisto->Fill(18, ybin->second);
      if (m_badTrailer or m_missedTrailer) m_DQMUnpackerHisto->Fill(19, ybin->second);

      if (m_ftbFlags != 0) {
        if (m_ftbFlags & 16) m_DQMUnpackerHisto->Fill(5, ybin->second);
        if (m_ftbFlags & 8) m_DQMUnpackerHisto->Fill(4, ybin->second);
        if (m_ftbFlags & 4) m_DQMUnpackerHisto->Fill(3, ybin->second);
        if (m_ftbFlags & 2) m_DQMUnpackerHisto->Fill(2, ybin->second);
        if (m_ftbFlags & 1) m_DQMUnpackerHisto->Fill(1, ybin->second);
      }
      if (m_ftbError != 240) {
        switch (m_ftbError - 240) {
          case 3:
            m_DQMUnpackerHisto->Fill(6, ybin->second); //badFADC
            break;
          case 5:
            m_DQMUnpackerHisto->Fill(7, ybin->second); //badTTD
            break;
          case 6:
            m_DQMUnpackerHisto->Fill(8, ybin->second); //badFTB
            break;
          case 7:
            m_DQMUnpackerHisto->Fill(9, ybin->second);//badALL;
            break;
        }
      }

      if (m_apvError != 0) {
        if (m_apvError & 1) m_DQMUnpackerHisto->Fill(10, ybin->second);
        if (m_apvError & 2) m_DQMUnpackerHisto->Fill(11, ybin->second);
        if (m_apvError & 4) m_DQMUnpackerHisto->Fill(12, ybin->second);
        if (m_apvError & 8) m_DQMUnpackerHisto->Fill(13, ybin->second);
      }

      if (!m_apvMatch) m_DQMUnpackerHisto->Fill(14, ybin->second);
      if (!m_fadcMatch) m_DQMUnpackerHisto->Fill(15, ybin->second);
      if (m_upsetAPV) m_DQMUnpackerHisto->Fill(16, ybin->second);

    }
  } //end Diagnostics loop

  if (m_changeFADCaxis) {
    for (auto& iFADC : m_fadc_map)  m_DQMUnpackerHisto->GetYaxis()->SetBinLabel(iFADC.second, to_string(iFADC.first).c_str());
  }
  if (m_badEvent) m_nBadEvents++;
  m_errorFraction = 100 * float(m_nBadEvents) / float(m_nEvents);

  if (m_DQMEventFractionHisto != nullptr) {
    TString histoFractionTitle = TString::Format("SVD bad events fraction: %f %%,  Exp %d Run %d", m_errorFraction, m_expNumber,
                                                 m_runNumber);
    m_DQMEventFractionHisto->SetTitle(histoFractionTitle.Data());
  }


  m_DQMEventFractionHisto->Fill(m_badEvent);

} // end event function


void SVDUnpackerDQMModule::endRun()
{
  // Summary report on SVD DQM monitor
  if (m_nBadEvents) {
    B2WARNING("=================== SVD DQM Data Format Statistics: =============");
    B2WARNING(" We found " << m_nBadEvents << "/" << m_nEvents << " corrupted events, which is " << m_errorFraction <<
              "%");
    B2WARNING("=================================================================");
  }

}
