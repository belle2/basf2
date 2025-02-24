/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/calibration/SVDdEdxCalibrationAlgorithm.h>
#include <svd/dbobjects/SVDdEdxPDFs.h>

#include <TROOT.h>
#include <TStyle.h>
#include <TMath.h>
#include <TFile.h>
#include <TTree.h>
#include <TColor.h>
#include <TLegend.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <TAxis.h>

#include <RooDataSet.h>
#include <RooRealVar.h>
#include <RooAddPdf.h>
#include <RooGaussian.h>
#include <RooChebychev.h>
#include <RooBifurGauss.h>
#include <RooDstD0BG.h>
#include <RooAbsDataStore.h>
#include <RooTreeDataStore.h>
#include <RooMsgService.h>
#include <RooStats/SPlot.h>

using namespace RooFit;
using namespace Belle2;

SVDdEdxCalibrationAlgorithm::SVDdEdxCalibrationAlgorithm() : CalibrationAlgorithm("SVDdEdxCollector"),
  m_isMakePlots(true)
{
  setDescription("SVD dE/dx calibration algorithm");
}

/* Main calibration method */
CalibrationAlgorithm::EResult SVDdEdxCalibrationAlgorithm::calibrate()
{
  gROOT->SetBatch(true);

  const auto exprun = getRunList()[0];
  B2INFO("ExpRun used for calibration: " << exprun.first << " " << exprun.second);

  auto payload = new Belle2::SVDdEdxPDFs();

  // Get data objects
  auto ttreeLambda = getObjectPtr<TTree>("Lambda");
  auto ttreeDstar = getObjectPtr<TTree>("Dstar");
  auto ttreeGamma = getObjectPtr<TTree>("Gamma");

  if (ttreeLambda->GetEntries() < m_MinEvtsPerTree) {
    B2WARNING("Not enough data for calibration.");
    return c_NotEnoughData;
  }

  // call the calibration functions
  TH2F hLambdaP = LambdaMassFit(ttreeLambda);
  auto [hDstarK, hDstarPi, hDstarMu] = DstarMassFit(ttreeDstar);
  TH2F hGammaE = GammaHistogram(ttreeGamma);
  std::vector<double> pbins = CreatePBinningScheme();
  TH2F hEmpty("hEmpty", "A histogram returned if we cannot calibrate", m_numPBins, pbins.data(), m_numDEdxBins, 0, m_dedxCutoff);
  for (int pbin = 0; pbin <= m_numPBins + 1; pbin++) {
    for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
      hEmpty.SetBinContent(pbin, dedxbin, 0.01);
    };
  }

  B2INFO("Histograms are ready, proceed to creating the payload object...");
  std::vector<TH2F*> hDedxPDFs(6);

  std::array<std::string, 6> part = {"Electron", "Muon", "Pion", "Kaon", "Proton", "Deuteron"};

  TCanvas* candEdx = new TCanvas("candEdx", "SVD dEdx payloads", 1200, 700);
  candEdx->Divide(3, 2);
  gStyle->SetOptStat(11);

  for (bool trunmean : {false, true}) {
    for (int iPart = 0; iPart < 6; iPart++) {

      if (iPart == 0 && trunmean) {
        hDedxPDFs[iPart] = &hGammaE;
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 1 && trunmean) {
        hDedxPDFs[iPart] = &hDstarMu;
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 2 && trunmean) {
        hDedxPDFs[iPart] = &hDstarPi;
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 3 && trunmean) {
        hDedxPDFs[iPart] = &hDstarK;
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 4 && trunmean) {
        hDedxPDFs[iPart] = &hLambdaP;
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 5 && trunmean) {
        hDedxPDFs[iPart] = &hEmpty;
        hDedxPDFs[iPart]->SetName("hist_d1_1000010020_trunc");
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 0 && !trunmean) {
        hDedxPDFs[iPart] = &hEmpty;
        hDedxPDFs[iPart]->SetName("hist_d1_11");
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 1 && !trunmean) {
        hDedxPDFs[iPart] = &hEmpty;
        hDedxPDFs[iPart]->SetName("hist_d1_13");
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 2 && !trunmean) {
        hDedxPDFs[iPart] = &hEmpty;
        hDedxPDFs[iPart]->SetName("hist_d1_211");
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 3 && !trunmean) {
        hDedxPDFs[iPart] = &hEmpty;
        hDedxPDFs[iPart]->SetName("hist_d1_321");
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 4 && !trunmean) {
        hDedxPDFs[iPart] = &hEmpty;
        hDedxPDFs[iPart]->SetName("hist_d1_2212");
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 5 && !trunmean) {
        hDedxPDFs[iPart] = &hEmpty;
        hDedxPDFs[iPart]->SetName("hist_d1_1000010020");
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      }

      else
        hDedxPDFs[iPart] = &hEmpty;
      payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);

      candEdx->cd(iPart + 1);
      hDedxPDFs[iPart]->SetTitle(Form("%s; p(GeV/c) of %s; dE/dx", hDedxPDFs[iPart]->GetTitle(), part[iPart].data()));
      hDedxPDFs[iPart]->DrawCopy("colz");

      if (m_isMakePlots) {
        candEdx->SaveAs("PlotsSVDdEdxPDFs_wTruncMean.pdf");
      }
    }
    // candEdx->SetTitle(Form("Likehood dist. of charged particles from %s, trunmean = %s", idet.data(), check.str().data()));
  }

  saveCalibration(payload, "SVDdEdxPDFs");
  B2INFO("SVD dE/dx calibration done!");

  return c_OK;
}

