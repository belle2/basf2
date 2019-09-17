//+
// File : DQMHistAnalysisIP.cc
// Description : Median for IP position with delta histogramming
//
// Author : Bjoern Spruck, Mainz Univerisity
// Date : 2019
//-


#include <dqm/analysis/modules/DQMHistAnalysisIP.h>
#include <TROOT.h>
#include <TClass.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisIP)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisIPModule::DQMHistAnalysisIPModule()
  : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!

  //Parameter definition
  addParam("HistoName", m_histoname, "Name of Histogram (incl dir)", std::string(""));
  addParam("PVName", m_pvPrefix, "PV Prefix", std::string("DQM:TEST:hist:"));
  B2DEBUG(20, "DQMHistAnalysisIP: Constructor done.");
}


DQMHistAnalysisIPModule::~DQMHistAnalysisIPModule()
{
#ifdef _BELLE2_EPICS
  if (ca_current_context()) ca_context_destroy();
#endif
}

void DQMHistAnalysisIPModule::initialize()
{
  B2DEBUG(20, "DQMHistAnalysisIP: initialized.");

  TString a;
  a = m_histoname;
  a.ReplaceAll("/", "_");
  m_c1 = new TCanvas("c_" + a);

  m_line = new TLine(0, 10, 0, 0);
  m_line->SetVertical(true);
  m_line->SetLineColor(8);
  m_line->SetLineWidth(3);

  // need the function to get parameter names
#ifdef _BELLE2_EPICS
  if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
  std::string aa;
  aa = m_pvPrefix + "Median";
  SEVCHK(ca_create_channel(aa.c_str(), NULL, NULL, 10, &mychid[0]), "ca_create_channel failure");
  aa = m_pvPrefix + "RMS";
  SEVCHK(ca_create_channel(aa.c_str(), NULL, NULL, 10, &mychid[1]), "ca_create_channel failure");
  // Read LO and HI limits from EPICS, seems this needs additional channels?
  // SEVCHK(ca_get(DBR_DOUBLE,mychid[i],(void*)&data),"ca_get failure"); // data is only valid after ca_pend_io!!
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
#endif
}


void DQMHistAnalysisIPModule::beginRun()
{
  //m_serv->SetTimer(100, kFALSE);
  B2DEBUG(20, "DQMHistAnalysisIP: beginRun called.");
  m_c1->Clear();

  TH1* hh1;
  hh1 = findHist(m_histoname.c_str());

  if (hh1 == NULL) {
    B2DEBUG(20, "Histo " << m_histoname << " not in memfile");
    // the following code sux ... is there no root function for that?
    TDirectory* d = gROOT;
    TString myl = m_histoname;
    TString tok;
    Ssiz_t from = 0;
    while (myl.Tokenize(tok, from, "/")) {
      TString dummy;
      Ssiz_t f;
      f = from;
      if (myl.Tokenize(dummy, f, "/")) { // check if its the last one
        auto e = d->GetDirectory(tok);
        if (e) {
          B2DEBUG(20, "Cd Dir " << tok);
          d = e;
        }
        d->cd();
      } else {
        break;
      }
    }
    TObject* obj = d->FindObject(tok);
    if (obj != NULL) {
      if (obj->IsA()->InheritsFrom("TH1")) {
        B2DEBUG(20, "Histo " << m_histoname << " found in mem");
        hh1 = (TH1*)obj;
      }
    } else {
      B2DEBUG(20, "Histo " << m_histoname << " NOT found in mem");
    }
  }

  if (hh1 != NULL) {
    m_c1->cd();
    hh1->Draw();
    m_line->Draw();
  } else {
    B2DEBUG(20, "Histo " << m_histoname << " not found");
  }
}

void DQMHistAnalysisIPModule::event()
{
  TH1* hh1;
  bool flag = false;

  hh1 = findHist(m_histoname.c_str());
  if (hh1 == NULL) {
    B2DEBUG(20, "Histo " << m_histoname << " not in memfile");
    // the following code sux ... is there no root function for that?
    TDirectory* d = gROOT;
    TString myl = m_histoname;
    TString tok;
    Ssiz_t from = 0;
    while (myl.Tokenize(tok, from, "/")) {
      TString dummy;
      Ssiz_t f;
      f = from;
      if (myl.Tokenize(dummy, f, "/")) { // check if its the last one
        auto e = d->GetDirectory(tok);
        if (e) {
          B2DEBUG(20, "Cd Dir " << tok);
          d = e;
        }
        d->cd();
      } else {
        break;
      }
    }
    TObject* obj = d->FindObject(tok);
    if (obj != NULL) {
      if (obj->IsA()->InheritsFrom("TH1")) {
        B2DEBUG(20, "Histo " << m_histoname << " found in mem");
        hh1 = (TH1*)obj;
        flag = true;
      }
    } else {
      B2DEBUG(20, "Histo " << m_histoname << " NOT found in mem");
    }
  }
  if (hh1 != NULL) {

    if (!m_h_last) {
      m_h_last = (TH1*)hh1->Clone();
      m_h_last->Reset();
    }

    double last = m_h_last->GetEntries();
    double current = hh1->GetEntries();

    if (current - last >= m_min_entries) {
      TH1* delta = (TH1*)hh1->Clone();
      delta->Add(m_h_last, -1.);

      m_c1->cd();// necessary!
      delta->Draw("h");
      m_h_last->Reset();
      m_h_last->Add(hh1);

      double x = delta->GetMean();// must be double bc of EPICS below
      double w = delta->GetRMS();// must be double bc of EPICS below
      double y1 = delta->GetMaximum();
      double y2 = delta->GetMinimum();
      m_line->SetY1(y1 + (y1 - y2) * 0.05);
      m_line->SetX1(x);
      m_line->SetX2(x);
      if (!flag) {
        // dont add another line...
        m_line->Draw();
      }
      m_c1->Modified();
      m_c1->Update();
#ifdef _BELLE2_EPICS
      if (mychid[0]) SEVCHK(ca_put(DBR_DOUBLE, mychid[0], (void*)&x), "ca_set failure");
      if (mychid[1]) SEVCHK(ca_put(DBR_DOUBLE, mychid[1], (void*)&w), "ca_set failure");
      SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
#endif
    }
  } else {
    B2DEBUG(20, "Histo " << m_histoname << " not found");
  }

}

void DQMHistAnalysisIPModule::terminate()
{
#ifdef _BELLE2_EPICS
  if (m_parameters > 0) {
    for (auto i = 0; i < m_parameters; i++) {
      if (mychid[i]) SEVCHK(ca_clear_channel(mychid[i]), "ca_clear_channel failure");
    }
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
  B2DEBUG(20, "DQMHistAnalysisIP: terminate called");
}

