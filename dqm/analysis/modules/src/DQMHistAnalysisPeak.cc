/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisPeak.cc
// Description : Simple Peak Analysis (Mean/Media/Width) for simple peaked distributions with delta histogramming
//-


#include <dqm/analysis/modules/DQMHistAnalysisPeak.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisPeak);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisPeakModule::DQMHistAnalysisPeakModule()
  : DQMHistAnalysisModule()
{
  // This module CAN NOT be run in parallel!
  setDescription("Modify and analyze the peaking distributions in data quality histograms");

  //Parameter definition
  addParam("HistoName", m_histoName, "Name of Histogram (excl dir)", std::string(""));
  addParam("HistoDirectory", m_histoDirectory, "Name of Histogram dir", std::string(""));
  addParam("PVName", m_pvPrefix, "PV Prefix and Name", std::string(""));
  addParam("MonitorPrefix", m_monPrefix, "Monitor Prefix", std::string(""));
  addParam("MonitorObjectName", m_monObjectName, "Monitor Object Name", std::string(""));
  addParam("minEntries", m_minEntries, "minimum number of new Entries for a fit", m_minEntries);
  addParam("plot", m_plot, "plot histogram in canvas", m_plot);
  addParam("zoom", m_zoom, "zoom on peak", m_zoom);
  addParam("mean", m_mean, "extract and plot mean", m_mean);
  addParam("median", m_median, "extract and plot median", m_median);
  addParam("rms", m_rms, "extract rms", m_rms);
  B2DEBUG(20, "DQMHistAnalysisPeak: Constructor done.");
}

void DQMHistAnalysisPeakModule::initialize()
{
  B2DEBUG(20, "DQMHistAnalysisPeak: initialized.");

  if (m_monObjectName != "") {
    m_monObj = getMonitoringObject(m_monObjectName);
  }

  // register delta
  if (!hasDeltaPar(m_histoDirectory, m_histoName)) addDeltaPar(m_histoDirectory, m_histoName, HistDelta::c_Entries, m_minEntries, 1);

  // prefer to change canvas name to monitorPrefix, but then changes on the web gui are needed :-(
  if (m_plot) {
    m_canvas = new TCanvas((m_histoDirectory + "/c_" + m_histoName +  "_fit").data());

    if (m_mean) {
      m_lineMean = new TLine(0, 10, 0, 0);
      m_lineMean->SetVertical(true);
      m_lineMean->SetLineColor(8);
      m_lineMean->SetLineWidth(3);
    }
    if (m_median) {
      m_lineMedian = new TLine(0, 10, 0, 0);
      m_lineMedian->SetVertical(true);
      m_lineMedian->SetLineColor(9);
      m_lineMedian->SetLineWidth(3);
    }
  }

  if (m_monObj) m_monObj->addCanvas(m_canvas);

  if (m_pvPrefix != "") {
    if (m_mean) registerEpicsPV(m_pvPrefix + "Mean", "Mean");
    if (m_median) registerEpicsPV(m_pvPrefix + "Median", "Median");
    if (m_rms) registerEpicsPV(m_pvPrefix + "RMS", "RMS");
  }
}

void DQMHistAnalysisPeakModule::terminate()
{
  if (m_lineMean) delete m_lineMean;
  if (m_lineMedian) delete m_lineMedian;
  if (m_canvas) delete m_canvas;
}

void DQMHistAnalysisPeakModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisPeak: beginRun called.");
  if (m_canvas) m_canvas->Clear();
}

void DQMHistAnalysisPeakModule::event()
{
  auto delta = getDelta(m_histoDirectory, m_histoName);
  // do not care about initial filling handling. we wont show or update unless we reach the min req entries
  UpdateCanvas(m_canvas, delta != nullptr);
  if (delta != nullptr) {

    // we modify the current delta histogram, that is bad habit
    // but as long as no one else uses it, it may be o.k.
    // for more severe changes, maybe better work on a clone?)
    delta->ResetStats(); // kills the Mean from filling, now only use bin values excl over/underflow
    double mean = delta->GetMean();// must be double bc of EPICS below
    double rms = delta->GetRMS();// must be double bc of EPICS below
    double q = 0.5; // array size one for quantiles
    double median = 0; // array of size 1 for result = median
    delta->ComputeIntegral(); // precaution
    delta->GetQuantiles(1, &median, &q);
    double y1 = delta->GetMaximum();
    double y2 = delta->GetMinimum();
    B2DEBUG(20, "Fit " << mean << "," << rms << "," << y1 << "," << y2);

    // Now plot
    if (m_canvas) {
      m_canvas->Clear();
      m_canvas->cd();// necessary!
      delta->Draw("hist");

      if (m_zoom) delta->GetXaxis()->SetRangeUser(mean - 3 * rms, mean + 3 * rms);
      if (m_lineMean) {
        m_lineMean->SetY1(y1 + (y1 - y2) * 0.05);
        m_lineMean->SetX1(mean);
        m_lineMean->SetX2(mean);
        m_lineMean->Draw();
      }
      if (m_lineMedian) {
        m_lineMedian->SetY1(y1 + (y1 - y2) * 0.05);
        m_lineMedian->SetX1(median);
        m_lineMedian->SetX2(median);
        m_lineMedian->Draw();
      }
      m_canvas->Modified();
      m_canvas->Update();
    }
    if (m_monObj and m_monPrefix != "") {
      if (m_mean) m_monObj->setVariable(m_monPrefix + "_mean", mean);
      if (m_median) m_monObj->setVariable(m_monPrefix + "_median", median);
      if (m_rms) m_monObj->setVariable(m_monPrefix + "_width", rms);
    }

    B2DEBUG(20, "Now update Epics PVs");
    if (m_pvPrefix != "") {
      if (m_mean) setEpicsPV("Mean", mean);
      if (m_median) setEpicsPV("Median", median);
      if (m_rms) setEpicsPV("RMS", rms);
    }
  }
}
