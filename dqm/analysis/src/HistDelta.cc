/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "dqm/analysis/HistDelta.h"
#include <string>
#include <TROOT.h>

using namespace Belle2;

HistDelta::HistDelta(int t, int p, unsigned int a)
{
  m_type = t;
  m_parameter = p;
  m_amountDeltas = a;
  m_lastHist = nullptr;
};

void HistDelta::set(int t, int p, unsigned int a)
{
  m_type = t;
  m_parameter = p;
  m_amountDeltas = a;
};

void HistDelta::update(TH1* currentHist)
{
  if (currentHist == nullptr) return; // this wont make sense
  gROOT->cd(); // make sure we dont accidentally write the histograms to a open file
  // cover first update after start
  if (m_lastHist == nullptr) {
    m_lastHist = (TH1*)currentHist->Clone();
    m_lastHist->SetName(TString(currentHist->GetName()) + "_last");
    m_lastHist->Reset();
  }
  // now check if need to update m_deltaHists

  // start with the simple case, we may add other types later
  double last_entries = m_lastHist->GetEntries();
  double current_entries = currentHist->GetEntries();
  if (current_entries - last_entries >= m_parameter) {
    gROOT->cd();
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
  /// we will NOT cover the case of initial sampling in this code
  /// but leave it up to the user code
  /// e.g. if getDelta(0) returns a nullptr, the user code writer
  /// should decide if it is useful to use the basic histogram
  /// }
}

void HistDelta::reset(void)
{
  // m_deltaHists.clear(); // loop and delete? to be checked what is left in memory
  // by intention, we may not want to delete old m_deltaHists, thus having them from m_lastHist run? tbd
  if (m_lastHist) m_lastHist->Reset();
}

TH1* HistDelta::getDelta(unsigned int n)
{
  if (n >= m_deltaHists.size()) return nullptr;
  return m_deltaHists.at(n);
}
