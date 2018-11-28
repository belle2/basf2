//+
// File : DQMHistAnalysisSVDGeneral.cc
// Description :
//
// Author : Giulia Casarosa
// Date : 20181127
//-


#include <dqm/analysis/modules/DQMHistAnalysisSVDGeneral.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisSVDGeneral)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisSVDGeneralModule::DQMHistAnalysisSVDGeneralModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  B2DEBUG(20, "DQMHistAnalysisSVDGeneral: Constructor done.");
}


DQMHistAnalysisSVDGeneralModule::~DQMHistAnalysisSVDGeneralModule() { }

void DQMHistAnalysisSVDGeneralModule::initialize()
{
  B2DEBUG(20, "DQMHistAnalysisSVDGeneral: initialized.");

  gROOT->cd();
  m_cUnpacker = new TCanvas("c_SVDDataFormat");
}


void DQMHistAnalysisSVDGeneralModule::beginRun()
{
  //B2DEBUG(20, "DQMHistAnalysisSVDGeneral: beginRun called.");
  m_cUnpacker->Clear();
}

void DQMHistAnalysisSVDGeneralModule::event()
{

  //check DATA FORMAT
  TH1* h = findHist("SVDUnpacker/DQMUnpackerHisto");
  if (h != NULL) {
    m_cUnpacker->cd();

    //no entries mean no error
    if (h->GetEntries() == 0)
      m_cUnpacker->Pad()->SetFillColor(kGreen);
    else
      m_cUnpacker->Pad()->SetFillColor(kRed);
    h->Draw();
  } else {
    B2DEBUG(20, "Histogram SVDUnpacker/DQMUnpackerHisto from SVDUnpackedDQM not found!");
    m_cUnpacker->Pad()->SetFillColor(kRed);
  }
  m_cUnpacker->Modified();
  m_cUnpacker->Update();

  //  m_cUnpacker->Print("c_SVDDataFormat.pdf");
}

void DQMHistAnalysisSVDGeneralModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisSVDGeneral:  endRun called");
}


void DQMHistAnalysisSVDGeneralModule::terminate()
{
  B2DEBUG(20, "DQMHistAnalysisSVDGeneral: terminate called");
}

