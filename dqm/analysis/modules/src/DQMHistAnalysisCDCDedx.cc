/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jitendra Kumar                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <dqm/analysis/modules/DQMHistAnalysisCDCDedx.h>
#include <boost/format.hpp>
#include <TROOT.h>
#include <TClass.h>

#include <TF1.h>
#include <TH2F.h>
#include <TCanvas.h>


using namespace std;
using namespace Belle2;
using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisCDCDedx)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisCDCDedxModule::DQMHistAnalysisCDCDedxModule()
  : DQMHistAnalysisModule(),
    dedxmean(0.0),
    dedxsigma(0.0)
{
  //Parameter definition
  B2DEBUG(20, "DQMHistAnalysisCDCDedx: Constructor done.");
}


DQMHistAnalysisCDCDedxModule::~DQMHistAnalysisCDCDedxModule() { }

void DQMHistAnalysisCDCDedxModule::initialize()
{

  gROOT->cd(); // this seems to be important, or strange things happen

  B2DEBUG(1, "DQMHistAnalysisCDCDedx: initialized.");

  runstatus = "";
  dedxmean = 0.0;
  dedxsigma = 0.0;

  tLine = new TLine();
  tLine->SetLineColor(4);
  tLine->SetLineStyle(9);

  c_CDCdedxSigma = new TCanvas("c_CDCdedxSigma");

  c_CDCdedxMean = new TCanvas("c_CDCdedxMean");
  h_CDCdedxMean = new TH1F("h_CDCdedxMean", "dEdx distribution", 200, 0.0, 2.0);
  h_CDCdedxMean->SetDirectory(0);// dont mess with it, this is MY histogram
  h_CDCdedxMean->SetStats(false);

  f_fGaus = new TF1("f_Gaus", "gaus", 0.0, 2.0);
  f_fGaus->SetParameter(1, 1.00);
  f_fGaus->SetParameter(2, 0.06);
  f_fGaus->SetLineColor(kRed);
  // f_fGaus->SetNpx(256);
  // f_fGaus->SetNumberFitPoints(256);

}


void DQMHistAnalysisCDCDedxModule::beginRun()
{
  h_CDCdedxMean->Clear();
  B2DEBUG(1, "DQMHistAnalysisCDCDedx: beginRun called.");
}

void DQMHistAnalysisCDCDedxModule::event()
{
  computedEdxMeanSigma();
}


void DQMHistAnalysisCDCDedxModule::computedEdxMeanSigma()
{
  h_CDCdedxMean->Reset(); // dont sum up!!!
  runstatus.clear();
  tLine->Clear();

  TH1* hh1 = findHist("CDCDedx/hdEdx_PerRun");
  if (hh1 != NULL) {

    h_CDCdedxMean = (TH1F*)hh1->Clone("hdEdx_PerRun");
    h_CDCdedxMean->Fit(f_fGaus, "Q");

    runnumber = h_CDCdedxMean->GetTitle();
    runnumber = runnumber.substr(12, 5);

    if (h_CDCdedxMean->GetEntries() < 100) {
      dedxmean = 0.0; dedxsigma = 0.0;
      runstatus = "Low Stats";
    } else {
      if (!h_CDCdedxMean->GetFunction("f_Gaus")->IsValid()) {
        dedxmean = 0.0; dedxsigma = 0.0;
        runstatus = "Fit Failed";
      } else {
        dedxmean = h_CDCdedxMean->GetFunction("f_Gaus")->GetParameter(1);
        dedxsigma = h_CDCdedxMean->GetFunction("f_Gaus")->GetParameter(2);
        runstatus = "Good";
      }
    }

    c_CDCdedxMean->Clear();
    c_CDCdedxMean->cd();
    h_CDCdedxMean->GetXaxis()->SetRangeUser(0.0, 2.0);
    h_CDCdedxMean->SetTitle(Form("Run #: %s, Status: %s, dEdx mean: %0.04f", runnumber.data(), runstatus.data(), dedxmean));
    h_CDCdedxMean->SetFillColor(kYellow);
    h_CDCdedxMean->Draw("hist");

    f_fGaus->Draw("same");

    tLine->SetX1(dedxmean);
    tLine->SetX2(dedxmean);
    tLine->SetY1(0);
    tLine->SetY2(h_CDCdedxMean->GetMaximum());
    tLine->DrawClone("same");

    c_CDCdedxMean->Modified();
  }
}


void DQMHistAnalysisCDCDedxModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisCDCDedx : endRun called");
}


void DQMHistAnalysisCDCDedxModule::terminate()
{
  B2DEBUG(20, "terminate called");
}

