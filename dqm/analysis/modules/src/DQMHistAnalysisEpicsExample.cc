//+
// File : DQMHistAnalysisEpicsExample.cc
// Description :
//
// Author : Tomoyuki Konno, Tokyo Metropolitan Univerisity
// Date : 25 - Dec - 2015
//-


#include <dqm/analysis/modules/DQMHistAnalysisEpicsExample.h>
#include <TROOT.h>
#include <TClass.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisEpicsExample)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisEpicsExampleModule::DQMHistAnalysisEpicsExampleModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("HistoName", m_histoname, "Name of Histogram (incl dir)", std::string(""));
  addParam("Function", m_function, "Fit function definition", std::string("gaus"));
  addParam("Parameters", m_parameters, "Fit function parameters for EPICS", 3);
  addParam("PVName", m_pvname, "PV Prefix", std::string("PXD:DQM:hist"));
  B2DEBUG(20, "DQMHistAnalysisEpicsExample: Constructor done.");
}


DQMHistAnalysisEpicsExampleModule::~DQMHistAnalysisEpicsExampleModule() { }

void DQMHistAnalysisEpicsExampleModule::initialize()
{
  B2DEBUG(20, "DQMHistAnalysisEpicsExample: initialized.");

  TString a;
  a = m_histoname;
  a.ReplaceAll("/", "_");
  m_c1 = new TCanvas("c_" + a);
  m_f1 = new TF1("f_" + a, TString(m_function), -30, 300);
  m_f1->SetParameter(0, 1000);
  m_f1->SetParameter(1, 0);
  m_f1->SetParameter(2, 10);
  m_f1->SetLineColor(4);
  m_f1->SetNpx(512);
  m_f1->SetNumberFitPoints(512);

  m_line = new TLine(0, 10, 0, 0);
  m_line->SetVertical(true);
  m_line->SetLineColor(8);
  m_line->SetLineWidth(3);

  m_line_lo = new TLine(0, 10, 0, 0);
  m_line_lo->SetVertical(true);
  m_line_lo->SetLineColor(2);
  m_line_lo->SetLineWidth(3);

  m_line_hi = new TLine(0, 10, 0, 0);
  m_line_hi->SetVertical(true);
  m_line_hi->SetLineColor(2);
  m_line_hi->SetLineWidth(3);

  m_line_lo->SetX1(5);// get from epics
  m_line_lo->SetX2(5);

  m_line_hi->SetX1(50);// get from epics
  m_line_hi->SetX2(50);

  // need the function to get parameter names
  if (m_parameters > 0 && m_pvname != "") {
    if (m_parameters > 10) m_parameters = 10; // hard limit
//    SEVCHK(ca_context_create(ca_disable_preemptive_callback),"ca_context_create");
    for (auto i = 0; i < m_parameters; i++) {
      std::string a;
      a = m_f1->GetParName(i);
      if (a == "") a = "par" + i;
      a = m_pvname + ":" + a;
//      SEVCHK(ca_create_channel(a.c_str(),NULL,NULL,10,&mychid[i]),"ca_create_channel failure");
      // Read LO and HI limits from EPICS, seems this needs additional channels?
      //SEVCHK(ca_get(DBR_DOUBLE,mychid[i],(void*)&data),"ca_get failure"); // data is only valid after ca_pend_io!!
    }
//    SEVCHK(ca_pend_io(5.0),"ca_pend_io failure");
  } else {
    m_parameters = 0;
  }
}


void DQMHistAnalysisEpicsExampleModule::beginRun()
{
  //m_serv->SetTimer(100, kFALSE);
  B2DEBUG(20, "DQMHistAnalysisEpicsExample: beginRun called.");
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
    m_line_lo->Draw();
    m_line_hi->Draw();
  } else {
    B2DEBUG(20, "Histo " << m_histoname << " not found");
  }
}

void DQMHistAnalysisEpicsExampleModule::event()
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
    m_c1->cd();// necessary!
    hh1->Fit(m_f1, "");
    double y1 = hh1->GetMaximum();
    double y2 = hh1->GetMinimum();
    m_line->SetY1(y1 + (y1 - y2) * 0.05);
    m_line_lo->SetY1(y1 + (y1 - y2) * 0.05);
    m_line_hi->SetY1(y1 + (y1 - y2) * 0.05);
//     m_line->SetY2(y2-(y1-y2)*0.05);
//     m_line_lo->SetY2(y2-(y1-y2)*0.05);
//     m_line_hi->SetY2(y2-(y1-y2)*0.05);
    double x = m_f1->GetParameter(1);
    m_line->SetX1(x);
    m_line->SetX2(x);
    if (!flag) {
      // dont add another line...
      m_line->Draw();
      m_line_lo->Draw();
      m_line_hi->Draw();
    }
    m_c1->Modified();
    m_c1->Update();
  } else {
    B2DEBUG(20, "Histo " << m_histoname << " not found");
  }

  if (m_parameters > 0) {
    for (auto i = 0; i < m_parameters; i++) {
      //double data;
      //data = m_f1->GetParameter(i);
//      SEVCHK(ca_put(DBR_DOUBLE,mychid[i],(void*)&data),"ca_set failure");
    }
//    SEVCHK(ca_pend_io(5.0),"ca_pend_io failure");
  }
}

void DQMHistAnalysisEpicsExampleModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisEpicsExample : endRun called");
}


void DQMHistAnalysisEpicsExampleModule::terminate()
{
  B2DEBUG(20, "DQMHistAnalysisEpicsExample: terminate called");
}

