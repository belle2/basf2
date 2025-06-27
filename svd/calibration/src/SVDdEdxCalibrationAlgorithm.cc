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
#include <TFitResult.h>
#include <TKDTreeBinning.h>

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
#include <Math/MinimizerOptions.h>

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
  auto ttreeGeneric = getObjectPtr<TTree>("Generic");

  if (ttreeLambda->GetEntries() < m_MinEvtsPerTree) {
    B2WARNING("Not enough data for calibration.");
    return c_NotEnoughData;
  }

  // call the calibration function
  TList* GeneratedList = GenerateNewHistograms(ttreeLambda, ttreeDstar, ttreeGamma, ttreeGeneric);

  TH2F* histoE = (TH2F*) GeneratedList->FindObject("Electron2DHistogramNew");
  TH2F* histoMu = (TH2F*) GeneratedList->FindObject("Muon2DHistogramNew");
  TH2F* histoPi = (TH2F*) GeneratedList->FindObject("Pion2DHistogramNew");
  TH2F* histoK = (TH2F*) GeneratedList->FindObject("Kaon2DHistogramNew");
  TH2F* histoP = (TH2F*) GeneratedList->FindObject("Proton2DHistogramNew");
  TH2F* histoDeut = (TH2F*) GeneratedList->FindObject("Deuteron2DHistogramNew");

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
        hDedxPDFs[iPart] = histoE;
        hDedxPDFs[iPart]->SetName("hist_d1_11_trunc");
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 1 && trunmean) {
        hDedxPDFs[iPart] = histoMu;
        hDedxPDFs[iPart]->SetName("hist_d1_13_trunc");
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 2 && trunmean) {
        hDedxPDFs[iPart] = histoPi;
        hDedxPDFs[iPart]->SetName("hist_d1_211_trunc");
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 3 && trunmean) {
        hDedxPDFs[iPart] = histoK;
        hDedxPDFs[iPart]->SetName("hist_d1_321_trunc");
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 4 && trunmean) {
        hDedxPDFs[iPart] = histoP;
        hDedxPDFs[iPart]->SetName("hist_d1_2212_trunc");
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 5 && trunmean) {
        hDedxPDFs[iPart] = histoDeut;
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
    }

    if (m_isMakePlots) {
      candEdx->SaveAs("PlotsSVDdEdxPDFs_wTruncMean.pdf");
      TList* l = new TList();
      for (int iPart = 0; iPart < 6; iPart++) {
        l->Add(hDedxPDFs[iPart]);
      }
      TFile SVDdEdxPDFsPlotFile("PlotsSVDdEdxPDFs_wTruncMean.root", "RECREATE");
      l->Write("histlist", TObject::kSingleKey);
      SVDdEdxPDFsPlotFile.Close();
    }

    // candEdx->SetTitle(Form("Likehood dist. of charged particles from %s, trunmean = %s", idet.data(), check.str().data()));
  }

  saveCalibration(payload, "SVDdEdxPDFs");
  B2INFO("SVD dE/dx calibration done!");

  return c_OK;
}

TTree* SVDdEdxCalibrationAlgorithm::LambdaMassFit(std::shared_ptr<TTree> preselTree)
{
  B2INFO("Configuring the Lambda fit...");
  gROOT->SetBatch(true);
  RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING);

  RooRealVar InvM("InvM", "m(p^{+}#pi^{-})", 1.1, 1.13, "GeV/c^{2}");

  RooRealVar ProtonMomentum("ProtonMomentum", "momentum for p", -1.e8, 1.e8);
  RooRealVar ProtonSVDdEdxTrackMomentum("ProtonSVDdEdxTrackMomentum", "momentum for p", -1.e8, 1.e8);
  RooRealVar ProtonSVDdEdx("ProtonSVDdEdx", "", -1.e8, 1.e8);
  RooRealVar ProtonSVDdEdxTrackCosTheta("ProtonSVDdEdxTrackCosTheta", "", -10., 10.);

  RooRealVar exp("exp", "experiment number", 0, 1.e5);
  RooRealVar run("run", "run number", 0, 1.e7);

  auto variables = new RooArgSet();

  variables->add(InvM);

  variables->add(ProtonMomentum);
  variables->add(ProtonSVDdEdxTrackMomentum);
  variables->add(ProtonSVDdEdx);
  variables->add(ProtonSVDdEdxTrackCosTheta);
  variables->add(exp);
  variables->add(run);

  RooDataSet* LambdaDataset = new RooDataSet("LambdaDataset", "LambdaDataset", *variables, Import(*preselTree));

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
    B2INFO("Lambda: updated fit status: " << status << "; covariance quality: " << covqual);
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
  canvLambda->cd();
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

  TTree* treeLambdaSWeighted = LambdaDatasetSWeighted->GetClonedTree();
  treeLambdaSWeighted->SetName("treeLambdaSWeighted");

  B2INFO("Lambda: sPlot done. Proceed to histogramming");
  return treeLambdaSWeighted;
}

TList* SVDdEdxCalibrationAlgorithm::LambdaHistogramming(TTree* inputTree)
{
  gROOT->SetBatch(true);
  inputTree->SetEstimate(-1);
  std::vector<double> pbins = CreatePBinningScheme();

  TH2F* hLambdaPMomentum = new TH2F("hist_d1_2212_truncMomentum", "hist_d1_2212_trunc;Momentum [GeV/c];dEdx [arb. units]",
                                    m_numPBins, pbins.data(), m_numDEdxBins, 0,
                                    m_dedxCutoff);

  inputTree->Draw("ProtonSVDdEdx:ProtonSVDdEdxTrackMomentum>>hist_d1_2212_truncMomentum",
                  "nSignalLambda_sw * (ProtonSVDdEdx>0) * (ProtonSVDdEdxTrackMomentum>0.13)", "goff");

// create isopopulated beta*gamma binning
  inputTree->Draw(Form("ProtonSVDdEdxTrackMomentum/%f", m_ProtonPDGMass), "", "goff",
                  ((inputTree->GetEntries()) / m_numBGBins)*m_numBGBins);
  double* ProtonMomentumDataset = inputTree->GetV1();

  TKDTreeBinning* kdBinsP = new TKDTreeBinning(((inputTree->GetEntries()) / m_numBGBins)*m_numBGBins, 1, ProtonMomentumDataset,
                                               m_numBGBins);
  const double* binsMinEdgesP_pointer = kdBinsP->SortOneDimBinEdges();
  double* binsMinEdgesP =  const_cast<double*>(binsMinEdgesP_pointer);


  binsMinEdgesP[0] = 0.1;
  binsMinEdgesP[m_numBGBins + 1] = 50.;


  TH2F* hLambdaPBetaGamma = new TH2F("hist_d1_2212_truncBetaGamma", "hist_d1_2212_truncBetaGamma;#beta*#gamma;dEdx [arb. units]",
                                     m_numBGBins, binsMinEdgesP, m_numDEdxBins,
                                     0,
                                     m_dedxMaxPossible);

  inputTree->Draw(Form("ProtonSVDdEdx:ProtonSVDdEdxTrackMomentum/%f>>hist_d1_2212_truncBetaGamma", m_ProtonPDGMass),
                  "nSignalLambda_sw * (ProtonSVDdEdx>0) * (ProtonSVDdEdxTrackMomentum>0.13) * (ProtonSVDdEdx>1.2e6 - 1.e6*ProtonSVDdEdxTrackMomentum)",
                  "goff");

  // produce the 1D profile
  // momentum: for data-MC comparisons

  TCanvas* canvLambda = new TCanvas("canvLambda", "canvLambda");
  canvLambda->cd();


  TH1F* ProtonProfileMomentum = (TH1F*)hLambdaPMomentum->ProfileX("ProtonProfileMomentum");
  ProtonProfileMomentum->SetTitle("ProtonProfile");
  ProtonProfileMomentum->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
  ProtonProfileMomentum->GetXaxis()->SetTitle("Momentum, GeV/c");
  ProtonProfileMomentum->GetYaxis()->SetTitle("dE/dx");
  ProtonProfileMomentum->SetLineColor(kRed);
  ProtonProfileMomentum->Draw();

  TH1F* ProtonProfileBetaGamma = (TH1F*)hLambdaPBetaGamma->ProfileX("ProtonProfileBetaGamma");
  if (m_CustomProfile) {
    ProtonProfileBetaGamma = PrepareProfile(hLambdaPBetaGamma, "ProtonProfileBetaGamma");
  }
  ProtonProfileBetaGamma->SetTitle("ProtonProfile");
  ProtonProfileBetaGamma->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
  ProtonProfileBetaGamma->GetXaxis()->SetTitle("#beta*#gamma");
  ProtonProfileBetaGamma->GetYaxis()->SetTitle("dE/dx");
  ProtonProfileBetaGamma->SetLineColor(kRed);


  // for each momentum bin, normalize the pdf
  hLambdaPMomentum = Normalise2DHisto(hLambdaPMomentum);

  TList* histList = new TList;
  histList->Add(ProtonProfileMomentum);
  histList->Add(ProtonProfileBetaGamma);
  histList->Add(hLambdaPMomentum);

  if (m_isMakePlots) {
    TFile LambdaHistogrammingPlotFile("SVDdEdxCalibrationLambdaHistogramming.root", "RECREATE");
    histList->Write();
    LambdaHistogrammingPlotFile.Close();
  }

  return histList;
}

