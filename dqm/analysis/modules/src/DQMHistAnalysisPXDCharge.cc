//+
// File : DQMHistAnalysisPXDCharge.cc
// Description : Analysis of PXD Cluster Charge
//
// Author : Bjoern Spruck, University Mainz
// Date : 2018
//-


#include <dqm/analysis/modules/DQMHistAnalysisPXDCharge.h>
#include <TROOT.h>
#include <TLatex.h>
#include <vxd/geometry/GeoCache.h>

using namespace std;
using namespace Belle2;

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
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of Histogram dir", std::string("PXDER"));
  addParam("RangeLow", m_rangeLow, "Lower boarder for fit", 30.);
  addParam("RangeHigh", m_rangeHigh, "High border for fit", 85.);
  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("DQM:PXD:Charge:"));
  addParam("useEpics", m_useEpics, "useEpics", true);
  B2DEBUG(99, "DQMHistAnalysisPXDCharge: Constructor done.");
}

DQMHistAnalysisPXDChargeModule::~DQMHistAnalysisPXDChargeModule()
{
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (ca_current_context()) ca_context_destroy();
  }
#endif
}

void DQMHistAnalysisPXDChargeModule::initialize()
{
  B2DEBUG(99, "DQMHistAnalysisPXDCharge: initialized.");

  m_monObj = getMonitoringObject("pxd");
  const VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  //collect the list of all PXD Modules in the geometry here
  std::vector<VxdID> sensors = geo.getListOfSensors();
  for (VxdID& aVxdID : sensors) {
    VXD::SensorInfoBase info = geo.getSensorInfo(aVxdID);
    if (info.getType() != VXD::SensorInfoBase::PXD) continue;
    m_PXDModules.push_back(aVxdID); // reorder, sort would be better
  }
  std::sort(m_PXDModules.begin(), m_PXDModules.end());  // back to natural order

  gROOT->cd(); // this seems to be important, or strange things happen

  m_cCharge = new TCanvas((m_histogramDirectoryName + "/c_Charge").data());
  m_hCharge = new TH1F("Cluster Charge", "Cluster Charge; Module; Track Cluster Charge", m_PXDModules.size(), 0, m_PXDModules.size());
  m_hCharge->SetDirectory(0);// dont mess with it, this is MY histogram
  m_hCharge->SetStats(false);
  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    TString ModuleName = (std::string)m_PXDModules[i];
    m_hCharge->GetXaxis()->SetBinLabel(i + 1, ModuleName);
  }
  //Unfortunately this only changes the labels, but can't fill the bins by the VxdIDs
  m_hCharge->Draw("");

  m_monObj->addCanvas(m_cCharge);

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
  if (m_useEpics) {
    if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
    mychid.resize(3);
    SEVCHK(ca_create_channel((m_pvPrefix + "Mean").data(), NULL, NULL, 10, &mychid[0]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "Diff").data(), NULL, NULL, 10, &mychid[1]), "ca_create_channel failure");
    SEVCHK(ca_create_channel((m_pvPrefix + "Status").data(), NULL, NULL, 10, &mychid[2]), "ca_create_channel failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
}


void DQMHistAnalysisPXDChargeModule::beginRun()
{
  B2DEBUG(99, "DQMHistAnalysisPXDCharge: beginRun called.");

  m_cCharge->Clear();
}

void DQMHistAnalysisPXDChargeModule::event()
{
  if (!m_cCharge) return;
  m_hCharge->Reset(); // dont sum up!!!

  bool enough = false;

  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    std::string name = "DQMER_PXD_" + (std::string)m_PXDModules[i] + "_ClusterCharge";
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
      m_fLandau->SetParLimits(1, 10, 80);
      m_fLandau->SetParameter(2, 10);
      m_fLandau->SetParLimits(2, 1, 50);
      if (hh1->GetEntries() > 100) {
        hh1->Fit(m_fLandau, "R");
        m_hCharge->SetBinContent(i + 1, m_fLandau->GetParameter(1));
        m_hCharge->SetBinError(i + 1, m_fLandau->GetParError(1));
      } else {
        m_hCharge->SetBinContent(i + 1, 50);
        m_hCharge->SetBinError(i + 1, 100);
      }
      // m_hCharge->SetBinError(i + 1, m_fLandau->GetParameter(2) * 0.25); // arbitrary scaling
      // cout << m_fLandau->GetParameter(0) << " " << m_fLandau->GetParameter(1) << " " << m_fLandau->GetParameter(2) << endl;

      // m_hCharge->Fill(i, hh1->GetMean());
      m_cCharge->cd();
      hh1->Draw();
      if (hh1->GetEntries() > 100) m_fLandau->Draw("same");
//      m_cCharge->Print(str(format("cc_%d.pdf") % i).data());

      if (hh1->GetEntries() > 1000) enough = true;
    }
  }
  m_cCharge->cd();

  double data = 0;
  double diff = 0;
  if (m_hCharge && enough) {
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
  }

#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    SEVCHK(ca_put(DBR_DOUBLE, mychid[0], (void*)&data), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[1], (void*)&diff), "ca_set failure");
  }
#endif

  int status = 0;

  if (!enough) {
    // not enough Entries
    m_cCharge->Pad()->SetFillColor(kGray);// Magenta or Gray
    status = 0; // default
  } else {
    /// FIXME: what is the accpetable limit?
    if (fabs(data - 50.) > 20. || diff > 30) {
      m_cCharge->Pad()->SetFillColor(kRed);// Red
      status = 4;
    } else if (fabs(data - 50) > 15. || diff > 10) {
      m_cCharge->Pad()->SetFillColor(kYellow);// Yellow
      status = 3;
    } else {
      m_cCharge->Pad()->SetFillColor(kGreen);// Green
      status = 2;
    }

    // FIXME overwrite for now
    m_cCharge->Pad()->SetFillColor(kGreen);// Green
  }

#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    SEVCHK(ca_put(DBR_INT, mychid[2], (void*)&status), "ca_set failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif

  auto tt = new TLatex(5.5, 0, "1.3.2 Module is excluded, please ignore");
  tt->SetTextAngle(90);// Rotated
  tt->SetTextAlign(12);// Centered
  tt->Draw();

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
  if (m_useEpics) {
    for (auto m : mychid) SEVCHK(ca_clear_channel(m), "ca_clear_channel failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
  B2DEBUG(99, "DQMHistAnalysisPXDCharge: terminate called");
}

