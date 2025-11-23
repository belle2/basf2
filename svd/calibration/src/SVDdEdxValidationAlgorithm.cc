/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/calibration/SVDdEdxValidationAlgorithm.h>

#include <tuple>
#include <vector>
#include <string>

#include <TROOT.h>
#include <TStyle.h>
#include <TMath.h>
#include <TFile.h>
#include <TColor.h>
#include <TLegend.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TAxis.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TCut.h>

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
#include <ROOT/RDataFrame.hxx>

using namespace ROOT;
using namespace RooFit;
using namespace Belle2;

SVDdEdxValidationAlgorithm::SVDdEdxValidationAlgorithm() : CalibrationAlgorithm("SVDdEdxValidationCollector"),
  m_isMakePlots(true)
{
  setDescription("SVD dE/dx validation algorithm");
}

/* Main calibration method */
CalibrationAlgorithm::EResult SVDdEdxValidationAlgorithm::calibrate()
{
  gROOT->SetBatch(true);

  // const auto exprun = getRunList()[0];
  // B2INFO("ExpRun used for calibration: " << exprun.first << " " << exprun.second);

  // Get data objects
  auto TTreeLambda = getObjectPtr<TTree>("Lambda");
  auto TTreeDstar = getObjectPtr<TTree>("Dstar");
  auto TTreeGamma = getObjectPtr<TTree>("Gamma");

  if (TTreeLambda->GetEntries() < m_MinEvtsPerTree) {
    B2WARNING("Not enough data for calibration.");
    return c_NotEnoughData;
  }

  // call the calibration functions
  TTree* TTreeLambdaSW = LambdaMassFit(TTreeLambda);
  TTree* TTreeDstarSW = DstarMassFit(TTreeDstar);
  TTree* TTreeGammaWrap = TTreeGamma.get();

  std::vector<TString> PIDDetectors;
  PIDDetectors.push_back("SVDonly");
  if (m_fullValidation) {
    PIDDetectors.push_back("ALL");
    PIDDetectors.push_back("noSVD");
  }

  std::map<TTree*, TString> SWeightNameMap = {
    {TTreeGammaWrap, "1"},
    {TTreeDstarSW, "nSignalDstar_sw"},
    {TTreeLambdaSW, "nSignalLambda_sw"}
  };

  for (const TString& PIDDetectorsName : PIDDetectors) {
    PlotEfficiencyPlots(PIDDetectorsName, TTreeGammaWrap, SWeightNameMap[TTreeGammaWrap], "FirstElectron", "electron", TTreeDstarSW,
                        SWeightNameMap[TTreeDstarSW], "PionD", "pion",
                        "BinaryElectronPionID",
                        "0.5", m_NumEffBins, 0., m_MomHighEff);
    PlotEfficiencyPlots(PIDDetectorsName, TTreeGammaWrap, SWeightNameMap[TTreeGammaWrap], "FirstElectron", "electron", TTreeDstarSW,
                        SWeightNameMap[TTreeDstarSW], "Kaon", "kaon",
                        "BinaryElectronKaonID", "0.5",
                        m_NumEffBins, 0., m_MomHighEff);
    PlotEfficiencyPlots(PIDDetectorsName, TTreeLambdaSW, SWeightNameMap[TTreeLambdaSW], "Proton", "proton", TTreeDstarSW,
                        SWeightNameMap[TTreeDstarSW], "PionD", "pion",
                        "BinaryProtonPionID", "0.5",
                        m_NumEffBins, 0.25, m_MomHighEff);
    PlotEfficiencyPlots(PIDDetectorsName, TTreeLambdaSW, SWeightNameMap[TTreeLambdaSW], "Proton", "proton", TTreeDstarSW,
                        SWeightNameMap[TTreeDstarSW], "Kaon", "kaon",
                        "BinaryProtonKaonID", "0.5",
                        m_NumEffBins, 0.25, m_MomHighEff);
    PlotEfficiencyPlots(PIDDetectorsName, TTreeDstarSW, SWeightNameMap[TTreeDstarSW], "PionD", "pion", TTreeDstarSW,
                        SWeightNameMap[TTreeDstarSW],
                        "Kaon", "kaon",
                        "BinaryPionKaonID", "0.5", m_NumEffBins,
                        0., m_MomHighEff);
    PlotEfficiencyPlots(PIDDetectorsName, TTreeDstarSW, SWeightNameMap[TTreeDstarSW], "Kaon", "kaon", TTreeDstarSW,
                        SWeightNameMap[TTreeDstarSW],
                        "PionD", "pion",
                        "BinaryKaonPionID", "0.5", m_NumEffBins,
                        0., m_MomHighEff);
  }

  if (m_fullValidation) {
    PlotROCCurve(TTreeGammaWrap, SWeightNameMap[TTreeGammaWrap], "FirstElectron", "electron", TTreeDstarSW,
                 SWeightNameMap[TTreeDstarSW], "PionD",
                 "pion", "BinaryElectronPionID");
    PlotROCCurve(TTreeGammaWrap, SWeightNameMap[TTreeGammaWrap], "FirstElectron", "electron", TTreeDstarSW,
                 SWeightNameMap[TTreeDstarSW], "Kaon",
                 "kaon", "BinaryElectronKaonID");
    PlotROCCurve(TTreeLambdaSW, SWeightNameMap[TTreeLambdaSW], "Proton", "proton", TTreeDstarSW, SWeightNameMap[TTreeDstarSW], "PionD",
                 "pion",
                 "BinaryProtonPionID");
    PlotROCCurve(TTreeLambdaSW, SWeightNameMap[TTreeLambdaSW], "Proton", "proton", TTreeDstarSW, SWeightNameMap[TTreeDstarSW], "Kaon",
                 "kaon",
                 "BinaryProtonKaonID");
    PlotROCCurve(TTreeDstarSW, SWeightNameMap[TTreeDstarSW], "PionD", "pion", TTreeDstarSW, SWeightNameMap[TTreeDstarSW], "Kaon",
                 "kaon",
                 "BinaryPionKaonID");
    PlotROCCurve(TTreeDstarSW, SWeightNameMap[TTreeDstarSW], "Kaon", "kaon", TTreeDstarSW, SWeightNameMap[TTreeDstarSW], "PionD",
                 "pion",
                 "BinaryKaonPionID");
  }
  B2INFO("SVD dE/dx validation done!");

  return c_OK;
}

