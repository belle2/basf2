/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <dqm/analysis/modules/DQMHistAnalysisCDCEpics.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisCDCEpics);

DQMHistAnalysisCDCEpicsModule::DQMHistAnalysisCDCEpicsModule()
  : DQMHistAnalysisModule()
{
  addParam("HistDirectory", m_histoDir, "CDC Dir of DQM Histogram", std::string("CDC"));
  addParam("HistADC", m_histoADC, "ADC Histogram Name", std::string("hADC"));
  addParam("HistTDC", m_histoTDC, "TDC Histogram Name", std::string("hTDC"));
  addParam("PvPrefix", m_pvPrefix, "PV Prefix and Name", std::string("CDC:"));
  addParam("MinEvt", m_minevt, "Min events for intra-run point", 20000);
  for (int i = 0; i < 300; i++) {
    m_hADCs[i] = nullptr;
    m_hTDCs[i] = nullptr;
  }
  B2DEBUG(20, "DQMHistAnalysisCDCEpics: Constructor done.");
}

DQMHistAnalysisCDCEpicsModule::~DQMHistAnalysisCDCEpicsModule()
{

}

void DQMHistAnalysisCDCEpicsModule::initialize()
{
  gROOT->cd();
  m_hist_adc = new TH1F("CDC/hist_adc", "m_hist_adc", 300, 0, 300);
  m_hist_adc->SetTitle("ADC Medians; CDC board index; ADC medians");

  m_hist_tdc = new TH1F("CDC/hist_tdc", "m_hist_tdc", 300, 0, 300);
  m_hist_tdc->SetTitle("TDC Medians; CDC board index; TDC medians");

  c_hist_adc = new TCanvas("CDC/c_hist_adc", "c_hist_adc", 500, 400);
  c_hist_tdc = new TCanvas("CDC/c_hist_tdc", "c_hist_tdc", 500, 400);

  if (!hasDeltaPar(m_histoDir, m_histoADC))
    addDeltaPar(m_histoDir, m_histoADC, HistDelta::c_Entries, m_minevt, 1);

  if (!hasDeltaPar(m_histoDir, m_histoTDC))
    addDeltaPar(m_histoDir, m_histoTDC, HistDelta::c_Entries, m_minevt, 1);

  registerEpicsPV(m_pvPrefix + "cdcboards_wadc", "adcboards");
  registerEpicsPV(m_pvPrefix + "cdcboards_wtdc", "tdcboards");

  registerEpicsPV(m_pvPrefix + "adc_median_window", "adcmedianwindow");
  registerEpicsPV(m_pvPrefix + "tdc_median_window", "tdcmedianwindow");

  B2DEBUG(20, "DQMHistAnalysisCDCEpics: initialized.");
}

void DQMHistAnalysisCDCEpicsModule::beginRun()
{
  double unused = 0;
  requestLimitsFromEpicsPVs("adcmedianwindow", unused, m_minadc, m_maxadc, unused);
  requestLimitsFromEpicsPVs("tdcmedianwindow", unused, m_mintdc, m_maxtdc, unused);

  //in case if something is wrong in config file
  if (std::isnan(m_minadc)) m_minadc = 60.0;
  if (std::isnan(m_maxadc)) m_maxadc = 130.0;
  if (std::isnan(m_mintdc)) m_mintdc = 4600.0;
  if (std::isnan(m_maxtdc)) m_maxtdc = 5000.0;

  //creating box for normal adc and tdc windows
  m_line_ladc = new TLine(0, m_minadc, 300, m_minadc);
  m_line_ladc->SetLineColor(kRed);
  m_line_ladc->SetLineWidth(2);

  m_line_hadc = new TLine(0, m_maxadc, 300, m_maxadc);
  m_line_hadc->SetLineColor(kRed);
  m_line_hadc->SetLineWidth(2);

  m_line_ltdc = new TLine(0, m_mintdc, 300, m_mintdc);
  m_line_ltdc->SetLineColor(kRed);
  m_line_ltdc->SetLineWidth(2);

  m_line_htdc = new TLine(0, m_maxtdc, 300, m_maxtdc);
  m_line_htdc->SetLineColor(kRed);
  m_line_htdc->SetLineWidth(2);

  B2DEBUG(20, "DQMHistAnalysisCDCEpics: beginRun run called");
}

