/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistSnapshots.cc
// Description : DQM Histogram analysis module, generate snapshots of histograms
//-


#include <framework/core/ModuleParam.templateDetails.h>
#include <dqm/analysis/modules/DQMHistSnapshots.h>
#include <daq/slc/base/StringUtil.h>
#include <TROOT.h>
#include <TClass.h>
#include <TH1F.h>
#include <TH2F.h>

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
  addParam("CheckInterval", m_check_interval, "Interval between two checks [s]", 180);
  B2DEBUG(1, "DQMHistSnapshots: Constructor done.");
}


DQMHistSnapshotsModule::~DQMHistSnapshotsModule() { }

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
  int check = 0;
  if ((m_last_check == 0) || (cur_time - m_last_check > m_check_interval)) {
    check = 1;
    m_last_check = cur_time;
  }

  const HistList& hlist = getHistList();

  for (HistList::const_iterator it = hlist.begin(); it != hlist.end(); ++it) {
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
      n->stale = 0;

      m_ssnode.push_back(n);
    } else {
      TH1* h = it->second;
      if (check == 1) {
        if (h->GetEntries() > n->histo->GetEntries()) { // histogram has been updated
          delete n->histo;
          n->histo = (TH1*)h->Clone();
          n->stale = 0;
        } else { // notify that the histogram is stale
          n->stale = 1;
        }
      }
      if (n->stale == 1 && n->canvas != NULL) {
        h->SetTitle((h->GetTitle() + string(" [STALLED]")).c_str());
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
