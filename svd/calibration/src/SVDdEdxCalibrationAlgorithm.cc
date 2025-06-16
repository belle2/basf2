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

  // call the calibration function
  TList* GeneratedList = GenerateNewHistograms(ttreeLambda, ttreeDstar, ttreeGamma);

  TH2F* histoE = (TH2F*) GeneratedList->FindObject("histoE_2D_new");
  TH2F* histoMu = (TH2F*) GeneratedList->FindObject("histoMu_2D_new");
  TH2F* histoPi = (TH2F*) GeneratedList->FindObject("histoPi_2D_new");
  TH2F* histoK = (TH2F*) GeneratedList->FindObject("histoK_2D_new");
  TH2F* histoP = (TH2F*) GeneratedList->FindObject("histoP_2D_new");
  TH2F* histoDeut = (TH2F*) GeneratedList->FindObject("histoDeut_2D_new");

  // call the calibration functions
  // TH2F hLambdaP = LambdaMassFit(ttreeLambda);
  // auto [hDstarK, hDstarPi, hDstarMu] = DstarMassFit(ttreeDstar);
  // TH2F hGammaE = GammaHistogram(ttreeGamma);
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
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 1 && trunmean) {
        hDedxPDFs[iPart] = histoMu;
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 2 && trunmean) {
        hDedxPDFs[iPart] = histoPi;
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 3 && trunmean) {
        hDedxPDFs[iPart] = histoK;
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 4 && trunmean) {
        hDedxPDFs[iPart] = histoP;
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
  return treeLambda_sw;
}
// std::vector<double> pbins = CreatePBinningScheme();

// TH2F* hLambdaP = new TH2F("hist_d1_2212_trunc", "hist_d1_2212_trunc", m_numPBins, pbins.data(), m_numDEdxBins, 0, m_dedxCutoff);

// treeLambda_sw->Draw("ProtonSVDdEdx:ProtonMomentum>>hist_d1_2212_trunc",
//                     "nSignalLambda_sw * (ProtonMomentum>0.15) * (ProtonSVDdEdx>0)", "goff");

// // produce the 1D profile (for data-MC comparisons)
// if (m_isMakePlots) {
//   TH1F* ProtonProfile = (TH1F*)hLambdaP->ProfileX("ProtonProfile");
//   ProtonProfile->SetTitle("ProtonProfile");
//   canvLambda->SetTicky(1);
//   ProtonProfile->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
//   ProtonProfile->GetXaxis()->SetTitle("Momentum, GeV/c");
//   ProtonProfile->GetYaxis()->SetTitle("dE/dx");
//   ProtonProfile->Draw();
//   canvLambda->Print("SVDdEdxCalibrationProfileProton.pdf");
//   TFile ProtonProfileFile("SVDdEdxCalibrationProfileProton.root", "RECREATE");
//   ProtonProfile->Write();
//   ProtonProfileFile.Close();
//   canvLambda->SetTicky(0);
// }

// for each momentum bin, normalize the pdf

//   // hLambdaP normalisation
//   for (int pbin = 0; pbin <= m_numPBins + 1; pbin++) {
//     for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
//       // get rid of the bins with negative weights
//       if (hLambdaP->GetBinContent(pbin, dedxbin) <= 1) {
//         hLambdaP->SetBinContent(pbin, dedxbin, 0);
//       };
//     }
//     // create a projection (1D histogram) in a given momentum bin
//     TH1D* slice = (TH1D*)hLambdaP->ProjectionY("slice", pbin, pbin);
//     // normalise, but ignore the cases with empty histograms
//     if (slice->Integral() > 0) {
//       slice->Scale(1. / slice->Integral());
//     }
//     // fill back the 2D histo with the result
//     for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
//       hLambdaP->SetBinContent(pbin, dedxbin, slice->GetBinContent(dedxbin));
//     }
//   }

//   if (m_isMakePlots) {
//     hLambdaP->Draw("COLZ");
//     canvLambda->Print("SVDdEdxCalibrationHistoLambda.pdf");
//   }

//   return *hLambdaP;
// }

TList* SVDdEdxCalibrationAlgorithm::LambdaHistogramming(TTree* inputTree)
{

  inputTree->SetEstimate(-1);
  std::vector<double> pbins = CreatePBinningScheme();

  TH2F* hLambdaP_tr = new TH2F("hist_d1_2212_trunc_tr", "hist_d1_2212_trunc_tr", m_numPBins, pbins.data(), m_numDEdxBins, 0,
                               m_dedxCutoff);

  inputTree->Draw("ProtonSVDdEdx:ProtonSVDdEdxTrackMomentum>>hist_d1_2212_trunc_tr",
                  "nSignalLambda_sw * (ProtonSVDdEdx>0) * (ProtonSVDdEdxTrackMomentum>0.2)", "goff");

// create isopopulated beta*gamma binning
  inputTree->Draw("ProtonSVDdEdxTrackMomentum/0.938", "", "goff", ((inputTree->GetEntries()) / m_numPBins)*m_numPBins);
  Double_t* ProtonMomentumDataset = inputTree->GetV1();

  TKDTreeBinning* kdBinsP = new TKDTreeBinning(((inputTree->GetEntries()) / m_numPBins)*m_numPBins, 1, ProtonMomentumDataset,
                                               m_numPBins);
  const Double_t* binsMinEdgesP_pointer = kdBinsP->SortOneDimBinEdges();
  Double_t* binsMinEdgesP =  const_cast<Double_t*>(binsMinEdgesP_pointer);


  binsMinEdgesP[0] = 0.1;
  binsMinEdgesP[m_numPBins + 1] = 50.;


  TH2F* hLambdaP_bg_tr = new TH2F("hist_d1_2212_trunc_bg_tr", "hist_d1_2212_trunc_bg_tr", m_numPBins, binsMinEdgesP, m_numDEdxBins, 0,
                                  m_dedxCutoff);

  inputTree->Draw("ProtonSVDdEdx:ProtonSVDdEdxTrackMomentum/0.938>>hist_d1_2212_trunc_bg_tr",
                  "nSignalLambda_sw * (ProtonSVDdEdx>0) * (ProtonSVDdEdxTrackMomentum>0.2) * (ProtonSVDdEdx>1.2e6 - 1.e6*ProtonSVDdEdxTrackMomentum)",
                  "goff");

  // produce the 1D profile
  // momentum: for data-MC comparisons

  TCanvas* canvLambda = new TCanvas("canvLambda", "canvLambda");


  TH1F* ProtonProfile_tr = (TH1F*)hLambdaP_tr->ProfileX("ProtonProfile_tr");
  ProtonProfile_tr->SetTitle("ProtonProfile");
  // canvLambda->SetTicky(1);
  ProtonProfile_tr->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
  ProtonProfile_tr->GetXaxis()->SetTitle("Momentum, GeV/c");
  ProtonProfile_tr->GetYaxis()->SetTitle("dE/dx");
  ProtonProfile_tr->SetLineColor(kRed);
  ProtonProfile_tr->Draw();

  TH1F* ProtonProfile_bg_tr = (TH1F*)hLambdaP_bg_tr->ProfileX("ProtonProfile_bg_tr");
  ProtonProfile_bg_tr->SetTitle("ProtonProfile");
  // canvLambda->SetTicky(1);
  ProtonProfile_bg_tr->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
  ProtonProfile_bg_tr->GetXaxis()->SetTitle("#beta*#gamma");
  ProtonProfile_bg_tr->GetYaxis()->SetTitle("dE/dx");
  ProtonProfile_bg_tr->SetLineColor(kRed);


  // for each momentum bin, normalize the pdf
  hLambdaP_tr = Normalise2DHisto(hLambdaP_tr);

  TList* histList = new TList;
  histList->Add(ProtonProfile_tr);
  histList->Add(ProtonProfile_bg_tr);
  histList->Add(hLambdaP_tr);


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
  return treeDstar_sw;
}

