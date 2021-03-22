//+
// File : DQMHistDeltaHisto.cc
// Description : DQM Histogram analysis module to generate delta histograms
//
// Author : Boqun Wang, MPI for physics
// Date : yesterday
//-


#include <framework/core/ModuleParam.templateDetails.h>
#include <dqm/analysis/modules/DQMHistDeltaHisto.h>
#include <daq/slc/base/StringUtil.h>
#include <TROOT.h>
#include <TClass.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistDeltaHisto)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistDeltaHistoModule::DQMHistDeltaHistoModule()
  : DQMHistAnalysisModule()
{
  addParam("Interval", m_interval, "Interval time for diff histos [s]", 180);
  addParam("MonitoredHistos", m_monitored_histos, "List of histograms to monitor", vector<string>());
  B2DEBUG(20, "DQMHistDeltaHisto: Constructor done.");
}


DQMHistDeltaHistoModule::~DQMHistDeltaHistoModule() { }

void DQMHistDeltaHistoModule::initialize()
{
  gROOT->cd();
  B2DEBUG(20, "DQMHistDeltaHisto: initialized.");
  for (auto& histoname : m_monitored_histos) {
    queue<SSNODE*> hq;
    m_histos_queues[histoname] = hq;
  }
  m_evtMetaDataPtr.isRequired();
}


void DQMHistDeltaHistoModule::beginRun()
{
  B2DEBUG(20, "DQMHistDeltaHisto: beginRun called.");
  for (auto& histoname : m_monitored_histos) {
    queue<SSNODE*>& hq = m_histos_queues[histoname];
    while (!hq.empty()) {
      SSNODE* nn = hq.front();
      clear_node(nn);
      delete nn;
      hq.pop();
    }
  }
}

TCanvas* DQMHistDeltaHistoModule::find_canvas(TString canvas_name)
{
  TIter nextkey(gROOT->GetListOfCanvases());
  TObject* obj = nullptr;

  while ((obj = dynamic_cast<TObject*>(nextkey()))) {
    if (obj->IsA()->InheritsFrom("TCanvas")) {
      if (obj->GetName() == canvas_name)
        return dynamic_cast<TCanvas*>(obj);
    }
  }
  return nullptr;
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

  for (auto& histoname : m_monitored_histos) {
    TH1* hh = findHist(histoname.c_str());
    if (hh == nullptr) continue;
    if (hh->GetDimension() != 1) continue;
    queue<SSNODE*>& hq = m_histos_queues[histoname];
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
    TString a = histoname;
    StringList s = StringUtil::split(a.Data(), '/');
    std::string dirname = s[0];
    std::string hname = s[1];
    std::string canvas_name = dirname + "/c_" + hname;
    TCanvas* c = find_canvas(canvas_name);
    if (c == nullptr) continue;
    TH1* h_diff = hq.back()->diff_histo;
    h_diff->SetName((a + "_diff").Data());
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