TTree* SVDdEdxCalibrationAlgorithm::DstarMassFit(std::shared_ptr<TTree> preselTree)
{
  B2INFO("Configuring the Dstar fit...");
  gROOT->SetBatch(true);
  RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING);

  RooRealVar deltaM("deltaM", "m(D*)-m(D^{0})", 0.139545, 0.151, "GeV/c^{2}");

  RooRealVar KaonMomentum("KaonMomentum", "momentum for Kaon (GeV)", -1.e8, 1.e8);
  RooRealVar KaonSVDdEdxTrackMomentum("KaonSVDdEdxTrackMomentum", "momentum for Kaon (GeV), from the track", -1.e8, 1.e8);
  RooRealVar KaonSVDdEdx("KaonSVDdEdx", "", -1.e8, 1.e8);
  RooRealVar PionDMomentum("PionDMomentum", "momentum for pion (GeV)", -1.e8, 1.e8);
  RooRealVar PionDSVDdEdxTrackMomentum("PionDSVDdEdxTrackMomentum", "momentum for pion (GeV), from the track", -1.e8, 1.e8);
  RooRealVar PionDSVDdEdx("PionDSVDdEdx", "", -1.e8, 1.e8);
  RooRealVar SlowPionMomentum("SlowPionMomentum", "momentum for slow pion (GeV)", -1.e8, 1.e8);
  RooRealVar SlowPionSVDdEdxTrackMomentum("SlowPionSVDdEdxTrackMomentum", "momentum for slow pion (GeV), from the track", -1.e8,
                                          1.e8);
  RooRealVar SlowPionSVDdEdx("SlowPionSVDdEdx", "", -1.e8, 1.e8);

  RooRealVar exp("exp", "experiment number", 0, 1.e5);
  RooRealVar run("run", "run number", 0, 1.e8);
  RooRealVar event("event", "event number", 0, 1.e10);

  auto variables = new RooArgSet();
  variables->add(deltaM);
  variables->add(KaonMomentum);
  variables->add(KaonSVDdEdxTrackMomentum);
  variables->add(KaonSVDdEdx);
  variables->add(PionDMomentum);
  variables->add(PionDSVDdEdxTrackMomentum);
  variables->add(PionDSVDdEdx);
  variables->add(SlowPionMomentum);
  variables->add(SlowPionSVDdEdxTrackMomentum);
  variables->add(SlowPionSVDdEdx);
  variables->add(exp);
  variables->add(run);
  variables->add(event);

  RooDataSet* DstarDataset = new RooDataSet("DstarDataset", "DstarDataset", *variables, Import(*preselTree));

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
    B2INFO("Dstar: Updated fit status: " << status << "; covariance quality: " << covqual);
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

  TTree* treeDstarSWeighted = DstarDatasetSWeighted->GetClonedTree();
  treeDstarSWeighted->SetName("treeDstarSWeighted");

  B2INFO("Dstar: sPlot done. Proceed to histogramming");
  return treeDstarSWeighted;
}

TList* SVDdEdxCalibrationAlgorithm::DstarHistogramming(TTree* inputTree)
{
  gROOT->SetBatch(true);
  inputTree->SetEstimate(-1);
  std::vector<double> pbins = CreatePBinningScheme();

  TH2F* hDstarKMomentum = new TH2F("hist_d1_321_truncMomentum", "hist_d1_321_trunc;Momentum [GeV/c];dEdx [arb. units]", m_numPBins,
                                   pbins.data(),
                                   m_numDEdxBins, 0, m_dedxCutoff);
  // the pion payload
  TH2F* hDstarPiMomentum = new TH2F("hist_d1_211_truncMomentum", "hist_d1_211_trunc;Momentum [GeV/c];dEdx [arb. units]", m_numPBins,
                                    pbins.data(),
                                    m_numDEdxBins, 0, m_dedxCutoff);

  inputTree->Draw("KaonSVDdEdx:KaonSVDdEdxTrackMomentum>>hist_d1_321_truncMomentum", "nSignalDstar_sw * (KaonSVDdEdx>0)", "goff");
  // the pion one will be built from both pions in the Dstar decay tree
  TH2F* hDstarPiPart1Momentum = (TH2F*)hDstarPiMomentum->Clone("hist_d1_211_truncPart1Momentum");
  TH2F* hDstarPiPart2Momentum = (TH2F*)hDstarPiMomentum->Clone("hist_d1_211_truncPart2Momentum");

  inputTree->Draw("PionDSVDdEdx:PionDSVDdEdxTrackMomentum>>hist_d1_211_truncPart1Momentum", "nSignalDstar_sw * (PionDSVDdEdx>0)",
                  "goff");
  inputTree->Draw("SlowPionSVDdEdx:SlowPionSVDdEdxTrackMomentum>>hist_d1_211_truncPart2Momentum",
                  "nSignalDstar_sw * (SlowPionSVDdEdx>0)",
                  "goff");
  hDstarPiMomentum->Add(hDstarPiPart1Momentum);
  hDstarPiMomentum->Add(hDstarPiPart2Momentum);

  inputTree->Draw(Form("KaonSVDdEdxTrackMomentum/%f", m_KaonPDGMass), "", "goff",
                  ((inputTree->GetEntries()) / m_numBGBins)*m_numBGBins);
  double* KaonMomentumDataset = inputTree->GetV1();
  TKDTreeBinning* kdBinsK = new TKDTreeBinning(((inputTree->GetEntries()) / m_numBGBins)*m_numBGBins, 1, KaonMomentumDataset,
                                               m_numBGBins);
  const double* binsMinEdgesKOriginal = kdBinsK->SortOneDimBinEdges();
  double* binsMinEdgesK =  const_cast<double*>(binsMinEdgesKOriginal);
  binsMinEdgesK[0] = 0.1;
  binsMinEdgesK[m_numBGBins + 1] = 50.;

// get a distribution that contains both pions to get a typical kinematics for the binning scheme
  inputTree->Draw(Form("SlowPionSVDdEdxTrackMomentum/%f * (event %% 2 == 0) + PionDSVDdEdxTrackMomentum/%f * (event %% 2 ==1)",
                       m_PionPDGMass, m_PionPDGMass), "", "goff",
                  ((inputTree->GetEntries()) / m_numBGBins)*m_numBGBins);
  double* PionMomentumDataset = inputTree->GetV1();

  TKDTreeBinning* kdBinsPi = new TKDTreeBinning(((inputTree->GetEntries()) / m_numBGBins)*m_numBGBins, 1, PionMomentumDataset,
                                                m_numBGBins);
  const double* binsMinEdgesPiOriginal = kdBinsPi->SortOneDimBinEdges();
  double* binsMinEdgesPi =  const_cast<double*>(binsMinEdgesPiOriginal);
  binsMinEdgesPi[0] = 0.1;
  binsMinEdgesPi[m_numBGBins + 1] = 50.;

  TH2F* hDstarKBetaGamma = new TH2F("hist_d1_321_truncBetaGamma", "hist_d1_321_truncBetaGamma;#beta*#gamma;dEdx [arb. units]",
                                    m_numBGBins,
                                    binsMinEdgesK,
                                    m_numDEdxBins, 0, m_dedxMaxPossible);
  // the pion payload
  TH2F* hDstarPiBetaGamma = new TH2F("hist_d1_211_truncBetaGamma", "hist_d1_211_truncBetaGamma;#beta*#gamma;dEdx [arb. units]",
                                     m_numBGBins,
                                     binsMinEdgesPi,
                                     m_numDEdxBins, 0, m_dedxMaxPossible);

  inputTree->Draw(Form("KaonSVDdEdx:KaonSVDdEdxTrackMomentum/%f>>hist_d1_321_truncBetaGamma", m_KaonPDGMass),
                  "nSignalDstar_sw * (KaonSVDdEdx>0)", "goff");
  // the pion one will be built from both pions in the Dstar decay tree
  TH2F* hDstarPiPart1BetaGamma = (TH2F*)hDstarPiBetaGamma->Clone("hist_d1_211_truncPart1BetaGamma");
  TH2F* hDstarPiPart2BetaGamma = (TH2F*)hDstarPiBetaGamma->Clone("hist_d1_211_truncPart2BetaGamma");

  inputTree->Draw(Form("PionDSVDdEdx:PionDSVDdEdxTrackMomentum/%f>>hist_d1_211_truncPart1BetaGamma", m_PionPDGMass),
                  "nSignalDstar_sw * (PionDSVDdEdx>0)",
                  "goff");
  inputTree->Draw(Form("SlowPionSVDdEdx:SlowPionSVDdEdxTrackMomentum/%f>>hist_d1_211_truncPart2BetaGamma", m_PionPDGMass),
                  "nSignalDstar_sw * (SlowPionSVDdEdx>0)", "goff");
  hDstarPiBetaGamma->Add(hDstarPiPart1BetaGamma);
  hDstarPiBetaGamma->Add(hDstarPiPart2BetaGamma);



  TCanvas* canvDstar = new TCanvas("canvDstar2", "canvDstar");
  canvDstar->cd();
  // produce the 1D profiled


  TH1F* PionProfileMomentum = (TH1F*)hDstarPiMomentum->ProfileX("PionProfileMomentum");
  PionProfileMomentum->SetTitle("PionProfile");
  // canvDstar->SetTicky(1);
  PionProfileMomentum->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
  PionProfileMomentum->GetXaxis()->SetTitle("Momentum, GeV/c");
  PionProfileMomentum->GetYaxis()->SetTitle("dE/dx");
  PionProfileMomentum->SetLineColor(kRed);
  PionProfileMomentum->Draw();
  // canvDstar->Print("SVDdEdxCalibrationProfilePion.pdf");

  TH1F* PionProfileBetaGamma = (TH1F*)hDstarPiBetaGamma->ProfileX("PionProfileBetaGamma");
  if (m_CustomProfile) {
    PionProfileBetaGamma = PrepareProfile(hDstarPiBetaGamma, "PionProfileBetaGamma");
  }
  PionProfileBetaGamma->SetTitle("PionProfile");
  PionProfileBetaGamma->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
  PionProfileBetaGamma->GetXaxis()->SetTitle("#beta*#gamma");
  PionProfileBetaGamma->GetYaxis()->SetTitle("dE/dx");
  PionProfileBetaGamma->SetLineColor(kRed);


  TH1F* KaonProfileMomentum = (TH1F*)hDstarKMomentum->ProfileX("KaonProfileMomentum");
  KaonProfileMomentum->SetTitle("KaonProfile");
  KaonProfileMomentum->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
  KaonProfileMomentum->GetXaxis()->SetTitle("Momentum, GeV/c");
  KaonProfileMomentum->GetYaxis()->SetTitle("dE/dx");
  KaonProfileMomentum->SetLineColor(kRed);
  KaonProfileMomentum->Draw();
  // canvDstar->Print("SVDdEdxCalibrationProfileKaon.pdf");


  TH1F* KaonProfileBetaGamma = (TH1F*)hDstarKBetaGamma->ProfileX("KaonProfileBetaGamma");
  if (m_CustomProfile) {
    KaonProfileBetaGamma = PrepareProfile(hDstarKBetaGamma, "KaonProfileBetaGamma");
  }
  KaonProfileBetaGamma->SetTitle("KaonProfile");

  KaonProfileBetaGamma->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
  KaonProfileBetaGamma->GetXaxis()->SetTitle("#beta*#gamma");
  KaonProfileBetaGamma->GetYaxis()->SetTitle("dE/dx");
  KaonProfileBetaGamma->SetLineColor(kRed);



  //  normalisation
  hDstarKMomentum->Sumw2();
  hDstarKMomentum = Normalise2DHisto(hDstarKMomentum);

  hDstarPiMomentum->Sumw2();
  hDstarPiMomentum = Normalise2DHisto(hDstarPiMomentum);

  TList* histList = new TList;
  histList->Add(KaonProfileMomentum);
  histList->Add(KaonProfileBetaGamma);
  histList->Add(hDstarKMomentum);

  histList->Add(PionProfileMomentum);
  histList->Add(PionProfileBetaGamma);
  histList->Add(hDstarPiMomentum);

  if (m_isMakePlots) {
    TFile DstarHistogrammingPlotFile("SVDdEdxCalibrationDstarHistogramming.root", "RECREATE");
    histList->Write();
    DstarHistogrammingPlotFile.Close();
  }

  return histList;
}


