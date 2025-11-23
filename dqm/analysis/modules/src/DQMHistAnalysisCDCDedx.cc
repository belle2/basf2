/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <dqm/analysis/modules/DQMHistAnalysisCDCDedx.h>
#include <vector>

using namespace std;
using namespace Belle2;
using boost::format;

REG_MODULE(DQMHistAnalysisCDCDedx);

//-----------------------------------------------------------------
DQMHistAnalysisCDCDedxModule::DQMHistAnalysisCDCDedxModule()
  : DQMHistAnalysisModule()
{
  B2DEBUG(20, "DQMHistAnalysisCDCDedx: Constructor done.");
  setDescription("Module to draw and compute values related to dEdx for CDC. ");

  //Parameter definition here
  addParam("mmode", m_mode, "default monitoring mode is basic", std::string("basic"));

  Double_t stops[m_NRGBs] = { 0.00, 0.00, 0.34, 0.61, 0.84, 1.00 };
  Double_t red[m_NRGBs]   = { 0.00, 0.00, 0.00, 0.87, 1.00, 0.51 };
  Double_t green[m_NRGBs] = { 0.00, 0.00, 0.81, 1.00, 0.20, 0.00 };
  Double_t blue[m_NRGBs]  = { 0.00, 0.51, 1.00, 0.12, 0.00, 0.00 };
  m_pal = TColor::CreateGradientColorTable(m_NRGBs, stops, red, green, blue, m_NCont, false);
  for (auto i = 0; i < m_NCont; i++) m_palarr[i] = m_pal + i;
}


//---------------------------------------------
void DQMHistAnalysisCDCDedxModule::initialize()
{

  gStyle->SetOptStat(0);
  gROOT->cd();

  B2DEBUG(20, "DQMHistAnalysisCDCDedx: initialized.");

  //intra-run cavnas 1D
  m_c_ir_dedx = new TCanvas("CDCDedx/m_c_ir_dedx", "", 1400, 500);

  //per run canvas
  m_c_pr_dedx = new TCanvas("CDCDedx/c_CDCdedxMean", "", 800, 800);

  f_gaus = new TF1("f_gaus", "gaus", 0.0, 2.5);
  f_gaus->SetLineColor(kRed);

  l_line = new TLine();
  l_line->SetLineColor(kRed);
  l_line->SetLineStyle(1);
  l_line->SetLineWidth(2);

  m_text_dedx = new TPaveText(0.60, 0.60, 0.85, 0.89, "NBNDC");
  m_text_dedx->SetBorderSize(0);

  m_text_dedx_fit = new TPaveText(0.12, 0.72, 0.37, 0.89, "NBNDC");
  m_text_dedx_fit->SetBorderSize(0);

  m_text_bandplot = new TPaveText(0.60, 0.72, 0.85, 0.89, "NBNDC");
  m_text_bandplot->SetBorderSize(0);

  m_text_dedxWire = new TPaveText(0.60, 0.72, 0.85, 0.89, "NBNDC");
  m_text_dedxWire->SetBorderSize(0);

  m_text_dedx_ir = new TPaveText(0.609, 0.720, 0.942, 0.911, "NBNDC");
  m_text_dedx_ir->SetBorderSize(0);

  m_text_mean = new TPaveText(0.60, 0.82, 0.85, 0.89, "NBNDC");
  m_text_mean->SetBorderSize(0);

  m_text_sigma = new TPaveText(0.60, 0.82, 0.85, 0.89, "NBNDC");
  m_text_sigma->SetBorderSize(0);

  m_lego = new TLegend(0.45, 0.77, 0.60, 0.89);
  m_legoI = new TLegend(0.40, 0.77, 0.55, 0.89);

  //Monitoring object for run dependency at mirabelle
  m_monObj = getMonitoringObject("cdcdedx");
  m_monObj->addCanvas(m_c_ir_dedx);
  m_monObj->addCanvas(m_c_pr_dedx);
}


//-------------------------------------------
void DQMHistAnalysisCDCDedxModule::beginRun()
{

  B2DEBUG(20, "DQMHistAnalysisCDCDedx: beginRun called.");

}


