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
#include <TH1I.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
CDCDedxRunGainAlgorithm::CDCDedxRunGainAlgorithm() :
  CalibrationAlgorithm("CDCDedxElectronCollector"),
  isMakePlots(true),
  isMergePayload(true),
  fsrun(""),
  fSigLim(2.5),
  fdEdxBins(500),
  fdEdxMin(0.0),
  fdEdxMax(2.5),
  fAdjust(1.00)
{
  // Set module properties
  setDescription("A calibration algorithm for CDC dE/dx run gains");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------
CalibrationAlgorithm::EResult CDCDedxRunGainAlgorithm::calibrate()
{
  B2INFO("Preparing dE/dx calibration for CDC dE/dx run gain");

  const auto expRun = getRunList()[0];
  updateDBObjPtrs(1, expRun.second, expRun.first);
  double ExistingRG = m_DBRunGain->getRunGain();

  // Get data objects
  auto ttree = getObjectPtr<TTree>("tree");
  if (!ttree)return c_NotEnoughData;

  double dedx;
  int run = 0, runtemp = 0;
  TString status = "";
  ttree->SetBranchAddress("dedx", &dedx);
  ttree->SetBranchAddress("run", &run);

  TH1D* hDedx = new TH1D("hDedx", "hDedx;dEdx rel (no hadsat);entries", fdEdxBins, fdEdxMin, fdEdxMax);
  TH1D* hDedxAbs = new TH1D("hDedxAbs", "hDedxAbs;dEdx abs(no hadsat);entries", fdEdxBins, fdEdxMin, fdEdxMax);

  for (int i = 0; i < ttree->GetEntries(); ++i) {
    ttree->GetEvent(i);
    if (dedx <= 0) continue;
    hDedx->Fill(dedx);
    hDedxAbs->Fill(dedx * ExistingRG);
    runtemp = run;
  }

  //min 500 good events otherwise merge to next run
  if (hDedx->Integral() < 1000) {
    B2INFO("Not enough data for run (" << runtemp << ") so far only " << hDedx->Integral() << " therefore adding next run");
    fsrun += Form("%d_", runtemp);
    delete hDedx;
    delete hDedxAbs;
    return c_NotEnoughData;
  }

  B2INFO("Total track for this run: " << runtemp << " = " << hDedx->Integral());
  fsrun += Form("%d", runtemp);
  hDedx->SetName(Form("%s_%s", hDedx->GetName(), fsrun.Data()));
  hDedxAbs->SetName(Form("%s_%s", hDedxAbs->GetName(), fsrun.Data()));

  //Fit absolute run for the reference only
  double RGabs_Mean = 1.0;
  FitGaussianWRange(hDedxAbs, status);
  if (status != "FitOK") {
    hDedxAbs->SetTitle(Form("dE/dx abs, Run %s (%s)", fsrun.Data(), status.Data()));
  } else {
    RGabs_Mean = hDedxAbs->GetFunction("gaus")->GetParameter(1);
    double RGabs_MeanErr = hDedxAbs->GetFunction("gaus")->GetParError(1);
    double RGabs_Sigma = hDedxAbs->GetFunction("gaus")->GetParameter(2);
    hDedxAbs->SetTitle(Form("dE/dx abs, Run %s (%s), #mu_{fit}: %0.04f#pm%0.04f,, #sigma_{fit}: %0.04f", fsrun.Data(), status.Data(),
                            RGabs_Mean, RGabs_MeanErr, RGabs_Sigma));
  }

  //Fit relative run for actual calibration
  double RGrel_Mean = 1.0, RGrel_MeanErr = -99.0, RGrel_Sigma = -99.0;
  FitGaussianWRange(hDedx, status);
  if (status != "FitOK") {
    RGrel_Mean = 1.0; //No cal result and setting manually to 1.0
    hDedx->SetTitle(Form("dE/dx abs, Run %s (%s)", fsrun.Data(), status.Data()));
  } else {
    RGrel_Mean = hDedx->GetFunction("gaus")->GetParameter(1);
    RGrel_MeanErr = hDedx->GetFunction("gaus")->GetParError(1);
    RGrel_Sigma = hDedx->GetFunction("gaus")->GetParameter(2);
    hDedx->SetTitle(Form("dE/dx rel, Run %s (%s), #mu_{fit}: %0.04f#pm%0.04f,, #sigma_{fit}: %0.04f", fsrun.Data(), status.Data(),
                         RGrel_Mean, RGrel_MeanErr, RGrel_Sigma));
  }

  if (RGrel_Mean <= 0.0)RGrel_Mean = 1.0; //safe button

  //for validation purpose
  if (isMakePlots) {

    std::ofstream fileRGout;
    fileRGout.open(Form("fRG_Constants_%s.txt", fsrun.Data()));
    fileRGout << expRun.second << " " << ExistingRG << " " << RGrel_Mean << " " << RGrel_MeanErr << " " << RGrel_Sigma << "\n";
    fileRGout.close();

    TCanvas* ctmp = new TCanvas("tmp", "tmp", 1000, 500);
    ctmp->SetBatch(kTRUE);
    ctmp->Divide(2, 1);

    ctmp->cd(1);
    hDedx->SetFillColorAlpha(kYellow, 0.30);
    hDedx->DrawCopy("");

    TLine* tl = new TLine();
    tl->SetLineColor(kRed);
    tl->SetX1(RGrel_Mean); tl->SetX2(RGrel_Mean);
    tl->SetY1(0); tl->SetY2(hDedx->GetMaximum());
    tl->DrawClone("same");

    ctmp->cd(2);
    hDedxAbs->SetFillColorAlpha(kBlue, 0.10);
    hDedxAbs->DrawCopy("");

    TLine* tlAbs = new TLine();
    tlAbs->SetLineColor(kRed);
    tlAbs->SetX1(RGabs_Mean); tlAbs->SetX2(RGabs_Mean);
    tlAbs->SetY1(0); tlAbs->SetY2(hDedxAbs->GetMaximum());
    tlAbs->DrawClone("same");

    ctmp->Print(Form("cdcdedx_rungain_fits_%s_%s.pdf", fsrun.Data(), status.Data()));

    TCanvas* cstats = new TCanvas("cstats", "cstats", 1000, 500);
    cstats->SetBatch(kTRUE);
    cstats->Divide(2, 1);
    cstats->cd(1);
    auto hestats = getObjectPtr<TH1I>("hestats");
    if (hestats)hestats->DrawCopy("");
    cstats->cd(2);
    auto htstats = getObjectPtr<TH1I>("htstats");
    if (htstats)htstats->DrawCopy("");
    cstats->Print(Form("cdcdedx_rungain_stats_%s.pdf", fsrun.Data()));

    delete ctmp;
    delete tl;
    delete tlAbs;
    delete cstats;
  }

  B2INFO("Saving new rung for (Exp, Run) : (" << expRun.first << "," << expRun.second << ")");
  generateNewPayloads(RGrel_Mean, ExistingRG);

  fsrun.Resize(0);
  delete hDedx;
  delete hDedxAbs;
  return c_OK;
}


void CDCDedxRunGainAlgorithm::generateNewPayloads(double RGrel_Mean, double ExistingRG)
{
  if (isMergePayload) {
    B2INFO("--> RunGain: Previous = " << ExistingRG << ", Relative = " << RGrel_Mean << ", Adjustment = " << fAdjust <<
           ", Merged (saved) = " << RGrel_Mean *
           ExistingRG * fAdjust);
    RGrel_Mean *= ExistingRG;
    RGrel_Mean *= fAdjust; //default is 1.0
  } else {
    B2INFO("--> RunGain: Previous = " << ExistingRG << ", Relative (saved) = " << RGrel_Mean);
  }

  CDCDedxRunGain* gain = new CDCDedxRunGain(RGrel_Mean);
  saveCalibration(gain, "CDCDedxRunGain");
}


void CDCDedxRunGainAlgorithm::FitGaussianWRange(TH1D*& temphist, TString& status)
{
  temphist->GetXaxis()->SetRange(temphist->FindFirstBinAbove(0, 1), temphist->FindLastBinAbove(0, 1));
  int fs = temphist->Fit("gaus", "QR");
  if (fs != 0) {
    B2INFO(Form("\tFit (round 1) for hist (%s) failed (status = %d)", temphist->GetName(), fs));
    status = "FitFailed";
    return;
  } else {
    double mean = temphist->GetFunction("gaus")->GetParameter(1);
    double width = temphist->GetFunction("gaus")->GetParameter(2);
    temphist->GetXaxis()->SetRangeUser(mean - 5.0 * width, mean + 5.0 * width);
    fs = temphist->Fit("gaus", "QR", "", mean - fSigLim * width, mean + fSigLim * width);
    if (fs != 0) {
      B2INFO(Form("\tFit (round 2) for hist (%s) failed (status = %d)", temphist->GetName(), fs));
      status = "FitFailed";
      return;
    } else {
      temphist->GetXaxis()->SetRangeUser(mean - 5.0 * width, mean + 5.0 * width);
      B2INFO(Form("\tFit for hist (%s) sucessfull (status = %d)", temphist->GetName(), fs));
      status = "FitOK";
    }
  }
}