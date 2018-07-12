//+
// File : DQMHistAnalysisExampleFlags.cc
// Description : Example code, creates a new "flag" histo in DQM analysis
//
// Author : Bjoern Spruck, Univerisity Mainz
// Date : 2017
//-


#include <dqm/analysis/modules/DQMHistAnalysisExampleFlags.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TClass.h>

using namespace std;
using namespace Belle2;


using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisExampleFlags)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisExampleFlagsModule::DQMHistAnalysisExampleFlagsModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("HistoName", m_histoname, "Name of Histogram (incl dir)", std::string(""));
  B2DEBUG(20, "DQMHistAnalysisExampleFlags: Constructor done.");
}


DQMHistAnalysisExampleFlagsModule::~DQMHistAnalysisExampleFlagsModule() { }

void DQMHistAnalysisExampleFlagsModule::initialize()
{
  B2DEBUG(20, "DQMHistAnalysisExampleFlags: initialized.");

  gROOT->cd(); // this seems to be important, or strange things happen
  m_cFlagtest = new TCanvas("c_Flagtest");
  m_hFlagtest = new TH2F("Flagtest", "Flagtest;xxx;yyy", 12, 0, 12, 4, 0, 4);
}


void DQMHistAnalysisExampleFlagsModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisExampleFlags: beginRun called.");

  m_cFlagtest->Clear();
  m_hFlagtest->Reset();
}


TH1* DQMHistAnalysisExampleFlagsModule::findHistLocal(TString& a)
{
  B2DEBUG(20, "Histo " << a << " not in memfile");
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
      B2DEBUG(20, "Histo " << a << " found in mem");
      return (TH1*)obj;
    }
  } else {
    B2DEBUG(20, "Histo " << a << " NOT found in mem");
  }
  return NULL;
}

void DQMHistAnalysisExampleFlagsModule::event()
{
  m_hFlagtest->Reset(); // dont sum up!!!
  static int nr = 0;
  m_hFlagtest->Fill(nr + 0., 0., -2.);
  m_hFlagtest->Fill(nr + 1., 1., -1.);
  m_hFlagtest->Fill(nr + 2., 2., +1.);
  m_hFlagtest->Fill(nr + 3., 3., +2.);
  nr = (nr + 1) % 4;

  // search for hist is missing in this example look at Fitter code

  // doesnt change
  const Int_t colNum = 5;
  Int_t palette[colNum] {13 /* (dark) grey*/, 0 /* White*/ , 3 /* Green */, 5 /* Yellow */, 2 /* Red */};

  // Guess I have to set this for every update
  gStyle->SetPalette(5, palette);

  // Nice try, but jsroot is not using the palette stored in canvas
  // FIXED in jsroot 5.3.0 ... but this is not in externals yet
  // needs root 6.12 ... until then no color palette is saved
  if (m_cFlagtest) {
    m_cFlagtest->cd();
    m_cFlagtest->Clear();
    if (m_hFlagtest) {
      m_hFlagtest->Draw("colz"); // ,PFC,PLC,PMC

    }
    m_cFlagtest->Modified();
    m_cFlagtest->Update();
  }
}

void DQMHistAnalysisExampleFlagsModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisExampleFlags : endRun called");
}


void DQMHistAnalysisExampleFlagsModule::terminate()
{
  B2DEBUG(20, "DQMHistAnalysisExampleFlags: terminate called");

  // should delete canvas here, maybe hist, too? Who owns it?
}

