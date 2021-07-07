/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisPXDFits.cc
// Description : DQM module, which fits many PXD histograms and writes out fit parameters in new histograms
//-


#include <dqm/analysis/modules/DQMHistAnalysisPXDFits.h>
#include <TROOT.h>

#include <boost/format.hpp>

using namespace std;
using namespace Belle2;

using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisPXDFits)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisPXDFitsModule::DQMHistAnalysisPXDFitsModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms were placed",
           std::string("PXDRAW"));
//  addParam("HistoName", m_histoname, "Name of Histogram (incl dir)", std::string(""));
  for (auto i = 0, j = 0; i < 64; i++) {
    auto layer = (((i >> 5) & 0x1) + 1);
    auto ladder = ((i >> 1) & 0xF);
    if (ladder == 0) continue; // numbering starts at 1
    if (layer == 1 && ladder > 8) continue; // 8 inner ladders
    if (layer == 2 && ladder > 12) continue; // 12 outer ladders
    m_id_to_inx[i] = j; // i = id , j - index
    m_inx_to_id[j] = i;
    j++;
    if (j == NUM_MODULES) break;
  }
  for (auto i = 0; i < NUM_MODULES; i++) {
    m_hSignal[i] = nullptr;
    m_hCommon[i] = nullptr;
    m_hCounts[i] = nullptr;
    m_cSignal[i] = nullptr;
    m_cCommon[i] = nullptr;
    m_cCounts[i] = nullptr;
  }
  B2DEBUG(1, "DQMHistAnalysisPXDFits: Constructor done.");
}

void DQMHistAnalysisPXDFitsModule::initialize()
{
  B2DEBUG(1, "DQMHistAnalysisPXDFits: initialized.");

  gROOT->cd(); // this seems to be important, or strange things happen

  TString a;

  a = "pxdraw/hSignalAll";
  a.ReplaceAll("/", "_");
  m_cSignalAll = new TCanvas((m_histogramDirectoryName + "/c_").data() + a);
  m_hSignalAll = new TH1F(a, a, NUM_MODULES, 0, NUM_MODULES);
  m_hSignalAll->SetDirectory(0);// dont mess with it, this is MY histogram
  m_hSignalAll->SetStats(false);

  a = "pxdraw/hCommonAll";
  a.ReplaceAll("/", "_");
  m_cCommonAll = new TCanvas((m_histogramDirectoryName + "/c_").data() + a);
  m_hCommonAll = new TH1F(a, a, NUM_MODULES, 0, NUM_MODULES);
  m_hCommonAll->SetDirectory(0);// dont mess with it, this is MY histogram
  m_hCommonAll->SetStats(false);

  a = "pxdraw/hCountsAll";
  a.ReplaceAll("/", "_");
  m_cCountsAll = new TCanvas((m_histogramDirectoryName + "/c_").data() + a);
  m_hCountsAll = new TH1F(a, a, NUM_MODULES, 0, NUM_MODULES);
  m_hCountsAll->SetDirectory(0);// dont mess with it, this is MY histogram
  m_hCountsAll->SetStats(false);

  a = "pxdraw/hOccupancyAll";
  a.ReplaceAll("/", "_");
  m_cOccupancyAll = new TCanvas((m_histogramDirectoryName + "/c_").data() + a);
  m_hOccupancyAll = new TH1F(a, a, NUM_MODULES, 0, NUM_MODULES);
  m_hOccupancyAll->SetDirectory(0);// dont mess with it, this is MY histogram
  m_hOccupancyAll->SetStats(false);

  for (auto i = 0; i < NUM_MODULES; i++) {
    auto id = m_inx_to_id[i];
    auto layer = (((id >> 5) & 0x1) + 1);
    auto ladder = ((id >> 1) & 0xF);
    auto sensor = ((id & 0x1) + 1);
    string s2 = str(format("_%d.%d.%d") % layer % ladder % sensor);

    m_hSignalAll->GetXaxis()->SetBinLabel(i + 1, TString(s2));
    m_hCommonAll->GetXaxis()->SetBinLabel(i + 1, TString(s2));
    m_hCountsAll->GetXaxis()->SetBinLabel(i + 1, TString(s2));
    m_hOccupancyAll->GetXaxis()->SetBinLabel(i + 1, TString(s2));

    a = "pxdraw/hSignal";
    a.ReplaceAll("/", "_");
    a += s2;

    m_cSignal[i] = new TCanvas((m_histogramDirectoryName + "/c_").data() + a);
    m_hSignal[i] = new TH2F(a, a, 6, 0, 6, 4, 0, 4);
    m_hSignal[i]->SetDirectory(0);// dont mess with it, this is MY histogram
    m_hSignal[i]->SetStats(false);
    m_hSignal[i]->SetMinimum(0);
    m_hSignal[i]->SetMaximum(64);

    a = "pxdraw/hCommon";
    a.ReplaceAll("/", "_");
    a += s2;
    m_cCommon[i] = new TCanvas((m_histogramDirectoryName + "/c_").data() + a);
    m_hCommon[i] = new TH2F(a, a, 6, 0, 6, 4, 0, 4);
    m_hCommon[i]->SetDirectory(0);// dont mess with it, this is MY histogram
    m_hCommon[i]->SetStats(false);
    m_hCommon[i]->SetMinimum(0);
    m_hCommon[i]->SetMaximum(256);

    a = "pxdraw/hCounts";
    a.ReplaceAll("/", "_");
    a += s2;
    m_cCounts[i] = new TCanvas((m_histogramDirectoryName + "/c_").data() + a);
    m_hCounts[i] = new TH2F(a, a, 6, 0, 6, 4, 0, 4);
    m_hCounts[i]->SetDirectory(0);// dont mess with it, this is MY histogram
    m_hCounts[i]->SetStats(false);
  }

  m_fLandau = new TF1("f_Landau", "landau", 0, 256);
  m_fLandau->SetParameter(0, 1000);
  m_fLandau->SetParameter(1, 0);
  m_fLandau->SetParameter(2, 10);
  m_fLandau->SetLineColor(4);
  m_fLandau->SetNpx(256);
  m_fLandau->SetNumberFitPoints(256);

  m_fGaus = new TF1("f_Gaus", "gaus", 0, 8096);
  m_fGaus->SetParameter(0, 1000);
  m_fGaus->SetParameter(1, 0);
  m_fGaus->SetParameter(2, 10);
  m_fGaus->SetLineColor(4);
  m_fGaus->SetNpx(256);
  m_fGaus->SetNumberFitPoints(256);
}