//----------------------------------------
void DQMHistAnalysisCDCDedxModule::event()
{

  B2DEBUG(20, "DQMHistAnalysisCDCDedx: event called.");

  //Plot 0 getmeta those are useful for cosmetics
  getMetadata();

  m_c_pr_dedx->Clear();
  if (m_mode != "basic") {
    m_c_pr_dedx->SetWindowSize(1400, 800);
    m_c_pr_dedx->Divide(3, 3);
  } else {
    m_c_pr_dedx->SetWindowSize(1000, 800);
    m_c_pr_dedx->Divide(3, 2);
  }

  //Plot 1 dE/dx per run gain/reso
  drawDedxPR();

  // Plot 2  dE/dx bands vs p
  drawBandPlot();

  // Plot 3/4 dE/dx vs phi and costh
  drawDedxCosPhi();

  // Plot 5/6 dE/dx mean/reso vs inject time
  drawDedxInjTimeBin();

  m_c_pr_dedx->Modified();
  m_c_pr_dedx->Update();

  m_c_ir_dedx->Clear();
  if (m_mode != "basic") {
    m_c_ir_dedx->SetWindowSize(900, 400);
    m_c_ir_dedx->Divide(3, 1);
  } else {
    m_c_ir_dedx->SetWindowSize(900, 400);
    m_c_ir_dedx->Divide(2, 1);
  }

  //Plot 1 dE/dx intra-run gain/reso
  drawDedxIR();

  m_c_ir_dedx->Modified();
  m_c_ir_dedx->Update();

  //Plot 7/8 wire status/ injection time
  if (m_mode != "basic") {
    drawDedxInjTime();
    drawWireStatus();
  }
}

//-----------------------------------------
void DQMHistAnalysisCDCDedxModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisCDCDedx : endRun called");
}

//--------------------------------------------
void DQMHistAnalysisCDCDedxModule::terminate()
{

  B2DEBUG(20, "DQMHistAnalysisCDCDedx : terminate called");

  delete m_c_pr_dedx;
  delete m_c_ir_dedx;
  delete l_line;
  delete m_hdEdxIRMean;
  delete m_hdEdxIRSigma;
  delete m_hSigmaHer;
  delete m_hMeanHer;
  delete m_hMeanLer;
  delete m_hSigmaLer;
  delete m_hdEdxIRInd;
  delete m_lego;
  delete m_legoI;
  delete m_text_dedx_fit;
  delete m_text_dedxWire;
  delete m_text_bandplot;
  delete m_text_dedx;
  delete m_text_dedx_ir;
  delete m_text_mean;
  delete m_text_sigma;

}

//-------------------------------------------
void DQMHistAnalysisCDCDedxModule::getMetadata()
{

  TH1D* h_Meta = (TH1D*)findHist("CDCDedx/hMeta");

  if (h_Meta != nullptr) {
    m_nallevt = int(h_Meta->GetBinContent(1));
    m_nbhabhaevt = int(h_Meta->GetBinContent(2));
    m_nhadevt = int(h_Meta->GetBinContent(3));

    std::string title = h_Meta->GetTitle();

    m_first = title.find("Exp:");
    m_last = title.find(", Run:");
    std::string expN = title.substr(m_first + 4, m_last - m_first - 4);

    m_first = title.find(", Run:");
    m_last = title.find(", RG:");
    std::string runN = title.substr(m_first + 6, m_last - m_first - 6);

    m_first = title.find(", RG:");
    m_last = title.find(")");
    std::string runGain = title.substr(m_first + 5, m_last - m_first - 5);

    m_exp = std::stoi(expN.c_str());
    m_run = std::stoi(runN.c_str());
    m_dbrg = std::stof(runGain.c_str());
  }

}

