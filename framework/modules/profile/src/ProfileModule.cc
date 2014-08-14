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

ProfileModule::ProfileModule() : Module(), m_outputFileName(""), m_timeOffset(Utils::getClock() / Unit::s), m_nEvents(0), m_step(1), m_eventInfo(k_maxPoints)
{
  // Set module properties
  setDescription("Records execution time and memory usage in ProfileInfo objects for each event. Can also graph memory usage.");
  addParam("outputFileName", m_outputFileName, "File name of memory usage plot.", std::string(""));
}

void ProfileModule::initialize()
{
  StoreObjPtr<ProfileInfo> profileInfoPtr;
  StoreObjPtr<ProfileInfo> profileInfoStartPtr("ProfileInfo_Start", DataStore::c_Persistent);
  StoreObjPtr<ProfileInfo> profileInfoEndPtr("ProfileInfo_End", DataStore::c_Persistent);
  // Register the profile info objects in the data store
  profileInfoPtr.registerInDataStore();
  profileInfoStartPtr.registerInDataStore();
  profileInfoEndPtr.registerInDataStore();

  // Store and print profile info at initialization
  profileInfoStartPtr.create();
  profileInfoStartPtr->set(m_timeOffset);
  m_initializeInfo.mem = profileInfoStartPtr->getMemory();
  m_initializeInfo.time = profileInfoStartPtr->getTimeInSec();
  B2INFO("Memory usage at initialization [MB]: " << m_initializeInfo.mem / 1024);
}

void ProfileModule::event()
{
  // Store and print profile info at this event
  StoreObjPtr<ProfileInfo> profileInfoPtr;
  profileInfoPtr.create();
  profileInfoPtr->set(m_timeOffset);
  m_eventInfo.push_back(MemTime(profileInfoPtr->getMemory(), profileInfoPtr->getTimeInSec()));
  B2DEBUG(100, "Memory usage [MB]: " << profileInfoPtr->getMemory() / 1024);

  // Remember profile info at first (+burn in) and last event
  if (m_nEvents == k_burnIn) {
    m_startInfo.mem = profileInfoPtr->getMemory();
    m_startInfo.time = profileInfoPtr->getTimeInSec();
  }
  m_endInfo.mem = profileInfoPtr->getMemory();
  m_endInfo.time = profileInfoPtr->getTimeInSec();

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

    m_eventInfo[index].mem = profileInfoPtr->getMemory();
    m_eventInfo[index].time = profileInfoPtr->getTimeInSec();
  }

  m_nEvents++;
}

void ProfileModule::terminate()
{
  // Store and print profile info at termination
  StoreObjPtr<ProfileInfo> profileInfoEndPtr("ProfileInfo_End", DataStore::c_Persistent);
  profileInfoEndPtr.create();
  profileInfoEndPtr->set(m_timeOffset);
  m_terminateInfo.mem = profileInfoEndPtr->getMemory();
  m_terminateInfo.time = profileInfoEndPtr->getTimeInSec();
  B2INFO("Memory usage at termination [MB]: " << m_terminateInfo.mem / 1024);

  if (m_nEvents > k_burnIn) {
    B2INFO("Memory increase per event [kB]  : " << ((long)m_endInfo.mem - (long)m_startInfo.mem) / (m_nEvents - k_burnIn));
    B2INFO("Execution time per event [ms]   : " << 1000 * (m_endInfo.time - m_startInfo.time) / (m_nEvents - k_burnIn));
  }

  // Create and save a plot of the memory usage
  if (!m_outputFileName.empty()) {
    TDirectory* saveDir = gDirectory;
    gROOT->cd();
    TCanvas* can = new TCanvas();
    int nPoints = m_nEvents / m_step;
    TGraph* graph = new TGraph(nPoints);
    for (int i = 0; i < nPoints; i++) {
      graph->SetPoint(i, m_eventInfo[i].time, (double)m_eventInfo[i].mem / 1024);
    }
    graph->SetMarkerStyle(kMultiply);
    graph->Draw("ALP");
    TH1* histo = graph->GetHistogram();
    histo->SetTitle("");
    histo->GetYaxis()->SetTitle("Memory Usage [MB]");
    histo->GetXaxis()->SetTitle("Time [s]");
    can->Print(m_outputFileName.c_str());
    saveDir->cd();
  }
}
