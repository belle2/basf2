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
  : DQMHistAnalysisSVDModule()
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

  m_legProblem->Clear();
  m_legProblem->AddText("ERROR!!");

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

  TString tmp = hnEvnts->GetTitle();
  Int_t pos = tmp.Last('~');
  if (pos == -1) pos = 0;

  TString runID = tmp(pos, tmp.Length() - pos);
  B2INFO("DQMHistAnalysisSVDUnpackerModule::runID = " << runID);
  Float_t nEvents = hnEvnts->GetEntries();

  //check DATA FORMAT
  TH2* h = (TH2*)findHist("SVDUnpacker/DQMUnpackerHisto");

  if (h != NULL) {
    h->SetTitle(Form("SVD Data Format Monitor %s", runID.Data()));
    //check if number of errors is above the allowed limit
    Int_t nXbins = h->GetXaxis()->GetNbins();
    Int_t nYbins = h->GetYaxis()->GetNbins();

    //test ERROR:
    // h->SetBinContent(10, 20, 10000000);

    Float_t counts = 0;
    bool hasError = false;
    for (int i = 0; i < nXbins - 1; ++i) { // exclude SEU recovery
      for (int j = 0; j < nYbins; ++j) {
        counts += h->GetBinContent(i + 1, j + 1);
        if (h->GetBinContent(i + 1, j + 1) / nEvents > m_unpackError) {
          hasError = true;
        }
      }
    }

    if (!hasError) {
      m_cUnpacker->cd();
      h->Draw("colztext");
      h->SetStats(0);
      setStatusOfCanvas(good, m_cUnpacker, false);
    } else {
      m_cUnpacker->cd();
      h->Draw("colztext");
      h->SetStats(0);
      setStatusOfCanvas(error, m_cUnpacker, true);
    }
    if (nEvents > 0)
      setEpicsPV("UnpackError", counts / nEvents);
  } else {
    B2INFO("Histogram SVDUnpacker/DQMUnpackerHisto from SVDUnpackerDQM not found!");
    m_cUnpacker->cd();
    colorizeCanvas(m_cUnpacker, c_StatusDefault);
    setStatusOfCanvas(noStat, m_cUnpacker);
  }

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
}
