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
  isMergePayload(true)
{
  // Set module properties
  setDescription("A calibration algorithm for CDC dE/dx electron cos(theta) dependence");

}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------

CalibrationAlgorithm::EResult CDCDedxCosineAlgorithm::calibrate()
{

  B2INFO("Preparing dE/dx calibration for CDC dE/dx electron saturation");

  // Get data objects
  auto ttree = getObjectPtr<TTree>("tree");
  if (ttree->GetEntries() < 100)return c_NotEnoughData;

  double dedx, costh; int charge;
  ttree->SetBranchAddress("dedx", &dedx);
  ttree->SetBranchAddress("costh", &costh);
  ttree->SetBranchAddress("charge", &charge);

  // make histograms to store dE/dx values in bins of cos(theta)
  // bin size can be arbitrary, but for now just make uniform bins
  const int nbins = 100;
  TH1D* hdEdx_elCosbin[nbins], *hdEdx_poCosbin[nbins], *hdEdx_epCosbin[nbins];
  for (unsigned int i = 0; i < nbins; ++i) {
    hdEdx_elCosbin[i] = new TH1D(Form("hdEdx_elCosbin%d", i), "dE/dx (e-) in bins of cosine", 400, 0, 2);
    hdEdx_elCosbin[i]->GetXaxis()->SetTitle("dE/dx (no had sat, for e-)");
    hdEdx_elCosbin[i]->GetYaxis()->SetTitle("Entries");

    hdEdx_poCosbin[i] = new TH1D(Form("hdEdx_poCosbin%d", i), "dE/dx (e+) in bins of cosine", 400, 0, 2);
    hdEdx_poCosbin[i]->GetXaxis()->SetTitle("dE/dx (no had sat, for e+)");
    hdEdx_poCosbin[i]->GetYaxis()->SetTitle("Entries");

    hdEdx_epCosbin[i] = new TH1D(Form("hdEdx_epCosbin%d", i), "dE/dx (e- and e+) in bins of cosine", 400, 0, 2);
    hdEdx_epCosbin[i]->GetXaxis()->SetTitle("dE/dx (no had sat, for e-,e+)");
    hdEdx_epCosbin[i]->GetYaxis()->SetTitle("Entries");
  }

  // fill histograms, bin size may be arbitrary
  TH1D* hCosthElec = new TH1D("hCosthElec", "Cos(#theta) dist; Cos(#theta): e-; Entries", 100, -1.0, 1.0);
  TH1D* hCosthPosi = new TH1D("hCosthPosi", "Cos(#theta) dist; Cos(#theta): e+; Entries", 100, -1.0, 1.0);
  TH1D* hCosthElPo = new TH1D("hCosthElPo", "Cos(#theta) dist; Cos(#theta): avg e+, e-; Entries", 100, -1.0, 1.0);

  const double costhmin = -1.0, costhmax = 1.0;
  for (int i = 0; i < ttree->GetEntries(); ++i) {

    ttree->GetEvent(i);
    if (dedx == 0 || charge == 0 || costh < costhmin || costh > costhmax) continue;

    int bin = int((costh - costhmin) * nbins / (costhmax - costhmin));
    if (bin < 0 || bin >= nbins) continue;

    if (isMethodSep) {
      if (charge < 0) {
        hCosthElec->Fill(costh);
        hdEdx_elCosbin[bin]->Fill(dedx);
      } else if (charge > 0) {
        hCosthPosi->Fill(costh);
        hdEdx_poCosbin[bin]->Fill(dedx);
      }
    } else {
      hCosthElPo->Fill(costh);
      hdEdx_epCosbin[bin]->Fill(dedx);
    }
  }

  // Print the histograms for quality control
  TCanvas* ctmpElPo = new TCanvas("ctmpElPo", "ctmpElPo", 800, 400);
  if (isMethodSep)ctmpElPo->Divide(2, 1);
  else ctmpElPo->SetCanvasSize(400, 800);
  std::stringstream psnameElPo;

  TLine* tl = new TLine();
  tl->SetLineColor(kBlack);

  if (isMakePlots) {
    psnameElPo << "dedxFits_cosinebins_ElPo.pdf[";
    ctmpElPo->Print(psnameElPo.str().c_str());
    psnameElPo.str("");
    psnameElPo << "dedxFits_cosinebins_ElPo.pdf";
  }

  //Plot constants
  TH1D* hCosConstMeanPosi = new TH1D("hCosConstMeanPosi", "dEdx Mean vs Costh Bin; Cos(#theta); dEdx (e+) means", 100, -1.0, 1.0);
  TH1D* hCosConstMeanElec = new TH1D("hCosConstMeanElec", "dEdx Mean vs Costh Bin; Cos(#theta); dEdx (e-) means", 100, -1.0, 1.0);
  TH1D* hCosConstMeanElPo = new TH1D("hCosConstMeanElPo", "dEdx Mean vs Costh Bin; Cos(#theta); dEdx (e-,e+) means", 100, -1.0, 1.0);

  TH1D* hCosConstSigmaPosi = new TH1D("hCosConstSigmaPosi", "dEdx Sigma vs Costh Bin; Cos(#theta); dEdx (e+) sigmas", 100, -1.0, 1.0);
  TH1D* hCosConstSigmaElec = new TH1D("hCosConstSigmaElec", "dEdx Sigma vs Costh Bin; Cos(#theta); dEdx (e-) sigmas", 100, -1.0, 1.0);
  TH1D* hCosConstSigmaElPo = new TH1D("hCosConstSigmaElPo", "dEdx Sigma vs Costh Bin; Cos(#theta); dEdx (e-,e+) sigmas", 100, -1.0,
                                      1.0);

  // fit histograms to get gains in bins of cos(theta)
  std::vector<double> cosine;

  for (unsigned int i = 0; i < nbins; ++i) {

    double BW = (costhmax - costhmin) / nbins;
    double LowE = i * BW + costhmin;
    double UpE  = LowE + BW;

    TString status = "";
    double mean = 1.0;

    if (!isMethodSep) {

      double meanErr = 0.0;
      double sigma = 0.0, sigmaErr = 0.0;
      FitGaussianWRange(hdEdx_epCosbin[i], status);
      if (status != "FitOK")mean = 1.0;
      else {
        mean = hdEdx_epCosbin[i]->GetFunction("gaus")->GetParameter(1);
        meanErr = hdEdx_epCosbin[i]->GetFunction("gaus")->GetParError(1);
        sigma = hdEdx_epCosbin[i]->GetFunction("gaus")->GetParameter(2);
        sigmaErr = hdEdx_epCosbin[i]->GetFunction("gaus")->GetParError(2);
      }
      hdEdx_epCosbin[i]->SetTitle(Form("dE/dx (avg) in cos(#theta) %0.03f,%0.03f, Fit: %s", LowE, UpE, status.Data()));

      if (isMakePlots) {

        hCosConstMeanElPo->SetBinContent(i + 1, mean);
        hCosConstMeanElPo->SetBinError(i + 1, meanErr);
        hCosConstSigmaElPo->SetBinContent(i + 1, sigma);
        hCosConstSigmaElPo->SetBinError(i + 1, sigmaErr);

        ctmpElPo->cd(i % 4 + 1); // each canvas is 2x2
        hdEdx_epCosbin[i]->SetFillColor(kYellow);
        hdEdx_epCosbin[i]->DrawCopy("hist");

        tl->SetX1(mean); tl->SetX2(mean);
        tl->SetY1(0); tl->SetY2(hdEdx_epCosbin[i]->GetMaximum());
        tl->DrawClone("same");
        if ((i + 1) % 4 == 0)ctmpElPo->Print(psnameElPo.str().c_str());
      }
    } else {

      double meanelec = 1.0, meanelecErr = 0.0;
      double sigmaelec = 0.0, sigmaelecErr = 0.0;
      double meanposi = 1.0, meanposiErr = 0.0;
      double sigmaposi = 0.0, sigmaposiErr = 0.0;
      //Fit Electrons in cos bins
      FitGaussianWRange(hdEdx_elCosbin[i], status);
      if (status != "FitOK")meanelec = 1.0;
      else {
        meanelec = hdEdx_elCosbin[i]->GetFunction("gaus")->GetParameter(1);
        meanelecErr = hdEdx_elCosbin[i]->GetFunction("gaus")->GetParError(1);
        sigmaelec = hdEdx_elCosbin[i]->GetFunction("gaus")->GetParameter(2);
        sigmaelecErr = hdEdx_elCosbin[i]->GetFunction("gaus")->GetParError(2);
        hdEdx_elCosbin[i]->SetTitle(Form("dE/dx (e-) in cos(#theta) %0.03f,%0.03f, Fit: %s, Mean = %0.03f", LowE, UpE, status.Data(),
                                         meanelec));
      }


      //Fit Positron in cos bins
      FitGaussianWRange(hdEdx_poCosbin[i], status);
      if (status != "FitOK")meanposi = 1.0;
      else {
        meanposi = hdEdx_poCosbin[i]->GetFunction("gaus")->GetParameter(1);
        meanposiErr = hdEdx_poCosbin[i]->GetFunction("gaus")->GetParError(1);
        sigmaposi = hdEdx_poCosbin[i]->GetFunction("gaus")->GetParameter(2);
        sigmaposiErr = hdEdx_poCosbin[i]->GetFunction("gaus")->GetParError(2);
        hdEdx_poCosbin[i]->SetTitle(Form("dE/dx (e+) in cos(#theta) %0.03f,%0.03f, Fit: %s, Mean = %0.03f", LowE, UpE, status.Data(),
                                         meanposi));
      }

      if (isMakePlots) {

        hCosConstMeanElec->SetBinContent(i + 1, meanelec);
        hCosConstMeanElec->SetBinError(i + 1, meanelecErr);
        hCosConstSigmaElec->SetBinContent(i + 1, sigmaelec);
        hCosConstSigmaElec->SetBinError(i + 1, sigmaelecErr);

        hCosConstMeanPosi->SetBinContent(i + 1, meanposi);
        hCosConstMeanPosi->SetBinError(i + 1, meanposiErr);
        hCosConstSigmaPosi->SetBinContent(i + 1, sigmaposi);
        hCosConstSigmaPosi->SetBinError(i + 1, sigmaposiErr);

        ctmpElPo->cd(1); // each canvas is 2x2
        hdEdx_elCosbin[i]->SetFillColorAlpha(kYellow, 0.10);
        hdEdx_elCosbin[i]->DrawCopy("");
        tl->SetX1(meanelec); tl->SetX2(meanelec);
        tl->SetY1(0); tl->SetY2(hdEdx_elCosbin[i]->GetMaximum());
        tl->DrawClone("same");

        ctmpElPo->cd(2); // each canvas is 2x2
        hdEdx_poCosbin[i]->SetFillColorAlpha(kBlue, 0.10);
        hdEdx_poCosbin[i]->DrawCopy("");
        tl->SetX1(meanposi); tl->SetX2(meanposi);
        tl->SetY1(0); tl->SetY2(hdEdx_poCosbin[i]->GetMaximum());
        tl->DrawClone("same");
        ctmpElPo->Print(psnameElPo.str().c_str());
      }

      //avg of both e+ and e- mean
      mean = 0.5 * (meanposi + meanelec);
      if (mean <= 0)mean = 1.0; //protection only
      hCosConstMeanElPo->SetBinContent(i + 1, mean);
      hCosConstMeanElPo->SetBinError(i + 1, 0.0);
    }

    cosine.push_back(mean);
  }


  if (isMakePlots) {

    TCanvas* ctmpElPoCCComp = new TCanvas("ctmpElPoCCComp", "ctmpElPoCCComp", 800, 400);
    ctmpElPoCCComp->Divide(2, 1);

    TCanvas* ctmpElPoCosth = new TCanvas("ctmpElPoCosth", "ctmpElPoCosth", 500, 500);

    if (isMethodSep) {

      ctmpElPoCCComp->cd(1);
      hCosConstMeanElec->SetMarkerStyle(20);
      hCosConstMeanElec->SetMarkerSize(0.80);
      hCosConstMeanElec->SetMarkerColor(kRed);
      hCosConstMeanElec->SetStats(0);
      hCosConstMeanElec->SetTitle("rel-dedx-mean comparison (e-=red, e+=blue, avg=black)");
      hCosConstMeanElec->GetYaxis()->SetRangeUser(0.95, 1.05);
      hCosConstMeanElec->DrawCopy("");

      hCosConstMeanPosi->SetMarkerStyle(20);
      hCosConstMeanPosi->SetMarkerSize(0.80);
      hCosConstMeanPosi->SetMarkerColor(kBlue);
      hCosConstMeanPosi->SetStats(0);
      hCosConstMeanPosi->DrawCopy("same");

      hCosConstMeanElPo->SetMarkerStyle(20);
      hCosConstMeanElPo->SetMarkerSize(0.80);
      hCosConstMeanElPo->SetMarkerColor(kBlack);
      hCosConstMeanElPo->SetStats(0);
      hCosConstMeanElPo->DrawCopy("same");

      ctmpElPoCCComp->cd(2);
      hCosConstSigmaElec->SetMarkerStyle(20);
      hCosConstSigmaElec->SetMarkerColor(kRed);
      hCosConstSigmaElec->SetMarkerSize(1.1);
      hCosConstSigmaElec->SetTitle("rel-dedx-mean comparison (e-=red, e+=blue)");
      hCosConstSigmaElec->GetYaxis()->SetRangeUser(0.2, 0.12);
      hCosConstSigmaElec->SetStats(0);
      hCosConstSigmaElec->DrawCopy("");

      hCosConstSigmaPosi->SetMarkerStyle(20);
      hCosConstSigmaPosi->SetMarkerSize(0.80);
      hCosConstSigmaPosi->SetMarkerColor(kBlue);
      hCosConstSigmaPosi->SetStats(0);
      hCosConstSigmaPosi->DrawCopy("same");

      ctmpElPoCCComp->Print("hMeanSigma_CosthBin.pdf");
      ctmpElPoCCComp->Print("hMeanSigma_CosthBin.root");
      delete ctmpElPoCCComp;

      psnameElPo.str("");
      psnameElPo << "dedxFits_cosinebins_ElPo.pdf]";
      ctmpElPo->Print(psnameElPo.str().c_str());
      delete ctmpElPo;

      ctmpElPoCosth->cd();
      hCosthElec->SetStats(0);
      hCosthElec->SetLineColor(kRed);
      hCosthElec->DrawCopy("");
      hCosthPosi->SetStats(0);
      hCosthPosi->SetLineColor(kBlue);
      hCosthPosi->DrawCopy("same");
      ctmpElPoCosth->Print("hCosth_Dist_Comparison.pdf");
      delete ctmpElPoCosth;

    } else {

      ctmpElPoCCComp->cd(1);
      hCosConstMeanElPo->SetLineColor(kBlack);
      hCosConstMeanElPo->DrawCopy("");

      ctmpElPoCCComp->cd(2);
      hCosConstSigmaElPo->SetLineColor(kBlack);
      hCosConstSigmaElPo->DrawCopy("same");

      ctmpElPoCCComp->Print("hMeanSigma_CosthBin.pdf");
      ctmpElPoCCComp->Print("hMeanSigma_CosthBin.root");

      psnameElPo.str("");
      psnameElPo << "dedxFits_cosinebins_ElPo.pdf]";
      ctmpElPo->Print(psnameElPo.str().c_str());
      delete ctmpElPo;

      ctmpElPoCosth->cd();
      hCosthElPo->SetStats(0);
      hCosthElPo->SetLineColor(kBlack);
      hCosthElPo->DrawCopy("same");
      ctmpElPoCosth->Print("hCosth_Dist.pdf");
      delete ctmpElPoCosth;
    }
  }

  delete tl;
  generateNewPayloads(cosine);
  return c_OK;
}