void DQMHistAnalysisPXDFitsModule::beginRun()
{
  B2DEBUG(1, "DQMHistAnalysisPXDFits: beginRun called.");

  // not much we can do here ... as we have created everything already
  for (auto i = 0; i < NUM_MODULES; i++) {
    m_cSignal[i]->Clear();
    m_cCommon[i]->Clear();
    m_cCounts[i]->Clear();
    // no need to Cd and Draw yet
  }
}

void DQMHistAnalysisPXDFitsModule::event()
{
//  bool flag = false;

  m_hSignalAll->Reset(); // dont sum up!!!
  m_hCommonAll->Reset(); // dont sum up!!!
  m_hCountsAll->Reset(); // dont sum up!!!
  m_hOccupancyAll->Reset(); // dont sum up!!!

  for (auto i = 0; i < NUM_MODULES; i++) {
    auto id = m_inx_to_id[i];
    auto layer = (((id >> 5) & 0x1) + 1);
    auto ladder = ((id >> 1) & 0xF);
    auto sensor = ((id & 0x1) + 1);

    m_hSignal[i]->Reset(); // dont sum up!!!
    m_hCommon[i]->Reset(); // dont sum up!!!
    m_hCounts[i]->Reset(); // dont sum up!!!

    for (auto j = 0; j < 6; j++) {
      for (auto k = 0; k < 4; k++) {
        //TH1* hh1 = NULL;
        string s2 = str(format("_%d.%d.%d_%d_%d") % layer % ladder % sensor % j % k);

        std::string name = "hrawPxdHitsCharge" + s2;
        TH1* hh1 = findHist(name);
        if (hh1 == NULL) {
          hh1 = findHist(m_histogramDirectoryName, name);
        }

        if (hh1 != NULL) {
//           cout << "do da fit " << endl;
//           m_fLandau->SetParameter(0, 1000);
//           m_fLandau->SetParameter(1, 0);
//           m_fLandau->SetParameter(2, 10);
//           hh1->Fit(m_fLandau, "0");
//           m_hSignal[i]->Fill(j, k, m_fLandau->GetParameter(1));
//           cout << m_fLandau->GetParameter(0) << " " << m_fLandau->GetParameter(1) << " " << m_fLandau->GetParameter(2) << endl;
          m_hSignal[i]->Fill(j, k, hh1->GetMean());
          m_hSignalAll->Fill(i, hh1->GetMean());
        } else {
          B2INFO("Histo " << name << " not found");
        }

        name = "hrawPxdHitsCommonMode" + s2;
        hh1 = findHist(name);
        if (hh1 == NULL) {
          hh1 = findHist(m_histogramDirectoryName, name);
        }

        if (hh1 != NULL) {
//           cout << "do da fit " << endl;
//           m_fGaus->SetParameter(0, 1000);
//           m_fGaus->SetParameter(1, 10);
//           m_fGaus->SetParameter(2, 10);
//           hh1->Fit(m_fGaus, "0");
//           m_hCommon[i]->Fill(j, k, m_fGaus->GetParameter(1));
//           cout << m_fGaus->GetParameter(0) << " " << m_fGaus->GetParameter(1) << " " << m_fGaus->GetParameter(2) << endl;
          m_hCommon[i]->Fill(j, k, hh1->GetMean());
          m_hCommonAll->Fill(i, hh1->GetMean());
        } else {
          B2INFO("Histo " << name << " not found");
        }

        name = "hrawPxdCount" + s2;
        hh1 = findHist(name);
        if (hh1 == NULL) {
          hh1 = findHist(m_histogramDirectoryName, name);
        }

        if (hh1 != NULL) {
//           cout << "do da fit " << endl;
//           m_fGaus->SetParameter(0, 1000);
//           m_fGaus->SetParameter(1, 100);
//           m_fGaus->SetParameter(2, 10);
//           hh1->Fit(m_fGaus, "0");
//           m_hCounts[i]->Fill(j, k, m_fGaus->GetParameter(1));
//           cout << m_fGaus->GetParameter(0) << " " << m_fGaus->GetParameter(1) << " " << m_fGaus->GetParameter(2) << endl;
          m_hCounts[i]->Fill(j, k, hh1->GetMean());
          m_hCountsAll->Fill(i, hh1->GetMean());
          m_hOccupancyAll->Fill(i, hh1->GetMean() / (250 * 768 / 24)); // Occupancy in percent
        } else {
          B2INFO("Histo " << name << " not found");
        }
      }
    }
    if (m_cSignal[i]) {
      m_cSignal[i]->cd();
      if (m_hSignal[i]) m_hSignal[i]->Draw("colz");
      m_cSignal[i]->Modified();
      m_cSignal[i]->Update();
    }
    if (m_cCommon[i]) {
      m_cCommon[i]->cd();
      if (m_hCommon[i]) m_hCommon[i]->Draw("colz");
      m_cCommon[i]->Modified();
      m_cCommon[i]->Update();
    }
    if (m_cCounts[i]) {
      m_cCounts[i]->cd();
      if (m_hCounts[i]) m_hCounts[i]->Draw("colz");
      m_cCounts[i]->Modified();
      m_cCounts[i]->Update();
    }
  }
  if (m_cSignalAll) {
    m_cSignalAll->cd();
    if (m_hSignalAll) {
      m_hSignalAll->Scale(1.0 / 24.0); // need to scale
      m_hSignalAll->Draw();
    }
    m_cSignalAll->Modified();
    m_cSignalAll->Update();
  }
  if (m_cCommonAll) {
    m_cCommonAll->cd();
    if (m_hCommonAll) {
      m_hCommonAll->Scale(1.0 / 24.0); // need to scale
      m_hCommonAll->Draw();
    }
    m_cCommonAll->Modified();
    m_cCommonAll->Update();
  }
  if (m_cCountsAll) {
    m_cCountsAll->cd();
    if (m_hCountsAll) {
      // m_hCountsAll->Scale(1.0/24.0); // dont scale counts, we want to have sum
      // but we would need to scale if we directly calculate occupancy!
      m_hCountsAll->Draw();
    }
    m_cCountsAll->Modified();
    m_cCountsAll->Update();
  }
  if (m_cOccupancyAll) {
    m_cOccupancyAll->cd();
    if (m_hOccupancyAll) {
      m_hOccupancyAll->Scale(1.0 / 24.0); // need to scale
      m_hOccupancyAll->Draw();
    }
    m_cOccupancyAll->Modified();
    m_cOccupancyAll->Update();
  }
}

void DQMHistAnalysisPXDFitsModule::endRun()
{
  B2DEBUG(1, "DQMHistAnalysisPXDFits : endRun called");
}


void DQMHistAnalysisPXDFitsModule::terminate()
{
  B2DEBUG(1, "DQMHistAnalysisPXDFits: terminate called");
}