//-------------------------------------------
void DQMHistAnalysisCDCDedxModule::drawDedxPR()
{

  m_mean = 0.0;
  m_sigma = 0.0;

  TH1* h_dEdx = findHist("CDCDedx/hdEdx");
  if (h_dEdx != nullptr) {

    double meanerr = 0.0;
    double sigmaerr = 0.0;

    m_c_pr_dedx->cd(1);
    l_line->Clear();

    m_status = "LowStats";

    if (h_dEdx->Integral() > 250) {
      fitHistogram(h_dEdx, m_status);
      if (m_status == "OK") {
        m_mean = h_dEdx->GetFunction("f_gaus")->GetParameter(1);
        m_sigma = h_dEdx->GetFunction("f_gaus")->GetParameter(2);
        meanerr = h_dEdx->GetFunction("f_gaus")->GetParError(1);
        sigmaerr = h_dEdx->GetFunction("f_gaus")->GetParError(2);
      }
      setHistStyle(h_dEdx);
      h_dEdx->SetFillColor(kYellow);
      h_dEdx->SetTitle("CDC-dEdx");
      h_dEdx->Draw();

      //Draw line for dE/dx mean
      l_line->DrawLine(m_mean, 0, m_mean, h_dEdx->GetMaximum());
    }

    m_monObj->setVariable("CDCDedxMean", m_mean);
    m_monObj->setVariable("CDCDedxReso", m_sigma);
    m_monObj->setVariable("CDCDedxMeanErr", meanerr);
    m_monObj->setVariable("CDCDedxResoErr", sigmaerr);

    m_text_dedx_fit->Clear();
    m_text_dedx->Clear();

    setTextStyle(m_text_dedx_fit);
    m_text_dedx_fit->AddText(Form("Fit Status: %s", m_status.data()));
    m_text_dedx_fit->AddText(Form("Fit #mu^{dE/dx}: %0.3f ", m_mean));
    m_text_dedx_fit->AddText(Form("Fit #sigma^{dE/dx}: %0.3f ", m_sigma));
    m_text_dedx_fit->SetTextColor(kRed);
    m_text_dedx_fit->Draw();

    setTextStyle(m_text_dedx);
    m_text_dedx->AddText(Form("-- Expert info"));
    m_text_dedx->AddText(Form("-------------"));
    setBEvtInfo(m_text_dedx);
    m_text_dedx->AddText(Form("Prev Gain: %0.03f", m_dbrg));
    m_text_dedx->Draw();

    m_c_pr_dedx->Draw();
    m_c_pr_dedx->Modified();
    m_c_pr_dedx->Update();

    m_status.clear();

  }

  m_text_mean->Clear();
  setTextStyle(m_text_mean);
  m_text_mean->AddText(Form("Avg #mu_{fit}: %0.3f", m_mean));

  m_text_sigma->Clear();
  setTextStyle(m_text_sigma);
  m_text_sigma->AddText(Form("Avg #sigma_{fit}: %0.3f", m_sigma));
}

