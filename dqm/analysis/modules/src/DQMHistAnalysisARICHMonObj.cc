/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <dqm/analysis/modules/DQMHistAnalysisARICHMonObj.h>

//DQM
#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TF1.h>
#include <TH1F.h>
#include <TH3.h>
#include <TH2.h>
#include <TLine.h>
#include <TH2Poly.h>
#include <TText.h>
#include <TROOT.h>
#include <TLegend.h>
#include <iostream>
#include <TStyle.h>
#include <TGaxis.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(DQMHistAnalysisARICHMonObj);

DQMHistAnalysisARICHMonObjModule::DQMHistAnalysisARICHMonObjModule()
  : DQMHistAnalysisModule()
{
  // set module description (e.g. insert text)
  setDescription("Example module for making MonitoringObject in DQMHistAnalysis module");
  setPropertyFlags(c_ParallelProcessingCertified);
}

DQMHistAnalysisARICHMonObjModule::~DQMHistAnalysisARICHMonObjModule()
{
}

void DQMHistAnalysisARICHMonObjModule::initialize()
{
  // make monitoring object related to this module (use arich as an example)
  // if monitoring object already exists this will return pointer to it
  m_monObj = getMonitoringObject("arich");

  // make canvases to be added to MonitoringObject
  m_c_main = new TCanvas("arich_main", "main", 1500, 800);
  m_c_mask = new TCanvas("arich_mask", "mask", 750, 1600);
  m_c_mirror = new TCanvas("arich_mirror", "mirror", 1000, 1000);
  m_c_tracks = new TCanvas("arich_tracks", "tracks", 500, 500);

  m_apdHist = new ARICHChannelHist("tmpApdHist", "tmpApdHist", 2); /**<ARICH TObject to draw hit map for each APD*/
  //m_chHist = new ARICHChannelHist("tmpChHist", "tmpChHist", 0); /**<ARICH TObject to draw hit map for each channel*/
  m_hapdHist = new ARICHChannelHist("tmpHapdHist", "tmpHapdHist", 1); /**<ARICH TObject to draw flash map for each HAPD*/

  // add canvases to MonitoringObject
  m_monObj->addCanvas(m_c_main);
  m_monObj->addCanvas(m_c_mask);
  m_monObj->addCanvas(m_c_mirror);
  m_monObj->addCanvas(m_c_tracks);

}

void DQMHistAnalysisARICHMonObjModule::beginRun()
{
}

void DQMHistAnalysisARICHMonObjModule::event()
{
  // can put the analysis code here or in endRun() function
  // for the start tests we will store output only end of run so better to put code there
}

