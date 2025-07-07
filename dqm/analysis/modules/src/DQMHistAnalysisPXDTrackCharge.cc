/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisPXDTrackCharge.cc
// Description : Analysis of PXD Cluster Charge
//-

#include <dqm/analysis/modules/DQMHistAnalysisPXDTrackCharge.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TLatex.h>
#include <vxd/geometry/GeoCache.h>

#include <RooDataHist.h>
#include <RooAbsPdf.h>
#include <RooPlot.h>
#include <RooFitResult.h>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisPXDTrackCharge);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisPXDTrackChargeModule::DQMHistAnalysisPXDTrackChargeModule()
  : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!
  setDescription("DQM Analysis for PXD Track-Cluster Charge");

  // Parameter definition
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of Histogram dir", std::string("PXDER"));
  addParam("RangeLow", m_rangeLow, "Lower border for fit", 20.);
  addParam("RangeHigh", m_rangeHigh, "High border for fit", 80.);
//   addParam("PeakBefore", m_peakBefore, "Range for fit before peak (positive)", 5.);
//   addParam("PeakAfter", m_peakAfter, "Range for after peak", 40.);
  addParam("excluded", m_excluded, "excluded module (indizes starting from 0 to 39)", std::vector<int>());
  B2DEBUG(99, "DQMHistAnalysisPXDTrackCharge: Constructor done.");
}

DQMHistAnalysisPXDTrackChargeModule::~DQMHistAnalysisPXDTrackChargeModule()
{
}