//--------------------------------------------
void DQMHistAnalysisCDCDedxModule::drawDedxIR()
{

  //1. Draw Scattered plot
  if (m_mode != "basic") {

    m_c_ir_dedx->cd(3);

    TH2D* hdEdxIRScat = (TH2D*)findHist("CDCDedx/hdEdxvsEvt");
    if (hdEdxIRScat != nullptr) {

      setPadStyle(0.143, 0.045, 0.077, 0.0);

      if (hdEdxIRScat->GetEntries() > 0) {
        hdEdxIRScat->GetXaxis()->SetRange(hdEdxIRScat->FindFirstBinAbove(0, 1), hdEdxIRScat->FindLastBinAbove(0, 1));
      }

      setHistStyle(hdEdxIRScat);
      hdEdxIRScat->SetTitle("CDC-dE/dx Intra-run variation");
      hdEdxIRScat->Draw("");

      m_text_dedx_ir->Clear();

      setTextStyle(m_text_dedx_ir);
      m_text_dedx_ir->AddText("CDC-dE/dx Intra-run");
      setBEvtInfo(m_text_dedx_ir);
      m_text_dedx_ir->Draw("same");
    }

  }

  //Intra rungain/reso variation
  TH2D* hdEdxIRScatC = (TH2D*)findHist("CDCDedx/hdEdxvsEvt");

  if (hdEdxIRScatC != nullptr) {

    m_c_ir_dedx->cd(1);
    gPad->SetGridy(1);

    int fbin = hdEdxIRScatC->FindFirstBinAbove(0, 1);
    int lbin = hdEdxIRScatC->FindLastBinAbove(0, 1);
    int nbin = lbin - fbin + 1;

    if (nbin <= 0) nbin = 1;

    delete m_hdEdxIRMean;
    m_hdEdxIRMean = new TH1F("m_hdEdxIRMean", "", nbin, 0.5, nbin + 0.5);
    m_hdEdxIRMean->SetTitle("CDC-dE/dx gain(#mu): intra-run variation;Events(M);dE/dx (#mu_{fit})");

    delete m_hdEdxIRSigma;
    m_hdEdxIRSigma = new TH1F("m_hdEdxIRSigma", "", nbin, 0.5, nbin + 0.5);
    m_hdEdxIRSigma->SetTitle("CDC-dE/dx reso.(#sigma): intra-run variation;Events(M);dE/dx (#sigma_{fit})");

    setHistPars(hdEdxIRScatC, m_hdEdxIRMean, m_hdEdxIRSigma, nbin);

    //2 intra-gain trend
    setPadStyle(0.143, 0.045, 0.077, 0.0);
    m_hdEdxIRMean->SetMarkerColor(kBlue);
    m_hdEdxIRMean->GetYaxis()->SetRangeUser(m_mean - 0.10, m_mean + 0.10);
    m_hdEdxIRMean->Draw("");

    l_line->DrawLine(0.5, m_mean, m_hdEdxIRMean->GetXaxis()->GetBinUpEdge(nbin), m_mean);

    m_text_mean->Draw("same");


    //3 intra-resolution trend
    m_c_ir_dedx->cd(2);
    gPad->SetGridy(1);

    setPadStyle(0.143, 0.045, 0.077, 0.0);
    m_hdEdxIRSigma->SetMarkerColor(kBlue);
    m_hdEdxIRSigma->GetYaxis()->SetRangeUser(m_sigma - 0.04, m_sigma + 0.04);
    m_hdEdxIRSigma->Draw("");

    l_line->DrawLine(0.5, m_sigma, m_hdEdxIRSigma->GetXaxis()->GetBinUpEdge(nbin), m_sigma);

    m_text_sigma->Draw("same");
  }

}

//-----------------------------------------------
void DQMHistAnalysisCDCDedxModule::drawBandPlot()
{
  //Draw Scattered plot
  TH2D* hdEdxVsP = (TH2D*)findHist("CDCDedx/hdEdxVsP");
  if (hdEdxVsP != nullptr) {

    m_c_pr_dedx->cd(2);
    gPad->SetLogx();
    gPad->SetLogy();

    setPlotStyle();
    setHistStyle(hdEdxVsP);
    hdEdxVsP->SetTitle("CDC-dEdx band plot");
    hdEdxVsP->SetMinimum(0.10);
    hdEdxVsP->Draw("col");

    m_text_bandplot->Clear();

    setTextStyle(m_text_bandplot);
    m_text_bandplot->AddText(Form("IP tracks (hadron)"));
    m_text_bandplot->AddText(Form("Exp/Run: %d/%d", m_exp, m_run));
    if (m_nhadevt > 1e5)
      m_text_bandplot->AddText(Form("Events: %0.02fM", double(m_nhadevt / 1e6)));
    if (m_nhadevt > 1e3)
      m_text_bandplot->AddText(Form("Events: %0.02fK", double(m_nhadevt / 1e3)));
    else
      m_text_bandplot->AddText(Form("Events: %d", m_nhadevt));
    m_text_bandplot->Draw("same");
  }
}


//---------------------------------------------
void DQMHistAnalysisCDCDedxModule::drawDedxCosPhi()
{

  TH2D* hdEdxvsPhi = (TH2D*)findHist("CDCDedx/hdEdxvsPhi");
  if (hdEdxvsPhi != nullptr) {

    m_c_pr_dedx->cd(3);

    setHistStyle(hdEdxvsPhi);
    hdEdxvsPhi->SetTitle("CDC-dEdx vs Phi");
    hdEdxvsPhi->Draw("col");

    l_line->DrawLine(-3.20, m_mean, 3.20, m_mean);

  }

  //plot # 2
  TH2D* hdEdxvsCosth = (TH2D*)findHist("CDCDedx/hdEdxvsCosth");
  if (hdEdxvsCosth != nullptr) {

    m_c_pr_dedx->cd(4);

    setHistStyle(hdEdxvsCosth);
    hdEdxvsCosth->SetTitle("CDC-dEdx vs Costh");
    hdEdxvsCosth->Draw("col");

    l_line->DrawLine(-1.0, m_mean, 1.0, m_mean);
  }
}