// generic efficiency and fake rate
void SVDdEdxValidationAlgorithm::PlotEfficiencyPlots(const TString& PIDDetectorsName, TTree* SignalTree, TString SignalWeightName,
                                                     TString SignalVarName, TString SignalVarNameFull, TTree* FakeTree, TString FakeWeightName, TString FakeVarName,
                                                     TString FakeVarNameFull, TString PIDVarName, TString PIDCut, unsigned int nbins, double MomLow, double MomHigh)
{

  if ((SignalTree == nullptr) || (FakeTree == nullptr)) {
    B2FATAL("Invalid dataset, stopping here");
  }

  if ((SignalTree->GetEntries() == 0) || (FakeTree->GetEntries() == 0)) {
    B2FATAL("The dataset is empty, stopping here");
  }

  if ((SignalTree->GetBranch(Form("%sMomentum", SignalVarName.Data())) == nullptr)
      || (FakeTree->GetBranch(Form("%sMomentum", FakeVarName.Data())) == nullptr)) {
    B2FATAL("Check the provided branch name, stopping here");
  }

  TString SignalFiducialCut = "(1>0)"; // placeholder for a possible sanity cut
  TString FakesFiducialCut = "(1>0)";

  // Produce the plots of the SVD PID distribution
  if (PIDDetectorsName == "SVDonly") {
    SignalTree->Draw(Form("%s%s%s>>hSignalPIDDistribution(100,0.,1.)", SignalVarName.Data(), PIDVarName.Data(),
                          PIDDetectorsName.Data()),
                     SignalWeightName + Form("* (%sMomentum>%f && %sMomentum<%f)", SignalVarName.Data(), MomLow, SignalVarName.Data(), MomHigh), "goff");
    TH1F* hSignalPIDDistribution = (TH1F*)gDirectory->Get("hSignalPIDDistribution");
    hSignalPIDDistribution->Scale(1. / hSignalPIDDistribution->Integral());
    hSignalPIDDistribution->GetXaxis()->SetTitle(PIDVarName + PIDDetectorsName + " for " + SignalVarNameFull);
    hSignalPIDDistribution->GetYaxis()->SetTitle("Candidates, normalised");
    hSignalPIDDistribution->SetMaximum(1.35 * hSignalPIDDistribution->GetMaximum());

    SignalTree->Draw(Form("%sElectronLLSVDonly>>hSignalElectronLLDistribution(100,-17.,3.)", SignalVarName.Data()),
                     SignalWeightName + Form("* (%sMomentum>%f && %sMomentum<%f)", SignalVarName.Data(), MomLow, SignalVarName.Data(), MomHigh), "goff");
    TH1F* hSignalElectronLLDistribution = (TH1F*)gDirectory->Get("hSignalElectronLLDistribution");
    SignalTree->Draw(Form("%sPionLLSVDonly>>hSignalPionLLDistribution(100,-17.,3.)", SignalVarName.Data()),
                     SignalWeightName + Form("* (%sMomentum>%f && %sMomentum<%f)", SignalVarName.Data(), MomLow, SignalVarName.Data(), MomHigh), "goff");
    TH1F* hSignalPionLLDistribution = (TH1F*)gDirectory->Get("hSignalPionLLDistribution");
    SignalTree->Draw(Form("%sKaonLLSVDonly>>hSignalKaonLLDistribution(100,-17.,3.)", SignalVarName.Data()),
                     SignalWeightName + Form("* (%sMomentum>%f && %sMomentum<%f)", SignalVarName.Data(), MomLow, SignalVarName.Data(), MomHigh), "goff");
    TH1F* hSignalKaonLLDistribution = (TH1F*)gDirectory->Get("hSignalKaonLLDistribution");
    SignalTree->Draw(Form("%sProtonLLSVDonly>>hSignalProtonLLDistribution(100,-17.,3.)", SignalVarName.Data()),
                     SignalWeightName + Form("* (%sMomentum>%f && %sMomentum<%f)", SignalVarName.Data(), MomLow, SignalVarName.Data(), MomHigh), "goff");
    TH1F* hSignalProtonLLDistribution = (TH1F*)gDirectory->Get("hSignalProtonLLDistribution");

    // same but only for tracks that are expected to actually have SVD dEdx info
    SignalTree->Draw(Form("%sElectronLLSVDonly>>hSignalElectronLLDistributionGood(100,-17.,3.)", SignalVarName.Data()),
                     SignalWeightName + Form("* (%sSVDdEdx>0) * (%sMomentum>%f && %sMomentum<%f)", SignalVarName.Data(), SignalVarName.Data(), MomLow,
                                             SignalVarName.Data(), MomHigh), "goff");
    TH1F* hSignalElectronLLDistributionGood = (TH1F*)gDirectory->Get("hSignalElectronLLDistributionGood");
    SignalTree->Draw(Form("%sPionLLSVDonly>>hSignalPionLLDistributionGood(100,-17.,3.)", SignalVarName.Data()),
                     SignalWeightName + Form("* (%sSVDdEdx>0) * (%sMomentum>%f && %sMomentum<%f)", SignalVarName.Data(), SignalVarName.Data(), MomLow,
                                             SignalVarName.Data(), MomHigh), "goff");
    TH1F* hSignalPionLLDistributionGood = (TH1F*)gDirectory->Get("hSignalPionLLDistributionGood");
    SignalTree->Draw(Form("%sKaonLLSVDonly>>hSignalKaonLLDistributionGood(100,-17.,3.)", SignalVarName.Data()),
                     SignalWeightName + Form("* (%sSVDdEdx>0) * (%sMomentum>%f && %sMomentum<%f)", SignalVarName.Data(), SignalVarName.Data(), MomLow,
                                             SignalVarName.Data(), MomHigh), "goff");
    TH1F* hSignalKaonLLDistributionGood = (TH1F*)gDirectory->Get("hSignalKaonLLDistributionGood");
    SignalTree->Draw(Form("%sProtonLLSVDonly>>hSignalProtonLLDistributionGood(100,-17.,3.)", SignalVarName.Data()),
                     SignalWeightName + Form("* (%sSVDdEdx>0) * (%sMomentum>%f && %sMomentum<%f)", SignalVarName.Data(), SignalVarName.Data(), MomLow,
                                             SignalVarName.Data(), MomHigh), "goff");
    TH1F* hSignalProtonLLDistributionGood = (TH1F*)gDirectory->Get("hSignalProtonLLDistributionGood");


    hSignalElectronLLDistribution->Scale(1. / hSignalElectronLLDistribution->Integral());
    hSignalPionLLDistribution->Scale(1. / hSignalPionLLDistribution->Integral());
    hSignalKaonLLDistribution->Scale(1. / hSignalKaonLLDistribution->Integral());
    hSignalProtonLLDistribution->Scale(1. / hSignalProtonLLDistribution->Integral());

    hSignalElectronLLDistributionGood->Scale(1. / hSignalElectronLLDistributionGood->Integral());
    hSignalPionLLDistributionGood->Scale(1. / hSignalPionLLDistributionGood->Integral());
    hSignalKaonLLDistributionGood->Scale(1. / hSignalKaonLLDistributionGood->Integral());
    hSignalProtonLLDistributionGood->Scale(1. / hSignalProtonLLDistributionGood->Integral());

    hSignalElectronLLDistribution->GetXaxis()->SetTitle(PIDVarName + "ElectronLLSVDonly");
    hSignalElectronLLDistribution->GetYaxis()->SetTitle("Candidates, normalised");
    hSignalElectronLLDistribution->SetMaximum(1.35 * hSignalElectronLLDistribution->GetMaximum());

    hSignalPionLLDistribution->GetXaxis()->SetTitle(PIDVarName + "PionLLSVDonly");
    hSignalPionLLDistribution->GetYaxis()->SetTitle("Candidates, normalised");
    hSignalPionLLDistribution->SetMaximum(1.35 * hSignalPionLLDistribution->GetMaximum());

    hSignalKaonLLDistribution->GetXaxis()->SetTitle(PIDVarName + "KaonLLSVDonly");
    hSignalKaonLLDistribution->GetYaxis()->SetTitle("Candidates, normalised");
    hSignalKaonLLDistribution->SetMaximum(1.35 * hSignalKaonLLDistribution->GetMaximum());

    hSignalProtonLLDistribution->GetXaxis()->SetTitle(PIDVarName + "ProtonLLSVDonly");
    hSignalProtonLLDistribution->GetYaxis()->SetTitle("Candidates, normalised");
    hSignalProtonLLDistribution->SetMaximum(1.35 * hSignalProtonLLDistribution->GetMaximum());

    hSignalElectronLLDistributionGood->GetXaxis()->SetTitle(PIDVarName + "ElectronLLSVDonly");
    hSignalElectronLLDistributionGood->GetYaxis()->SetTitle("Candidates, normalised");
    hSignalElectronLLDistributionGood->SetMaximum(1.35 * hSignalElectronLLDistributionGood->GetMaximum());

    hSignalPionLLDistributionGood->GetXaxis()->SetTitle(PIDVarName + "PionLLSVDonly");
    hSignalPionLLDistributionGood->GetYaxis()->SetTitle("Candidates, normalised");
    hSignalPionLLDistributionGood->SetMaximum(1.35 * hSignalPionLLDistributionGood->GetMaximum());

    hSignalKaonLLDistributionGood->GetXaxis()->SetTitle(PIDVarName + "KaonLLSVDonly");
    hSignalKaonLLDistributionGood->GetYaxis()->SetTitle("Candidates, normalised");
    hSignalKaonLLDistributionGood->SetMaximum(1.35 * hSignalKaonLLDistributionGood->GetMaximum());

    hSignalProtonLLDistributionGood->GetXaxis()->SetTitle(PIDVarName + "ProtonLLSVDonly");
    hSignalProtonLLDistributionGood->GetYaxis()->SetTitle("Candidates, normalised");
    hSignalProtonLLDistributionGood->SetMaximum(1.35 * hSignalProtonLLDistributionGood->GetMaximum());

    TCanvas* DistribCanvas = new TCanvas("DistribCanvas", "", 600, 600);
    gPad->SetTopMargin(0.05);
    gPad->SetRightMargin(0.05);
    gPad->SetLeftMargin(0.13);
    gPad->SetBottomMargin(0.12);

    hSignalPIDDistribution->SetLineWidth(2);
    hSignalPIDDistribution->SetLineColor(TColor::GetColor("#2166ac"));
    hSignalPIDDistribution->Draw("hist");

    DistribCanvas->Print("SVDdEdxValidation_Distribution_" + SignalVarNameFull + PIDVarName + PIDDetectorsName +
                         "_MomRange_" +
                         std::to_string(
                           MomLow)
                         .substr(0, 3) +
                         "_" + std::to_string(MomHigh).substr(0, 3) + ".pdf");

    hSignalElectronLLDistribution->SetLineWidth(2);
    hSignalPionLLDistribution->SetLineWidth(2);
    hSignalKaonLLDistribution->SetLineWidth(2);
    hSignalProtonLLDistribution->SetLineWidth(2);

    hSignalElectronLLDistributionGood->SetLineWidth(2);
    hSignalPionLLDistributionGood->SetLineWidth(2);
    hSignalKaonLLDistributionGood->SetLineWidth(2);
    hSignalProtonLLDistributionGood->SetLineWidth(2);

    hSignalElectronLLDistributionGood->SetLineColor(kBlack);
    hSignalPionLLDistributionGood->SetLineColor(kBlack);
    hSignalKaonLLDistributionGood->SetLineColor(kBlack);
    hSignalProtonLLDistributionGood->SetLineColor(kBlack);

    hSignalElectronLLDistribution->SetTitle("ElectronLL (SVD), all tracks");
    hSignalPionLLDistribution->SetTitle("PionLL (SVD), all tracks");
    hSignalKaonLLDistribution->SetTitle("KaonLL (SVD), all tracks");
    hSignalProtonLLDistribution->SetTitle("ProtonLL (SVD), all tracks");

    hSignalElectronLLDistributionGood->SetTitle("ElectronLL (SVD), tracks with dEdx info");
    hSignalPionLLDistributionGood->SetTitle("PionLL (SVD), tracks with dEdx info");
    hSignalKaonLLDistributionGood->SetTitle("KaonLL (SVD), tracks with dEdx info");
    hSignalProtonLLDistributionGood->SetTitle("ProtonLL (SVD), tracks with dEdx info");

    hSignalElectronLLDistribution->GetXaxis()->SetTitleSize(0.04);
    hSignalElectronLLDistribution->GetYaxis()->SetTitleSize(0.04);
    hSignalElectronLLDistribution->GetXaxis()->SetTitleOffset(1.0);
    hSignalElectronLLDistribution->GetYaxis()->SetTitleOffset(1.3);
    hSignalElectronLLDistribution->GetYaxis()->SetLabelSize(0.04);
    hSignalElectronLLDistribution->GetXaxis()->SetLabelSize(0.04);

    hSignalPionLLDistribution->GetXaxis()->SetTitleSize(0.04);
    hSignalPionLLDistribution->GetYaxis()->SetTitleSize(0.04);
    hSignalPionLLDistribution->GetXaxis()->SetTitleOffset(1.0);
    hSignalPionLLDistribution->GetYaxis()->SetTitleOffset(1.3);
    hSignalPionLLDistribution->GetYaxis()->SetLabelSize(0.04);
    hSignalPionLLDistribution->GetXaxis()->SetLabelSize(0.04);

    hSignalKaonLLDistribution->GetXaxis()->SetTitleSize(0.04);
    hSignalKaonLLDistribution->GetYaxis()->SetTitleSize(0.04);
    hSignalKaonLLDistribution->GetXaxis()->SetTitleOffset(1.0);
    hSignalKaonLLDistribution->GetYaxis()->SetTitleOffset(1.3);
    hSignalKaonLLDistribution->GetYaxis()->SetLabelSize(0.04);
    hSignalKaonLLDistribution->GetXaxis()->SetLabelSize(0.04);

    hSignalProtonLLDistribution->GetXaxis()->SetTitleSize(0.04);
    hSignalProtonLLDistribution->GetYaxis()->SetTitleSize(0.04);
    hSignalProtonLLDistribution->GetXaxis()->SetTitleOffset(1.0);
    hSignalProtonLLDistribution->GetYaxis()->SetTitleOffset(1.3);
    hSignalProtonLLDistribution->GetYaxis()->SetLabelSize(0.04);
    hSignalProtonLLDistribution->GetXaxis()->SetLabelSize(0.04);

    TCanvas* LLCanvas = new TCanvas("LLCanvas", "", 900, 700);

    gPad->SetTopMargin(0.05);
    gPad->SetRightMargin(0.05);
    gPad->SetLeftMargin(0.13);
    gPad->SetBottomMargin(0.12);

    LLCanvas->Divide(2, 2, 0.01, 0.01);
    LLCanvas->cd(1);
    hSignalElectronLLDistribution->Draw("hist");
    LLCanvas->cd(2);
    hSignalPionLLDistribution->Draw("hist");
    LLCanvas->cd(3);
    hSignalKaonLLDistribution->Draw("hist");
    LLCanvas->cd(4);
    hSignalProtonLLDistribution->Draw("hist");

    TCanvas* LLCanvasGood = new TCanvas("LLCanvasGood", "", 900, 700);

    gPad->SetTopMargin(0.05);
    gPad->SetRightMargin(0.05);
    gPad->SetLeftMargin(0.13);
    gPad->SetBottomMargin(0.12);

    LLCanvasGood->Divide(2, 2, 0.01, 0.01);
    LLCanvasGood->cd(1);
    hSignalElectronLLDistributionGood->Draw("hist");
    LLCanvasGood->cd(2);
    hSignalPionLLDistributionGood->Draw("hist");
    LLCanvasGood->cd(3);
    hSignalKaonLLDistributionGood->Draw("hist");
    LLCanvasGood->cd(4);
    hSignalProtonLLDistributionGood->Draw("hist");

    LLCanvas->Print("SVDdEdxValidation_LLDistributions_" + SignalVarNameFull +
                    "_SVDonly_MomRange_" +
                    std::to_string(
                      MomLow)
                    .substr(0, 3) +
                    "_" + std::to_string(MomHigh).substr(0, 3) + ".pdf");

    LLCanvasGood->Print("SVDdEdxValidation_LLDistributions_GoodSVDTracks_" + SignalVarNameFull +
                        "_SVDonly_MomRange_" +
                        std::to_string(
                          MomLow)
                        .substr(0, 3) +
                        "_" + std::to_string(MomHigh).substr(0, 3) + ".pdf");

    TFile DistribFile("SVDdEdxValidation_Distribution_" + SignalVarNameFull + PIDVarName + PIDDetectorsName +
                      "_MomRange_" +
                      std::to_string(
                        MomLow)
                      .substr(0, 3) +
                      "_" + std::to_string(MomHigh).substr(0, 3) + ".root",
                      "RECREATE");
    hSignalPIDDistribution->SetLineColor(kBlack);
    hSignalPIDDistribution->Write();
    DistribFile.Close();
    delete DistribCanvas;

    TFile LLDistribFile(TString("SVDdEdxValidation_LLDistributions_" + SignalVarNameFull + "_SVDonly_MomRange_" +
                                std::to_string(
                                  MomLow)
                                .substr(0, 3) +
                                "_" + std::to_string(MomHigh).substr(0, 3) + ".root"),
                        "RECREATE");
    hSignalElectronLLDistribution->Write();
    hSignalPionLLDistribution->Write();
    hSignalKaonLLDistribution->Write();
    hSignalProtonLLDistribution->Write();
    LLDistribFile.Close();
    delete LLCanvas;
    delete LLCanvasGood;
  }

  // ---------- Momentum distributions (for efficiency determination) ----------

  SignalTree->Draw(Form("%sMomentum>>hAllSignal(%i,%f,%f)", SignalVarName.Data(), nbins, MomLow, MomHigh),
                   SignalWeightName + " * (" + SignalFiducialCut + ")", "goff");
  SignalTree->Draw(Form("%sMomentum>>hSelectedSignal(%i,%f,%f)", SignalVarName.Data(), nbins, MomLow, MomHigh),
                   SignalWeightName + " * (" + SignalVarName + PIDVarName + PIDDetectorsName + ">" + PIDCut + "&&" + SignalFiducialCut +
                   ")",
                   "goff");

  FakeTree->Draw(Form("%sMomentum>>hAllFakes(%i,%f,%f)", FakeVarName.Data(), nbins, MomLow, MomHigh),
                 FakeWeightName + " * (" + FakesFiducialCut + ")", "goff");
  FakeTree->Draw(Form("%sMomentum>>hSelectedFakes(%i,%f,%f)", FakeVarName.Data(), nbins, MomLow, MomHigh),
                 FakeWeightName + " * (" + FakeVarName + PIDVarName + PIDDetectorsName + ">" + PIDCut + "&&" + FakesFiducialCut + ")",
                 "goff");

  TH1F* hAllSignal = (TH1F*)gDirectory->Get("hAllSignal");
  TH1F* hSelectedSignal = (TH1F*)gDirectory->Get("hSelectedSignal");
  TH1F* hAllFakes = (TH1F*)gDirectory->Get("hAllFakes");
  TH1F* hSelectedFakes = (TH1F*)gDirectory->Get("hSelectedFakes");

  // ---------- Add slow pions to the pion dataset ----------
  if (strncmp(SignalVarName.Data(), "PionD", 5) == 0) {
    SignalTree->Draw(Form("SlowPionMomentum>>hAllSignalSlow(%i,%f,%f)", nbins, MomLow, MomHigh),
                     SignalWeightName + " * (" + SignalFiducialCut + ")", "goff");
    SignalTree->Draw(Form("SlowPionMomentum>>hSelectedSignalSlow(%i,%f,%f)", nbins, MomLow, MomHigh),
                     SignalWeightName + " * (SlowPion" + PIDVarName + PIDDetectorsName + ">" + PIDCut + "&&" + SignalFiducialCut + ")", "goff");
    TH1F* hAllSignalSlow = (TH1F*)gDirectory->Get("hAllSignalSlow");
    TH1F* hSelectedSignalSlow = (TH1F*)gDirectory->Get("hSelectedSignalSlow");
    hAllSignal->Add(hAllSignalSlow);
    hSelectedSignal->Add(hSelectedSignalSlow);
  }

  if (strncmp(FakeVarName.Data(), "PionD", 5) == 0) {
    FakeTree->Draw(Form("SlowPionMomentum>>hAllFakesSlow(%i,%f,%f)", nbins, MomLow, MomHigh),
                   FakeWeightName + " * (" + FakesFiducialCut + ")",
                   "goff");
    FakeTree->Draw(Form("SlowPionMomentum>>hSelectedFakesSlow(%i,%f,%f)", nbins, MomLow, MomHigh),
                   FakeWeightName + " * (SlowPion" + PIDVarName + PIDDetectorsName + ">" + PIDCut + "&&" + FakesFiducialCut + ")", "goff");
    TH1F* hAllFakesSlow = (TH1F*)gDirectory->Get("hAllFakesSlow");
    TH1F* hSelectedFakesSlow = (TH1F*)gDirectory->Get("hSelectedFakesSlow");
    hAllFakes->Add(hAllFakesSlow);
    hSelectedFakes->Add(hSelectedFakesSlow);
  }

  TH1F* EffHistoSig = (TH1F*)hAllSignal->Clone("EffHistoSig");   // signal efficiency
  TH1F* EffHistoFake = (TH1F*)hAllFakes->Clone("EffHistoFake");  // fakes efficiency

  EffHistoSig->Divide(hSelectedSignal, hAllSignal);//, 1, 1, "B");
  EffHistoFake->Divide(hSelectedFakes, hAllFakes);//, 1, 1, "B");

  // PID plots
  TH1F* hBase = new TH1F("hBase", "", 100, 0.0, MomHigh);
  hBase->SetTitle(";Momentum [GeV];Efficiency");
  hBase->SetMaximum(1.20);
  hBase->SetMinimum(0.0);

  TLegend* tleg1 = new TLegend(0.63, 0.82, 0.93, 0.94);
  tleg1->AddEntry(EffHistoSig, SignalVarNameFull + " efficiency", "pl");
  tleg1->AddEntry(EffHistoFake, FakeVarNameFull + " fake rate", "pl");

  TCanvas* ResultCanvas = new TCanvas("ResultCanvas", "", 600, 600);
  gPad->SetTopMargin(0.05);
  gPad->SetRightMargin(0.05);
  gPad->SetLeftMargin(0.13);
  gPad->SetBottomMargin(0.12);

  ResultCanvas->SetGrid();
  hBase->Draw();
  EffHistoSig->SetMarkerSize(1.5);
  EffHistoSig->SetMarkerStyle(22);
  EffHistoSig->SetMarkerColor(TColor::GetColor("#2166ac"));
  EffHistoSig->SetLineColor(TColor::GetColor("#2166ac"));
  EffHistoSig->Draw("P,same");

  EffHistoFake->SetMarkerSize(1.5);
  EffHistoFake->SetMarkerStyle(23);
  EffHistoFake->SetMarkerColor(TColor::GetColor("#ef8a62"));
  EffHistoFake->SetLineColor(TColor::GetColor("#ef8a62"));
  EffHistoFake->Draw("P,same");

  tleg1->Draw("same");

  hBase->SetStats(0);
  hBase->GetXaxis()->SetTitleSize(0.04);
  hBase->GetYaxis()->SetTitleSize(0.04);
  hBase->GetXaxis()->SetTitleOffset(1.0);
  hBase->GetYaxis()->SetTitleOffset(1.3);
  hBase->GetYaxis()->SetLabelSize(0.04);
  hBase->GetXaxis()->SetLabelSize(0.04);

  // std::setprecision(2);
  ResultCanvas->Print("SVDdEdxValidation_Efficiency_" + SignalVarNameFull + "_vs_" + FakeVarNameFull + PIDVarName + "_" +
                      PIDDetectorsName +
                      "_Cut" +
                      PIDCut + "_MomRange_" + std::to_string(MomLow).substr(0, 3) + "_" + std::to_string(MomHigh).substr(0, 3) + ".pdf");
  TFile ResultFile("SVDdEdxValidation_Efficiency_" + SignalVarNameFull + "_vs_" + FakeVarNameFull + PIDVarName + "_" +
                   PIDDetectorsName +
                   "_Cut" +
                   PIDCut + "_MomRange_" + std::to_string(MomLow).substr(0, 3) + "_" + std::to_string(MomHigh).substr(0, 3) + ".root",
                   "RECREATE");
  EffHistoSig->SetLineColor(kBlack);
  EffHistoSig->SetMarkerColor(kBlack);
  EffHistoFake->SetLineColor(kBlack);
  EffHistoFake->SetMarkerColor(kBlack);
  EffHistoSig->Write();
  EffHistoFake->Write();
  ResultFile.Close();
  delete ResultCanvas;
  delete hBase;
}

