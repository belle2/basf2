/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <dqm/analysis/modules/DQMHistAnalysisCDCEpics.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <TLatex.h>

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
  addParam("HistPhiIndex", m_histoPhiIndex, "Phi Index Histogram Name", std::string("hPhiIndex"));
  addParam("HistPhiEff", m_histoPhiEff, "Phi Eff Histogram Name", std::string("hPhiEff"));
  addParam("HistCellEff", m_histoCellEff, "Cell Eff Histogram Name", std::string("hCellEff"));
  addParam("PvPrefix", m_pvPrefix, "PV Prefix and Name", std::string("CDC:"));
  addParam("RefFilePhi", m_refNamePhi, "Reference histogram file name", std::string("CDCDQM_PhiRef.root"));
  addParam("RefDirectory", m_refDir, "Reference histogram dir", std::string("ref/CDC/default"));
  addParam("MinEvt", m_minevt, "Min events for intra-run point", 20000);
  addParam("FirstEffBoundary", m_firstEffBoundary, "The first boundary of the efficiency range", m_firstEffBoundary);
  addParam("SecondEffBoundary", m_secondEffBoundary, "The second boundary of the efficiency range", m_secondEffBoundary);
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
  c_hist_adc = new TCanvas("CDC/c_hist_adc", "c_hist_adc", 500, 400);
  m_hist_adc = new TH1F("CDC/hist_adc", "m_hist_adc", 300, 0, 300);
  m_hist_adc->SetTitle("ADC Medians; CDC board index; ADC medians");

  c_hist_tdc = new TCanvas("CDC/c_hist_tdc", "c_hist_tdc", 500, 400);
  m_hist_tdc = new TH1F("CDC/hist_tdc", "m_hist_tdc", 300, 0, 300);
  m_hist_tdc->SetTitle("TDC Medians; CDC board index; TDC medians");

  //array of various phi histograms
  c_hist_skimphi = new TCanvas("CDC/c_hist_skimphi", "c_hist_skimphi", 1600, 800);

  c_hist_crphi = new TCanvas("CDC/c_hist_crphi", "c_hist_crphi", 500, 400);

  //CR alram reference
  if (m_refNamePhi != "") {
    m_fileRefPhi = TFile::Open(m_refNamePhi.data(), "READ");
    if (m_fileRefPhi && m_fileRefPhi->IsOpen()) {
      B2INFO("DQMHistAnalysisCDCEpics: reference (" << m_refNamePhi << ") found OK");
      m_histref_phiindex = (TH2F*)m_fileRefPhi->Get((m_refDir + "/hPhiIndex").data());
      if (!m_histref_phiindex)B2INFO("\t .. but (histogram) not found");
      else B2INFO("\t ..and (cdcdqm_phiref) also exist");
    }
  }

  c_hist_effphi = new TCanvas("CDC/c_hist_effphi", "c_hist_effphi", 500, 400);
  m_hist_effphi = new TH1D("CDC/hist_effphi", "m_hist_effphi", 360, -180.0, 180.0);

  c_hist_efficiency = new TCanvas("CDC/c_hist_efficiency", "c_hist_efficiency", 840, 800);
  m_hist_efficiency[0] = createEffiTH2Poly("CDC/hist_observedCellHits", "hist_observedCellHits;X [cm];Y [cm]; Track / bin");
  m_hist_efficiency[0]->GetYaxis()->SetTitleOffset(1.4);
  m_hist_efficiency[0]->SetDirectory(gDirectory);
  m_hist_efficiency[1] = (TH2Poly*)m_hist_efficiency[0]->Clone();
  m_hist_efficiency[1]->SetNameTitle("CDC/hist_expectedCellHits", "hist_expectedCellHits;X [cm];Y [cm]; Track / bin");
  m_hist_efficiency[1]->SetDirectory(gDirectory);
  m_hist_efficiency[2] = (TH2Poly*)m_hist_efficiency[0]->Clone();
  m_hist_efficiency[2]->SetNameTitle("CDC/hist_cellEfficiency", "hist_cellEfficiency;X [cm];Y [cm]; Efficiency");
  m_hist_efficiency[2]->SetDirectory(gDirectory);
  m_hist_cellEff1D = new TH1F("CDC/hist_cellEfficiency1D", "hist_cellEfficiency1D;Cell Efficiency;Cell / bin", 208, -0.02, 1.02);
  m_hist_cellEff1D->GetYaxis()->SetTitleOffset(1.4);

  if (!hasDeltaPar(m_histoDir, m_histoADC))
    addDeltaPar(m_histoDir, m_histoADC, HistDelta::c_Entries, m_minevt, 1);

  if (!hasDeltaPar(m_histoDir, m_histoTDC))
    addDeltaPar(m_histoDir, m_histoTDC, HistDelta::c_Entries, m_minevt, 1);

  if (!hasDeltaPar(m_histoDir, m_histoPhiIndex))
    addDeltaPar(m_histoDir, m_histoPhiIndex, HistDelta::c_Entries, m_minevt, 1);

  if (!hasDeltaPar(m_histoDir, m_histoPhiEff))
    addDeltaPar(m_histoDir, m_histoPhiEff, HistDelta::c_Entries, m_minevt, 1);

  if (!hasDeltaPar(m_histoDir, m_histoCellEff))
    addDeltaPar(m_histoDir, m_histoCellEff, HistDelta::c_Entries, m_minevt, 1);

  registerEpicsPV(m_pvPrefix + "cdcboards_wadc", "adcboards");
  registerEpicsPV(m_pvPrefix + "cdcboards_wtdc", "tdcboards");

  registerEpicsPV(m_pvPrefix + "adc_median_window", "adcmedianwindow");
  registerEpicsPV(m_pvPrefix + "tdc_median_window", "tdcmedianwindow");

  registerEpicsPV(m_pvPrefix + "phi_compare_window", "phicomparewindow");

  m_monObj = getMonitoringObject("cdc");

  B2DEBUG(20, "DQMHistAnalysisCDCEpics: initialized.");
}

