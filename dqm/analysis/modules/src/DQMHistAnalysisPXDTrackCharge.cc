//+
// File : DQMHistAnalysisPXDTrackCharge.cc
// Description : Analysis of PXD Cluster Charge
//
// Author : Bjoern Spruck, University Mainz
// Date : 2019
//-


#include <dqm/analysis/modules/DQMHistAnalysisPXDTrackCharge.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TClass.h>
#include <TLatex.h>
#include <vxd/geometry/GeoCache.h>
#include <TKey.h>

#include <RooDataHist.h>
#include <RooAbsPdf.h>
#include <RooPlot.h>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisPXDTrackCharge)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisPXDTrackChargeModule::DQMHistAnalysisPXDTrackChargeModule()
  : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!

  //Parameter definition
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of Histogram dir", std::string("PXDER"));
  addParam("RangeLow", m_rangeLow, "Lower border for fit", 20.);
  addParam("RangeHigh", m_rangeHigh, "High border for fit", 80.);
//   addParam("PeakBefore", m_peakBefore, "Range for fit before peak (positive)", 5.);
//   addParam("PeakAfter", m_peakAfter, "Range for after peak", 40.);
  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("DQM:PXD:TrackCharge:"));
  addParam("useEpics", m_useEpics, "useEpics", true);
  addParam("RefHistoFile", m_refFileName, "Reference histrogram file name", std::string("refHisto.root"));
  addParam("ColorAlert", m_color, "Whether to show the color alert", true);

  B2DEBUG(99, "DQMHistAnalysisPXDTrackCharge: Constructor done.");
}

DQMHistAnalysisPXDTrackChargeModule::~DQMHistAnalysisPXDTrackChargeModule()
{
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (ca_current_context()) ca_context_destroy();
  }
#endif
}

