/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <dqm/analysis/modules/DQMHistAnalysisEcmsMonObj.h>
#include <framework/particledb/EvtGenDatabasePDG.h>
#include <string>
#include <unordered_map>

#include <TROOT.h>
#include <TRandom.h>
#include <TH1D.h>
#include <TLine.h>

#include <RooFitResult.h>
#include <RooRealVar.h>
#include <RooDataSet.h>
#include <RooGaussian.h>
#include <RooPlot.h>
#include <RooHist.h>
#include <RooAddPdf.h>
#include <RooPolyVar.h>
#include <RooArgusBG.h>
#include <RooSimultaneous.h>
#include <RooCategory.h>
#include <RooArgSet.h>
#include <RooConstVar.h>


using namespace std;
using namespace Belle2;
using namespace RooFit;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(DQMHistAnalysisEcmsMonObj);

DQMHistAnalysisEcmsMonObjModule::DQMHistAnalysisEcmsMonObjModule()
  : DQMHistAnalysisModule()
{
  setDescription("Module to monitor Ecms information.");
  B2DEBUG(20, "DQMHistAnalysisEcmsMonObj: Constructor done.");
}

DQMHistAnalysisEcmsMonObjModule::~DQMHistAnalysisEcmsMonObjModule()
{
}

void DQMHistAnalysisEcmsMonObjModule::initialize()
{
  m_monObj = getMonitoringObject("ecms");
  B2DEBUG(20, "DQMHistAnalysisEcmsMonObj: initialized.");
}

void DQMHistAnalysisEcmsMonObjModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisEcmsMonObj: beginRun called.");
}

void DQMHistAnalysisEcmsMonObjModule::event()
{
  B2DEBUG(20, "DQMHistAnalysisEcmsMonObj: event called.");
}


// Plot the resulting fit into the file
TCanvas*  DQMHistAnalysisEcmsMonObjModule::plotArgusFit(RooDataHist* dataE, RooAddPdf&  sumB, RooArgusBG& argus, RooGaussian& gauss,
                                                        RooRealVar& eNow,
                                                        TString nTag)
{
  // switch to the batch mode and store the current setup
  bool isBatch = gROOT->IsBatch();
  gROOT->SetBatch(kTRUE);


  // --- Plot toy data and composite PDF overlaid ---
  RooPlot* frame = eNow.frame(40) ;
  dataE->plotOn(frame) ;

  TString name = (nTag == "B0") ? "B^{0}" : "B^{#pm}";
  sumB.paramOn(frame, Label(name), Format("TE"), Layout(0.5, 0.8, 0.85));//, Parameters(pars));


  sumB.plotOn(frame, Components(argus), LineStyle(kDashed)) ;
  sumB.plotOn(frame, Components(gauss), LineStyle(kDashed), LineColor(kRed)) ;

  sumB.plotOn(frame);

  frame->GetXaxis()->SetTitleSize(0.0001);
  frame->GetXaxis()->SetLabelSize(0.0001);
  frame->SetTitleSize(0.0001);
  frame->GetYaxis()->SetTitleSize(0.055);
  frame->GetYaxis()->SetTitleOffset(1.1);
  frame->GetYaxis()->SetLabelSize(0.05) ;

  frame->SetTitle("");


  RooHist* hpull = frame->pullHist() ;
  hpull->Print();
  RooPlot* frameRat = eNow.frame(Title("."));
  frameRat->GetYaxis()->SetTitle("Pull");
  frameRat->GetYaxis()->CenterTitle();
  frameRat->GetYaxis()->SetTitleSize(0.13);

  frameRat->GetYaxis()->SetNdivisions(504);
  frameRat->GetYaxis()->SetLabelSize(0.12);
  frameRat->GetXaxis()->SetTitleSize(0.12);
  frameRat->GetXaxis()->SetTitleOffset(1.1);
  frameRat->GetXaxis()->SetLabelSize(0.12);

  frameRat->GetYaxis()->SetTitleOffset(0.4);
  frameRat->addPlotable(hpull, "x0 P E1");
  frameRat->SetMaximum(5.);
  frameRat->SetMinimum(-5.);

  TCanvas* c1 = new TCanvas(nTag + "_can");
  TPad* pad = new TPad(nTag + "_pad", "pad", 0, 0.3, 1, 1.0);
  pad->SetBottomMargin(0.03);
  pad->SetRightMargin(0.05);
  pad->SetLeftMargin(0.15);
  pad->Draw();
  pad->cd();
  frame->Draw() ;


  TLine* ll = new TLine;
  double mRev = 10579.4e-3 / 2; // Optimal collision energy
  ll->SetLineColor(kGreen);
  ll->DrawLine(mRev, 0, mRev,  frame->GetMaximum());


  c1->cd();
  TPad* padRat = new TPad(nTag + "_padRat", "padRat", 0, 0.00, 1, 0.3);
  padRat->SetTopMargin(0.04);
  padRat->SetBottomMargin(0.35);
  padRat->SetRightMargin(0.05);
  padRat->SetLeftMargin(0.15);
  padRat->Draw();
  padRat->cd();
  frameRat->Draw() ;
  c1->Update();

  TLine* l = new TLine(5279.34e-3, 0.0, 5.37, 0.0);
  l->SetLineColor(kBlue);
  l->SetLineWidth(3);
  l->Draw();


  return c1;
  gROOT->SetBatch(isBatch);
}


