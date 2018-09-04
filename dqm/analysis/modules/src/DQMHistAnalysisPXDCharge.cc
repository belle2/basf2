//+
// File : DQMHistAnalysisPXDCharge.cc
// Description : Analysis of PXD Cluster Charge
//
// Author : Bjoern Spruck, Univerisity Mainz
// Date : 2018
//-


#include <dqm/analysis/modules/DQMHistAnalysisPXDCharge.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TClass.h>
#include <vxd/geometry/GeoCache.h>

using namespace std;
using namespace Belle2;


using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisPXDCharge)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisPXDChargeModule::DQMHistAnalysisPXDChargeModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("HistoDir", m_histodir, "Name of Histogram dir", std::string("pxd"));
  B2DEBUG(1, "DQMHistAnalysisPXDCharge: Constructor done.");

}

void DQMHistAnalysisPXDChargeModule::initialize()
{
  B2DEBUG(1, "DQMHistAnalysisPXDCharge: initialized.");

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

  m_cCharge = new TCanvas("c_Charge");
  m_hCharge = new TH1F("Cluster Charge", "Cluster Charge; Module; Cluster Charge", m_PXDModules.size(), 0, m_PXDModules.size());
  m_hCharge->SetDirectory(0);// dont mess with it, this is MY histogram
  m_hCharge->SetStats(false);
  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    TString ModuleName = (std::string)m_PXDModules[i];
    m_hCharge->GetXaxis()->SetBinLabel(i + 1, ModuleName);
  }
  //Unfortunately this only changes the labels, but can't fill the bins by the VxdIDs
  m_hCharge->Draw("");

  /// FIXME were to put the lines depends ...
//   m_line1 = new TLine(0, 10, m_PXDModules.size(), 10);
//   m_line2 = new TLine(0, 16, m_PXDModules.size(), 16);
//   m_line3 = new TLine(0, 3, m_PXDModules.size(), 3);
//   m_line1->SetHorizontal(true);
//   m_line1->SetLineColor(3);// Green
//   m_line1->SetLineWidth(3);
//   m_line2->SetHorizontal(true);
//   m_line2->SetLineColor(1);// Black
//   m_line2->SetLineWidth(3);
//   m_line3->SetHorizontal(true);
//   m_line3->SetLineColor(1);
//   m_line3->SetLineWidth(3);

  /// FIXME : define the range, because its not a landau but now its hard to notice what happens outside!

  m_fLandau = new TF1("f_Landau", "landau", 25, 85);
  m_fLandau->SetParameter(0, 1000);
  m_fLandau->SetParameter(1, 50);
  m_fLandau->SetParameter(2, 10);
  m_fLandau->SetLineColor(4);
  m_fLandau->SetNpx(60);
  m_fLandau->SetNumberFitPoints(60);

}


void DQMHistAnalysisPXDChargeModule::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisPXDCharge: beginRun called.");

  m_cCharge->Clear();
}


TH1* DQMHistAnalysisPXDChargeModule::findHistLocal(TString& a)
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

void DQMHistAnalysisPXDChargeModule::event()
{
//   double data = 0.0;
  if (!m_cCharge) return;
  m_hCharge->Reset(); // dont sum up!!!

  bool enough = false;

  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    VxdID& aModule = m_PXDModules[i ];

    TString buff = (std::string)aModule;
    buff.ReplaceAll(".", "_");

    TH1* hh1 = NULL;

    TString a = "DQMER_PXD_" + buff + "_ClusterCharge";
    hh1 = findHist(a.Data());
    if (hh1 == NULL) {
      hh1 = findHistLocal(a);
    }
    if (hh1 == NULL) {
      a = m_histodir + "/DQMER_PXD_" + buff + "_ClusterCharge";
      hh1 = findHist(a.Data());
    }
    if (hh1 == NULL) {
      a = m_histodir + "/DQMER_PXD_" + buff + "_ClusterCharge";
      hh1 = findHistLocal(a);
    }
    if (hh1) {
      B2INFO("Histo " << a << " found in mem");
      /// FIXME Replave by a nice fit
      m_fLandau->SetParameter(0, 1000);
      m_fLandau->SetParameter(1, 50);
      m_fLandau->SetParameter(2, 10);
      hh1->Fit(m_fLandau, "R");
      m_hCharge->SetBinContent(i + 1, m_fLandau->GetParameter(1));
      m_hCharge->SetBinError(i + 1, m_fLandau->GetParameter(2) * 0.25); // arbitrary scaling
      // cout << m_fLandau->GetParameter(0) << " " << m_fLandau->GetParameter(1) << " " << m_fLandau->GetParameter(2) << endl;

      // m_hCharge->Fill(i, hh1->GetMean());
      m_cCharge->cd();
      hh1->Draw();
      m_fLandau->Draw("same");
      m_cCharge->Print(str(format("cc_%d.pdf") % i).data());

      if (hh1->GetEntries() > 1000) enough = true;
    }
  }
  m_cCharge->cd();


  // not enough Entries
  if (!enough) {
    m_cCharge->Pad()->SetFillColor(6);// Magenta
  } else {
//   B2INFO("data "<<data);
    /// FIXME: absolute numbers or relative numbers and what is the laccpetable limit?
//   if (data > 100.) {
//     m_cCharge->Pad()->SetFillColor(2);// Red
//   } else if (data > 50.) {
//     m_cCharge->Pad()->SetFillColor(5);// Yellow
//   } else {
    m_cCharge->Pad()->SetFillColor(0);// White
  }

  if (m_hCharge) {
    m_hCharge->Draw("");
//     m_line1->Draw();
//     m_line2->Draw();
//     m_line3->Draw();
  }

  m_cCharge->Modified();
  m_cCharge->Update();
}

void DQMHistAnalysisPXDChargeModule::endRun()
{
  B2DEBUG(1, "DQMHistAnalysisPXDCharge : endRun called");
}


void DQMHistAnalysisPXDChargeModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisPXDCharge: terminate called");
  m_cCharge->Print("c1.pdf");
  // should delete canvas here, maybe hist, too? Who owns it?
}

