/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/calibration/SVDdEdxValidationAlgorithm.h>
// #include <svd/dbobjects/SVDdEdxPDFs.h>
#include <tuple>
#include <vector>
#include <string>

#include <TROOT.h>
#include <TStyle.h>
#include <TMath.h>
#include <TFile.h>
#include <TTree.h>
#include <TColor.h>
#include <TLegend.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TAxis.h>
#include <TGraph.h>
#include <TMultiGraph.h>

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
  // TH2F h_GammaE = GammaHistogram(TTreeGamma);
  TTree* TTreeGammaWrap = TTreeGamma.get();

  std::vector<TString> LAYER;
  LAYER.push_back("ALL");
  LAYER.push_back("SVDonly");
  LAYER.push_back("noSVD");


  for (const TString& LayerName : LAYER) {
    PlotEfficiencyPlots(LayerName, TTreeGammaWrap, "1", "e_1", "electron", TTreeDstarSW, "nSignalDstar_sw", "pi", "pion",
                        "electron_pionID",
                        "0.5", 30, 0.2, 2.5);
    PlotEfficiencyPlots(LayerName, TTreeGammaWrap, "1", "e_1", "electron", TTreeDstarSW, "nSignalDstar_sw", "K", "kaon",
                        "electron_kaonID", "0.5",
                        30, 0.2, 2.5);
    PlotEfficiencyPlots(LayerName, TTreeLambdaSW, "nSignalLambda_sw", "p", "proton", TTreeDstarSW, "nSignalDstar_sw", "pi", "pion",
                        "proton_pionID", "0.5",
                        30, 0.25, 2.5);
    PlotEfficiencyPlots(LayerName, TTreeLambdaSW, "nSignalLambda_sw", "p", "proton", TTreeDstarSW, "nSignalDstar_sw", "K", "kaon",
                        "proton_kaonID", "0.5",
                        30, 0.25, 2.5);
    PlotEfficiencyPlots(LayerName, TTreeDstarSW, "nSignalDstar_sw", "pi", "pion", TTreeDstarSW, "nSignalDstar_sw", "K", "kaon",
                        "pion_kaonID", "0.5", 30,
                        0.2, 2.5);
    PlotEfficiencyPlots(LayerName, TTreeDstarSW, "nSignalDstar_sw", "K", "kaon", TTreeDstarSW, "nSignalDstar_sw", "pi", "pion",
                        "kaon_pionID", "0.5", 30,
                        0.2, 2.5);
  }

  PlotROCCurve(TTreeGammaWrap, "1", "e_1", "electron", TTreeDstarSW, "nSignalDstar_sw", "pi", "pion", "electron_pionID", 0., 7.);
  PlotROCCurve(TTreeGammaWrap, "1", "e_1", "electron", TTreeDstarSW, "nSignalDstar_sw", "K", "kaon", "electron_kaonID", 0., 7.);
  PlotROCCurve(TTreeLambdaSW, "nSignalLambda_sw", "p", "proton", TTreeDstarSW, "nSignalDstar_sw", "pi", "pion", "proton_pionID", 0.,
               7.);
  PlotROCCurve(TTreeLambdaSW, "nSignalLambda_sw", "p", "proton", TTreeDstarSW, "nSignalDstar_sw", "K", "kaon", "proton_kaonID", 0.,
               7.);
  PlotROCCurve(TTreeDstarSW, "nSignalDstar_sw", "pi", "pion", TTreeDstarSW, "nSignalDstar_sw", "K", "kaon", "pion_kaonID", 0., 7.);
  PlotROCCurve(TTreeDstarSW, "nSignalDstar_sw", "K", "kaon", TTreeDstarSW, "nSignalDstar_sw", "pi", "pion", "kaon_pionID", 0., 7.);

  // saveCalibration(payload, "SVDdEdxPDFs"); //"DedxPDFs");

  B2INFO("SVD dE/dx validation done!");

  // fout_read->Close();
  return c_OK;
}

