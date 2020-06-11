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
#include <TCanvas.h>
#include <TLine.h>
#include <iostream>
#include <fstream>

using namespace Belle2;


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDedxRunGainAlgorithm::CDCDedxRunGainAlgorithm() :
  CalibrationAlgorithm("CDCDedxElectronCollector"),
  isMakePlots(true),
  isMergePayload(true)
{
  // Set module properties
  setDescription("A calibration algorithm for CDC dE/dx run gains");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------

CalibrationAlgorithm::EResult CDCDedxRunGainAlgorithm::calibrate()
{

  const auto expRun = getRunList()[0];
  updateDBObjPtrs(1, expRun.second, expRun.first);
  double ExistingRG = m_DBRunGain->getRunGain();

  // Get data objects
  auto ttree = getObjectPtr<TTree>("tree");

  double dedx;
  int run = 0, runtemp = 0;
  ttree->SetBranchAddress("dedx", &dedx);
  ttree->SetBranchAddress("run", &run);

  TH1D* hDedx = new TH1D("hDedx", "hDedx;dEdx;entries", 400, 0, 2.0);
  TH1D* hDedxAbs = new TH1D("hDedxAbs", "hDedxAbs;dEdx;entries", 400, 0, 2.0);
  for (int i = 0; i < ttree->GetEntries(); ++i) {
    ttree->GetEvent(i);
    if (dedx <= 0) continue;
    hDedx->Fill(dedx);
    hDedxAbs->Fill(dedx * ExistingRG);
    runtemp = run;
  }

  //Fit relative run
  TString status = "";
  double RunGainConst = 1.0, RunGainConstErr = 0.0;
  FitGaussianWRange(hDedx, status);
  if (status == "LowStats") {
    B2INFO("Not enough data for this run: adding next run data");
    return c_NotEnoughData;
  } else if (status == "FitFailed") {
    B2INFO("Fit failed for this run: setting gain 1.0");
    RunGainConst = 1.0;
  } else {
    RunGainConst = hDedx->GetFunction("gaus")->GetParameter(1);
    RunGainConstErr = hDedx->GetFunction("gaus")->GetParError(1);
    B2INFO("Fit OK for this run: (run = " << expRun.second << ", gain = " << RunGainConst << ")");
    hDedx->SetTitle(Form("dE/dx rel, Run %d, Fit %s, Fit-mean %0.03f#pm%0.03f", expRun.second, status.Data(), RunGainConst,
                         RunGainConstErr));
  }


  //Fit absolute run
  status = "";
  FitGaussianWRange(hDedxAbs, status);
  if (status != "FitOK") RunGainAbs = 1.0;
  else {
    RunGainAbs = hDedxAbs->GetFunction("gaus")->GetParameter(1);
    Double_t RunGainAbsErr = hDedxAbs->GetFunction("gaus")->GetParError(1);
    hDedxAbs->SetTitle(Form("dE/dx abs, Run %d, Fit %s, Fit-mean %0.03f#pm%0.03f", expRun.second, status.Data(), RunGainAbs,
                            RunGainAbsErr));
  }


  if (isMakePlots) {

    std::ofstream fileRGout;
    fileRGout.open(Form("fRG_Constants_Rel_%d.txt", expRun.second));
    fileRGout << expRun.second << " " << RunGainConst << " " << RunGainConstErr << "\n";
    fileRGout.close();

    //Making support histograms
    TCanvas* ctmp = new TCanvas("tmp", "tmp", 1000, 500);
    ctmp->SetBatch(kTRUE);
    ctmp->Divide(2, 1);

    ctmp->cd(1);
    hDedx->SetName(Form("hDedxRel_run%d_%s", expRun.second, status.Data()));
    hDedx->SetFillColorAlpha(kYellow, 0.20);
    hDedx->DrawCopy("");

    TLine* tl = new TLine();
    tl->SetLineColor(kRed);
    tl->SetX1(RunGainConst); tl->SetX2(RunGainConst);
    tl->SetY1(0); tl->SetY2(hDedx->GetMaximum());
    tl->DrawClone("same");

    // ctmp->cd(2);
    ctmp->cd(2);
    hDedxAbs->SetName(Form("hDedxAbs_run%d_%s", expRun.second, status.Data()));
    hDedxAbs->SetFillColorAlpha(kBlue, 0.10);
    hDedxAbs->DrawCopy("");

    TLine* tlAbs = new TLine();
    tlAbs->SetLineColor(kRed);
    tlAbs->SetX1(RunGainAbs); tlAbs->SetX2(RunGainAbs);
    tlAbs->SetY1(0); tlAbs->SetY2(hDedxAbs->GetMaximum());
    tlAbs->DrawClone("same");

    ctmp->Print(Form("dedx_rungains_%d_wSTATUS_%s.pdf", expRun.second, status.Data()));

    delete hDedx;
    delete hDedxAbs;
    delete ctmp;
    delete tl;
    delete tlAbs;
  }


  B2INFO("dE/dx run gain done: " << RunGainConst);
  printf("run = %d, const = %0.05f, status = %s\n", runtemp, RunGainConst, status.Data());
  B2INFO("Saving new rung for (Exp, Run) : (" << expRun.first << "," << expRun.second << ")");
  generateNewPayloads(RunGainConst, ExistingRG);

  return c_OK;

}


void CDCDedxRunGainAlgorithm::generateNewPayloads(double RunGainConst, double ExistingRG)
{

  // bool refchange = m_DBRunGain.hasChanged(); //Add this feature for major processing
  B2INFO("--> RunGain: Previous = " << ExistingRG << ", Relative = " << RunGainConst << ", Merged = " << RunGainConst * ExistingRG);
  if (isMergePayload) RunGainConst *= ExistingRG;

  CDCDedxRunGain* gain = new CDCDedxRunGain(RunGainConst);
  saveCalibration(gain, "CDCDedxRunGain");
}


void CDCDedxRunGainAlgorithm::FitGaussianWRange(TH1D*& temphist, TString& status)
{

  if (temphist->Integral() < 250) {
    std::cout << "\t\t" << Form("%s no-FIT as this run have Low Stats < 250", temphist->GetName()) << std::endl;
    status = "LowStats";
    return;
  }

  temphist->GetXaxis()->SetRange(temphist->FindFirstBinAbove(0, 1), temphist->FindLastBinAbove(0, 1));
  int fs = temphist->Fit("gaus", "QR");
  if (fs != 0) {
    std::cout << "\t\t" << Form("%s FIT (round 1) STATUS Failed ", temphist->GetName()) << fs << std::endl;
    status = "FitFailed";
    return;
  }

  double mean = temphist->GetFunction("gaus")->GetParameter(1);
  double width = temphist->GetFunction("gaus")->GetParameter(2);
  temphist->GetXaxis()->SetRangeUser(mean - 4.5 * width, mean + 4.5 * width);
  fs = temphist->Fit("gaus", "QR", "", mean - fSigLim * width, mean + fSigLim * width);
  if (fs != 0) {
    std::cout << "\t\t" << Form("%s FIT (round 2) STATUS Failed ", temphist->GetName()) << fs << std::endl;
    status = "FitFailed";
    return;
  } else {
    status = "FitOK";
    std::cout << "\t\t" << Form("%s FIT STATUS OK ", temphist->GetName()) << fs << std::endl;
    temphist->GetXaxis()->SetRangeUser(mean - 4.5 * width, mean + 4.5 * width);
  }
}
