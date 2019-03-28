//+
// File : DQMHistAnalysisPXDEff.cc
// Description : DQM module, which gives histograms showing the efficiency of PXD sensors
//
// Author : Uwe Gebauer
// based on work from B. Spruck
// based on work from Tomoyuki Konno, Tokyo Metropolitan Univerisity
// Date : someday
//-


#include <dqm/analysis/modules/DQMHistAnalysisPXDEff.h>
#include <TROOT.h>
#include <TClass.h>
#include <vxd/geometry/GeoCache.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisPXDEff)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisPXDEffModule::DQMHistAnalysisPXDEffModule() : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!

  //Parameter definition

  //Would be much more elegant to get bin numbers from the saved histograms, but would need to retrieve at least one of them before the initialize function for this
  //Or get one and clone it
  addParam("binsU", m_u_bins, "histogram bins in u direction, needs to be the same as in PXDDQMEfficiency", int(4));
  addParam("binsV", m_v_bins, "histogram bins in v direction, needs to be the same as in PXDDQMEfficiency", int(6));
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms were placed",
           std::string("pxdeff"));
  addParam("singleHists", m_singleHists, "Also plot one efficiency histogram per module", bool(false));
  addParam("PVName", m_pvPrefix, "PV Prefix", std::string("DQM:PXD:Eff"));
  B2DEBUG(1, "DQMHistAnalysisPXDEff: Constructor done.");
}

DQMHistAnalysisPXDEffModule::~DQMHistAnalysisPXDEffModule()
{
#ifdef _BELLE2_EPICS
  if (ca_current_context()) ca_context_destroy();
#endif
}

void DQMHistAnalysisPXDEffModule::initialize()
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  //collect the list of all PXD Modules in the geometry here
  std::vector<VxdID> sensors = geo.getListOfSensors();
  for (VxdID& aVxdID : sensors) {
    VXD::SensorInfoBase info = geo.getSensorInfo(aVxdID);
    // B2DEBUG(20,"VXD " << aVxdID);
    if (info.getType() != VXD::SensorInfoBase::PXD) continue;
    m_PXDModules.push_back(aVxdID); // reorder, sort would be better

  }
  std::sort(m_PXDModules.begin(), m_PXDModules.end());  // back to natural order

  gROOT->cd(); // this seems to be important, or strange things happen



  int nu = 1;//If this does not get overwritten, the histograms will anyway never contain anything useful
  int nv = 1;
  //Have been promised that all modules have the same number of pixels, so just take from the first one
  if (m_PXDModules.size() == 0) {
    //This could as well be a B2FATAL, the module won't do anything useful if this happens
    B2ERROR("No PXDModules found! Can't really do anything useful now...");
    // set some default size to nu, nv?
  } else {
    VXD::SensorInfoBase cellGetInfo = geo.getSensorInfo(m_PXDModules[0]);
    nu = cellGetInfo.getUCells();
    nv = cellGetInfo.getVCells();
  }

  for (VxdID& aPXDModule : m_PXDModules) {
    TString buff = (std::string)aPXDModule;
    buff.ReplaceAll(".", "_");
    TString histTitle = "Hit Efficiency on Module " + (std::string)aPXDModule + ";Pixel in U;Pixel in V";
    if (m_singleHists) {
      m_cEffModules[aPXDModule] = new TCanvas("c_Eff_" + buff);
    }
    m_hEffModules[aPXDModule] = new TH2D("HitEff_" + buff, histTitle,
                                         m_u_bins, -0.5, nu - 0.5, m_v_bins, -0.5, nv - 0.5);
    m_hEffModules[aPXDModule]->SetStats(false);
  }

  //One bin for each module in the geometry, one histogram for each layer
  m_cEffAll = new TCanvas("c_EffAll");

  m_hEffAll = new TH1D("HitEffAll", "Integrated Efficiency of each module;PXD Module;",
                       m_PXDModules.size(), 0, m_PXDModules.size());

  m_hEffAll->SetStats(false);
  m_hEffAll->GetYaxis()->SetRangeUser(0, 1.05);

  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    TString ModuleName = (std::string)m_PXDModules[i];
    m_hEffAll->GetXaxis()->SetBinLabel(i + 1, ModuleName);
  }
  //Unfortunately this only changes the labels, but can't fill the bins by the VxdIDs