// generic efficiency and fake rate
void SVDdEdxValidationAlgorithm::PlotEfficiencyPlots(const TString& LayerName, TTree* SignalTree, TString SignalWeightName,
                                                     TString SignalVarName, TString SignalVarNameFull, TTree* FakeTree, TString FakeWeightName, TString FakeVarName,
                                                     TString FakeVarNameFull, TString PIDVarName, TString PIDCut, unsigned int nbins, double MomLow, double MomHigh)
{

  TString SignalFiducialCut = "(1>0)"; // placeholder for a possible sanity cut
  TString FakesFiducialCut = "(1>0)";

  // Data
  // TFile *SignalFile = new TFile(SignalFileName);
  // TTree *SignalTree = (TTree *)SignalFile->Get(SignalTreeName);
  // TFile *FakeFile = new TFile(FakeFileName);
  // TTree *FakeTree = (TTree *)FakeFile->Get(FakeTreeName);

  // Produce the plots of the SVD PID distribution
  if (LayerName == "SVDonly") {
    SignalTree->Draw(Form("%s_%s_%s>>h_SignalPIDDistribution(100,0.,1.)", SignalVarName.Data(), PIDVarName.Data(), LayerName.Data()),
                     SignalWeightName + Form("* (%s_p>%f && %s_p<%f)", SignalVarName.Data(), MomLow, SignalVarName.Data(), MomHigh), "goff");
    TH1F* h_SignalPIDDistribution = (TH1F*)gDirectory->Get("h_SignalPIDDistribution");
    h_SignalPIDDistribution->Scale(1. / h_SignalPIDDistribution->Integral());
    h_SignalPIDDistribution->GetXaxis()->SetTitle(PIDVarName + "_" + LayerName + " for " + SignalVarNameFull);
    h_SignalPIDDistribution->GetYaxis()->SetTitle("Candidates, normalised");
    h_SignalPIDDistribution->SetMaximum(1.35 * h_SignalPIDDistribution->GetMaximum());

    TCanvas* DistribCanvas = new TCanvas("DistribCanvas", "", 600, 600);
    gPad->SetTopMargin(0.05);
    gPad->SetRightMargin(0.05);
    gPad->SetLeftMargin(0.13);
    gPad->SetBottomMargin(0.12);

    h_SignalPIDDistribution->SetLineWidth(2);
    h_SignalPIDDistribution->SetLineColor(TColor::GetColor("#2166ac"));
    h_SignalPIDDistribution->Draw("hist ");

    DistribCanvas->Print("Distribution_" + SignalVarNameFull + "_" + PIDVarName + "_" + LayerName + "_MomRange_" + std::to_string(
                           MomLow).substr(0, 3) + "_" + std::to_string(MomHigh).substr(0, 3) + ".pdf");

    delete DistribCanvas;
  }

  // ---------- Momentum distributions (for efficiency determination) ----------

  SignalTree->Draw(Form("%s_p>>h_AllSignal(%i,%f,%f)", SignalVarName.Data(), nbins, MomLow, MomHigh),
                   SignalWeightName + " * (" + SignalFiducialCut + ")", "goff");
  SignalTree->Draw(Form("%s_p>>h_SelectedSignal(%i,%f,%f)", SignalVarName.Data(), nbins, MomLow, MomHigh),
                   SignalWeightName + " * (" + SignalVarName + "_" + PIDVarName + "_" + LayerName + ">" + PIDCut + "&&" + SignalFiducialCut + ")",
                   "goff");

  FakeTree->Draw(Form("%s_p>>h_AllFakes(%i,%f,%f)", FakeVarName.Data(), nbins, MomLow, MomHigh),
                 FakeWeightName + " * (" + FakesFiducialCut + ")", "goff");
  FakeTree->Draw(Form("%s_p>>h_SelectedFakes(%i,%f,%f)", FakeVarName.Data(), nbins, MomLow, MomHigh),
                 FakeWeightName + " * (" + FakeVarName + "_" + PIDVarName + "_" + LayerName + ">" + PIDCut + "&&" + FakesFiducialCut + ")", "goff");

  TH1F* h_AllSignal = (TH1F*)gDirectory->Get("h_AllSignal");
  TH1F* h_SelectedSignal = (TH1F*)gDirectory->Get("h_SelectedSignal");
  TH1F* h_AllFakes = (TH1F*)gDirectory->Get("h_AllFakes");
  TH1F* h_SelectedFakes = (TH1F*)gDirectory->Get("h_SelectedFakes");

  TH1F* EffHistoSig = (TH1F*)h_AllSignal->Clone("EffHistoSig");   // signal efficiency
  TH1F* EffHistoFake = (TH1F*)h_AllFakes->Clone("EffHistoFake");  // fakes efficiency

  EffHistoSig->Divide(h_SelectedSignal, h_AllSignal, 1, 1, "B");
  EffHistoFake->Divide(h_SelectedFakes, h_AllFakes, 1, 1, "B");

  // PID plots
  TH1F* h_Base = new TH1F("h_Base", "", 100, 0.0, MomHigh);
  h_Base->SetTitle(";Momentum [GeV];Efficiency"); //, SignalVarNameFull.Data(), FakeVarNameFull.Data()));
  h_Base->SetMaximum(1.20);
  h_Base->SetMinimum(0.0);

  TLegend* tleg1 = new TLegend(0.63, 0.82, 0.93, 0.94);
  tleg1->AddEntry(EffHistoSig, SignalVarNameFull + " efficiency", "pl");
  tleg1->AddEntry(EffHistoFake, FakeVarNameFull + " fake rate", "pl");

  TCanvas* ResultCanvas = new TCanvas("ResultCanvas", "", 600, 600);
  gPad->SetTopMargin(0.05);
  gPad->SetRightMargin(0.05);
  gPad->SetLeftMargin(0.13);
  gPad->SetBottomMargin(0.12);

  ResultCanvas->SetGrid();
  h_Base->Draw();
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

  h_Base->SetStats(0);
  h_Base->GetXaxis()->SetTitleSize(0.04);
  h_Base->GetYaxis()->SetTitleSize(0.04);
  h_Base->GetXaxis()->SetTitleOffset(1.0);
  h_Base->GetYaxis()->SetTitleOffset(1.3);
  h_Base->GetYaxis()->SetLabelSize(0.04);
  h_Base->GetXaxis()->SetLabelSize(0.04);

  // std::setprecision(2);
  ResultCanvas->Print("Efficiency_" + SignalVarNameFull + "_vs_" + FakeVarNameFull + "_" + PIDVarName + "_" + LayerName + "_Cut" +
                      PIDCut + "_MomRange_" + std::to_string(MomLow).substr(0, 3) + "_" + std::to_string(MomHigh).substr(0, 3) + ".pdf");

  delete ResultCanvas;
  delete h_Base;

  // SignalFile->Close();
  // FakeFile->Close();
}