void DQMHistAnalysisARICHMonObjModule::endRun()
{


  TGaxis::SetMaxDigits(3);
  gStyle->SetPalette(1);
  gStyle->SetOptStat(0);


  // get existing histograms produced by DQM modules
  TH1* chHit = findHist("ARICH/chHit");/**<The number of hits in each chip */
  TH1* bits = findHist("ARICH/bits");
  TH1* hitsPerTrack = findHist("ARICH/hitsPerTrack");
  TH1* theta = findHist("ARICH/theta");
  TH2* tracks2D = (TH2*)findHist("ARICH/tracks2D");
  TH1* hitsPerEvent = findHist("ARICH/hitsPerEvent");
  TH2* hapdHitPerEvent = (TH2*)findHist("ARICH/hapdHitPerEvent");
  TH2* thetaPhi = (TH2*)findHist("ARICH/thetaPhi");
  TH3* mirrorThetaPhi = (TH3*)findHist("ARICH/mirrorThetaPhi");
  TH1* chDigit = findHist("ARICH/chDigit");
  TH1* hapdDigit = findHist("ARICH/hapdDigit");

  if (chHit == NULL) {m_monObj->setVariable("comment", "No arich histograms in file");  B2INFO("Histogram named chHit is not found.");  return;}
  if (chHit->GetEntries() == 0) {m_monObj->setVariable("comment", "No arich hits in histograms"); B2INFO("No arich hits in histograms.");  return;}

  // set the content of main canvas
  m_c_main->Clear(); // clear existing content
  m_c_main->Divide(3, 2);
  m_c_mirror->Clear(); // clear existing content
  m_c_mirror->Divide(3, 3);
  m_c_mask->Clear(); // clear existing content
  m_c_mask->Divide(2, 4);
  m_c_tracks->Clear(); // clear existing content

  //Draw 2D hit map of channels and APDs
  pp1 = new TH2Poly();
  pp1->SetTitle("Number of hits / APD / event");
  pp1->SetOption("colz");

  pp2 = new TH2Poly();
  pp2->SetMaximum(0.1 / 36.);
  pp2->SetMinimum(0.0001 / 36.);
  pp2->SetTitle("Number of hits / channel / event");

  pflash = new TH2Poly();
  pflash->SetTitle("Number of flash (>40 hits) / event");

  int nevt = 0;
  if (hitsPerEvent) nevt = hitsPerEvent->GetEntries();
  m_apdHist->fillFromTH1(chHit);
  //  m_chHist->fillFromTH1(chHit);
  if (nevt) {
    m_apdHist->Scale(1. / float(nevt));
    // m_chHist->Scale(1. / float(nevt));
  }

  pp1->SetMaximum(0.1);
  m_apdHist->setPoly(pp1);
  pp1->SetMinimum(0.0001);
  pp2->SetMaximum(0.1 / 36.);
  //  m_chHist->setPoly(pp2);
  // pp2->SetMinimum(0.0001 / 36.);


  //TCanvas main
  m_c_main->cd(1);
  pp1->Draw("colz");
  pp1->GetXaxis()->SetTickLength(0);
  pp1->GetYaxis()->SetTickLength(0);
  gPad->SetLogz();
  //m_c_main->Update();

  TH1F* flash = (TH1F*)hapdHitPerEvent->ProjectionX("flash", 40, 144);
  m_hapdHist->fillFromTH1(flash);
  if (nevt) m_hapdHist->Scale(1. / float(nevt));

  m_hapdHist->setPoly(pflash);
  // draw sector lines
  double rlin = 40;
  double rlout = 113;
  for (int isec = 0; isec < 6; isec++) {
    double x1 = rlin * cos(M_PI / 3.*isec);
    double x2 = rlout * cos(M_PI / 3.*isec);
    double y1 = rlin * sin(M_PI / 3.*isec);
    double y2 = rlout * sin(M_PI / 3.*isec);
    TLine* line = new TLine(x1, y1, x2, y2);
    line->Draw();
    x1 = rlin * cos(M_PI / 3.*isec + M_PI / 6.);
    y1 = rlin * sin(M_PI / 3.*isec + M_PI / 6.);
    TText* lab = new TText(x1, y1, TString::Format("S-%d", isec + 1));
    lab->SetTextAlign(22);
    lab->SetTextSize(0.03);
    lab->Draw();
  }


  //TCanvas tracks
  m_c_tracks->cd();
  tracks2D->RebinX(4);
  tracks2D->RebinY(4);
  double trkevt = nevt > 0 ? tracks2D->GetEntries() / nevt : 0;
  int ntracks = tracks2D->GetEntries();
  m_monObj->setVariable("ntracks", ntracks ? ntracks : 0);
  m_monObj->setVariable("ntracksPerEvent", trkevt ? trkevt : 0);
  string comment = "";
  if (ntracks == 0) comment = "No arich tracks in file.";
  if (theta->GetEntries() == 0) comment.append(" No cherenkov photons available");
  m_monObj->setVariable("comment", comment);

  tracks2D->SetTitle(TString::Format("Track distribution (avg %f trk/evt)", trkevt));
  tracks2D->Draw("colz");


  if (ntracks) {
    double sigbkg[8] = {0};
    //fit two gauses
    if (theta->GetEntries() == 0) return;
    TF1* f1 = new TF1("thcFit", "gaus(0)+gaus(3)", 0.2, 0.4);
    f1->SetParameters(0.8 * theta->GetMaximum(), 0.323, 0.016, 0.2 * theta->GetMaximum() , 0.323, 0.13);
    f1->FixParameter(5, 0.13);
    f1->SetParName(0, "C");
    f1->SetParName(1, "mean");
    f1->SetParName(2, "sigma");
    f1->SetParName(3, "p0");
    f1->SetParName(4, "p1");
    int status = theta->Fit(f1, "R");
    double xmin = f1->GetParameter(1) - 2.*f1->GetParameter(2);
    double xmax = f1->GetParameter(1) + 2.*f1->GetParameter(2);
    double tmp = f1->GetParameter(3);
    f1->SetParameter(3, 0);
    double nphot = f1->Integral(xmin, xmax);
    f1->SetParameter(3, tmp);
    tmp = f1->GetParameter(0);
    f1->SetParameter(0, 0);
    double nbkg = f1->Integral(xmin, xmax);
    f1->SetParameter(0, tmp);
    if (status) return;

    sigbkg[0] = nphot;
    sigbkg[1] = f1->GetParameter(0) > 0 ? nphot * f1->GetParError(0) / f1->GetParameter(0) : 0.;
    sigbkg[2] = nbkg;
    sigbkg[3] = f1->GetParameter(3) > 0 ? nbkg * f1->GetParError(3) / f1->GetParameter(3) : 0.;
    sigbkg[4] = f1->GetParameter(1);
    sigbkg[5] = f1->GetParError(1);
    sigbkg[6] = f1->GetParameter(2);
    sigbkg[7] = f1->GetParError(2);
    if (theta->GetBinWidth(1) == 0) return;
    m_monObj->setVariable("nsig", sigbkg[0] / float(ntracks) / theta->GetBinWidth(1),
                          sigbkg[1] / float(ntracks) / theta->GetBinWidth(1));
    m_monObj->setVariable("nbgr", sigbkg[2] / float(ntracks) / theta->GetBinWidth(1),
                          sigbkg[3] / float(ntracks) / theta->GetBinWidth(1));
    m_monObj->setVariable("theta", sigbkg[4], sigbkg[5]);
    m_monObj->setVariable("sigma", sigbkg[6], sigbkg[7]);
    std::cout << sigbkg[0]  << " " << sigbkg[1] / float(ntracks) / theta->GetBinWidth(1) << std::endl;
  }


  //TCanvas mirror
  TH1F* thetaCl = (TH1F*)theta->Clone("thetaCl");
  thetaCl->SetLineColor(16);
  thetaCl->SetLineWidth(2);
  thetaCl->SetTitle("");
  TLegend* leg[9];
  gStyle->SetOptTitle(0);

  if (mirrorThetaPhi) {
    for (int i = 1; i < 18 + 1; i++) {
      TH1F* hmir = (TH1F*)mirrorThetaPhi->ProjectionZ(TString::Format("hmir_%d", i), i, i, 1, 10000);
      hmir->SetTitle(TString::Format("mirror %d", i));
      if (hmir->GetEntries() > 0) hmir->Scale(theta->GetEntries() / hmir->GetEntries());
      hmir->Rebin(2);
      hmir->SetLineWidth(2);
      int iplot = (i - 1) / 2;
      if ((i - 1) % 2 == 0) { m_c_mirror->cd(iplot + 1); thetaCl->Draw("hist"); hmir->SetLineColor(1); hmir->Draw("sames hist");  leg[iplot] = new TLegend(0.1, 0.75, 0.4, 0.9);  leg[iplot]->AddEntry(hmir, TString::Format("mirror %d", i));}
      else {
        hmir->SetLineColor(2); hmir->Draw("sames hist");
        leg[iplot]->AddEntry(hmir, TString::Format("mirror %d", i));
        leg[iplot]->Draw();
      }
    }
  }

  gStyle->SetOptTitle(1);


  //chDigit
  if (chDigit != NULL && nevt) chDigit->Scale(1. / nevt);
  if (nevt) {
    chHit->Scale(1. / nevt);
    flash->Scale(1. / nevt);
  }
  int nhot = 0, ndead = 0;
  TH2F* hotCh = new TH2F("arich_hot", "Number of channels in APD with >0.5% occ.", 42, 0.5, 42.5, 40, 0.5, 40.5);
  TH2F* hotCh1 = new TH2F("arich_hot1", "Number of channels in APD with >0.5% occ. after mask", 42, 0.5, 42.5, 40, 0.5, 40.5);
  TH2F* deadCh = new TH2F("arich_dead", "Number of channels in APD with no hits", 42, 0.5, 42.5, 40, 0.5, 40.5);
  TH2F* falseCh = new TH2F("arich_false", "Number of wrongly masked channels in APD (masked but not dead/hot)", 42, 0.5, 42.5, 40,
                           0.5,
                           40.5);
  TH1F* occ = new TH1F("arich_occ", "nhits / nevt for all channels; nhits/nevt;# of chn", 500, 0, 0.005);

  int ndeadHapd = 0;
  if (chDigit != NULL) {
    double hotlim = 0.005;
    for (int i = 0; i < chDigit->GetNbinsX(); i++) {
      int hapd = i / 144 + 1; int chip = (i % 144) / 36 + 1;
      int binx = (hapd - 1) % 42 + 1; int biny = ((hapd - 1) / 42) * 4 + chip;
      if (chDigit->GetBinContent(i + 1) > hotlim) { hotCh->Fill(binx, biny); nhot++;}
      if (chDigit->GetBinContent(i + 1) == 0) {
        deadCh->Fill(binx, biny);
        if (hapdDigit->GetBinContent(hapd) != 0) ndead++;
      }
      if (chHit->GetBinContent(i + 1) == 0 && chDigit->GetBinContent(i + 1) < hotlim
          && chDigit->GetBinContent(i + 1) > 0.00002) falseCh->Fill(binx, biny);
      if (chHit->GetBinContent(i + 1) > hotlim) hotCh1->Fill(binx, biny);
      occ->Fill(chHit->GetBinContent(i + 1));
    }

    for (int i = 0; i < hapdDigit->GetNbinsX(); i++) {
      if (hapdDigit->GetBinContent(i + 1) == 0) ndeadHapd++;
    }
  }
  m_monObj->setVariable("nhot", nhot);
  m_monObj->setVariable("ndead", ndead);
  m_monObj->setVariable("ndeadHapd", ndeadHapd);


  // TCanvas mask
  m_c_mask->cd(1);
  occ->Draw();
  m_c_mask->cd(2);
  pflash->Draw("colz");
  m_c_mask->cd(3);
  // pp2->Draw("colz");
  //pp2->GetXaxis()->SetTickLength(0);
  //pp2->GetYaxis()->SetTickLength(0);
  gPad->SetLogz();
  m_c_mask->cd(4);
  hotCh->Draw("colz text");
  m_c_mask->cd(5);
  deadCh->Draw("colz text");
  m_c_mask->cd(6);
  hotCh1->Draw("colz text");
  m_c_mask->cd(7);
  falseCh->Draw("colz text");

  for (int i = 0; i < 42; i++) {
    double x1 = i + 0.5 + 1;
    TLine* line = new TLine(x1, 0.5, x1, 40.5);
    m_c_mask->cd(5); line->Draw();
    m_c_mask->cd(6); line->Draw();
    m_c_mask->cd(7); line->Draw();
    m_c_mask->cd(4); line->Draw();
  }
  for (int i = 0; i < 40; i++) {
    double y1 = i + 0.5 + 1;
    TLine* line = new TLine(0.5, y1, 42.5, y1);
    if ((i + 1) % 4 == 0) line->SetLineColor(2);
    else line->SetLineColor(15);
    m_c_mask->cd(5); line->Draw();
    m_c_mask->cd(6); line->Draw();
    m_c_mask->cd(7); line->Draw();
    m_c_mask->cd(4); line->Draw();
  }

  if (bits) {
    bits->SetLineWidth(2);
    bits->SetLineColor(2);
    bits->SetOption("hist");
    bits->SetFillStyle(3010);
    bits->SetFillColor(3);
  }

  if (hitsPerTrack) {
    hitsPerTrack->SetLineWidth(2);
    hitsPerTrack->SetLineColor(2);
    hitsPerTrack->SetOption("hist");
  }
  if (hitsPerEvent) {
    hitsPerEvent->SetLineWidth(2);
    hitsPerEvent->SetLineColor(2);
    hitsPerEvent->SetOption("hist");
  }
  //Tcanvas main
  m_c_main->cd(2);
  if (bits) { bits->Draw(); bits->GetYaxis()->SetTitleOffset(0.5); }
  m_c_main->cd(3);
  if (theta) theta->Draw();
  m_c_main->cd(4);
  if (hitsPerTrack) hitsPerTrack->Draw();
  m_c_main->cd(5);
  if (hitsPerEvent) hitsPerEvent->Draw();
  m_c_main->cd(6);
  if (thetaPhi != NULL) thetaPhi->Draw("colz");

  // set values of monitoring variables (if variable already exists this will change its value, otherwise it will insert new variable)
  // with error (also asymmetric error can be used as m_monObj->setVariable(name, value, upError, downError))
  m_monObj->setVariable("hitsPerEvent", hitsPerEvent ? hitsPerEvent->GetMean() : 0, hitsPerEvent ? hitsPerEvent->GetMeanError() : -1);
  // without error
  m_monObj->setVariable("bitsMean", bits ? bits->GetMean() : 0);
  B2DEBUG(20, "DQMHistAnalysisARICHMonObj : endRun called");
}

void DQMHistAnalysisARICHMonObjModule::terminate()
{

  B2DEBUG(20, "terminate called");
}