void DQMHistAnalysisPXDTrackChargeModule::initialize()
{
  B2DEBUG(99, "DQMHistAnalysisPXDTrackCharge: initialized.");

  m_refFile = NULL;
  if (m_refFileName != "") {
    m_refFile = new TFile(m_refFileName.data());
  }

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


void DQMHistAnalysisPXDTrackChargeModule::beginRun()
{
  B2DEBUG(99, "DQMHistAnalysisPXDTrackCharge: beginRun called.");

  m_cCharge->Clear();
}

void DQMHistAnalysisPXDTrackChargeModule::event()
{
  if (!m_cCharge) return;

  gStyle->SetOptStat(0);
  gStyle->SetStatStyle(1);
  gStyle->SetOptDate(22);// Date and Time in Bottom Right, does no work

  bool enough = false;

//   auto landau = m_rfws->pdf("landau");
//   auto gauss = m_rfws->pdf("gauss");
  auto model = m_rfws->pdf("lxg");

  auto ml = m_rfws->var("ml");
//   auto sl = m_rfws->var("sl");
//   auto mg = m_rfws->var("mg");
//   auto sg = m_rfws->var("sg");

  {
    m_cTrackedClusters->Clear();
    m_cTrackedClusters->cd();
    m_hTrackedClusters->Reset();

    std::string name = "Tracked_Clusters"; // new name
    TH1* hh2 = findHist(m_histogramDirectoryName, "PXD_Tracked_Clusters");
    if (hh2) {
      auto scale = hh2->GetBinContent(0);// overflow misused as event counter!
      if (scale > 0) {
        int j = 1;
        for (int i = 0; i < 64; i++) {
          auto layer = (((i >> 5) & 0x1) + 1);
          auto ladder = ((i >> 1) & 0xF);
          auto sensor = ((i & 0x1) + 1);

          auto id = Belle2::VxdID(layer, ladder, sensor);
          // Check if sensor exist
          if (Belle2::VXD::GeoCache::getInstance().validSensorID(id)) {
            m_hTrackedClusters->SetBinContent(j, hh2->GetBinContent(i + 1) / scale);
            j++;
          }
        }
      }
      m_hTrackedClusters->SetName(name.data());
      m_hTrackedClusters->SetTitle("Tracked Clusters/Event");
      m_hTrackedClusters->SetFillColor(kWhite);
      m_hTrackedClusters->SetStats(kFALSE);
      m_hTrackedClusters->SetLineStyle(1);// 2 or 3
      m_hTrackedClusters->SetLineColor(kBlue);
      m_hTrackedClusters->Draw("hist");

      TH1* href2 = GetHisto("ref/" + m_histogramDirectoryName + "/" + name);

      if (href2) {
        href2->SetLineStyle(3);// 2 or 3
        href2->SetLineColor(kBlack);
        href2->Draw("same,hist");
      }

      auto tt = new TLatex(5.5, 0, " 1.3.2 Module is excluded, please ignore");
      tt->SetTextAngle(90);// Rotated
      tt->SetTextAlign(12);// Centered
      tt->Draw();
    }
  }

  m_gCharge->Set(0);

  for (unsigned int i = 0; i < m_PXDModules.size(); i++) {
    TCanvas* canvas = m_cChargeMod[m_PXDModules[i]];
    if (canvas == nullptr) continue;

    std::string name = "PXD_Track_Cluster_Charge_" + (std::string)m_PXDModules[i];
    std::replace(name.begin(), name.end(), '.', '_');

    canvas->cd();
    canvas->Clear();

    TH1* hh1 = findHist(m_histogramDirectoryName, name);
    if (hh1) {

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
      }

      TH1* hist2 = GetHisto("ref/" + m_histogramDirectoryName + "/" + name);

      if (hist2) {
//         B2INFO("Draw Normalized " << hist2->GetName());
        hist2->SetLineStyle(3);// 2 or 3
        hist2->SetLineColor(kBlack);

//         TIter nextkey(canvas->GetListOfPrimitives());
//         TObject* obj = NULL;
//         while ((obj = (TObject*)nextkey())) {
//           if (obj->IsA()->InheritsFrom("TH1")) {
//             if (string(obj->GetName()) == string(hist2->GetName())) {
//               delete obj;
//             }
//           }
//         }

        canvas->cd();

        // if draw normalized
        TH1* h = (TH1*)hist2->Clone(); // Annoying ... Maybe an memory leak? TODO
        if (abs(hist2->GetEntries()) > 0) h->Scale(hh1->GetEntries() / hist2->GetEntries());

        h->SetStats(kFALSE);
        h->Draw("same,hist");
      }

      // add coloring, cuts? based on fit, compare with ref?
      canvas->Pad()->SetFrameFillColor(10);
      if (m_color) {
        if (hh1->GetEntries() < 100) {
          // not enough Entries
          canvas->Pad()->SetFillColor(kGray);
        } else {
          canvas->Pad()->SetFillColor(kGreen);
        }
      } else {
        canvas->Pad()->SetFillColor(kWhite);// White
      }

      canvas->Modified();
      canvas->Update();

      // means if ANY plot is > 100 entries, all plots are assumed to be o.k.
      if (hh1->GetEntries() >= 1000) enough = true;
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
  {
    auto tt = new TLatex(5.5, 0, " 1.3.2 Module is excluded, please ignore");
    tt->SetTextAngle(90);// Rotated
    tt->SetTextAlign(12);// Centered
    tt->Draw();
  }
  m_cCharge->cd(0);
  m_cCharge->Modified();
  m_cCharge->Update();

  double data = 0;
  double diff = 0;
  if (m_gCharge && enough) {
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
    data = mean; // m_fMean->GetParameter(0); // we are more interessted in the maximum deviation from mean
    // m_gCharge->GetMinimumAndMaximum(currentMin, currentMax);
    diff = m_gCharge->GetRMS(2);// RMS of Y
    // better, max deviation as fabs(data - currentMin) > fabs(currentMax - data) ? fabs(data - currentMin) : fabs(currentMax - data);
    m_line_up->Draw();
    m_line_mean->Draw();
    m_line_low->Draw();

    m_monObj->setVariable("trackcharge", mean, diff);
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
    if (fabs(data - 30.) > 20. || diff > 12) {
      m_cCharge->Pad()->SetFillColor(kRed);// Red
      status = 4;
    } else if (fabs(data - 30) > 15. || diff > 8) {
      m_cCharge->Pad()->SetFillColor(kYellow);// Yellow
      status = 3;
    } else {
      m_cCharge->Pad()->SetFillColor(kGreen);// Green
      status = 2;
    }

    // FIXME overwrite for now
    // m_cCharge->Pad()->SetFillColor(kGreen);// Green

  }

#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    SEVCHK(ca_put(DBR_INT, mychid[2], (void*)&status), "ca_set failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif

}

void DQMHistAnalysisPXDTrackChargeModule::endRun()
{
  B2DEBUG(99, "DQMHistAnalysisPXDTrackCharge : endRun called");
}


void DQMHistAnalysisPXDTrackChargeModule::terminate()
{
  B2DEBUG(99, "DQMHistAnalysisPXDTrackCharge: terminate called");
  // should delete canvas here, maybe hist, too? Who owns it?
#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    for (auto m : mychid) SEVCHK(ca_clear_channel(m), "ca_clear_channel failure");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
  }
#endif
  if (m_refFile) delete m_refFile;

}

TH1* DQMHistAnalysisPXDTrackChargeModule::GetHisto(TString histoname)
{
  TH1* hh1 = nullptr;
  gROOT->cd();
//   hh1 = findHist(histoname.Data());
  if (hh1 == NULL) {
    B2DEBUG(20, "findHisto failed " << histoname << " not in memfile");

    // first search reference root file ... if ther is one
    if (m_refFile && m_refFile->IsOpen()) {
      TDirectory* d = m_refFile;
      TString myl = histoname;
      TString tok;
      Ssiz_t from = 0;
      B2DEBUG(20, myl);
      while (myl.Tokenize(tok, from, "/")) {
        TString dummy;
        Ssiz_t f;
        f = from;
        if (myl.Tokenize(dummy, f, "/")) { // check if its the last one
          auto e = d->GetDirectory(tok);
          if (e) {
            B2DEBUG(20, "Cd Dir " << tok << " from " << d->GetPath());
            d = e;
          } else {
            B2DEBUG(20, "cd failed " << tok << " from " << d->GetPath());
          }
        } else {
          break;
        }
      }
      TObject* obj = d->FindObject(tok);
      if (obj != NULL) {
        if (obj->IsA()->InheritsFrom("TH1")) {
          B2DEBUG(20, "Histo " << histoname << " found in ref file");
          hh1 = (TH1*)obj;
        } else {
          B2DEBUG(20, "Histo " << histoname << " found in ref file but wrong type");
        }
      } else {
        // seems find will only find objects, not keys, thus get the object on first access
        TIter next(d->GetListOfKeys());
        TKey* key;
        while ((key = (TKey*)next())) {
          TObject* obj2 = key->ReadObj() ;
          if (obj2->InheritsFrom("TH1")) {
            if (obj2->GetName() == tok) {
              hh1 = (TH1*)obj2;
              B2DEBUG(20, "Histo " << histoname << " found as key -> readobj");
              break;
            }
          }
        }
        if (hh1 == NULL) B2DEBUG(20, "Histo " << histoname << " NOT found in ref file " << tok);
      }
    }

    if (hh1 == NULL) {
      B2DEBUG(20, "Histo " << histoname << " not in memfile or ref file");

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
            B2DEBUG(20, "Cd Dir " << tok);
            d = e;
          } else B2DEBUG(20, "cd failed " << tok);
          d->cd();
        } else {
          break;
        }
      }
      TObject* obj = d->FindObject(tok);
      if (obj != NULL) {
        if (obj->IsA()->InheritsFrom("TH1")) {
          B2DEBUG(20, "Histo " << histoname << " found in mem");
          hh1 = (TH1*)obj;
        }
      } else {
        B2DEBUG(20, "Histo " << histoname << " NOT found in mem");
      }
    }
  }

  if (hh1 == NULL) {
    B2DEBUG(20, "Histo " << histoname << " not found");
  }

  return hh1;
}