TH2F SVDdEdxCalibrationAlgorithm::LambdaMassFit(std::shared_ptr<TTree> preselTree)
{
  B2INFO("Configuring the Lambda fit...");
  gROOT->SetBatch(true);
  RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING);

  RooRealVar InvM("InvM", "m(p^{+}#pi^{-})", 1.1, 1.13, "GeV/c^{2}");

  RooRealVar ProtonMomentum("ProtonMomentum", "momentum for p", -1.e8, 1.e8);
  RooRealVar ProtonSVDdEdx("ProtonSVDdEdx", "", -1.e8, 1.e8);

  RooRealVar exp("exp", "experiment number", 0, 1.e5);
  RooRealVar run("run", "run number", 0, 1.e7);

  auto variables = new RooArgSet();

  variables->add(InvM);

  variables->add(ProtonMomentum);
  variables->add(ProtonSVDdEdx);
  variables->add(exp);
  variables->add(run);

  RooDataSet* LambdaDataset = new RooDataSet("LambdaDataset", "LambdaDataset", preselTree.get(), *variables);

  if (LambdaDataset->sumEntries() == 0) {
    B2FATAL("The Lambda dataset is empty, stopping here");
  }

  // the signal PDF; might be revisited at a later point

  RooRealVar GaussMean("GaussMean", " GaussMean", 1.116, 1.111, 1.12);
  RooRealVar GaussSigma("GaussSigma", "#sigma_{1}", 3.e-3, 3.e-5, 10.e-3);
  RooGaussian LambdaGauss("LambdaGauss", "LambdaGauss", InvM, GaussMean, GaussSigma);

  /* temporary RooRealVar sigmaBifurGaussL1 and sigmaBifurGaussR1 to replace
   * RooRealVar resolutionParamL("resolutionParamL", "resolutionParamL", 0.4, 5.e-4, 1.0);
   * RooRealVar resolutionParamR("resolutionParamR", "resolutionParamR", 0.4, 5.e-4, 1.0);
   * RooFormulaVar sigmaBifurGaussL1("sigmaBifurGaussL1", "resolutionParamL*GaussSigma", RooArgSet(resolutionParamL, GaussSigma));
   * RooFormulaVar sigmaBifurGaussR1("sigmaBifurGaussR1", "resolutionParamR*GaussSigma", RooArgSet(resolutionParamR, GaussSigma));
   */
  RooRealVar sigmaBifurGaussL1("sigmaBifurGaussL1", "sigma left", 0.4 * 3.e-3, 3.e-5, 10.e-3);
  RooRealVar sigmaBifurGaussR1("sigmaBifurGaussR1", "sigma right", 0.4 * 3.e-3, 3.e-5, 10.e-3);
  RooBifurGauss LambdaBifurGauss("LambdaBifurGauss", "LambdaBifurGauss", InvM, GaussMean, sigmaBifurGaussL1, sigmaBifurGaussR1);

  /* temporary RooRealVar sigmaBifurGaussL2 to replace
   * RooRealVar resolutionParam2("resolutionParam2", "resolutionParam2", 0.2, 5.e-4, 1.0);
   * sigmaBifurGaussL2("sigmaBifurGaussL2", "resolutionParam2*GaussSigma", RooArgSet(resolutionParam2, GaussSigma));
   */
  RooRealVar sigmaBifurGaussL2("sigmaBifurGaussL2", "sigmaBifurGaussL2", 0.2 * 3.e-3, 3.e-5, 10.e-3);
  RooGaussian LambdaBifurGauss2("LambdaBifurGauss2", "LambdaBifurGauss2", InvM, GaussMean, sigmaBifurGaussL2);

  RooRealVar fracBifurGaussYield("fracBifurGaussYield", "fracBifurGaussYield", 0.3, 5.e-4, 1.0);
  RooRealVar fracGaussYield("fracGaussYield", "fracGaussYield", 0.8, 5.e-4, 1.0);

  RooAddPdf LambdaCombinedBifurGauss("LambdaCombinedBifurGauss", "LambdaBifurGauss + LambdaBifurGauss2 ", RooArgList(LambdaBifurGauss,
                                     LambdaBifurGauss2), RooArgList(fracBifurGaussYield));

  RooAddPdf LambdaSignalPDF("LambdaSignalPDF", "LambdaCombinedBifurGauss + LambdaGauss", RooArgList(LambdaCombinedBifurGauss,
                            LambdaGauss), RooArgList(fracGaussYield));

  // Background PDF
  RooRealVar BkgPolyCoef0("BkgPolyCoef0", "BkgPolyCoef0", 0.1, 0., 1.5);
  RooRealVar BkgPolyCoef1("BkgPolyCoef1", "BkgPolyCoef1", -0.5, -1.5, -1.e-3);
  RooChebychev BkgPolyPDF("BkgPolyPDF", "BkgPolyPDF", InvM, RooArgList(BkgPolyCoef0, BkgPolyCoef1));

  RooRealVar nSignalLambda("nSignalLambda", "nSignalLambda", 0.6 * preselTree->GetEntries(), 0., 0.99 * preselTree->GetEntries());
  RooRealVar nBkgLambda("nBkgLambda", "nBkgLambda", 0.4 * preselTree->GetEntries(), 0., 0.99 * preselTree->GetEntries());
  RooAddPdf totalPDFLambda("totalPDFLambda", "totalPDFLambda pdf", RooArgList(LambdaSignalPDF, BkgPolyPDF),
                           RooArgList(nSignalLambda, nBkgLambda));

  B2INFO("Lambda: Start fitting...");
  RooFitResult* LambdaFitResult = totalPDFLambda.fitTo(*LambdaDataset, Save(kTRUE), PrintLevel(-1));

  int status = LambdaFitResult->status();
  int covqual = LambdaFitResult->covQual();
  double diff = nSignalLambda.getValV() + nBkgLambda.getValV() - LambdaDataset->sumEntries();

  B2INFO("Lambda: Fit status: " << status << "; covariance quality: " << covqual);
  // if the fit is not healthy, try again once before giving up, with a slightly different setup:
  if ((status > 0) || (TMath::Abs(diff) > 1.) || (nSignalLambda.getError() < sqrt(nSignalLambda.getValV()))
      || (nSignalLambda.getError() > (nSignalLambda.getValV()))) {

    LambdaFitResult = totalPDFLambda.fitTo(*LambdaDataset, Save(), Strategy(2), Offset(1));
    status = LambdaFitResult->status();
    covqual = LambdaFitResult->covQual();
    diff = nSignalLambda.getValV() + nBkgLambda.getValV() - LambdaDataset->sumEntries();
  }

  if ((status > 0) || (TMath::Abs(diff) > 1.) || (nSignalLambda.getError() < sqrt(nSignalLambda.getValV()))
      || (nSignalLambda.getError() > (nSignalLambda.getValV()))) {
    B2WARNING("Lambda: Fit problem: fit status " << status << "; sum of component yields minus the dataset yield is " << diff <<
              "; signal yield is " << nSignalLambda.getValV() << ", while its uncertainty is " << nSignalLambda.getError());
  }
  if (covqual < 2) {
    B2INFO("Lambda: Fit warning: covariance quality " << covqual);
  }

  TCanvas* canvLambda = new TCanvas("canvLambda", "canvLambda");
  RooPlot* LambdaFitFrame = LambdaDataset->plotOn(InvM.frame(130));
  totalPDFLambda.plotOn(LambdaFitFrame, LineColor(TColor::GetColor("#4575b4")));

  double chisquare = LambdaFitFrame->chiSquare();
  B2INFO("Lambda: Fit chi2 = " << chisquare);
  totalPDFLambda.paramOn(LambdaFitFrame, Layout(0.6, 0.96, 0.93), Format("NEU", AutoPrecision(2)));
  LambdaFitFrame->getAttText()->SetTextSize(0.03);

  totalPDFLambda.plotOn(LambdaFitFrame, Components("LambdaSignalPDF"), LineColor(TColor::GetColor("#d73027")));
  totalPDFLambda.plotOn(LambdaFitFrame, Components("BkgPolyPDF"), LineColor(TColor::GetColor("#fc8d59")));
  totalPDFLambda.plotOn(LambdaFitFrame, LineColor(TColor::GetColor("#4575b4")));

  LambdaFitFrame->GetXaxis()->SetTitle("m(p#pi^{-}) (GeV/c^{2})");

  LambdaFitFrame->Draw();

  if (m_isMakePlots) {
    canvLambda->Print("SVDdEdxCalibrationFitLambda.pdf");
    TFile LambdaFitPlotFile("SVDdEdxCalibrationLambdaFitPlotFile.root", "RECREATE");
    canvLambda->Write();
    LambdaFitPlotFile.Close();
  }
  RooStats::SPlot* sPlotDatasetLambda = new RooStats::SPlot("sData", "An SPlot", *LambdaDataset, &totalPDFLambda,
                                                            RooArgList(nSignalLambda, nBkgLambda));

  for (int iEvt = 0; iEvt < 5; iEvt++) {
    if (TMath::Abs(sPlotDatasetLambda->GetSWeight(iEvt, "nSignalLambda") + sPlotDatasetLambda->GetSWeight(iEvt,
                   "nBkgLambda") - 1) > 5.e-3)
      B2FATAL("Lambda: sPlot error: sum of weights not equal to 1");
  }

  RooDataSet* LambdaDatasetSWeighted = new RooDataSet(LambdaDataset->GetName(), LambdaDataset->GetTitle(), LambdaDataset,
                                                      *LambdaDataset->get());

  TTree* treeLambda_sw = LambdaDatasetSWeighted->GetClonedTree();
  treeLambda_sw->SetName("treeLambda_sw");

  B2INFO("Lambda: sPlot done. Proceed to histogramming");

  std::vector<double> pbins = CreatePBinningScheme();

  TH2F* hLambdaP = new TH2F("hist_d1_2212_trunc", "hist_d1_2212_trunc", m_numPBins, pbins.data(), m_numDEdxBins, 0, m_dedxCutoff);

  treeLambda_sw->Draw("ProtonSVDdEdx:ProtonMomentum>>hist_d1_2212_trunc",
                      "nSignalLambda_sw * (ProtonMomentum>0.15) * (ProtonSVDdEdx>0)", "goff");

  // produce the 1D profile (for data-MC comparisons)
  if (m_isMakePlots) {
    TH1F* ProtonProfile = (TH1F*)hLambdaP->ProfileX("ProtonProfile");
    ProtonProfile->SetTitle("ProtonProfile");
    canvLambda->SetTicky(1);
    ProtonProfile->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
    ProtonProfile->GetXaxis()->SetTitle("Momentum, GeV/c");
    ProtonProfile->GetYaxis()->SetTitle("dE/dx");
    ProtonProfile->Draw();
    canvLambda->Print("SVDdEdxCalibrationProfileProton.pdf");
    TFile ProtonProfileFile("SVDdEdxCalibrationProfileProton.root", "RECREATE");
    ProtonProfile->Write();
    ProtonProfileFile.Close();
    canvLambda->SetTicky(0);
  }

  // for each momentum bin, normalize the pdf

  // hLambdaP normalisation
  for (int pbin = 0; pbin <= m_numPBins + 1; pbin++) {
    for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
      // get rid of the bins with negative weights
      if (hLambdaP->GetBinContent(pbin, dedxbin) <= 1) {
        hLambdaP->SetBinContent(pbin, dedxbin, 0);
      };
    }
    // create a projection (1D histogram) in a given momentum bin
    TH1D* slice = (TH1D*)hLambdaP->ProjectionY("slice", pbin, pbin);
    // normalise, but ignore the cases with empty histograms
    if (slice->Integral() > 0) {
      slice->Scale(1. / slice->Integral());
    }
    // fill back the 2D histo with the result
    for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
      hLambdaP->SetBinContent(pbin, dedxbin, slice->GetBinContent(dedxbin));
    }
  }

  if (m_isMakePlots) {
    hLambdaP->Draw("COLZ");
    canvLambda->Print("SVDdEdxCalibrationHistoLambda.pdf");
  }

  return *hLambdaP;
}

