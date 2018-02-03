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

  //Would be much more elegant to get bin numbers from the saved histograms, but would need to retrieve at least one of them before the initialize function for this
  //Or get one and clone it
  addParam("binsU", m_u_bins, "histogram bins in u direction, needs to be the same as in PXDDQMEfficiency", int(4));
  addParam("binsV", m_v_bins, "histogram bins in v direction, needs to be the same as in PXDDQMEfficiency", int(6));
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms were placed",
           std::string("pxdeff"));
  addParam("singleHists", m_singleHists, "Also plot one efficiency histogram per module", bool(false));

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

    if (aVxdID.getLayerNumber() == 1) {
      m_PXDLayer1.push_back(aVxdID);
    } else if (aVxdID.getLayerNumber() == 2) {
      m_PXDLayer2.push_back(aVxdID);
    } else {
      B2ERROR("Layer number " << aVxdID.getLayerNumber() << " given for a PXD Module!");
      continue;
    }


  }

  gROOT->cd(); // this seems to be important, or strange things happen



  int nu = 1;//If this does not get overwritten, the histograms will anyway never contain anything useful
  int nv = 1;
  //Have been promised that all modules have the same number of pixels, so just take from the first one
  if (m_PXDModules.size() == 0) {
    //This could as well be a B2FATAL, the module won't do anything useful if this happens
    B2ERROR("No PXDModules found! Can't really do anything useful now...");
  } else {
    VXD::SensorInfoBase cellGetInfo = m_vxdGeometry.getSensorInfo(m_PXDModules[0]);
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
                                         m_u_bins, -0.5, nu + 0.5, m_v_bins, -0.5, nv + 0.5);
    m_hEffModules[aPXDModule]->SetStats(false);
    m_hEffModules[aPXDModule]->GetZaxis()->SetLimits(0, 1);

    //todo is this needed for histograms created here?
    //m_hEffModules[aPXDModule]->SetDirectory(0);

  }

  m_hEffMerge["IF"] = new TH2D("HitEff_IF", "Average Hit Efficiency of IF Modules;Pixel in U;Pixel in V",
                               m_u_bins, -0.5, nu + 0.5, m_v_bins, -0.5, nv + 0.5);
  m_hEffMerge["IB"] = new TH2D("HitEff_IB", "Average Hit Efficiency of IB Modules;Pixel in U;Pixel in V",
                               m_u_bins, -0.5, nu + 0.5, m_v_bins, -0.5, nv + 0.5);
  m_hEffMerge["OF"] = new TH2D("HitEff_OF", "Average Hit Efficiency of OF Modules;Pixel in U;Pixel in V",
                               m_u_bins, -0.5, nu + 0.5, m_v_bins, -0.5, nv + 0.5);
  m_hEffMerge["OB"] = new TH2D("HitEff_OB", "Average Hit Efficiency of OB Modules;Pixels in U;Pixels in V",
                               m_u_bins, -0.5, nu + 0.5, m_v_bins, -0.5, nv + 0.5);
  m_cEffMerge["IF"] = new TCanvas("c_EffIF");
  m_cEffMerge["IB"] = new TCanvas("c_EffIB");
  m_cEffMerge["OF"] = new TCanvas("c_EffOF");
  m_cEffMerge["OB"] = new TCanvas("c_EffOB");

  m_hEffMerge["IF"]->SetStats(false);
  m_hEffMerge["IB"]->SetStats(false);
  m_hEffMerge["OF"]->SetStats(false);
  m_hEffMerge["OB"]->SetStats(false);


  //One bin for each module in the geometry, one histogram for each layer
  m_cEffAll1 = new TCanvas("c_EffAll1");
  m_cEffAll2 = new TCanvas("c_EffAll2");

  m_hEffAll1 = new TH1D("HitEffAll1", "Integrated Efficiency of each layer 1 module;PXD Module;",
                        m_PXDLayer1.size(), 0, m_PXDLayer1.size());
  m_hEffAll2 = new TH1D("HitEffAll2", "Integrated Efficiency of each layer 2 module;PXD Module;",
                        m_PXDLayer2.size(), 0, m_PXDLayer2.size());

  m_hEffAll1->SetStats(false);
  m_hEffAll2->SetStats(false);
  m_hEffAll1->GetYaxis()->SetRangeUser(0, 1);
  m_hEffAll2->GetYaxis()->SetRangeUser(0, 1);

  for (unsigned int i = 1; i <= m_PXDLayer1.size(); i++) {
    TString ModuleName = (std::string)m_PXDLayer1[i - 1];
    m_hEffAll1->GetXaxis()->SetBinLabel(i, ModuleName);
  }
  for (unsigned int i = 1; i <= m_PXDLayer2.size(); i++) {
    TString ModuleName = (std::string)m_PXDLayer2[i - 1];
    m_hEffAll2->GetXaxis()->SetBinLabel(i, ModuleName);
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

  m_cEffAll1->Clear();
  m_cEffAll2->Clear();

  for (auto  single_cmap : m_cEffModules) {
    single_cmap.second->Clear();
  }
}