//   std::vector<double> pbins = CreatePBinningScheme();

//   // the kaon payload
//   TH2F* hDstarK = new TH2F("hist_d1_321_trunc", "hist_d1_321_trunc", m_numPBins, pbins.data(),
//                            m_numDEdxBins, 0, m_dedxCutoff);
//   // the pion payload
//   TH2F* hDstarPi = new TH2F("hist_d1_211_trunc", "hist_d1_211_trunc", m_numPBins, pbins.data(),
//                             m_numDEdxBins, 0, m_dedxCutoff);

//   treeDstar_sw->Draw("KaonSVDdEdx:KaonMomentum>>hist_d1_321_trunc", "nSignalDstar_sw * (KaonSVDdEdx>0)", "goff");
//   // the pion one will be built from both pions in the Dstar decay tree
//   TH2F* hDstarPiPart1 = (TH2F*)hDstarPi->Clone("hist_d1_211_truncPart1");
//   TH2F* hDstarPiPart2 = (TH2F*)hDstarPi->Clone("hist_d1_211_truncPart2");

//   treeDstar_sw->Draw("PionDSVDdEdx:PionDMomentum>>hist_d1_211_truncPart1", "nSignalDstar_sw * (PionDSVDdEdx>0)", "goff");
//   treeDstar_sw->Draw("SlowPionSVDdEdx:SlowPionMomentum>>hist_d1_211_truncPart2", "nSignalDstar_sw * (SlowPionSVDdEdx>0)", "goff");
//   hDstarPi->Add(hDstarPiPart1);
//   hDstarPi->Add(hDstarPiPart2);

//   // the current strategy assumes that the muon and pion payloads are indistinguishable: clone the pion one
//   TH2F* hDstarMu = (TH2F*)hDstarPi->Clone("hist_d1_13_trunc");
//   hDstarMu->SetTitle("hist_d1_13_trunc");

//   // produce the 1D profile (for data-MC comparisons)
//   if (m_isMakePlots) {
//     TH1F* PionProfile = (TH1F*)hDstarPi->ProfileX("PionProfile");
//     PionProfile->SetTitle("PionProfile");
//     canvDstar->SetTicky(1);
//     PionProfile->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
//     PionProfile->GetXaxis()->SetTitle("Momentum, GeV/c");
//     PionProfile->GetYaxis()->SetTitle("dE/dx");
//     PionProfile->Draw();
//     canvDstar->Print("SVDdEdxCalibrationProfilePion.pdf");
//     TFile PionProfileFile("SVDdEdxCalibrationProfilePion.root", "RECREATE");
//     PionProfile->Write();
//     PionProfileFile.Close();

//     TH1F* KaonProfile = (TH1F*)hDstarK->ProfileX("KaonProfile");
//     KaonProfile->SetTitle("KaonProfile");
//     KaonProfile->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
//     KaonProfile->GetXaxis()->SetTitle("Momentum, GeV/c");
//     KaonProfile->GetYaxis()->SetTitle("dE/dx");
//     KaonProfile->Draw();
//     canvDstar->Print("SVDdEdxCalibrationProfileKaon.pdf");
//     TFile KaonProfileFile("SVDdEdxCalibrationProfileKaon.root", "RECREATE");
//     KaonProfile->Write();
//     KaonProfileFile.Close();
//     canvDstar->SetTicky(0);
//   }

//   // hDstarK normalisation
//   // for each momentum bin, normalize the pdf

//   for (int pbin = 0; pbin <= m_numPBins + 1; pbin++) {
//     for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
//       // get rid of the bins with negative weights
//       if (hDstarK->GetBinContent(pbin, dedxbin) <= 1) {
//         hDstarK->SetBinContent(pbin, dedxbin, 0);
//       };
//     }
//     // create a projection (1D histogram) in a given momentum bin
//     TH1D* slice = (TH1D*)hDstarK->ProjectionY("slice", pbin, pbin);
//     // normalise, but ignore the cases with empty histograms
//     if (slice->Integral() > 0) {
//       slice->Scale(1. / slice->Integral());
//     }
//     // fill back the 2D histo with the result
//     for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
//       hDstarK->SetBinContent(pbin, dedxbin, slice->GetBinContent(dedxbin));
//     }
//   }

//   // hDstarPi normalisation
//   for (int pbin = 0; pbin <= m_numPBins + 1; pbin++) {
//     for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
//       // get rid of the bins with negative weights
//       if (hDstarPi->GetBinContent(pbin, dedxbin) <= 1) {
//         hDstarPi->SetBinContent(pbin, dedxbin, 0);
//       };
//     }
//     // create a projection (1D histogram) in a given momentum bin
//     TH1D* slice = (TH1D*)hDstarPi->ProjectionY("slice", pbin, pbin);
//     // normalise, but ignore the cases with empty histograms
//     if (slice->Integral() > 0) {
//       slice->Scale(1. / slice->Integral());
//     }
//     // fill back the 2D histo with the result
//     for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
//       hDstarPi->SetBinContent(pbin, dedxbin, slice->GetBinContent(dedxbin));
//     }
//   }

//   // hDstarMu normalisation
//   for (int pbin = 0; pbin <= m_numPBins + 1; pbin++) {
//     for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
//       // get rid of the bins with negative weights
//       if (hDstarMu->GetBinContent(pbin, dedxbin) <= 1) {
//         hDstarMu->SetBinContent(pbin, dedxbin, 0);
//       };
//     }
//     // create a projection (1D histogram) in a given momentum bin
//     TH1D* slice = (TH1D*)hDstarMu->ProjectionY("slice", pbin, pbin);
//     // normalise, but ignore the cases with empty histograms
//     if (slice->Integral() > 0) {
//       slice->Scale(1. / slice->Integral());
//     }
//     // fill back the 2D histo with the result
//     for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
//       hDstarMu->SetBinContent(pbin, dedxbin, slice->GetBinContent(dedxbin));
//     }
//   }
//   if (m_isMakePlots) {
//     hDstarK->Draw("COLZ");
//     canvDstar->Print("SVDdEdxCalibrationHistoDstarK.pdf");
//     hDstarPi->Draw("COLZ");
//     canvDstar->Print("SVDdEdxCalibrationHistoDstarPi.pdf");
//     hDstarMu->Draw("COLZ");
//     canvDstar->Print("SVDdEdxCalibrationHistoDstarMu.pdf");
//   }

//   return std::make_tuple(*hDstarK, *hDstarPi, *hDstarMu);
// }

