/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/profile/ProfileModule.h>
#include <framework/dataobjects/ProfileInfo.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/utilities/Utils.h>
#include <framework/gearbox/Unit.h>

#include <TROOT.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TH1.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Profile)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ProfileModule::ProfileModule() : Module(), m_outputFileName(""), m_rssOutputFileName(""), m_timeOffset(Utils::getClock() / Unit::s),
  m_nEvents(0), m_step(1),
  m_eventInfo(k_maxPoints)
{
  // Set module properties
  setDescription("Records execution time and memory usage in ProfileInfo objects for each event. Can also graph memory usage.");
  addParam("outputFileName", m_outputFileName, "File name of virtual memory usage plot.", std::string(""));
  addParam("rssOutputFileName", m_rssOutputFileName, "File name of rss memory usage plot.", std::string(""));
}

void ProfileModule::initialize()
{
  m_timeOffset = Utils::getClock() / Unit::s;
  m_nEvents = 0;
  m_step = 1;

  StoreObjPtr<ProfileInfo> profileInfoPtr;
  StoreObjPtr<ProfileInfo> profileInfoStartPtr("ProfileInfo_Start", DataStore::c_Persistent);
  StoreObjPtr<ProfileInfo> profileInfoEndPtr("ProfileInfo_End", DataStore::c_Persistent);
  StoreObjPtr<TH1D> virtprofileHistogram("VirtualMemoryProfile", DataStore::c_Persistent);
  StoreObjPtr<TH1D> rssProfileHistogram("RssMemoryProfile", DataStore::c_Persistent);
  // Register the profile info objects in the data store
  profileInfoPtr.registerInDataStore();
  profileInfoStartPtr.registerInDataStore();
  profileInfoEndPtr.registerInDataStore();
  virtprofileHistogram.registerInDataStore();
  rssProfileHistogram.registerInDataStore();

  // Store and print profile info at initialization
  profileInfoStartPtr.create(true);
  profileInfoStartPtr->set(m_timeOffset);
  m_initializeInfo = MemTime(profileInfoStartPtr->getVirtualMemory(),
                             profileInfoStartPtr->getRssMemory(),
                             profileInfoStartPtr->getTimeInSec());
  B2INFO("Virtual Memory usage at initialization [MB]: " << m_initializeInfo.virtualMem / 1024);
}

void ProfileModule::event()
{
  // Store and print profile info at this event
  StoreObjPtr<ProfileInfo> profileInfoPtr;
  profileInfoPtr.create(true);
  profileInfoPtr->set(m_timeOffset);
  B2DEBUG(100, "Virtual Memory usage [MB]: " << profileInfoPtr->getVirtualMemory() / 1024);

  // Remember profile info at first (+burn in) and last event
  if (m_nEvents == k_burnIn) {
    m_startInfo = MemTime(profileInfoPtr->getVirtualMemory(),
                          profileInfoPtr->getRssMemory(),
                          profileInfoPtr->getTimeInSec());
  }
  m_endInfo = MemTime(profileInfoPtr->getVirtualMemory(),
                      profileInfoPtr->getRssMemory(),
                      profileInfoPtr->getTimeInSec());

  // Add profile info to the vector after each step of events
  if (m_nEvents % m_step == 0) {
    int index = m_nEvents / m_step;

    // Shrink the vector if the maximal number of points is reached
    if (index >= k_maxPoints) {
      for (int i = 1; i < k_maxPoints / 2; i++) {
        m_eventInfo[i] = m_eventInfo[2 * i];
      }
      m_step *= 2;
      index /= 2;
    }

    m_eventInfo[index] = MemTime(profileInfoPtr->getVirtualMemory(),
                                 profileInfoPtr->getRssMemory(),
                                 profileInfoPtr->getTimeInSec());
  }

  m_nEvents++;
}

void ProfileModule::storeMemoryGraph(std::string name, std::string title, std::string yAxisName,
                                     std::string imgOutput, MemoryExtractLambda lmdMemoryExtract)
{
  // Create and save a plot of the memory usage vs. time
  int nPoints = m_nEvents / m_step;
  double factorMB = 1. / 1024;
  if (!imgOutput.empty()) {
    TDirectory* saveDir = gDirectory;
    gROOT->cd();
    TCanvas* can = new TCanvas();
    TGraph* graph = new TGraph(nPoints);
    for (int i = 0; i < nPoints; i++) {
      graph->SetPoint(i, m_eventInfo[i].time, lmdMemoryExtract(m_eventInfo[i]) * factorMB);
    }
    graph->SetMarkerStyle(kMultiply);
    graph->Draw("ALP");
    TH1* histo = graph->GetHistogram();
    histo->SetTitle("");
    histo->GetYaxis()->SetTitle(yAxisName.c_str());
    histo->GetXaxis()->SetTitle("Time [s]");
    can->Print(imgOutput.c_str());
    saveDir->cd();
    delete can;
    delete graph;
  }

  // Create a histogram of the memory usage vs. number of events and add it to the DataStore
  StoreObjPtr<TH1D> profileHistogram(name, DataStore::c_Persistent);
  profileHistogram.assign(new TH1D(name.c_str(), title.c_str(),  nPoints + 1, 0 - 0.5 * m_step, m_nEvents + 0.5 * m_step), true);
  profileHistogram->SetDirectory(0);
  profileHistogram->SetStats(0);
  profileHistogram->GetYaxis()->SetTitle(yAxisName.c_str());
  profileHistogram->GetXaxis()->SetTitle("Event");
  profileHistogram->SetBinContent(1, lmdMemoryExtract(m_initializeInfo) * factorMB);
  for (int i = 0; i < nPoints; i++) {
    profileHistogram->SetBinContent(i + 2, lmdMemoryExtract(m_eventInfo[i]) * factorMB);
  }
}

void ProfileModule::terminate()
{
  // Store and print profile info at termination
  StoreObjPtr<ProfileInfo> profileInfoEndPtr("ProfileInfo_End", DataStore::c_Persistent);
  profileInfoEndPtr.create(true);
  profileInfoEndPtr->set(m_timeOffset);
  m_terminateInfo = MemTime(profileInfoEndPtr->getVirtualMemory(),
                            profileInfoEndPtr->getRssMemory(),
                            profileInfoEndPtr->getTimeInSec());
  B2INFO("Virtual Memory usage at termination [MB]: " << m_terminateInfo.virtualMem / 1024);
  B2INFO("Rss Memory usage at termination [MB]    : " << m_terminateInfo.rssMem / 1024);

  if (m_nEvents > k_burnIn) {
    B2INFO("Virtual Memory increase per event [kB]  : " << ((long)m_endInfo.virtualMem - (long)m_startInfo.virtualMem) /
           (m_nEvents - k_burnIn));
    B2INFO("Rss Memory increase per event [kB]      : " << ((long)m_endInfo.rssMem - (long)m_startInfo.rssMem) /
           (m_nEvents - k_burnIn));
    B2INFO("Execution time per event [ms]           : " << 1000 * (m_endInfo.time - m_startInfo.time) / (m_nEvents - k_burnIn));
  }

  // store the plots of ther memory consumption for virtual and rss memory
  storeMemoryGraph("VirtualMemoryProfile", "Virtual Memory usage", "Virtual Memory Usage [MB]",
                   m_outputFileName, m_extractVirtualMem);
  storeMemoryGraph("RssMemoryProfile", "Rss Memory usage", "Rss Memory Usage [MB]",
                   m_rssOutputFileName, m_extractRssMem);
}