//-----------------------------------------------
void DQMHistAnalysisCDCDedxModule::drawDedxInjTime()
{

  TH2D* hinjtimeHer = (TH2D*)findHist("CDCDedx/hinjtimeHer");
  TH2D* hinjtimeLer = (TH2D*)findHist("CDCDedx/hinjtimeLer");

  if (hinjtimeHer != nullptr && hinjtimeLer != nullptr) {

    m_c_pr_dedx->cd(7);

    setPlotStyle();
    setHistStyle(hinjtimeHer);
    hinjtimeHer->SetFillColor(kBlue);
    hinjtimeHer->SetTitle("Time since m_last injection (HER)");
    hinjtimeHer->Draw("box");

    setHistStyle(hinjtimeLer);
    hinjtimeLer->SetFillColor(kRed);
    hinjtimeLer->Draw("same box");

    l_line->DrawLine(0, m_mean, 80e3, m_mean);

    m_lego->Clear();
    m_lego->AddEntry(hinjtimeHer, "HER", "f");
    m_lego->AddEntry(hinjtimeLer, "LER", "f");
    m_lego->Draw("same");

  }
}

//---------------------------------------------
void DQMHistAnalysisCDCDedxModule::drawDedxInjTimeBin()
{

  //Injection time variation
  TH2D* hdEdxITHer = (TH2D*)findHist("CDCDedx/hinjtimeHer");
  TH2D* hdEdxITLer = (TH2D*)findHist("CDCDedx/hinjtimeLer");

  if (hdEdxITHer != nullptr && hdEdxITLer != nullptr) {

    m_c_pr_dedx->cd(5);

    int fbin = hdEdxITHer->FindFirstBinAbove(0, 1);
    int lbin = hdEdxITHer->FindLastBinAbove(0, 1);
    int nbin = (lbin - fbin + 1) / 2;
    if (nbin <= 0)nbin = 1;

    delete m_hSigmaHer;
    delete m_hMeanHer;
    delete m_hMeanLer;
    delete m_hSigmaLer;
    m_hMeanHer = new TH1F("m_hMeanHer", "CDC-dE/dx gain(#mu);Injection time (ms);dE/dx (#mu_{fit})", nbin, 0.5, nbin + 0.5);
    m_hSigmaHer = new TH1F("m_hSigmaHer", "CDC-dE/dx reso.(#sigma);Injection time (ms);dE/dx (#sigma_{fit})", nbin, 0.5, nbin + 0.5);

    m_hMeanLer = new TH1F("m_hMeanLer", "CDC-dE/dx gain(#mu);Injection time (ms);dE/dx (#mu_{fit})", nbin, 0.5, nbin + 0.5);
    m_hSigmaLer = new TH1F("m_hSigmaLer", "CDC-dE/dx reso.(#sigma);Injection time (ms);dE/dx (#sigma_{fit})", nbin, 0.5, nbin + 0.5);

    setHistPars(hdEdxITHer, m_hMeanHer, m_hSigmaHer, nbin);
    setHistPars(hdEdxITLer, m_hMeanLer, m_hSigmaLer, nbin);

    // dE/dx mean vs injection time
    gPad->SetGridy(1);
    setPadStyle(0.143, 0.045, 0.077, 0.0);
    m_hMeanHer->SetMarkerColor(kBlue);
    m_hMeanHer->GetYaxis()->SetRangeUser(m_mean - 0.40, m_mean + 0.40);
    m_hMeanHer->Draw("");

    l_line->DrawLine(0.5, m_mean, m_hMeanHer->GetXaxis()->GetBinUpEdge(nbin), m_mean);

    m_hMeanLer->SetMarkerColor(kRed);
    m_hMeanLer->Draw("same");
    m_text_mean->Draw("same");

    m_legoI->Clear();

    m_legoI->AddEntry(m_hMeanHer, "HER", "p");
    m_legoI->AddEntry(m_hMeanLer, "LER", "p");
    m_legoI->Draw("same");

    // dE/dx sigma vs injection time

    m_c_pr_dedx->cd(6);
    gPad->SetGridy(1);

    setPadStyle(0.143, 0.045, 0.077, 0.0);
    m_hSigmaHer->SetMarkerColor(kBlue);
    m_hSigmaHer->GetYaxis()->SetRangeUser(m_sigma - 0.15, m_sigma + 0.15);
    m_hSigmaHer->Draw("");

    l_line->DrawLine(0.5, m_sigma, m_hSigmaHer->GetXaxis()->GetBinUpEdge(nbin), m_sigma);

    m_hSigmaLer->SetMarkerColor(kRed);
    m_hSigmaLer->Draw("same");

    m_text_sigma->Draw("same");

    m_legoI->Draw("same");
  }
}

