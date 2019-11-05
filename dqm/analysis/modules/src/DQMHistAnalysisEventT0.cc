/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ewan Hill  (ehill@mail.ubc.ca)                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <dqm/analysis/modules/DQMHistAnalysisEventT0.h>
#include <boost/format.hpp>
#include <TROOT.h>

#include <TF1.h>
#include <TCanvas.h>

using namespace std;
using namespace Belle2;
using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisEventT0)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisEventT0Module::DQMHistAnalysisEventT0Module()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  B2DEBUG(20, "DQMHistAnalysisEventT0: Constructor done.");
}


DQMHistAnalysisEventT0Module::~DQMHistAnalysisEventT0Module() { }

void DQMHistAnalysisEventT0Module::initialize()
{
  gROOT->cd(); // this seems to be important, or strange things happen
  B2DEBUG(1, "DQMHistAnalysisEventT0: initialized.");

  c_h_eventT0_ECL_bhabha = new TCanvas("EventT0/c_h_eventT0_ECL_bhabha");
  c_h_eventT0_CDC_bhabha = new TCanvas("EventT0/c_h_eventT0_CDC_bhabha");
  c_h_eventT0_TOP_bhabha = new TCanvas("EventT0/c_h_eventT0_TOP_bhabha");
  c_h_eventT0_ECL_hadron = new TCanvas("EventT0/c_h_eventT0_ECL_hadron");
  c_h_eventT0_CDC_hadron = new TCanvas("EventT0/c_h_eventT0_CDC_hadron");
  c_h_eventT0_TOP_hadron = new TCanvas("EventT0/c_h_eventT0_TOP_hadron");

  int nBins = 160 ;
  double minT0 = -40 ;
  double maxT0 =  40 ;
  h_eventT0_ECL_bhabha = new TH1F("EventT0/h_eventT0_ECL_bhabha", "ECL event t0 - bhabhas;event t0 [ns];events / 0.5 ns", nBins,
                                  minT0, maxT0);
  h_eventT0_CDC_bhabha = new TH1F("EventT0/h_eventT0_CDC_bhabha", "CDC event t0 - bhabhas;event t0 [ns];events / 0.5 ns", nBins,
                                  minT0, maxT0);
  h_eventT0_TOP_bhabha = new TH1F("EventT0/h_eventT0_TOP_bhabha", "TOP event t0 - bhabhas;event t0 [ns];events / 0.5 ns", nBins,
                                  minT0, maxT0);
  h_eventT0_ECL_hadron = new TH1F("EventT0/h_eventT0_ECL_hadron", "ECL event t0 - hadrons;event t0 [ns];events / 0.5 ns", nBins,
                                  minT0, maxT0);
  h_eventT0_CDC_hadron = new TH1F("EventT0/h_eventT0_CDC_hadron", "CDC event t0 - hadrons;event t0 [ns];events / 0.5 ns", nBins,
                                  minT0, maxT0);
  h_eventT0_TOP_hadron = new TH1F("EventT0/h_eventT0_TOP_hadron", "TOP event t0 - hadrons;event t0 [ns];events / 0.5 ns", nBins,
                                  minT0, maxT0);

  h_eventT0_ECL_bhabha->SetDirectory(0);
  h_eventT0_CDC_bhabha->SetDirectory(0);
  h_eventT0_TOP_bhabha->SetDirectory(0);
  h_eventT0_ECL_hadron->SetDirectory(0);
  h_eventT0_CDC_hadron->SetDirectory(0);
  h_eventT0_TOP_hadron->SetDirectory(0);
}


void DQMHistAnalysisEventT0Module::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisEventT0: beginRun called.");
}

void DQMHistAnalysisEventT0Module::event()
{
  B2DEBUG(20, "DQMHistAnalysisEventT0 : event called");
}

void DQMHistAnalysisEventT0Module::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisEventT0 : endRun called");
}

void DQMHistAnalysisEventT0Module::terminate()
{
  B2DEBUG(20, "terminate called");
}