void DQMHistAnalysisPXDEffModule::event()
{

  //Save the pointers to create the summary hists later
  std::map<VxdID, TH2D*> mapHits;
  std::map<VxdID, TH2D*> mapMatches;

  //Count how much of each type of histogram there is for the averaging
  std::map<std::string, int> typeCounter;


  for (unsigned int i = 1; i <= m_PXDModules.size(); i++) {
    VxdID& aPXDModule = m_PXDModules[i - 1];

    TString buff = (std::string)aPXDModule;
    buff.ReplaceAll(".", "_");

    //Summing up events is done in the module creating these
    m_hEffModules[aPXDModule]->Reset();

    TH2D* Hits, *Matches;
    Hits = (TH2D*)findHist(m_histogramDirectoryName, std::string("track_hits_" + buff));
    Matches = (TH2D*)findHist(m_histogramDirectoryName, std::string("matched_cluster_" + buff));

    //Finding only one of them should only happen in very strange situations...
    if (Hits == nullptr) {
      TString locationHits = "track_hits_" + buff;
      if (m_histogramDirectoryName != "") {
        locationHits = m_histogramDirectoryName + "/" + locationHits;
      }
      Hits = (TH2D*)findHistLocal(locationHits);
    }
    if (Matches == nullptr)    {
      TString locationMatches = "matched_cluster_" + buff;
      if (m_histogramDirectoryName != "") {
        locationMatches = m_histogramDirectoryName + "/" + locationMatches;
      }
      Matches = (TH2D*)findHistLocal(locationMatches);
    }

    if (Hits == nullptr || Matches == nullptr) {
      B2ERROR("Missing histogram for sensor " << aPXDModule);
      mapHits[aPXDModule] = nullptr;
      mapMatches[aPXDModule] = nullptr;
    } else {
      mapHits[aPXDModule] = Hits;
      mapMatches[aPXDModule] = Matches;
      m_hEffModules[aPXDModule]->Divide(Matches, Hits);

      if (aPXDModule.getLayerNumber() == 1) {
        if (aPXDModule.getSensorNumber() == 1) {
          m_hEffMerge["IF"]->Add(m_hEffModules[aPXDModule]);
          typeCounter["IF"]++;
        } else if (aPXDModule.getSensorNumber() == 2) {
          m_hEffMerge["IB"]->Add(m_hEffModules[aPXDModule]);
          typeCounter["IB"]++;
        } else {
          B2ERROR("Module " << aPXDModule << " currently not supported for averaged histograms, will be skipped.");
        }
      } else if (aPXDModule.getLayerNumber() == 2) {
        if (aPXDModule.getSensorNumber() == 1) {
          m_hEffMerge["OF"]->Add(m_hEffModules[aPXDModule]);
          typeCounter["OF"]++;
        } else if (aPXDModule.getSensorNumber() == 2) {
          m_hEffMerge["OB"]->Add(m_hEffModules[aPXDModule]);
          typeCounter["OB"]++;
        } else {
          B2ERROR("Module " << aPXDModule << " currently not supported for averaged histograms, will be skipped.");
        }
      }
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
  m_hEffAll1->Reset();
  for (unsigned int i = 1; i <= m_PXDLayer1.size(); i++) {
    VxdID& aL1Module = m_PXDLayer1[i - 1];

    if (mapHits[aL1Module] == nullptr || mapMatches[aL1Module] == nullptr) {
      m_hEffAll1->SetBinContent(i, 0);
    } else {
      double moduleAverage = 0;
      if (mapHits[aL1Module]->Integral() != 0) {
        moduleAverage = mapMatches[aL1Module]->Integral() / mapHits[aL1Module]->Integral();
      }
      m_hEffAll1->SetBinContent(i, moduleAverage);
    }

    m_cEffAll1->cd();
    m_hEffAll1->Draw();
    m_cEffAll1->Modified();
    m_cEffAll1->Update();
  }

  //This code duplication is rather ugly, there should be a better way
  m_hEffAll2->Reset();
  for (unsigned int i = 1; i <= m_PXDLayer2.size(); i++) {
    VxdID& aL2Module = m_PXDLayer2[i - 1];

    if (mapHits[aL2Module] == nullptr || mapMatches[aL2Module] == nullptr) {
      m_hEffAll2->SetBinContent(i, 0);
    } else {
      double moduleAverage = 0;
      if (mapHits[aL2Module]->Integral() != 0) {
        moduleAverage = mapMatches[aL2Module]->Integral() / mapHits[aL2Module]->Integral();
      }
      m_hEffAll2->SetBinContent(i, moduleAverage);
    }
    m_cEffAll2->cd();
    m_hEffAll2->Draw();
    m_cEffAll2->Modified();
    m_cEffAll2->Update();
  }

}

void DQMHistAnalysisPXDEffModule::endRun()
{
  B2DEBUG(1, "DQMHistAnalysisPXDEff : endRun called");
}


void DQMHistAnalysisPXDEffModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisPXDEff: terminate called");
}

TH1* DQMHistAnalysisPXDEffModule::findHistLocal(TString& a)
{
  B2INFO("Histo " << a << " not in memfile");
  // the following code sux ... is there no root function for that?
  TDirectory* d = gROOT;

  TString myl = a;
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
      B2INFO("Histo " << a << " found in mem");
      return (TH1*)obj;
    }
  } else {
    B2INFO("Histo " << a << " NOT found in mem");
  }
  return NULL;
}