//------------------------------------------------
void DQMHistAnalysisCDCDedxModule::drawWireStatus()
{

  //Draw Scattered plot
  TH2D* hWires = (TH2D*)findHist("CDCDedx/hWires");
  TH2D* hWireStatus = (TH2D*)findHist("CDCDedx/hWireStatus");
  if (hWires != nullptr && hWireStatus != nullptr) {

    m_c_pr_dedx->cd(8);
    setHistStyle(hWires);
    hWires->SetMarkerColor(kGray);
    hWires->Draw("");

    std::string s_ndead = hWireStatus->GetTitle();
    int m_ndead = atof(s_ndead.c_str());
    m_monObj->setVariable("CDCDedxDeadWires", m_ndead);

    setHistStyle(hWireStatus);
    hWireStatus->SetMarkerColor(kRed);
    hWireStatus->SetMarkerStyle(7);
    hWireStatus->Draw("same");

    m_text_dedxWire->Clear();

    setTextStyle(m_text_dedxWire);
    m_text_dedxWire->AddText(Form("CDC Wire Status"));
    m_text_dedxWire->AddText(Form("Exp/Run: %d/%d", m_exp, m_run));
    m_text_dedxWire->AddText(Form("Dead: %d (%0.02f%%)", m_ndead, (100.0 * m_ndead / c_nSenseWires)));
    if (m_nallevt > 1e5)
      m_text_dedxWire->AddText(Form("Events: %0.02fM", double(m_nallevt / 1e6)));
    if (m_nallevt > 1e3)
      m_text_dedxWire->AddText(Form("Events: %0.02fK", double(m_nallevt / 1e3)));
    else
      m_text_dedxWire->AddText(Form("Events: %d", m_nallevt));
    m_text_dedxWire->Draw("same");
  }
}

//-----------------------------------------------
void DQMHistAnalysisCDCDedxModule::setHistPars(TH2D*& hdEdx, TH1F*& hmean, TH1F*& hsigma, int nbin)
{

  int fbin = hdEdx->FindFirstBinAbove(0, 1);

  for (int ibin = 0; ibin < nbin; ibin++) {
    int localbin = ibin + fbin;
    delete m_hdEdxIRInd;
    m_hdEdxIRInd = (TH1*)hdEdx->ProjectionY(Form("htemp_%d", localbin), localbin, localbin);

    double mean = 0.0, meanerr = 0.0;
    double sigma = 0.0, sigmaerr = 0.0;

    fitHistogram(m_hdEdxIRInd, m_status);

    if (m_status == "OK") {
      mean = m_hdEdxIRInd->GetFunction("f_gaus")->GetParameter(1);
      meanerr = m_hdEdxIRInd->GetFunction("f_gaus")->GetParError(1);
      sigma = m_hdEdxIRInd->GetFunction("f_gaus")->GetParameter(2);
      sigmaerr = m_hdEdxIRInd->GetFunction("f_gaus")->GetParError(2);
    }

    hmean->SetBinContent(ibin + 1, mean);
    hmean->SetBinError(ibin + 1, meanerr);
    hsigma->SetBinContent(ibin + 1, sigma);
    hsigma->SetBinError(ibin + 1, sigmaerr);
  }
  hmean->SetMarkerStyle(20);
  hmean->SetMarkerSize(1.10);
  hsigma->SetMarkerStyle(20);
  hsigma->SetMarkerSize(1.10);
}