void CDCDedxCosineAlgorithm::generateNewPayloads(std::vector<double> cosine)
{

  TH1D* hCompareCorrOld = new TH1D("hCompareCorrOld", "Const Comp (Red->old);Cos(#theta);dEdx mean", 100, -1.0, 1.0);
  TH1D* hCompareCorrNew = new TH1D("hCompareCorrNew", "Const Comp;Cos(#theta);dEdx mean", 100, -1.0, 1.0);

  if (isMergePayload) {
    const auto expRun = getRunList()[0];
    updateDBObjPtrs(1, expRun.second, expRun.first);
    // bool refchange = m_DBCosineCor.hasChanged(); //Add this feature for major processing
    B2INFO("Saving new rung for (Exp, Run) : (" << expRun.first << "," << expRun.second << ")");
    for (unsigned int ibin = 0; ibin < m_DBCosineCor->getSize(); ibin++) {
      hCompareCorrOld->SetBinContent(ibin + 1, (double)m_DBCosineCor->getMean(ibin));
      B2INFO("Cosine Corr for Bin # " << ibin << ", Previous = " << m_DBCosineCor->getMean(ibin) << ", Relative = " << cosine.at(
               ibin) << ", Merged = " << m_DBCosineCor->getMean(ibin)*cosine.at(ibin));
      cosine.at(ibin) *= (double)m_DBCosineCor->getMean(ibin);
      hCompareCorrNew->SetBinContent(ibin + 1, cosine.at(ibin));
    }
  }

  if (isMakePlots) {
    TCanvas* coldCCComp = new TCanvas("coldCCComp", "coldCCComp", 500, 500);
    hCompareCorrOld->SetStats(0);
    hCompareCorrOld->SetLineColor(kRed);
    hCompareCorrOld->DrawCopy("");
    hCompareCorrNew->SetStats(0);
    hCompareCorrNew->SetLineColor(kBlue);
    hCompareCorrNew->DrawCopy("same");
    coldCCComp->Print("hCosineCorr_CompareConstNewvsOld.pdf");
    coldCCComp->Print("hCosineCorr_CompareConstNewvsOld.root");
    delete coldCCComp;
  }

  B2INFO("dE/dx calibration done for CDC dE/dx electron saturation");
  CDCDedxCosineCor* gain = new CDCDedxCosineCor(cosine);
  saveCalibration(gain, "CDCDedxCosineCor");
}


void CDCDedxCosineAlgorithm::FitGaussianWRange(TH1D*& temphist, TString& status)
{

  if (temphist->Integral() < 250) {
    std::cout << "\t\t" << Form("%s no-FIT as this bin is LowStats < 250", temphist->GetName()) << std::endl;
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

