//+
// File : DQMHistAnalysisPXDFits.cc
// Description : DQM module, which fits many PXD histograms and writes out fit parameters in new histograms
//
// Author : B. Spruck
// based on work from Tomoyuki Konno, Tokyo Metropolitan Univerisity
// Date : someday
//-


#include <dqm/analysis/modules/DQMHistAnalysisPXDFits.h>
#include <TROOT.h>
#include <TClass.h>

using namespace std;
using namespace Belle2;


using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisPXDFits)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisPXDFitsModule::DQMHistAnalysisPXDFitsModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
//  addParam("HistoName", m_histoname, "Name of Histogram (incl dir)", std::string(""));
  B2DEBUG(1, "DQMHistAnalysisPXDFits: Constructor done.");
}


DQMHistAnalysisPXDFitsModule::~DQMHistAnalysisPXDFitsModule() { }

void DQMHistAnalysisPXDFitsModule::initialize()
{
  B2DEBUG(1, "DQMHistAnalysisPXDFits: initialized.");

  gROOT->cd(); // this seems to be important, or strange things happen
  for (auto i = 0; i < 64; i++) {
    TString a;
    auto num1 = (((i >> 5) & 0x1) + 1);
    auto num2 = ((i >> 1) & 0xF);
    auto num3 = ((i & 0x1) + 1);
    string s2 = str(format("_%d.%d.%d") % num1 % num2 % num3);

    a = "hSignal";
    a.ReplaceAll("/", "_");
    a += s2;

    m_cSignal[i] = new TCanvas("c_" + a);
    m_hSignal[i] = new TH2F(a, a, 6, 0, 6, 4, 0, 4);
    m_hSignal[i]->SetDirectory(0);// dont mess with it, this is MY histogram

    a = "hCommon";
    a.ReplaceAll("/", "_");
    a += s2;
    m_cCommon[i] = new TCanvas("c_" + a);
    m_hCommon[i] = new TH2F(a, a, 6, 0, 6, 4, 0, 4);
    m_hCommon[i]->SetDirectory(0);// dont mess with it, this is MY histogram

    a = "hCounts";
    a.ReplaceAll("/", "_");
    a += s2;
    m_cCounts[i] = new TCanvas("c_" + a);
    m_hCounts[i] = new TH2F(a, a, 6, 0, 6, 4, 0, 4);
    m_hCounts[i]->SetDirectory(0);// dont mess with it, this is MY histogram
  }

  m_fLandau = new TF1("f_Landau", "landau", 0, 256);
  m_fLandau->SetParameter(0, 1000);
  m_fLandau->SetParameter(1, 0);
  m_fLandau->SetParameter(2, 10);
  m_fLandau->SetLineColor(4);
  m_fLandau->SetNpx(256);
  m_fLandau->SetNumberFitPoints(256);

  m_fGaus = new TF1("f_Gaus", "gaus", 0, 8096);
  m_fGaus->SetParameter(0, 1000);
  m_fGaus->SetParameter(1, 0);
  m_fGaus->SetParameter(2, 10);
  m_fGaus->SetLineColor(4);
  m_fGaus->SetNpx(256);
  m_fGaus->SetNumberFitPoints(256);
}


void DQMHistAnalysisPXDFitsModule::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisPXDFits: beginRun called.");

  // not much we can do here ... as we have created everything already
  for (auto i = 0; i < 64; i++) {
    m_cSignal[i]->Clear();
    m_cCommon[i]->Clear();
    m_cCounts[i]->Clear();
    // no need to Cd and Draw yet
  }
}