void DQMHistAnalysisCDCEpicsModule::beginRun()
{
  double unused = 0;
  requestLimitsFromEpicsPVs("adcmedianwindow", unused, m_minadc, m_maxadc, unused);
  requestLimitsFromEpicsPVs("tdcmedianwindow", unused, m_mintdc, m_maxtdc, unused);
  requestLimitsFromEpicsPVs("phicomparewindow", m_phialarm, m_phiwarn, unused, unused);

  //in case if something is wrong in config file
  if (std::isnan(m_minadc)) m_minadc = 60.0;
  if (std::isnan(m_maxadc)) m_maxadc = 130.0;
  if (std::isnan(m_mintdc)) m_mintdc = 4600.0;
  if (std::isnan(m_maxtdc)) m_maxtdc = 5000.0;

  if (std::isnan(m_phiwarn)) m_phiwarn = 0.05; //>%5 is warning
  if (std::isnan(m_phialarm)) m_phialarm = 0.15; //>%15 is warning

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

  //get phi plots for various options
  auto m_delta_skimphi = (TH2F*)getDelta(m_histoDir, m_histoPhiIndex, 0, true); //true=only if updated
  c_hist_skimphi->Clear();
  c_hist_skimphi->Divide(4, 2);
  if (m_delta_skimphi) {
    TString sip[2] = {"OffIP", "IP"};
    TString sname[4] = {"all", "bhabha", "hadron", "mumutrk"};
    for (int j = 0; j < 2; j++) { //ip selections
      for (int i = 0; i < 4; i++) { //skim selections
        int k = 4 * j + i; //0 to 7
        TString hname = TString::Format("histphi_%s_%sevt", sip[j].Data(), sname[i].Data());
        m_hist_skimphi[k] = m_delta_skimphi->ProjectionX(hname, k + 1, k + 1, "");
        m_hist_skimphi[k]->SetTitle(TString::Format("cdc-track #phi (%s, %s-events);#phi;entries", sip[j].Data(), sname[i].Data()));
        if (k < 4)m_hist_skimphi[k]->SetFillColor(kGray);
        else m_hist_skimphi[k]->SetFillColor(kCyan);
        c_hist_skimphi->cd(k + 1);
        gPad->SetGridx(1);
        gPad->SetGridy(1);
        m_hist_skimphi[k]->Draw("hist");
      }
    }
  }

  //for CR shifter IP + all hadrons including alarm system
  if (m_delta_skimphi) {
    c_hist_crphi->Clear();
    bool isFew = false, isAlarm = false, isWarn = false;
    m_hist_crphi = m_delta_skimphi->ProjectionX("histphi_ip_hadrons", 7, 7, "");
    m_hist_crphi->SetTitle("cdc-track #phi (IP + hadrons);cdc-track #phi;norm entries");
    if (m_hist_crphi) {
      double maxnow = m_hist_crphi->Integral();
      m_hist_crphi->Scale(1.0 / maxnow);
      if (maxnow < 10000) isFew = true;
      else {
        if (m_histref_phiindex) {
          m_hist_refphi = m_histref_phiindex->ProjectionX("histphi_ip_hadronsref", 7, 7, "");
          double nbinref = m_hist_refphi->GetNbinsX();
          double nbinnow = m_hist_crphi->GetNbinsX();
          if (nbinref == nbinnow) { //same bins
            double maxref = m_hist_refphi->Integral();
            m_hist_refphi->Scale(1.0 / maxref);
            double maxphidiff = 0;
            double maxphidiff_angle = 0;
            for (int iphi = 0; iphi < nbinnow; iphi++) {
              double icnow = m_hist_crphi->GetBinContent(iphi + 1);
              double icref = m_hist_refphi->GetBinContent(iphi + 1);
              double phidiff = fabs(icnow - icref);
              if (phidiff > m_phiwarn)isWarn = true;
              if (phidiff > m_phialarm)isAlarm = true;
              if (phidiff > maxphidiff) {
                maxphidiff = phidiff;
                maxphidiff_angle = m_hist_crphi->GetBinLowEdge(iphi + 1) + m_hist_crphi->GetBinWidth(iphi + 1);
              }
            }
            m_hist_crphi->SetTitle(Form("%s (diff = %0.03f at %0.1f)", m_hist_crphi->GetTitle(), maxphidiff, maxphidiff_angle));
          }
        }
      }
    }
    c_hist_crphi->cd();
    gPad->SetGridx(1);
    gPad->SetGridy(1);
    if (!m_histref_phiindex)m_hist_crphi->SetTitle(Form("%s (no ref file)", m_hist_crphi->GetTitle()));
    m_hist_crphi->Draw("hist");
    if (isFew) colorizeCanvas(c_hist_crphi, c_StatusTooFew);
    else if (isAlarm)colorizeCanvas(c_hist_crphi, c_StatusError);
    else if (isWarn)colorizeCanvas(c_hist_crphi, c_StatusWarning);
    else colorizeCanvas(c_hist_crphi, c_StatusGood);
    c_hist_crphi->Update();
    UpdateCanvas(c_hist_crphi);
  }

  //get tracking efficiency
  auto m_delta_effphi = (TH2F*)getDelta(m_histoDir, m_histoPhiEff, 0, true); //true=only if updated
  c_hist_effphi->Clear();
  if (m_delta_effphi) {
    double eff = -1;
    const int all_phibins = m_delta_effphi->GetNbinsX();
    const int all_hitbins = m_delta_effphi->GetNbinsY();
    const int thr_hitbin = m_delta_effphi->GetYaxis()->FindBin(20);//min hits bin
    for (int iphi = 0; iphi < all_phibins; iphi++) {
      TH1D* temp = (TH1D*)m_delta_effphi->ProjectionY(Form("hhits_bin_%d", iphi + 1), iphi + 1, iphi + 1, "");
      Double_t num = temp->Integral(thr_hitbin, all_hitbins);
      Double_t den = temp->Integral();
      if (den > 0)eff = num * 100.0 / den;
      m_hist_effphi->SetBinContent(iphi + 1, eff);
      m_hist_effphi->SetBinError(iphi + 1, 0);
    }
    m_hist_effphi->GetYaxis()->SetRangeUser(80.0, 110.0); //per efficiency
    m_hist_effphi->SetTitle("Tracking efficiency via cdchits (>20/all); cdc-track #phi; efficiency");
    c_hist_effphi->cd();
    gPad->SetGridx();
    gPad->SetGridy();
    m_hist_effphi->SetFillColor(kCyan);
    m_hist_effphi->Draw("hist");
    c_hist_effphi->Update();
    UpdateCanvas(c_hist_effphi);
  }

  // get cell efficiency
  double meanCellEff = 0;
  double cellsWithLowEff = 0;
  double cellsWithMidEff = 0;
  double cellsWithHighEff = 0;
  gStyle->SetNumberContours(100);
  auto m_delta_efflay = (TH2F*)getDelta(m_histoDir, m_histoCellEff, 0, true); //true=only if updated
  c_hist_efficiency->Clear();
  if (m_delta_efflay) {
    fillEffiTH2Poly(m_delta_efflay, m_hist_efficiency[0], m_hist_efficiency[1], m_hist_efficiency[2]);
    c_hist_efficiency->Divide(2, 2);
    c_hist_efficiency->cd(1);
    gPad->SetRightMargin(0.05 + gPad->GetRightMargin());
    m_hist_efficiency[0]->SetStats(0);
    m_hist_efficiency[0]->Draw("COLZ");
    c_hist_efficiency->cd(2);
    gPad->SetRightMargin(0.05 + gPad->GetRightMargin());
    m_hist_efficiency[1]->SetStats(0);
    m_hist_efficiency[1]->Draw("COLZ");
    c_hist_efficiency->cd(3);
    gPad->SetRightMargin(0.05 + gPad->GetRightMargin());
    int nEffiValues = 0;
    for (int ij = 0; ij < m_hist_efficiency[2]->GetNumberOfBins(); ij++) {
      if (m_hist_efficiency[1]->GetBinContent(ij + 1) == 0) continue;
      double binEffi = m_hist_efficiency[2]->GetBinContent(ij + 1);
      m_hist_cellEff1D->Fill(binEffi);
      meanCellEff += binEffi;
      nEffiValues++;
    }
    if (nEffiValues) meanCellEff /= nEffiValues;
    m_hist_efficiency[2]->SetMinimum(0.0);
    m_hist_efficiency[2]->SetMaximum(1.0);
    m_hist_efficiency[2]->SetStats(0);
    m_hist_efficiency[2]->Draw("COLZ");
    TLatex latex;
    latex.SetTextSize(0.025);
    latex.DrawLatexNDC(0.12, 0.87, TString::Format("mean = %.3f%%", meanCellEff * 100.0));
    c_hist_efficiency->cd(4);
    if (nEffiValues) {
      int firstBoundaryBin = m_hist_cellEff1D->GetXaxis()->FindBin(m_firstEffBoundary) - 1;
      cellsWithLowEff = m_hist_cellEff1D->Integral(1, firstBoundaryBin) / nEffiValues;
      int secondBoundaryBin = m_hist_cellEff1D->GetXaxis()->FindBin(m_secondEffBoundary) - 1;
      cellsWithMidEff = m_hist_cellEff1D->Integral(firstBoundaryBin + 1, secondBoundaryBin) / nEffiValues;
      cellsWithHighEff =  m_hist_cellEff1D->Integral(secondBoundaryBin + 1, m_hist_cellEff1D->GetNbinsX()) / nEffiValues;
      m_hist_cellEff1D->SetStats(0);
      m_hist_cellEff1D->Draw();
      latex.DrawLatexNDC(0.15, 0.87, TString::Format("%06.3f%% cell : eff < %.2f",
                                                     cellsWithLowEff * 100,
                                                     m_firstEffBoundary));
      latex.DrawLatexNDC(0.15, 0.84, TString::Format("%06.3f%% cell : %.2f < eff < %.2f",
                                                     cellsWithMidEff * 100,
                                                     m_firstEffBoundary, m_secondEffBoundary));
      latex.DrawLatexNDC(0.15, 0.81, TString::Format("%06.3f%% cell : %.2f < eff",
                                                     cellsWithHighEff * 100,
                                                     m_secondEffBoundary));
    }
    c_hist_efficiency->Update();
    UpdateCanvas(c_hist_efficiency);
  }

  m_monObj->setVariable("meanCellEff", meanCellEff);
  m_monObj->setVariable("cellsWithLowEff", cellsWithLowEff);
  m_monObj->setVariable("cellsWithMidEff", cellsWithMidEff);
  m_monObj->setVariable("cellsWithHighEff", cellsWithHighEff);

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


TH2Poly* DQMHistAnalysisCDCEpicsModule::createEffiTH2Poly(const TString& name, const TString& title)
{
  TH2Poly* hist = new TH2Poly();
  static CDC::CDCGeometryPar& cdcgeo = CDC::CDCGeometryPar::Instance();
  int nSLayers = cdcgeo.getNumberOfSenseLayers();
  for (int lay = 0; lay < nSLayers; lay++) {
    int nWires = cdcgeo.nWiresInLayer(lay);
    double offset = cdcgeo.offset(lay);
    double layerR = cdcgeo.senseWireR(lay);
    double r_inner = 0;
    double r_outer = 0;
    if (lay == 0) {
      r_inner = layerR - (cdcgeo.senseWireR(1) - cdcgeo.senseWireR(0)) / 2;
      r_outer = layerR + (cdcgeo.senseWireR(1) - cdcgeo.senseWireR(0)) / 2;
    } else if (lay == nSLayers - 1) {
      r_inner = layerR - (cdcgeo.senseWireR(lay) - cdcgeo.senseWireR(lay - 1)) / 2;
      r_outer = layerR + (cdcgeo.senseWireR(lay) - cdcgeo.senseWireR(lay - 1)) / 2;
    } else {
      r_inner = layerR - (cdcgeo.senseWireR(lay) - cdcgeo.senseWireR(lay - 1)) / 2;
      r_outer = layerR + (cdcgeo.senseWireR(lay + 1) - cdcgeo.senseWireR(lay)) / 2;
    }
    for (int wire = 0; wire < nWires; wire++) {
      double phi_inner = (wire - 0.5 + offset) * 2 * TMath::Pi() / nWires;
      double phi_outer = (wire + 0.5 + offset) * 2 * TMath::Pi() / nWires;
      // Calculate the four corners of the bin
      double x0 = r_inner * TMath::Cos(phi_inner);
      double y0 = r_inner * TMath::Sin(phi_inner);
      double x1 = r_outer * TMath::Cos(phi_inner);
      double y1 = r_outer * TMath::Sin(phi_inner);
      double x2 = r_outer * TMath::Cos(phi_outer);
      double y2 = r_outer * TMath::Sin(phi_outer);
      double x3 = r_inner * TMath::Cos(phi_outer);
      double y3 = r_inner * TMath::Sin(phi_outer);
      double xx[] = {x0, x1, x2, x3};
      double yy[] = {y0, y1, y2, y3};
      hist->AddBin(4, xx, yy);
    }
  }
  hist->SetNameTitle(name, title);
  return hist;
}

void DQMHistAnalysisCDCEpicsModule::fillEffiTH2Poly(TH2F* hist, TH2Poly* observed, TH2Poly* expected, TH2Poly* efficiency)
{
  static CDC::CDCGeometryPar& cdcgeo = CDC::CDCGeometryPar::Instance();
  int nSLayers = cdcgeo.getNumberOfSenseLayers();
  for (int lay = 0; lay < nSLayers; lay++) {
    int nWires = cdcgeo.nWiresInLayer(lay);
    double layerR = cdcgeo.senseWireR(lay);
    double offset = cdcgeo.offset(lay);
    int expBin = hist->GetYaxis()->FindBin(lay);
    int obsBin = expBin + nSLayers;
    // fill the bins for this layer
    for (int wire = 0; wire < nWires; wire++) {
      double phi = (wire + offset) * 2 * TMath::Pi() / nWires;
      double fillX = layerR * TMath::Cos(phi);
      double fillY = layerR * TMath::Sin(phi);
      observed->Fill(fillX, fillY, hist->GetBinContent(wire + 1, obsBin));
      expected->Fill(fillX, fillY, hist->GetBinContent(wire + 1, expBin));
    }
  }
  efficiency->Divide(observed, expected);
}
