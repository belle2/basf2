/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jikumar, jvbennett                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/calibration/CDCDedxRunGainAlgorithm.h>

#include <TF1.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TPaveText.h>



using namespace Belle2;


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDedxRunGainAlgorithm::CDCDedxRunGainAlgorithm() :
  CalibrationAlgorithm("CDCDedxElectronCollector"),
  m_badRunFPath(""),
  m_badRunFName(""),
  isRmBadruns(false),
  isMakePlots(true)
{
  // Set module properties
  setDescription("A calibration algorithm for CDC dE/dx run gains");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------

CalibrationAlgorithm::EResult CDCDedxRunGainAlgorithm::calibrate()
{

  // Get data objects
  auto ttree = getObjectPtr<TTree>("tree");

  double dedx;
  int run = 0, runtemp = 0;
  ttree->SetBranchAddress("dedx", &dedx);
  ttree->SetBranchAddress("run", &run);

  TH1D* hDedx = new TH1D("hDedx", "hDedx", 200, 0, 2);
  for (int i = 0; i < ttree->GetEntries(); ++i) {
    ttree->GetEvent(i);
    if (dedx == 0) continue;
    hDedx->Fill(dedx);
    runtemp = run;
  }

  bool IsSkipRun = false;
  double rmean = 1.0;
  if (isRmBadruns)CheckRunStatus(runtemp, IsSkipRun, rmean);

  TF1* fitG = new TF1("fitG", "gaus", 0.5, 1.5);
  fitG->SetParLimits(1, 0.90, 1.10);
  fitG->SetParLimits(2, 0.03, 0.12);

  double RunGainConst = 1.0;
  std::string rstatus = "";
  if (IsSkipRun) {
    RunGainConst = rmean;
    rstatus = "BadRun";
  } else {
    if (hDedx->GetEntries() < 100) {
      RunGainConst = 1.0; //try global running avg accorsing all runs: next
      rstatus = "LowStatsRun";
    } else {
      if (isMakePlots)hDedx->Fit("fitG");
      else if (!isMakePlots)hDedx->Fit("fitG", "QRM"); //silent fitting
      if (!hDedx->GetFunction("fitG")->IsValid()) {
        RunGainConst = 1.0;
        rstatus = "FitFailedRun";
      } else {
        RunGainConst = hDedx->GetFunction("fitG")->GetParameter(1);
        rstatus = "GoodRun";
      }
    }
  }


  if (isMakePlots) {
    //Making support histograms
    TCanvas* ctmp = new TCanvas("tmp", "tmp", 500, 500);
    ctmp->SetBatch(kTRUE);
    ctmp->cd();

    hDedx->SetName(Form("hDedx_run%d_%s", runtemp, rstatus.data()));
    hDedx->SetTitle(Form("run = %d, status: %s, const: %0.04f", runtemp, rstatus.data(), RunGainConst));
    hDedx->SetFillColor(kYellow);
    hDedx->DrawCopy("hist");
    fitG->DrawCopy("same");


    TLine* tl = new TLine();
    tl->SetLineColor(kRed);
    tl->SetX1(RunGainConst); tl->SetX2(RunGainConst);
    tl->SetY1(0); tl->SetY2(hDedx->GetMaximum());
    tl->DrawClone("same");

    ctmp->Print(Form("dedx_rungains_%d_wSTATUS_%s.pdf", runtemp, rstatus.data()));

    delete hDedx;
    delete ctmp;
    delete tl;
  }

  B2INFO("dE/dx run gains done: " << RunGainConst);
  printf("run = %d, const = %0.03f, status = %s\n", runtemp, RunGainConst, rstatus.data());
  CDCDedxRunGain* gain = new CDCDedxRunGain(RunGainConst);
  saveCalibration(gain, "CDCDedxRunGain");

  return c_OK;

}


