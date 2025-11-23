/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisRooFitExample.cc
// Description : DQM Histogram analysis module, using roofit to fit the histogram
//-


#include <dqm/analysis/modules/DQMHistAnalysisRootFitExample.h>
#include <RooRealVar.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisRooFitExample);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisRooFitExampleModule::DQMHistAnalysisRooFitExampleModule()
  : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!

  //Parameter definition
  B2DEBUG(1, "DQMHistAnalysisRooFitExample: Constructor done.");
}

DQMHistAnalysisRooFitExampleModule::~DQMHistAnalysisRooFitExampleModule()
{
}

void DQMHistAnalysisRooFitExampleModule::initialize()
{
  B2INFO("DQMHistAnalysisRooFitExample: initialized.");

  registerEpicsPV("example/fit_value", "fit_value"); // in real usage, add a proper prefix

  w = new RooWorkspace("w");
  w->factory("Gaussian::f(x[-20,20],mean[0,-5,5],sigma[3,1,10])");
  model = w->pdf("f");

  m_c0 = new TCanvas("example0");
}


void DQMHistAnalysisRooFitExampleModule::beginRun()
{
  B2INFO("DQMHistAnalysisRooFitExample: beginRun called.");
  m_c0->Clear();

  TH1* hh1 = findHist("FirstDet/h_HitXPositionCh01");
  if (hh1 != NULL) {

    //RooDataHist::RooDataHist(const char *name, const char *title, const RooArgList& vars, const TH1* hist, Double_t wgt) :
    x = w->var("x");
    data = new RooDataHist("data", "dataset with x", *x, (const TH1*) hh1);
    plot = x->frame();
    r = model->fitTo(*data);

    // plot data and function

    data->plotOn(plot);
    model->plotOn(plot);

    m_c0->cd();
    plot->Draw();
  } else {
    B2WARNING("Histo not there ...");
  }

}

void DQMHistAnalysisRooFitExampleModule::event()
{
  TH1* hh1;

  hh1 = findHist("FirstDet/h_HitXPositionCh01");

  if (hh1 != NULL) {
    if (data) delete data;

    data = new RooDataHist("data", "dataset with x", *(w->var("x")), hh1);
    data->Print();
    r = model->fitTo(*data);
    data->Print();

//     RooPlot *m_plot = x->frame();
    plot = x->frame();

    //    while(plot->numItems()>0){
    //      plot->remove("",false);
    //    }
    //    printf("5\n");

    data->plotOn(plot);
    model->plotOn(plot);

    m_c0->cd();
    plot->Draw();

    m_c0->Modified();
    m_c0->Update();

  }

  double fitdata = 0;
  setEpicsPV("fit_value", fitdata);
}

void DQMHistAnalysisRooFitExampleModule::endRun()
{
  B2INFO("DQMHistAnalysisRooFitExample: endRun called");
}


void DQMHistAnalysisRooFitExampleModule::terminate()
{
  B2INFO("DQMHistAnalysisRooFitExample: terminate called");
}