std::tuple<TH2F, TH2F, TH2F> SVDdEdxCalibrationAlgorithm::DstarMassFit(std::shared_ptr<TTree> preselTree)
{
  B2INFO("Configuring the Dstar fit...");
  gROOT->SetBatch(true);
  RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING);

  RooRealVar deltaM("deltaM", "m(D*)-m(D^{0})", 0.139545, 0.151, "GeV/c^{2}");

  RooRealVar KaonMomentum("KaonMomentum", "momentum for Kaon(GeV)", -1.e8, 1.e8);
  RooRealVar KaonSVDdEdx("KaonSVDdEdx", "", -1.e8, 1.e8);
  RooRealVar PionDMomentum("PionDMomentum", "momentum for pion(GeV)", -1.e8, 1.e8);
  RooRealVar PionDSVDdEdx("PionDSVDdEdx", "", -1.e8, 1.e8);
  RooRealVar SlowPionMomentum("SlowPionMomentum", "momentum for slow pion(GeV)", -1.e8, 1.e8);
  RooRealVar SlowPionSVDdEdx("SlowPionSVDdEdx", "", -1.e8, 1.e8);

  RooRealVar exp("exp", "experiment number", 0, 1.e5);
  RooRealVar run("run", "run number", 0, 1.e8);

  auto variables = new RooArgSet();
  variables->add(deltaM);
  variables->add(KaonMomentum);
  variables->add(KaonSVDdEdx);
  variables->add(PionDMomentum);
  variables->add(PionDSVDdEdx);
  variables->add(SlowPionMomentum);
  variables->add(SlowPionSVDdEdx);
  variables->add(exp);
  variables->add(run);

  RooDataSet* DstarDataset = new RooDataSet("DstarDataset", "DstarDataset", preselTree.get(), *variables);

  if (DstarDataset->sumEntries() == 0) {
    B2FATAL("The Dstar dataset is empty, stopping here");
  }

  RooPlot* DstarFitFrame = DstarDataset->plotOn(deltaM.frame());

  RooRealVar GaussMean("GaussMean", "GaussMean", 0.145, 0.140, 0.150);
  RooRealVar GaussSigma1("GaussSigma1", "GaussSigma1", 0.01, 1.e-4, 1.0);
  RooGaussian DstarGauss1("DstarGauss1", "DstarGauss1", deltaM, GaussMean, GaussSigma1);
  RooRealVar GaussSigma2("GaussSigma2", "GaussSigma2", 0.001, 1.e-4, 1.0);
  RooGaussian DstarGauss2("DstarGauss2", "DstarGauss2", deltaM, GaussMean, GaussSigma2);
  RooRealVar fracGaussYield("fracGaussYield", "Fraction of two Gaussians", 0.75, 0.0, 1.0);
  RooAddPdf DstarSignalPDF("DstarSignalPDF", "DstarGauss1+DstarGauss2", RooArgList(DstarGauss1, DstarGauss2), fracGaussYield);

  RooRealVar dm0Bkg("dm0Bkg", "dm0", 0.13957018, 0.130, 0.140);
  RooRealVar aBkg("aBkg", "a", -0.0784, -0.08, 3.0);
  RooRealVar bBkg("bBkg", "b", -0.444713, -0.5, 0.4);
  RooRealVar cBkg("cBkg", "c", 0.3);
  RooDstD0BG DstarBkgPDF("DstarBkgPDF", "DstarBkgPDF", deltaM, dm0Bkg, cBkg, aBkg, bBkg);
  RooRealVar nSignalDstar("nSignalDstar", "signal yield", 0.5 * preselTree->GetEntries(), 0, preselTree->GetEntries());
  RooRealVar nBkgDstar("nBkgDstar", "background yield", 0.5 * preselTree->GetEntries(), 0, preselTree->GetEntries());
  RooAddPdf totalPDFDstar("totalPDFDstar", "totalPDFDstar pdf", RooArgList(DstarSignalPDF, DstarBkgPDF),
                          RooArgList(nSignalDstar, nBkgDstar));

  B2INFO("Dstar: Start fitting...");
  RooFitResult* DstarFitResult = totalPDFDstar.fitTo(*DstarDataset, Save(kTRUE), PrintLevel(-1));

  int status = DstarFitResult->status();
  int covqual = DstarFitResult->covQual();
  double diff = nSignalDstar.getValV() + nBkgDstar.getValV() - DstarDataset->sumEntries();

  B2INFO("Dstar: Fit status: " << status << "; covariance quality: " << covqual);
  // if the fit is not healthy, try again once before giving up, with a slightly different setup:
  if ((status > 0) || (TMath::Abs(diff) > 1.) || (nSignalDstar.getError() < sqrt(nSignalDstar.getValV()))
      || (nSignalDstar.getError() > (nSignalDstar.getValV()))) {

    DstarFitResult = totalPDFDstar.fitTo(*DstarDataset, Save(), Strategy(2), Offset(1));
    status = DstarFitResult->status();
    covqual = DstarFitResult->covQual();
    diff = nSignalDstar.getValV() + nBkgDstar.getValV() - DstarDataset->sumEntries();
  }

  if ((status > 0) || (TMath::Abs(diff) > 1.) || (nSignalDstar.getError() < sqrt(nSignalDstar.getValV()))
      || (nSignalDstar.getError() > (nSignalDstar.getValV()))) {
    B2WARNING("Dstar: Fit problem: fit status " << status << "; sum of component yields minus the dataset yield is " << diff <<
              "; signal yield is " << nSignalDstar.getValV() << ", while its uncertainty is " << nSignalDstar.getError());
  }
  if (covqual < 2) {
    B2INFO("Dstar: Fit warning: covariance quality " << covqual);
  }

  totalPDFDstar.plotOn(DstarFitFrame, LineColor(TColor::GetColor("#4575b4")));

  double chisquare = DstarFitFrame->chiSquare();
  B2INFO("Dstar: Fit chi2 = " << chisquare);
  totalPDFDstar.paramOn(DstarFitFrame, Layout(0.63, 0.96, 0.93), Format("NEU", AutoPrecision(2)));
  DstarFitFrame->getAttText()->SetTextSize(0.03);

  totalPDFDstar.plotOn(DstarFitFrame, Components("DstarSignalPDF"), LineColor(TColor::GetColor("#d73027")));
  totalPDFDstar.plotOn(DstarFitFrame, Components("DstarBkgPDF"), LineColor(TColor::GetColor("#fc8d59")));
  totalPDFDstar.plotOn(DstarFitFrame, LineColor(TColor::GetColor("#4575b4")));

  DstarFitFrame->GetXaxis()->SetTitle("#Deltam [GeV/c^{2}]");
  TCanvas* canvDstar = new TCanvas("canvDstar", "canvDstar");
  canvDstar->cd();

  DstarFitFrame->Draw();

  if (m_isMakePlots) {
    canvDstar->Print("SVDdEdxCalibrationFitDstar.pdf");
    TFile DstarFitPlotFile("SVDdEdxCalibrationDstarFitPlotFile.root", "RECREATE");
    canvDstar->Write();
    DstarFitPlotFile.Close();
  }

  /////////////////// SPlot ///////////////////////////////////////////////////////////

  RooStats::SPlot* sPlotDatasetDstar = new RooStats::SPlot("sData", "An SPlot", *DstarDataset, &totalPDFDstar,
                                                           RooArgList(nSignalDstar, nBkgDstar));

  for (int iEvt = 0; iEvt < 5; iEvt++) {
    if (TMath::Abs(sPlotDatasetDstar->GetSWeight(iEvt, "nSignalDstar") + sPlotDatasetDstar->GetSWeight(iEvt, "nBkgDstar") - 1) > 5.e-3)
      B2FATAL("Dstar: sPlot error: sum of weights not equal to 1");
  }

  RooDataSet* DstarDatasetSWeighted = new RooDataSet(DstarDataset->GetName(), DstarDataset->GetTitle(), DstarDataset,
                                                     *DstarDataset->get());

  TTree* treeDstar_sw = DstarDatasetSWeighted->GetClonedTree();
  treeDstar_sw->SetName("treeDstar_sw");

  B2INFO("Dstar: sPlot done. Proceed to histogramming");

  std::vector<double> pbins = CreatePBinningScheme();

  // the kaon payload
  TH2F* hDstarK = new TH2F("hist_d1_321_trunc", "hist_d1_321_trunc", m_numPBins, pbins.data(),
                           m_numDEdxBins, 0, m_dedxCutoff);
  // the pion payload
  TH2F* hDstarPi = new TH2F("hist_d1_211_trunc", "hist_d1_211_trunc", m_numPBins, pbins.data(),
                            m_numDEdxBins, 0, m_dedxCutoff);

  treeDstar_sw->Draw("KaonSVDdEdx:KaonMomentum>>hist_d1_321_trunc", "nSignalDstar_sw * (KaonSVDdEdx>0)", "goff");
  // the pion one will be built from both pions in the Dstar decay tree
  TH2F* hDstarPiPart1 = (TH2F*)hDstarPi->Clone("hist_d1_211_truncPart1");
  TH2F* hDstarPiPart2 = (TH2F*)hDstarPi->Clone("hist_d1_211_truncPart2");

  treeDstar_sw->Draw("PionDSVDdEdx:PionDMomentum>>hist_d1_211_truncPart1", "nSignalDstar_sw * (PionDSVDdEdx>0)", "goff");
  treeDstar_sw->Draw("SlowPionSVDdEdx:SlowPionMomentum>>hist_d1_211_truncPart2", "nSignalDstar_sw * (SlowPionSVDdEdx>0)", "goff");
  hDstarPi->Add(hDstarPiPart1);
  hDstarPi->Add(hDstarPiPart2);

  // the current strategy assumes that the muon and pion payloads are indistinguishable: clone the pion one
  TH2F* hDstarMu = (TH2F*)hDstarPi->Clone("hist_d1_13_trunc");
  hDstarMu->SetTitle("hist_d1_13_trunc");

  // produce the 1D profile (for data-MC comparisons)
  if (m_isMakePlots) {
    TH1F* PionProfile = (TH1F*)hDstarPi->ProfileX("PionProfile");
    PionProfile->SetTitle("PionProfile");
    canvDstar->SetTicky(1);
    PionProfile->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
    PionProfile->GetXaxis()->SetTitle("Momentum, GeV/c");
    PionProfile->GetYaxis()->SetTitle("dE/dx");
    PionProfile->Draw();
    canvDstar->Print("SVDdEdxCalibrationProfilePion.pdf");
    TFile PionProfileFile("SVDdEdxCalibrationProfilePion.root", "RECREATE");
    PionProfile->Write();
    PionProfileFile.Close();

    TH1F* KaonProfile = (TH1F*)hDstarK->ProfileX("KaonProfile");
    KaonProfile->SetTitle("KaonProfile");
    KaonProfile->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
    KaonProfile->GetXaxis()->SetTitle("Momentum, GeV/c");
    KaonProfile->GetYaxis()->SetTitle("dE/dx");
    KaonProfile->Draw();
    canvDstar->Print("SVDdEdxCalibrationProfileKaon.pdf");
    TFile KaonProfileFile("SVDdEdxCalibrationProfileKaon.root", "RECREATE");
    KaonProfile->Write();
    KaonProfileFile.Close();
    canvDstar->SetTicky(0);
  }

  // hDstarK normalisation
  // for each momentum bin, normalize the pdf

  for (int pbin = 0; pbin <= m_numPBins + 1; pbin++) {
    for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
      // get rid of the bins with negative weights
      if (hDstarK->GetBinContent(pbin, dedxbin) <= 1) {
        hDstarK->SetBinContent(pbin, dedxbin, 0);
      };
    }
    // create a projection (1D histogram) in a given momentum bin
    TH1D* slice = (TH1D*)hDstarK->ProjectionY("slice", pbin, pbin);
    // normalise, but ignore the cases with empty histograms
    if (slice->Integral() > 0) {
      slice->Scale(1. / slice->Integral());
    }
    // fill back the 2D histo with the result
    for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
      hDstarK->SetBinContent(pbin, dedxbin, slice->GetBinContent(dedxbin));
    }
  }

  // hDstarPi normalisation
  for (int pbin = 0; pbin <= m_numPBins + 1; pbin++) {
    for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
      // get rid of the bins with negative weights
      if (hDstarPi->GetBinContent(pbin, dedxbin) <= 1) {
        hDstarPi->SetBinContent(pbin, dedxbin, 0);
      };
    }
    // create a projection (1D histogram) in a given momentum bin
    TH1D* slice = (TH1D*)hDstarPi->ProjectionY("slice", pbin, pbin);
    // normalise, but ignore the cases with empty histograms
    if (slice->Integral() > 0) {
      slice->Scale(1. / slice->Integral());
    }
    // fill back the 2D histo with the result
    for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
      hDstarPi->SetBinContent(pbin, dedxbin, slice->GetBinContent(dedxbin));
    }
  }

  // hDstarMu normalisation
  for (int pbin = 0; pbin <= m_numPBins + 1; pbin++) {
    for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
      // get rid of the bins with negative weights
      if (hDstarMu->GetBinContent(pbin, dedxbin) <= 1) {
        hDstarMu->SetBinContent(pbin, dedxbin, 0);
      };
    }
    // create a projection (1D histogram) in a given momentum bin
    TH1D* slice = (TH1D*)hDstarMu->ProjectionY("slice", pbin, pbin);
    // normalise, but ignore the cases with empty histograms
    if (slice->Integral() > 0) {
      slice->Scale(1. / slice->Integral());
    }
    // fill back the 2D histo with the result
    for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
      hDstarMu->SetBinContent(pbin, dedxbin, slice->GetBinContent(dedxbin));
    }
  }
  if (m_isMakePlots) {
    hDstarK->Draw("COLZ");
    canvDstar->Print("SVDdEdxCalibrationHistoDstarK.pdf");
    hDstarPi->Draw("COLZ");
    canvDstar->Print("SVDdEdxCalibrationHistoDstarPi.pdf");
    hDstarMu->Draw("COLZ");
    canvDstar->Print("SVDdEdxCalibrationHistoDstarMu.pdf");
  }

  return std::make_tuple(*hDstarK, *hDstarPi, *hDstarMu);
}