void SVDdEdxValidationAlgorithm::PlotROCCurve(TTree* SignalTree, TString SignalWeightName, TString SignalVarName,
                                              TString SignalVarNameFull, TTree* FakeTree, TString FakeWeightName, TString FakeVarName, TString FakeVarNameFull,
                                              TString PIDVarName, double MomLow, double MomHigh)
{
  const unsigned int npoints = 250; // the more the nicer curve but slower execution

  std::vector<TString> LAYER;
  LAYER.clear();
  LAYER.push_back("_ALL");
  LAYER.push_back("_noSVD");

  // TFile *SignalFile = TFile::Open(SignalFileName, "r");
  // TTree *SignalTree = (TTree *)SignalFile->Get(SignalTreeName);

  double SignalEfficiency_ALL[npoints], FakeEfficiency_ALL[npoints];
  double SignalEfficiency_noSVD[npoints], FakeEfficiency_noSVD[npoints];

  TString SignalFiducialCut = SignalVarName + "_" + PIDVarName + "_noSVD>=0"; // sanity cuts to reject events with NaN
  TString FakesFiducialCut = FakeVarName + "_" + PIDVarName + "_noSVD>=0";

  // calculate efficiencies
  for (unsigned int i = 0; i < LAYER.size(); i++) {
    for (unsigned int j = 0; j < npoints; ++j) {
      delete gROOT->FindObject("PIDCut");
      delete gROOT->FindObject("h_AllSignal");
      delete gROOT->FindObject("h_SelectedSignal");

      double x = 1. / npoints * j;
      TString PIDCut = TString::Format("%f", 1. / (1 + TMath::Power(x / (1 - x), -3)));

      // TString PIDCut = TString::Format("%f", 0. + 1. / npoints * j);

      SignalTree->Draw(Form("%s_p>>h_AllSignal(1,%f,%f)", SignalVarName.Data(), MomLow, MomHigh),
                       SignalWeightName + " * (" + SignalFiducialCut + ")", "goff");
      SignalTree->Draw(Form("%s_p>>h_SelectedSignal(1,%f,%f)", SignalVarName.Data(), MomLow, MomHigh),
                       SignalWeightName + " * (" + SignalVarName + "_" + PIDVarName + LAYER[i] + ">" + PIDCut + "&&" + SignalFiducialCut + ")", "goff");

      TH1F* h_AllSignal = (TH1F*)gDirectory->Get("h_AllSignal");
      TH1F* h_SelectedSignal = (TH1F*)gDirectory->Get("h_SelectedSignal");

      if (LAYER[i] == "_ALL") {
        SignalEfficiency_ALL[j] = h_SelectedSignal->Integral() / h_AllSignal->Integral();
      }

      if (LAYER[i] == "_noSVD") {
        SignalEfficiency_noSVD[j] = h_SelectedSignal->Integral() / h_AllSignal->Integral();
      }
    }
  }

  // SignalFile->Close();

  // calculate fake rates
  // TFile *FakeFile = TFile::Open(FakeFileName, "r");
  // TTree *FakeTree = (TTree *)FakeFile->Get(FakeTreeName);

  for (unsigned int i = 0; i < LAYER.size(); i++) {
    for (unsigned int j = 0; j < npoints; ++j) {
      delete gROOT->FindObject("PIDCut");
      delete gROOT->FindObject("h_AllFakes");
      delete gROOT->FindObject("h_SelectedFakes");
      delete gROOT->FindObject("h_AllFakes");

      double x = 1. / npoints * j;
      TString PIDCut = TString::Format("%f", 1. / (1 + TMath::Power(x / (1 - x), -3)));

      // TString PIDCut = TString::Format("%f", 0. + 1. / npoints * j);

      FakeTree->Draw(Form("%s_p>>h_AllFakes(1,%f,%f)", FakeVarName.Data(), MomLow, MomHigh),
                     FakeWeightName + " * (" + FakesFiducialCut + ")", "goff");
      FakeTree->Draw(Form("%s_p>>h_SelectedFakes(1,%f,%f)", FakeVarName.Data(), MomLow, MomHigh),
                     FakeWeightName + " * (" + FakeVarName + "_" + PIDVarName + LAYER[i] + ">" + PIDCut + "&&" + FakesFiducialCut + ")", "goff");

      TH1F* h_SelectedFakes = (TH1F*)gDirectory->Get("h_SelectedFakes");
      TH1F* h_AllFakes = (TH1F*)gDirectory->Get("h_AllFakes");

      if (LAYER[i] == "_ALL") {
        FakeEfficiency_ALL[j] = h_SelectedFakes->Integral() / h_AllFakes->Integral();
      }

      if (LAYER[i] == "_noSVD") {
        FakeEfficiency_noSVD[j] = h_SelectedFakes->Integral() / h_AllFakes->Integral();
      }
    }
  }

  // FakeFile->Close();

  auto ResultCanvas = new TCanvas("ResultCanvas", "", 600, 400);
  TMultiGraph* h_mgraph = new TMultiGraph();

  // efficiency and kaon fake rate
  TGraph* h_graph_ALL = new TGraph(npoints, FakeEfficiency_ALL, SignalEfficiency_ALL);
  h_graph_ALL->SetMarkerColor(TColor::GetColor("#2166ac"));
  h_graph_ALL->SetMarkerStyle(20);
  h_graph_ALL->SetLineColor(TColor::GetColor("#2166ac"));
  h_graph_ALL->SetLineWidth(3);
  h_graph_ALL->SetDrawOption("AP*");
  h_graph_ALL->SetTitle("with SVD");

  TGraph* h_graph_noSVD = new TGraph(npoints, FakeEfficiency_noSVD, SignalEfficiency_noSVD);
  h_graph_noSVD->SetMarkerColor(TColor::GetColor("#ef8a62"));
  h_graph_noSVD->SetLineColor(TColor::GetColor("#ef8a62"));
  h_graph_noSVD->SetLineWidth(3);
  h_graph_noSVD->SetMarkerStyle(22);
  h_graph_noSVD->SetDrawOption("P*");
  h_graph_noSVD->SetTitle("without SVD");

  h_mgraph->Add(h_graph_ALL);
  h_mgraph->Add(h_graph_noSVD);
  h_mgraph->Draw("A");
  h_mgraph->GetHistogram()->GetXaxis()->SetTitle(FakeVarNameFull + " fake rate");
  h_mgraph->GetHistogram()->GetYaxis()->SetTitle(SignalVarNameFull + " signal efficiency");

  ResultCanvas->BuildLegend(0.6, 0.25, 0.9, 0.5);
  ResultCanvas->SetGrid();

  ResultCanvas->Print("ROC_curve_" + SignalVarNameFull + "_vs_" + FakeVarNameFull + "_" + PIDVarName + "_MomRange" + std::to_string(
                        MomLow).substr(0, 3) + "_" + std::to_string(MomHigh).substr(0, 3) + ".pdf");

  delete ResultCanvas;
}