void DQMHistAnalysisPXDTrackChargeModule::initialize()
{
  B2DEBUG(99, "DQMHistAnalysisPXDTrackCharge: initialized.");

  m_monObj = getMonitoringObject("pxd");
  const VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  m_rfws = new RooWorkspace("w");
  m_rfws->factory("Landau::landau(x[0,100],ml[20,10,50],sl[5,1,30])");
  m_rfws->factory("Gaussian::gauss(x,mg[0],sg[2,0.1,10])");
  m_rfws->factory("FCONV::lxg(x,landau,gauss)");

  m_x = m_rfws->var("x");
  m_x->setRange("signal", m_rangeLow, m_rangeHigh);

  // collect the list of all PXD Modules in the geometry here
  std::vector<VxdID> sensors = geo.getListOfSensors();
  for (VxdID& aVxdID : sensors) {
    VXD::SensorInfoBase info = geo.getSensorInfo(aVxdID);
    if (info.getType() != VXD::SensorInfoBase::PXD) continue;
    m_PXDModules.push_back(aVxdID); // reorder, sort would be better

    std::string name = "PXD_Track_Cluster_Charge_" + (std::string)aVxdID;
    std::replace(name.begin(), name.end(), '.', '_');
    m_cChargeMod[aVxdID] = new TCanvas((m_histogramDirectoryName + "/c_Fit_" + name).data());
    if (aVxdID == VxdID("1.5.1")) {
      for (int s = 0; s < 6; s++) {
        for (int d = 0; d < 4; d++) {
          m_cChargeModASIC[aVxdID][s][d] = new TCanvas((m_histogramDirectoryName + "/c_Fit_" + name + Form("_s%d_d%d", s + 1, d + 1)).data());
        }
      }
      m_hChargeModASIC2d[aVxdID] = new TH2F(("hPXD_TCChargeMPV_" + name).data(),
                                            ("PXD TCCharge MPV " + name + ";Switcher;DCD;MPV").data(),
                                            6, 0.5, 6.5, 4, 0.5, 4.5);
      m_cChargeModASIC2d[aVxdID] = new TCanvas((m_histogramDirectoryName + "/c_TCCharge_MPV_" + name).data());
    }
  }
  if (m_PXDModules.size() == 0) {
    // Backup if no geometry is present (testing...)
    B2WARNING("No PXDModules in Geometry found! Use hard-coded setup.");
    std::vector <string> mod = {
      "1.1.1", "1.1.2", "1.2.1", "1.2.2", "1.3.1", "1.3.2", "1.4.1", "1.4.2",
      "1.5.1", "1.5.2", "1.6.1", "1.6.2", "1.7.1", "1.7.2", "1.8.1", "1.8.2",
      "2.1.1", "2.1.2", "2.2.1", "2.2.2", "2.3.1", "2.3.2", "2.4.1", "2.4.2",
      "2.5.1", "2.5.2", "2.6.1", "2.6.2", "2.7.1", "2.7.2", "2.8.1", "2.8.2",
      "2.9.1", "2.9.2", "2.10.1", "2.10.2", "2.11.1", "2.11.2", "2.12.1", "2.12.2"
    };
    for (auto& it : mod) m_PXDModules.push_back(VxdID(it));
  }
  std::sort(m_PXDModules.begin(), m_PXDModules.end());  // back to natural order

  gROOT->cd(); // this seems to be important, or strange things happen

  m_cTrackedClusters = new TCanvas((m_histogramDirectoryName + "/c_TrackedClusters").data());
  m_hTrackedClusters = new TH1F("hPXDTrackedClusters", "PXD Tracked Clusters/Event;Module", 40, 0, 40);
  m_hTrackedClusters->Draw();
  auto ax = m_hTrackedClusters->GetXaxis();
  if (ax) {
    ax->Set(m_PXDModules.size(), 0, m_PXDModules.size());
    for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
      TString ModuleName = (std::string)m_PXDModules[i];
      ax->SetBinLabel(i + 1, ModuleName);
    }
  } else B2ERROR("no axis");

  m_cCharge = new TCanvas((m_histogramDirectoryName + "/c_TrackCharge").data());
  m_monObj->addCanvas(m_cCharge);

  m_gCharge = new TGraphErrors();
  m_gCharge->SetName("Track_Cluster_Charge");
  m_gCharge->SetTitle("Track Cluster Charge");

  /// FIXME were to put the lines depends ...
  m_line_up = new TLine(0, 10, m_PXDModules.size(), 10);
  m_line_mean = new TLine(0, 16, m_PXDModules.size(), 16);
  m_line_low = new TLine(0, 3, m_PXDModules.size(), 3);
  m_line_up->SetHorizontal(true);
  m_line_up->SetLineColor(kMagenta);// Green
  m_line_up->SetLineWidth(3);
  m_line_up->SetLineStyle(7);
  m_line_mean->SetHorizontal(true);
  m_line_mean->SetLineColor(kGreen);// Black
  m_line_mean->SetLineWidth(3);
  m_line_mean->SetLineStyle(4);
  m_line_low->SetHorizontal(true);
  m_line_low->SetLineColor(kMagenta);
  m_line_low->SetLineWidth(3);
  m_line_low->SetLineStyle(7);

  m_fMean = new TF1("f_Mean", "pol0", 0, m_PXDModules.size());
  m_fMean->SetParameter(0, 50);
  m_fMean->SetLineColor(kYellow);
  m_fMean->SetLineWidth(3);
  m_fMean->SetLineStyle(7);
  m_fMean->SetNpx(m_PXDModules.size());
  m_fMean->SetNumberFitPoints(m_PXDModules.size());

  registerEpicsPV("PXD:TrackCharge:Mean", "Mean");
  registerEpicsPV("PXD:TrackCharge:Diff", "Diff");
  registerEpicsPV("PXD:TrackCharge:Status", "Status");
}


void DQMHistAnalysisPXDTrackChargeModule::beginRun()
{
  B2DEBUG(99, "DQMHistAnalysisPXDTrackCharge: beginRun called.");

  if (m_cCharge) m_cCharge->Clear();
  if (m_cTrackedClusters) m_cTrackedClusters->Clear();
}

