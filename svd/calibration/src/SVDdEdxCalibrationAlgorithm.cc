/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/calibration/SVDdEdxCalibrationAlgorithm.h>
// #include <framework/database/DBObjPtr.h>
// #include <reconstruction/dbobjects/DedxPDFs.h>
#include <svd/dbobjects/SVDdEdxPDFs.h>
#include <tuple>

#include <TROOT.h>
#include <TStyle.h>
#include <TMath.h>
#include <TFile.h>
#include <TTree.h>
#include <TColor.h>
#include <TLegend.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <TH2F.h>
#include <TAxis.h>

#include <RooDataSet.h>
#include <RooRealVar.h>
#include <RooFormulaVar.h>
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
  // get the DB dEdx PDFs

  const auto exprun = getRunList()[0];
  B2INFO("ExpRun used for calibration: " << exprun.first << " " << exprun.second);
  // updateDBObjPtrs(1, exprun.second, exprun.first);

  // DBObjPtr<DedxPDFs> m_DBDedxPDFs;
  // if (!m_DBDedxPDFs)
  //   B2FATAL("No VXD dEdx PDFs available");
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
  TH2F h_LambdaP = LambdaMassFit(ttreeLambda);
  auto [h_DstarK, h_DstarPi, h_DstarMu] = DstarMassFit(ttreeDstar);
  TH2F h_GammaE = GammaHistogram(ttreeGamma);
  std::vector<double> pbins = CreatePBinningScheme();
  TH2F h_Empty("h_Empty", "A histogram returned if we cannot calibrate", m_numPBins, pbins.data(), m_numDEdxBins, 0, m_dedxCutoff);
  for (int pbin = 1; pbin <= m_numPBins; pbin++) {
    for (int dedxbin = 1; dedxbin <= m_numDEdxBins; dedxbin++) {
      h_Empty.SetBinContent(pbin, dedxbin, 0.01);
    };
  }

  B2INFO("Histograms are ready, proceed to creating the payload object...");
  //    Belle2::DedxPDFs* pdfs = (Belle2::DedxPDFs*)runfile->Get("DedxPDFs");
  std::vector<TH2F*> hDedxPDFs(6);

  std::array<std::string, 6> part = {"Electron", "Muon", "Pion", "Kaon", "Proton", "Deuteron"};
  // std::array<std::string, 3> det = {"PXD", "VXD", "CDC"};

  TCanvas* candEdx = new TCanvas("candEdx", "SVD dEdx payloads", 1200, 700);
  candEdx->Divide(3, 2);
  gStyle->SetOptStat(11);
  // TFile* fout = new TFile("hDedxPDFs_payload.root", "RECREATE");
  // for (const auto& idet : det) {
  for (bool trunmean : {false, true}) {
    for (int iPart = 0; iPart < 6; iPart++) {
      // if (idet.compare("PXD") == 0)
      //   hDedxPDFs[iPart] = (TH2F*)m_DBDedxPDFs->getPXDPDF(iPart, trunmean);
      // else if (idet.compare("VXD") == 0) {

      if (iPart == 0 && trunmean) {
        hDedxPDFs[iPart] = &h_GammaE;
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 1 && trunmean) {
        hDedxPDFs[iPart] = &h_DstarMu;
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 2 && trunmean) {
        hDedxPDFs[iPart] = &h_DstarPi;
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 3 && trunmean) {
        hDedxPDFs[iPart] = &h_DstarK;
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      } else if (iPart == 4 && trunmean) {
        hDedxPDFs[iPart] = &h_LambdaP;
        payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);
      }
      // else if (iPart == 5 && trunmean)
      // { // for deuteron, set equal to that of the proton

      //   hDedxPDFs[iPart] = &h_LambdaP;
      //   hDedxPDFs[iPart]->SetTitle("hist_d1_1000010020_trunc");
      //   payload->setPDF(*hDedxPDFs[iPart], iPart, trunmean);

      // }
      else
        hDedxPDFs[iPart] = &h_Empty;//(TH2F*)m_DBDedxPDFs->getSVDPDF(iPart, trunmean);

      candEdx->cd(iPart + 1);
      hDedxPDFs[iPart]->SetTitle(Form("%s; p(GeV/c) of %s; dE/dx", hDedxPDFs[iPart]->GetTitle(), part[iPart].data()));
      hDedxPDFs[iPart]->DrawCopy("colz");

      if (m_isMakePlots) {
        candEdx->SaveAs("Plots_SVDDedxPDFs_wTruncMean.pdf");
      }
      // } else if (idet.compare("CDC") == 0)
      //   hDedxPDFs[iPart] = (TH2F*)m_DBDedxPDFs->getCDCPDF(iPart, trunmean);
      // hDedxPDFs[iPart]->Write();
    }
    // candEdx->SetTitle(Form("Likehood dist. of charged particles from %s, trunmean = %s", idet.data(), check.str().data()));
  }
  // }

  // fout->Close();

  // TFile* fout_read = new TFile("hDedxPDFs_payload.root", "READ"); // not sure if it's the best way to handle this
  // TClonesArray dedxPDFs("Belle2::DedxPDFs");
  // auto payload = new Belle2::DedxPDFs(fout_read);
  // new (dedxPDFs[0]) DedxPDFs(fout_read);
  saveCalibration(payload, "SVDdEdxPDFs"); //"DedxPDFs");

  B2INFO("SVD dE/dx calibration done!");

  // fout_read->Close();
  return c_OK;
}