TH2F SVDdEdxCalibrationAlgorithm::GammaHistogram(std::shared_ptr<TTree> preselTree)
{
  B2INFO("Histogramming the converted photon selection...");
  gROOT->SetBatch(true);

  if (preselTree->GetEntries() == 0) {
    B2FATAL("The Gamma tree is empty, stopping here");
  }
  std::vector<double> pbins = CreatePBinningScheme();

  TH2F* hGammaE = new TH2F("hist_d1_11_trunc", "hist_d1_11_trunc", m_numPBins, pbins.data(), m_numDEdxBins, 0, m_dedxCutoff);

  TH2F* hGammaEPart1 = (TH2F*)hGammaE->Clone("hist_d1_11_truncPart1");
  TH2F* hGammaEPart2 = (TH2F*)hGammaE->Clone("hist_d1_11_truncPart2");

  preselTree->Draw("FirstElectronSVDdEdx:FirstElectronMomentum>>hist_d1_11_truncPart1", "FirstElectronSVDdEdx>0", "goff");
  preselTree->Draw("SecondElectronSVDdEdx:SecondElectronMomentum>>hist_d1_11_truncPart2", "SecondElectronSVDdEdx>0", "goff");
  hGammaE->Add(hGammaEPart1);
  hGammaE->Add(hGammaEPart2);

  // produce the 1D profile (for data-MC comparisons)
  TCanvas* canvGamma = new TCanvas("canvGamma", "canvGamma");
  if (m_isMakePlots) {
    TH1F* ElectronProfile = (TH1F*)hGammaE->ProfileX("ElectronProfile");
    ElectronProfile->SetTitle("ElectronProfile");
    canvGamma->SetTicky(1);
    ElectronProfile->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
    ElectronProfile->GetXaxis()->SetTitle("Momentum, GeV/c");
    ElectronProfile->GetYaxis()->SetTitle("dE/dx");
    ElectronProfile->Draw();
    canvGamma->Print("SVDdEdxCalibrationProfileElectron.pdf");
    TFile ElectronProfileFile("SVDdEdxCalibrationProfileElectron.root", "RECREATE");
    ElectronProfile->Write();
    ElectronProfileFile.Close();
    canvGamma->SetTicky(0);
  }

  // for each momentum bin, normalize the pdf
  // hGammaE normalisation
  for (int pbin = 0; pbin <= m_numPBins + 1; pbin++) {
    for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
      // get rid of the bins with negative weights
      if (hGammaE->GetBinContent(pbin, dedxbin) <= 1) {
        hGammaE->SetBinContent(pbin, dedxbin, 0);
      };
    }

    // create a projection (1D histogram) in a given momentum bin
    TH1D* slice = (TH1D*)hGammaE->ProjectionY("slice", pbin, pbin);
    // normalise, but ignore the cases with empty histograms
    if (slice->Integral() > 0) {
      slice->Scale(1. / slice->Integral());
    }
    // fill back the 2D histo with the result
    for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
      hGammaE->SetBinContent(pbin, dedxbin, slice->GetBinContent(dedxbin));
    }
  }

  if (m_isMakePlots) {
    hGammaE->Draw("COLZ");
    canvGamma->Print("SVDdEdxCalibrationHistoGamma.pdf");
  }

  return *hGammaE;
}
