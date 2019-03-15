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
  // This module CAN NOT be run in parallel!

  //Parameter definition
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of Histogram dir", std::string("pxd"));
  addParam("RangeLow", m_rangeLow, "Lower boarder for fit", 30.);
  addParam("RangeHigh", m_rangeHigh, "High border for fit", 85.);
  addParam("PVName", m_pvPrefix, "PV Prefix", std::string("DQM:PXD:Charge:"));
  B2DEBUG(99, "DQMHistAnalysisPXDCharge: Constructor done.");
}

void DQMHistAnalysisPXDChargeModule::initialize()
{
  B2DEBUG(99, "DQMHistAnalysisPXDCharge: initialized.");

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

  m_fLandau = new TF1("f_Landau", "landau", m_rangeLow, m_rangeHigh);
  m_fLandau->SetParameter(0, 1000);
  m_fLandau->SetParameter(1, 50);
  m_fLandau->SetParameter(2, 10);
  m_fLandau->SetLineColor(4);
  m_fLandau->SetNpx(60);
  m_fLandau->SetNumberFitPoints(60);

  m_fMean = new TF1("f_Mean", "pol0", 0.5, m_PXDModules.size() - 0.5);
  m_fMean->SetParameter(0, 50);
  m_fMean->SetLineColor(5);
  m_fMean->SetNpx(m_PXDModules.size());
  m_fMean->SetNumberFitPoints(m_PXDModules.size());

#ifdef _BELLE2_EPICS
  SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
  SEVCHK(ca_create_channel((m_pvPrefix + "Mean").data(), NULL, NULL, 10, &mychid[0]), "ca_create_channel failure");
  SEVCHK(ca_create_channel((m_pvPrefix + "Diff").data(), NULL, NULL, 10, &mychid[1]), "ca_create_channel failure");
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
#endif
}


void DQMHistAnalysisPXDChargeModule::beginRun()
{
  B2DEBUG(99, "DQMHistAnalysisPXDCharge: beginRun called.");

  m_cCharge->Clear();
}

void DQMHistAnalysisPXDChargeModule::event()
{
//   double data = 0.0;
  if (!m_cCharge) return;
  m_hCharge->Reset(); // dont sum up!!!

  bool enough = false;

  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    std::string name = "DQMER_PXD_" + (std::string)m_PXDModules[i ] + "_ClusterCharge";;
    std::replace(name.begin(), name.end(), '.', '_');

    TH1* hh1 = findHist(name);
    if (hh1 == NULL) {
      hh1 = findHist(m_histogramDirectoryName, name);
    }
    if (hh1) {
//       B2INFO("Histo " << name << " found in mem");
      /// FIXME Replace by a nice fit
      m_fLandau->SetParameter(0, 1000);
      m_fLandau->SetParameter(1, 50);
      m_fLandau->SetParameter(2, 10);
      hh1->Fit(m_fLandau, "R");
      m_hCharge->SetBinContent(i + 1, m_fLandau->GetParameter(1));
      m_hCharge->SetBinError(i + 1, m_fLandau->GetParError(1));
      // m_hCharge->SetBinError(i + 1, m_fLandau->GetParameter(2) * 0.25); // arbitrary scaling
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
    double data = 0;
    double diff = 0;
    double currentMin, currentMax;
    m_hCharge->Draw("");
//     m_line1->Draw();
//     m_line2->Draw();
//     m_line3->Draw();
    m_hCharge->Fit(m_fMean, "R");
    data = m_fMean->GetParameter(0); // we are more interessted in the maximum deviation from mean
    m_hCharge->GetMinimumAndMaximum(currentMin, currentMax);
    diff = fabs(data - currentMin) > fabs(currentMax - data) ? fabs(data - currentMin) : fabs(currentMax - data);
    if (0) B2INFO("Mean: " << data << " Max Diff: " << diff);

#ifdef _BELLE2_EPICS
    SEVCHK(ca_put(DBR_DOUBLE, mychid[0], (void*)&data), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[1], (void*)&diff), "ca_set failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
#endif
  }

  m_cCharge->Modified();
  m_cCharge->Update();
}

void DQMHistAnalysisPXDChargeModule::endRun()
{
  B2DEBUG(99, "DQMHistAnalysisPXDCharge : endRun called");
}


void DQMHistAnalysisPXDChargeModule::terminate()
{
  // should delete canvas here, maybe hist, too? Who owns it?
#ifdef _BELLE2_EPICS
  SEVCHK(ca_clear_channel(mychid[0]), "ca_clear_channel failure");
  SEVCHK(ca_clear_channel(mychid[1]), "ca_clear_channel failure");
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  ca_context_destroy();
#endif
  B2DEBUG(99, "DQMHistAnalysisPXDCharge: terminate called");
}