void DQMHistAnalysisPXDTrackChargeModule::event()
{
  gStyle->SetOptStat(0);
  gStyle->SetStatStyle(1);
  gStyle->SetOptDate(22);// Date and Time in Bottom Right, does no work

  if (m_cTrackedClusters and m_hTrackedClusters) { // tracked clusters
    // we already have a plot, but we need to rearrange the X labels in a new plot and scale to events
    std::string name = "Tracked_Clusters"; // new name
    TH1* hh2 = findHist(m_histogramDirectoryName, "PXD_Tracked_Clusters", true);
    if (hh2) {// update only if histogram is updated
      m_cTrackedClusters->Clear();
      m_cTrackedClusters->cd();
      m_hTrackedClusters->Reset();

      auto scale = hh2->GetBinContent(0);// overflow misused as event counter!
      if (scale > 0) {
        auto iscale = 1. / scale;
        int j = 1;
        for (int i = 0; i < 64; i++) {
          auto layer = (((i >> 5) & 0x1) + 1);
          auto ladder = ((i >> 1) & 0xF);
          auto sensor = ((i & 0x1) + 1);

          auto id = Belle2::VxdID(layer, ladder, sensor);
          // Check if sensor exist
          if (Belle2::VXD::GeoCache::getInstance().validSensorID(id)) {
            m_hTrackedClusters->SetBinContent(j, hh2->GetBinContent(i + 1) * iscale);
            j++;
          }
        }
      }
      m_hTrackedClusters->SetName(name.data());
      m_hTrackedClusters->SetTitle("Tracked Clusters/Event");
      m_hTrackedClusters->SetFillColor(kWhite);
      m_hTrackedClusters->SetStats(kFALSE);
      m_hTrackedClusters->SetLineStyle(1);// 2 or 3
      m_hTrackedClusters->SetLineColor(kBlack);
      m_hTrackedClusters->Draw("hist");

      // get ref histogram
      auto href2 = findRefHist(name); // no scaling!
      // TODO: we would expect that it changes with luminosity and maybe beam condition, but not clear how to factor this out. simple scaling seems not the right way.
      if (href2) {
        href2->SetLineStyle(3);// 2 or 3
        href2->SetLineColor(kBlue);
        href2->Draw("same,hist");
      }

      for (auto& it : m_excluded) {
        static std::map <int, TLatex*> ltmap;
        auto tt = ltmap[it];
        if (!tt) {
          tt = new TLatex(it + 0.5, 0, (" " + std::string(m_PXDModules[it]) + " Module is excluded, please ignore").c_str());
          tt->SetTextSize(0.035);
          tt->SetTextAngle(90);// Rotated
          tt->SetTextAlign(12);// Centered
          ltmap[it] = tt;
        }
        tt->Draw();
      }

      UpdateCanvas(m_cTrackedClusters);
    }
  } // end of tracked clusters plot

  bool any_enought_flag = false;

//   auto landau = m_rfws->pdf("landau");
//   auto gauss = m_rfws->pdf("gauss");
  auto model = m_rfws->pdf("lxg");

  auto ml = m_rfws->var("ml");
//   auto sl = m_rfws->var("sl");
//   auto mg = m_rfws->var("mg");
//   auto sg = m_rfws->var("sg");

  if (!m_cCharge) return;
  m_gCharge->Set(0);

  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    TCanvas* canvas = m_cChargeMod[m_PXDModules[i]];
    if (canvas == nullptr) continue;

    std::string name = "PXD_Track_Cluster_Charge_" + (std::string)m_PXDModules[i];
    std::replace(name.begin(), name.end(), '.', '_');

    TH1* hh1 = findHist(m_histogramDirectoryName, name, true);
    if (hh1) {// update only if histo was updated
      canvas->cd();
      canvas->Clear();
      UpdateCanvas(canvas);

      if (hh1->GetEntries() > 50) {

        auto hdata = new RooDataHist(hh1->GetName(), hh1->GetTitle(), *m_x, (const TH1*) hh1);
        auto plot = m_x->frame(RooFit::Title(hh1->GetTitle()));
        /*auto r =*/ model->fitTo(*hdata, RooFit::Range("signal"));

        model->paramOn(plot, RooFit::Format("NELU", RooFit::AutoPrecision(2)), RooFit::Layout(0.6, 0.9, 0.9));
        hdata->plotOn(plot, RooFit::LineColor(kBlue)/*, RooFit::Range("plot"), RooFit::NormRange("signal")*/);
        model->plotOn(plot, RooFit::LineColor(kRed), RooFit::Range("signal"), RooFit::NormRange("signal"));

        plot->Draw("");

//            model->Print("");
//            ml->Print("");
//            sl->Print("");
//            mg->Print("");
//            sg->Print("");
//            cout << "ZZZ , " << Form("%d%02d%d ,", std::get<0>(t), std::get<1>(t), std::get<2>(t)) << ml->getValV() << "," << ml->getError() << "," << sl->getValV() << "," << sl->getError() << "," << sg->getValV() << "," << sg->getError() << endl;


        int p = m_gCharge->GetN();
        m_gCharge->SetPoint(p, i + 0.49, ml->getValV());
        m_gCharge->SetPointError(p, 0.1, ml->getError()); // error in x is useless
        m_monObj->setVariable(("trackcharge_" + (std::string)m_PXDModules[i]).c_str(), ml->getValV(), ml->getError());
      } else {
        hh1->Draw("hist"); // avoid to confuse people by showing nothing for low stat
      }

      // get ref histogram
      auto hist2 = findRefHist(name);// no scaling
      if (hist2) {
        B2DEBUG(20, "Draw Normalized " << hist2->GetName());
        hist2->SetLineStyle(3);// 2 or 3
        hist2->SetLineColor(kBlack);

        canvas->cd();

        // if draw normalized
        TH1* h = (TH1*)hist2->Clone(); // Annoying ... Maybe an memory leak? TODO
        // would it work to scale it each time again?
        if (abs(hist2->GetEntries()) > 0) h->Scale(hh1->GetEntries() / hist2->GetEntries());

        h->SetStats(kFALSE);
        h->Draw("same,hist");
      }

      // add coloring, cuts? based on fit, compare with ref?
      auto status = makeStatus(hh1->GetEntries() >= 100, false, false); // only statistics, no alarm (yet)
      colorizeCanvas(canvas, status);

      canvas->Modified();
      canvas->Update();
      UpdateCanvas(canvas);

      // means if ANY plot is > 100 entries, all plots are assumed to be o.k. from statistics
      if (hh1->GetEntries() >= 1000) any_enought_flag = true;
    }
  }

  // now loop per module over asics pairs (1.5.1)
  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