//-----------------------------------------------
void DQMHistAnalysisCDCDedxModule::setBEvtInfo(TPaveText* pt)
{

  pt->AddText("CDC dE/dx (e^{+}e^{-})");
  pt->AddText(Form("Exp/Run: %d/%d", m_exp, m_run));
  if (m_nbhabhaevt > 1e5)
    pt->AddText(Form("Events: %0.02fM", double(m_nbhabhaevt / 1e6)));
  if (m_nbhabhaevt > 1e3)
    pt->AddText(Form("Events: %0.02fK", double(m_nbhabhaevt / 1e3)));
  else
    pt->AddText(Form("Events: %d", m_nbhabhaevt));
}

//----------------------------------------------------------------------------------------
void DQMHistAnalysisCDCDedxModule::fitHistogram(TH1*& temphist, std::string& status)
{

  if (temphist != nullptr) {
    temphist->GetXaxis()->SetRange(temphist->FindFirstBinAbove(0, 1), temphist->FindLastBinAbove(0, 1));
    int fs = temphist->Fit(f_gaus, "QR");
    if (fs != 0) {
      status = "Failed";
    } else {
      double mean = temphist->GetFunction("f_gaus")->GetParameter(1);
      double width = temphist->GetFunction("f_gaus")->GetParameter(2);
      temphist->GetXaxis()->SetRangeUser(mean - 5.0 * width, mean + 5.0 * width);
      fs = temphist->Fit(f_gaus, "QR", "", mean - 3.0 * width, mean + 3.0 * width);
      if (fs != 0)status = "Failed";
      else {
        temphist->GetXaxis()->SetRangeUser(mean - 5.0 * width, mean + 5.0 * width);
        status = "OK";
      }
    }
  }

}

//------------------------------------------------
void DQMHistAnalysisCDCDedxModule::setPlotStyle()
{
  gStyle->SetNumberContours(m_NCont);
  TColor::SetPalette(m_pal, m_palarr);
}


//------------------------------------------------------------------
void DQMHistAnalysisCDCDedxModule::setTextStyle(TPaveText* obj)
{

  obj->SetFillColor(TColor::GetColor("#FFFF99"));  // Light yellow background for visibility
  obj->SetFillStyle(1001);  // Solid fill
  obj->SetLineColor(TColor::GetColor("#000000"));
  obj->SetLineWidth(1);  // Add a border for visibility
  obj->SetTextAlign(12);
  obj->SetTextColor(kBlack);  // Set text color to black for better contrast
  obj->SetTextFont(82);
  obj->SetTextSize(0.03157895);

}

//------------------------------------------------------------------
void DQMHistAnalysisCDCDedxModule::setHistStyle(TH1* obj)
{

  obj->SetStats(0);
  obj->SetTitle("");

  obj->SetTitleOffset(1.15, "x");
  obj->SetTitleSize(.040, "x");
  obj->SetTitleOffset(1.15, "y");
  obj->SetTitleSize(.040, "y");

  obj->SetLabelOffset(0.015, "x");
  obj->SetLabelSize(.040, "x");
  obj->SetLabelOffset(0.015, "y");
  obj->SetLabelSize(.040, "y");

  obj->SetTickLength(0.03, "x");
  obj->SetTickLength(0.02, "y");

}

//-------------------------------------------------------------------------------------
void DQMHistAnalysisCDCDedxModule::setPadStyle(double l, double r, double t, double b)
{

  if (l != 0)gPad->SetLeftMargin(l);
  if (r != 0)gPad->SetRightMargin(r);
  if (t != 0)gPad->SetTopMargin(t);
  if (b != 0)gPad->SetBottomMargin(b);
  gPad->SetTickx(1);
  gPad->SetTicky(1);

}