TH2F SVDdEdxCalibrationAlgorithm::LambdaMassFit(std::shared_ptr<TTree> preselTree)
{
  B2INFO("Configuring the Lambda fit...");
  gROOT->SetBatch(true);
  RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING);

  RooRealVar InvM("InvM", "m(p^{+}#pi^{-})", 1.1, 1.13, "GeV/c^{2}");

  RooRealVar p_p("p_p", "momentum for p", -1.e8, 1.e8);
  RooRealVar p_SVDdEdx("p_SVDdEdx", "", -1.e8, 1.e8);

  RooRealVar exp("exp", "experiment number", 0, 1.e5);
  RooRealVar run("run", "run number", 0, 1.e7);

  auto variables = new RooArgSet();

  variables->add(InvM);

  variables->add(p_p);
  variables->add(p_SVDdEdx);
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

  RooRealVar resolutionParamL("resolutionParamL", "resolutionParamL", 0.4, 5.e-4, 1.0);
  RooRealVar resolutionParamR("resolutionParamR", "resolutionParamR", 0.4, 5.e-4, 1.0);
  RooFormulaVar sigmaBifurGaussL1("sigmaBifurGaussL1", "resolutionParamL*GaussSigma", RooArgSet(resolutionParamL, GaussSigma));
  RooFormulaVar sigmaBifurGaussR1("sigmaBifurGaussR1", "resolutionParamR*GaussSigma", RooArgSet(resolutionParamR, GaussSigma));
  RooBifurGauss LambdaBifurGauss("LambdaBifurGauss", "LambdaBifurGauss", InvM, GaussMean, sigmaBifurGaussL1, sigmaBifurGaussR1);

  RooRealVar resolutionParam2("resolutionParam2", "resolutionParam2", 0.2, 5.e-4, 1.0);
  RooFormulaVar sigmaBifurGaussL2("sigmaBifurGaussL2", "resolutionParam2*GaussSigma", RooArgSet(resolutionParam2, GaussSigma));
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

  totalPDFLambda.plotOn(LambdaFitFrame, Components("LambdaSignalPDF"), LineColor(TColor::GetColor("#d73027")));
  totalPDFLambda.plotOn(LambdaFitFrame, Components("BkgPolyPDF"), LineColor(TColor::GetColor("#fc8d59")));
  totalPDFLambda.plotOn(LambdaFitFrame, LineColor(TColor::GetColor("#4575b4")));

  LambdaFitFrame->GetXaxis()->SetTitle("m(p#pi^{-}) (GeV/c^{2})");

  LambdaFitFrame->Draw();

  if (m_isMakePlots) {
    canvLambda->Print("SVDdEdxCalibrationFitLambda.pdf");
  }
  RooStats::SPlot* sPlotDatasetLambda = new RooStats::SPlot("sData", "An SPlot", *LambdaDataset, &totalPDFLambda,
                                                            RooArgList(nSignalLambda, nBkgLambda));

  for (int iEvt = 0; iEvt < 5; iEvt++) {
    if (TMath::Abs(sPlotDatasetLambda->GetSWeight(iEvt, "nSignalLambda") + sPlotDatasetLambda->GetSWeight(iEvt,
                   "nBkgLambda") - 1) > 5.e-3)
      B2FATAL("Lambda: sPlot error: sum of weights not equal to 1");
  }

  RooDataSet* LambdaDatasetw_sig = new RooDataSet(LambdaDataset->GetName(), LambdaDataset->GetTitle(), LambdaDataset,
                                                  *LambdaDataset->get());

  RooDataSet::setDefaultStorageType(RooAbsData::Tree);
  ((RooTreeDataStore*)(LambdaDatasetw_sig->store())->tree())->SetName("treeLambda_sw");
  TTree* treeLambda_sw = LambdaDatasetw_sig->GetClonedTree();

  B2INFO("Lambda: sPlot done. Proceed to histogramming");

  std::vector<double> pbins = CreatePBinningScheme();

  TH2F* h_LambdaP = new TH2F("hist_d1_2212_trunc", "hist_d1_2212_trunc", m_numPBins, pbins.data(), m_numDEdxBins, 0, m_dedxCutoff);

  treeLambda_sw->Draw("p_SVDdEdx:p_p>>hist_d1_2212_trunc", "nSignalLambda_sw * (p_p>0.15) * (p_SVDdEdx>0)", "goff");

  // for each momentum bin, normalize the pdf

  // h_LambdaP normalisation
  for (int pbin = 1; pbin <= m_numPBins; pbin++) {
    for (int dedxbin = 1; dedxbin <= m_numDEdxBins; dedxbin++) {
      // get rid of the bins with negative weights
      if (h_LambdaP->GetBinContent(pbin, dedxbin) < 0) {
        h_LambdaP->SetBinContent(pbin, dedxbin, 0);
      };
    }
    // create a projection (1D histogram) in a given momentum bin
    TH1D* slice = (TH1D*)h_LambdaP->ProjectionY("slice", pbin, pbin);
    // normalise, but ignore the cases with empty histograms
    if (slice->Integral() > 0) {
      slice->Scale(1. / slice->Integral());
    }
    // fill back the 2D histo with the result
    for (int dedxbin = 1; dedxbin <= m_numDEdxBins; dedxbin++) {
      h_LambdaP->SetBinContent(pbin, dedxbin, slice->GetBinContent(dedxbin));
    }
  }

  if (m_isMakePlots) {
    h_LambdaP->Draw("COLZ");
    canvLambda->Print("SVDdEdxCalibrationHistoLambda.pdf");
  }

  return *h_LambdaP;
}