TList* SVDdEdxCalibrationAlgorithm::GammaHistogramming(std::shared_ptr<TTree> preselTree)
{
  B2INFO("Histogramming the converted photon selection...");
  gROOT->SetBatch(true);


  if (preselTree->GetEntries() == 0) {
    B2FATAL("The Gamma tree is empty, stopping here");
  }
  preselTree->SetEstimate(-1);
  std::vector<double> pbins = CreatePBinningScheme();


  TH2F* hGammaEMomentum = new TH2F("hist_d1_11_truncMomentum", "hist_d1_11_trunc;Momentum [GeV/c];dEdx [arb. units]", m_numPBins,
                                   pbins.data(), m_numDEdxBins, 0, m_dedxCutoff);

  TH2F* hGammaEPart1Momentum = (TH2F*)hGammaEMomentum->Clone("hist_d1_11_truncPart1Momentum");
  TH2F* hGammaEPart2Momentum = (TH2F*)hGammaEMomentum->Clone("hist_d1_11_truncPart2Momentum");

  preselTree->Draw("FirstElectronSVDdEdx:FirstElectronSVDdEdxTrackMomentum>>hist_d1_11_truncPart1Momentum",
                   "FirstElectronSVDdEdx>0 && DIRA>0.995 && dr>1.2", "goff");
  preselTree->Draw("SecondElectronSVDdEdx:SecondElectronSVDdEdxTrackMomentum>>hist_d1_11_truncPart2Momentum",
                   "SecondElectronSVDdEdx>0 && DIRA>0.995 && dr>1.2", "goff");
  hGammaEMomentum->Add(hGammaEPart1Momentum);
  hGammaEMomentum->Add(hGammaEPart2Momentum);

// get a distribution that contains both pions to get a typical kinematics for the binning scheme
  preselTree->Draw(
    Form("FirstElectronSVDdEdxTrackMomentum/%f* (event %% 2==0) + SecondElectronSVDdEdxTrackMomentum/%f* (event %% 2==1)",
         m_ElectronPDGMass, m_ElectronPDGMass),
    "", "goff", ((preselTree->GetEntries()) / m_numBGBins)*m_numBGBins);
  double* ElectronMomentumDataset = preselTree->GetV1();

  TKDTreeBinning* kdBinsE = new TKDTreeBinning(((preselTree->GetEntries()) / m_numBGBins)*m_numBGBins, 1, ElectronMomentumDataset,
                                               m_numBGBins);
  const double* binsMinEdgesEOriginal = kdBinsE->SortOneDimBinEdges();
  double* binsMinEdgesE =  const_cast<double*>(binsMinEdgesEOriginal);
  binsMinEdgesE[0] = 0.;
  binsMinEdgesE[m_numBGBins + 1] = 10000.;


  TH2F* hGammaEBetaGamma = new TH2F("hist_d1_11_truncBetaGamma", "hist_d1_11_truncBetaGamma;#beta*#gamma;dEdx [arb. units]",
                                    m_numBGBins,
                                    binsMinEdgesE,
                                    m_numDEdxBins, 0, m_dedxMaxPossible);
  TH2F* hGammaEPart1BetaGamma = (TH2F*)hGammaEBetaGamma->Clone("hist_d1_11_truncPart1BetaGamma");
  TH2F* hGammaEPart2BetaGamma = (TH2F*)hGammaEBetaGamma->Clone("hist_d1_11_truncPart2BetaGamma");

  preselTree->Draw(Form("FirstElectronSVDdEdx:FirstElectronSVDdEdxTrackMomentum/%f>>hist_d1_11_truncPart1BetaGamma",
                        m_ElectronPDGMass),
                   "FirstElectronSVDdEdx>0 && DIRA>0.995 && dr>1.2 && FirstElectronSVDdEdx<1.8e6", "goff");
  preselTree->Draw(Form("SecondElectronSVDdEdx:SecondElectronSVDdEdxTrackMomentum/%f>>hist_d1_11_truncPart2BetaGamma",
                        m_ElectronPDGMass),
                   "SecondElectronSVDdEdx>0 && DIRA>0.995 && dr>1.2 && SecondElectronSVDdEdx<1.8e6", "goff");
  hGammaEBetaGamma->Add(hGammaEPart1BetaGamma);
  hGammaEBetaGamma->Add(hGammaEPart2BetaGamma);


  // produce the 1D profile (for data-MC comparisons)
  TCanvas* canvGamma = new TCanvas("canvGamma", "canvGamma");
  canvGamma->cd();


  TH1F* ElectronProfileMomentum = (TH1F*)hGammaEMomentum->ProfileX("ElectronProfileMomentum");
  ElectronProfileMomentum->SetTitle("ElectronProfile");
  ElectronProfileMomentum->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
  ElectronProfileMomentum->GetXaxis()->SetTitle("Momentum, GeV/c");
  ElectronProfileMomentum->GetYaxis()->SetTitle("dE/dx");
  ElectronProfileMomentum->SetLineColor(kRed);
  ElectronProfileMomentum->Draw();


  TH1F* ElectronProfileBetaGamma = (TH1F*)hGammaEBetaGamma->ProfileX("ElectronProfileBetaGamma");
  if (m_CustomProfile) {
    ElectronProfileBetaGamma = PrepareProfile(hGammaEBetaGamma, "ElectronProfileBetaGamma");
  }
  ElectronProfileBetaGamma->SetTitle("ElectronProfile");

  ElectronProfileBetaGamma->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
  ElectronProfileBetaGamma->GetXaxis()->SetTitle("#beta*#gamma");
  ElectronProfileBetaGamma->GetYaxis()->SetTitle("dE/dx");
  ElectronProfileBetaGamma->SetLineColor(kRed);


  hGammaEMomentum = Normalise2DHisto(hGammaEMomentum);

  TList* histList = new TList;
  histList->Add(ElectronProfileMomentum);
  histList->Add(ElectronProfileBetaGamma);
  histList->Add(hGammaEMomentum);

  if (m_isMakePlots) {
    TFile GammaHistogrammingPlotFile("SVDdEdxCalibrationGammaHistogramming.root", "RECREATE");
    histList->Write();
    GammaHistogrammingPlotFile.Close();
  }

  return histList;

}

