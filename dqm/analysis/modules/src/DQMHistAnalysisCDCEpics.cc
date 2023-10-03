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
  addParam("MinEvt", m_minevt, "Min events for intra-run point", 10000);
  addParam("MinADC", m_minadc, "Min ADC median acceptable", 70.0);
  addParam("MaxADC", m_maxadc, "max ADC median acceptable", 130.0);
  addParam("MinTDC", m_mintdc, "min TDC median acceptable", 4600.0);
  addParam("MaxTDC", m_maxtdc, "max TDC median acceptable", 4900.0);

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
  m_hist_adc = new TH1F("hist_adc", "m_hist_adc", 300, 0, 300);
  m_hist_adc->SetTitle("ADC Medians of CDC boards; CDC board index; ADC medians");

  m_hist_tdc = new TH1F("hist_tdc", "m_hist_tdc", 300, 0, 300);
  m_hist_tdc->SetTitle("TDC Medians of CDC boards; CDC board index; TDC medians");

  m_cBoards = new TCanvas("cdc_boardstatus", "cdc_boardstatus", 1000, 400);
  m_cBoards->Divide(1, 2);

  if (!hasDeltaPar(m_histoDir, m_histoADC))
    addDeltaPar(m_histoDir, m_histoADC, HistDelta::c_Entries, m_minevt, 1);

  if (!hasDeltaPar(m_histoDir, m_histoTDC))
    addDeltaPar(m_histoDir, m_histoTDC, HistDelta::c_Entries, m_minevt, 1);

  registerEpicsPV(m_pvPrefix + "cdcboards_wadc", "adcboards");
  registerEpicsPV(m_pvPrefix + "cdcboards_wtdc", "tdcboards");

  //creating box for normal adc and tdc windows
  m_boxadc = new TBox(0, m_minadc, 300, m_maxadc);
  m_boxadc->SetFillColorAlpha(kYellow, 0.40);
  m_boxadc->SetLineColor(kGray + 3);

  m_boxtdc = new TBox(0, m_mintdc, 300, m_maxtdc);
  m_boxtdc->SetFillColorAlpha(kYellow, 0.40);
  m_boxtdc->SetLineColor(kGray + 3);

  B2DEBUG(20, "DQMHistAnalysisCDCEpics: initialized.");
}

void DQMHistAnalysisCDCEpicsModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisCDCEpics: beginRun run called");
}

void DQMHistAnalysisCDCEpicsModule::event()
{
  //get intra-run histogram from CDC DQM module
  m_delta_adc = (TH2F*)getDelta(m_histoDir, m_histoADC, 0, true); //true=only if updated
  m_delta_tdc = (TH2F*)getDelta(m_histoDir, m_histoTDC, 0, true);
  if (m_delta_adc == nullptr || m_delta_tdc == nullptr) {
    if (m_delta_adc == nullptr)B2INFO("Histogram of hADCs not found (intra-run). Exiting");
    if (m_delta_tdc == nullptr)B2INFO("Histogram of hTDCs not found (intra-run). Exiting");
    return;
  }

  int cadcgood = 0, ctdcgood = 0;
  double sumadcgood = 0, sumtdcgood = 0;
  for (int ic = 0; ic < 300; ++ic) {
    m_hADCs[ic] = m_delta_adc->ProjectionY(Form("hADC%d", ic + 1), ic + 1, ic + 1, "");
    m_hADCs[ic]->SetTitle(Form("hADC%d", ic));
    float md_adc = getHistMedian(m_hADCs[ic]);
    if (md_adc >= m_minadc && md_adc <= m_maxadc) {
      sumadcgood = sumadcgood + md_adc;
      cadcgood++;
    }

    m_hTDCs[ic] = m_delta_tdc->ProjectionY(Form("hTDC%d", ic + 1), ic + 1, ic + 1, "");
    m_hTDCs[ic]->SetTitle(Form("hTDC%d", ic));
    float md_tdc = getHistMedian(m_hTDCs[ic]);
    if (md_tdc >= m_mintdc && md_tdc <= m_maxtdc) {
      sumtdcgood = sumtdcgood + md_tdc;
      ctdcgood++;
    }
  }

  double adcfrac = cadcgood / 3.0; // (100.0/300) in %
  setEpicsPV("adcboards", adcfrac);

  double tdcfrac = ctdcgood / 3.0;
  setEpicsPV("tdcboards", tdcfrac);

  updateEpicsPVs(5.0); // 5 is time in seconds
  B2DEBUG(20, "DQMHistAnalysisCDCEpics: end event");
}

void DQMHistAnalysisCDCEpicsModule::endRun()
{
  //run integrated histogram from CDC Module
  auto hrun_adc = (TH2F*)findHist("CDC/hADC");// only if updated
  auto hrun_tdc = (TH2F*)findHist("CDC/hTDC");
  if (hrun_adc == nullptr || hrun_tdc == nullptr) {
    if (hrun_adc == nullptr)B2INFO("Histogram hADC not found (integrated-run)");
    if (hrun_tdc == nullptr)B2INFO("Histogram hTDC not found (integrated-run)");
    return;
  }

  m_hist_adc->Reset();
  m_hist_tdc->Reset();
  for (int ic = 0; ic < 300; ++ic) {
    auto hadc_proj = (TH1D*)hrun_adc->ProjectionY(Form("hrunADC%d", ic + 1), ic + 1, ic + 1, "");
    float md_adc_run = getHistMedian(hadc_proj);
    m_hist_adc->SetBinContent(ic + 1, md_adc_run);

    auto htdc_proj = hrun_tdc->ProjectionY(Form("hrunADC%d", ic + 1), ic + 1, ic + 1, "");
    float md_tdc_run = getHistMedian(htdc_proj);
    m_hist_tdc->SetBinContent(ic + 1, md_tdc_run);
  }

  // Draw canvas
  m_cBoards->Clear();
  m_cBoards->Divide(2, 1);
  m_cBoards->cd(1);// necessary!
  getHistStyle(m_hist_adc, 2, 20);
  m_boxadc->Draw();
  m_hist_adc->Draw();
  m_cBoards->cd(2);// necessary!
  getHistStyle(m_hist_tdc, 4, 21);
  m_boxtdc->Draw();
  m_hist_tdc->Draw();

  B2DEBUG(20, "DQMHistAnalysisCDCEpics: end run");
}

float DQMHistAnalysisCDCEpicsModule::getHistMedian(TH1D* h) const
{
  TH1D* hist = (TH1D*)h->Clone();
  hist->SetBinContent(1, 0.0); // Exclude 0-th bin
  if (hist->GetMean() == 0) {return 0.0;} // Avoid an error if only TCD/ADC=0 entries
  double quantiles[1] = {0.0}; // One element to store median
  double probSums[1] = {0.5}; // Median definition
  hist->GetQuantiles(1, quantiles, probSums);
  float median = quantiles[0];
  return median;
}

void DQMHistAnalysisCDCEpicsModule::terminate()
{
  B2DEBUG(20, "DQMHistAnalysisCDCEpics: terminate called");
}