void SVDdEdxValidationAlgorithm::PlotROCCurve(TTree* SignalTree, TString SignalWeightName, TString SignalVarName,
                                              TString SignalVarNameFull, TTree* FakeTree, TString FakeWeightName, TString FakeVarName, TString FakeVarNameFull,
                                              TString PIDVarName)
{

  if ((SignalTree == nullptr) || (FakeTree == nullptr)) {
    B2FATAL("Invalid dataset, stopping here");
  }

  if ((SignalTree->GetEntries() == 0) || (FakeTree->GetEntries() == 0)) {
    B2FATAL("The dataset is empty, stopping here");
  }

  if ((SignalTree->GetBranch(Form("%sMomentum", SignalVarName.Data())) == nullptr)
      || (FakeTree->GetBranch(Form("%sMomentum", FakeVarName.Data())) == nullptr)) {
    B2FATAL("Check the provided branch name, stopping here");
  }

  std::vector<TString> PIDDetectors;
  PIDDetectors.clear();
  PIDDetectors.push_back("ALL");
  PIDDetectors.push_back("noSVD");

  std::vector<double> SignalEfficiencyALL, FakeEfficiencyALL;
  SignalEfficiencyALL.reserve(m_NumROCpoints);
  FakeEfficiencyALL.reserve(m_NumROCpoints);
  std::vector<double> SignalEfficiencynoSVD, FakeEfficiencynoSVD;
  SignalEfficiencynoSVD.reserve(m_NumROCpoints);
  FakeEfficiencynoSVD.reserve(m_NumROCpoints);

  TString SignalFiducialCut = SignalVarName + PIDVarName + "noSVD>=0"; // sanity cuts to reject events with NaN
  TString FakesFiducialCut = FakeVarName + PIDVarName + "noSVD>=0";
  TString SignalFiducialCutSlow = "SlowPion" + PIDVarName + "noSVD>=0";
  TString FakesFiducialCutSlow = "SlowPion" + PIDVarName + "noSVD>=0";

  // calculate efficiencies

  TCut AllSignalCut = SignalFiducialCut * Form("%sMomentum>%f && %sMomentum<%f", SignalVarName.Data(), m_MomLowROC,
                                               SignalVarName.Data(), m_MomHighROC);

  double AllSignalIntegral, SelectedSignalIntegral;

  auto DataFrameSignalAll = RDataFrame(*SignalTree).Filter(AllSignalCut.GetTitle());

  if (SignalWeightName == "1") {
    AllSignalIntegral = DataFrameSignalAll.Count().GetValue();
  } else {
    AllSignalIntegral = DataFrameSignalAll.Sum(SignalWeightName).GetValue();
  }

  std::unique_ptr<ROOT::RDF::RNode> DataFrameSlowSignalAll;

  if (strncmp(SignalVarName.Data(), "PionD", 5) == 0) {
    TString SignalVarNameSlow = "SlowPion";
    TCut AllSignalCutSlow = TCut(SignalFiducialCut) * TCut(SignalFiducialCutSlow) * Form("(%sMomentum>%f && %sMomentum<%f)",
                            SignalVarNameSlow.Data(), m_MomLowROC, SignalVarNameSlow.Data(), m_MomHighROC);
    DataFrameSlowSignalAll = std::make_unique<ROOT::RDF::RNode>(RDataFrame(*SignalTree).Filter(AllSignalCutSlow.GetTitle()));

    if (SignalWeightName == "1") {
      AllSignalIntegral += DataFrameSlowSignalAll->Count().GetValue();
    } else {
      AllSignalIntegral += DataFrameSlowSignalAll->Sum(SignalWeightName).GetValue();
    }
  }

  for (unsigned int i = 0; i < PIDDetectors.size(); i++) {
    for (unsigned int j = 0; j < m_NumROCpoints; ++j) {
      delete gROOT->FindObject("PIDCut");

      // scan cut values from 0 to 1, with a denser scan closer to 0 or 1, to get a nicer ROC curve
      double x = 1. / m_NumROCpoints * j;
      TString PIDCut = TString::Format("%f", 1. / (1 + TMath::Power(x / (1 - x), -3)));

      TCut SelectedSignalCut = Form("(%s%s%s > %s)", SignalVarName.Data(), PIDVarName.Data(), PIDDetectors[i].Data(), PIDCut.Data());

      if (SignalWeightName == "1") {
        SelectedSignalIntegral = DataFrameSignalAll.Filter(SelectedSignalCut.GetTitle()).Count().GetValue();
      } else {
        SelectedSignalIntegral = DataFrameSignalAll.Filter(SelectedSignalCut.GetTitle()).Sum(SignalWeightName).GetValue();
      }

      // special treatment for pions: add also the slow pions from Dstar to gain low-momentum coverage
      if (strncmp(SignalVarName.Data(), "PionD", 5) == 0) {
        TString SignalVarNameSlow = "SlowPion";
        TCut SelectedSignalCutSlow = Form("(%s%s%s > %s)", SignalVarNameSlow.Data(), PIDVarName.Data(), PIDDetectors[i].Data(),
                                          PIDCut.Data());

        if (SignalWeightName == "1") {
          SelectedSignalIntegral += DataFrameSlowSignalAll->Filter(SelectedSignalCutSlow.GetTitle()).Count().GetValue();
        } else {
          SelectedSignalIntegral += DataFrameSlowSignalAll->Filter(SelectedSignalCutSlow.GetTitle()).Sum(SignalWeightName).GetValue();
        }
      }

      if (PIDDetectors[i] == "ALL") {
        SignalEfficiencyALL.push_back(SelectedSignalIntegral / AllSignalIntegral);
      }

      if (PIDDetectors[i] == "noSVD") {
        SignalEfficiencynoSVD.push_back(SelectedSignalIntegral / AllSignalIntegral);
      }
    }
  }

  // calculate fake rates

  TCut AllFakeCut = FakesFiducialCut * Form("%sMomentum>%f && %sMomentum<%f", FakeVarName.Data(), m_MomLowROC, FakeVarName.Data(),
                                            m_MomHighROC);

  double AllFakeIntegral, SelectedFakeIntegral;
  auto DataFrameFakeAll = RDataFrame(*FakeTree).Filter(AllFakeCut.GetTitle());

  if (FakeWeightName == "1") {
    AllFakeIntegral = DataFrameFakeAll.Count().GetValue();
  } else {
    AllFakeIntegral = DataFrameFakeAll.Sum(FakeWeightName).GetValue();
  }

  std::unique_ptr<ROOT::RDF::RNode> DataFrameSlowFakeAll;

  // special treatment for pions: add also the slow pions from Dstar to gain low-momentum coverage
  if (strncmp(FakeVarName.Data(), "PionD", 5) == 0) {

    TString FakeVarNameSlow = "SlowPion";
    TCut AllFakeCutSlow = TCut(FakesFiducialCut) * TCut(FakesFiducialCutSlow) * Form("(%sMomentum>%f && %sMomentum<%f)",
                          FakeVarNameSlow.Data(), m_MomLowROC, FakeVarNameSlow.Data(), m_MomHighROC);
    DataFrameSlowFakeAll = std::make_unique<ROOT::RDF::RNode>(RDataFrame(*FakeTree).Filter(AllFakeCutSlow.GetTitle()));

    if (FakeWeightName == "1") {
      AllFakeIntegral += DataFrameSlowFakeAll->Count().GetValue();
    } else {
      AllFakeIntegral += DataFrameSlowFakeAll->Sum(FakeWeightName).GetValue();
    }
  }

  for (unsigned int i = 0; i < PIDDetectors.size(); i++) {
    for (unsigned int j = 0; j < m_NumROCpoints; ++j) {
      delete gROOT->FindObject("PIDCut");
      delete gROOT->FindObject("hAllFakes");
      delete gROOT->FindObject("hSelectedFakes");

      // scan cut values from 0 to 1, with a denser scan closer to 0 or 1, to get a nicer ROC curve
      double x = 1. / m_NumROCpoints * j;
      TString PIDCut = TString::Format("%f", 1. / (1 + TMath::Power(x / (1 - x), -3)));

      TCut SelectedFakeCut = Form("(%s%s%s > %s)", FakeVarName.Data(), PIDVarName.Data(), PIDDetectors[i].Data(), PIDCut.Data());

      if (FakeWeightName == "1") {
        SelectedFakeIntegral = DataFrameFakeAll.Filter(SelectedFakeCut.GetTitle()).Count().GetValue();
      } else {
        SelectedFakeIntegral = DataFrameFakeAll.Filter(SelectedFakeCut.GetTitle()).Sum(FakeWeightName).GetValue();
      }

      if (strncmp(FakeVarName.Data(), "PionD", 5) == 0) {
        TString FakeVarNameSlow = "SlowPion";

        TCut SelectedFakeCutSlow = Form("(%s%s%s > %s)", FakeVarNameSlow.Data(), PIDVarName.Data(), PIDDetectors[i].Data(), PIDCut.Data());

        if (FakeWeightName == "1") {
          SelectedFakeIntegral += DataFrameSlowFakeAll->Filter(SelectedFakeCutSlow.GetTitle()).Count().GetValue();
        } else {
          SelectedFakeIntegral += DataFrameSlowFakeAll->Filter(SelectedFakeCutSlow.GetTitle()).Sum(FakeWeightName).GetValue();
        }
      }

      if (PIDDetectors[i] == "ALL") {
        FakeEfficiencyALL.push_back(SelectedFakeIntegral / AllFakeIntegral);
      }

      if (PIDDetectors[i] == "noSVD") {
        FakeEfficiencynoSVD.push_back(SelectedFakeIntegral / AllFakeIntegral);
      }
    }
  }

  auto ResultCanvas = new TCanvas("ResultCanvas", "", 600, 400);
  TMultiGraph* hmgraph = new TMultiGraph();

  // efficiency vs fake rate graph
  TGraph* hgraphALL = new TGraph(m_NumROCpoints, FakeEfficiencyALL.data(), SignalEfficiencyALL.data());
  hgraphALL->SetMarkerColor(TColor::GetColor("#2166ac"));
  hgraphALL->SetMarkerStyle(20);
  hgraphALL->SetLineColor(TColor::GetColor("#2166ac"));
  hgraphALL->SetLineWidth(3);
  hgraphALL->SetDrawOption("AP*");
  hgraphALL->SetTitle("with SVD");

  TGraph* hgraphnoSVD = new TGraph(m_NumROCpoints, FakeEfficiencynoSVD.data(), SignalEfficiencynoSVD.data());
  hgraphnoSVD->SetMarkerColor(TColor::GetColor("#ef8a62"));
  hgraphnoSVD->SetLineColor(TColor::GetColor("#ef8a62"));
  hgraphnoSVD->SetLineWidth(3);
  hgraphnoSVD->SetMarkerStyle(22);
  hgraphnoSVD->SetDrawOption("P*");
  hgraphnoSVD->SetTitle("without SVD");

  hmgraph->Add(hgraphALL);
  hmgraph->Add(hgraphnoSVD);
  hmgraph->Draw("A");
  hmgraph->GetHistogram()->GetXaxis()->SetTitle(FakeVarNameFull + " fake rate");
  hmgraph->GetHistogram()->GetYaxis()->SetTitle(SignalVarNameFull + " signal efficiency");

  ResultCanvas->BuildLegend(0.6, 0.25, 0.9, 0.5);
  ResultCanvas->SetGrid();

  ResultCanvas->Print("SVDdEdxValidation_ROC_curve_" + SignalVarNameFull + "_vs_" + FakeVarNameFull + PIDVarName + "_MomRange" +
                      std::to_string(m_MomLowROC).substr(0, 3) + "_" + std::to_string(m_MomHighROC).substr(0, 3) + ".pdf");

  TFile ResultFile("SVDdEdxValidation_ROC_curve_" + SignalVarNameFull + "_vs_" + FakeVarNameFull + PIDVarName + "_MomRange" +
                   std::to_string(m_MomLowROC).substr(0, 3) + "_" + std::to_string(m_MomHighROC).substr(0, 3) + ".root",
                   "RECREATE");
  hmgraph->Write();
  ResultFile.Close();

  delete ResultCanvas;
}