TTree* SVDdEdxValidationAlgorithm::LambdaMassFit(std::shared_ptr<TTree> preselTree)
{
  B2INFO("Configuring the Lambda fit...");
  gROOT->SetBatch(true);
  RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING);

  RooRealVar InvM("InvM", "m(p^{+}#pi^{-})", 1.1, 1.13, "GeV/c^{2}");

  RooRealVar p_p("p_p", "momentum for p", -1.e8, 1.e8);
  RooRealVar p_SVDdEdx("p_SVDdEdx", "", -1.e8, 1.e8);

  RooRealVar exp("exp", "experiment number", 0, 1.e5);
  RooRealVar run("run", "run number", 0, 1.e7);

  RooRealVar p_protonID_ALL("p_protonID_ALL", "", -1.e8, 1.e8);
  RooRealVar p_protonID_SVDonly("p_protonID_SVDonly", "", -1.e8, 1.e8);
  RooRealVar p_protonID_noSVD("p_protonID_noSVD", "", -1.e8, 1.e8);

  RooRealVar p_proton_pionID_ALL("p_proton_pionID_ALL", "", -1.e8, 1.e8);
  RooRealVar p_proton_pionID_SVDonly("p_proton_pionID_SVDonly", "", -1.e8, 1.e8);
  RooRealVar p_proton_pionID_noSVD("p_proton_pionID_noSVD", "", -1.e8, 1.e8);

  RooRealVar p_proton_kaonID_ALL("p_proton_kaonID_ALL", "", -1.e8, 1.e8);
  RooRealVar p_proton_kaonID_SVDonly("p_proton_kaonID_SVDonly", "", -1.e8, 1.e8);
  RooRealVar p_proton_kaonID_noSVD("p_proton_kaonID_noSVD", "", -1.e8, 1.e8);

  RooRealVar p_proton_electronID_ALL("p_proton_electronID_ALL", "", -1.e8, 1.e8);
  RooRealVar p_proton_electronID_SVDonly("p_proton_electronID_SVDonly", "", -1.e8, 1.e8);
  RooRealVar p_proton_electronID_noSVD("p_proton_electronID_noSVD", "", -1.e8, 1.e8);

  RooRealVar p_pion_protonID_ALL("p_pion_protonID_ALL", "", -1.e8, 1.e8);
  RooRealVar p_pion_protonID_SVDonly("p_pion_protonID_SVDonly", "", -1.e8, 1.e8);
  RooRealVar p_pion_protonID_noSVD("p_pion_protonID_noSVD", "", -1.e8, 1.e8);

  RooRealVar p_kaon_protonID_ALL("p_kaon_protonID_ALL", "", -1.e8, 1.e8);
  RooRealVar p_kaon_protonID_SVDonly("p_kaon_protonID_SVDonly", "", -1.e8, 1.e8);
  RooRealVar p_kaon_protonID_noSVD("p_kaon_protonID_noSVD", "", -1.e8, 1.e8);

  RooRealVar p_electron_protonID_ALL("p_electron_protonID_ALL", "", -1.e8, 1.e8);
  RooRealVar p_electron_protonID_SVDonly("p_electron_protonID_SVDonly", "", -1.e8, 1.e8);
  RooRealVar p_electron_protonID_noSVD("p_electron_protonID_noSVD", "", -1.e8, 1.e8);

  auto variables = new RooArgSet();

  variables->add(InvM);

  variables->add(p_p);
  variables->add(p_SVDdEdx);
  variables->add(exp);
  variables->add(run);

  variables->add(p_protonID_ALL);
  variables->add(p_protonID_SVDonly);
  variables->add(p_protonID_noSVD);
  variables->add(p_proton_pionID_ALL);
  variables->add(p_proton_pionID_SVDonly);
  variables->add(p_proton_pionID_noSVD);
  variables->add(p_proton_kaonID_ALL);
  variables->add(p_proton_kaonID_SVDonly);
  variables->add(p_proton_kaonID_noSVD);
  variables->add(p_proton_electronID_ALL);
  variables->add(p_proton_electronID_SVDonly);
  variables->add(p_proton_electronID_noSVD);
  variables->add(p_pion_protonID_ALL);
  variables->add(p_pion_protonID_SVDonly);
  variables->add(p_pion_protonID_noSVD);
  variables->add(p_kaon_protonID_ALL);
  variables->add(p_kaon_protonID_SVDonly);
  variables->add(p_kaon_protonID_noSVD);
  variables->add(p_electron_protonID_ALL);
  variables->add(p_electron_protonID_SVDonly);
  variables->add(p_electron_protonID_noSVD);

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
    canvLambda->Print("SVDdEdxValidationFitLambda.pdf");
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

  B2INFO("Lambda: sPlot done. ");

  return treeLambda_sw;
}