TList* SVDdEdxCalibrationAlgorithm::GenerateNewHistograms(std::shared_ptr<TTree> ttreeLambda, std::shared_ptr<TTree> ttreeDstar,
                                                          std::shared_ptr<TTree> ttreeGamma, std::shared_ptr<TTree> ttreeGeneric)
{
  gROOT->SetBatch(true);
  gStyle->SetOptStat(0);
// run the background subtraction and histogramming parts
  TTree* treeLambda = LambdaMassFit(ttreeLambda);
  TList* HistListLambda = LambdaHistogramming(treeLambda);
  TH1F* ProtonProfileBetaGamma = (TH1F*) HistListLambda->FindObject("ProtonProfileBetaGamma");
  TH2F* Proton2DHistogram = (TH2F*) HistListLambda->FindObject("hist_d1_2212_truncMomentum");

  TTree* treeDstar = DstarMassFit(ttreeDstar);
  TList* HistListDstar = DstarHistogramming(treeDstar);
  TH1F* PionProfileBetaGamma = (TH1F*) HistListDstar->FindObject("PionProfileBetaGamma");
  TH2F* Pion2DHistogram = (TH2F*) HistListDstar->FindObject("hist_d1_211_truncMomentum");
  TH1F* KaonProfileBetaGamma = (TH1F*) HistListDstar->FindObject("KaonProfileBetaGamma");
  TH2F* Kaon2DHistogram = (TH2F*) HistListDstar->FindObject("hist_d1_321_truncMomentum");

  TList* HistListGamma = GammaHistogramming(ttreeGamma);
  TH1F* ElectronProfileBetaGamma = (TH1F*) HistListGamma->FindObject("ElectronProfileBetaGamma");
  TH2F* Electron2DHistogram = (TH2F*) HistListGamma->FindObject("hist_d1_11_truncMomentum");

  int cred = TColor::GetColor("#e31a1c");
  PionProfileBetaGamma->SetMarkerSize(4);
  PionProfileBetaGamma->SetLineWidth(2);
  PionProfileBetaGamma->SetMarkerColor(cred);
  PionProfileBetaGamma->SetLineColor(cred);

  int cpink = TColor::GetColor("#807dba");
  KaonProfileBetaGamma->SetMarkerSize(4);
  KaonProfileBetaGamma->SetLineWidth(2);
  KaonProfileBetaGamma->SetMarkerColor(cpink);
  KaonProfileBetaGamma->SetLineColor(cpink);

  int cblue = TColor::GetColor("#084594");
  ProtonProfileBetaGamma->SetMarkerSize(4);
  ProtonProfileBetaGamma->SetLineWidth(2);
  ProtonProfileBetaGamma->SetMarkerColor(cblue);
  ProtonProfileBetaGamma->SetLineColor(cblue);

  int cgreen = TColor::GetColor("#238b45");
  ElectronProfileBetaGamma->SetMarkerSize(4);
  ElectronProfileBetaGamma->SetLineWidth(2);
  ElectronProfileBetaGamma->SetMarkerColor(cgreen);
  ElectronProfileBetaGamma->SetLineColor(cgreen);

// prepare the fitting

  PionProfileBetaGamma->GetYaxis()->SetRangeUser(5.e5, 5.5e6);
  KaonProfileBetaGamma->GetYaxis()->SetRangeUser(5.e5, 5.5e6);
  ProtonProfileBetaGamma->GetYaxis()->SetRangeUser(5.e5, 5.5e6);

// enhance the proton histogram (which ends at beta*gamma around 3) by adding pion data above this value – otherwise the fit is very unstable
  auto PionEdges = PionProfileBetaGamma->GetXaxis()->GetXbins()->GetArray();
  auto ProtonEdges = ProtonProfileBetaGamma->GetXaxis()->GetXbins()->GetArray();

  std::vector<float> CombinedEdgesVector;

  double borderline = 3.;

  for (int i = 0; i < ProtonProfileBetaGamma->GetNbinsX() + 1; i++)
    if (ProtonEdges[i] < borderline) CombinedEdgesVector.push_back(ProtonEdges[i]);


  for (int i = 0; i < PionProfileBetaGamma->GetNbinsX() + 1; i++)
    if (PionEdges[i] > borderline) CombinedEdgesVector.push_back(PionEdges[i]);


  TH1F* CombinedHistogramPAndPi = new TH1F("CombinedHistogramPAndPi", "histo_for_fit", CombinedEdgesVector.size() - 1,
                                           CombinedEdgesVector.data());

  int iterator = 1;
  for (int i = 1; i < ProtonProfileBetaGamma->GetNbinsX() + 1; i++)
    if (ProtonEdges[i - 1] < borderline) {
      CombinedHistogramPAndPi->SetBinContent(i, ProtonProfileBetaGamma->GetBinContent(i));
      CombinedHistogramPAndPi->SetBinError(i, ProtonProfileBetaGamma->GetBinError(i));
      iterator++;
    }

  for (int i = 1; i < PionProfileBetaGamma->GetNbinsX() + 1; i++)
    if (PionEdges[i - 1] > borderline) {

      CombinedHistogramPAndPi->SetBinContent(iterator, PionProfileBetaGamma->GetBinContent(i));
      CombinedHistogramPAndPi->SetBinError(iterator, PionProfileBetaGamma->GetBinError(i));
      iterator++;
    }

// define the beta*gamma vs momentum function
  TF1* BetaGammaFunctionPion = new TF1("BetaGammaFunctionPion", "[0] + [1] * x/[2] +  [5]/(x^2/[2]^2 + [3])**[4] + [6]* (x/[2])**0.5",
                                       0.01, 25.);

  BetaGammaFunctionPion->SetNpx(1000);

  BetaGammaFunctionPion->SetParameters(5.e5, 2.e3, 1, 0.15, 1.2, 6.e5, 3.e5);

  BetaGammaFunctionPion->SetParLimits(0, 3.e5, 7.e5);
  BetaGammaFunctionPion->SetParLimits(1, -3.e4, 1.e4);
  BetaGammaFunctionPion->SetParLimits(3, 0.1, 0.2);
  BetaGammaFunctionPion->SetParLimits(4, 0.9, 1.6);
  BetaGammaFunctionPion->SetParLimits(5, 3.e5, 7.e5);
  BetaGammaFunctionPion->SetParLimits(6, 0., 1.e6);
  BetaGammaFunctionPion->FixParameter(2, 1);
  if (m_FixUnstableFitParameter) BetaGammaFunctionPion->FixParameter(3, 0.15);

// fit it to the pion data
  ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2", "Migrad");
  auto FitResultBetaGammaPion = PionProfileBetaGamma->Fit("BetaGammaFunctionPion", "0SI", "", 0.4, 25);

  if ((FitResultBetaGammaPion->Status() > 1) || (BetaGammaFunctionPion->Eval(1) < 5.e5) || (BetaGammaFunctionPion->Eval(1) > 5.e6)) {
    BetaGammaFunctionPion->FixParameter(3, 0.15);
    FitResultBetaGammaPion = PionProfileBetaGamma->Fit("BetaGammaFunctionPion", "0SI", "", 0.4, 25);
  }
  if ((FitResultBetaGammaPion->Status() > 1) || (BetaGammaFunctionPion->Eval(1) < 5.e5) || (BetaGammaFunctionPion->Eval(1) > 5.e6)) {
    BetaGammaFunctionPion->FixParameter(3, 0.15);
    FitResultBetaGammaPion = PionProfileBetaGamma->Fit("BetaGammaFunctionPion", "0SI", "", 0.45, 25);
  }
  if ((FitResultBetaGammaPion->Status() > 1) || (BetaGammaFunctionPion->Eval(1) < 5.e5) || (BetaGammaFunctionPion->Eval(1) > 5.e6)) {
    BetaGammaFunctionPion->FixParameter(3, 0.15);
    FitResultBetaGammaPion = PionProfileBetaGamma->Fit("BetaGammaFunctionPion", "0S", "", 0.5, 25);
  }

  B2INFO("BetaGamma fit for pions done. Fit status: " <<  FitResultBetaGammaPion->Status());
  // B2INFO(FitResultBetaGammaPion->Print(Belle2::LogConfig::c_Info));
  B2INFO("Fit parameters:");
  B2INFO("p0: " << BetaGammaFunctionPion->GetParameter(0) << " +- " << BetaGammaFunctionPion->GetParError(0));
  B2INFO("p1: " << BetaGammaFunctionPion->GetParameter(1) << " +- " << BetaGammaFunctionPion->GetParError(1));
  B2INFO("p2: " << BetaGammaFunctionPion->GetParameter(2) << " +- " << BetaGammaFunctionPion->GetParError(2));
  B2INFO("p3: " << BetaGammaFunctionPion->GetParameter(3) << " +- " << BetaGammaFunctionPion->GetParError(3));
  B2INFO("p4: " << BetaGammaFunctionPion->GetParameter(4) << " +- " << BetaGammaFunctionPion->GetParError(4));
  B2INFO("p5: " << BetaGammaFunctionPion->GetParameter(5) << " +- " << BetaGammaFunctionPion->GetParError(5));
  B2INFO("p6: " << BetaGammaFunctionPion->GetParameter(6) << " +- " << BetaGammaFunctionPion->GetParError(6));

  // repeat the same for kaons
  TF1* BetaGammaFunctionKaon = new TF1("BetaGammaFunctionKaon", "[0] + [1] * x/[2] +  [5]/(x^2/[2]^2 + [3])**[4]+ [6]* (x/[2])**0.5",
                                       0.01, 25.);

  BetaGammaFunctionKaon->SetNpx(1000);
  BetaGammaFunctionKaon->SetParameters(5.e5, 2.e3, 1, 0.15, 1.2, 6.e5, 3.e5);

  BetaGammaFunctionKaon->SetParLimits(0, 3.e5, 7.e5);
  BetaGammaFunctionKaon->SetParLimits(1, -3.e4, 1.e4);
  BetaGammaFunctionKaon->SetParLimits(3, 0.1, 0.2);
  BetaGammaFunctionKaon->SetParLimits(4, 0.9, 1.6);
  BetaGammaFunctionKaon->SetParLimits(5, 3.e5, 7.e5);
  BetaGammaFunctionKaon->SetParLimits(6, 0., 1.e6);

  BetaGammaFunctionKaon->FixParameter(2, 1);
  if (m_FixUnstableFitParameter) BetaGammaFunctionKaon->FixParameter(3, 0.15);

  BetaGammaFunctionKaon->SetLineColor(KaonProfileBetaGamma->GetMarkerColor());

  auto FitResultBetaGammaKaon = KaonProfileBetaGamma->Fit("BetaGammaFunctionKaon", "0SI", "", 0.4, 8.5);

  if ((FitResultBetaGammaKaon->Status() > 1) || (BetaGammaFunctionKaon->Eval(1) < 5.e5) || (BetaGammaFunctionKaon->Eval(1) > 5.e6)) {
    BetaGammaFunctionKaon->FixParameter(3, 0.15);
    FitResultBetaGammaKaon = KaonProfileBetaGamma->Fit("BetaGammaFunctionKaon", "0SI", "", 0.4, 8.5);
  }
  if ((FitResultBetaGammaKaon->Status() > 1) || (BetaGammaFunctionKaon->Eval(1) < 5.e5) || (BetaGammaFunctionKaon->Eval(1) > 5.e6)) {
    BetaGammaFunctionKaon->FixParameter(3, 0.15);
    FitResultBetaGammaKaon = KaonProfileBetaGamma->Fit("BetaGammaFunctionKaon", "0SI", "", 0.45, 8);
  }
  if ((FitResultBetaGammaKaon->Status() > 1) || (BetaGammaFunctionKaon->Eval(1) < 5.e5) || (BetaGammaFunctionKaon->Eval(1) > 5.e6)) {
    BetaGammaFunctionKaon->FixParameter(3, 0.15);
    FitResultBetaGammaKaon = KaonProfileBetaGamma->Fit("BetaGammaFunctionKaon", "0S", "", 0.5, 8);
  }

  B2INFO("BetaGamma fit for kaons done. Fit status: " <<  FitResultBetaGammaKaon->Status());
  B2INFO("Fit parameters:");
  B2INFO("p0: " << BetaGammaFunctionKaon->GetParameter(0) << " +- " << BetaGammaFunctionKaon->GetParError(0));
  B2INFO("p1: " << BetaGammaFunctionKaon->GetParameter(1) << " +- " << BetaGammaFunctionKaon->GetParError(1));
  B2INFO("p2: " << BetaGammaFunctionKaon->GetParameter(2) << " +- " << BetaGammaFunctionKaon->GetParError(2));
  B2INFO("p3: " << BetaGammaFunctionKaon->GetParameter(3) << " +- " << BetaGammaFunctionKaon->GetParError(3));
  B2INFO("p4: " << BetaGammaFunctionKaon->GetParameter(4) << " +- " << BetaGammaFunctionKaon->GetParError(4));
  B2INFO("p5: " << BetaGammaFunctionKaon->GetParameter(5) << " +- " << BetaGammaFunctionKaon->GetParError(5));
  B2INFO("p6: " << BetaGammaFunctionKaon->GetParameter(6) << " +- " << BetaGammaFunctionKaon->GetParError(6));

  // repeat the same for protons
  TF1* BetaGammaFunctionProton = new TF1("BetaGammaFunctionProton",
                                         "[0] + [1] * x/[2] +  [5]/(x^2/[2]^2 + [3])**[4]+ [6]* (x/[2])**0.5", 0.01, 25.);

  BetaGammaFunctionProton->SetNpx(1000);

  BetaGammaFunctionProton->SetParameters(5.e5, 2.e3, 1, 0.15, 1.2, 6.e5, 3.e5);

  BetaGammaFunctionProton->SetParLimits(0, 3.e5, 7.e5);
  BetaGammaFunctionProton->SetParLimits(1, -3.e4, 1.e4);
  BetaGammaFunctionProton->SetParLimits(3, 0.1, 0.2);
  BetaGammaFunctionProton->SetParLimits(4, 0.9, 1.6);
  BetaGammaFunctionProton->SetParLimits(5, 3.e5, 7.e5);
  BetaGammaFunctionProton->SetParLimits(6, 0., 1.e6);

  BetaGammaFunctionProton->FixParameter(2, 1);
  if (m_FixUnstableFitParameter) BetaGammaFunctionProton->FixParameter(3, 0.15);

  BetaGammaFunctionProton->SetLineColor(ProtonProfileBetaGamma->GetMarkerColor());

  auto FitResultBetaGammaProton = CombinedHistogramPAndPi->Fit("BetaGammaFunctionProton", "0SI", "", 0.45, 15);

  if ((FitResultBetaGammaProton->Status() > 1) || (BetaGammaFunctionProton->Eval(1) < 5.e5)
      || (BetaGammaFunctionProton->Eval(1) > 5.e6)) {
    BetaGammaFunctionProton->FixParameter(3, 0.15);
    FitResultBetaGammaProton = CombinedHistogramPAndPi->Fit("BetaGammaFunctionProton", "0SI", "", 0.45, 15);
  }
  if ((FitResultBetaGammaProton->Status() > 1) || (BetaGammaFunctionProton->Eval(1) < 5.e5)
      || (BetaGammaFunctionProton->Eval(1) > 5.e6)) {
    BetaGammaFunctionProton->FixParameter(3, 0.15);
    FitResultBetaGammaProton = CombinedHistogramPAndPi->Fit("BetaGammaFunctionProton", "0SI", "", 0.45, 10);
  }
  if ((FitResultBetaGammaProton->Status() > 1) || (BetaGammaFunctionProton->Eval(1) < 5.e5)
      || (BetaGammaFunctionProton->Eval(1) > 5.e6)) {
    BetaGammaFunctionProton->FixParameter(3, 0.15);
    FitResultBetaGammaProton = CombinedHistogramPAndPi->Fit("BetaGammaFunctionProton", "0S", "", 0.5, 10);
  }

  B2INFO("BetaGamma fit for protons done. Fit status: " <<  FitResultBetaGammaProton->Status());
  B2INFO("Fit parameters:");
  B2INFO("p0: " << BetaGammaFunctionProton->GetParameter(0) << " +- " << BetaGammaFunctionProton->GetParError(0));
  B2INFO("p1: " << BetaGammaFunctionProton->GetParameter(1) << " +- " << BetaGammaFunctionProton->GetParError(1));
  B2INFO("p2: " << BetaGammaFunctionProton->GetParameter(2) << " +- " << BetaGammaFunctionProton->GetParError(2));
  B2INFO("p3: " << BetaGammaFunctionProton->GetParameter(3) << " +- " << BetaGammaFunctionProton->GetParError(3));
  B2INFO("p4: " << BetaGammaFunctionProton->GetParameter(4) << " +- " << BetaGammaFunctionProton->GetParError(4));
  B2INFO("p5: " << BetaGammaFunctionProton->GetParameter(5) << " +- " << BetaGammaFunctionProton->GetParError(5));
  B2INFO("p6: " << BetaGammaFunctionProton->GetParameter(6) << " +- " << BetaGammaFunctionProton->GetParError(6));

// plot a summary of all beta*gamma fits for hadrons
  TCanvas* CombinedCanvasHadrons = new TCanvas("CombinedCanvasHadrons", "Hadron beta*gamma fits", 10, 10, 1000, 700);
  gStyle->SetOptFit(1111);


  PionProfileBetaGamma->Draw();
  PionProfileBetaGamma->GetListOfFunctions()->Add(BetaGammaFunctionPion);
  KaonProfileBetaGamma->Draw("SAME");
  KaonProfileBetaGamma->GetListOfFunctions()->Add(BetaGammaFunctionKaon);
  ProtonProfileBetaGamma->Draw("SAME");
  ProtonProfileBetaGamma->GetListOfFunctions()->Add(BetaGammaFunctionProton);
// BetaGammaFunctionPion->Draw("SAME");
// BetaGammaFunctionKaon->Draw("SAME");
// BetaGammaFunctionProton->Draw("SAME");
  auto legend = new TLegend(0.4, 0.7, 0.65, 0.9);
  legend->AddEntry(PionProfileBetaGamma, "Pions", "lep");
  legend->AddEntry(KaonProfileBetaGamma, "Kaons", "lep");
  legend->AddEntry(ProtonProfileBetaGamma, "Protons", "lep");
  legend->Draw();

  gPad->SetLogx();
  gPad->SetLogy();

  if (m_isMakePlots) {
    CombinedCanvasHadrons->Print("HadronBetaGammaFits.pdf");
    TFile HadronFitPlotFile("SVDdEdxCalibrationHadronFitPlotFile.root", "RECREATE");
    PionProfileBetaGamma->Write();
    KaonProfileBetaGamma->Write();
    ProtonProfileBetaGamma->Write();
    CombinedHistogramPAndPi->Write();
    BetaGammaFunctionPion->Write();
    BetaGammaFunctionKaon->Write();
    BetaGammaFunctionProton->Write();
    CombinedCanvasHadrons->Write();
    HadronFitPlotFile.Close();
  }


  // in case we assume that all hadrons are equal
  if (m_UsePionBGFunctionForEverything) {
    BetaGammaFunctionKaon = (TF1*) BetaGammaFunctionPion->Clone("BetaGammaFunctionKaon");
    BetaGammaFunctionProton = (TF1*) BetaGammaFunctionPion->Clone("BetaGammaFunctionProton");
  }
  if (m_UseProtonBGFunctionForEverything) {
    BetaGammaFunctionKaon = (TF1*) BetaGammaFunctionProton->Clone("BetaGammaFunctionKaon");
    BetaGammaFunctionPion = (TF1*) BetaGammaFunctionProton->Clone("BetaGammaFunctionPion");
  }

  // sanity checks: are all fits ok?
  if ((FitResultBetaGammaProton->Status() > 1) || (BetaGammaFunctionProton->Eval(1) < 5.e5)
      || (BetaGammaFunctionProton->Eval(1) > 5.e6)) {
    if (FitResultBetaGammaPion->Status() == 0) {
      BetaGammaFunctionProton = (TF1*) BetaGammaFunctionPion->Clone("BetaGammaFunctionProton");
    } else if (FitResultBetaGammaKaon->Status() == 0) {
      BetaGammaFunctionProton = (TF1*) BetaGammaFunctionKaon->Clone("BetaGammaFunctionProton");
    } else {
      B2WARNING("Problem with the beta*gamma fit for protons, reverting to the default values");
      BetaGammaFunctionProton->SetParameters(450258, -10900.8, 1, 0.126797, 1.155, 641907, 86304.5);
    }
  }

  if ((FitResultBetaGammaKaon->Status() > 1) || (BetaGammaFunctionKaon->Eval(1) < 5.e5) || (BetaGammaFunctionKaon->Eval(1) > 5.e6)) {
    if (FitResultBetaGammaProton->Status() == 0) {
      BetaGammaFunctionKaon = (TF1*) BetaGammaFunctionProton->Clone("BetaGammaFunctionKaon");
    } else if (FitResultBetaGammaPion->Status() == 0) {
      BetaGammaFunctionKaon = (TF1*) BetaGammaFunctionPion->Clone("BetaGammaFunctionKaon");
    } else {
      B2WARNING("Problem with the beta*gamma fit for kaons, reverting to the default values");
      BetaGammaFunctionKaon->SetParameters(543386, 3013.81, 1, 0.135517, 1.19742, 619509, 15484.4);
    }
  }

  if ((FitResultBetaGammaPion->Status() > 1) || (BetaGammaFunctionPion->Eval(1) < 5.e5) || (BetaGammaFunctionPion->Eval(1) > 5.e6)) {
    if (FitResultBetaGammaKaon->Status() == 0) {
      BetaGammaFunctionPion = (TF1*) BetaGammaFunctionKaon->Clone("BetaGammaFunctionPion");
    } else if (FitResultBetaGammaProton->Status() == 0) {
      BetaGammaFunctionPion = (TF1*) BetaGammaFunctionProton->Clone("BetaGammaFunctionPion");
    } else {
      B2WARNING("Problem with the beta*gamma fit for pions, reverting to the default values");
      BetaGammaFunctionPion->SetParameters(537623, -1937.62, 1, 0.15292, 1.23803, 623678, 30400.9);
    }
  }


// electrons
  TCanvas* ElectronCanvas = new TCanvas("ElectronCanvas", "Electron histogram", 10, 10, 1000, 700);
  TF1* BetaGammaFunctionElectron = new TF1("BetaGammaFunctionElectron", "[0] + [1]* x", 1, 10000.);
  BetaGammaFunctionElectron->SetParameters(6.e5, -1);
  BetaGammaFunctionElectron->SetParLimits(0, 3.e5, 8.e5);
  BetaGammaFunctionElectron->SetParLimits(1, -1.e5, 1.e5);
  auto FitResultBetaGammaElectron = ElectronProfileBetaGamma->Fit("BetaGammaFunctionElectron", "0SI", "", 100, 8000);


  if ((FitResultBetaGammaElectron->Status() > 1) || (BetaGammaFunctionElectron->Eval(1) < 3.e5)
      || (BetaGammaFunctionElectron->Eval(1) > 5.e6)) {
    FitResultBetaGammaElectron = ElectronProfileBetaGamma->Fit("BetaGammaFunctionElectron", "0S", "", 100, 10000);
  }
  B2INFO("BetaGamma fit for electrons done. Fit status: " <<  FitResultBetaGammaElectron->Status());
  B2INFO("Fit parameters:");
  B2INFO("p0: " << BetaGammaFunctionElectron->GetParameter(0) << " +- " << BetaGammaFunctionElectron->GetParError(0));
  B2INFO("p1: " << BetaGammaFunctionElectron->GetParameter(1) << " +- " << BetaGammaFunctionElectron->GetParError(1));


  ElectronProfileBetaGamma->SetMarkerSize(4);
  ElectronProfileBetaGamma->SetLineWidth(2);
  ElectronProfileBetaGamma->GetYaxis()->SetRangeUser(5e5, 1e6);
  ElectronProfileBetaGamma->GetListOfFunctions()->Add(BetaGammaFunctionElectron);
  ElectronProfileBetaGamma->Draw();

  gPad->SetLogx();


  if (m_isMakePlots) {
    ElectronCanvas->Print("ElectronBetaGammaFits.pdf");
    TFile ElectronFitPlotFile("SVDdEdxCalibrationElectronFitPlotFile.root", "RECREATE");
    ElectronProfileBetaGamma->Write();
    BetaGammaFunctionElectron->Write();
    ElectronCanvas->Write();
    ElectronFitPlotFile.Close();
  }

  TF1* MomentumFunctionElectron = (TF1*) BetaGammaFunctionElectron->Clone("MomentumFunctionElectron");
  MomentumFunctionElectron->SetParameter(2, m_ElectronPDGMass);
  MomentumFunctionElectron->SetRange(0.01, 5.5);
  MomentumFunctionElectron->SetLineColor(kRed);
  MomentumFunctionElectron->SetLineWidth(4);

  TF1* MomentumFunctionPion = (TF1*) BetaGammaFunctionPion->Clone("MomentumFunctionPion");
  MomentumFunctionPion->SetParameter(2, m_PionPDGMass);
  MomentumFunctionPion->SetRange(0.01, 5.5);
  MomentumFunctionPion->SetLineColor(kRed);
  MomentumFunctionPion->SetLineWidth(4);

  TF1* MomentumFunctionProton = (TF1*) BetaGammaFunctionProton->Clone("MomentumFunctionProton");
  MomentumFunctionProton->SetParameter(2, m_ProtonPDGMass);
  MomentumFunctionProton->SetRange(0.01, 5.5);
  MomentumFunctionProton->SetLineColor(kRed);
  MomentumFunctionProton->SetLineWidth(4);

  TF1* MomentumFunctionKaon = (TF1*) BetaGammaFunctionKaon->Clone("MomentumFunctionKaon");
  MomentumFunctionKaon->SetParameter(2, m_KaonPDGMass);
  MomentumFunctionKaon->SetRange(0.01, 5.5);
  MomentumFunctionKaon->SetLineColor(kRed);
  MomentumFunctionKaon->SetLineWidth(4);

  gStyle->SetOptFit(1111);
  TCanvas* CanvasOverlays = new TCanvas("CanvasOverlays", "overlays", 1300, 1000);
  CanvasOverlays->Divide(2, 2);
  CanvasOverlays->cd(1); Electron2DHistogram->Draw();   MomentumFunctionElectron->Draw("SAME");
  CanvasOverlays->cd(2); Pion2DHistogram->Draw();  MomentumFunctionPion->Draw("SAME");
  CanvasOverlays->cd(3); Kaon2DHistogram->Draw();  MomentumFunctionKaon->Draw("SAME");
  CanvasOverlays->cd(4); Proton2DHistogram->Draw();  MomentumFunctionProton->Draw("SAME");
  CanvasOverlays->Print("SVDdEdxOverlaysFitsHistos.pdf");

  TF1* MomentumFunctionDeuteron = (TF1*) BetaGammaFunctionProton->Clone("MomentumFunctionDeuteron");
  MomentumFunctionDeuteron->SetParameter(2, m_DeuteronPDGMass);
  MomentumFunctionDeuteron->SetRange(0.01, 5.5);
  MomentumFunctionDeuteron->SetLineColor(kRed);

  TF1* MomentumFunctionMuon = (TF1*) BetaGammaFunctionPion->Clone("MomentumFunctionMuon");
  MomentumFunctionMuon->SetParameter(2, m_MuonPDGMass);
  MomentumFunctionMuon->SetRange(0.01, 5.5);
  MomentumFunctionMuon->SetLineColor(kRed);

// overlay all fits in one plot

  TCanvas* OverlayAllTracksCanvas = new TCanvas("OverlayAllTracksCanvas", "The Ultimate Plot", 10, 10, 1000, 700);

  TH2F* AllTracksHistogram = new TH2F("AllTracksHistogram", "AllTracksHistogram;Momentum [GeV/c];dEdx [arb. units]", 1000, 0.05, 5,
                                      1000, 2.e5, 6.e6);

  ttreeGeneric->Draw("TrackSVDdEdx:TrackSVDdEdxTrackMomentum>>AllTracksHistogram", "TracknSVDHits>7", "goff");
  AllTracksHistogram->Draw("COLZ");
  AllTracksHistogram->GetXaxis()->SetTitle("Momentum [GeV/c]");
  AllTracksHistogram->GetYaxis()->SetTitle("dE/dx [arbitrary units]");
  MomentumFunctionElectron->Draw("SAME");
  MomentumFunctionMuon->Draw("SAME");
  MomentumFunctionPion->Draw("SAME");
  MomentumFunctionKaon->Draw("SAME");
  MomentumFunctionProton->Draw("SAME");
  MomentumFunctionDeuteron->Draw("SAME");
  OverlayAllTracksCanvas->SetLogx();
  OverlayAllTracksCanvas->SetLogz();

  if (m_isMakePlots) {
    OverlayAllTracksCanvas->Print("SVDdEdxAllTracksWithFits.pdf");
    TFile OverlayAllTracksPlotFile("SVDdEdxCalibrationOverlayAllTracks.root", "RECREATE");
    AllTracksHistogram->Write();
    MomentumFunctionElectron->Write();
    MomentumFunctionMuon->Write();
    MomentumFunctionPion->Write();
    MomentumFunctionKaon->Write();
    MomentumFunctionProton->Write();
    MomentumFunctionDeuteron->Write();
    OverlayAllTracksCanvas->Write();
    OverlayAllTracksPlotFile.Close();
  }

// resolution studies //

// For resolution measurement, we need to take a ProjectionY of the data histograms in the momentum range where the dEdx is flat vs momentum. We use our educated guess of the flat range (e.g. 0.6-1 GeV for pions) and FindBin to figure out which bin numbers those momentum values correspond to.
  double PionRangeMin = 0.6;
  double PionRangeMax = 1.;
  double KaonRangeMin = 1.9;
  double KaonRangeMax = 3;
  double ElectronRangeMin = 1.;
  double ElectronRangeMax = 1.4;

  auto PionResolutionHistogram = Pion2DHistogram->ProjectionY("PionResolutionHistogram",
                                                              Pion2DHistogram->GetXaxis()->FindBin(PionRangeMin),
                                                              Pion2DHistogram->GetXaxis()->FindBin(PionRangeMax));
  auto ElectronResolutionHistogram = Electron2DHistogram->ProjectionY("ElectronResolutionHistogram",
                                     Electron2DHistogram->GetXaxis()->FindBin(ElectronRangeMin), Electron2DHistogram->GetXaxis()->FindBin(ElectronRangeMax));
  auto KaonResolutionHistogram = Kaon2DHistogram->ProjectionY("KaonResolutionHistogram",
                                                              Kaon2DHistogram->GetXaxis()->FindBin(KaonRangeMin),
                                                              Kaon2DHistogram->GetXaxis()->FindBin(KaonRangeMax));
// for protons, there is not enough data in the flat range.

  // PionResolutionHistogram->Draw();

  TF1* PionResolutionFunction = new TF1("PionResolutionFunction",
                                        "[0]*TMath::Landau(x, [1], [1]*[2])*TMath::Gaus(x, [1], [1]*[2]*[4]) + [3]*TMath::Gaus(x, [1], [1]*[2]*[5])", 100e3, 1500e3);
// parameter [1] is the mean of the Landau
// parameter [2] is the relative resolution (w.r.t. the mean) of the Landau
// parameters [4]-[5] are the relative resolution of Gauss contributions w.r.t. that of Landau
// parameters [0] and [3] are fractions of the two components
  PionResolutionFunction->SetParameters(1, 6.e5, 0.1, 0.5, 2, 1);
  PionResolutionFunction->SetParLimits(0, 0, 500);
  PionResolutionFunction->SetParLimits(1, 3.e5, 8.e5);
  PionResolutionFunction->SetParLimits(2, 0, 1);
  PionResolutionFunction->SetParLimits(3, 0, 500);
  PionResolutionFunction->SetParLimits(4, 0, 7);
  PionResolutionFunction->SetParLimits(5, 1, 7);
  PionResolutionFunction->SetNpx(1000);
  auto FitResultResolutionPion = PionResolutionHistogram->Fit(PionResolutionFunction, "RSI");

  B2INFO("relative resolution for pions: " << PionResolutionFunction->GetParameter(2));
  B2INFO("resolution for pions: fit status" << FitResultResolutionPion->Status());

  TF1* KaonResolutionFunction = new TF1("KaonResolutionFunction",
                                        "[0]*TMath::Landau(x, [1], [1]*[2])*TMath::Gaus(x, [1], [1]*[2]*[4]) + [3]*TMath::Gaus(x, [1], [1]*[2]*[5])", 100e3, 1500e3);


  KaonResolutionFunction->SetParameters(1, 6.e5, 0.1, 0.5, 2, 1);
  KaonResolutionFunction->SetParLimits(0, 0, 500);
  KaonResolutionFunction->SetParLimits(1, 3.e5, 8.e5);
  KaonResolutionFunction->SetParLimits(2, 0, 1);
  KaonResolutionFunction->SetParLimits(3, 0, 500);
  KaonResolutionFunction->SetParLimits(4, 0, 7);
  KaonResolutionFunction->SetParLimits(5, 1, 7);
  KaonResolutionFunction->SetNpx(1000);
  auto FitResultResolutionKaon = KaonResolutionHistogram->Fit(KaonResolutionFunction, "RSI");

  B2INFO("relative resolution for kaons: " << KaonResolutionFunction->GetParameter(2));
  B2INFO("resolution for kaons: fit status" << FitResultResolutionKaon->Status());

  if ((FitResultResolutionKaon->Status() > 1)
      && (FitResultResolutionPion->Status() <= 1)) KaonResolutionFunction = (TF1*)
            PionResolutionFunction->Clone("KaonResolutionFunction");




  TF1* ElectronResolutionFunction = new TF1("ElectronResolutionFunction",
                                            "[0]*TMath::Landau(x, [1], [1]*[2])*TMath::Gaus(x, [1], [1]*[2]*[4]) + [3]*TMath::Gaus(x, [1], [1]*[2]*[5])", 50e3, 1500e3);


  ElectronResolutionFunction->SetParameters(1, 6.e5, 0.1, 0.5, 2, 1);
  ElectronResolutionFunction->SetParLimits(0, 0, 500);
  ElectronResolutionFunction->SetParLimits(1, 3.e5, 8.e5);
  ElectronResolutionFunction->SetParLimits(2, 0, 1);
  ElectronResolutionFunction->SetParLimits(3, 0, 500);
  ElectronResolutionFunction->SetParLimits(4, 0, 7);
  ElectronResolutionFunction->SetParLimits(5, 1, 7);
  ElectronResolutionFunction->SetNpx(1000);
  auto FitResultResolutionElectron = ElectronResolutionHistogram->Fit(ElectronResolutionFunction, "RSI");

  B2INFO("relative resolution for electrons: " << ElectronResolutionFunction->GetParameter(2));
  B2INFO("resolution for electrons: fit status" << FitResultResolutionElectron->Status());

  // plot all the resolution fits
  TCanvas* CanvasResolutions = new TCanvas("CanvasResolutions", "Resolutions", 1200, 650);
  CanvasResolutions->Divide(3, 1);
  CanvasResolutions->cd(1); PionResolutionHistogram->Draw();
  CanvasResolutions->cd(2); KaonResolutionHistogram->Draw();
  CanvasResolutions->cd(3); ElectronResolutionHistogram->Draw();


  if (m_isMakePlots) {
    CanvasResolutions->Print("SVDdEdxResolutions.pdf");
    TFile OverlayResolutionsPlotFile("SVDdEdxCalibrationResolutions.root", "RECREATE");
    PionResolutionHistogram->Write();
    KaonResolutionHistogram->Write();
    ElectronResolutionHistogram->Write();
    CanvasResolutions->Write();
    OverlayResolutionsPlotFile.Close();
  }

// evaluate the bias correction:
// difference between the MomentumFunction prediction and the mean of the resolution function in the flat part
// it should be of the order -1e4, i.e. about -1% of the absolute dEdx value
  double BiasCorrectionPion = PionResolutionFunction->GetParameter(1) - MomentumFunctionPion->Eval((
                                PionRangeMax + PionRangeMin) / 2.);
  B2INFO("BiasCorrectionPion = " << BiasCorrectionPion);

// generate a new pion payload using the MomentumFunctionPion, PionResolutionFunction and the bias correction
  TH2F* Pion2DHistogramNew = PrepareNewHistogram(Pion2DHistogram, Form("%sNew", Pion2DHistogram->GetName()), MomentumFunctionPion,
                                                 PionResolutionFunction, BiasCorrectionPion);

// sanity check: residual between the generated distribution and the data one
  TH2F* Pion2DHistogramResidual = (TH2F*) Pion2DHistogram->Clone("Pion2DHistogramResidual");
  Pion2DHistogramResidual->Add(Pion2DHistogramNew, Pion2DHistogram, 1, -1);
  Pion2DHistogramResidual->SetMinimum(-0.15);
  Pion2DHistogramResidual->SetMaximum(0.15);

  // repeat, for kaons
  double BiasCorrectionKaon = KaonResolutionFunction->GetParameter(1) - MomentumFunctionKaon->Eval((
                                KaonRangeMax + KaonRangeMin) / 2.);
  B2INFO("BiasCorrectionKaon = " << BiasCorrectionKaon);

  // for protons, we compare the flat part of the MomentumFunction (~3 GeV) with the mean of the kaon resolution function
  // as there's not enough stats in the flat part to extract proton resolution from data
  double BiasCorrectionProton = KaonResolutionFunction->GetParameter(1) - MomentumFunctionProton->Eval(3.);
  B2INFO("BiasCorrectionProton = " << BiasCorrectionProton);

  if ((BiasCorrectionProton / BiasCorrectionKaon)  > 1.5) BiasCorrectionProton =
      BiasCorrectionKaon; // probably something went wrong due to low statistics

  // back to kaons: generate a new payload
  TH2F* Kaon2DHistogramNew = PrepareNewHistogram(Kaon2DHistogram, Form("%sNew", Kaon2DHistogram->GetName()), MomentumFunctionKaon,
                                                 KaonResolutionFunction, BiasCorrectionKaon);
// residual generated - data for kaons
  TH2F* Kaon2DHistogramResidual = (TH2F*) Kaon2DHistogram->Clone("Kaon2DHistogramResidual");
  Kaon2DHistogramResidual->Add(Kaon2DHistogramNew, Kaon2DHistogram, 1, -1);
  Kaon2DHistogramResidual->SetMinimum(-0.15);
  Kaon2DHistogramResidual->SetMaximum(0.15);

  // same for protons (we use the kaon resolution function as explained above)
  TH2F* Proton2DHistogramNew = PrepareNewHistogram(Proton2DHistogram, Form("%sNew", Proton2DHistogram->GetName()),
                                                   MomentumFunctionProton,
                                                   KaonResolutionFunction, BiasCorrectionProton);

// residual for protons
  TH2F* Proton2DHistogramResidual = (TH2F*) Proton2DHistogram->Clone("Proton2DHistogramResidual");
  Proton2DHistogramResidual->Add(Proton2DHistogramNew, Proton2DHistogram, 1, -1);
  Proton2DHistogramResidual->SetMinimum(-0.15);
  Proton2DHistogramResidual->SetMaximum(0.15);

// deuterons: same as protons, but use the MomentumFunctionDeuteron
  TH2F* Deuteron2DHistogramNew = PrepareNewHistogram(Proton2DHistogram, "Deuteron2DHistogramNew", MomentumFunctionDeuteron,
                                                     KaonResolutionFunction,
                                                     BiasCorrectionKaon);
  Deuteron2DHistogramNew->SetTitle("hist_d1_1000010020_trunc");

// muons: same as pions, but use the MomentumFunctionMuon
  TH2F* Muon2DHistogramNew = PrepareNewHistogram(Pion2DHistogram, "Muon2DHistogramNew", MomentumFunctionMuon, PionResolutionFunction,
                                                 BiasCorrectionPion);
  Muon2DHistogramNew->SetTitle("hist_d1_13_trunc");

// same for electrons
  double BiasCorrectionElectron = ElectronResolutionFunction->GetParameter(1) - MomentumFunctionElectron->Eval((
                                    ElectronRangeMax + ElectronRangeMin) / 2.);
  B2INFO("BiasCorrectionElectron = " << BiasCorrectionElectron);
  TH2F* Electron2DHistogramNew = PrepareNewHistogram(Electron2DHistogram, Form("%sNew", Electron2DHistogram->GetName()),
                                                     MomentumFunctionElectron,
                                                     ElectronResolutionFunction, BiasCorrectionElectron);

  TH2F* Electron2DHistogramResidual = (TH2F*) Electron2DHistogram->Clone("Electron2DHistogramResidual");
  Electron2DHistogramResidual->Add(Electron2DHistogramNew, Electron2DHistogram, 1, -1);
  Electron2DHistogramResidual->SetMinimum(-0.15);
  Electron2DHistogramResidual->SetMaximum(0.15);

// plot the summary of all the distributions
  TCanvas* CanvasSummaryGenerated = new TCanvas("CanvasSummaryGenerated", "Generated payloads", 1700, 850);
  CanvasSummaryGenerated->Divide(3, 2);
  CanvasSummaryGenerated->cd(1); Electron2DHistogramNew->Draw("COLZ");
  CanvasSummaryGenerated->cd(2); Muon2DHistogramNew->Draw("COLZ");
  CanvasSummaryGenerated->cd(3); Pion2DHistogramNew->Draw("COLZ");
  CanvasSummaryGenerated->cd(4); Kaon2DHistogramNew->Draw("COLZ");
  CanvasSummaryGenerated->cd(5); Proton2DHistogramNew->Draw("COLZ");
  CanvasSummaryGenerated->cd(6); Deuteron2DHistogramNew->Draw("COLZ");


  if (m_isMakePlots) {
    CanvasSummaryGenerated->Print("SVDdEdxGeneratedPayloads.pdf");
    TFile SummaryGeneratedPlotFile("SVDdEdxCalibrationSummaryGenerated.root", "RECREATE");
    Electron2DHistogramNew->Write();
    Muon2DHistogramNew->Write();
    Pion2DHistogramNew->Write();
    Kaon2DHistogramNew->Write();
    Proton2DHistogramNew->Write();
    Deuteron2DHistogramNew->Write();
    SummaryGeneratedPlotFile.Close();
  }

  TCanvas* CanvasSummaryData = new TCanvas("CanvasSummaryData", "Data distributions", 1700, 850);
  CanvasSummaryData->Divide(3, 2);
  CanvasSummaryData->cd(1); Electron2DHistogram->Draw("COLZ");
  CanvasSummaryData->cd(3); Pion2DHistogram->Draw("COLZ");
  CanvasSummaryData->cd(4); Kaon2DHistogram->Draw("COLZ");
  CanvasSummaryData->cd(5); Proton2DHistogram->Draw("COLZ");


  if (m_isMakePlots) {
    CanvasSummaryData->Print("SVDdEdxDataDistributions.pdf");
    TFile SummaryDataPlotFile("SVDdEdxCalibrationSummaryData.root", "RECREATE");
    Electron2DHistogram->Write();
    Pion2DHistogram->Write();
    Kaon2DHistogram->Write();
    Proton2DHistogram->Write();
    SummaryDataPlotFile.Close();
  }

  TCanvas* CanvasSummaryResiduals = new TCanvas("CanvasSummaryResiduals", "Residuals", 1700, 850);
  CanvasSummaryResiduals->Divide(3, 2);
  CanvasSummaryResiduals->cd(1); Electron2DHistogramResidual->Draw("COLZ");
  CanvasSummaryResiduals->cd(3); Pion2DHistogramResidual->Draw("COLZ");
  CanvasSummaryResiduals->cd(4); Kaon2DHistogramResidual->Draw("COLZ");
  CanvasSummaryResiduals->cd(5); Proton2DHistogramResidual->Draw("COLZ");

  if (m_isMakePlots) {
    CanvasSummaryResiduals->Print("SVDdEdxResiduals.pdf");
    TFile SummaryResidualsPlotFile("SVDdEdxCalibrationSummaryResiduals.root", "RECREATE");
    Electron2DHistogramResidual->Write();
    Pion2DHistogramResidual->Write();
    Kaon2DHistogramResidual->Write();
    Proton2DHistogramResidual->Write();
    SummaryResidualsPlotFile.Close();
  }

  Electron2DHistogramNew->SetName("Electron2DHistogramNew");
  Muon2DHistogramNew->SetName("Muon2DHistogramNew");
  Pion2DHistogramNew->SetName("Pion2DHistogramNew");
  Kaon2DHistogramNew->SetName("Kaon2DHistogramNew");
  Proton2DHistogramNew->SetName("Proton2DHistogramNew");
  Deuteron2DHistogramNew->SetName("Deuteron2DHistogramNew");

  // return all the generated payloads
  TList* histList = new TList;
  histList->Add(Electron2DHistogramNew);
  histList->Add(Muon2DHistogramNew);
  histList->Add(Pion2DHistogramNew);
  histList->Add(Kaon2DHistogramNew);
  histList->Add(Proton2DHistogramNew);
  histList->Add(Deuteron2DHistogramNew);

  return histList;

}