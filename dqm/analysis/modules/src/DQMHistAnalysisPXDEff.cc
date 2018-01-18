//+
// File : DQMHistAnalysisPXDFits.cc
// Description : DQM module, which fits many PXD histograms and writes out fit parameters in new histograms
//
// Author : Uwe Gebauer
// based on work from B. Spruck
// based on work from Tomoyuki Konno, Tokyo Metropolitan Univerisity
// Date : someday
//-


#include <dqm/analysis/modules/DQMHistAnalysisPXDEff.h>
#include <TROOT.h>
#include <TClass.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisPXDEff)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisPXDEffModule::DQMHistAnalysisPXDEffModule() : DQMHistAnalysisModule(), m_vxdGeometry(VXD::GeoCache::getInstance())
{
  //Parameter definition

  //Would be much more elegant to get this from the saved histograms, but would need to get at least one of them before the initialize function for this
  //Or get one and clone it
  addParam("binsU", m_u_bins, "histogram bins in u direction", int(4));
  addParam("binsV", m_v_bins, "histogram bins in v direction", int(6));
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms were placed",
           std::string("pxdeff"));
  B2DEBUG(1, "DQMHistAnalysisPXDEff: Constructor done.");
}

DQMHistAnalysisPXDEffModule::~DQMHistAnalysisPXDEffModule() { }

void DQMHistAnalysisPXDEffModule::initialize()
{
  //collect the list of all PXD Modules in the geometry here
  std::vector<VxdID> sensors = m_vxdGeometry.getListOfSensors();
  for (VxdID& aVxdID : sensors) {
    VXD::SensorInfoBase info = m_vxdGeometry.getSensorInfo(aVxdID);
    if (info.getType() != VXD::SensorInfoBase::PXD) continue;
    m_PXDModules.push_back(aVxdID);
  }

  gROOT->cd(); // this seems to be important, or strange things happen


  //Have been promised that all modules have the same number of pixels, so just take from the first one
  VXD::SensorInfoBase cellGetInfo = m_vxdGeometry.getSensorInfo(m_PXDModules[0]);
  int nu = cellGetInfo.getUCells();
  int nv = cellGetInfo.getVCells();

  for (VxdID& aPXDModule : m_PXDModules) {
    TString buff = (std::string)aPXDModule;
    buff.ReplaceAll(".", "_");
    m_cEffModules[aPXDModule] = new TCanvas("c_Eff_" + buff);
    m_hEffModules[aPXDModule] = new TH2D("HitEff_" + buff, "Hit Efficiency on Module " + buff,
                                         m_u_bins, -0.5, nu + 0.5, m_v_bins, -0.5, nv + 0.5);
  }

  m_hEffIF = new TH2D("HitEff_IF", "Average Hit Efficiency of IF Modules",
                      m_u_bins, -0.5, nu + 0.5, m_v_bins, -0.5, nv + 0.5);
  m_hEffIB = new TH2D("HitEff_IB", "Average Hit Efficiency of IB Modules",
                      m_u_bins, -0.5, nu + 0.5, m_v_bins, -0.5, nv + 0.5);
  m_hEffOF = new TH2D("HitEff_OF", "Average Hit Efficiency of OF Modules",
                      m_u_bins, -0.5, nu + 0.5, m_v_bins, -0.5, nv + 0.5);
  m_hEffOB = new TH2D("HitEff_OB", "Average Hit Efficiency of OB Modules",
                      m_u_bins, -0.5, nu + 0.5, m_v_bins, -0.5, nv + 0.5);
  m_cEffIF = new TCanvas("c_EffIF");
  m_cEffIB = new TCanvas("c_EffIB");
  m_cEffOF = new TCanvas("c_EffOF");
  m_cEffOB = new TCanvas("c_EffOB");

  //One bin for each module in the geometry
  m_cEffAll = new TCanvas("c_EffAll");
  m_hEffAll = new TH1D("HitEffAll", "Integrated Efficiency of each module",
                       m_PXDModules.size(), 0, m_PXDModules.size());
  for (unsigned int i = 1; i <= m_PXDModules.size(); i++) {
    TString ModuleName = (std::string)m_PXDModules[i - 1];
    m_hEffAll->GetXaxis()->SetBinLabel(i, ModuleName);
  }
  //Unfortunately this only changes the labels, but can't fill the bins by the VxdIDs


  //todo need this??
  //m_hSignal[i]->SetDirectory(0);// dont mess with it, this is MY histogram

  B2DEBUG(1, "DQMHistAnalysisPXDEff: initialized.");
}


void DQMHistAnalysisPXDEffModule::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisPXDEff: beginRun called.");

  m_cEffIF->Clear();
  m_cEffIB->Clear();
  m_cEffOF->Clear();
  m_cEffOB->Clear();

  m_cEffAll->Clear();

  //todo make this dependant on selection if plots of individual modules are made?
  for (VxdID& aPXDModule : m_PXDModules) {
    m_cEffModules[aPXDModule]->Clear();
  }
}






void DQMHistAnalysisPXDEffModule::event()
{

  for (VxdID& aPXDModule : m_PXDModules) {
    TString buff = (std::string)aPXDModule;
    buff.ReplaceAll(".", "_");

    //Summing up events is done in the module creating these
    m_hEffModules[aPXDModule]->Reset();

    TH2D* Hits, *Matches;
    Hits = (TH2D*)findHist(m_histogramDirectoryName, std::string("track_hits_" + buff));
    Matches = (TH2D*)findHist(m_histogramDirectoryName, std::string("matched_cluster_" + buff));

    if (Hits == nullptr || Matches == nullptr) {
      B2ERROR("Missing histogram for sensor " << aPXDModule);
      //todo
      //What to do here? Fill with zeroes/ones to make averages work, or just throw a fatal and stop?
    }

    m_hEffModules[aPXDModule]->Divide(Matches, Hits);

    //todo this canvas always exists, don't create them to suppress output of the individual histos?
    if (m_cEffModules[aPXDModule]) {
      m_cEffModules[aPXDModule]->cd();
      m_hEffModules[aPXDModule]->Draw("colz");
      m_cEffModules[aPXDModule]->Modified();
      m_cEffModules[aPXDModule]->Update();
    }

  }

  //todo Here: fill the summary hist and the four averaged histos

}

void DQMHistAnalysisPXDEffModule::endRun()
{
  B2DEBUG(1, "DQMHistAnalysisPXDEff : endRun called");
}


void DQMHistAnalysisPXDEffModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisPXDEff: terminate called");
}
