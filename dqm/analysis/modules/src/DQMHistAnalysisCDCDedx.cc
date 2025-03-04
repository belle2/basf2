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
  addParam("mmode", mmode, "default monitoring mode is basic", std::string("basic"));

  Double_t stops[NRGBs] = { 0.00, 0.00, 0.34, 0.61, 0.84, 1.00 };
  Double_t red[NRGBs]   = { 0.00, 0.00, 0.00, 0.87, 1.00, 0.51 };
  Double_t green[NRGBs] = { 0.00, 0.00, 0.81, 1.00, 0.20, 0.00 };
  Double_t blue[NRGBs]  = { 0.00, 0.51, 1.00, 0.12, 0.00, 0.00 };
  m_pal = TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont, false);
  for (auto i = 0; i < NCont; i++) m_palarr[i] = m_pal + i;
}


//---------------------------------------------
void DQMHistAnalysisCDCDedxModule::initialize()
{

  gStyle->SetOptStat(0);
  gROOT->cd();

  B2DEBUG(20, "DQMHistAnalysisCDCDedx: initialized.");

  //intra-run cavnas 1D
  c_ir_dedx = new TCanvas("CDCDedx/c_ir_dedx", "", 1400, 500);

  //per run canvas
  c_pr_dedx = new TCanvas("CDCDedx/c_CDCdedxMean", "", 800, 800);

  f_gaus = new TF1("f_gaus", "gaus", 0.0, 2.5);
  f_gaus->SetLineColor(kRed);

  l_line = new TLine();
  l_line->SetLineColor(kRed);
  l_line->SetLineStyle(1);
  l_line->SetLineWidth(2);

  m_pave = new TPaveText();
  m_pave->SetBorderSize(0);

  m_lego = new TLegend(0.45, 0.77, 0.60, 0.89);

  //Monitoring object for run dependency at mirabelle
  m_monObj = getMonitoringObject("cdcdedx");
  m_monObj->addCanvas(c_ir_dedx);
  m_monObj->addCanvas(c_pr_dedx);

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

  c_pr_dedx->Clear();
  if (mmode != "basic") {
    c_pr_dedx->SetWindowSize(1400, 800);
    c_pr_dedx->Divide(3, 3);
  } else {
    c_pr_dedx->SetWindowSize(1000, 800);
    c_pr_dedx->Divide(3, 2);
  }

  //Plot 1 dE/dx per run gain/reso
  drawDedxPR();

  // Plot 2  dE/dx bands vs p
  drawBandPlot();

  // Plot 3/4 dE/dx vs phi and costh
  drawDedxCosPhi();

  // Plot 5/6 dE/dx mean/reso vs inject time
  drawDedxInjTimeBin();

  c_pr_dedx->Modified();
  c_pr_dedx->Update();

  c_ir_dedx->Clear();
  if (mmode != "basic") {
    c_ir_dedx->SetWindowSize(900, 400);
    c_ir_dedx->Divide(3, 1);
  } else {
    c_ir_dedx->SetWindowSize(900, 400);
    c_ir_dedx->Divide(2, 1);
  }

  //Plot 1 dE/dx intra-run gain/reso
  drawDedxIR();

  c_ir_dedx->Modified();
  c_ir_dedx->Update();

  //Plot 7/8 wire status/ injection time
  if (mmode != "basic") {
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

  delete c_pr_dedx;
  delete c_ir_dedx;
  delete l_line;
  delete m_pave;
  delete h_dEdxIRMean;
  delete h_dEdxIRSigma;
  delete h_SigmaHer;
  delete h_MeanHer;
  delete h_MeanLer;
  delete h_SigmaLer;
  delete h_dEdxIRInd;
  delete m_lego;
}

//-------------------------------------------
void DQMHistAnalysisCDCDedxModule::getMetadata()
{

  TH1D* h_Meta = (TH1D*)findHist("CDCDedx/hMeta");

  if (h_Meta != nullptr) {
    m_nallevt = int(h_Meta->GetBinContent(1));
    m_nbhabhaevt = int(h_Meta->GetBinContent(2));
    m_nhadevt = int(h_Meta->GetBinContent(3));

    std::string m_title = h_Meta->GetTitle();

    first = m_title.find("Exp:");
    last = m_title.find(", Run:");
    std::string expN = m_title.substr(first + 4, last - first - 4);

    first = m_title.find(", Run:");
    last = m_title.find(", RG:");
    std::string runN = m_title.substr(first + 6, last - first - 6);

    first = m_title.find(", RG:");
    last = m_title.find(")");
    std::string runGain = m_title.substr(first + 5, last - first - 5);

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

    double m_meanerr = 0.0;
    double m_sigmaerr = 0.0;

    c_pr_dedx->cd(1);
    l_line->Clear();

    m_status = "LowStats";

    if (h_dEdx->Integral() > 250) {
      fitHistogram(h_dEdx, m_status);
      if (m_status == "OK") {
        m_mean = h_dEdx->GetFunction("f_gaus")->GetParameter(1);
        m_sigma = h_dEdx->GetFunction("f_gaus")->GetParameter(2);
        m_meanerr = h_dEdx->GetFunction("f_gaus")->GetParError(1);
        m_sigmaerr = h_dEdx->GetFunction("f_gaus")->GetParError(2);
      }
      // }
      setHistStyle(h_dEdx);
      h_dEdx->SetTitle("CDC-dEdx");
      h_dEdx->Draw("");
      //Draw line for dE/dx mean
      l_line->DrawLine(m_mean, 0, m_mean, h_dEdx->GetMaximum());
    }

    m_monObj->setVariable("CDCDedxMean", m_mean);
    m_monObj->setVariable("CDCDedxReso", m_sigma);
    m_monObj->setVariable("CDCDedxMeanErr", m_meanerr);
    m_monObj->setVariable("CDCDedxResoErr", m_sigmaerr);

    f_gaus->SetLineColor(kRed);
    f_gaus->DrawClone("same");

    m_pave->Clear();

    // TPaveText* pinfo0= new TPaveText(0.12, 0.72, 0.37, 0.89, "NBNDC");
    m_pave->SetX1NDC(0.12); m_pave->SetX2NDC(0.37); m_pave->SetY1NDC(0.72); m_pave->SetY2NDC(0.89);
    setTextStyle(m_pave);
    m_pave->AddText(Form("Fit Status: %s", m_status.data()));
    m_pave->AddText(Form("Fit #mu^{dE/dx}: %0.3f ", m_mean));
    m_pave->AddText(Form("Fit #sigma^{dE/dx}: %0.3f ", m_sigma));
    m_pave->SetTextColor(kRed);
    m_pave->DrawClone("same");

    m_pave->Clear();
    m_pave->SetX1NDC(0.60); m_pave->SetX2NDC(0.85); m_pave->SetY1NDC(0.69); m_pave->SetY2NDC(0.89);

    setTextStyle(m_pave);
    m_pave->AddText(Form("-- Expert info"));
    m_pave->AddText(Form("-------------"));
    setBEvtInfo(m_pave);
    m_pave->AddText(Form("Prev Gain: %0.03f", m_dbrg));
    m_pave->DrawClone("same");

    m_status.clear();
  }
}

//--------------------------------------------
void DQMHistAnalysisCDCDedxModule::drawDedxIR()
{

  //1. Draw Scattered plot
  if (mmode != "basic") {

    c_ir_dedx->cd(3);

    TH2D* hdEdxIRScat = (TH2D*)findHist("CDCDedx/hdEdxvsEvt");
    if (hdEdxIRScat != nullptr) {

      setPadStyle(0.143, 0.045, 0.077, 0.0);

      if (hdEdxIRScat->GetEntries() > 0) {
        hdEdxIRScat->GetXaxis()->SetRange(hdEdxIRScat->FindFirstBinAbove(0, 1), hdEdxIRScat->FindLastBinAbove(0, 1));
      }

      setHistStyle(hdEdxIRScat);
      hdEdxIRScat->Draw("");

      m_pave->Clear();
      m_pave->SetX1NDC(0.609); m_pave->SetX2NDC(0.942); m_pave->SetY1NDC(0.710); m_pave->SetY2NDC(0.911);

      setTextStyle(m_pave);
      m_pave->AddText("CDC-dE/dx Intra-run");
      setBEvtInfo(m_pave);
      m_pave->DrawClone("same");
    }

  }

  //Intra rungain/reso variation
  TH2D* hdEdxIRScatC = (TH2D*)findHist("CDCDedx/hdEdxvsEvt");

  if (hdEdxIRScatC != nullptr) {

    c_ir_dedx->cd(1);
    gPad->SetGridy(1);

    int fbin = hdEdxIRScatC->FindFirstBinAbove(0, 1);
    int lbin = hdEdxIRScatC->FindLastBinAbove(0, 1);
    int nbin = lbin - fbin + 1;

    if (nbin <= 0) nbin = 1;

    delete h_dEdxIRMean;
    h_dEdxIRMean = new TH1F("h_dEdxIRMean", "", nbin, 0.5, nbin + 0.5);
    h_dEdxIRMean->SetTitle("CDC-dE/dx gain(#mu): intra-run variation;Events(M);dE/dx (#mu_{fit})");

    delete h_dEdxIRSigma;
    h_dEdxIRSigma = new TH1F("h_dEdxIRSigma", "", nbin, 0.5, nbin + 0.5);
    h_dEdxIRSigma->SetTitle("CDC-dE/dx reso.(#sigma): intra-run variation;Events(M);dE/dx (#sigma_{fit})");

    setHistPars(hdEdxIRScatC, h_dEdxIRMean, h_dEdxIRSigma, nbin);

    //2 intra-gain trend

    drawHistPars(h_dEdxIRMean, nbin, m_mean, 0.10, "#mu_{fit}");

    //3 intra-resolution trend
    c_ir_dedx->cd(2);
    gPad->SetGridy(1);

    drawHistPars(h_dEdxIRSigma, nbin, m_sigma, 0.04, "#sigma_{fit}");
  }

}

//-----------------------------------------------
void DQMHistAnalysisCDCDedxModule::drawBandPlot()
{
  //Draw Scattered plot
  TH2D* hdEdxVsP = (TH2D*)findHist("CDCDedx/hdEdxVsP");
  if (hdEdxVsP != nullptr) {

    c_pr_dedx->cd(2);
    gPad->SetLogx();
    gPad->SetLogy();

    setPlotStyle();
    setHistStyle(hdEdxVsP);
    hdEdxVsP->SetTitle("CDC-dEdx band plot");
    hdEdxVsP->SetMinimum(0.10);
    hdEdxVsP->Draw("col");

    m_pave->Clear();
    m_pave->SetX1NDC(0.60); m_pave->SetX2NDC(0.85); m_pave->SetY1NDC(0.77); m_pave->SetY2NDC(0.89);

    setTextStyle(m_pave);
    m_pave->AddText(Form("IP tracks (hadron)"));
    m_pave->AddText(Form("Exp/Run: %d/%d", m_exp, m_run));
    if (m_nhadevt > 1e5)
      m_pave->AddText(Form("Events: %0.02fM", double(m_nhadevt / 1e6)));
    if (m_nhadevt > 1e3)
      m_pave->AddText(Form("Events: %0.02fK", double(m_nhadevt / 1e3)));
    else
      m_pave->AddText(Form("Events: %d", m_nhadevt));
    m_pave->DrawClone("same");
  }
}


//---------------------------------------------
void DQMHistAnalysisCDCDedxModule::drawDedxCosPhi()
{

  TH2D* hdEdxvsPhi = (TH2D*)findHist("CDCDedx/hdEdxvsPhi");
  if (hdEdxvsPhi != nullptr) {

    c_pr_dedx->cd(3);

    setHistStyle(hdEdxvsPhi);
    hdEdxvsPhi->SetTitle("CDC-dEdx vs Phi");
    hdEdxvsPhi->Draw("col");

    l_line->DrawLine(-3.20, m_mean, 3.20, m_mean);

    m_pave->Clear();
    m_pave->SetX1NDC(0.60); m_pave->SetX2NDC(0.85); m_pave->SetY1NDC(0.77); m_pave->SetY2NDC(0.89);

    setTextStyle(m_pave);
    setBEvtInfo(m_pave);
    m_pave->DrawClone("same");
  }

  //plot # 2
  TH2D* hdEdxvsCosth = (TH2D*)findHist("CDCDedx/hdEdxvsCosth");
  if (hdEdxvsCosth != nullptr) {

    c_pr_dedx->cd(4);

    setHistStyle(hdEdxvsCosth);
    hdEdxvsCosth->SetTitle("CDC-dEdx vs Costh");
    hdEdxvsCosth->Draw("col");

    l_line->DrawLine(-1.0, m_mean, 1.0, m_mean);

    m_pave->Clear();
    m_pave->SetX1NDC(0.60); m_pave->SetX2NDC(0.85); m_pave->SetY1NDC(0.77); m_pave->SetY2NDC(0.89);

    setTextStyle(m_pave);
    setBEvtInfo(m_pave);
    m_pave->DrawClone("same");
  }
}

//-----------------------------------------------
void DQMHistAnalysisCDCDedxModule::drawDedxInjTime()
{

  TH2D* hinjtimeHer = (TH2D*)findHist("CDCDedx/hinjtimeHer");
  TH2D* hinjtimeLer = (TH2D*)findHist("CDCDedx/hinjtimeLer");

  if (hinjtimeHer != nullptr && hinjtimeLer != nullptr) {

    c_pr_dedx->cd(7);

    setPlotStyle();
    setHistStyle(hinjtimeHer);
    hinjtimeHer->SetTitle("Time since last injection (HER)");
    hinjtimeHer->DrawClone("box");

    setHistStyle(hinjtimeLer);
    hinjtimeLer->SetFillColor(kRed);
    hinjtimeLer->DrawClone("same box");

    l_line->DrawLine(0, m_mean, 80e3, m_mean);

    m_lego->Clear();
    m_lego->AddEntry(hinjtimeHer, "HER", "f");
    m_lego->AddEntry(hinjtimeLer, "LER", "f");
    m_lego->DrawClone("same");

    m_pave->Clear();
    m_pave->SetX1NDC(0.60); m_pave->SetX2NDC(0.85); m_pave->SetY1NDC(0.77); m_pave->SetY2NDC(0.89);

    setTextStyle(m_pave);
    setBEvtInfo(m_pave);
    m_pave->DrawClone("same");
  }
}

//---------------------------------------------
void DQMHistAnalysisCDCDedxModule::drawDedxInjTimeBin()
{

  //Injection time variation
  TH2D* hdEdxITHer = (TH2D*)findHist("CDCDedx/hinjtimeHer");
  TH2D* hdEdxITLer = (TH2D*)findHist("CDCDedx/hinjtimeLer");

  if (hdEdxITHer != nullptr && hdEdxITLer != nullptr) {

    c_pr_dedx->cd(5);

    int fbin = hdEdxITHer->FindFirstBinAbove(0, 1);
    int lbin = hdEdxITHer->FindLastBinAbove(0, 1);
    int nbin = (lbin - fbin + 1) / 2;
    if (nbin <= 0)nbin = 1;

    delete h_SigmaHer;
    delete h_MeanHer;
    delete h_MeanLer;
    delete h_SigmaLer;
    h_MeanHer = new TH1F("h_MeanHer", "CDC-dE/dx gain(#mu);Injection time (ms);dE/dx (#mu_{fit})", nbin, 0.5, nbin + 0.5);
    h_SigmaHer = new TH1F("h_SigmaHer", "CDC-dE/dx reso.(#sigma);Injection time (ms);dE/dx (#sigma_{fit})", nbin, 0.5, nbin + 0.5);

    h_MeanLer = new TH1F("h_MeanLer", "CDC-dE/dx gain(#mu);Injection time (ms);dE/dx (#mu_{fit})", nbin, 0.5, nbin + 0.5);
    h_SigmaLer = new TH1F("h_SigmaLer", "CDC-dE/dx reso.(#sigma);Injection time (ms);dE/dx (#sigma_{fit})", nbin, 0.5, nbin + 0.5);

    setHistPars(hdEdxITHer, h_MeanHer, h_SigmaHer, nbin);
    setHistPars(hdEdxITLer, h_MeanLer, h_SigmaLer, nbin);

    gPad->SetGridy(1);
    drawHistPars(h_MeanHer, nbin, m_mean, 0.40, "#mu_{fit}");

    h_MeanLer->SetMarkerColor(kRed);
    h_MeanLer->SetMarkerStyle(20);
    h_MeanLer->SetMarkerSize(1.10);
    h_MeanLer->Draw("same");


    m_lego->Clear();

    m_lego->AddEntry(h_MeanHer, "HER", "p");
    m_lego->AddEntry(h_MeanLer, "LER", "p");
    m_lego->DrawClone("same");

    c_pr_dedx->cd(6);
    gPad->SetGridy(1);

    drawHistPars(h_SigmaHer, nbin, m_sigma, 0.15, "#sigma_{fit}");

    h_SigmaLer->SetMarkerColor(kRed);
    h_SigmaLer->SetMarkerStyle(20);
    h_SigmaLer->SetMarkerSize(1.10);
    h_SigmaLer->Draw("same");

    m_lego->Clear();
    m_lego->AddEntry(h_SigmaHer, "HER", "p");
    m_lego->AddEntry(h_SigmaLer, "LER", "p");
    m_lego->DrawClone("same");
  }
}

//------------------------------------------------
void DQMHistAnalysisCDCDedxModule::drawWireStatus()
{

  //Draw Scattered plot
  TH2D* hWires = (TH2D*)findHist("CDCDedx/hWires");
  TH2D* hWireStatus = (TH2D*)findHist("CDCDedx/hWireStatus");
  if (hWires != nullptr && hWireStatus != nullptr) {

    c_pr_dedx->cd(8);
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

    m_pave->Clear();
    m_pave->SetX1NDC(0.117); m_pave->SetX2NDC(0.148); m_pave->SetY1NDC(0.832); m_pave->SetY2NDC(0.976);

    setTextStyle(m_pave);
    m_pave->AddText(Form("CDC Wire Status"));
    m_pave->AddText(Form("Exp/Run: %d/%d", m_exp, m_run));
    m_pave->AddText(Form("Dead: %d (%0.02f%%)", m_ndead, (100.0 * m_ndead / c_nSenseWires)));
    if (m_nallevt > 1e5)
      m_pave->AddText(Form("Events: %0.02fM", double(m_nallevt / 1e6)));
    if (m_nallevt > 1e3)
      m_pave->AddText(Form("Events: %0.02fK", double(m_nallevt / 1e3)));
    else
      m_pave->AddText(Form("Events: %d", m_nallevt));
    m_pave->DrawClone("same");
  }
}

//-----------------------------------------------
void DQMHistAnalysisCDCDedxModule::setHistPars(TH2D*& hdEdx, TH1F*& hmean, TH1F*& hsigma, int nbin)
{

  int fbin = hdEdx->FindFirstBinAbove(0, 1);

  for (int ibin = 0; ibin < nbin; ibin++) {
    int localbin = ibin + fbin;
    delete h_dEdxIRInd;
    h_dEdxIRInd = (TH1*)hdEdx->ProjectionY(Form("htemp_%d", localbin), localbin, localbin);

    double mean = 0.0, meanerr = 0.0;
    double sigma = 0.0, sigmaerr = 0.0;

    fitHistogram(h_dEdxIRInd, m_status);

    if (m_status == "OK") {
      mean = h_dEdxIRInd->GetFunction("f_gaus")->GetParameter(1);
      meanerr = h_dEdxIRInd->GetFunction("f_gaus")->GetParError(1);
      sigma = h_dEdxIRInd->GetFunction("f_gaus")->GetParameter(2);
      sigmaerr = h_dEdxIRInd->GetFunction("f_gaus")->GetParError(2);
    }

    hmean->SetBinContent(ibin + 1, mean);
    hmean->SetBinError(ibin + 1, meanerr);
    hsigma->SetBinContent(ibin + 1, sigma);
    hsigma->SetBinError(ibin + 1, sigmaerr);
  }

}

//-----------------------------------------------
void DQMHistAnalysisCDCDedxModule::drawHistPars(TH1F*& hist, int nbin, double pars, double fac, std::string var)
{
  std::string hname = hist->GetName();
  setPadStyle(0.143, 0.045, 0.077, 0.0);
  hist->SetMarkerColor(kBlue);
  hist->SetMarkerStyle(20);
  hist->SetMarkerSize(1.10);
  hist->GetYaxis()->SetRangeUser(pars - fac, pars + fac); //m_mean - 0.10, m_mean + 0.10);
  hist->Draw("");

  l_line->DrawLine(0.5, pars, hist->GetXaxis()->GetBinUpEdge(nbin), pars);

  m_pave->Clear();
  m_pave->SetX1NDC(0.609); m_pave->SetX2NDC(0.942); m_pave->SetY1NDC(0.680); m_pave->SetY2NDC(0.911);

  setTextStyle(m_pave);
  if (hname == "h_dEdxIRMean" || hname == "h_dEdxIRSigma")  m_pave->AddText("Intra-run variation");
  setBEvtInfo(m_pave);
  m_pave->AddText(Form("Avg %s: %0.3f", var.data(), pars));
  m_pave->DrawClone("same");
}

//-----------------------------------------------
void DQMHistAnalysisCDCDedxModule::setBEvtInfo(TPaveText*& pt)
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
  gStyle->SetNumberContours(NCont);
  TColor::SetPalette(m_pal, m_palarr);
}


//------------------------------------------------------------------
void DQMHistAnalysisCDCDedxModule::setTextStyle(TPaveText*& obj)
{

  obj->SetFillColor(0);
  obj->SetFillStyle(0);

  obj->SetLineColor(TColor::GetColor("#000000"));
  obj->SetLineWidth(0);
  obj->SetTextAlign(12);

  obj->SetTextColor(kGray + 3);
  obj->SetTextFont(82);
  obj->SetTextSize(0.03157895);
  obj->SetTextAlign(12);

}

//------------------------------------------------------------------
void DQMHistAnalysisCDCDedxModule::setHistStyle(TH1* obj)
{

  obj->SetStats(0);
  obj->SetTitle("");
  obj->SetFillColor(kYellow);

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