//     TCanvas* canvas = m_cChargeMod[m_PXDModules[i]];
    VxdID& aVxdID = m_PXDModules[i];

    if (m_hChargeModASIC2d[aVxdID]) m_hChargeModASIC2d[aVxdID]->Reset();
    if (m_cChargeModASIC2d[aVxdID]) m_cChargeModASIC2d[aVxdID]->Clear();

    for (int s = 1; s <= 6; s++) {
      for (int d = 1; d <= 4; d++) {
        std::string name = "PXD_Track_Cluster_Charge_" + (std::string)m_PXDModules[i] + Form("_sw%d_dcd%d", s, d);
        std::replace(name.begin(), name.end(), '.', '_');

        if (m_cChargeModASIC[aVxdID][s - 1][d - 1]) {
          m_cChargeModASIC[aVxdID][s - 1][d - 1]->Clear();
          m_cChargeModASIC[aVxdID][s - 1][d - 1]->cd();
        }

        TH1* hh1 = findHist(m_histogramDirectoryName, name);
        if (hh1) {
          double mpv = 0.0;
          if (hh1->GetEntries() > 50) {
            auto hdata = new RooDataHist(hh1->GetName(), hh1->GetTitle(), *m_x, (const TH1*) hh1);
            auto plot = m_x->frame(RooFit::Title(hh1->GetTitle()));
            /*auto r =*/ model->fitTo(*hdata, RooFit::Range("signal"));

            if (m_cChargeModASIC[aVxdID][s - 1][d - 1]) {
              model->paramOn(plot, RooFit::Format("NELU", RooFit::AutoPrecision(2)), RooFit::Layout(0.6, 0.9, 0.9));
              hdata->plotOn(plot, RooFit::LineColor(kBlue)/*, RooFit::Range("plot"), RooFit::NormRange("signal")*/);
              model->plotOn(plot, RooFit::LineColor(kRed), RooFit::Range("signal"), RooFit::NormRange("signal"));
            }
            plot->Draw("");

            mpv = ml->getValV();
          }

          if (m_hChargeModASIC2d[aVxdID]) {
            if (mpv > 0.0) m_hChargeModASIC2d[aVxdID]->Fill(s, d, mpv); // TODO check what is s, d
          }
        }
      }
    }

    // Overview map of ASCI combinations
    if (m_hChargeModASIC2d[aVxdID] && m_cChargeModASIC2d[aVxdID]) {
      m_cChargeModASIC2d[aVxdID]->cd();
      m_hChargeModASIC2d[aVxdID]->Draw("colz");
      UpdateCanvas(m_cChargeModASIC2d[aVxdID]);
    }
  }

  m_cCharge->cd();
  m_cCharge->Clear();
  m_gCharge->SetMinimum(0);
  m_gCharge->SetMaximum(70);
  auto ax = m_gCharge->GetXaxis();
  if (ax) {
    ax->Set(m_PXDModules.size(), 0, m_PXDModules.size());
    for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
      TString ModuleName = (std::string)m_PXDModules[i];
      ax->SetBinLabel(i + 1, ModuleName);
    }
  } else B2ERROR("no axis");

  m_gCharge->SetLineColor(4);
  m_gCharge->SetLineWidth(2);
  m_gCharge->SetMarkerStyle(8);
  m_gCharge->Draw("AP");

  for (auto& it : m_excluded) {
    static std::map <int, TLatex*> ltmap;
    auto tt = ltmap[it];
    if (!tt) {
      tt = new TLatex(it + 0.5, 0, (" " + std::string(m_PXDModules[it]) + " Module is excluded, please ignore").c_str());
      tt->SetTextSize(0.035);
      tt->SetTextAngle(90);// Rotated
      tt->SetTextAlign(12);// Centered
      ltmap[it] = tt;
    }
    tt->Draw();
  }
  m_cCharge->cd(0);
  m_cCharge->Modified();
  m_cCharge->Update();
  UpdateCanvas(m_cCharge);

  double data = 0;
  double diff = 0;
  if (m_gCharge && any_enought_flag) {
//     double currentMin, currentMax;
    m_gCharge->Fit(m_fMean, "R");
    double mean = m_gCharge->GetMean(2);
    double maxi = mean + 15;
    double mini = mean - 15;
    m_line_up->SetY1(maxi);
    m_line_up->SetY2(maxi);
    m_line_mean->SetY1(mean);
    m_line_mean->SetY2(mean);
    m_line_low->SetY1(mini);
    m_line_low->SetY2(mini);
    data = mean; // m_fMean->GetParameter(0); // we are more interested in the maximum deviation from mean
    // m_gCharge->GetMinimumAndMaximum(currentMin, currentMax);
    diff = m_gCharge->GetRMS(2);// RMS of Y
    // better, max deviation as fabs(data - currentMin) > fabs(currentMax - data) ? fabs(data - currentMin) : fabs(currentMax - data);
    m_line_up->Draw();
    m_line_mean->Draw();
    m_line_low->Draw();

    m_monObj->setVariable("trackcharge", mean, diff);
  }

  setEpicsPV("Mean", data);
  setEpicsPV("Diff", diff);

  // FIXME: what is the acceptable limit?
  auto status = makeStatus(any_enought_flag, fabs(data - 30) > 15. || diff > 8, fabs(data - 30.) > 20. || diff > 12);
  colorizeCanvas(m_cCharge, status);

  setEpicsPV("Status", status);
}

void DQMHistAnalysisPXDTrackChargeModule::endRun()
{
  B2DEBUG(99, "DQMHistAnalysisPXDTrackCharge : endRun called");
}


void DQMHistAnalysisPXDTrackChargeModule::terminate()
{
  B2DEBUG(99, "DQMHistAnalysisPXDTrackCharge: terminate called");

  if (m_rfws) delete m_rfws;

  if (m_cTrackedClusters) delete m_cTrackedClusters;
  if (m_hTrackedClusters) delete m_hTrackedClusters;
  if (m_cCharge) delete m_cCharge;
  if (m_gCharge) delete m_gCharge;
  if (m_line_up) delete m_line_up;
  if (m_line_mean) delete m_line_mean;
  if (m_line_low) delete m_line_low;
  if (m_fMean) delete m_fMean;
}