std::tuple<TH2F, TH2F, TH2F> SVDdEdxCalibrationAlgorithm::DstarMassFit(std::shared_ptr<TTree> preselTree)
{
  B2INFO("Configuring the Dstar fit...");
  gROOT->SetBatch(true);
  RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING);

  RooRealVar deltaM("deltaM", "m(D*)-m(D^{0})", 0.139545, 0.151, "GeV/c^{2}");

  RooRealVar K_p("K_p", "momentum for Kaon(GeV)", -1.e8, 1.e8);
  RooRealVar K_SVDdEdx("K_SVDdEdx", "", -1.e8, 1.e8);
  RooRealVar pi_p("pi_p", "momentum for pion(GeV)", -1.e8, 1.e8);
  RooRealVar pi_SVDdEdx("pi_SVDdEdx", "", -1.e8, 1.e8);
  RooRealVar piS_p("piS_p", "momentum for slow pion(GeV)", -1.e8, 1.e8);
  RooRealVar piS_SVDdEdx("piS_SVDdEdx", "", -1.e8, 1.e8);

  RooRealVar exp("exp", "experiment number", 0, 1.e5);
  RooRealVar run("run", "run number", 0, 1.e8);

  auto variables = new RooArgSet();
  variables->add(deltaM);
  variables->add(K_p);
  variables->add(K_SVDdEdx);
  variables->add(pi_p);
  variables->add(pi_SVDdEdx);
  variables->add(piS_p);
  variables->add(piS_SVDdEdx);
  variables->add(exp);
  variables->add(run);

  RooDataSet* DstarDataset = new RooDataSet("DstarDataset", "DstarDataset", preselTree.get(), *variables);

  if (DstarDataset->sumEntries() == 0) {
    B2FATAL("The Dstar dataset is empty, stopping here");
  }

  RooPlot* myframe = DstarDataset->plotOn(deltaM.frame());

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

  totalPDFDstar.plotOn(myframe, LineColor(TColor::GetColor("#4575b4")));

  double chisquare = myframe->chiSquare();
  B2INFO("Dstar: Fit chi2 = " << chisquare);
  totalPDFDstar.paramOn(myframe, Layout(0.66, 0.96, 0.93), Format("NEU", AutoPrecision(2)));

  totalPDFDstar.plotOn(myframe, Components("DstarSignalPDF"), LineColor(TColor::GetColor("#d73027")));
  totalPDFDstar.plotOn(myframe, Components("DstarBkgPDF"), LineColor(TColor::GetColor("#fc8d59")));
  totalPDFDstar.plotOn(myframe, LineColor(TColor::GetColor("#4575b4")));

  myframe->GetXaxis()->SetTitle("#Deltam [GeV/c^2]");
  TCanvas* canvDstar = new TCanvas("canvDstar", "canvDstar");
  canvDstar->cd();

  myframe->Draw();

  if (m_isMakePlots) {
    canvDstar->Print("SVDdEdxCalibrationFitDstar.pdf");
  }

  /////////////////// SPlot ///////////////////////////////////////////////////////////

  RooStats::SPlot* sPlotDatasetDstar = new RooStats::SPlot("sData", "An SPlot", *DstarDataset, &totalPDFDstar,
                                                           RooArgList(nSignalDstar, nBkgDstar));

  for (int iEvt = 0; iEvt < 5; iEvt++) {
    if (TMath::Abs(sPlotDatasetDstar->GetSWeight(iEvt, "nSignalDstar") + sPlotDatasetDstar->GetSWeight(iEvt, "nBkgDstar") - 1) > 5.e-3)
      B2FATAL("Dstar: sPlot error: sum of weights not equal to 1");
  }

  RooDataSet* DstarDatasetw_sig = new RooDataSet(DstarDataset->GetName(), DstarDataset->GetTitle(), DstarDataset,
                                                 *DstarDataset->get());

  RooDataSet::setDefaultStorageType(RooAbsData::Tree);
  ((RooTreeDataStore*)(DstarDatasetw_sig->store())->tree())->SetName("treeDstar_sw");
  TTree* treeDstar_sw = DstarDatasetw_sig->GetClonedTree();

  B2INFO("Dstar: sPlot done. Proceed to histogramming");

  std::vector<double> pbins = CreatePBinningScheme();

  // the kaon payload
  TH2F* h_DstarK = new TH2F("hist_d1_321_trunc", "hist_d1_321_trunc", m_numPBins, pbins.data(),
                            m_numDEdxBins, 0, m_dedxCutoff);
  // the pion payload
  TH2F* h_DstarPi = new TH2F("hist_d1_211_trunc", "hist_d1_211_trunc", m_numPBins, pbins.data(),
                             m_numDEdxBins, 0, m_dedxCutoff);

  treeDstar_sw->Draw("K_SVDdEdx:K_p>>hist_d1_321_trunc", "nSignalDstar_sw * (K_SVDdEdx>0)", "goff");
  // the pion one will be built from both pions in the Dstar decay tree
  TH2F* h_DstarPiPart1 = (TH2F*)h_DstarPi->Clone("hist_d1_211_truncPart1");
  TH2F* h_DstarPiPart2 = (TH2F*)h_DstarPi->Clone("hist_d1_211_truncPart2");

  treeDstar_sw->Draw("pi_SVDdEdx:pi_p>>hist_d1_211_truncPart1", "nSignalDstar_sw * (pi_SVDdEdx>0)", "goff");
  treeDstar_sw->Draw("piS_SVDdEdx:piS_p>>hist_d1_211_truncPart2", "nSignalDstar_sw * (piS_SVDdEdx>0)", "goff");
  h_DstarPi->Add(h_DstarPiPart1);
  h_DstarPi->Add(h_DstarPiPart2);

  // the current strategy assumes that the muon and pion payloads are indistinguishable: clone the pion one
  TH2F* h_DstarMu = (TH2F*)h_DstarPi->Clone("hist_d1_13_trunc");
  h_DstarMu->SetTitle("hist_d1_13_trunc");
  // h_DstarK normalisation
  // for each momentum bin, normalize the pdf

  for (int pbin = 1; pbin <= m_numPBins; pbin++) {
    for (int dedxbin = 1; dedxbin <= m_numDEdxBins; dedxbin++) {
      // get rid of the bins with negative weights
      if (h_DstarK->GetBinContent(pbin, dedxbin) < 0) {
        h_DstarK->SetBinContent(pbin, dedxbin, 0);
      };
    }
    // create a projection (1D histogram) in a given momentum bin
    TH1D* slice = (TH1D*)h_DstarK->ProjectionY("slice", pbin, pbin);
    // normalise, but ignore the cases with empty histograms
    if (slice->Integral() > 0) {
      slice->Scale(1. / slice->Integral());
    }
    // fill back the 2D histo with the result
    for (int dedxbin = 1; dedxbin <= m_numDEdxBins; dedxbin++) {
      h_DstarK->SetBinContent(pbin, dedxbin, slice->GetBinContent(dedxbin));
    }
  }

  // h_DstarPi normalisation
  for (int pbin = 1; pbin <= m_numPBins; pbin++) {
    for (int dedxbin = 1; dedxbin <= m_numDEdxBins; dedxbin++) {
      // get rid of the bins with negative weights
      if (h_DstarPi->GetBinContent(pbin, dedxbin) < 0) {
        h_DstarPi->SetBinContent(pbin, dedxbin, 0);
      };
    }
    // create a projection (1D histogram) in a given momentum bin
    TH1D* slice = (TH1D*)h_DstarPi->ProjectionY("slice", pbin, pbin);
    // normalise, but ignore the cases with empty histograms
    if (slice->Integral() > 0) {
      slice->Scale(1. / slice->Integral());
    }
    // fill back the 2D histo with the result
    for (int dedxbin = 1; dedxbin <= m_numDEdxBins; dedxbin++) {
      h_DstarPi->SetBinContent(pbin, dedxbin, slice->GetBinContent(dedxbin));
    }
  }

  // h_DstarMu normalisation
  for (int pbin = 1; pbin <= m_numPBins; pbin++) {
    for (int dedxbin = 1; dedxbin <= m_numDEdxBins; dedxbin++) {
      // get rid of the bins with negative weights
      if (h_DstarMu->GetBinContent(pbin, dedxbin) < 0) {
        h_DstarMu->SetBinContent(pbin, dedxbin, 0);
      };
    }
    // create a projection (1D histogram) in a given momentum bin
    TH1D* slice = (TH1D*)h_DstarMu->ProjectionY("slice", pbin, pbin);
    // normalise, but ignore the cases with empty histograms
    if (slice->Integral() > 0) {
      slice->Scale(1. / slice->Integral());
    }
    // fill back the 2D histo with the result
    for (int dedxbin = 1; dedxbin <= m_numDEdxBins; dedxbin++) {
      h_DstarMu->SetBinContent(pbin, dedxbin, slice->GetBinContent(dedxbin));
    }
  }
  if (m_isMakePlots) {
    h_DstarK->Draw("COLZ");
    canvDstar->Print("SVDdEdxCalibrationHistoDstarK.pdf");
    h_DstarPi->Draw("COLZ");
    canvDstar->Print("SVDdEdxCalibrationHistoDstarPi.pdf");
    h_DstarMu->Draw("COLZ");
    canvDstar->Print("SVDdEdxCalibrationHistoDstarMu.pdf");
  }

  return std::make_tuple(*h_DstarK, *h_DstarPi, *h_DstarMu);
}

