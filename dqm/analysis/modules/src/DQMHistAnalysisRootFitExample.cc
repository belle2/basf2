//+
// File : DQMHistAnalysisRooFitExample.cc
// Description : DQM Histogram analysis module, using roofit to fit the histogram
//
// Author : B. Spruck
// Date : 25 - Mar - 2017
// based on work from Tomoyuki Konno, Tokyo Metropolitan Univerisity
//-


#include <dqm/analysis/modules/DQMHistAnalysisRootFitExample.h>
#include <RooRealVar.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisRooFitExample)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisRooFitExampleModule::DQMHistAnalysisRooFitExampleModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  B2DEBUG(1, "DQMHistAnalysisRooFitExample: Constructor done.");
}


DQMHistAnalysisRooFitExampleModule::~DQMHistAnalysisRooFitExampleModule() { }

void DQMHistAnalysisRooFitExampleModule::initialize()
{
  B2INFO("DQMHistAnalysisRooFitExample: initialized.");

//   SEVCHK(ca_context_create(ca_disable_preemptive_callback),"ca_context_create");
//   SEVCHK(ca_create_channel("fit_value",NULL,NULL,10,&mychid),"ca_create_channel failure");
//   SEVCHK(ca_pend_io(5.0),"ca_pend_io failure");

  w = new RooWorkspace("w");
  w->factory("Gaussian::f(x[-20,20],mean[0,-5,5],sigma[3,1,10])");
  model = w->pdf("f");

  m_c0 = new TCanvas("example0");
}


void DQMHistAnalysisRooFitExampleModule::beginRun()
{
  //m_serv->SetTimer(100, kFALSE);
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
    B2FATAL("Histo now there ... -> zero pointer crash");
  }

}

void DQMHistAnalysisRooFitExampleModule::event()
{
  TH1* hh1;

  printf("0\n");
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

#ifdef _BELLE2_EPICS
//   double data=0;
//   SEVCHK(ca_put(DBR_DOUBLE,mychid,(void*)&data),"ca_set failure");
//   SEVCHK(ca_pend_io(5.0),"ca_pend_io failure");
#endif
}

void DQMHistAnalysisRooFitExampleModule::endRun()
{
  B2INFO("DQMHistAnalysisRooFitExample: endRun called");
}


void DQMHistAnalysisRooFitExampleModule::terminate()
{
  B2INFO("DQMHistAnalysisRooFitExample: terminate called");
}