TList* SVDdEdxCalibrationAlgorithm::DstarHistogramming(TTree* inputTree)
{

  inputTree->SetEstimate(-1);
  std::vector<double> pbins = CreatePBinningScheme();

  TH2F* hDstarK_tr = new TH2F("hist_d1_321_trunc_tr", "hist_d1_321_trunc_tr", m_numPBins, pbins.data(),
                              m_numDEdxBins, 0, m_dedxCutoff);
  // the pion payload
  TH2F* hDstarPi_tr = new TH2F("hist_d1_211_trunc_tr", "hist_d1_211_trunc_tr", m_numPBins, pbins.data(),
                               m_numDEdxBins, 0, m_dedxCutoff);

  inputTree->Draw("KaonSVDdEdx:KaonSVDdEdxTrackMomentum>>hist_d1_321_trunc_tr", "nSignalDstar_sw * (KaonSVDdEdx>0)", "goff");
  // the pion one will be built from both pions in the Dstar decay tree
  TH2F* hDstarPiPart1_tr = (TH2F*)hDstarPi_tr->Clone("hist_d1_211_truncPart1_tr");
  TH2F* hDstarPiPart2_tr = (TH2F*)hDstarPi_tr->Clone("hist_d1_211_truncPart2_tr");

  inputTree->Draw("PionDSVDdEdx:PionDSVDdEdxTrackMomentum>>hist_d1_211_truncPart1_tr", "nSignalDstar_sw * (PionDSVDdEdx>0)", "goff");
  inputTree->Draw("SlowPionSVDdEdx:SlowPionSVDdEdxTrackMomentum>>hist_d1_211_truncPart2_tr", "nSignalDstar_sw * (SlowPionSVDdEdx>0)",
                  "goff");
  hDstarPi_tr->Add(hDstarPiPart1_tr);
  hDstarPi_tr->Add(hDstarPiPart2_tr);

  inputTree->Draw("KaonSVDdEdxTrackMomentum/0.494", "", "goff", ((inputTree->GetEntries()) / m_numPBins)*m_numPBins);
  Double_t* KaonMomentumDataset = inputTree->GetV1();
  TKDTreeBinning* kdBinsK = new TKDTreeBinning(((inputTree->GetEntries()) / m_numPBins)*m_numPBins, 1, KaonMomentumDataset,
                                               m_numPBins);
  const Double_t* binsMinEdgesK_orig = kdBinsK->SortOneDimBinEdges();
  Double_t* binsMinEdgesK =  const_cast<Double_t*>(binsMinEdgesK_orig);
  binsMinEdgesK[0] = 0.1;
  binsMinEdgesK[m_numPBins + 1] = 50.;

// get a distribution that contains both pions to get a typical kinematics for the binning scheme
  inputTree->Draw("SlowPionSVDdEdxTrackMomentum/0.140* (event%2==0) + PionDSVDdEdxTrackMomentum/0.140* (event%2==1)", "", "goff",
                  ((inputTree->GetEntries()) / m_numPBins)*m_numPBins);
  Double_t* PionMomentumDataset = inputTree->GetV1();

  TKDTreeBinning* kdBinsPi = new TKDTreeBinning(((inputTree->GetEntries()) / m_numPBins)*m_numPBins, 1, PionMomentumDataset,
                                                m_numPBins);
  const Double_t* binsMinEdgesPi_orig = kdBinsPi->SortOneDimBinEdges();
  Double_t* binsMinEdgesPi =  const_cast<Double_t*>(binsMinEdgesPi_orig);
  binsMinEdgesPi[0] = 0.1;
  binsMinEdgesPi[m_numPBins + 1] = 50.;

  TH2F* hDstarK_bg_tr = new TH2F("hist_d1_321_trunc_bg_tr", "hist_d1_321_trunc_bg_tr", m_numPBins, binsMinEdgesK,
                                 m_numDEdxBins, 0, m_dedxCutoff);
  // the pion payload
  TH2F* hDstarPi_bg_tr = new TH2F("hist_d1_211_trunc_bg_tr", "hist_d1_211_trunc_bg_tr", m_numPBins, binsMinEdgesPi,
                                  m_numDEdxBins, 0, m_dedxCutoff);

  inputTree->Draw("KaonSVDdEdx:KaonSVDdEdxTrackMomentum/0.494>>hist_d1_321_trunc_bg_tr", "nSignalDstar_sw * (KaonSVDdEdx>0)", "goff");
  // the pion one will be built from both pions in the Dstar decay tree
  TH2F* hDstarPiPart1_bg_tr = (TH2F*)hDstarPi_bg_tr->Clone("hist_d1_211_truncPart1_bg_tr");
  TH2F* hDstarPiPart2_bg_tr = (TH2F*)hDstarPi_bg_tr->Clone("hist_d1_211_truncPart2_bg_tr");

  inputTree->Draw("PionDSVDdEdx:PionDSVDdEdxTrackMomentum/0.140>>hist_d1_211_truncPart1_bg_tr", "nSignalDstar_sw * (PionDSVDdEdx>0)",
                  "goff");
  inputTree->Draw("SlowPionSVDdEdx:SlowPionSVDdEdxTrackMomentum/0.140>>hist_d1_211_truncPart2_bg_tr",
                  "nSignalDstar_sw * (SlowPionSVDdEdx>0)", "goff");
  hDstarPi_bg_tr->Add(hDstarPiPart1_bg_tr);
  hDstarPi_bg_tr->Add(hDstarPiPart2_bg_tr);



  TCanvas* canvDstar = new TCanvas("canvDstar2", "canvDstar");
  // produce the 1D profiled


  TH1F* PionProfile_tr = (TH1F*)hDstarPi_tr->ProfileX("PionProfile_tr");
  PionProfile_tr->SetTitle("PionProfile");
  // canvDstar->SetTicky(1);
  PionProfile_tr->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
  PionProfile_tr->GetXaxis()->SetTitle("Momentum, GeV/c");
  PionProfile_tr->GetYaxis()->SetTitle("dE/dx");
  PionProfile_tr->SetLineColor(kRed);
  PionProfile_tr->Draw("SAME");

  TH1F* PionProfile_bg_tr = (TH1F*)hDstarPi_bg_tr->ProfileX("PionProfile_bg_tr");
  PionProfile_bg_tr->SetTitle("PionProfile");
  // canvLambda->SetTicky(1);
  PionProfile_bg_tr->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
  PionProfile_bg_tr->GetXaxis()->SetTitle("#beta*#gamma");
  PionProfile_bg_tr->GetYaxis()->SetTitle("dE/dx");
  PionProfile_bg_tr->SetLineColor(kRed);


  canvDstar->Print("SVDdEdxCalibrationProfilePion.pdf");


  TH1F* KaonProfile_tr = (TH1F*)hDstarK_tr->ProfileX("KaonProfile_tr");
  KaonProfile_tr->SetTitle("KaonProfile");
  KaonProfile_tr->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
  KaonProfile_tr->GetXaxis()->SetTitle("Momentum, GeV/c");
  KaonProfile_tr->GetYaxis()->SetTitle("dE/dx");
  KaonProfile_tr->SetLineColor(kRed);
  KaonProfile_tr->Draw("SAME");
  canvDstar->Print("SVDdEdxCalibrationProfileKaon.pdf");


  TH1F* KaonProfile_bg_tr = (TH1F*)hDstarK_bg_tr->ProfileX("KaonProfile_bg_tr");
  KaonProfile_bg_tr->SetTitle("KaonProfile");

  KaonProfile_bg_tr->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
  KaonProfile_bg_tr->GetXaxis()->SetTitle("#beta*#gamma");
  KaonProfile_bg_tr->GetYaxis()->SetTitle("dE/dx");
  KaonProfile_bg_tr->SetLineColor(kRed);



  //  normalisation
  hDstarK_tr->Sumw2();
  hDstarK_tr = Normalise2DHisto(hDstarK_tr);

  hDstarPi_tr->Sumw2();
  hDstarPi_tr = Normalise2DHisto(hDstarPi_tr);

  TList* histList = new TList;
  histList->Add(KaonProfile_tr);
  histList->Add(KaonProfile_bg_tr);
  histList->Add(hDstarK_tr);

  histList->Add(PionProfile_tr);
  histList->Add(PionProfile_bg_tr);
  histList->Add(hDstarPi_tr);

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


  TH2F* hGammaE_tr = new TH2F("hist_d1_11_trunc_tr", "hist_d1_11_trunc_tr", m_numPBins, pbins.data(), m_numDEdxBins, 0, m_dedxCutoff);

  TH2F* hGammaEPart1_tr = (TH2F*)hGammaE_tr->Clone("hist_d1_11_truncPart1_tr");
  TH2F* hGammaEPart2_tr = (TH2F*)hGammaE_tr->Clone("hist_d1_11_truncPart2_tr");

  preselTree->Draw("FirstElectronSVDdEdx:FirstElectronSVDdEdxTrackMomentum>>hist_d1_11_truncPart1_tr",
                   "FirstElectronSVDdEdx>0 && DIRA>0.995 && dr>1.2", "goff");
  preselTree->Draw("SecondElectronSVDdEdx:SecondElectronSVDdEdxTrackMomentum>>hist_d1_11_truncPart2_tr",
                   "SecondElectronSVDdEdx>0 && DIRA>0.995 && dr>1.2", "goff");
  hGammaE_tr->Add(hGammaEPart1_tr);
  hGammaE_tr->Add(hGammaEPart2_tr);

// get a distribution that contains both pions to get a typical kinematics for the binning scheme
  preselTree->Draw("FirstElectronSVDdEdxTrackMomentum/0.000511* (event%2==0) + SecondElectronSVDdEdxTrackMomentum/0.000511* (event%2==1)",
                   "", "goff", ((preselTree->GetEntries()) / m_numPBins)*m_numPBins);
  Double_t* ElectronMomentumDataset = preselTree->GetV1();

  TKDTreeBinning* kdBinsE = new TKDTreeBinning(((preselTree->GetEntries()) / m_numPBins)*m_numPBins, 1, ElectronMomentumDataset,
                                               m_numPBins);
  const Double_t* binsMinEdgesE_orig = kdBinsE->SortOneDimBinEdges();
  Double_t* binsMinEdgesE =  const_cast<Double_t*>(binsMinEdgesE_orig);
  binsMinEdgesE[0] = 0.;
  binsMinEdgesE[m_numPBins + 1] = 10000.;


  TH2F* hGammaE_bg_tr = new TH2F("hist_d1_11_trunc_bg_tr", "hist_d1_11_trunc_bg_tr", m_numPBins, binsMinEdgesE,
                                 m_numDEdxBins, 0, m_dedxCutoff);
  TH2F* hGammaEPart1_bg_tr = (TH2F*)hGammaE_bg_tr->Clone("hist_d1_11_truncPart1_bg_tr");
  TH2F* hGammaEPart2_bg_tr = (TH2F*)hGammaE_bg_tr->Clone("hist_d1_11_truncPart2_bg_tr");

  preselTree->Draw("FirstElectronSVDdEdx:FirstElectronSVDdEdxTrackMomentum/0.000511>>hist_d1_11_truncPart1_bg_tr",
                   "FirstElectronSVDdEdx>0 && DIRA>0.995 && dr>1.2 && FirstElectronSVDdEdx<1.8e6", "goff");
  preselTree->Draw("SecondElectronSVDdEdx:SecondElectronSVDdEdxTrackMomentum/0.000511>>hist_d1_11_truncPart2_bg_tr",
                   "SecondElectronSVDdEdx>0 && DIRA>0.995 && dr>1.2 && SecondElectronSVDdEdx<1.8e6", "goff");
  hGammaE_bg_tr->Add(hGammaEPart1_bg_tr);
  hGammaE_bg_tr->Add(hGammaEPart2_bg_tr);


  // produce the 1D profile (for data-MC comparisons)
  TCanvas* canvGamma = new TCanvas("canvGamma", "canvGamma");
  canvGamma->cd();


  TH1F* ElectronProfile_tr = (TH1F*)hGammaE_tr->ProfileX("ElectronProfile_tr");
  ElectronProfile_tr->SetTitle("ElectronProfile");
  ElectronProfile_tr->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
  ElectronProfile_tr->GetXaxis()->SetTitle("Momentum, GeV/c");
  ElectronProfile_tr->GetYaxis()->SetTitle("dE/dx");
  ElectronProfile_tr->SetLineColor(kRed);
  ElectronProfile_tr->Draw();


  TH1F* ElectronProfile_bg_tr = (TH1F*)hGammaE_bg_tr->ProfileX("ElectronProfile_bg_tr");
  ElectronProfile_bg_tr->SetTitle("ElectronProfile");

  ElectronProfile_bg_tr->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
  ElectronProfile_bg_tr->GetXaxis()->SetTitle("#beta*#gamma");
  ElectronProfile_bg_tr->GetYaxis()->SetTitle("dE/dx");
  ElectronProfile_bg_tr->SetLineColor(kRed);


  hGammaE_tr = Normalise2DHisto(hGammaE_tr);

  TList* histList = new TList;
  histList->Add(ElectronProfile_tr);
  histList->Add(ElectronProfile_bg_tr);
  histList->Add(hGammaE_tr);


  return histList;

}





