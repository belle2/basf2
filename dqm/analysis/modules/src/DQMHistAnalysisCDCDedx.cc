/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <dqm/analysis/modules/DQMHistAnalysisCDCDedx.h>

using namespace std;
using namespace Belle2;
using boost::format;

REG_MODULE(DQMHistAnalysisCDCDedx)

//-----------------------------------------------------------------
DQMHistAnalysisCDCDedxModule::DQMHistAnalysisCDCDedxModule()
  : DQMHistAnalysisModule()
{
  //Parameter definition here
  B2DEBUG(20, "DQMHistAnalysisCDCDedx: Constructor done.");
}

//--------------------------------------------------------------
DQMHistAnalysisCDCDedxModule::~DQMHistAnalysisCDCDedxModule()
{

}

//---------------------------------------------
void DQMHistAnalysisCDCDedxModule::initialize()
{

  gStyle->SetOptStat(0);
  gROOT->cd();

  B2DEBUG(20, "DQMHistAnalysisCDCDedx: initialized.");

  //per run canvas
  c_pr_dedx = new TCanvas("CDCDedx/c_pr_dedx", "", 700, 600);
  c_pr_wires = new TCanvas("CDCDedx/c_pr_wires", "", 700, 700);

  //2D plots
  c_pr_bands =  new TCanvas("CDCDedx/c_pr_bands", "", 700, 600);
  c_pr_dedxphi = new TCanvas("CDCDedx/c_pr_dedxphi", "", 700, 500);
  c_pr_dedxcos = new TCanvas("CDCDedx/c_pr_dedxcos", "", 700, 500);

  //intra-run cavnas
  c_ir_dedx = new TCanvas("CDCDedx/c_ir_dedx", "", 700, 500);
  c_ir_mean = new TCanvas("CDCDedx/c_ir_mean", "", 700, 500);
  c_ir_reso = new TCanvas("CDCDedx/c_ir_reso", "", 700, 500);

  f_gaus = new TF1("f_gaus", "gaus", 0.0, 2.5);
  f_gaus->SetLineColor(kRed);

  l_line = new TLine();
  l_line->SetLineColor(4);
  l_line->SetLineStyle(9);

  //Monitoring object for run dependency at mirabelle
  m_monObj = getMonitoringObject("cdcdedx");
  m_monObj->addCanvas(c_pr_dedx);
  m_monObj->addCanvas(c_pr_wires);
  m_monObj->addCanvas(c_pr_bands);
  m_monObj->addCanvas(c_pr_dedxphi);
  m_monObj->addCanvas(c_pr_dedxcos);
  m_monObj->addCanvas(c_ir_dedx);
  m_monObj->addCanvas(c_ir_mean);
  m_monObj->addCanvas(c_ir_reso);

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

  //Plot 1 dE/dx per run gain/reso
  drawDedx();

  //Plot 2 dE/dx intra-run gain/reso
  drawDedxIR();

  //Plot 3 wire status
  drawWireStatus();

  //Plot 4  dE/dx bands vs p
  drawBandPlot();

  //Plot 5 dE/dx vs phi and costh
  drawDedxCosPhi();

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
  delete c_pr_dedxphi;
  delete c_pr_dedxcos;
  delete c_pr_bands;
  delete c_pr_wires;
  delete c_ir_dedx;
  delete c_ir_mean;
  delete c_ir_reso;

}

