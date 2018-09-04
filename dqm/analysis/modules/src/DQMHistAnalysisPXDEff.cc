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
  //Parameter definition

  //Would be much more elegant to get bin numbers from the saved histograms, but would need to retrieve at least one of them before the initialize function for this
  //Or get one and clone it
  addParam("binsU", m_u_bins, "histogram bins in u direction, needs to be the same as in PXDDQMEfficiency", int(4));
  addParam("binsV", m_v_bins, "histogram bins in v direction, needs to be the same as in PXDDQMEfficiency", int(6));
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms were placed",
           std::string("pxdeff"));
  addParam("singleHists", m_singleHists, "Also plot one efficiency histogram per module", bool(false));

  B2DEBUG(1, "DQMHistAnalysisPXDEff: Constructor done.");
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

  B2DEBUG(1, "DQMHistAnalysisPXDEff: initialized.");
}


void DQMHistAnalysisPXDEffModule::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisPXDEff: beginRun called.");

  for (auto merge_cmap : m_cEffMerge) {
    merge_cmap.second->Clear();
  }

  m_cEffAll->Clear();

  for (auto single_cmap : m_cEffModules) {
    single_cmap.second->Clear();
  }
}

TH1* DQMHistAnalysisPXDEffModule::GetHisto(TString histoname)
{
  TH1* hh1;
  gROOT->cd();
  // look in memfile
  hh1 = findHist(histoname.Data());
  if (hh1) return hh1; // found in memfile

  B2INFO("Histo " << histoname << " not in memfile");
  // the following code sux ... is there no root function for that?
  TDirectory* d = gROOT;

  TString myl = histoname;
  TString tok;
  Ssiz_t from = 0;
  while (myl.Tokenize(tok, from, "/")) {
    TString dummy;
    Ssiz_t f;
    f = from;
    if (myl.Tokenize(dummy, f, "/")) { // check if its the last one
      auto e = d->GetDirectory(tok);
      if (e) {
        B2INFO("Cd Dir " << tok);
        d = e;
      }
      d->cd();
    } else {
      break;
    }
  }
  TObject* obj = d->FindObject(tok);
  if (obj != NULL) {
    if (obj->IsA()->InheritsFrom("TH1")) {
      B2INFO("Histo " << histoname << " found in mem");
      return (TH1*)obj;
    }
  } else {
    B2INFO("Histo " << histoname << " NOT found in mem");
  }
  return NULL;
}

void DQMHistAnalysisPXDEffModule::event()
{

  //Save the pointers to create the summary hists later
  std::map<VxdID, TH2D*> mapHits;
  std::map<VxdID, TH2D*> mapMatches;

  //Count how many of each type of histogram there are for the averaging
  std::map<std::string, int> typeCounter;

  //Reset the histograms for new averaging
  for (auto mergers : m_hEffMerge) {
    mergers.second->Reset();
  }

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
    Hits = (TH2D*)GetHisto(locationHits);
    TString locationMatches = "matched_cluster_" + buff;
    if (m_histogramDirectoryName != "") {
      locationMatches = m_histogramDirectoryName + "/" + locationMatches;
    }
    Matches = (TH2D*)GetHisto(locationMatches);

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

  //Plotting the average sensor type histograms
  for (auto mergers : m_hEffMerge) {
    m_cEffMerge[mergers.first]->cd();
    mergers.second->Scale(1. / typeCounter[mergers.first]);
    mergers.second->Draw("colz");
    m_cEffMerge[mergers.first]->Modified();
    m_cEffMerge[mergers.first]->Update();

  }


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

}

void DQMHistAnalysisPXDEffModule::endRun()
{
  B2DEBUG(1, "DQMHistAnalysisPXDEff : endRun called");
}


void DQMHistAnalysisPXDEffModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisPXDEff: terminate called");
}