//   if (preselTree->GetEntries() == 0) {
//     B2FATAL("The Gamma tree is empty, stopping here");
//   }
//   std::vector<double> pbins = CreatePBinningScheme();

//   TH2F* hGammaE = new TH2F("hist_d1_11_trunc", "hist_d1_11_trunc", m_numPBins, pbins.data(), m_numDEdxBins, 0, m_dedxCutoff);

//   TH2F* hGammaEPart1 = (TH2F*)hGammaE->Clone("hist_d1_11_truncPart1");
//   TH2F* hGammaEPart2 = (TH2F*)hGammaE->Clone("hist_d1_11_truncPart2");

//   preselTree->Draw("FirstElectronSVDdEdx:FirstElectronMomentum>>hist_d1_11_truncPart1", "FirstElectronSVDdEdx>0", "goff");
//   preselTree->Draw("SecondElectronSVDdEdx:SecondElectronMomentum>>hist_d1_11_truncPart2", "SecondElectronSVDdEdx>0", "goff");
//   hGammaE->Add(hGammaEPart1);
//   hGammaE->Add(hGammaEPart2);

//   // produce the 1D profile (for data-MC comparisons)
//   TCanvas* canvGamma = new TCanvas("canvGamma", "canvGamma");
//   if (m_isMakePlots) {
//     TH1F* ElectronProfile = (TH1F*)hGammaE->ProfileX("ElectronProfile");
//     ElectronProfile->SetTitle("ElectronProfile");
//     canvGamma->SetTicky(1);
//     ElectronProfile->GetYaxis()->SetRangeUser(0, m_dedxCutoff);
//     ElectronProfile->GetXaxis()->SetTitle("Momentum, GeV/c");
//     ElectronProfile->GetYaxis()->SetTitle("dE/dx");
//     ElectronProfile->Draw();
//     canvGamma->Print("SVDdEdxCalibrationProfileElectron.pdf");
//     TFile ElectronProfileFile("SVDdEdxCalibrationProfileElectron.root", "RECREATE");
//     ElectronProfile->Write();
//     ElectronProfileFile.Close();
//     canvGamma->SetTicky(0);
//   }