TTree* SVDdEdxValidationAlgorithm::DstarMassFit(std::shared_ptr<TTree> preselTree)
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

  RooRealVar K_kaonID_ALL("K_kaonID_ALL", "", -1.e8, 1.e8);
  RooRealVar K_kaonID_SVDonly("K_kaonID_SVDonly", "", -1.e8, 1.e8);
  RooRealVar K_kaonID_noSVD("K_kaonID_noSVD", "", -1.e8, 1.e8);

  RooRealVar K_pionID_ALL("K_pionID_ALL", "", -1.e8, 1.e8);
  RooRealVar K_pionID_SVDonly("K_pionID_SVDonly", "", -1.e8, 1.e8);
  RooRealVar K_pionID_noSVD("K_pionID_noSVD", "", -1.e8, 1.e8);

  RooRealVar K_protonID_ALL("K_protonID_ALL", "", -1.e8, 1.e8);
  RooRealVar K_protonID_SVDonly("K_protonID_SVDonly", "", -1.e8, 1.e8);
  RooRealVar K_protonID_noSVD("K_protonID_noSVD", "", -1.e8, 1.e8);

  RooRealVar K_electronID_ALL("K_electronID_ALL", "", -1.e8, 1.e8);
  RooRealVar K_electronID_SVDonly("K_electronID_SVDonly", "", -1.e8, 1.e8);
  RooRealVar K_electronID_noSVD("K_electronID_noSVD", "", -1.e8, 1.e8);

  RooRealVar K_kaon_pionID_ALL("K_kaon_pionID_ALL", "", -1.e8, 1.e8);
  RooRealVar K_kaon_pionID_SVDonly("K_kaon_pionID_SVDonly", "", -1.e8, 1.e8);
  RooRealVar K_kaon_pionID_noSVD("K_kaon_pionID_noSVD", "", -1.e8, 1.e8);

  RooRealVar K_pion_kaonID_ALL("K_pion_kaonID_ALL", "", -1.e8, 1.e8);
  RooRealVar K_pion_kaonID_SVDonly("K_pion_kaonID_SVDonly", "", -1.e8, 1.e8);
  RooRealVar K_pion_kaonID_noSVD("K_pion_kaonID_noSVD", "", -1.e8, 1.e8);

  RooRealVar K_proton_kaonID_ALL("K_proton_kaonID_ALL", "", -1.e8, 1.e8);
  RooRealVar K_proton_kaonID_SVDonly("K_proton_kaonID_SVDonly", "", -1.e8, 1.e8);
  RooRealVar K_proton_kaonID_noSVD("K_proton_kaonID_noSVD", "", -1.e8, 1.e8);

  RooRealVar K_electron_kaonID_ALL("K_electron_kaonID_ALL", "", -1.e8, 1.e8);
  RooRealVar K_electron_kaonID_SVDonly("K_electron_kaonID_SVDonly", "", -1.e8, 1.e8);
  RooRealVar K_electron_kaonID_noSVD("K_electron_kaonID_noSVD", "", -1.e8, 1.e8);

  RooRealVar pi_kaonID_ALL("pi_kaonID_ALL", "", -1.e8, 1.e8);
  RooRealVar pi_kaonID_SVDonly("pi_kaonID_SVDonly", "", -1.e8, 1.e8);
  RooRealVar pi_kaonID_noSVD("pi_kaonID_noSVD", "", -1.e8, 1.e8);

  RooRealVar pi_pionID_ALL("pi_pionID_ALL", "", -1.e8, 1.e8);
  RooRealVar pi_pionID_SVDonly("pi_pionID_SVDonly", "", -1.e8, 1.e8);
  RooRealVar pi_pionID_noSVD("pi_pionID_noSVD", "", -1.e8, 1.e8);

  RooRealVar pi_electronID_ALL("pi_electronID_ALL", "", -1.e8, 1.e8);
  RooRealVar pi_electronID_SVDonly("pi_electronID_SVDonly", "", -1.e8, 1.e8);
  RooRealVar pi_electronID_noSVD("pi_electronID_noSVD", "", -1.e8, 1.e8);

  RooRealVar pi_protonID_ALL("pi_protonID_ALL", "", -1.e8, 1.e8);
  RooRealVar pi_protonID_SVDonly("pi_protonID_SVDonly", "", -1.e8, 1.e8);
  RooRealVar pi_protonID_noSVD("pi_protonID_noSVD", "", -1.e8, 1.e8);

  RooRealVar pi_pion_kaonID_ALL("pi_pion_kaonID_ALL", "", -1.e8, 1.e8);
  RooRealVar pi_pion_kaonID_SVDonly("pi_pion_kaonID_SVDonly", "", -1.e8, 1.e8);
  RooRealVar pi_pion_kaonID_noSVD("pi_pion_kaonID_noSVD", "", -1.e8, 1.e8);

  RooRealVar pi_kaon_pionID_ALL("pi_kaon_pionID_ALL", "", -1.e8, 1.e8);
  RooRealVar pi_kaon_pionID_SVDonly("pi_kaon_pionID_SVDonly", "", -1.e8, 1.e8);
  RooRealVar pi_kaon_pionID_noSVD("pi_kaon_pionID_noSVD", "", -1.e8, 1.e8);

  RooRealVar pi_proton_pionID_ALL("pi_proton_pionID_ALL", "", -1.e8, 1.e8);
  RooRealVar pi_proton_pionID_SVDonly("pi_proton_pionID_SVDonly", "", -1.e8, 1.e8);
  RooRealVar pi_proton_pionID_noSVD("pi_proton_pionID_noSVD", "", -1.e8, 1.e8);

  RooRealVar pi_electron_pionID_ALL("pi_electron_pionID_ALL", "", -1.e8, 1.e8);
  RooRealVar pi_electron_pionID_SVDonly("pi_electron_pionID_SVDonly", "", -1.e8, 1.e8);
  RooRealVar pi_electron_pionID_noSVD("pi_electron_pionID_noSVD", "", -1.e8, 1.e8);

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

  variables->add(K_kaonID_ALL);
  variables->add(K_kaonID_SVDonly);
  variables->add(K_kaonID_noSVD);
  variables->add(K_pionID_ALL);
  variables->add(K_pionID_SVDonly);
  variables->add(K_pionID_noSVD);
  variables->add(K_protonID_ALL);
  variables->add(K_protonID_SVDonly);
  variables->add(K_protonID_noSVD);
  variables->add(K_electronID_ALL);
  variables->add(K_electronID_SVDonly);
  variables->add(K_electronID_noSVD);
  variables->add(K_kaon_pionID_ALL);
  variables->add(K_kaon_pionID_SVDonly);
  variables->add(K_kaon_pionID_noSVD);
  variables->add(K_pion_kaonID_ALL);
  variables->add(K_pion_kaonID_SVDonly);
  variables->add(K_pion_kaonID_noSVD);
  variables->add(K_proton_kaonID_ALL);
  variables->add(K_proton_kaonID_SVDonly);
  variables->add(K_proton_kaonID_noSVD);
  variables->add(K_electron_kaonID_ALL);
  variables->add(K_electron_kaonID_SVDonly);
  variables->add(K_electron_kaonID_noSVD);

  variables->add(pi_pionID_ALL);
  variables->add(pi_pionID_SVDonly);
  variables->add(pi_pionID_noSVD);
  variables->add(pi_kaonID_ALL);
  variables->add(pi_kaonID_SVDonly);
  variables->add(pi_kaonID_noSVD);
  variables->add(pi_electronID_ALL);
  variables->add(pi_electronID_SVDonly);
  variables->add(pi_electronID_noSVD);
  variables->add(pi_protonID_ALL);
  variables->add(pi_protonID_SVDonly);
  variables->add(pi_protonID_noSVD);
  variables->add(pi_pion_kaonID_ALL);
  variables->add(pi_pion_kaonID_SVDonly);
  variables->add(pi_pion_kaonID_noSVD);
  variables->add(pi_kaon_pionID_ALL);
  variables->add(pi_kaon_pionID_SVDonly);
  variables->add(pi_kaon_pionID_noSVD);
  variables->add(pi_proton_pionID_ALL);
  variables->add(pi_proton_pionID_SVDonly);
  variables->add(pi_proton_pionID_noSVD);
  variables->add(pi_electron_pionID_ALL);
  variables->add(pi_electron_pionID_SVDonly);
  variables->add(pi_electron_pionID_noSVD);

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
    canvDstar->Print("SVDdEdxValidationFitDstar.pdf");
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

  B2INFO("Dstar: sPlot done. ");

  return treeDstar_sw;
}

