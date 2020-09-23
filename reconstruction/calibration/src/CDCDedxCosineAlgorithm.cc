/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jikumar, jvbennett                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/calibration/CDCDedxCosineAlgorithm.h>

#include <TF1.h>
#include <TLine.h>
#include <TCanvas.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
CDCDedxCosineAlgorithm::CDCDedxCosineAlgorithm() :
  CalibrationAlgorithm("CDCDedxElectronCollector"),
  isMethodSep(true),
  isMakePlots(true),
  isMergePayload(true),
  fSigLim(2.5),
  fCosbins(100),
  fCosMin(-1.0),
  fCosMax(1.0),
  fHistbins(600),
  fdEdxMin(0.0),
  fdEdxMax(3.0)
{
  // Set module properties
  setDescription("A calibration algorithm for CDC dE/dx _eltron cos(theta) dependence");

}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------
CalibrationAlgorithm::EResult CDCDedxCosineAlgorithm::calibrate()
{
  B2INFO("Preparing dE/dx calibration for CDC dE/dx _eltron saturation");

  // Get data objects
  auto ttree = getObjectPtr<TTree>("tree");
  if (ttree->GetEntries() < 100)return c_NotEnoughData;

  double dedx, costh; int charge;
  ttree->SetBranchAddress("dedx", &dedx);
  ttree->SetBranchAddress("costh", &costh);
  ttree->SetBranchAddress("charge", &charge);

  // make histograms to store dE/dx values in bins of cos(theta)
  // bin size can be arbitrary, but for now just make uniform bins
  TH1D* hdEdx_elCosbin[fCosbins], *hdEdx_poCosbin[fCosbins], *hdEdx_epCosbin[fCosbins];
  const double binW = (fCosMax - fCosMin) / fCosbins;

  for (unsigned int i = 0; i < fCosbins; ++i) {

    double coslow = i * binW,  coshigh = (i + 1) * binW;

    hdEdx_elCosbin[i] = new TH1D(Form("hdEdx_elCosbin%d", i), "", fHistbins, fdEdxMin, fdEdxMax);
    hdEdx_elCosbin[i]->SetTitle(Form("dE/dx dist (e-) in costh (%0.02f, %0.02f)", coslow, coshigh));
    hdEdx_elCosbin[i]->GetXaxis()->SetTitle("dE/dx (no had sat, for e-)");
    hdEdx_elCosbin[i]->GetYaxis()->SetTitle("Entries");

    hdEdx_poCosbin[i] = new TH1D(Form("hdEdx_poCosbin%d", i), "", fHistbins, fdEdxMin, fdEdxMax);
    hdEdx_poCosbin[i]->SetTitle(Form("dE/dx dist (e+) in costh (%0.02f, %0.02f)", coslow, coshigh));
    hdEdx_poCosbin[i]->GetXaxis()->SetTitle("dE/dx (no had sat, for e+)");
    hdEdx_poCosbin[i]->GetYaxis()->SetTitle("Entries");

    hdEdx_epCosbin[i] = new TH1D(Form("hdEdx_epCosbin%d", i), "", fHistbins, fdEdxMin, fdEdxMax);
    hdEdx_epCosbin[i]->SetTitle(Form("dE/dx dist (e-,e+) in costh (%0.02f, %0.02f)", coslow, coshigh));
    hdEdx_epCosbin[i]->GetXaxis()->SetTitle("dE/dx (no had sat, for e-,e+)");
    hdEdx_epCosbin[i]->GetYaxis()->SetTitle("Entries");
  }

  // fill histograms, bin size may be arbitrary
  TH1D* hCosth_el = new TH1D("hCosth_el", "cos(#theta) dist (e- and e+); cos(#theta); Entries", fCosbins, fCosMin, fCosMax);
  TH1D* hCosth_po = new TH1D("hCosth_po", "cos(#theta) dist (e+); cos(#theta); Entries", fCosbins, fCosMin, fCosMax);
  TH1D* hCosth_ep = new TH1D("hCosth_ep", "cos(#theta) dist (e- and e+); cos(#theta); Entries", fCosbins, fCosMin, fCosMax);

  for (int i = 0; i < ttree->GetEntries(); ++i) {

    ttree->GetEvent(i);

    //if track is junk
    if (dedx <= 0 || charge == 0) continue;

    //if track is in CDC accpetance
    if (costh < TMath::Cos(150 * TMath::DegToRad()) || costh > TMath::Cos(17 * TMath::DegToRad())) continue;

    int bin = int((costh - fCosMin) / binW);
    if (bin < 0 || bin >= int(fCosbins)) continue;

    if (isMethodSep) {
      if (charge < 0) {
        hCosth_el->Fill(costh);
        hdEdx_elCosbin[bin]->Fill(dedx);
      } else if (charge > 0) {
        hCosth_po->Fill(costh);
        hdEdx_poCosbin[bin]->Fill(dedx);
      }
    } else {
      hCosth_ep->Fill(costh);
      hdEdx_epCosbin[bin]->Fill(dedx);
    }
  }

  //Plot constants
  TH1D* hdEdxMeanvsCos_po = new TH1D("hdEdxMeanvsCos_po", "dE/dx(e+) rel means; cos(#theta); dE/dx (#mu_{fit})", fCosbins, fCosMin,
                                     fCosMax);
  TH1D* hdEdxSigmavsCos_po = new TH1D("hdEdxSigmavsCos_po", "dE/dx(e+) rel means; cos(#theta); dE/dx (#mu_{fit})", fCosbins, fCosMin,
                                      fCosMax);

  TH1D* hdEdxMeanvsCos_el = new TH1D("hdEdxMeanvsCos_el", "dE/dx(e-) rel means; cos(#theta); dE/dx (#mu_{fit})", fCosbins, fCosMin,
                                     fCosMax);
  TH1D* hdEdxSigmavsCos_el = new TH1D("hdEdxSigmavsCos_el", "dE/dx(e-) rel means; cos(#theta); dE/dx (#mu_{fit})", fCosbins, fCosMin,
                                      fCosMax);

  TH1D* hdEdxMeanvsCos_ep = new TH1D("hdEdxMeanvsCos_ep", "dE/dx(e+, e-) rel means; cos(#theta); dE/dx (#mu_{fit})", fCosbins,
                                     fCosMin, fCosMax);
  TH1D* hdEdxSigmavsCos_ep = new TH1D("hdEdxSigmavsCos_ep", "dE/dx(e+, e-) rel means; cos(#theta); dE/dx (#mu_{fit})", fCosbins,
                                      fCosMin, fCosMax);

  // more validation plots
  TCanvas* ctmp_ep = new TCanvas("ctmp_ep", "ctmp_ep", 800, 400);
  if (isMethodSep)ctmp_ep->Divide(2, 1);
  else {
    ctmp_ep->Divide(2, 2);
    ctmp_ep->SetCanvasSize(800, 800);
  }
  std::stringstream psname_ep;

  //validation plots: individual bin dedx dist and fits
  if (isMakePlots) {
    psname_ep << "cdcdedx_coscal_fits.pdf[";
    ctmp_ep->Print(psname_ep.str().c_str());
    psname_ep.str("");
    psname_ep << "cdcdedx_coscal_fits.pdf";
  }

  // fit histograms to get gains in bins of cos(theta)
  std::vector<double> cosine;
  for (unsigned int i = 0; i < fCosbins; ++i) {

    TString status = "";

    TLine* tl = new TLine();
    tl->SetLineColor(kBlack);

    double fdEdxMean = 1.0; //This is what we need for calibration
    double fdEdxMeanErr = 0.0;

    if (!isMethodSep) {

      double fdEdxSigma = 0.0, fdEdxSigmaErr = 0.0;
      FitGaussianWRange(hdEdx_epCosbin[i], status);

      if (status != "FitOK") {
        fdEdxMean = 1.0;
        hdEdx_epCosbin[i]->SetTitle(Form("%s, Fit(%s)", hdEdx_epCosbin[i]->GetTitle(), status.Data()));
      } else {
        fdEdxMean = hdEdx_epCosbin[i]->GetFunction("gaus")->GetParameter(1);
        fdEdxMeanErr = hdEdx_epCosbin[i]->GetFunction("gaus")->GetParError(1);
        fdEdxSigma = hdEdx_epCosbin[i]->GetFunction("gaus")->GetParameter(2);
        fdEdxSigmaErr = hdEdx_epCosbin[i]->GetFunction("gaus")->GetParError(2);
        hdEdx_epCosbin[i]->SetTitle(Form("%s, Fit (%s), #mu_{fit}: %0.04f#pm%0.04f,, #sigma_{fit}: %0.04f", hdEdx_epCosbin[i]->GetTitle(),
                                         status.Data(), fdEdxMean, fdEdxMeanErr, fdEdxSigma));
      }

      hdEdxMeanvsCos_ep->SetBinContent(i + 1, fdEdxMean);
      hdEdxMeanvsCos_ep->SetBinError(i + 1, fdEdxMeanErr);
      hdEdxSigmavsCos_ep->SetBinContent(i + 1, fdEdxSigma);
      hdEdxSigmavsCos_ep->SetBinError(i + 1, fdEdxSigmaErr);

      if (isMakePlots) {
        ctmp_ep->cd(i % 4 + 1); // each canvas is 2x2
        hdEdx_epCosbin[i]->SetFillColorAlpha(kYellow, 0.25);
        hdEdx_epCosbin[i]->DrawCopy("hist");

        tl->SetX1(fdEdxMean); tl->SetX2(fdEdxMean);
        tl->SetY1(0); tl->SetY2(hdEdx_epCosbin[i]->GetMaximum());
        tl->DrawClone("same");
        if ((i + 1) % 4 == 0 || (i + 1 == fCosbins))ctmp_ep->Print(psname_ep.str().c_str());
      }
    } else {

      double fdEdxMean_el = 1.0, fdEdxMean_elErr = 0.0;
      double fdEdxSigma_el = 0.0, fdEdxSigma_elErr = 0.0;
      double fdEdxMean_po = 1.0, fdEdxMean_poErr = 0.0;
      double fdEdxSigma_po = 0.0, fdEdxSigma_poErr = 0.0;

      //Fit _eltrons in cos bins
      FitGaussianWRange(hdEdx_elCosbin[i], status);
      if (status != "FitOK") {
        fdEdxMean_el = 1.0;
        hdEdx_elCosbin[i]->SetTitle(Form("%s, Fit(%s)", hdEdx_elCosbin[i]->GetTitle(), status.Data()));
      } else {
        fdEdxMean_el = hdEdx_elCosbin[i]->GetFunction("gaus")->GetParameter(1);
        fdEdxMean_elErr = hdEdx_elCosbin[i]->GetFunction("gaus")->GetParError(1);
        fdEdxSigma_el = hdEdx_elCosbin[i]->GetFunction("gaus")->GetParameter(2);
        fdEdxSigma_elErr = hdEdx_elCosbin[i]->GetFunction("gaus")->GetParError(2);
        hdEdx_elCosbin[i]->SetTitle(Form("%s, Fit (%s), #mu_{fit}: %0.04f#pm%0.04f,, #sigma_{fit}: %0.04f", hdEdx_elCosbin[i]->GetTitle(),
                                         status.Data(), fdEdxMean_el, fdEdxMean_elErr, fdEdxSigma_el));
      }

      hdEdxMeanvsCos_el->SetBinContent(i + 1, fdEdxMean_el);
      hdEdxMeanvsCos_el->SetBinError(i + 1, fdEdxMean_elErr);
      hdEdxSigmavsCos_el->SetBinContent(i + 1, fdEdxSigma_el);
      hdEdxSigmavsCos_el->SetBinError(i + 1, fdEdxSigma_elErr);

      //Fit _potron in cos bins
      FitGaussianWRange(hdEdx_poCosbin[i], status);
      if (status != "FitOK") {
        fdEdxMean_po = 1.0;
        hdEdx_poCosbin[i]->SetTitle(Form("%s, Fit(%s)", hdEdx_poCosbin[i]->GetTitle(), status.Data()));
      } else {
        fdEdxMean_po = hdEdx_poCosbin[i]->GetFunction("gaus")->GetParameter(1);
        fdEdxMean_poErr = hdEdx_poCosbin[i]->GetFunction("gaus")->GetParError(1);
        fdEdxSigma_po = hdEdx_poCosbin[i]->GetFunction("gaus")->GetParameter(2);
        fdEdxSigma_poErr = hdEdx_poCosbin[i]->GetFunction("gaus")->GetParError(2);
        hdEdx_poCosbin[i]->SetTitle(Form("%s, Fit (%s), #mu_{fit}: %0.04f#pm%0.04f,, #sigma_{fit}: %0.04f", hdEdx_poCosbin[i]->GetTitle(),
                                         status.Data(), fdEdxMean_po, fdEdxMean_poErr, fdEdxSigma_po));
      }

      hdEdxMeanvsCos_po->SetBinContent(i + 1, fdEdxMean_po);
      hdEdxMeanvsCos_po->SetBinError(i + 1, fdEdxMean_poErr);
      hdEdxSigmavsCos_po->SetBinContent(i + 1, fdEdxSigma_po);
      hdEdxSigmavsCos_po->SetBinError(i + 1, fdEdxSigma_poErr);

      //for validation purpose
      if (isMakePlots) {

        ctmp_ep->cd(1); // each canvas is 2x2
        hdEdx_elCosbin[i]->SetFillColorAlpha(kYellow, 0.25);
        hdEdx_elCosbin[i]->DrawCopy("");
        tl->SetX1(fdEdxMean_el); tl->SetX2(fdEdxMean_el);
        tl->SetY1(0); tl->SetY2(hdEdx_elCosbin[i]->GetMaximum());
        tl->DrawClone("same");

        ctmp_ep->cd(2); // each canvas is 2x2
        hdEdx_poCosbin[i]->SetFillColorAlpha(kBlue, 0.25);
        hdEdx_poCosbin[i]->DrawCopy("");
        tl->SetX1(fdEdxMean_po); tl->SetX2(fdEdxMean_po);
        tl->SetY1(0); tl->SetY2(hdEdx_poCosbin[i]->GetMaximum());
        tl->DrawClone("same");
        ctmp_ep->Print(psname_ep.str().c_str());
      }

      //avg of both e+ and e- fdEdxMean
      fdEdxMean = 0.5 * (fdEdxMean_po + fdEdxMean_el);
      if (fdEdxMean <= 0)fdEdxMean = 1.0; //protection only
      fdEdxMeanErr = 0.5 * TMath::Sqrt(fdEdxMean_elErr * fdEdxMean_elErr +  fdEdxMean_poErr * fdEdxMean_poErr);
      hdEdxMeanvsCos_ep->SetBinContent(i + 1, fdEdxMean);
      hdEdxMeanvsCos_ep->SetBinError(i + 1, fdEdxMeanErr);
    }

    cosine.push_back(fdEdxMean);
    delete tl;
  }

  //more validation plots for debugging
  if (isMakePlots) {

    psname_ep.str("");
    psname_ep << "cdcdedx_coscal_fits.pdf]";
    ctmp_ep->Print(psname_ep.str().c_str());
    delete ctmp_ep;

    TCanvas* ctmp_epConst = new TCanvas("ctmp_epConst", "ctmp_epConst", 800, 400);
    ctmp_epConst->Divide(2, 1);

    TCanvas* ctmp_epCosth = new TCanvas("ctmp_epCosth", "ctmp_epCosth", 600, 500);

    if (isMethodSep) {

      ctmp_epConst->cd(1);
      gPad->SetGridy(1);
      hdEdxMeanvsCos_el->SetMarkerStyle(20);
      hdEdxMeanvsCos_el->SetMarkerSize(0.60);
      hdEdxMeanvsCos_el->SetMarkerColor(kRed);
      hdEdxMeanvsCos_el->SetStats(0);
      hdEdxMeanvsCos_el->SetTitle("comparison of dedx #mu_{fit}^{rel}: (e-=red, e+=blue, avg=black)");
      hdEdxMeanvsCos_el->GetYaxis()->SetRangeUser(0.96, 1.04);
      hdEdxMeanvsCos_el->DrawCopy("");

      hdEdxMeanvsCos_po->SetMarkerStyle(20);
      hdEdxMeanvsCos_po->SetMarkerSize(0.60);
      hdEdxMeanvsCos_po->SetMarkerColor(kBlue);
      hdEdxMeanvsCos_po->SetStats(0);
      hdEdxMeanvsCos_po->DrawCopy("same");

      hdEdxMeanvsCos_ep->SetMarkerStyle(20);
      hdEdxMeanvsCos_ep->SetMarkerSize(0.60);
      hdEdxMeanvsCos_ep->SetMarkerColor(kBlack);
      hdEdxMeanvsCos_ep->SetStats(0);
      hdEdxMeanvsCos_ep->DrawCopy("same");

      ctmp_epConst->cd(2);
      gPad->SetGridy(1);
      hdEdxSigmavsCos_el->SetMarkerStyle(4);
      hdEdxSigmavsCos_el->SetMarkerColor(kRed);
      hdEdxSigmavsCos_el->SetMarkerSize(0.90);
      hdEdxSigmavsCos_el->SetTitle("comparison of dedx #mu_{fit}^{rel}: (e-=open, e+=closed)");
      hdEdxSigmavsCos_el->GetYaxis()->SetRangeUser(0.4, 0.12);
      hdEdxSigmavsCos_el->SetStats(0);
      hdEdxSigmavsCos_el->DrawCopy("");

      hdEdxSigmavsCos_po->SetMarkerStyle(8);
      hdEdxSigmavsCos_po->SetMarkerSize(0.80);
      hdEdxSigmavsCos_po->SetMarkerColor(kBlue);
      hdEdxSigmavsCos_po->SetStats(0);
      hdEdxSigmavsCos_po->DrawCopy("same");

      ctmp_epCosth->cd();
      hCosth_el->SetStats(0);
      hCosth_el->SetLineColor(kRed);
      hCosth_el->SetFillColorAlpha(kYellow, 0.55);
      hCosth_el->DrawCopy("");
      hCosth_po->SetStats(0);
      hCosth_po->SetLineColor(kBlue);
      hCosth_po->SetFillColorAlpha(kGray, 0.35);
      hCosth_po->DrawCopy("same");

    } else {

      ctmp_epConst->cd(1);
      gPad->SetGridy(1);
      hdEdxMeanvsCos_ep->SetMarkerStyle(20);
      hdEdxMeanvsCos_ep->SetMarkerSize(0.60);
      hdEdxMeanvsCos_ep->SetMarkerColor(kBlack);
      hdEdxMeanvsCos_ep->SetStats(0);
      hdEdxMeanvsCos_ep->SetTitle("dedx rel(#mu_{fit}) for e- and e+ combined");
      hdEdxMeanvsCos_ep->GetYaxis()->SetRangeUser(0.97, 1.04);
      hdEdxMeanvsCos_ep->DrawCopy("");

      ctmp_epConst->cd(2);
      gPad->SetGridy(1);
      hdEdxSigmavsCos_ep->SetMarkerStyle(20);
      hdEdxSigmavsCos_ep->SetMarkerColor(kRed);
      hdEdxSigmavsCos_ep->SetMarkerSize(1.1);
      hdEdxSigmavsCos_ep->SetTitle("dedx rel(#sigma_{fit}) for e- and e+ combined");
      hdEdxSigmavsCos_ep->GetYaxis()->SetRangeUser(0.4, 0.12);
      hdEdxSigmavsCos_ep->SetStats(0);
      hdEdxSigmavsCos_ep->DrawCopy("");

      ctmp_epCosth->cd();
      hCosth_ep->SetStats(0);
      hCosth_ep->SetLineColor(kGray);
      hCosth_ep->SetFillColorAlpha(kGray, 0.25);
      hCosth_ep->DrawCopy("same");
    }

    ctmp_epCosth->SaveAs("cdcdedx_coscal_costhdist.pdf");
    delete ctmp_epCosth;

    ctmp_epConst->SaveAs("cdcdedx_coscal_relmeans.pdf");
    ctmp_epConst->SaveAs("cdcdedx_coscal_relmeans.root");
    delete ctmp_epConst;
  }

  generateNewPayloads(cosine);
  return c_OK;
}