TTree* SVDdEdxValidationAlgorithm::LambdaMassFit(std::shared_ptr<TTree> preselTree)
{
  B2INFO("Configuring the Lambda fit...");
  gROOT->SetBatch(true);
  RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING);

  RooRealVar InvM("InvM", "m(p^{+}#pi^{-})", 1.1, 1.13, "GeV/c^{2}");

  RooRealVar ProtonMomentum("ProtonMomentum", "momentum for p", -1.e8, 1.e8);
  RooRealVar ProtonSVDdEdx("ProtonSVDdEdx", "", -1.e8, 1.e8);

  RooRealVar exp("exp", "experiment number", 0, 1.e5);
  RooRealVar run("run", "run number", 0, 1.e7);

  RooRealVar ProtonProtonIDALL("ProtonProtonIDALL", "", -1.e8, 1.e8);
  RooRealVar ProtonProtonIDSVDonly("ProtonProtonIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar ProtonProtonIDnoSVD("ProtonProtonIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar ProtonElectronLLSVDonly("ProtonElectronLLSVDonly", "", -1.e8, 1.e8);
  RooRealVar ProtonPionLLSVDonly("ProtonPionLLSVDonly", "", -1.e8, 1.e8);
  RooRealVar ProtonKaonLLSVDonly("ProtonKaonLLSVDonly", "", -1.e8, 1.e8);
  RooRealVar ProtonProtonLLSVDonly("ProtonProtonLLSVDonly", "", -1.e8, 1.e8);

  RooRealVar ProtonBinaryProtonPionIDALL("ProtonBinaryProtonPionIDALL", "", -1.e8, 1.e8);
  RooRealVar ProtonBinaryProtonPionIDSVDonly("ProtonBinaryProtonPionIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar ProtonBinaryProtonPionIDnoSVD("ProtonBinaryProtonPionIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar ProtonBinaryProtonKaonIDALL("ProtonBinaryProtonKaonIDALL", "", -1.e8, 1.e8);
  RooRealVar ProtonBinaryProtonKaonIDSVDonly("ProtonBinaryProtonKaonIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar ProtonBinaryProtonKaonIDnoSVD("ProtonBinaryProtonKaonIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar ProtonBinaryProtonElectronIDALL("ProtonBinaryProtonElectronIDALL", "", -1.e8, 1.e8);
  RooRealVar ProtonBinaryProtonElectronIDSVDonly("ProtonBinaryProtonElectronIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar ProtonBinaryProtonElectronIDnoSVD("ProtonBinaryProtonElectronIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar ProtonBinaryPionProtonIDALL("ProtonBinaryPionProtonIDALL", "", -1.e8, 1.e8);
  RooRealVar ProtonBinaryPionProtonIDSVDonly("ProtonBinaryPionProtonIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar ProtonBinaryPionProtonIDnoSVD("ProtonBinaryPionProtonIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar ProtonBinaryKaonProtonIDALL("ProtonBinaryKaonProtonIDALL", "", -1.e8, 1.e8);
  RooRealVar ProtonBinaryKaonProtonIDSVDonly("ProtonBinaryKaonProtonIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar ProtonBinaryKaonProtonIDnoSVD("ProtonBinaryKaonProtonIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar ProtonBinaryElectronProtonIDALL("ProtonBinaryElectronProtonIDALL", "", -1.e8, 1.e8);
  RooRealVar ProtonBinaryElectronProtonIDSVDonly("ProtonBinaryElectronProtonIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar ProtonBinaryElectronProtonIDnoSVD("ProtonBinaryElectronProtonIDnoSVD", "", -1.e8, 1.e8);

  auto variables = new RooArgSet();

  variables->add(InvM);

  variables->add(ProtonMomentum);
  variables->add(ProtonSVDdEdx);
  variables->add(exp);
  variables->add(run);

  variables->add(ProtonProtonIDALL);
  variables->add(ProtonProtonIDSVDonly);
  variables->add(ProtonProtonIDnoSVD);
  variables->add(ProtonElectronLLSVDonly);
  variables->add(ProtonPionLLSVDonly);
  variables->add(ProtonKaonLLSVDonly);
  variables->add(ProtonProtonLLSVDonly);
  variables->add(ProtonBinaryProtonPionIDALL);
  variables->add(ProtonBinaryProtonPionIDSVDonly);
  variables->add(ProtonBinaryProtonPionIDnoSVD);
  variables->add(ProtonBinaryProtonKaonIDALL);
  variables->add(ProtonBinaryProtonKaonIDSVDonly);
  variables->add(ProtonBinaryProtonKaonIDnoSVD);
  variables->add(ProtonBinaryProtonElectronIDALL);
  variables->add(ProtonBinaryProtonElectronIDSVDonly);
  variables->add(ProtonBinaryProtonElectronIDnoSVD);
  variables->add(ProtonBinaryPionProtonIDALL);
  variables->add(ProtonBinaryPionProtonIDSVDonly);
  variables->add(ProtonBinaryPionProtonIDnoSVD);
  variables->add(ProtonBinaryKaonProtonIDALL);
  variables->add(ProtonBinaryKaonProtonIDSVDonly);
  variables->add(ProtonBinaryKaonProtonIDnoSVD);
  variables->add(ProtonBinaryElectronProtonIDALL);
  variables->add(ProtonBinaryElectronProtonIDSVDonly);
  variables->add(ProtonBinaryElectronProtonIDnoSVD);

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
    canvLambda->Print("SVDdEdxValidationFitLambda.pdf");
    TFile LambdaFitPlotFile("SVDdEdxValidationLambdaFitPlotFile.root", "RECREATE");
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

  B2INFO("Lambda: sPlot done. ");

  return treeLambda_sw;
}

TTree* SVDdEdxValidationAlgorithm::DstarMassFit(std::shared_ptr<TTree> preselTree)
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

  RooRealVar KaonKaonIDALL("KaonKaonIDALL", "", -1.e8, 1.e8);
  RooRealVar KaonKaonIDSVDonly("KaonKaonIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar KaonKaonIDnoSVD("KaonKaonIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar KaonPionIDALL("KaonPionIDALL", "", -1.e8, 1.e8);
  RooRealVar KaonPionIDSVDonly("KaonPionIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar KaonPionIDnoSVD("KaonPionIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar KaonProtonIDALL("KaonProtonIDALL", "", -1.e8, 1.e8);
  RooRealVar KaonProtonIDSVDonly("KaonProtonIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar KaonProtonIDnoSVD("KaonProtonIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar KaonElectronIDALL("KaonElectronIDALL", "", -1.e8, 1.e8);
  RooRealVar KaonElectronIDSVDonly("KaonElectronIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar KaonElectronIDnoSVD("KaonElectronIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar KaonElectronLLSVDonly("KaonElectronLLSVDonly", "", -1.e8, 1.e8);
  RooRealVar KaonPionLLSVDonly("KaonPionLLSVDonly", "", -1.e8, 1.e8);
  RooRealVar KaonKaonLLSVDonly("KaonKaonLLSVDonly", "", -1.e8, 1.e8);
  RooRealVar KaonProtonLLSVDonly("KaonProtonLLSVDonly", "", -1.e8, 1.e8);

  RooRealVar KaonBinaryKaonPionIDALL("KaonBinaryKaonPionIDALL", "", -1.e8, 1.e8);
  RooRealVar KaonBinaryKaonPionIDSVDonly("KaonBinaryKaonPionIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar KaonBinaryKaonPionIDnoSVD("KaonBinaryKaonPionIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar KaonBinaryPionKaonIDALL("KaonBinaryPionKaonIDALL", "", -1.e8, 1.e8);
  RooRealVar KaonBinaryPionKaonIDSVDonly("KaonBinaryPionKaonIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar KaonBinaryPionKaonIDnoSVD("KaonBinaryPionKaonIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar KaonBinaryProtonKaonIDALL("KaonBinaryProtonKaonIDALL", "", -1.e8, 1.e8);
  RooRealVar KaonBinaryProtonKaonIDSVDonly("KaonBinaryProtonKaonIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar KaonBinaryProtonKaonIDnoSVD("KaonBinaryProtonKaonIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar KaonBinaryElectronKaonIDALL("KaonBinaryElectronKaonIDALL", "", -1.e8, 1.e8);
  RooRealVar KaonBinaryElectronKaonIDSVDonly("KaonBinaryElectronKaonIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar KaonBinaryElectronKaonIDnoSVD("KaonBinaryElectronKaonIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar PionDKaonIDALL("PionDKaonIDALL", "", -1.e8, 1.e8);
  RooRealVar PionDKaonIDSVDonly("PionDKaonIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar PionDKaonIDnoSVD("PionDKaonIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar PionDPionIDALL("PionDPionIDALL", "", -1.e8, 1.e8);
  RooRealVar PionDPionIDSVDonly("PionDPionIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar PionDPionIDnoSVD("PionDPionIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar PionDElectronIDALL("PionDElectronIDALL", "", -1.e8, 1.e8);
  RooRealVar PionDElectronIDSVDonly("PionDElectronIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar PionDElectronIDnoSVD("PionDElectronIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar PionDProtonIDALL("PionDProtonIDALL", "", -1.e8, 1.e8);
  RooRealVar PionDProtonIDSVDonly("PionDProtonIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar PionDProtonIDnoSVD("PionDProtonIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar PionDElectronLLSVDonly("PionDElectronLLSVDonly", "", -1.e8, 1.e8);
  RooRealVar PionDPionLLSVDonly("PionDPionLLSVDonly", "", -1.e8, 1.e8);
  RooRealVar PionDKaonLLSVDonly("PionDKaonLLSVDonly", "", -1.e8, 1.e8);
  RooRealVar PionDProtonLLSVDonly("PionDProtonLLSVDonly", "", -1.e8, 1.e8);

  RooRealVar PionDBinaryPionKaonIDALL("PionDBinaryPionKaonIDALL", "", -1.e8, 1.e8);
  RooRealVar PionDBinaryPionKaonIDSVDonly("PionDBinaryPionKaonIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar PionDBinaryPionKaonIDnoSVD("PionDBinaryPionKaonIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar PionDBinaryKaonPionIDALL("PionDBinaryKaonPionIDALL", "", -1.e8, 1.e8);
  RooRealVar PionDBinaryKaonPionIDSVDonly("PionDBinaryKaonPionIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar PionDBinaryKaonPionIDnoSVD("PionDBinaryKaonPionIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar PionDBinaryProtonPionIDALL("PionDBinaryProtonPionIDALL", "", -1.e8, 1.e8);
  RooRealVar PionDBinaryProtonPionIDSVDonly("PionDBinaryProtonPionIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar PionDBinaryProtonPionIDnoSVD("PionDBinaryProtonPionIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar PionDBinaryElectronPionIDALL("PionDBinaryElectronPionIDALL", "", -1.e8, 1.e8);
  RooRealVar PionDBinaryElectronPionIDSVDonly("PionDBinaryElectronPionIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar PionDBinaryElectronPionIDnoSVD("PionDBinaryElectronPionIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar SlowPionKaonIDALL("SlowPionKaonIDALL", "", -1.e8, 1.e8);
  RooRealVar SlowPionKaonIDSVDonly("SlowPionKaonIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar SlowPionKaonIDnoSVD("SlowPionKaonIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar SlowPionPionIDALL("SlowPionPionIDALL", "", -1.e8, 1.e8);
  RooRealVar SlowPionPionIDSVDonly("SlowPionPionIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar SlowPionPionIDnoSVD("SlowPionPionIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar SlowPionElectronIDALL("SlowPionElectronIDALL", "", -1.e8, 1.e8);
  RooRealVar SlowPionElectronIDSVDonly("SlowPionElectronIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar SlowPionElectronIDnoSVD("SlowPionElectronIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar SlowPionProtonIDALL("SlowPionProtonIDALL", "", -1.e8, 1.e8);
  RooRealVar SlowPionProtonIDSVDonly("SlowPionProtonIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar SlowPionProtonIDnoSVD("SlowPionProtonIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar SlowPionElectronLLSVDonly("SlowPionElectronLLSVDonly", "", -1.e8, 1.e8);
  RooRealVar SlowPionPionLLSVDonly("SlowPionPionLLSVDonly", "", -1.e8, 1.e8);
  RooRealVar SlowPionKaonLLSVDonly("SlowPionKaonLLSVDonly", "", -1.e8, 1.e8);
  RooRealVar SlowPionProtonLLSVDonly("SlowPionProtonLLSVDonly", "", -1.e8, 1.e8);

  RooRealVar SlowPionBinaryPionKaonIDALL("SlowPionBinaryPionKaonIDALL", "", -1.e8, 1.e8);
  RooRealVar SlowPionBinaryPionKaonIDSVDonly("SlowPionBinaryPionKaonIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar SlowPionBinaryPionKaonIDnoSVD("SlowPionBinaryPionKaonIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar SlowPionBinaryKaonPionIDALL("SlowPionBinaryKaonPionIDALL", "", -1.e8, 1.e8);
  RooRealVar SlowPionBinaryKaonPionIDSVDonly("SlowPionBinaryKaonPionIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar SlowPionBinaryKaonPionIDnoSVD("SlowPionBinaryKaonPionIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar SlowPionBinaryProtonPionIDALL("SlowPionBinaryProtonPionIDALL", "", -1.e8, 1.e8);
  RooRealVar SlowPionBinaryProtonPionIDSVDonly("SlowPionBinaryProtonPionIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar SlowPionBinaryProtonPionIDnoSVD("SlowPionBinaryProtonPionIDnoSVD", "", -1.e8, 1.e8);

  RooRealVar SlowPionBinaryElectronPionIDALL("SlowPionBinaryElectronPionIDALL", "", -1.e8, 1.e8);
  RooRealVar SlowPionBinaryElectronPionIDSVDonly("SlowPionBinaryElectronPionIDSVDonly", "", -1.e8, 1.e8);
  RooRealVar SlowPionBinaryElectronPionIDnoSVD("SlowPionBinaryElectronPionIDnoSVD", "", -1.e8, 1.e8);

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

  variables->add(KaonKaonIDALL);
  variables->add(KaonKaonIDSVDonly);
  variables->add(KaonKaonIDnoSVD);
  variables->add(KaonPionIDALL);
  variables->add(KaonPionIDSVDonly);
  variables->add(KaonPionIDnoSVD);
  variables->add(KaonProtonIDALL);
  variables->add(KaonProtonIDSVDonly);
  variables->add(KaonProtonIDnoSVD);
  variables->add(KaonElectronIDALL);
  variables->add(KaonElectronIDSVDonly);
  variables->add(KaonElectronIDnoSVD);

  variables->add(KaonElectronLLSVDonly);
  variables->add(KaonPionLLSVDonly);
  variables->add(KaonKaonLLSVDonly);
  variables->add(KaonProtonLLSVDonly);

  variables->add(KaonBinaryKaonPionIDALL);
  variables->add(KaonBinaryKaonPionIDSVDonly);
  variables->add(KaonBinaryKaonPionIDnoSVD);
  variables->add(KaonBinaryPionKaonIDALL);
  variables->add(KaonBinaryPionKaonIDSVDonly);
  variables->add(KaonBinaryPionKaonIDnoSVD);
  variables->add(KaonBinaryProtonKaonIDALL);
  variables->add(KaonBinaryProtonKaonIDSVDonly);
  variables->add(KaonBinaryProtonKaonIDnoSVD);
  variables->add(KaonBinaryElectronKaonIDALL);
  variables->add(KaonBinaryElectronKaonIDSVDonly);
  variables->add(KaonBinaryElectronKaonIDnoSVD);

  variables->add(PionDPionIDALL);
  variables->add(PionDPionIDSVDonly);
  variables->add(PionDPionIDnoSVD);
  variables->add(PionDKaonIDALL);
  variables->add(PionDKaonIDSVDonly);
  variables->add(PionDKaonIDnoSVD);
  variables->add(PionDElectronIDALL);
  variables->add(PionDElectronIDSVDonly);
  variables->add(PionDElectronIDnoSVD);
  variables->add(PionDProtonIDALL);
  variables->add(PionDProtonIDSVDonly);
  variables->add(PionDProtonIDnoSVD);

  variables->add(PionDElectronLLSVDonly);
  variables->add(PionDPionLLSVDonly);
  variables->add(PionDKaonLLSVDonly);
  variables->add(PionDProtonLLSVDonly);

  variables->add(PionDBinaryPionKaonIDALL);
  variables->add(PionDBinaryPionKaonIDSVDonly);
  variables->add(PionDBinaryPionKaonIDnoSVD);
  variables->add(PionDBinaryKaonPionIDALL);
  variables->add(PionDBinaryKaonPionIDSVDonly);
  variables->add(PionDBinaryKaonPionIDnoSVD);
  variables->add(PionDBinaryProtonPionIDALL);
  variables->add(PionDBinaryProtonPionIDSVDonly);
  variables->add(PionDBinaryProtonPionIDnoSVD);
  variables->add(PionDBinaryElectronPionIDALL);
  variables->add(PionDBinaryElectronPionIDSVDonly);
  variables->add(PionDBinaryElectronPionIDnoSVD);

  variables->add(SlowPionPionIDALL);
  variables->add(SlowPionPionIDSVDonly);
  variables->add(SlowPionPionIDnoSVD);
  variables->add(SlowPionKaonIDALL);
  variables->add(SlowPionKaonIDSVDonly);
  variables->add(SlowPionKaonIDnoSVD);
  variables->add(SlowPionElectronIDALL);
  variables->add(SlowPionElectronIDSVDonly);
  variables->add(SlowPionElectronIDnoSVD);
  variables->add(SlowPionProtonIDALL);
  variables->add(SlowPionProtonIDSVDonly);
  variables->add(SlowPionProtonIDnoSVD);

  variables->add(SlowPionElectronLLSVDonly);
  variables->add(SlowPionPionLLSVDonly);
  variables->add(SlowPionKaonLLSVDonly);
  variables->add(SlowPionProtonLLSVDonly);

  variables->add(SlowPionBinaryPionKaonIDALL);
  variables->add(SlowPionBinaryPionKaonIDSVDonly);
  variables->add(SlowPionBinaryPionKaonIDnoSVD);
  variables->add(SlowPionBinaryKaonPionIDALL);
  variables->add(SlowPionBinaryKaonPionIDSVDonly);
  variables->add(SlowPionBinaryKaonPionIDnoSVD);
  variables->add(SlowPionBinaryProtonPionIDALL);
  variables->add(SlowPionBinaryProtonPionIDSVDonly);
  variables->add(SlowPionBinaryProtonPionIDnoSVD);
  variables->add(SlowPionBinaryElectronPionIDALL);
  variables->add(SlowPionBinaryElectronPionIDSVDonly);
  variables->add(SlowPionBinaryElectronPionIDnoSVD);

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
    canvDstar->Print("SVDdEdxValidationFitDstar.pdf");
    TFile DstarFitPlotFile("SVDdEdxValidationDstarFitPlotFile.root", "RECREATE");
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

  B2INFO("Dstar: sPlot done. ");

  return treeDstar_sw;
}