// Fit the EcmsBB histogram with Gaus+Argus function
unordered_map<string, double>  DQMHistAnalysisEcmsMonObjModule::fitEcmsBB(TH1D* hB0, TH1D* hBp)
{
  const double cMBp = EvtGenDatabasePDG::Instance()->GetParticle("B+")->Mass();
  const double cMB0 = EvtGenDatabasePDG::Instance()->GetParticle("B0")->Mass();

  RooRealVar eNow("eNow", "E^{*}_{B} [GeV]", cMBp, 5.37);

  RooDataHist* dataE0 = new RooDataHist("dataE0hist", "Dataset from histogram", RooArgSet(eNow), hB0);
  RooDataHist* dataEp = new RooDataHist("dataEphist", "Dataset from histogram", RooArgSet(eNow), hBp);


  RooCategory Bcharge("sample", "sample") ;
  Bcharge.defineType("B0") ;
  Bcharge.defineType("Bp") ;

  RooDataHist combData("combData", "combined data", eNow, Index(Bcharge), Import("B0", *dataE0), Import("Bp", *dataEp));


  // --- Build Gaussian signal PDF ---
  RooRealVar sigmean("mean", "#mu", 5.29, 5.27, 5.30) ;
  RooRealVar sigwidth("sigma", "#sigma", 0.00237, 0.0001, 0.030) ;

  RooGaussian gauss("gauss", "gaussian PDF", eNow, sigmean, sigwidth) ;

  // --- Build Argus background PDF ---
  RooRealVar argpar("Argus_param", "c_{Argus}", -150.7, -300., +50.0) ;
  RooRealVar endpointBp("EndPointBp", "endPoint parameter", cMBp, 5.27, 5.291) ; //B+ value
  RooRealVar endpointB0("EndPointB0", "endPoint parameter", cMB0, 5.27, 5.291) ; //B0 value
  endpointB0.setConstant(kTRUE);
  endpointBp.setConstant(kTRUE);


  //B0 pars
  RooPolyVar shape2B0("EndPoint2B0", "shape parameter", endpointB0, RooArgSet(RooConst(0.), RooConst(2.)));
  RooPolyVar eNowDifB0("eNowDifB0", "eNowDifB0", eNow, RooArgSet(shape2B0, RooConst(-1.)));
  RooArgusBG argusB0("argusB0", "Argus PDF", eNowDifB0, endpointB0, argpar) ;

  //Bp pars
  RooPolyVar shape2Bp("EndPoint2Bp", "shape parameter", endpointBp, RooArgSet(RooConst(0.), RooConst(2.)));
  RooPolyVar eNowDifBp("eNowDifBp", "eNowDifBp", eNow, RooArgSet(shape2Bp, RooConst(-1.)));
  RooArgusBG argusBp("argusBp", "Argus PDF", eNowDifBp, endpointBp, argpar) ;


  // --- Construct signal+background PDF ---
  RooRealVar nsigB0("nsigB0", "N_{sig}^{B^{0}}", 100, 0., 100000) ;
  RooRealVar nbkgB0("nbkgB0", "N_{bg}", 100, 0., 500000) ;

  RooRealVar nsigBp("nsigBp", "N_{sig}^{B^{#pm}}", 100, 0., 100000) ;
  RooRealVar nbkgBp("nbkgBp", "N_{bg}", 100, 0., 500000) ;



  RooAddPdf sumB0("sumB0", "g0+a0", RooArgList(gauss, argusB0), RooArgList(nsigB0, nbkgB0)) ;
  RooAddPdf sumBp("sumBp", "gP+aP", RooArgList(gauss, argusBp), RooArgList(nsigBp, nbkgBp)) ;


  // Construct a simultaneous pdf using category sample as index
  RooSimultaneous simPdf("simPdf", "simultaneous pdf", Bcharge) ;


  // Associate model with the physics state and model_ctl with the control state
  simPdf.addPdf(sumB0,  "B0") ;
  simPdf.addPdf(sumBp,  "Bp") ;


  simPdf.fitTo(combData);

  // Plot the results into the Canvas
  m_canvas = new TCanvas("ecms", "ecms", 1500, 800);
  TCanvas* c0 = plotArgusFit(dataE0, sumB0, argusB0, gauss, eNow, "B0");
  TCanvas* cp = plotArgusFit(dataEp, sumBp, argusBp, gauss, eNow, "Bp");

  m_canvas->cd();
  m_canvas->Divide(2, 1);

  m_canvas->cd(1);
  for (auto obj : *cp->GetListOfPrimitives()) obj->DrawClone();
  m_canvas->cd(2);
  for (auto obj : *c0->GetListOfPrimitives()) obj->DrawClone();

  // Go back to the main cBase canvas
  m_canvas->cd();
  m_monObj->addCanvas(m_canvas);


  // Delete RooFit objects
  delete dataE0;
  delete dataEp;


  return {{"EcmsBBcnt",    2 * sigmean.getValV()},  {"EcmsBBcntUnc",    2 * sigmean.getError()},
    {"EcmsBBspread", 2 * sigwidth.getValV()}, {"EcmsBBspreadUnc", 2 * sigwidth.getError()}};
}


void DQMHistAnalysisEcmsMonObjModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisEcmsMonObj: endRun called.");

  auto* hB0 = (TH1D*)findHist("PhysicsObjectsMiraBelleEcmsBB/hB0");
  auto* hBp = (TH1D*)findHist("PhysicsObjectsMiraBelleEcmsBB/hBp");

  auto res = fitEcmsBB(hB0, hBp);

  m_monObj->setVariable("EcmsBBcnt", res.at("EcmsBBcnt"), res.at("EcmsBBcntUnc"));
  m_monObj->setVariable("EcmsBBspread", res.at("EcmsBBspread"), res.at("EcmsBBspreadUnc"));
}

void DQMHistAnalysisEcmsMonObjModule::terminate()
{

  B2DEBUG(20, "terminate called");
}