void CDCDedxCosineAlgorithm::generateNewPayloads(std::vector<double> cosine)
{

  TH1D* hCosCorrOld = new TH1D("hCosCorrOld", "cos corr const comparison (red=old, blue=new);cos(#theta);dE/dx #mu_{fit}", fCosbins,
                               fCosMin, fCosMax);
  TH1D* hCosCorrNew = new TH1D("hCosCorrNew", "coss corr ;cos(#theta);dE/dx #mu_{fit}", fCosbins, fCosMin, fCosMax);
  TH1D* hCosCorrRel = new TH1D("hCosCorrRel", "new relative cos corr;cos(#theta);dE/dx #mu_{fit}", fCosbins, fCosMin, fCosMax);

  if (isMergePayload) {
    const auto expRun = getRunList()[0];
    updateDBObjPtrs(1, expRun.second, expRun.first);
    // bool refchange = m_DBCosineCor.hasChanged(); //Add this feature for major processing
    B2INFO("Saving new rung for (Exp, Run) : (" << expRun.first << "," << expRun.second << ")");
    for (unsigned int ibin = 0; ibin < m_DBCosineCor->getSize(); ibin++) {
      hCosCorrOld->SetBinContent(ibin + 1, (double)m_DBCosineCor->getMean(ibin));
      hCosCorrRel->SetBinContent(ibin + 1, cosine.at(ibin));
      B2INFO("Cosine Corr for Bin # " << ibin << ", Previous = " << m_DBCosineCor->getMean(ibin) << ", Relative = " << cosine.at(
               ibin) << ", Merged = " << m_DBCosineCor->getMean(ibin)*cosine.at(ibin));
      cosine.at(ibin) *= (double)m_DBCosineCor->getMean(ibin);
      hCosCorrNew->SetBinContent(ibin + 1, cosine.at(ibin));
    }
  }

  if (isMakePlots) {
    TCanvas* ctmp_const = new TCanvas("ctmp_const", "ctmp_const", 900, 450);
    ctmp_const->Divide(2, 1);

    ctmp_const->cd(1);
    gPad->SetGridy(1);
    gPad->SetGridx(1);
    hCosCorrOld->SetStats(0);
    hCosCorrOld->SetLineColor(kRed);
    hCosCorrOld->GetYaxis()->SetRangeUser(0.64, 1.20);
    hCosCorrOld->DrawCopy("");
    hCosCorrNew->SetStats(0);
    hCosCorrNew->SetLineColor(kBlue);
    hCosCorrNew->DrawCopy("same");

    ctmp_const->cd(2);
    gPad->SetGridy(1);
    hCosCorrRel->SetStats(0);
    hCosCorrRel->GetYaxis()->SetRangeUser(0.97, 1.03);
    hCosCorrRel->SetLineColor(kBlack);
    hCosCorrRel->DrawCopy("");

    ctmp_const->SaveAs("cdcdedx_coscal_constants.pdf");
    ctmp_const->SaveAs("cdcdedx_coscal_constants.root");
    delete ctmp_const;
  }

  B2INFO("dE/dx calibration done for CDC dE/dx _eltron saturation");
  CDCDedxCosineCor* gain = new CDCDedxCosineCor(cosine);
  saveCalibration(gain, "CDCDedxCosineCor");
}

