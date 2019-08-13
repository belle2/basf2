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

using namespace std;
using namespace Belle2;


using boost::format;

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
  addParam("RangeLow", m_rangeLow, "Lower boarder for fit", 10.);
  addParam("RangeHigh", m_rangeHigh, "High border for fit", 100.);
  addParam("PeakBefore", m_peakBefore, "Range for fit before peak (positive)", 5.);
  addParam("PeakAfter", m_peakAfter, "Range for after peak", 40.);
  addParam("PVName", m_pvPrefix, "PV Prefix", std::string("DQM:PXD:TrackCharge:"));
  addParam("RefHistoFile", m_refFileName, "Reference histrogram file name", std::string("refHisto.root"));
  addParam("ColorAlert", m_color, "Whether to show the color alert", true);
  B2DEBUG(99, "DQMHistAnalysisPXDTrackCharge: Constructor done.");
}

DQMHistAnalysisPXDTrackChargeModule::~DQMHistAnalysisPXDTrackChargeModule()
{
#ifdef _BELLE2_EPICS
  if (ca_current_context()) ca_context_destroy();
#endif
}

void DQMHistAnalysisPXDTrackChargeModule::initialize()
{
  B2DEBUG(99, "DQMHistAnalysisPXDTrackCharge: initialized.");
  m_refFile = NULL;
  if (m_refFileName != "") {
    m_refFile = new TFile(m_refFileName.data());
  }

  VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  // collect the list of all PXD Modules in the geometry here
  std::vector<VxdID> sensors = geo.getListOfSensors();
  for (VxdID& aVxdID : sensors) {
    VXD::SensorInfoBase info = geo.getSensorInfo(aVxdID);
    if (info.getType() != VXD::SensorInfoBase::PXD) continue;
    m_PXDModules.push_back(aVxdID); // reorder, sort would be better

    std::string name = "PXD_Track_Cluster_Charge_" + (std::string)aVxdID;
    std::replace(name.begin(), name.end(), '.', '_');
    m_cChargeMod[aVxdID] = new TCanvas((m_histogramDirectoryName + "/c_Fit_" + name).data());
  }
  std::sort(m_PXDModules.begin(), m_PXDModules.end());  // back to natural order

  gROOT->cd(); // this seems to be important, or strange things happen

  m_cCharge = new TCanvas((m_histogramDirectoryName + "/c_TrackCharge").data());

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

  m_fLandau = new TF1("f_Landau", "landau(0)", m_rangeLow, m_rangeHigh);
  m_fLandau->SetParameter(0, 1000);
  m_fLandau->SetParameter(1, 50);
  m_fLandau->SetParameter(2, 10);
  m_fLandau->SetNpx(100);
  m_fLandau->SetNumberFitPoints(100);

  m_fMean = new TF1("f_Mean", "pol0", 0, m_PXDModules.size());
  m_fMean->SetParameter(0, 50);
  m_fMean->SetLineColor(kYellow);
  m_fMean->SetLineWidth(3);
  m_fMean->SetLineStyle(7);
  m_fMean->SetNpx(m_PXDModules.size());
  m_fMean->SetNumberFitPoints(m_PXDModules.size());

#ifdef _BELLE2_EPICS
  if (!ca_current_context()) SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
  mychid.resize(3);
  SEVCHK(ca_create_channel((m_pvPrefix + "Mean").data(), NULL, NULL, 10, &mychid[0]), "ca_create_channel failure");
  SEVCHK(ca_create_channel((m_pvPrefix + "Diff").data(), NULL, NULL, 10, &mychid[1]), "ca_create_channel failure");
  SEVCHK(ca_create_channel((m_pvPrefix + "Status").data(), NULL, NULL, 10, &mychid[2]), "ca_create_channel failure");
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
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

      /// FIXME Replace by a nice fit
      m_fLandau->SetParameter(0, 1000);
      m_fLandau->SetParameter(1, 50);
      m_fLandau->SetParLimits(1, 10, 80);
      m_fLandau->SetParameter(2, 10);
      m_fLandau->SetParLimits(2, 1, 50);

      if (hh1->GetEntries() > 100) {
        for (int f = 0; f < 5; f++) {
          hh1->Fit(m_fLandau, "R");
          m_fLandau->SetRange(m_fLandau->GetParameter(1) - m_peakBefore, m_fLandau->GetParameter(1) + m_peakAfter);
        }

        int p = m_gCharge->GetN();
        m_gCharge->SetPoint(p, i + 0.5, m_fLandau->GetParameter(1));
        m_gCharge->SetPointError(p, 0.1, m_fLandau->GetParError(1)); // error in x is useless
      }

      TH1* hist2 = GetHisto("ref/" + m_histogramDirectoryName + "/" + name);

      if (hist2) {
        B2INFO("Draw Normalized " << hist2->GetName());
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
        TH1* h = (TH1*)hist2->Clone(); // Anoying ... Maybe an memory leak? TODO
        if (abs(hist2->Integral()) > 0)
          h->Scale(hh1->Integral() / hist2->Integral());

        h->SetFillColor(kWhite);
        h->SetStats(kFALSE);
        hh1->SetFillColor(kWhite);
        if (h->GetMaximum() > hh1->GetMaximum())
          hh1->SetMaximum(1.1 * h->GetMaximum());
        hh1->Draw("hist");
        h->Draw("same hist");

        double data = 1.0;
        canvas->Pad()->SetFrameFillColor(10);
        if (m_color) {
          if (hh1->GetEntries() < 1000) {
            // not enough Entries
            canvas->Pad()->SetFillColor(kGray);
          } else {
            if (data < 1e-2) {
              canvas->Pad()->SetFillColor(kRed);
            } else if (data < 1e-4) {
              canvas->Pad()->SetFillColor(kYellow);
            } else {
              canvas->Pad()->SetFillColor(kGreen);
            }
          }
        } else {
          canvas->Pad()->SetFillColor(kWhite);// White
        }

      }
      canvas->Modified();
      canvas->Update();

      if (hh1->GetEntries() >= 1000) enough = true;
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
    auto tt = new TLatex(5.5, 0.1, "1.3.2 Module is broken, please ignore");
    tt->SetTextAngle(90);// Rotated
    tt->SetTextAlign(12);// Centered
    tt->Draw();
  }
  m_cCharge->cd(0);
  m_cCharge->Modified();
  m_cCharge->Update();

  double data = 0;
  double diff = 0;
  if (m_gCharge) {
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

#ifdef _BELLE2_EPICS
    SEVCHK(ca_put(DBR_DOUBLE, mychid[0], (void*)&data), "ca_set failure");
    SEVCHK(ca_put(DBR_DOUBLE, mychid[1], (void*)&diff), "ca_set failure");
#endif
  }

  int status = 0;

  if (!enough) {
    // not enough Entries
    m_cCharge->Pad()->SetFillColor(kGray);// Magenta or Gray
    // status = 0; default
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
  SEVCHK(ca_put(DBR_INT, mychid[2], (void*)&status), "ca_set failure");
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
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
  for (auto m : mychid) SEVCHK(ca_clear_channel(m), "ca_clear_channel failure");
  SEVCHK(ca_pend_io(5.0), "ca_pend_io failure");
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
    // the following code sux ... is there no root function for that?


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