// TTree SVDdEdxValidationAlgorithm::GammaHistogram(std::shared_ptr<TTree> preselTree)
// {
//   B2INFO("Histogramming the converted photon selection...");
//   gROOT->SetBatch(true);

//   if (preselTree->GetEntries() == 0)
//   {
//     B2FATAL("The Gamma tree is empty, stopping here");
//   }
//   std::vector<double> pbins = CreatePBinningScheme();
//   // const int m_numDEdxBins = 100;
//   // const int m_numPBins = 69;
//   // double pbins[m_numPBins + 1];
//   // pbins[0] = 0.0;
//   // pbins[1] = 0.05;

//   // for (int bin = 2; bin <= m_numPBins; bin++) {
//   //   if (bin <= 19)
//   //     pbins[bin] = 0.025 + 0.025 * bin;
//   //   else if (bin <= 59)
//   //     pbins[bin] = pbins[19] + 0.05 * (bin - 19);
//   //   else
//   //     pbins[bin] = pbins[59] + 0.3 * (bin - 59);
//   // }

//   // double m_dedxCutoff = 0;
//   // m_dedxCutoff = 5e6;

//   TH2F *h_GammaE = new TH2F("hist_d1_11_trunc", "hist_d1_11_trunc", m_numPBins, pbins.data(), m_numDEdxBins, 0, m_dedxCutoff);