void CDCDedxCosineAlgorithm::FitGaussianWRange(TH1D*& temphist, TString& status)
{
  if (temphist->Integral() < 500) {
    B2INFO(Form("\tThis hist (%s) have insufficient entries to perform fit (%0.03f)", temphist->GetName(), temphist->Integral()));
    status = "LowStats";
    return;
  } else {
    temphist->GetXaxis()->SetRange(temphist->FindFirstBinAbove(0, 1), temphist->FindLastBinAbove(0, 1));
    int fs = temphist->Fit("gaus", "QR");
    if (fs != 0) {
      B2INFO(Form("\tFit (round 1) for hist (%s) failed (status = %d)", temphist->GetName(), fs));
      status = "FitFailed";
      return;
    } else {
      double fdEdxMean = temphist->GetFunction("gaus")->GetParameter(1);
      double width = temphist->GetFunction("gaus")->GetParameter(2);
      temphist->GetXaxis()->SetRangeUser(fdEdxMean - 5.0 * width, fdEdxMean + 5.0 * width);
      fs = temphist->Fit("gaus", "QR", "", fdEdxMean - fSigLim * width, fdEdxMean + fSigLim * width);
      if (fs != 0) {
        B2INFO(Form("\tFit (round 2) for hist (%s) failed (status = %d)", temphist->GetName(), fs));
        status = "FitFailed";
        return;
      } else {
        temphist->GetXaxis()->SetRangeUser(fdEdxMean - 5.0 * width, fdEdxMean + 5.0 * width);
        B2INFO(Form("\tFit for hist (%s) sucessfull (status = %d)", temphist->GetName(), fs));
        status = "FitOK";
      }
    }
  }
}