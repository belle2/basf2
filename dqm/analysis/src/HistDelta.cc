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


void HistDelta::update(TH1* currentHist)
{
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
//  B2DEBUG(20, "Entries: " << last_entries << "," << current_entries);
  if (current_entries - last_entries >= m_parameter) {
//      B2DEBUG(20, "Update Delta");
    gROOT->cd();
    TH1* delta = (TH1*)currentHist->Clone();
    delta->Add(m_lastHist, -1.);
    m_deltaHists.push(delta);// insert at position 0, move all others, maybe better use push and pop!
    if (m_deltaHists.size() > m_amountDeltas) {
      auto h = m_deltaHists.front();
      m_deltaHists.pop(); // does it proper delet?
      if (h) delete h;
    }
    m_lastHist->Reset();
    m_lastHist->Add(currentHist);
  } else {
    // and now check if we didnt update, do we want to have initial sampling
    // a bit hard to check
  }
}

void HistDelta::reset(void)
{
  // m_deltaHists.clear(); // loop and delete? to be checked what is left in memory
  // by intention, we may not want to delete old m_deltaHists, thus having them from m_lastHist run? tbd
  if (m_lastHist) m_lastHist->Reset();
}