TH2F SVDdEdxCalibrationAlgorithm::GammaHistogram(std::shared_ptr<TTree> preselTree)
{
  B2INFO("Histogramming the converted photon selection...");
  gROOT->SetBatch(true);

  if (preselTree->GetEntries() == 0) {
    B2FATAL("The Gamma tree is empty, stopping here");
  }
  std::vector<double> pbins = CreatePBinningScheme();

  TH2F* h_GammaE = new TH2F("hist_d1_11_trunc", "hist_d1_11_trunc", m_numPBins, pbins.data(), m_numDEdxBins, 0, m_dedxCutoff);

  TH2F* h_GammaEPart1 = (TH2F*)h_GammaE->Clone("hist_d1_11_truncPart1");
  TH2F* h_GammaEPart2 = (TH2F*)h_GammaE->Clone("hist_d1_11_truncPart2");

  preselTree->Draw("e_1_SVDdEdx:e_1_p>>hist_d1_11_truncPart1", "e_1_SVDdEdx>0", "goff");
  preselTree->Draw("e_2_SVDdEdx:e_2_p>>hist_d1_11_truncPart2", "e_2_SVDdEdx>0", "goff");
  h_GammaE->Add(h_GammaEPart1);
  h_GammaE->Add(h_GammaEPart2);

  // for each momentum bin, normalize the pdf
  // h_GammaE normalisation
  for (int pbin = 1; pbin <= m_numPBins; pbin++) {
    for (int dedxbin = 1; dedxbin <= m_numDEdxBins; dedxbin++) {
      // get rid of the bins with negative weights
      if (h_GammaE->GetBinContent(pbin, dedxbin) < 0) {
        h_GammaE->SetBinContent(pbin, dedxbin, 0);
      };
    }

    // create a projection (1D histogram) in a given momentum bin
    TH1D* slice = (TH1D*)h_GammaE->ProjectionY("slice", pbin, pbin);
    // normalise, but ignore the cases with empty histograms
    if (slice->Integral() > 0) {
      slice->Scale(1. / slice->Integral());
    }
    // fill back the 2D histo with the result
    for (int dedxbin = 1; dedxbin <= m_numDEdxBins; dedxbin++) {
      h_GammaE->SetBinContent(pbin, dedxbin, slice->GetBinContent(dedxbin));
    }
  }

  if (m_isMakePlots) {
    TCanvas* canvGamma = new TCanvas("canvGamma", "canvGamma");
    h_GammaE->Draw("COLZ");
    canvGamma->Print("SVDdEdxCalibrationHistoGamma.pdf");
  }

  return *h_GammaE;
}
