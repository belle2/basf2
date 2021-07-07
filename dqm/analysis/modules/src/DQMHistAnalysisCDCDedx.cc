/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <dqm/analysis/modules/DQMHistAnalysisCDCDedx.h>
#include <boost/format.hpp>
#include <TROOT.h>

#include <TF1.h>
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

  c_CDCdedxSigma = new TCanvas("CDCDedx/c_CDCdedxSigma");

  c_CDCdedxMean = new TCanvas("CDCDedx/c_CDCdedxMean");
  h_CDCdedxMean = new TH1F("CDCDedx/h_CDCdedxMean", "dEdx distribution", 200, 0.0, 2.0);
  h_CDCdedxMean->SetDirectory(0);
  h_CDCdedxMean->SetStats(false);

  f_fGaus = new TF1("f_Gaus", "gaus", 0.0, 2.0);
  f_fGaus->SetParameter(1, 1.00);
  f_fGaus->SetParameter(2, 0.06);
  f_fGaus->SetLineColor(kRed);
}


void DQMHistAnalysisCDCDedxModule::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisCDCDedx: beginRun called.");
}

void DQMHistAnalysisCDCDedxModule::event()
{
  computedEdxMeanSigma();
}


void DQMHistAnalysisCDCDedxModule::computedEdxMeanSigma()
{

  runstatus.clear();
  tLine->Clear();

  TH1* hh1 = findHist("CDCDedx/hdEdx_PerRun");
  if (hh1 != NULL) {
    c_CDCdedxMean->Clear();
    c_CDCdedxMean->cd();
    //Copy bins and X ranges as well as bin-content (Clone and Copy histogram not working here)
    h_CDCdedxMean->SetBins(int(hh1->GetXaxis()->GetNbins()), double(hh1->GetXaxis()->GetXmin()), double(hh1->GetXaxis()->GetXmax()));
    h_CDCdedxMean->SetTitle(hh1->GetTitle());
    for (Int_t i = 1; i <= hh1->GetXaxis()->GetNbins(); i++) {
      h_CDCdedxMean->SetBinContent(i, hh1->GetBinContent(i));
      h_CDCdedxMean->SetBinError(i, hh1->GetBinError(i));
    }

    h_CDCdedxMean->Fit(f_fGaus, "Q");

    runnumber = h_CDCdedxMean->GetTitle();
    runnumber = runnumber.substr(12, 5);

    if (h_CDCdedxMean->GetEntries() < 100) {
      dedxmean = 0.0; dedxsigma = 0.0;
      runstatus = "Low Stats";
    } else {
      if (h_CDCdedxMean->GetFunction("f_Gaus") == NULL || !h_CDCdedxMean->GetFunction("f_Gaus")->IsValid()) {
        dedxmean = 0.0; dedxsigma = 0.0;
        runstatus = "Fit Failed";
      } else {
        dedxmean = h_CDCdedxMean->GetFunction("f_Gaus")->GetParameter(1);
        dedxsigma = h_CDCdedxMean->GetFunction("f_Gaus")->GetParameter(2);
        runstatus = "Good";
      }
    }

    h_CDCdedxMean->GetXaxis()->SetRangeUser(0.0, 2.0);
    h_CDCdedxMean->SetTitle(Form("Run #: %s, Status: %s, mean: %0.04f, sigma: %0.04f", runnumber.data(), runstatus.data(), dedxmean,
                                 dedxsigma));
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