void DQMHistAnalysisCDCEpicsModule::event()
{
  //get intra-run histogram from CDC DQM module
  auto m_delta_adc = (TH2F*)getDelta(m_histoDir, m_histoADC, 0, true); //true=only if updated
  if (m_delta_adc) {
    m_hist_adc->Reset();
    int cadcgood = 0;
    int cadcbad = 0;
    double sumadcgood = 0;
    for (int ic = 0; ic < 300; ++ic) {
      if (ic == 0) continue; //299 boards only
      if (m_hADCs[ic]) delete m_hADCs[ic];
      m_hADCs[ic] = m_delta_adc->ProjectionY(Form("hADC%d", ic + 1), ic + 1, ic + 1, "");
      m_hADCs[ic]->SetTitle(Form("hADC%d", ic));
      float md_adc = getHistMedian(m_hADCs[ic]);
      m_hist_adc->SetBinContent(ic + 1, md_adc);
      if (md_adc >= m_minadc && md_adc <= m_maxadc) {
        sumadcgood = sumadcgood + md_adc;
        cadcgood++;
      } else cadcbad++;
    }
    double adcfrac = cadcgood / 2.99; // (100.0/299) in %
    setEpicsPV("adcboards", adcfrac);
    // Draw canvas
    c_hist_adc->Clear();
    c_hist_adc->cd();
    if (cadcgood > 0)sumadcgood = sumadcgood * 1.0 / cadcgood;
    getHistStyle(m_hist_adc, "adc", sumadcgood);
    m_hist_adc->SetTitle(Form("ADC Medians: Bad board count = %d (%0.01f%%)", cadcbad - 1, 100.0 - adcfrac));
    m_hist_adc->Draw("");
    m_line_ladc->Draw("same");
    m_line_hadc->Draw("same");
    c_hist_adc->Update();
    UpdateCanvas(c_hist_adc);

  }

  auto m_delta_tdc = (TH2F*)getDelta(m_histoDir, m_histoTDC, 0, true);
  if (m_delta_tdc) {
    m_hist_tdc->Reset();
    int ctdcgood = 0;
    int ctdcbad = 0;
    double sumtdcgood = 0;
    for (int ic = 0; ic < 300; ++ic) {
      if (ic == 0) continue; //299 boards only
      if (m_hTDCs[ic]) delete m_hTDCs[ic];
      m_hTDCs[ic] = m_delta_tdc->ProjectionY(Form("hTDC%d", ic + 1), ic + 1, ic + 1, "");
      m_hTDCs[ic]->SetTitle(Form("hTDC%d", ic));
      float md_tdc = getHistMedian(m_hTDCs[ic]);
      m_hist_tdc->SetBinContent(ic + 1, md_tdc);
      if (md_tdc >= m_mintdc && md_tdc <= m_maxtdc) {
        ctdcgood++;
        sumtdcgood = sumtdcgood + md_tdc;
      } else ctdcbad++;

    }
    double tdcfrac = ctdcgood / 2.99;
    setEpicsPV("tdcboards", tdcfrac);
    c_hist_tdc->Clear();
    c_hist_tdc->cd();
    if (ctdcgood > 0)sumtdcgood = sumtdcgood * 1.0 / ctdcgood;
    getHistStyle(m_hist_tdc, "tdc", sumtdcgood);
    m_hist_tdc->SetTitle(Form("TDC Medians: Bad board count = %d (%0.01f%%)", ctdcbad - 1, 100.0 - tdcfrac));
    m_hist_tdc->Draw("");
    m_line_ltdc->Draw("same");
    m_line_htdc->Draw("same");
    c_hist_tdc->Update();
    UpdateCanvas(c_hist_tdc);
  }

  B2DEBUG(20, "DQMHistAnalysisCDCEpics: end event");
}

//------------------------------------
void DQMHistAnalysisCDCEpicsModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisCDCEpics: end run");
}

//------------------------------------
float DQMHistAnalysisCDCEpicsModule::getHistMedian(TH1D* h) const
{
  TH1D* hist = (TH1D*)h->Clone();
  hist->SetBinContent(1, 0.0); // Exclude 0-th bin
  float median = 0.0;
  if (hist->GetMean() != 0) {
    // Avoid an error if only TCD/ADC=0 entries
    double quantiles[1] = {0.0}; // One element to store median
    double probSums[1] = {0.5}; // Median definition
    hist->GetQuantiles(1, quantiles, probSums);
    median = quantiles[0];
  }
  delete hist;
  return median;
}

//------------------------------------
void DQMHistAnalysisCDCEpicsModule::terminate()
{
  B2DEBUG(20, "DQMHistAnalysisCDCEpics: terminate called");
}
