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
#include <TH1D.h>
#include <TLine.h>
#include <TCanvas.h>
#include <framework/core/HistoModule.h>

using namespace Belle2;


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDedxCosineAlgorithm::CDCDedxCosineAlgorithm() :
  CalibrationAlgorithm("CDCDedxElectronCollector"),
  isMethodSep(true),
  isMakePlots(true)
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

  // require at least 100 tracks (arbitrary for now)
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
    hdEdx_elCosbin[i] = new TH1D(Form("hdEdx_elCosbin%d", i), "dE/dx (e-) in bins of cosine", 100, 0, 2);
    hdEdx_poCosbin[i] = new TH1D(Form("hdEdx_poCosbin%d", i), "dE/dx (e+) in bins of cosine", 100, 0, 2);
    hdEdx_epCosbin[i] = new TH1D(Form("hdEdx_epCosbin%d", i), "dE/dx (e- and e+) in bins of cosine", 100, 0, 2);
  }

  TH1D* hCosth = new TH1D("hCosth", "hCosth distribution", 100, -1.0, 1.0);
  hCosth->GetXaxis()->SetTitle("Cos(#theta)");
  hCosth->GetYaxis()->SetTitle("Entries");

  TH1D* hMeanvsCosthep = new TH1D("hMeanvsCosthep", "Const", 100, -1.0, 1.0);
  hMeanvsCosthep->GetXaxis()->SetTitle("Cos(#theta)");
  hMeanvsCosthep->GetYaxis()->SetTitle("Entries");

  TH1D* hMeanvsCosthpo = new TH1D("hMeanvsCosthpo", "Const Comparision", 100, -1.0, 1.0);
  hMeanvsCosthpo->GetXaxis()->SetTitle("Cos(#theta)");
  hMeanvsCosthpo->GetYaxis()->SetTitle("dEdx Mean");

  TH1D* hMeanvsCosthel = new TH1D("hMeanvsCosthel", "Const Comparision", 100, -1.0, 1.0);
  hMeanvsCosthel->GetXaxis()->SetTitle("Cos(#theta)");
  hMeanvsCosthel->GetYaxis()->SetTitle("dEdx Mean");

  // fill histograms, bin size may be arbitrary
  const double costhmin = -1.0, costhmax = 1.0;
  for (int i = 0; i < ttree->GetEntries(); ++i) {

    ttree->GetEvent(i);
    if (dedx == 0 || charge == 0 || costh < costhmin || costh > costhmax) continue;

    int bin = int((costh - costhmin) * nbins / (costhmax - costhmin));
    if (bin < 0 || bin >= nbins) continue;

    hCosth->Fill(costh);

    if (isMethodSep) {
      if (charge < 0)hdEdx_elCosbin[bin]->Fill(dedx);
      else if (charge > 0)hdEdx_poCosbin[bin]->Fill(dedx);
    } else {
      hdEdx_epCosbin[bin]->Fill(dedx);
    }
  }

  // Print the histograms for quality control
  TCanvas* ctmp = new TCanvas("tmp", "tmp", 900, 900);
  ctmp->Divide(3, 3);
  std::stringstream psname;
  TLine* tl = new TLine();

  if (isMakePlots) {
    psname << "dedx_cosine.pdf[";
    ctmp->Print(psname.str().c_str());
    psname.str("");
    psname << "dedx_cosine.pdf";
  }
  // fit histograms to get gains in bins of cos(theta)
  std::vector<double> cosine;

  for (unsigned int i = 0; i < nbins; ++i) {

    double BW = (costhmax - costhmin) / nbins;
    double LowE = i * BW + costhmin;
    double UpE  = LowE + BW;

    if (isMakePlots)ctmp->cd(i % 9 + 1); // each canvas is 9x9

    double mean = 1.0;
    std::string status = "";

    TF1* mygaus = new TF1("mygaus", "gaus", 0.5, 1.5);
    TF1* mygausel = new TF1("mygausel", "gaus", 0.5, 1.5);
    TF1* mygauspo = new TF1("mygauspo", "gaus", 0.5, 1.5);

    if (!isMethodSep) {

      mygaus->SetParLimits(1, 0.90, 1.10);
      mygaus->SetParLimits(2, 0.03, 0.12);
      mygaus->SetLineColor(kBlack);
      mygaus->SetLineWidth(1);
      mygaus->SetLineStyle(2);

      if (hdEdx_epCosbin[i]->Integral() < 30) {
        mean = 1.0;
        status = "LowStats";
      } else {
        hdEdx_epCosbin[i]->Fit("mygaus", "QM");
        if (!hdEdx_epCosbin[i]->GetFunction("mygaus")->IsValid()) {
          mean = 1.0;
          status = "FitFailed";
        } else {
          mean = hdEdx_epCosbin[i]->GetFunction("mygaus")->GetParameter(1);
          status = "all_OK";
        }
      }
      //printf("fit status e+e- = %s, %0.3f \n", status.data(), mean);
      hMeanvsCosthep->SetBinContent(i + 1, mean);

      if (isMakePlots) {
        hdEdx_epCosbin[i]->SetTitle(Form("dE/dx in cos(#theta) %0.03f,%0.03f, W/ status = %s", LowE, UpE, status.data()));
        hdEdx_epCosbin[i]->GetXaxis()->SetTitle("dE/dx (no had sat, all e+e-)");
        hdEdx_epCosbin[i]->GetYaxis()->SetTitle("Entries");
        hdEdx_epCosbin[i]->SetFillColor(kYellow);
        hdEdx_epCosbin[i]->DrawCopy("hist");
        mygaus->DrawCopy("same");

        tl->SetLineColor(kRed);
        tl->SetX1(mean); tl->SetX2(mean);
        tl->SetY1(0); tl->SetY2(hdEdx_epCosbin[i]->GetMaximum());
        tl->DrawClone("same");
        //printf("track = %d) >>  mean = %.04f \n", i, mean);
      }

    } else {

      mygausel->SetParLimits(1, 0.90, 1.10);
      mygausel->SetParLimits(2, 0.03, 0.12);
      mygausel->SetLineColor(kBlack);
      mygausel->SetLineWidth(1);
      mygausel->SetLineStyle(2);

      double  meaneminus = 1.0;
      std::string status2 = "";
      if (hdEdx_elCosbin[i]->Integral() < 100) {
        meaneminus = 1.0;
        status = "LowStats";
      } else {
        hdEdx_elCosbin[i]->Fit("mygausel", "QM");
        if (!hdEdx_elCosbin[i]->GetFunction("mygausel")->IsValid()) {
          meaneminus = 1.0;
          status = "FitFailed";
        } else {
          meaneminus = hdEdx_elCosbin[i]->GetFunction("mygausel")->GetParameter(1);
          status = "all_OK";
        }
      }
      //printf("fit status e- = %s, %0.3f \n", status.data(), meaneminus);

      if (meaneminus <= 0) meaneminus = 1.0; //nothing but a protection against arb value from fit
      hMeanvsCosthel->SetBinContent(i + 1, meaneminus);

      mygauspo->SetParLimits(1, 0.90, 1.10);
      mygauspo->SetParLimits(2, 0.03, 0.12);
      mygauspo->SetLineColor(kBlack);
      mygauspo->SetLineWidth(1);
      mygauspo->SetLineStyle(1);

      double  meaneplus = 1.0;
      if (hdEdx_poCosbin[i]->Integral() < 100) {
        meaneplus = 1.0;
        status2 = "LowStats";
      } else {
        hdEdx_poCosbin[i]->Fit("mygauspo", "QM");
        if (!hdEdx_poCosbin[i]->GetFunction("mygauspo")->IsValid()) {
          meaneplus = 1.0;
          status2 = "FitFailed";
        } else {
          meaneplus = hdEdx_poCosbin[i]->GetFunction("mygauspo")->GetParameter(1);
          status2 = "all_OK";
        }
      }

      //printf("fit status e+ = %s, %0.3f \n", status2.data(), meaneplus);

      if (meaneplus <= 0) meaneplus = 1.0; //nothing but a protection against arb value from fit
      hMeanvsCosthpo->SetBinContent(i + 1, meaneplus);

      mean = 0.5 * (meaneplus + meaneminus); //avg of both e+ and e- mean
      hMeanvsCosthep->SetBinContent(i + 1, mean);

      if (isMakePlots) {
        hdEdx_elCosbin[i]->SetTitle(Form("dE/dx in cos(#theta) %0.03f,%0.03f, W/ status = e-:%s, e+:%s", LowE, UpE, status.data(),
                                         status2.data()));
        hdEdx_elCosbin[i]->GetXaxis()->SetTitle("dE/dx (no had sat, for e- and e-)");
        hdEdx_elCosbin[i]->GetYaxis()->SetTitle("Entries");
        hdEdx_elCosbin[i]->SetMarkerColor(kRed);
        hdEdx_elCosbin[i]->SetFillColorAlpha(kRed, 0.70);
        if (hdEdx_elCosbin[i]->GetMaximum() < hdEdx_poCosbin[i]->GetMaximum())hdEdx_elCosbin[i]->SetMaximum(
            1.05 * hdEdx_poCosbin[i]->GetMaximum());
        hdEdx_elCosbin[i]->DrawCopy("hist");
        mygausel->DrawCopy("same");

        tl->SetLineColor(kBlack);
        tl->SetX1(mean); tl->SetX2(mean);
        tl->SetY1(0); tl->SetY2(hdEdx_elCosbin[i]->GetMaximum());
        tl->DrawClone("same");

        hdEdx_poCosbin[i]->SetFillColorAlpha(kBlue, 0.40);
        hdEdx_poCosbin[i]->DrawCopy("hist same");
        mygauspo->DrawCopy("same");
      }
      //printf("track = %d) eplus mean = %0.3f, eminus mean = %0.3f, avg mean = %0.3f \n", i, meaneplus, meaneminus, mean);
    }

    //printf("Pushing means to cosine vector = %0.3f \n", mean);
    cosine.push_back(mean);
    if (isMakePlots)if ((i + 1) % 9 == 0)ctmp->Print(psname.str().c_str());

  }

  if (isMakePlots) {
    psname.str(""); psname << "dedx_cosine.pdf]";
    ctmp->Print(psname.str().c_str());

    TCanvas* ctmpCosth = new TCanvas("ctmpCosth", "ctmpCosth", 500, 500);
    hCosth->DrawCopy();
    ctmpCosth->Print("hDistributionCosth.pdf");

    TCanvas* ctmpCCComp = new TCanvas("ctmpCCComp", "ctmpCosth", 500, 500);
    if (isMethodSep) {

      hMeanvsCosthel->SetLineColor(kRed);
      hMeanvsCosthel->GetYaxis()->SetRangeUser(0.80, 1.20);
      hMeanvsCosthel->DrawCopy("");
      hMeanvsCosthpo->SetLineColor(kBlue);
      hMeanvsCosthpo->DrawCopy("same");
      hMeanvsCosthep->SetLineColor(kBlack);
      hMeanvsCosthep->DrawCopy("same");
      ctmpCCComp->Print("hCosineCorr_Constants_Comparision.pdf");
    } else {
      hMeanvsCosthep->SetLineColor(kBlack);
      hMeanvsCosthep->DrawCopy("");
      ctmpCCComp->Print("hCosineCorr_Constants.pdf");
    }

    delete ctmp;
    delete tl;
    delete ctmpCosth;
    delete ctmpCCComp;
  }

  B2INFO("dE/dx calibration done for CDC dE/dx electron saturation");
  CDCDedxCosineCor* gain = new CDCDedxCosineCor(cosine);
  saveCalibration(gain, "CDCDedxCosineCor");

  return c_OK;
}
