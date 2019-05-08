//+
// File : DQMHistAnalysisSVDEfficiency.cc
// Description :
//
// Author : Giulia Casarosa (PI), Gaetano De Marino (PI)
// Date : 20190428
//-


#include <dqm/analysis/modules/DQMHistAnalysisSVDEfficiency.h>
#include <vxd/geometry/GeoCache.h>

#include <TROOT.h>
#include <TStyle.h>
#include <TString.h>
#include <TPaletteAxis.h>
#include <TBox.h>
#include <TAxis.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisSVDEfficiency)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisSVDEfficiencyModule::DQMHistAnalysisSVDEfficiencyModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  B2INFO("DQMHistAnalysisSVDEfficiency: Constructor done.");

  addParam("RefHistoFile", m_refFileName, "Reference histrogram file name", std::string("SVDrefHisto.root"));
  addParam("effLevel_Error", m_effError, "Maximum Efficiency (%) allowed for safe operations (red)", float(0.5));
  addParam("effLevel_Warning", m_effWarning, "Efficiency (%) at WARNING level (orange)", float(0.7));
  addParam("effLevel_Empty", m_effEmpty, "Maximum Efficiency (%) for which the sensor is considered empty", float(0));
  addParam("printCanvas", m_printCanvas, "if True prints pdf of the analysis canvas", bool(false));
  /*  addParam("errEffLevel_Error", m_errEffError, "Maximum Efficiency Error allowed for safe operations (red)", float(5));
  addParam("errEffLevel_Warning", m_errEffWarning, "Efficiency Error at WARNING level (orange)", float(3));*/

}

DQMHistAnalysisSVDEfficiencyModule::~DQMHistAnalysisSVDEfficiencyModule() { }

void DQMHistAnalysisSVDEfficiencyModule::initialize()
{
  B2INFO("DQMHistAnalysisSVDEfficiency: initialized.");
  B2DEBUG(10, " black = " << kBlack);
  B2DEBUG(10, " green = " << kGreen);
  B2DEBUG(10, " orange = " << kOrange);
  B2DEBUG(10, " Red = " << kRed);

  m_refFile = NULL;
  if (m_refFileName != "") {
    m_refFile = new TFile(m_refFileName.data(), "READ");
  }

  //search for reference
  if (m_refFile && m_refFile->IsOpen()) {
    B2INFO("SVD DQMHistAnalysis: reference root file (" << m_refFileName << ") FOUND, reading ref histograms");

    TH1F* ref_eff = (TH1F*)m_refFile->Get("refEfficiency");
    if (!ref_eff)
      B2WARNING("SVD DQMHistAnalysis: Efficiency Level Refence not found! using module parameters");
    else {
      m_effEmpty = ref_eff->GetBinContent(1);
      m_effWarning = ref_eff->GetBinContent(2);
      m_effError = ref_eff->GetBinContent(3);
    }

  } else
    B2WARNING("SVD DQMHistAnalysis: reference root file (" << m_refFileName << ") not found, or closed, using module parameters");

  B2INFO(" SVD efficiency thresholds:");
  B2INFO(" EFFICIENCY: empty < " << m_effEmpty << " normal < " << m_effWarning << "warning < " << m_effError << " < error");

  VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  //collect the list of all SVD Modules in the geometry here
  std::vector<VxdID> sensors = geo.getListOfSensors();
  for (VxdID& aVxdID : sensors) {
    VXD::SensorInfoBase info = geo.getSensorInfo(aVxdID);
    // B2INFO("VXD " << aVxdID);
    if (info.getType() != VXD::SensorInfoBase::SVD) continue;
    m_SVDModules.push_back(aVxdID); // reorder, sort would be better
  }
  std::sort(m_SVDModules.begin(), m_SVDModules.end());  // back to natural order

  gROOT->cd();
  m_cEfficiencyU = new TCanvas("SVDAnalysis/c_SVDEfficiencyU");
  m_cEfficiencyV = new TCanvas("SVDAnalysis/c_SVDEfficiencyV");
  m_cEfficiencyErrU = new TCanvas("SVDAnalysis/c_SVDEfficiencyErrU");
  m_cEfficiencyErrV = new TCanvas("SVDAnalysis/c_SVDEfficiencyErrV");


  m_hEfficiency = new SVDSummaryPlots("SVDEfficiency@view", "Summary of SVD efficiencies for the @view/@side Side");
  m_hEfficiencyErr = new SVDSummaryPlots("SVDEfficiencyErr@view", "Summary of SVD efficiencies errors for the @view/@side Side");
}