#ifdef _BELLE2_EPICS
  if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
  SEVCHK(ca_create_channel(m_pvPrefix.data(), NULL, NULL, 10, &mychid), "ca_create_channel failure");
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
#endif
  B2DEBUG(1, "DQMHistAnalysisPXDEff: initialized.");
}


void DQMHistAnalysisPXDEffModule::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisPXDEff: beginRun called.");

  m_cEffAll->Clear();

  for (auto single_cmap : m_cEffModules) {
    single_cmap.second->Clear();
  }
}


void DQMHistAnalysisPXDEffModule::event()
{

  //Save the pointers to create the summary hists later
  std::map<VxdID, TH2D*> mapHits;
  std::map<VxdID, TH2D*> mapMatches;

  //Count how many of each type of histogram there are for the averaging
  //std::map<std::string, int> typeCounter;

  for (unsigned int i = 1; i <= m_PXDModules.size(); i++) {
    VxdID& aPXDModule = m_PXDModules[i - 1];

    TString buff = (std::string)aPXDModule;
    buff.ReplaceAll(".", "_");

    //Summing up events is done in the module creating these
    m_hEffModules[aPXDModule]->Reset();

    TH2D* Hits, *Matches;
    TString locationHits = "track_hits_" + buff;
    if (m_histogramDirectoryName != "") {
      locationHits = m_histogramDirectoryName + "/" + locationHits;
    }
    Hits = (TH2D*)findHist(locationHits.Data());
    TString locationMatches = "matched_cluster_" + buff;
    if (m_histogramDirectoryName != "") {
      locationMatches = m_histogramDirectoryName + "/" + locationMatches;
    }
    Matches = (TH2D*)findHist(locationMatches.Data());

    //Finding only one of them should only happen in very strange situations...
    if (Hits == nullptr || Matches == nullptr) {
      B2ERROR("Missing histogram for sensor " << aPXDModule);
      mapHits[aPXDModule] = nullptr;
      mapMatches[aPXDModule] = nullptr;
    } else {
      mapHits[aPXDModule] = Hits;
      mapMatches[aPXDModule] = Matches;
      m_hEffModules[aPXDModule]->Divide(Matches, Hits);
    }

    if (m_cEffModules[aPXDModule]) {
      m_cEffModules[aPXDModule]->cd();
      m_hEffModules[aPXDModule]->Draw("colz");
      m_cEffModules[aPXDModule]->Modified();
      m_cEffModules[aPXDModule]->Update();
    }
  }//One-Module histos finished

  //Fill both of the summary histograms
  m_hEffAll->Reset();
  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    VxdID& aModule = m_PXDModules[i ];
    int j = i + 1;

    if (mapHits[aModule] == nullptr || mapMatches[aModule] == nullptr) {
      m_hEffAll->SetBinContent(j, 0);
    } else {
      double moduleAverage = 0, moduleAverageErr = 0;
      if (mapHits[aModule]->Integral() != 0) {
        double i1, i2, di1, di2;
        i1 = mapMatches[aModule]->Integral();
        i2 = mapHits[aModule]->Integral();
        di1 = TMath::Sqrt(i1);
        di2 = TMath::Sqrt(i2);
        moduleAverage = i1 / i2;
        moduleAverageErr = (i2 * di1 + i1 * di2) / (i2 * i2);
      }
      m_hEffAll->SetBinContent(j, moduleAverage);
      m_hEffAll->SetBinError(j, moduleAverageErr);
    }
  }

  m_cEffAll->cd();
  m_hEffAll->Draw("E");
  m_cEffAll->Modified();
  m_cEffAll->Update();

#ifdef _BELLE2_EPICS
  double data = 0;
  SEVCHK(ca_put(DBR_DOUBLE, mychid, (void*)&data), "ca_set failure");
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
#endif
}

void DQMHistAnalysisPXDEffModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisPXDEff: terminate called");
}

