/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisSVDUnpacker.cc
// Description : module for DQM histogram analysis of SVD unpacker fromat error
//-


#include <dqm/analysis/modules/DQMHistAnalysisSVDUnpacker.h>
#include <vxd/geometry/GeoCache.h>

#include <TROOT.h>
#include <TStyle.h>
#include <TString.h>
#include <TAxis.h>

#include <TMath.h>
#include <iostream>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisSVDUnpacker);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisSVDUnpackerModule::DQMHistAnalysisSVDUnpackerModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  B2DEBUG(10, "DQMHistAnalysisSVDUnpacker: Constructor done.");

  setDescription("DQM Analysis Module that produces colored canvas for a straightforward interpretation of the SVD Data Quality.");

  addParam("unpackerErrorLevel", m_unpackError, "Maximum bin_content/ # events allowed before throwing ERROR", double(0.00001));
  addParam("printCanvas", m_printCanvas, "if True prints pdf of the analysis canvas", bool(false));
  addParam("samples3", m_3Samples, "if True 3 samples histograms analysis is performed", bool(false));
  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("SVD:"));
}

DQMHistAnalysisSVDUnpackerModule::~DQMHistAnalysisSVDUnpackerModule() { }

void DQMHistAnalysisSVDUnpackerModule::initialize()
{
  B2DEBUG(10, "DQMHistAnalysisSVDUnpacker: initialized.");

  m_legError = new TPaveText(-1, 54, 3, 57.5, "br");
  m_legError->AddText("ERROR!!");
  m_legError->SetFillColor(c_ColorError);
  m_legError->SetTextColor(c_ColorDefault);


  gROOT->cd();
  m_cUnpacker = new TCanvas("SVDAnalysis/c_SVDDataFormat");
  m_cUnpacker->SetGrid(1);

  //register limits for EPICS
  registerEpicsPV(m_pvPrefix + "UnpackError", "UnpackError");
}


void DQMHistAnalysisSVDUnpackerModule::beginRun()
{
  B2DEBUG(10, "DQMHistAnalysisSVDUnpacker: beginRun called.");
  m_cUnpacker->Clear();

  //Retrieve limits from EPICS

  double unpackWarnLo = 0.;
  double unpackWarnUp = 0.;
  double unpackErrorLo = 0.;
  requestLimitsFromEpicsPVs("UnpackError", unpackErrorLo, unpackWarnLo, unpackWarnUp,  m_unpackError);
  B2DEBUG(10, " SVD unpack error threshold taken from EPICS configuration file:");
  B2DEBUG(10, "  DATA UNPACK: error > " << m_unpackError);
}

void DQMHistAnalysisSVDUnpackerModule::event()
{
  B2DEBUG(10, "DQMHistAnalysisSVDUnpacker: event called.");

  //find nEvents
  TH1* hnEvnts = findHist("SVDExpReco/SVDDQM_nEvents", true);
  if (hnEvnts == NULL) {
    B2INFO("no events, nothing to do here");
    return;
  } else {
    B2DEBUG(10, "SVDExpReco/SVDDQM_nEvents found");
  }

  TH1* rtype = findHist("DQMInfo/rtype");
  if (rtype)
    B2DEBUG(10, "DQMInfo/rtype found");

  m_runtype = rtype ? rtype->GetTitle() : "physics"; // per default

  TString tmp = hnEvnts->GetTitle();
  Int_t pos = tmp.Last('~');
  if (pos == -1) pos = 0;

  TString runID = tmp(pos, tmp.Length() - pos);
  B2INFO("DQMHistAnalysisSVDUnpackerModule::runID = " << runID);
  Float_t nEvents = hnEvnts->GetEntries();

  //check DATA FORMAT
  TH1* h = findHist("SVDUnpacker/DQMUnpackerHisto");

  //test ERROR:
  //  h->SetBinContent(100,0.01);

  if (h != NULL) {
    h->SetTitle(Form("SVD Data Format Monitor %s", runID.Data()));
    //check if number of errors is above the allowed limit
    bool hasError = false;
    for (int un = 0; un < h->GetNcells(); un++)
      if (h->GetBinContent(un) / nEvents > m_unpackError)
        hasError = true;
    if (! hasError) {
      m_cUnpacker->cd();
      h->Draw("colztext");
      h->SetStats(0);
      colorizeCanvas(m_cUnpacker, c_StatusGood);
    } else {
      m_cUnpacker->cd();
      h->Draw("colztext");
      h->SetStats(0);
      m_legError->Draw();
      colorizeCanvas(m_cUnpacker, c_StatusError);
    }
    if (nEvents > 0)
      setEpicsPV("UnpackError", h->GetEntries() / nEvents);
  } else {
    B2INFO("Histogram SVDUnpacker/DQMUnpackerHisto from SVDUnpackerDQM not found!");
    m_cUnpacker->cd();
    colorizeCanvas(m_cUnpacker, c_StatusDefault);
  }

  m_cUnpacker->Modified();
  m_cUnpacker->Update();

  if (m_printCanvas)
    m_cUnpacker->Print("c_SVDDataFormat.pdf");

}

void DQMHistAnalysisSVDUnpackerModule::endRun()
{
  B2DEBUG(10, "DQMHistAnalysisSVDUnpacker: endRun called");
}


void DQMHistAnalysisSVDUnpackerModule::terminate()
{
  B2DEBUG(10, "DQMHistAnalysisSVDUnpacker: terminate called");

  delete m_cUnpacker;

  delete m_legError;

}