//   // for each momentum bin, normalize the pdf
//   // hGammaE normalisation
//   for (int pbin = 0; pbin <= m_numPBins + 1; pbin++) {
//     for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
//       // get rid of the bins with negative weights
//       if (hGammaE->GetBinContent(pbin, dedxbin) <= 1) {
//         hGammaE->SetBinContent(pbin, dedxbin, 0);
//       };
//     }

//     // create a projection (1D histogram) in a given momentum bin
//     TH1D* slice = (TH1D*)hGammaE->ProjectionY("slice", pbin, pbin);
//     // normalise, but ignore the cases with empty histograms
//     if (slice->Integral() > 0) {
//       slice->Scale(1. / slice->Integral());
//     }
//     // fill back the 2D histo with the result
//     for (int dedxbin = 0; dedxbin <= m_numDEdxBins + 1; dedxbin++) {
//       hGammaE->SetBinContent(pbin, dedxbin, slice->GetBinContent(dedxbin));
//     }
//   }

//   if (m_isMakePlots) {
//     hGammaE->Draw("COLZ");
//     canvGamma->Print("SVDdEdxCalibrationHistoGamma.pdf");
//   }

//   return *hGammaE;
// }



TList* SVDdEdxCalibrationAlgorithm::GenerateNewHistograms(std::shared_ptr<TTree> ttreeLambda, std::shared_ptr<TTree> ttreeDstar,
                                                          std::shared_ptr<TTree> ttreeGamma)
{

  TTree* treeLambda = LambdaMassFit(ttreeLambda);
  TList* histlist_Lambda = LambdaHistogramming(treeLambda);
  TH1F* ProtonProfile_bg_tr = (TH1F*) histlist_Lambda->FindObject("ProtonProfile_bg_tr");
  TH2F* histoP_2D = (TH2F*) histlist_Lambda->FindObject("hist_d1_2212_trunc_tr");

  TTree* treeDstar = DstarMassFit(ttreeDstar);
  TList* histlist_Dstar = DstarHistogramming(treeDstar);
  TH1F* PionProfile_bg_tr = (TH1F*) histlist_Dstar->FindObject("PionProfile_bg_tr");
  TH2F* histoPi_2D = (TH2F*) histlist_Dstar->FindObject("hist_d1_211_trunc_tr");
  TH1F* KaonProfile_bg_tr = (TH1F*) histlist_Dstar->FindObject("KaonProfile_bg_tr");
  TH2F* histoK_2D = (TH2F*) histlist_Dstar->FindObject("hist_d1_321_trunc_tr");

  TList* histlist_Gamma = GammaHistogramming(ttreeGamma);
  TH1F* ElectronProfile_bg_tr = (TH1F*) histlist_Gamma->FindObject("ElectronProfile_bg_tr");
  TH2F* histoE_2D = (TH2F*) histlist_Gamma->FindObject("hist_d1_11_trunc_tr");

  Int_t cred = TColor::GetColor("#e31a1c");
  PionProfile_bg_tr->SetMarkerSize(4);
  PionProfile_bg_tr->SetLineWidth(2);
  PionProfile_bg_tr->SetMarkerColor(cred);
  PionProfile_bg_tr->SetLineColor(cred);

  Int_t cpink = TColor::GetColor("#807dba");
  KaonProfile_bg_tr->SetMarkerSize(4);
  KaonProfile_bg_tr->SetLineWidth(2);
  KaonProfile_bg_tr->SetMarkerColor(cpink);
  KaonProfile_bg_tr->SetLineColor(cpink);

  Int_t cblue = TColor::GetColor("#084594");
  ProtonProfile_bg_tr->SetMarkerSize(4);
  ProtonProfile_bg_tr->SetLineWidth(2);
  ProtonProfile_bg_tr->SetMarkerColor(cblue);
  ProtonProfile_bg_tr->SetLineColor(cblue);

  Int_t cgreen = TColor::GetColor("#238b45");
  ElectronProfile_bg_tr->SetMarkerSize(4);
  ElectronProfile_bg_tr->SetLineWidth(2);
  ElectronProfile_bg_tr->SetMarkerColor(cgreen);
  ElectronProfile_bg_tr->SetLineColor(cgreen);

  for (int i = 1; i <= ProtonProfile_bg_tr->GetNbinsX();
       i++) if (ProtonProfile_bg_tr->GetBinError(i) / ProtonProfile_bg_tr->GetBinContent(i) > 0.15) ProtonProfile_bg_tr->SetBinContent(i,
               0);



  for (int i = 1; i <= KaonProfile_bg_tr->GetNbinsX();
       i++) if (KaonProfile_bg_tr->GetBinError(i) / KaonProfile_bg_tr->GetBinContent(i) > 0.15) KaonProfile_bg_tr->SetBinContent(i, 0);


  for (int i = 1; i <= PionProfile_bg_tr->GetNbinsX();
       i++) if (PionProfile_bg_tr->GetBinError(i) / PionProfile_bg_tr->GetBinContent(i) > 0.15) PionProfile_bg_tr->SetBinContent(i, 0);

  gStyle->SetOptStat(0);
  auto legend = new TLegend(0.5, 0.7, 0.9, 0.9);
  legend->AddEntry(PionProfile_bg_tr, "Pions", "lep");
  legend->AddEntry(KaonProfile_bg_tr, "Kaons", "lep");
  legend->AddEntry(ProtonProfile_bg_tr, "Protons", "lep");
  legend->Draw();


// fitting


//    TCanvas *c11 = new TCanvas("SimfitCanvas", "Canvas With All Hadrons", 10, 10, 1000, 700);

  PionProfile_bg_tr->GetYaxis()->SetRangeUser(5.e5, 5.5e6);
  KaonProfile_bg_tr->GetYaxis()->SetRangeUser(5.e5, 5.5e6);
  ProtonProfile_bg_tr->GetYaxis()->SetRangeUser(5.e5, 5.5e6);
//   PionProfile_bg_tr->Draw();
//    KaonProfile_bg_tr->Draw("SAME");
//    ProtonProfile_bg_tr->Draw("SAME");

//  gPad->SetLogx();
//  gPad->SetLogy();
//    legend->Draw();



  auto PionEdges = PionProfile_bg_tr->GetXaxis()->GetXbins()->GetArray();
  auto ProtonEdges = ProtonProfile_bg_tr->GetXaxis()->GetXbins()->GetArray();

  std::vector<float> CombinedEdgesVector;

  double borderline = 3.;

  for (int i = 0; i < ProtonProfile_bg_tr->GetNbinsX() + 1; i++)
    if (ProtonEdges[i] < borderline) CombinedEdgesVector.push_back(ProtonEdges[i]);


  for (int i = 0; i < PionProfile_bg_tr->GetNbinsX() + 1; i++)
    if (PionEdges[i] > borderline) CombinedEdgesVector.push_back(PionEdges[i]);


  TH1F* hCombined = new TH1F("hCombined", "histo_for_fit", CombinedEdgesVector.size() - 1, CombinedEdgesVector.data());

  int iterator = 1;
  for (int i = 1; i < ProtonProfile_bg_tr->GetNbinsX() + 1; i++)
    if (ProtonEdges[i - 1] < borderline) {
      hCombined->SetBinContent(i, ProtonProfile_bg_tr->GetBinContent(i));
      hCombined->SetBinError(i, ProtonProfile_bg_tr->GetBinError(i));
      iterator++;
    }


  for (int i = 1; i < PionProfile_bg_tr->GetNbinsX() + 1; i++)
    if (PionEdges[i - 1] > borderline) {

      hCombined->SetBinContent(iterator, PionProfile_bg_tr->GetBinContent(i));
      hCombined->SetBinError(iterator, PionProfile_bg_tr->GetBinError(i));
      iterator++;
    }


  TF1* BetaGammaFunctionPion = new TF1("BetaGammaFunctionPion", "[0] + [1] * x/[2] +  [5]/(x^2/[2]^2 + [3])**[4] + [6]* (x/[2])**0.5",
                                       0.01, 25.);

  BetaGammaFunctionPion->SetNpx(1000);

  BetaGammaFunctionPion->SetParameters(5.e5, 5.e3, 1, 0., 1., 5.e5, 1.e5);

  BetaGammaFunctionPion->SetParLimits(0, 3.e5, 8.e5);
  BetaGammaFunctionPion->SetParLimits(1, -1.e5, 1.e5);
  BetaGammaFunctionPion->SetParLimits(3, -10, 10);
  BetaGammaFunctionPion->SetParLimits(4, 0.5, 3);
  BetaGammaFunctionPion->SetParLimits(5, 3.e5, 8.e5);
  BetaGammaFunctionPion->SetParLimits(6, 0., 1.e6);

  BetaGammaFunctionPion->FixParameter(2, 1);
  PionProfile_bg_tr->Fit("BetaGammaFunctionPion", "0WLS", "", 0.4, 25);


  TF1* BetaGammaFunctionKaon = new TF1("BetaGammaFunctionKaon", "[0] + [1] * x/[2] +  [5]/(x^2/[2]^2 + [3])**[4]+ [6]* (x/[2])**0.5",
                                       0.01, 25.);
  gStyle->SetOptFit(1111);
  BetaGammaFunctionKaon->SetNpx(1000);
  BetaGammaFunctionKaon->SetParameters(5.e5, 5.e3, 1, 0., 1., 5.e5, 0.);

  BetaGammaFunctionKaon->SetParLimits(0, 3.e5, 8.e5);
  BetaGammaFunctionKaon->SetParLimits(1, -1.e5, 1.e5);
  BetaGammaFunctionKaon->SetParLimits(3, -10, 10);
  BetaGammaFunctionKaon->SetParLimits(4, 0.5, 3);
  BetaGammaFunctionKaon->SetParLimits(5, 3.e5, 8.e5);

  BetaGammaFunctionKaon->FixParameter(2, 1);
  BetaGammaFunctionKaon->FixParameter(6, 0);
  BetaGammaFunctionKaon->SetLineColor(KaonProfile_bg_tr->GetMarkerColor());

  KaonProfile_bg_tr->Fit("BetaGammaFunctionKaon", "0WLS", "", 0.4, 8);

  TF1* BetaGammaFunctionProton = new TF1("BetaGammaFunctionProton",
                                         "[0] + [1] * x/[2] +  [5]/(x^2/[2]^2 + [3])**[4]+ [6]* (x/[2])**0.5", 0.01, 25.);
  gStyle->SetOptFit(1111);
  BetaGammaFunctionProton->SetNpx(1000);
  BetaGammaFunctionProton->SetParameters(5.e5, 5.e3, 1, 0., 1., 5.e5, 0.);



  BetaGammaFunctionProton->SetParLimits(0, 3.e5, 8.e5);
  BetaGammaFunctionProton->SetParLimits(1, -1.e5, 1.e5);
  BetaGammaFunctionProton->SetParLimits(3, -10, 10);
  BetaGammaFunctionProton->SetParLimits(4, 0.5, 3);
  BetaGammaFunctionProton->SetParLimits(5, 3.e5, 8.e5);

  BetaGammaFunctionProton->FixParameter(2, 1);
  BetaGammaFunctionProton->FixParameter(6, 0);
  BetaGammaFunctionProton->SetLineColor(ProtonProfile_bg_tr->GetMarkerColor());

  TCanvas* c12 = new TCanvas("Simfit12", "", 10, 10, 1000, 700);
  gStyle->SetOptFit(1111);
  hCombined->Fit("BetaGammaFunctionProton", "0WLS", "", 0.35, 8);



  TCanvas* CombinedCanvas = new TCanvas("CombinedCanvas", "Combined histogram", 10, 10, 1000, 700);
  gStyle->SetOptFit(1111);



  PionProfile_bg_tr->Draw();
  PionProfile_bg_tr->GetListOfFunctions()->Add(BetaGammaFunctionPion);
  KaonProfile_bg_tr->Draw("SAME");
  KaonProfile_bg_tr->GetListOfFunctions()->Add(BetaGammaFunctionKaon);
  ProtonProfile_bg_tr->Draw("SAME");
  ProtonProfile_bg_tr->GetListOfFunctions()->Add(BetaGammaFunctionProton);
// BetaGammaFunctionPion->Draw("SAME");
// BetaGammaFunctionKaon->Draw("SAME");
// BetaGammaFunctionProton->Draw("SAME");
  gPad->SetLogx();
  gPad->SetLogy();
  CombinedCanvas->Print("HadronBetaGammaFits.pdf");


// electrons
  TCanvas* ElectronCanvas = new TCanvas("ElectronCanvas", "Electron histogram", 10, 10, 1000, 700);
  TF1* BetaGammaFunctionElectron = new TF1("BetaGammaFunctionElectron", "[0] + [1]* x", 1, 10000.);
  BetaGammaFunctionElectron->SetParameters(6.e5, 1);
  BetaGammaFunctionElectron->SetParLimits(0, 3.e5, 8.e5);
  BetaGammaFunctionElectron->SetParLimits(1, -1.e5, 1.e5);
  ElectronProfile_bg_tr->Fit("BetaGammaFunctionElectron", "0WLS", "", 100, 8000);

  ElectronProfile_bg_tr->SetMarkerSize(4);
  ElectronProfile_bg_tr->SetLineWidth(2);

  ElectronProfile_bg_tr->GetYaxis()->SetRangeUser(5e5, 1e6);
  ElectronProfile_bg_tr->GetListOfFunctions()->Add(BetaGammaFunctionElectron);
  ElectronProfile_bg_tr->Draw();

  gPad->SetLogx();
//  gPad->SetLogy();
  ElectronCanvas->Print("ElectronBetaGammaFits.pdf");

  TF1* MomentumFunctionElectron = (TF1*) BetaGammaFunctionElectron->Clone("MomentumFunctionElectron");
  MomentumFunctionElectron->SetParameter(2, 0.000511);
  MomentumFunctionElectron->SetRange(0.01, 5.5);


  TCanvas* c_E = new TCanvas("c_E", "overlay", 10, 10, 1000, 700);
  histoE_2D->Draw();
  MomentumFunctionElectron->SetLineColor(kRed);
  MomentumFunctionElectron->SetLineWidth(4);
  MomentumFunctionElectron->Draw("SAME");
  c_E->Print("fit_vs_2Dhisto_Electrons.pdf");

  TCanvas* c_pi = new TCanvas("c_pi", "overlay", 10, 10, 1000, 700);
  histoPi_2D->Draw();
  TF1* MomentumFunctionPion = (TF1*) BetaGammaFunctionPion->Clone("MomentumFunctionPion");
  MomentumFunctionPion->SetParameter(2, 0.140);
  MomentumFunctionPion->SetRange(0.01, 5.5);
// MomentumFunctionPion->Print("V");
// cout<<MomentumFunctionPion->GetParameter(2)<<endl;
  MomentumFunctionPion->SetLineColor(kRed);
  MomentumFunctionPion->SetLineWidth(4);
  MomentumFunctionPion->Draw("SAME");
  c_pi->Print("fit_vs_2Dhisto_pions.pdf");


  TCanvas* c_p = new TCanvas("c_p", "overlay", 10, 10, 1000, 700);
  histoP_2D->Draw();
  TF1* MomentumFunctionProton = (TF1*) BetaGammaFunctionProton->Clone("MomentumFunctionProton");
  MomentumFunctionProton->SetParameter(2, 0.938);
  MomentumFunctionProton->SetRange(0.01, 5.5);
// MomentumFunctionProton->Print("V");
// cout<<MomentumFunctionPion->GetParameter(2)<<endl;
  MomentumFunctionProton->SetLineColor(kRed);
  MomentumFunctionProton->SetLineWidth(4);
  MomentumFunctionProton->Draw("SAME");
  c_p->Print("fit_vs_2Dhisto_protons.pdf");

  TCanvas* c_k = new TCanvas("c_k", "overlay", 10, 10, 1000, 700);
  histoK_2D->Draw();
  TF1* MomentumFunctionKaon = (TF1*) BetaGammaFunctionKaon->Clone("MomentumFunctionKaon");
  MomentumFunctionKaon->SetParameter(2, 0.494);
  MomentumFunctionKaon->SetRange(0.01, 5.5);
// MomentumFunctionKaon->Print("V");
// cout<<MomentumFunctionPion->GetParameter(2)<<endl;
  MomentumFunctionKaon->SetLineColor(kRed);
  MomentumFunctionKaon->SetLineWidth(4);
  MomentumFunctionKaon->Draw("SAME");
  c_k->Print("fit_vs_2Dhisto_kaons.pdf");


  TF1* MomentumFunctionDeuteron = (TF1*) BetaGammaFunctionProton->Clone("MomentumFunctionDeuteron");
  MomentumFunctionDeuteron->SetParameter(2, 1.876);
  MomentumFunctionDeuteron->SetRange(0.01, 5.5);

  TF1* MomentumFunctionMuon = (TF1*) BetaGammaFunctionPion->Clone("MomentumFunctionMuon");
  MomentumFunctionMuon->SetParameter(2, 0.10565);
  MomentumFunctionMuon->SetRange(0.01, 5.5);




// resolution studies


// For resolution measurement, we need to take a ProjectionY of the data histograms in the momentum range where the dEdx is flat vs momentum. We use our educated guess of the flat range (e.g. 0.6-1 GeV for pions) and FindBin to figure out which bin numbers those momentum values correspond to.
  double PionRangeMin = 0.6;
  double PionRangeMax = 1.;
  double KaonRangeMin = 1.9;
  double KaonRangeMax = 3;
  double ElectronRangeMin = 1.;
  double ElectronRangeMax = 1.4;

  auto PionResolutionHistogram = histoPi_2D->ProjectionY("PionResolutionHistogram", histoPi_2D->GetXaxis()->FindBin(PionRangeMin),
                                                         histoPi_2D->GetXaxis()->FindBin(PionRangeMax));
  auto ElectronResolutionHistogram = histoE_2D->ProjectionY("ElectronResolutionHistogram",
                                                            histoE_2D->GetXaxis()->FindBin(ElectronRangeMin), histoE_2D->GetXaxis()->FindBin(ElectronRangeMax));
  auto KaonResolutionHistogram = histoK_2D->ProjectionY("histoK_reso_bins4252", histoK_2D->GetXaxis()->FindBin(KaonRangeMin),
                                                        histoK_2D->GetXaxis()->FindBin(KaonRangeMax));
// for protons, there is not enough data in the flat range.

  TCanvas* canvas_reso = new TCanvas("canvas_reso", " ", 1000, 700);
  PionResolutionHistogram->Draw();

  TF1* PionResolutionFunction = new TF1("PionResolutionFunction",
                                        "[0]*TMath::Landau(x, [1], [1]*[2])*TMath::Gaus(x, [1], [1]*[2]*[4]) + [3]*TMath::Gaus(x, [1], [1]*[2]*[5])", 100e3, 1500e3);

  PionResolutionFunction->SetParameters(1, 6.e5, 0.1, 0.5, 2, 1);
  PionResolutionFunction->SetParLimits(0, 0, 500);
  PionResolutionFunction->SetParLimits(1, 3.e5, 8.e5);
  PionResolutionFunction->SetParLimits(2, 0, 1);
  PionResolutionFunction->SetParLimits(3, 0, 500);
  PionResolutionFunction->SetParLimits(4, 0, 7);
  PionResolutionFunction->SetParLimits(5, 1, 7);
  PionResolutionFunction->SetNpx(1000);
  auto FitResultResolutionPion = PionResolutionHistogram->Fit(PionResolutionFunction, "RWLSI");

  // cout << "relative resolution: " << PionResolutionFunction->GetParameter(2) << endl;



  // TCanvas* canvas_reso2 = new TCanvas("canvas_reso2", " ", 1000, 700);

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

  KaonResolutionHistogram->Draw();
  auto FitResultResolutionKaon = KaonResolutionHistogram->Fit(KaonResolutionFunction, "RWLSI");


  if ((FitResultResolutionKaon->Status() > 1)
      && (FitResultResolutionPion->Status() <= 1)) KaonResolutionFunction = (TF1*)
            PionResolutionFunction->Clone("KaonResolutionFunction");

  // cout << "relative resolution: " << KaonResolutionFunction->GetParameter(2) << endl;


  // TCanvas* canvas_reso3 = new TCanvas("canvas_reso3", " ", 1000, 700);

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

  ElectronResolutionHistogram->Draw();
  ElectronResolutionHistogram->Fit(ElectronResolutionFunction, "RWLSI");

  // cout << "relative resolution: " << ElectronResolutionFunction->GetParameter(2) << endl;

  TCanvas* canvas_resolutions = new TCanvas("canvas_resolutions", "Resolutions", 1200, 800);
  canvas_resolutions->Divide(3, 1);
  canvas_resolutions->cd(1); PionResolutionHistogram->Draw();
  canvas_resolutions->cd(2); KaonResolutionHistogram->Draw();
  canvas_resolutions->cd(3); ElectronResolutionHistogram->Draw();

  canvas_resolutions->Print("SVDdEdxResolutions.pdf");



  double BiasCorrectionPion = PionResolutionFunction->GetParameter(1) - MomentumFunctionPion->Eval((
                                PionRangeMax + PionRangeMin) / 2.);
  // cout << BiasCorrectionPion << endl;


  TH2D* histoPi_2D_new = prepare_new_histogram(histoPi_2D, Form("%s_new", histoPi_2D->GetName()), MomentumFunctionPion,
                                               PionResolutionFunction, BiasCorrectionPion);
  TCanvas* canvas_result_pi = new TCanvas("canvas_result_pi", " ", 1000, 700);
  histoPi_2D->Draw("COL");
  canvas_result_pi->Print("Original_payload_Pi.pdf");
  histoPi_2D_new->Draw("COL");
  canvas_result_pi->Print("Generated_payload_Pi.pdf");


  TH2D* histoPi_2D_diff = (TH2D*) histoPi_2D->Clone("histoPi_2D_diff");
  histoPi_2D_diff->Add(histoPi_2D_new, histoPi_2D, 1, -1);
  TCanvas* canvas_result_pi_diff = new TCanvas("canvas_result_pi_diff", " ", 1000, 700);
  histoPi_2D_diff->SetMinimum(-0.15);
  histoPi_2D_diff->SetMaximum(0.15);

  histoPi_2D_diff->Draw("COLZ");
  canvas_result_pi_diff->Print("Generated_payload_Pi_diff.pdf");



  double BiasCorrectionKaon = KaonResolutionFunction->GetParameter(1) - MomentumFunctionKaon->Eval((
                                KaonRangeMax + KaonRangeMin) / 2.);
  // cout << BiasCorrectionKaon << endl;

  TH2D* histoK_2D_new = prepare_new_histogram(histoK_2D, Form("%s_new", histoK_2D->GetName()), MomentumFunctionKaon,
                                              KaonResolutionFunction, BiasCorrectionKaon);
  TCanvas* canvas_result_k = new TCanvas("canvas_result_k", " ", 1000, 700);
  histoK_2D->Draw("COL");
  canvas_result_k->Print("Original_payload_K.pdf");
  histoK_2D_new->Draw("COL");
  canvas_result_k->Print("Generated_payload_K.pdf");


  TH2D* histoK_2D_diff = (TH2D*) histoK_2D->Clone("histoK_2D_diff");
  histoK_2D_diff->Add(histoK_2D_new, histoK_2D, 1, -1);
  TCanvas* canvas_result_k_diff = new TCanvas("canvas_result_k_diff", " ", 1000, 700);
  histoK_2D_diff->SetMinimum(-0.15);
  histoK_2D_diff->SetMaximum(0.15);
  histoK_2D_diff->Draw("COLZ");
  canvas_result_k_diff->Print("Generated_payload_K_diff.pdf");


  TH2D* histoP_2D_new = prepare_new_histogram(histoP_2D, Form("%s_new", histoP_2D->GetName()), MomentumFunctionProton,
                                              KaonResolutionFunction, BiasCorrectionKaon);
  TCanvas* canvas_result_p = new TCanvas("canvas_result_p", " ", 1000, 700);
  histoP_2D->Draw("COL");
  canvas_result_p->Print("Original_payload_P.pdf");
  histoP_2D_new->Draw("COL");
  canvas_result_p->Print("Generated_payload_P.pdf");


  TH2D* histoP_2D_diff = (TH2D*) histoP_2D->Clone("histoP_2D_diff");
  histoP_2D_diff->Add(histoP_2D_new, histoP_2D, 1, -1);
  TCanvas* canvas_result_p_diff = new TCanvas("canvas_result_p_diff", " ", 1000, 700);
  histoP_2D_diff->SetMinimum(-0.15);
  histoP_2D_diff->SetMaximum(0.15);
  histoP_2D_diff->Draw("COLZ");
  canvas_result_p_diff->Print("Generated_payload_P_diff.pdf");


  TH2D* histoDeut_2D_new = prepare_new_histogram(histoP_2D, "histoDeut_2D_new", MomentumFunctionDeuteron, KaonResolutionFunction,
                                                 BiasCorrectionKaon);
  histoDeut_2D_new->SetTitle("hist_d1_1000010020_trunc");
  TCanvas* canvas_result_Deut = new TCanvas("canvas_result_Deut", " ", 1000, 700);
  histoDeut_2D_new->Draw("COL");
  canvas_result_Deut->Print("Generated_payload_Deut.pdf");


  TH2D* histoMu_2D_new = prepare_new_histogram(histoPi_2D, "histoMu_2D_new", MomentumFunctionMuon, PionResolutionFunction,
                                               BiasCorrectionPion);
  histoMu_2D_new->SetTitle("hist_d1_13_trunc");
  TCanvas* canvas_result_Mu = new TCanvas("canvas_result_Mu", " ", 1000, 700);
  histoMu_2D_new->Draw("COL");
  canvas_result_Mu->Print("Generated_payload_Mu.pdf");



  double BiasCorrectionElectron = ElectronResolutionFunction->GetParameter(1) - MomentumFunctionElectron->Eval((
                                    ElectronRangeMax + ElectronRangeMin) / 2.);
  // cout << BiasCorrectionElectron << endl;
  TH2D* histoE_2D_new = prepare_new_histogram(histoE_2D, Form("%s_new", histoE_2D->GetName()), MomentumFunctionElectron,
                                              ElectronResolutionFunction, BiasCorrectionElectron);

  TCanvas* canvas_result_E = new TCanvas("canvas_result_E", " ", 1000, 700);

  histoE_2D_new->Draw("COL");
  canvas_result_E->Print("Generated_payload_E.pdf");


  TH2D* histoE_2D_diff = (TH2D*) histoE_2D->Clone("histoE_2D_diff");
  histoE_2D_diff->Add(histoE_2D_new, histoE_2D, 1, -1);
  TCanvas* canvas_result_E_diff = new TCanvas("canvas_result_E_diff", " ", 1000, 700);
  histoE_2D_diff->SetMinimum(-0.15);
  histoE_2D_diff->SetMaximum(0.15);
  histoE_2D_diff->Draw("COLZ");
  canvas_result_E_diff->Print("Generated_payload_E_diff.pdf");

  TCanvas* canvas_result_all = new TCanvas("canvas_result_all", "Generated payloads", 2100, 700);
  canvas_result_all->Divide(3, 2);
  canvas_result_all->cd(1); histoE_2D_new->Draw("COLZ");
  canvas_result_all->cd(2); histoMu_2D_new->Draw("COLZ");
  canvas_result_all->cd(3); histoPi_2D_new->Draw("COLZ");
  canvas_result_all->cd(4); histoK_2D_new->Draw("COLZ");
  canvas_result_all->cd(5); histoP_2D_new->Draw("COLZ");
  canvas_result_all->cd(6); histoDeut_2D_new->Draw("COLZ");
  canvas_result_all->Print("SVDdEdxGeneratedPayloads.pdf");

  TCanvas* canvas_result_data = new TCanvas("canvas_result_data", "Data distributions", 2100, 700);
  canvas_result_data->Divide(3, 2);
  canvas_result_data->cd(1); histoE_2D->Draw("COLZ");
  canvas_result_data->cd(3); histoPi_2D->Draw("COLZ");
  canvas_result_data->cd(4); histoK_2D->Draw("COLZ");
  canvas_result_data->cd(5); histoP_2D->Draw("COLZ");
  canvas_result_data->Print("SVDdEdxDataDistributions.pdf");

  TCanvas* canvas_result_diff = new TCanvas("canvas_result_diff", "Residuals", 2100, 700);
  canvas_result_diff->Divide(3, 2);
  canvas_result_diff->cd(1); histoE_2D_diff->Draw("COLZ");
  canvas_result_diff->cd(3); histoPi_2D_diff->Draw("COLZ");
  canvas_result_diff->cd(4); histoK_2D_diff->Draw("COLZ");
  canvas_result_diff->cd(5); histoP_2D_diff->Draw("COLZ");
  canvas_result_diff->Print("SVDdEdxResiduals.pdf");

  histoE_2D_new->SetName("histoE_2D_new");
  histoMu_2D_new->SetName("histoMu_2D_new");
  histoPi_2D_new->SetName("histoPi_2D_new");
  histoK_2D_new->SetName("histoK_2D_new");
  histoP_2D_new->SetName("histoP_2D_new");
  histoDeut_2D_new->SetName("histoDeut_2D_new");

  TList* histList = new TList;
  histList->Add(histoE_2D_new);
  histList->Add(histoMu_2D_new);
  histList->Add(histoPi_2D_new);
  histList->Add(histoK_2D_new);
  histList->Add(histoP_2D_new);
  histList->Add(histoDeut_2D_new);

  return histList;

}