//-------------------------------------------
void DQMHistAnalysisCDCDedxModule::getMetadata()
{

  TH1D* h_Meta = (TH1D*)findHist("CDCDedx/hMeta");
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

//-------------------------------------------
void DQMHistAnalysisCDCDedxModule::drawDedx()
{

  c_pr_dedx->Clear();
  l_line->Clear();

  m_status = "LowStats";
  m_mean = 0.0;
  m_sigma = 0.0;

  double m_meanerr = 0.0;
  double m_sigmaerr = 0.0;

  TH1* h_dEdx = findHist("CDCDedx/hdEdx");
  TH1D* h_dEdxClone = (TH1D*)h_dEdx->Clone("hdEdx_clone");
  if (h_dEdxClone->Integral() > 250) {
    fitHistogram(h_dEdxClone, m_status);
    if (m_status == "OK") {
      m_mean = h_dEdxClone->GetFunction("f_gaus")->GetParameter(1);
      m_sigma = h_dEdxClone->GetFunction("f_gaus")->GetParameter(2);
      m_meanerr = h_dEdxClone->GetFunction("f_gaus")->GetParError(1);
      m_sigmaerr = h_dEdxClone->GetFunction("f_gaus")->GetParError(2);
    }
  }

  m_monObj->setVariable("CDCDedxMean", m_mean);
  m_monObj->setVariable("CDCDedxReso", m_sigma);
  m_monObj->setVariable("CDCDedxMeanErr", m_meanerr);
  m_monObj->setVariable("CDCDedxResoErr", m_sigmaerr);

  c_pr_dedx->cd();
  set_Pad_Style(0.120, 0.046, 0.051, 0.0);
  set_Hist_Style(h_dEdxClone);
  h_dEdxClone->DrawCopy("");

  //Draw line for dE/dx mean
  l_line->DrawLine(m_mean, 0, m_mean, h_dEdxClone->GetMaximum());

  TF1* f_gausC = (TF1*)f_gaus->Clone("f_gausC");
  f_gausC->SetLineColor(kRed);
  f_gausC->DrawClone("same");


  TPaveText* pinfo0 = new TPaveText(0.15, 0.72, 0.37, 0.92, "brNDC");
  set_Text_Style(pinfo0);
  pinfo0->AddText(Form("CDC dE/dx (e^{-}e^{+})"));
  pinfo0->AddText(Form("Exp/Run: %d/%d", m_exp, m_run));
  pinfo0->AddText(Form("-------------"));
  pinfo0->AddText(Form("Fit Status: %s", m_status.data()));
  pinfo0->AddText(Form("Fit #mu^{dE/dx}: %0.3f ", m_mean));
  pinfo0->AddText(Form("Fit #sigma^{dE/dx}: %0.3f ", m_sigma));
  pinfo0->SetTextColor(kRed);
  pinfo0->Draw("same");


  TPaveText* pinfo1 = new TPaveText(0.68, 0.80, 0.90, 0.92, "brNDC");
  set_Text_Style(pinfo1);
  pinfo1->AddText(Form("-- Expert info"));
  pinfo1->AddText(Form("Prev Gain: %0.03f", m_dbrg));
  if (m_nbhabhaevt > 1e5)
    pinfo1->AddText(Form("Events: %0.02fM", double(m_nbhabhaevt / 1e6)));
  if (m_nbhabhaevt > 1e3)
    pinfo1->AddText(Form("Events: %0.02fK", double(m_nbhabhaevt / 1e3)));
  else
    pinfo1->AddText(Form("Events: %d", m_nbhabhaevt));
  pinfo1->Draw("same");

  m_status.clear();
  c_pr_dedx->Modified();
  c_pr_dedx->Update();

}

//--------------------------------------------
void DQMHistAnalysisCDCDedxModule::drawDedxIR()
{

  //1. Draw Scattered plot
  c_ir_dedx->Clear();
  c_ir_dedx->cd();
  set_Pad_Style(0.143, 0.045, 0.077, 0.0);

  TH2D* hdEdxIRScat = (TH2D*)findHist("CDCDedx/hdEdxvsEvt");
  set_Hist_Style(hdEdxIRScat);
  hdEdxIRScat->Draw("");
  TPaveText* pinfo = new TPaveText(0.689, 0.690, 0.942, 0.911, "brNDC");
  set_Text_Style(pinfo);
  pinfo->AddText("CDC-dE/dx Intra-run");
  pinfo->AddText("Electrons (e^{+}e^{-})");
  pinfo->AddText(Form("Exp/Run: %d/%d", m_exp, m_run));
  if (m_nbhabhaevt > 1e5)
    pinfo->AddText(Form("Events: %0.02fM", double(m_nbhabhaevt / 1e6)));
  if (m_nbhabhaevt > 1e3)
    pinfo->AddText(Form("Events: %0.02fK", double(m_nbhabhaevt / 1e3)));
  else
    pinfo->AddText(Form("Events: %d", m_nbhabhaevt));
  pinfo->Draw("same");

  c_ir_dedx->Modified();
  c_ir_dedx->Update();


  //Intra rungain/reso variation
  c_ir_mean->Clear();
  c_ir_mean->cd();
  gPad->SetGridy(1);

  TH2D* hdEdxIRScatC = (TH2D*)findHist("CDCDedx/hdEdxvsEvt");
  int fbin = hdEdxIRScatC->FindFirstBinAbove(0, 1);
  int lbin = hdEdxIRScatC->FindLastBinAbove(0, 1);
  int nbin = lbin - fbin + 1;

  TH1D* hdEdxIRInd[nbin];
  for (int ibin = 0; ibin < nbin; ibin++) {
    int localbin = ibin + fbin;
    hdEdxIRInd[ibin] = (TH1D*)hdEdxIRScatC->ProjectionY(Form("htemp_%d", localbin), localbin, localbin);
  }

  if (nbin <= 0)nbin = 1;
  TH1F* hdEdxIRMean = new TH1F("hdEdxIRMean", "", nbin, 0.5, nbin + 0.5);
  TH1F* hdEdxIRSigma = new TH1F("hdEdxIRSigma", "", nbin, 0.5, nbin + 0.5);

  for (int ibin = 0; ibin < nbin; ibin++) {

    double m_imean = 0.0, m_imeanerr = 0.0;
    double m_isigma = 0.0, m_isigmaerr = 0.0;
    fitHistogram(hdEdxIRInd[ibin], m_status);

    if (m_status == "OK") {
      m_imean = hdEdxIRInd[ibin]->GetFunction("f_gaus")->GetParameter(1);
      m_imeanerr = hdEdxIRInd[ibin]->GetFunction("f_gaus")->GetParError(1);
      m_isigma = hdEdxIRInd[ibin]->GetFunction("f_gaus")->GetParameter(2);
      m_isigmaerr = hdEdxIRInd[ibin]->GetFunction("f_gaus")->GetParError(2);
    }
    hdEdxIRMean->SetBinContent(ibin + 1, m_imean);
    hdEdxIRMean->SetBinError(ibin + 1, m_imeanerr);
    hdEdxIRSigma->SetBinContent(ibin + 1, m_isigma);
    hdEdxIRSigma->SetBinError(ibin + 1, m_isigmaerr);
  }

  //2 intra-gain trend
  set_Pad_Style(0.143, 0.045, 0.077, 0.0);
  set_Hist_Style(hdEdxIRMean);
  hdEdxIRMean->SetMarkerColor(kRed);
  hdEdxIRMean->SetMarkerStyle(20);
  hdEdxIRMean->SetMarkerSize(1.10);
  hdEdxIRMean->GetYaxis()->SetRangeUser(0.80, 1.20);
  hdEdxIRMean->GetYaxis()->SetTitle("dE/dx (#mu_{fit})");
  hdEdxIRMean->GetXaxis()->SetTitle("Events(M)");
  hdEdxIRMean->SetTitle(Form("CDC-dE/dx mean (#mu) variation"));
  hdEdxIRMean->Draw("");

  l_line->DrawLine(0.5, m_mean, hdEdxIRMean->GetXaxis()->GetBinUpEdge(nbin), m_mean);

  TPaveText* pinfo0 = new TPaveText(0.689, 0.680, 0.942, 0.911, "brNDC");
  set_Text_Style(pinfo0);
  pinfo0->AddText("Intra-run variation");
  pinfo0->AddText("Electrons (e^{+}e^{-})");
  pinfo0->AddText(Form("Exp/Run: %d/%d", m_exp, m_run));
  pinfo0->AddText(Form("Avg #mu_{fit}: %0.3f", m_mean));
  if (m_nbhabhaevt > 1e5)
    pinfo0->AddText(Form("Events: %0.02fM", double(m_nbhabhaevt / 1e6)));
  if (m_nbhabhaevt > 1e3)
    pinfo0->AddText(Form("Events: %0.02fK", double(m_nbhabhaevt / 1e3)));
  else
    pinfo0->AddText(Form("Events: %d", m_nbhabhaevt));
  pinfo0->Draw("same");
  c_ir_mean->Modified();
  c_ir_mean->Update();

  //3 intra-resolution trend
  c_ir_reso->Clear();
  c_ir_reso->cd();
  gPad->SetGridy(1);
  set_Pad_Style(0.143, 0.045, 0.077, 0.0);
  set_Hist_Style(hdEdxIRSigma);
  hdEdxIRSigma->SetMarkerColor(kRed);
  hdEdxIRSigma->SetMarkerStyle(20);
  hdEdxIRSigma->SetMarkerSize(1.10);
  hdEdxIRSigma->GetYaxis()->SetRangeUser(0.03, 0.20);
  hdEdxIRSigma->GetYaxis()->SetTitle("dE/dx (#sigma_{fit})");
  hdEdxIRSigma->GetXaxis()->SetTitle("Events(M)");
  hdEdxIRSigma->SetTitle(Form("CDC-dE/dx resolution (#sigma) variation"));
  hdEdxIRSigma->Draw("");

  l_line->DrawLine(0.5, m_sigma, hdEdxIRSigma->GetXaxis()->GetBinUpEdge(nbin), m_sigma);

  TPaveText* pinfo1 = new TPaveText(0.689, 0.680, 0.942, 0.911, "brNDC");
  set_Text_Style(pinfo1);
  pinfo1->AddText("Intra-run variation");
  pinfo1->AddText("Electrons (e^{+}e^{-})");
  pinfo1->AddText(Form("Exp/Run: %d/%d", m_exp, m_run));
  pinfo1->AddText(Form("Avg #sigma_{fit}: %0.3f", m_sigma));
  if (m_nbhabhaevt > 1e5)
    pinfo1->AddText(Form("Events: %0.02fM", double(m_nbhabhaevt / 1e6)));
  if (m_nbhabhaevt > 1e3)
    pinfo1->AddText(Form("Events: %0.02fK", double(m_nbhabhaevt / 1e3)));
  else
    pinfo1->AddText(Form("Events: %d", m_nbhabhaevt));
  pinfo1->Draw("same");
  c_ir_reso->Modified();
  c_ir_reso->Update();

  //Let's reset histogram here
  for (int ibin = 0; ibin < nbin; ibin++) hdEdxIRInd[ibin]->Reset();

}

//------------------------------------------------
void DQMHistAnalysisCDCDedxModule::drawWireStatus()
{

  TH2D* hWires = (TH2D*)findHist("CDCDedx/hWires");
  TH2D* hWireStatus = (TH2D*)findHist("CDCDedx/hWireStatus");

  //Draw Scattered plot
  c_pr_wires->Clear();
  c_pr_wires->cd();
  set_Hist_Style(hWires);
  hWires->SetMarkerColor(kGray + 1);
  hWires->Draw("");

  std::string s_ndead = hWireStatus->GetTitle();
  int m_ndead = atof(s_ndead.c_str());
  m_monObj->setVariable("CDCDedxDeadWires", m_ndead);

  set_Hist_Style(hWireStatus);
  hWireStatus->SetMarkerColor(kRed);
  hWireStatus->SetMarkerStyle(7);
  hWireStatus->Draw("same");

  TPaveText* pinfo0 = new TPaveText(0.117, 0.832, 0.148, 0.976, "brNDC");
  set_Text_Style(pinfo0);
  pinfo0->AddText(Form("CDC Wire Status"));
  pinfo0->AddText(Form("Exp/Run: %d/%d", m_exp, m_run));
  pinfo0->AddText(Form("Dead: %d (%0.02f%%)", m_ndead, (100.0 * m_ndead / 14336.0)));
  if (m_nallevt > 1e5)
    pinfo0->AddText(Form("Events: %0.02fM", double(m_nallevt / 1e6)));
  if (m_nallevt > 1e3)
    pinfo0->AddText(Form("Events: %0.02fK", double(m_nallevt / 1e3)));
  else
    pinfo0->AddText(Form("Events: %d", m_nallevt));
  pinfo0->Draw("same");

  c_pr_wires->Modified();
  c_pr_wires->Update();

}

//-----------------------------------------------
void DQMHistAnalysisCDCDedxModule::drawBandPlot()
{

  //Draw Scattered plot
  TH2D* hdEdxVsP = (TH2D*)findHist("CDCDedx/hdEdxVsP");

  set_Plot_Style();
  c_pr_bands->Clear();
  c_pr_bands->cd();
  c_pr_bands->SetLogx();
  c_pr_bands->SetLogy();

  set_Hist_Style(hdEdxVsP);
  hdEdxVsP->SetTitle("CDC-dEdx band plots");
  hdEdxVsP->Draw("col");

  TPaveText* pinfo0 = new TPaveText(0.59, 0.75, 0.80, 0.88, "brNDC");
  set_Text_Style(pinfo0);
  pinfo0->AddText(Form("IP tracks (hadron)"));
  pinfo0->AddText(Form("Exp/Run: %d/%d", m_exp, m_run));
  if (m_nhadevt > 1e5)
    pinfo0->AddText(Form("Events: %0.02fM", double(m_nhadevt / 1e6)));
  if (m_nhadevt > 1e3)
    pinfo0->AddText(Form("Events: %0.02fK", double(m_nhadevt / 1e3)));
  else
    pinfo0->AddText(Form("Events: %d", m_nhadevt));
  pinfo0->DrawClone("same");

  c_pr_bands->Modified();
  c_pr_bands->Update();

}

//---------------------------------------------
void DQMHistAnalysisCDCDedxModule::drawDedxCosPhi()
{

  c_pr_dedxphi->Clear();
  c_pr_dedxphi->cd();
  set_Pad_Style(0.143, 0.045, 0.077, 0.0);

  TH2D* hdEdxvsPhi = (TH2D*)findHist("CDCDedx/hdEdxvsPhi");
  set_Hist_Style(hdEdxvsPhi);
  hdEdxvsPhi->SetTitle("CDC-dEdx vs Phi");
  hdEdxvsPhi->Draw("col");

  TPaveText* pinfo0 = new TPaveText(0.689, 0.751, 0.942, 0.89, "brNDC");
  set_Text_Style(pinfo0);
  pinfo0->AddText(Form("Electrons (e^{+}e^{-})"));
  pinfo0->AddText(Form("Exp/Run: %d/%d", m_exp, m_run));
  if (m_nbhabhaevt > 1e5)
    pinfo0->AddText(Form("Events: %0.02fM", double(m_nbhabhaevt / 1e6)));
  if (m_nbhabhaevt > 1e3)
    pinfo0->AddText(Form("Events: %0.02fK", double(m_nbhabhaevt / 1e3)));
  else
    pinfo0->AddText(Form("Events: %d", m_nbhabhaevt));
  pinfo0->DrawClone("same");

  c_pr_dedxphi->Modified();
  c_pr_dedxphi->Update();

  //plot # 2
  c_pr_dedxcos->Clear();
  c_pr_dedxcos->cd();
  set_Pad_Style(0.143, 0.045, 0.077, 0.0);

  TH2D* hdEdxvsCosth = (TH2D*)findHist("CDCDedx/hdEdxvsCosth");
  set_Hist_Style(hdEdxvsCosth);
  hdEdxvsCosth->SetTitle("CDC-dEdx vs Costh");
  hdEdxvsCosth->Draw("col");

  TPaveText* pinfo1 = new TPaveText(0.689, 0.751, 0.942, 0.89, "brNDC");
  set_Text_Style(pinfo1);
  pinfo1->AddText(Form("Electrons (e^{+}e^{-})"));
  pinfo1->AddText(Form("Exp/Run: %d/%d", m_exp, m_run));
  if (m_nbhabhaevt > 1e5)
    pinfo1->AddText(Form("Events: %0.02fM", double(m_nbhabhaevt / 1e6)));
  if (m_nbhabhaevt > 1e3)
    pinfo1->AddText(Form("Events: %0.02fK", double(m_nbhabhaevt / 1e3)));
  else
    pinfo1->AddText(Form("Events: %d", m_nbhabhaevt));
  pinfo1->DrawClone("same");

  c_pr_dedxcos->Modified();
  c_pr_dedxcos->Update();

}

//----------------------------------------------------------------------------------------
void DQMHistAnalysisCDCDedxModule::fitHistogram(TH1D*& temphist, std::string& status)
{

  temphist->GetXaxis()->SetRange(temphist->FindFirstBinAbove(0, 1), temphist->FindLastBinAbove(0, 1));
  int fs = temphist->Fit(f_gaus, "QR");
  if (fs != 0) status = "Failed";
  else {
    double mean = temphist->GetFunction("f_gaus")->GetParameter(1);
    double width = temphist->GetFunction("f_gaus")->GetParameter(2);
    temphist->GetXaxis()->SetRangeUser(mean - 5.0 * width, mean + 5.0 * width);
    fs = temphist->Fit(f_gaus, "QR", "", mean - 2.5 * width, mean + 2.5 * width);
    if (fs != 0)status = "Failed";
    else {
      temphist->GetXaxis()->SetRangeUser(mean - 6.0 * width, mean + 6.0 * width);
      status = "OK";
    }
  }

}

//------------------------------------------------
void DQMHistAnalysisCDCDedxModule::set_Plot_Style()
{

  const Int_t NRGBs = 6;
  const Int_t NCont = 255;
  Double_t stops[NRGBs] = { 0.00, 0.00, 0.34, 0.61, 0.84, 1.00 };
  Double_t red[NRGBs]   = { 0.00, 0.00, 0.00, 0.87, 1.00, 0.51 };
  Double_t green[NRGBs] = { 0.00, 0.00, 0.81, 1.00, 0.20, 0.00 };
  Double_t blue[NRGBs]  = { 0.00, 0.51, 1.00, 0.12, 0.00, 0.00 };
  TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
  gStyle->SetNumberContours(NCont);

}


//------------------------------------------------------------------
void DQMHistAnalysisCDCDedxModule::set_Text_Style(TPaveText*& obj)
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
void DQMHistAnalysisCDCDedxModule::set_Hist_Style(TH1* obj)
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
void DQMHistAnalysisCDCDedxModule::set_Pad_Style(double l, double r, double t, double b)
{

  if (l != 0)gPad->SetLeftMargin(l);
  if (r != 0)gPad->SetRightMargin(r);
  if (t != 0)gPad->SetTopMargin(t);
  if (b != 0)gPad->SetBottomMargin(b);
  gPad->SetTickx(1);
  gPad->SetTicky(1);

}
