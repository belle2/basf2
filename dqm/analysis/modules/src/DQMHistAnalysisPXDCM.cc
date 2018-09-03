//+
// File : DQMHistAnalysisPXDCM.cc
// Description : Analysis of PXD Common Modes
//
// Author : Bjoern Spruck, Univerisity Mainz
// Date : 2018
//-


#include <dqm/analysis/modules/DQMHistAnalysisPXDCM.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TClass.h>

using namespace std;
using namespace Belle2;


using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisPXDCM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisPXDCMModule::DQMHistAnalysisPXDCMModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("HistoDir", m_histodir, "Name of Histogram dir", std::string("pxd"));
  B2DEBUG(1, "DQMHistAnalysisPXDCM: Constructor done.");

  for (auto i = 0, j = 0; i < 64; i++) {
    auto layer = (((i >> 5) & 0x1) + 1);
    auto ladder = ((i >> 1) & 0xF);
    if (ladder == 0) continue; // numbering starts at 1
    if (layer == 1 && ladder > 8) continue; // 8 inner ladders
    if (layer == 2 && ladder > 12) continue; // 12 outer ladders
    m_id_to_inx[i] = j; // i = id , j - index
    m_inx_to_id[j] = i;
    j++;
    if (j == NUM_MODULES) break;
  }
}


DQMHistAnalysisPXDCMModule::~DQMHistAnalysisPXDCMModule() { }

void DQMHistAnalysisPXDCMModule::initialize()
{
  B2DEBUG(1, "DQMHistAnalysisPXDCM: initialized.");

  m_cCommonMode = new TCanvas("c_CommonMode");
  m_hCommonMode = new TH2F("CommonMode", "CommonMode", 40, 0, 40, 64, 0, 64);
  m_hCommonMode->SetDirectory(0);// dont mess with it, this is MY histogram
  m_hCommonMode->SetStats(false);
}


void DQMHistAnalysisPXDCMModule::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisPXDCM: beginRun called.");

  m_cCommonMode->Clear();
  m_cCommonMode->SetLogz();
}


TH1* DQMHistAnalysisPXDCMModule::findHistLocal(TString& a)
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

void DQMHistAnalysisPXDCMModule::event()
{
  if (!m_cCommonMode) return;
  m_hCommonMode->Reset(); // dont sum up!!!

  // search for hist is missing in this example look at Fitter code
  for (auto i = 0; i < NUM_MODULES; i++) {
    auto id = m_inx_to_id[i];
    auto layer = (((id >> 5) & 0x1) + 1);
    auto ladder = ((id >> 1) & 0xF);
    auto sensor = ((id & 0x1) + 1);

    TH1* hh1 = NULL;
    string s2 = str(format("%d.%d.%d") % layer % ladder % sensor);

    TString a = "PXDDAQCM2_" + s2;
    hh1 = findHist(a.Data());
    if (hh1 == NULL) {
      hh1 = findHistLocal(a);
    }
    if (hh1 == NULL) {
      a = m_histodir + "/PXDDAQCM2_" + s2;
      hh1 = findHist(a.Data());
    }
    if (hh1 == NULL) {
      a = m_histodir + "/PXDDAQCM2_" + s2;
      hh1 = findHistLocal(a);
    }
    if (hh1) {
      for (int bin = 0; bin < 64; bin++) {
        float v;
        v = hh1->GetBinContent(bin);
        m_hCommonMode->Fill(i, v);
      }
    }
  }
  m_cCommonMode->cd();
  if (m_hCommonMode) m_hCommonMode->Draw("col");
  m_cCommonMode->Modified();
  m_cCommonMode->Update();
}

void DQMHistAnalysisPXDCMModule::endRun()
{
  B2DEBUG(1, "DQMHistAnalysisPXDCM : endRun called");
}


void DQMHistAnalysisPXDCMModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisPXDCM: terminate called");

  // should delete canvas here, maybe hist, too? Who owns it?
}

