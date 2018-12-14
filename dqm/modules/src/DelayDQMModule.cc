/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Description: Create plots NOW-TriggerTime for performance monitoring   *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
  addParam("histgramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string(""));
  addParam("title", m_title, "Prefix for Title (ERECO, HLT, ...)", std::string("Processing "));
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
  if (m_histogramDirectoryName != "") oldDir->mkdir(m_histogramDirectoryName.c_str())->cd();
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
  m_eventMetaData.isRequired();

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
  unsigned long long int meta_time = 0;
  meta_time = m_eventMetaData->getTime();

  using namespace std::chrono;
  nanoseconds ns = duration_cast< nanoseconds >(
                     system_clock::now().time_since_epoch());
  Float_t deltaT = 0.0;
  deltaT = (std::chrono::duration_cast<milliseconds> (ns - (nanoseconds)meta_time)).count();
  m_DelayMs->Fill(deltaT);
  deltaT = (std::chrono::duration_cast<seconds> (ns - (nanoseconds)meta_time)).count();
  m_DelayS->Fill(deltaT);
  m_DelayLog->Fill(deltaT);
}