TH1* DQMHistAnalysisPXDFitsModule::findHistLocal(TString& a)
{
  B2INFO("Histo " << a << " not in memfile");
  // the following code sux ... is there no root function for that?
  TDirectory* d = gROOT;
  TString myl = a;
  TString tok;
  Ssiz_t from = 0;
  while (myl.Tokenize(tok, from, "/")) {
    TString dummy;
    Ssiz_t f;
    f = from;
    if (myl.Tokenize(dummy, f, "/")) { // check if its the last one
      auto e = d->GetDirectory(tok);
      if (e) {
        B2INFO("Cd Dir " << tok);
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
      B2INFO("Histo " << a << " found in mem");
      return (TH1*)obj;
    }
  } else {
    B2INFO("Histo " << a << " NOT found in mem");
  }
  return NULL;
}

void DQMHistAnalysisPXDFitsModule::event()
{
//  bool flag = false;

  for (auto i = 0; i < 64; i++) {
    auto num1 = (((i >> 5) & 0x1) + 1);
    auto num2 = ((i >> 1) & 0xF);
    auto num3 = ((i & 0x1) + 1);

    m_hSignal[i]->Reset(); // dont sum up!!!
    m_hCommon[i]->Reset(); // dont sum up!!!
    m_hCounts[i]->Reset(); // dont sum up!!!

    for (auto j = 0; j < 6; j++) {
      for (auto k = 0; k < 4; k++) {
        TH1* hh1;
        string s2 = str(format("_%d.%d.%d_%d_%d") % num1 % num2 % num3 % j % k);

        TString a;

        a = "hrawPxdHitsCharge" + s2;
        hh1 = findHist(a.Data());
        if (hh1 == NULL) {
          hh1 = findHistLocal(a);
        }
        if (hh1 == NULL) {
          a = "pxdraw/hrawPxdHitsCharge" + s2;
          hh1 = findHist(a.Data());
        }
        if (hh1 == NULL) {
          a = "pxdraw/hrawPxdHitsCharge" + s2;
          hh1 = findHistLocal(a);
        }

        if (hh1 != NULL) {
          cout << "do da fit " << endl;
          m_fLandau->SetParameter(0, 1000);
          m_fLandau->SetParameter(1, 0);
          m_fLandau->SetParameter(2, 10);
          hh1->Fit(m_fLandau, "0");
          m_hSignal[i]->Fill(j, k, m_fLandau->GetParameter(1));
          cout << m_fLandau->GetParameter(0) << " " << m_fLandau->GetParameter(1) << " " << m_fLandau->GetParameter(2) << endl;
        } else {
          B2INFO("Histo " << a << " not found");
        }

        a = "hrawPxdHitsCommonMode" + s2;
        hh1 = findHist(a.Data());
        if (hh1 == NULL) {
          hh1 = findHistLocal(a);
        }
        if (hh1 == NULL) {
          a = "pxdraw/hrawPxdHitsCommonMode" + s2;
          hh1 = findHist(a.Data());
        }
        if (hh1 == NULL) {
          a = "pxdraw/hrawPxdHitsCommonMode" + s2;
          hh1 = findHistLocal(a);
        }

        if (hh1 != NULL) {
          cout << "do da fit " << endl;
          m_fGaus->SetParameter(0, 1000);
          m_fGaus->SetParameter(1, 10);
          m_fGaus->SetParameter(2, 10);
          hh1->Fit(m_fGaus, "0");
          m_hCommon[i]->Fill(j, k, m_fGaus->GetParameter(1));
          cout << m_fGaus->GetParameter(0) << " " << m_fGaus->GetParameter(1) << " " << m_fGaus->GetParameter(2) << endl;
        } else {
          B2INFO("Histo " << a << " not found");
        }

        a = "hrawPxdCount" + s2;
        hh1 = findHist(a.Data());
        if (hh1 == NULL) {
          hh1 = findHistLocal(a);
        }
        if (hh1 == NULL) {
          a = "pxdraw/hrawPxdCount" + s2;
          hh1 = findHist(a.Data());
        }
        if (hh1 == NULL) {
          a = "pxdraw/hrawPxdCount" + s2;
          hh1 = findHistLocal(a);
        }

        if (hh1 != NULL) {
          cout << "do da fit " << endl;
          m_fGaus->SetParameter(0, 1000);
          m_fGaus->SetParameter(1, 100);
          m_fGaus->SetParameter(2, 10);
          hh1->Fit(m_fGaus, "0");
          m_hCounts[i]->Fill(j, k, m_fGaus->GetParameter(1));
          cout << m_fGaus->GetParameter(0) << " " << m_fGaus->GetParameter(1) << " " << m_fGaus->GetParameter(2) << endl;
        } else {
          B2INFO("Histo " << a << " not found");
        }
      }
    }
    if (m_cSignal[i]) {
      m_cSignal[i]->cd();
      if (m_hSignal[i]) m_hSignal[i]->Draw("colz");
      m_cSignal[i]->Modified();
      m_cSignal[i]->Update();
    }
    if (m_cCommon[i]) {
      m_cCommon[i]->cd();
      if (m_hCommon[i]) m_hCommon[i]->Draw("colz");
      m_cCommon[i]->Modified();
      m_cCommon[i]->Update();
    }
    if (m_cCounts[i]) {
      m_cCounts[i]->cd();
      if (m_hCounts[i]) m_hCounts[i]->Draw("colz");
      m_cCounts[i]->Modified();
      m_cCounts[i]->Update();
    }
  }
}

void DQMHistAnalysisPXDFitsModule::endRun()
{
  B2DEBUG(1, "DQMHistAnalysisPXDFits : endRun called");
}


void DQMHistAnalysisPXDFitsModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisPXDFits: terminate called");
}

