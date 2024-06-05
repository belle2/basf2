/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistDeltaHisto.cc
// Description : DQM Histogram analysis module to generate delta histograms
//-


#include <framework/core/ModuleParam.templateDetails.h>
#include <dqm/analysis/modules/DQMHistDeltaHisto.h>
#include <TROOT.h>
#include <TClass.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistDeltaHisto);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistDeltaHistoModule::DQMHistDeltaHistoModule()
  : DQMHistAnalysisModule()
{
  addParam("Interval", m_interval, "Interval time for diff histos [s]", 180);
  addParam("MonitoredHistos", m_monitoredHistos, "List of histograms to monitor", vector<string>());
  B2DEBUG(20, "DQMHistDeltaHisto: Constructor done.");
}


DQMHistDeltaHistoModule::~DQMHistDeltaHistoModule() { }

void DQMHistDeltaHistoModule::initialize()
{
  gROOT->cd();
  B2DEBUG(20, "DQMHistDeltaHisto: initialized.");
  for (auto& histoname : m_monitoredHistos) {
    queue<SSNODE*> hq;
    m_histosQueues[histoname] = hq;
  }
  m_evtMetaDataPtr.isRequired();
}


void DQMHistDeltaHistoModule::beginRun()
{
  B2DEBUG(20, "DQMHistDeltaHisto: beginRun called.");
  for (auto& histoname : m_monitoredHistos) {
    queue<SSNODE*>& hq = m_histosQueues[histoname];
    while (!hq.empty()) {
      SSNODE* nn = hq.front();
      clear_node(nn);
      delete nn;
      hq.pop();
    }
  }
}

void DQMHistDeltaHistoModule::clear_node(SSNODE* n)
{
  delete n->histo;
  delete n->diff_histo;
}

void DQMHistDeltaHistoModule::event()
{

  B2DEBUG(20, "DQMHistDeltaHisto: event called.");
  if (!m_evtMetaDataPtr.isValid()) {
    B2ERROR("No valid EventMetaData.");
    return;
  }
  time_t cur_mtime = m_evtMetaDataPtr->getTime();

  for (auto& histoname : m_monitoredHistos) {
    TH1* hh = findHist(histoname.c_str());
    if (hh == nullptr) continue;
    if (hh->GetDimension() != 1) continue;
    queue<SSNODE*>& hq = m_histosQueues[histoname];
    if (hq.empty()) {
      SSNODE* n = new SSNODE;
      n->histo = (TH1*)hh->Clone();
      n->diff_histo = (TH1*)hh->Clone();
      n->time_modified = cur_mtime;
      hq.push(n);
    } else {
      while (!hq.empty()) {
        SSNODE* nn = hq.front();
        if ((cur_mtime - nn->time_modified < m_interval) || (hq.size() == 1)) {
          if (hq.back()->time_modified == cur_mtime) {
            break;
          }
          SSNODE* n = new SSNODE;
          n->histo = (TH1*)hh->Clone();
          n->diff_histo = (TH1*)hh->Clone();
          n->diff_histo->Add(nn->histo, -1);
          n->time_modified = cur_mtime;
          hq.push(n);
          break;
        } else {
          clear_node(nn);
          delete nn;
          hq.pop();
        }
      }
    }
    auto s = StringSplit(histoname, '/');
    auto dirname = s.at(0);
    auto hname = s.at(1);
    std::string canvas_name = dirname + "/c_" + hname;
    TCanvas* c = findCanvas(canvas_name);
    if (c == nullptr) continue;
    TH1* h_diff = hq.back()->diff_histo;
    h_diff->SetName((histoname + "_diff").data());
    if (h_diff->Integral() != 0) h_diff->Scale(hh->Integral() / h_diff->Integral());
    c->cd();
    h_diff->SetLineColor(kRed);
    h_diff->SetLineStyle(kDotted);
    h_diff->SetStats(kFALSE);
    h_diff->Draw("hist,same");
    c->Modified();
    c->Update();
  }

}

void DQMHistDeltaHistoModule::endRun()
{
  B2DEBUG(20, "DQMHistDeltaHisto: endRun called");
}


void DQMHistDeltaHistoModule::terminate()
{
  B2DEBUG(20, "DQMHistDeltaHisto: terminate called");
}
