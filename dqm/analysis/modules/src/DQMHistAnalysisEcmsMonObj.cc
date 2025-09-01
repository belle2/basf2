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
#include <filesystem>

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
static void plotArgusFit(RooDataHist* dataE0, RooAddPdf&  sumB0, RooArgusBG& argus, RooGaussian& gauss, RooRealVar& eNow,
                         TString fName = "")
{
  // switch to the batch mode and store the current setup
  bool isBatch = gROOT->IsBatch();
  gROOT->SetBatch(kTRUE);


  // --- Plot toy data and composite PDF overlaid ---
  RooPlot* mbcframe = eNow.frame(40) ;
  dataE0->plotOn(mbcframe) ;

  sumB0.paramOn(mbcframe); //, dataE0);


  sumB0.plotOn(mbcframe, Components(argus), LineStyle(kDashed)) ;
  sumB0.plotOn(mbcframe, Components(gauss), LineStyle(kDashed), LineColor(kRed)) ;

  sumB0.plotOn(mbcframe);

  mbcframe->GetXaxis()->SetTitleSize(0.0001);
  mbcframe->GetXaxis()->SetLabelSize(0.0001);
  mbcframe->SetTitleSize(0.0001);
  mbcframe->GetYaxis()->SetTitleSize(0.06);
  mbcframe->GetYaxis()->SetTitleOffset(0.7);
  mbcframe->GetYaxis()->SetLabelSize(0.06) ;

  mbcframe->SetTitle("");


  RooHist* hpull = mbcframe->pullHist() ;
  hpull->Print();
  RooPlot* frame3 = eNow.frame(Title(".")) ;
  frame3->GetYaxis()->SetTitle("Pull") ;
  frame3->GetYaxis()->SetTitleSize(0.13) ;

  frame3->GetYaxis()->SetNdivisions(504) ;
  frame3->GetYaxis()->SetLabelSize(0.15) ;
  frame3->GetXaxis()->SetTitleSize(0.15) ;
  frame3->GetXaxis()->SetLabelSize(0.15) ;

  frame3->GetYaxis()->SetTitleOffset(0.3) ;
  frame3->addPlotable(hpull, "x0 P E1") ;
  frame3->SetMaximum(5.);
  frame3->SetMinimum(-5.);

  TString rn = Form("%d", gRandom->Integer(1000000)); // random name
  TCanvas* c1 = new TCanvas(rn) ;
  TPad* pad1 = new TPad("pad1", "pad1", 0, 0.3, 1, 1.0);
  pad1->Draw();             // Draw the upper pad: pad1
  pad1->cd();
  mbcframe->Draw() ;


  TLine* ll = new TLine;
  double mRev = 10579.4e-3 / 2;
  ll->SetLineColor(kGreen);
  ll->DrawLine(mRev, 0, mRev,  mbcframe->GetMaximum());


  c1->cd();          // Go back to the main canvas before defining pad2
  TPad* pad2 = new TPad("pad2", "pad2", 0, 0.00, 1, 0.3);
  pad2->SetTopMargin(0.05);
  pad2->SetBottomMargin(0.35);
  pad2->Draw();
  pad2->cd();
  frame3->Draw() ;
  c1->Update();

  TLine* l = new TLine(5279.34e-3, 0.0, 5.37, 0.0);
  l->SetLineColor(kBlue);
  l->SetLineWidth(3);
  l->Draw();


  if (fName != "") c1->SaveAs(fName);


  //delete hpull;
  delete ll;
  delete l;
  delete pad1;
  delete pad2;

  delete c1;
  //exit(0);


  gROOT->SetBatch(isBatch);
}


// Fit the EcmsBB histogram with Gaus+Argus function
static unordered_map<string, double> fitEcmsBB(TH1D* hB0, TH1D* hBp)
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
  RooRealVar sigmean("Mean", "B^{#pm} mass", 5.29, 5.27, 5.30) ;
  RooRealVar sigwidth("#sigma", "B^{#pm} width", 0.00237, 0.0001, 0.030) ;

  RooGaussian gauss("gauss", "gaussian PDF", eNow, sigmean, sigwidth) ;

  // --- Build Argus background PDF ---
  RooRealVar argpar("Argus_param", "argus shape parameter", -150.7, -300., +50.0) ;
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
  RooRealVar nsigB0("nsigB0", "#signal events", 100, 0., 100000) ;
  RooRealVar nbkgB0("nbkgB0", "#background events", 100, 0., 500000) ;

  RooRealVar nsigBp("nsigBp", "#signal events", 100, 0., 100000) ;
  RooRealVar nbkgBp("nbkgBp", "#background events", 100, 0., 500000) ;



  RooAddPdf sumB0("sumB0", "g0+a0", RooArgList(gauss, argusB0), RooArgList(nsigB0, nbkgB0)) ;
  RooAddPdf sumBp("sumBp", "gP+aP", RooArgList(gauss, argusBp), RooArgList(nsigBp, nbkgBp)) ;


  // Construct a simultaneous pdf using category sample as index
  RooSimultaneous simPdf("simPdf", "simultaneous pdf", Bcharge) ;


  // Associate model with the physics state and model_ctl with the control state
  simPdf.addPdf(sumB0,  "B0") ;
  simPdf.addPdf(sumBp,  "Bp") ;


  simPdf.fitTo(combData);

  // Creates fitting control plots, in "plotsHadBonly" dictionary, can be removed
  {
    std::filesystem::create_directories("plotsHadBonly");
    TString nTag = "dummy";
    plotArgusFit(dataE0, sumB0, argusB0, gauss, eNow, Form("plotsHadBonly/B0Single_%s.pdf", nTag.Data()));
    plotArgusFit(dataEp, sumBp, argusBp, gauss, eNow, Form("plotsHadBonly/BpSingle_%s.pdf", nTag.Data()));
  }

  // Delete rooFit objects
  delete dataE0;
  delete dataEp;


  return {{"EcmsBBcnt",    2 * sigmean.getValV()},  {"EcmsBBcntUnc",    2 * sigmean.getError()},
    {"EcmsBBspread", 2 * sigwidth.getValV()}, {"EcmsBBspreadUnc", 2 * sigwidth.getError()}};
}


void DQMHistAnalysisEcmsMonObjModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisEcmsMonObj: endRun called.");

  auto* hB0 = (TH1D*)findHist("EcmsBB/hB0");
  auto* hBp = (TH1D*)findHist("EcmsBB/hBp");

  auto res = fitEcmsBB(hB0, hBp);

  m_monObj->setVariable("EcmsBBcnt", res.at("EcmsBBcnt"), res.at("EcmsBBcntUnc"));
  m_monObj->setVariable("EcmsBBspread", res.at("EcmsBBspread"), res.at("EcmsBBspreadUnc"));
}

void DQMHistAnalysisEcmsMonObjModule::terminate()
{

  B2DEBUG(20, "terminate called");
}

