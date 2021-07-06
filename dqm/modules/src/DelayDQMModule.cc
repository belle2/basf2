/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "dqm/modules/DelayDQMModule.h"
#include "TMath.h"
#include "TDirectory.h"

#include <chrono>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DelayDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DelayDQMModule::DelayDQMModule() : HistoModule()
{
  //Set module properties
  setDescription("Processing Delay DQM module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("DAQ"));
  addParam("title", m_title, "Prefix for Title (ERECO, HLT, ...)", std::string("Processing "));
  addParam("useMeta", m_useMeta, "Use time from EvtMetadata or FTSW", false);
}

void DelayDQMModule::BinLogX(TH1* h)
{
// function copied from root-talk

  TAxis* axis = h->GetXaxis();
  Int_t bins = axis->GetNbins();

  Axis_t from = axis->GetXmin();
  Axis_t to = axis->GetXmax();
  Axis_t width = (to - from) / bins;
  Axis_t* new_bins = new Axis_t[bins + 1];

  for (Int_t i = 0; i <= bins; i++) {
    new_bins[i] = TMath::Power(10, from + i * width);
  }
  axis->Set(bins, new_bins);
  delete[] new_bins;
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void DelayDQMModule::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());// do not use return value with ->cd(), it is ZERO if dir already exists
    oldDir->cd(m_histogramDirectoryName.c_str());
  }
  //----------------------------------------------------------------

  m_DelayS = new TH1D("DelayS", (m_title + "Delay;time /s").c_str(), 600, 0, 600);
  m_DelayMs = new TH1D("DelayMs", (m_title + "Delay;time /ms").c_str(), 200, 0, 2000);
  m_DelayLog = new TH1D("DelayLog", (m_title + "Delay; time /s").c_str(), 200, -3, 6);
  BinLogX(m_DelayLog); // set log binning, later Draw with SetLogX

  // cd back to root directory
  oldDir->cd();
}


void DelayDQMModule::initialize()
{
  // Required input
  if (m_useMeta) {
    m_eventMetaData.isRequired();
  } else {
    m_rawFTSW.isOptional(); // actuall it would be Required(); but this prevents HLT/ERECO test from working
  }

  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM
}

void DelayDQMModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  m_DelayMs->Reset();
  m_DelayS->Reset();
  m_DelayLog->Reset();
}


void DelayDQMModule::event()
{
  // Calculate the time difference between now and the trigger time
  // This tells you how much delay we have summed up (it is NOT the processing time!)
  /** Time(Tag) from MetaInfo, ns since epoch */
  using namespace std::chrono;
  nanoseconds ns = duration_cast<nanoseconds> (system_clock::now().time_since_epoch());
  nanoseconds event_time{};
  if (m_useMeta) {
    // We get the time from EventMetaData, which gets the time from TTD (FTSW)
    // BUT, this time is inaccurate for longer runs, the difference is larger than teh effect we
    // monitor in the histograms!
    event_time = static_cast<nanoseconds>(m_eventMetaData->getTime());
  } else {
    // get the trigger time from the NEW member function in TDD data
    for (auto& it : m_rawFTSW) {
      struct timeval tv;
      it.GetPCTimeVal(0, &tv);
      event_time = (static_cast<seconds>(tv.tv_sec)) + (static_cast<microseconds>(tv.tv_usec));
      break;
    }
  }
  auto deltaT = (duration_cast<milliseconds> (ns - event_time)).count();
  m_DelayMs->Fill(deltaT);
  m_DelayLog->Fill(1e-3 * deltaT);
  m_DelayS->Fill(1e-3 * deltaT);
}

