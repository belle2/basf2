//+
// File : DQMHistSnapshots.cc
// Description : DQM Histogram analysis module, generate snapshots of histograms
//
// Author : Boqun Wang, U. of Cincinnati
// Date : yesterday
//-


#include <framework/core/ModuleParam.templateDetails.h>
#include <dqm/analysis/modules/DQMHistSnapshots.h>
#include <daq/slc/base/StringUtil.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TStyle.h>
#include <TClass.h>
#include <TDirectory.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TKey.h>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistSnapshots)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistSnapshotsModule::DQMHistSnapshotsModule()
  : DQMHistAnalysisModule()
{
  addParam("CheckInterval", m_check_interval, "Interval of two checks", 180);
  B2DEBUG(1, "DQMHistSnapshots: Constructor done.");
}


DQMHistSnapshotsModule::~DQMHistSnapshotsModule() { }


TH1* DQMHistSnapshotsModule::GetHisto(TString histoname)
{
  TH1* hh1;
  gROOT->cd();
  hh1 = findHist(histoname.Data());

  if (hh1 == NULL) {
    B2DEBUG(20, "Histo " << histoname << " not found");
  }

  return hh1;
}

void DQMHistSnapshotsModule::initialize()
{
  gROOT->cd();
  B2DEBUG(20, "DQMHistSnapshots: initialized.");
}


void DQMHistSnapshotsModule::beginRun()
{
  m_ssnode.clear();
  B2DEBUG(20, "DQMHistSnapshots: beginRun called.");
}

DQMHistSnapshotsModule::SSNODE* DQMHistSnapshotsModule::find_snapshot(TString a)
{
  for (auto& it : m_ssnode) {
    if (it->histo->GetName() == a)
      return it;
  }
  return NULL;
}

TCanvas* DQMHistSnapshotsModule::find_canvas(TString canvas_name)
{
  TIter nextkey(gROOT->GetListOfCanvases());
  TObject* obj = NULL;

  while ((obj = (TObject*)nextkey())) {
    if (obj->IsA()->InheritsFrom("TCanvas")) {
      if (obj->GetName() == canvas_name)
        return (TCanvas*)obj;
    }
  }
  return NULL;
}

void DQMHistSnapshotsModule::event()
{

  time_t cur_time = time(NULL);
  if (cur_time - m_last_check < m_check_interval) return;
  m_last_check = cur_time;

  const HistList& hlist = getHistList();

  for (HistList::const_iterator it = hlist.begin(); it != hlist.end(); it++) {
    TString a = it->first;

    SSNODE* n = find_snapshot(a);
    if (n == NULL) { // no existing snapshot, create new one
      n = new SSNODE;
      n->histo = (TH1*) it->second->Clone();

      StringList s = StringUtil::split(a.Data(), '/');
      std::string dirname = s[0];
      std::string hname = s[1];
      std::string canvas_name = dirname + "/c_" + hname;
      n->canvas = find_canvas(canvas_name);

      m_ssnode.push_back(n);
    } else { // compare with existing snapshot
      TH1* h = it->second;
      if (h->GetEntries() > n->histo->GetEntries()) { // need better way to determine whether the histo is updated
        delete n->histo;
        n->histo = (TH1*)h->Clone();
      } else { // notify that the histogram is stale
        if (n->canvas != NULL) {
          h->SetTitle((h->GetTitle() + string(" NOT UPDATED")).c_str());
        }
      }
    }

  }
}

void DQMHistSnapshotsModule::endRun()
{
  B2DEBUG(20, "DQMHistSnapshots: endRun called");
}


void DQMHistSnapshotsModule::terminate()
{
  B2DEBUG(20, "DQMHistSnapshots: terminate called");
}
