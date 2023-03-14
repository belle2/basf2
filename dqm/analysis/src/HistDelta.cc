/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <dqm/analysis/HistDelta.h>
#include <dqm/core/DQMHistAnalysis.h>
#include <string>
#include <TROOT.h>

using namespace Belle2;

HistDelta::HistDelta(EDeltaType t, int p, unsigned int a)
{
  m_type = t;
  m_parameter = p;
  m_amountDeltas = a;
  m_lastHist = nullptr;
  m_lastValue = 0;
};

void HistDelta::set(EDeltaType t, int p, unsigned int a)
{
  m_type = t;
  m_parameter = p;
  m_amountDeltas = a;
};

void HistDelta::update(TH1* currentHist)
{
  m_updated = false;
  if (currentHist == nullptr) return; // this wont make sense
  gROOT->cd(); // make sure we dont accidentally write the histograms to a open file
  // cover first update after start
  if (m_lastHist == nullptr) {
    m_lastHist = (TH1*)currentHist->Clone();
    m_lastHist->SetName(TString(currentHist->GetName()) + "_last");
    m_lastHist->Reset();
    m_updated = true;
  }
  // now check if need to update m_deltaHists
  bool need_update = false;
  switch (m_type) {
    case c_Entries:
      // default case, look at the entries in the histogram
      need_update = currentHist->GetEntries() - m_lastHist->GetEntries() >= m_parameter;
      break;
    case c_Underflow:
      // here we misuse underflow as event counter in some histograms, e.g. PXD
      need_update = currentHist->GetBinContent(0) - m_lastHist->GetBinContent(0) >= m_parameter;
      break;
    case c_Events:
      // use event processed counter
      need_update = DQMHistAnalysisModule::getEventProcessed() - m_lastValue >= m_parameter;
      if (need_update) m_lastValue = DQMHistAnalysisModule::getEventProcessed(); // update last value
      break;
    default:
      // any unsupported types map to case 0(Disabled), and will disable delta for this hist
      [[fallthrough]];
    case c_Disabled:
      break;
  }

  if (need_update) {
    m_updated = true;
    TH1* delta = (TH1*)currentHist->Clone();
    delta->Add(m_lastHist, -1.);

    // we use this as a fifo, but cannot use queue as we need the random access
    // maybe use deque?
    m_deltaHists.emplace(m_deltaHists.begin(), delta);
    if (m_deltaHists.size() > m_amountDeltas) {
      // remove (and delete) last element
      auto h = m_deltaHists.back();
      m_deltaHists.erase(m_deltaHists.begin() + m_deltaHists.size() - 1);
      if (h) delete h;
    }
    m_lastHist->Reset();
    m_lastHist->Add(currentHist);
  }
  /// else {
  /// not (yet) enough data for update
  /// special case of initial sampling is not covered in this code
  /// -> leave it up to the user code
  /// e.g. if getDelta(0) returns a nullptr, the user code writer
  /// should decide if it is useful to use the basic histogram
  /// }
}

void HistDelta::reset(void)
{
  // m_deltaHists.clear(); // loop and delete? to be checked what is left in memory
  // by intention, we may not want to delete old m_deltaHists, thus having them from m_lastHist run? tbd
  if (m_lastHist) m_lastHist->Reset();
  m_updated = true;
}

TH1* HistDelta::getDelta(unsigned int n, bool onlyIfUpdated)
{
  if (onlyIfUpdated && !m_updated) return nullptr;// not updated, but requested
  if (n >= m_deltaHists.size()) return nullptr;
  return m_deltaHists.at(n);
}