//   TH2F *h_GammaEPart1 = (TH2F *)h_GammaE->Clone("hist_d1_11_truncPart1");
//   TH2F *h_GammaEPart2 = (TH2F *)h_GammaE->Clone("hist_d1_11_truncPart2");

//   preselTree->Draw("e_1_SVDdEdx:e_1_p>>hist_d1_11_truncPart1", "e_1_SVDdEdx>0", "goff");
//   preselTree->Draw("e_2_SVDdEdx:e_2_p>>hist_d1_11_truncPart2", "e_2_SVDdEdx>0", "goff");
//   h_GammaE->Add(h_GammaEPart1);
//   h_GammaE->Add(h_GammaEPart2);

//   // for each momentum bin, normalize the pdf
//   // h_GammaE normalisation
//   for (int pbin = 1; pbin <= m_numPBins; pbin++)
//   {
//     for (int dedxbin = 1; dedxbin <= m_numDEdxBins; dedxbin++)
//     {
//       // get rid of the bins with negative weights
//       if (h_GammaE->GetBinContent(pbin, dedxbin) < 0)
//       {
//         h_GammaE->SetBinContent(pbin, dedxbin, 0);
//       };
//     }

//     // create a projection (1D histogram) in a given momentum bin
//     TH1D *slice = (TH1D *)h_GammaE->ProjectionY("slice", pbin, pbin);
//     // normalise, but ignore the cases with empty histograms
//     if (slice->Integral() > 0)
//     {
//       slice->Scale(1. / slice->Integral());
//     }
//     // fill back the 2D histo with the result
//     for (int dedxbin = 1; dedxbin <= m_numDEdxBins; dedxbin++)
//     {
//       h_GammaE->SetBinContent(pbin, dedxbin, slice->GetBinContent(dedxbin));
//     }
//   }

//   if (m_isMakePlots)
//   {
//     TCanvas *canvGamma = new TCanvas("canvGamma", "canvGamma");
//     h_GammaE->Draw("COLZ");
//     canvGamma->Print("SVDdEdxCalibrationHistoGamma.pdf");
//   }

//   return *h_GammaE;
// }