void DQMHistAnalysisSVDEfficiencyModule::beginRun()
{
  B2INFO("DQMHistAnalysisSVDEfficiency: beginRun called.");
  m_cEfficiencyU->Clear();
  m_cEfficiencyV->Clear();
  m_cEfficiencyErrU->Clear();
  m_cEfficiencyErrV->Clear();
}

void DQMHistAnalysisSVDEfficiencyModule::event()
{
  B2INFO("DQMHistAnalysisSVDEfficiency: event called.");

  //SETUP gSTYLE - all plots
  //  gStyle->SetOptStat(0);
  //  gStyle->SetTitleY(.97);

  //check MODULE EFFICIENCY
  m_effUstatus = 0;
  m_effVstatus = 0;
  m_effUErrstatus = 0;
  m_effVErrstatus = 0;

  //set dedicate gStyle
  const Int_t colNum = 4;
  Int_t palette[colNum] {kBlack,  kGreen, kOrange, kRed};
  gStyle->SetPalette(colNum, palette);
  gStyle->SetOptStat(0);
  gStyle->SetPaintTextFormat("2.3f");

  SVDSummaryPlots* found_tracksU = NULL;
  SVDSummaryPlots* matched_clusU = NULL;

  found_tracksU = (SVDSummaryPlots*)findHist("svdeff/TrackHits@view");
  matched_clusU = (SVDSummaryPlots*)findHist("svdeff/MatchedHits@view");

  B2INFO("DQMHistAnalysisSVDEfficiency: before cycle.");

  if (matched_clusU == NULL || found_tracksU == NULL) {
    B2INFO("Histograms not found");
    m_cEfficiencyU->SetFillColor(kRed);
  } else {
    for (unsigned int i = 0; i < m_SVDModules.size(); i++) {
      B2INFO("Before getValue, size :" << m_SVDModules.size());
      effU = matched_clusU->getValue(m_SVDModules[i], 1) / found_tracksU->getValue(m_SVDModules[i], 1);
      B2INFO("effU  :" << effU);
      B2INFO("After getValue");
      m_hEfficiency->fill(m_SVDModules[i], 1, effU);
      erreffU = std::sqrt(effU * (1 - effU));
      m_hEfficiencyErr->fill(m_SVDModules[i], 1, erreffU);

      if (effU <= m_effEmpty) {
        if (m_effUstatus < 1) m_effUstatus = 1;
      } else if (effU > m_effWarning) {
        if (effU < m_effError) {
          if (m_effUstatus < 2) m_effUstatus = 2;
        } else {
          if (m_effUstatus < 3) m_effUstatus = 3;
        }
      }
    }
  }

  B2INFO("DQMHistAnalysisSVDEfficiency: after cycle.");
  SVDSummaryPlots* found_tracksV = NULL;
  SVDSummaryPlots* matched_clusV = NULL;

  found_tracksV = (SVDSummaryPlots*)findHist("svdeff/TrackHits@view");
  matched_clusV = (SVDSummaryPlots*)findHist("svdeff/MatchedHits@view");

  if (matched_clusV == NULL || found_tracksV == NULL) {
    B2INFO("Histograms not found");
    m_cEfficiencyV->SetFillColor(kRed);
  } else {
    for (unsigned int i = 0; i < m_SVDModules.size(); i++) {
      effV = matched_clusV->getValue(m_SVDModules[i], 0) / found_tracksV->getValue(m_SVDModules[i], 0);
      m_hEfficiency->fill(m_SVDModules[i], 0, effV);
      erreffV = std::sqrt(effV * (1 - effV));
      m_hEfficiencyErr->fill(m_SVDModules[i], 0, erreffV);

      if (effV <= m_effEmpty) {
        if (m_effVstatus < 1) m_effVstatus = 1;
      } else if (effV > m_effWarning) {
        if (effV < m_effError) {
          if (m_effVstatus < 2) m_effVstatus = 2;
        } else {
          if (m_effVstatus < 3) m_effVstatus = 3;
        }
      }
    }
  }
  //update summary
  m_cEfficiencyU->cd();
  m_hEfficiency->getHistogram(1)->Draw();

  if (m_effUstatus == 0) {
    m_cEfficiencyU->SetFillColor(kGreen);
    m_cEfficiencyU->SetFrameFillColor(10);
    m_legNormal->Draw("same");
  } else {
    if (m_effUstatus == 3) {
      m_cEfficiencyU->SetFillColor(kRed);
      m_cEfficiencyU->SetFrameFillColor(10);
      m_legProblem->Draw("same");
    }
    if (m_effUstatus == 2) {
      m_cEfficiencyU->SetFillColor(kOrange);
      m_cEfficiencyU->SetFrameFillColor(10);
      m_legWarning->Draw("same");
    }
    if (m_effUstatus == 1) {
      m_cEfficiencyU->SetFillColor(kGray);
      m_cEfficiencyU->SetFrameFillColor(10);
      m_legEmpty->Draw("same");
    }
  }
  m_cEfficiencyU->Draw();
  m_cEfficiencyU->Update();
  m_cEfficiencyU->Modified();
  m_cEfficiencyU->Update();


  //update summary
  m_cEfficiencyV->cd();
  m_hEfficiency->getHistogram(0)->Draw();

  if (m_effVstatus == 0) {
    m_cEfficiencyV->SetFillColor(kGreen);
    m_cEfficiencyV->SetFrameFillColor(10);
    m_legNormal->Draw("same");
  } else {
    if (m_effVstatus == 3) {
      m_cEfficiencyV->SetFillColor(kRed);
      m_cEfficiencyV->SetFrameFillColor(10);
      m_legProblem->Draw("same");
    }
    if (m_effVstatus == 2) {
      m_cEfficiencyV->SetFillColor(kOrange);
      m_cEfficiencyV->SetFrameFillColor(10);
      m_legWarning->Draw("same");
    }
    if (m_effVstatus == 1) {
      m_cEfficiencyV->SetFillColor(kGray);
      m_cEfficiencyV->SetFrameFillColor(10);
      m_legEmpty->Draw("same");
    }
  }

}

void DQMHistAnalysisSVDEfficiencyModule::endRun()
{
  B2INFO("DQMHistAnalysisSVDEfficiency:  endRun called");
  if (m_printCanvas) {
    m_cEfficiencyU->Print("c_SVDEfficiencyU.pdf");
    m_cEfficiencyV->Print("c_SVDEfficiencyV.pdf");
    m_cEfficiencyErrU->Print("c_SVDEfficiencyErrU.pdf");
    m_cEfficiencyErrU->Print("c_SVDEfficiencyErrV.pdf");
  }
}

void DQMHistAnalysisSVDEfficiencyModule::terminate()
{
  B2INFO("DQMHistAnalysisSVDEfficiency: terminate called");

  delete m_refFile;
  delete m_legProblem;
  delete m_legWarning;
  delete m_legNormal;
  delete m_legEmpty;
  delete m_hEfficiency;
  delete m_cEfficiencyU;
  delete m_cEfficiencyV;
  delete m_hEfficiencyErr;
  delete m_cEfficiencyErrU;
  delete m_cEfficiencyErrV;
}
