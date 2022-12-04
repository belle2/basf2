/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <algorithm>
#include <iostream>
#include <fstream>
#include <TH1I.h>
#include <TCanvas.h>
#include <TMath.h>
#include <TLine.h>
#include <TStyle.h>
#include <TPaveText.h>
#include <TText.h>
#include <TLegend.h>
#include <reconstruction/calibration/CDCDedxBadWireAlgorithm.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
CDCDedxBadWireAlgorithm::CDCDedxBadWireAlgorithm() :
  CalibrationAlgorithm("CDCDedxElectronCollector"),
  nwireCDC(14336),
  fdedxBin(140),
  fdedxMin(0.0),
  fdedxMax(7.0),
  fadcBin(100),
  fadcMin(0.0),
  fadcMax(1000),
  fmeanThers(1.0),
  frmsThers(1.0),
  ffracThers(1.0),
  isadc(false),
  saddSfx("")
{
  // Set module properties
  setDescription("A calibration algorithm for CDC dE/dx bad wires");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------
CalibrationAlgorithm::EResult CDCDedxBadWireAlgorithm::calibrate()
{

  getExpRunInfo();
  if (!m_DBBadWires.isValid() || !m_DBWireGains.isValid())
    B2FATAL("There is no valid payload for BadWires and/or Wirgain");

  // Get data objects
  auto ttree = getObjectPtr<TTree>("tree");
  if (ttree->GetEntries() < 1000)return c_NotEnoughData;

  std::vector<int>* wire = 0;
  std::vector<double>* hitvar = 0;

  ttree->SetBranchAddress("wire", &wire);
  if (isadc)ttree->SetBranchAddress("adccorr", &hitvar);
  else ttree->SetBranchAddress("dedxhit", &hitvar);

  std::string svar;
  int nbins;
  double fmin = 0.0, fmax = 0.0;
  if (isadc) {
    svar = "adccount";
    nbins = fadcBin;
    fmin = fadcMin;
    fmax = fadcMax;
  } else {
    svar = "hitdedx";
    nbins = fdedxBin;
    fmin = fdedxMin;
    fmax = fdedxMax;
  }

  std::vector<std::vector<double>> vhitvar(nwireCDC, std::vector<double>());
  TH1D* hvarall = new TH1D("hvarall", "", nbins, fmin, fmax);
  hvarall->SetTitle(Form("dist %s; %s; %s", svar.data(), svar.data(), "entries"));

  // hit dedx or adc vector for each wire
  for (int i = 0; i < ttree->GetEntries(); ++i) {
    ttree->GetEvent(i);
    for (unsigned int ih = 0; ih < wire->size(); ++ih) {
      int iwire = wire->at(ih);
      double var = hitvar->at(ih);
      hvarall->Fill(var);
      vhitvar[iwire].push_back(var);
    }
  }

  //check statistics of dead wire
  int minstat = 0;
  for (unsigned int jw = 0; jw < nwireCDC; ++jw) {
    if (vhitvar[jw].size() == 0) minstat++;
  }

  double deadfrac = 100.0 * minstat / nwireCDC;
  if (deadfrac > 5.0) {
    B2INFO("CDCDedxBadWire: too many dead wires? " << deadfrac << "%");
    return c_NotEnoughData;
  }

  //starting calibration at this point
  saddSfx = Form("%s_%s", svar.data(), saddSfx.data());
  TH1D* hvar = new TH1D("hvar", "", nbins, fmin, fmax);
  hvar->SetTitle(Form("%s; %s; entries", saddSfx.data(), svar.data()));

  TH1D* hmean = new TH1D(Form("hmean_%s", saddSfx.data()), "", nwireCDC, -0.5, 14335.5);
  hmean->SetTitle(Form("mean vs wires (%s); wire ; <mean>", saddSfx.data()));

  TH1D* hrms = new TH1D(Form("hrms_%s", saddSfx.data()), "", nwireCDC, -0.5, 14335.5);
  hrms->SetTitle(Form("rms vs wires (%s); wire ; RMS", saddSfx.data()));

  TH1D* hfrac = new TH1D(Form("hfrac_%s", saddSfx.data()), "", nwireCDC, -0.5, 14335.5);
  hfrac->SetTitle(Form("high fraction vs wires (%s); wire; fraction", saddSfx.data()));

  double avgmean = hvarall->GetMean();
  double avgrms = hvarall->GetRMS();

  bool iwdefect;
  std::vector<double> m_vdefectwires;

  TCanvas* cgtmp = new TCanvas(Form("cgood_%s", saddSfx.data()), "", 1200, 1200);
  cgtmp->Divide(4, 4);
  cgtmp->SetBatch(kTRUE);
  std::stringstream psgood;
  psgood << Form("cdcdedx_bdcalcal_gwiredist_%s.pdf[", saddSfx.data());
  cgtmp->Print(psgood.str().c_str());
  psgood.str("");
  psgood << Form("cdcdedx_bdcalcal_gwiredist_%s.pdf", saddSfx.data());

  TCanvas* cbtmp = new TCanvas(Form("cbad_%s", saddSfx.data()), "", 1200, 1200);
  std::stringstream psdefect;
  cbtmp->Divide(4, 4);
  cbtmp->SetBatch(kTRUE);
  psdefect << Form("cdcdedx_bdcalcal_bwiredist_%s.pdf[", saddSfx.data());
  cbtmp->Print(psdefect.str().c_str());
  psdefect.str("");
  psdefect << Form("cdcdedx_bdcalcal_bwiredist_%s.pdf", saddSfx.data());

  std::ofstream ofile_newbad, ofile_newdead;
  ofile_newbad.open(Form("cdcdedx_bdcalcal_bwirelist_%s.txt", saddSfx.data()));
  ofile_newdead.open(Form("cdcdedx_bdcalcal_dwirelist_%s.txt", saddSfx.data()));

  int fentThres = 50;
  int cbadwire[2] = {0}, cdeadwire[2] = {0}, cgoodwire[2] = {0};

  for (unsigned int jw = 0; jw < nwireCDC; ++jw) {

    hvar->SetName(Form("%s_wire%d", saddSfx.data(), jw));
    hvar->SetTitle(Form("%s, wire = %d", saddSfx.data(), jw));

    int tsum = 0, nsum = 0;

    for (unsigned int jh = 0; jh < vhitvar[jw].size(); ++jh) {
      double ihvar = vhitvar[jw][jh];
      if (ihvar < fmax) {
        nsum++;
        hvar->Fill(ihvar);
      } else tsum++;
    }

    double fmean = 0., fmeanerr = 0.;
    double frms = 0., frmserr = 0.;

    if (nsum > 0) {
      fmean = hvar->GetMean();
      fmeanerr = hvar->GetMeanError();
      frms = hvar->GetRMS();
      frmserr = hvar->GetRMSError();
    }

    hmean->SetBinContent(jw + 1, fmean);
    hmean->SetBinError(jw + 1, fmeanerr);

    hrms->SetBinContent(jw + 1, frms);
    hrms->SetBinError(jw + 1, frmserr);

    double ffrac = 0.0;
    if (nsum > 0 && tsum > 0)ffrac = (1.0 * tsum) / (tsum + nsum);
    hfrac->SetBinContent(jw + 1, ffrac);
    hfrac->SetBinError(jw + 1, 0.0001);

    double oldwg = m_DBWireGains->getWireGain(jw);

    if ((nsum < fentThres) || (abs(fmean - avgmean) / avgmean > fmeanThers) || (abs(frms - avgrms) / avgrms > frmsThers)
        || (ffrac > ffracThers)) {
      iwdefect = true;
      if (nsum != 0) {
        cbadwire[0]++;
        if (oldwg == 0)cbadwire[1]++;
      } else {
        cdeadwire[0]++;
        if (oldwg == 0)cdeadwire[1]++;
      }
    } else {
      iwdefect = false;
      cgoodwire[0]++;
    }

    TPaveText* pinfo = new TPaveText(0.35, 0.63, 0.83, 0.89, "NBNDC");
    setTextCosmetics(pinfo);
    pinfo->AddText(Form("#mu: %0.2f(%0.2f#pm%0.2f)", fmean, avgmean, fmeanThers * avgmean));
    pinfo->AddText(Form("#sigma: %0.2f(%0.2f#pm%0.2f)", frms, avgrms, frmsThers * avgrms));
    pinfo->AddText(Form("fac: %0.2f(%0.2f)", ffrac, ffracThers));
    pinfo->AddText(Form("N: %d", nsum));

    if (iwdefect) {
      if (nsum != 0) {
        cbtmp->cd((cbadwire[0] - 1) % 16 + 1);
        hvar->SetStats(false);
        hvar->SetFillColorAlpha(kOrange, 0.20);
        if (oldwg == 0)hvar->SetFillColorAlpha(kRed, 0.70);
        hvar->DrawCopy("hist");
        pinfo->Draw("same");
        if ((cbadwire[0] % 16 == 0) || (jw == nwireCDC))  {
          cbtmp->Print(psdefect.str().c_str());
          cbtmp->Clear("D");
        }
        ofile_newbad <<  jw << std::endl;
      } else ofile_newdead <<  jw << std::endl;
      m_vdefectwires.push_back(0.0);
    } else {
      cgtmp->cd((cgoodwire[0] - 1) % 16 + 1);
      hvar->SetStats(false);
      hvar->SetFillColorAlpha(kGreen, 0.20);
      if (oldwg == 0)hvar->SetFillColorAlpha(kRed, 0.70);
      hvar->DrawCopy("hist");
      pinfo->Draw("same");
      if ((cgoodwire[0] % 16 == 0) || (jw == nwireCDC))  {
        cgtmp->Print(psgood.str().c_str());
        cgtmp->Clear("D");
      }
      m_vdefectwires.push_back(1.0);
    }
    hvar->Reset();
  }

  ofile_newbad.close();
  ofile_newdead.close();

  //1 for variable distribution
  psdefect.str("");
  psdefect << Form("cdcdedx_bdcalcal_bwiredist_%s.pdf]", saddSfx.data());
  cbtmp->Print(psdefect.str().c_str());
  cbtmp->Clear("D");
  delete cbtmp;

  psgood.str("");
  psgood << Form("cdcdedx_bdcalcal_gwiredist_%s.pdf]", saddSfx.data());
  cgtmp->Print(psgood.str().c_str());
  cgtmp->Clear("D");
  delete cgtmp;

  //2 for qa parameters
  TCanvas* c_pars = new TCanvas("c_pars", "", 600, 1200);
  c_pars->Divide(1, 3);

  c_pars->cd(1);
  hmean->SetTitle(Form("%s, avg: %0.03f", hmean->GetTitle(), avgmean));
  hmean->SetStats(0);
  hmean->Draw();
  TLine* lmeanmin = new TLine(-0.5, avgmean * (1 - fmeanThers), 14335.5, avgmean * (1 - fmeanThers));
  lmeanmin->SetLineColorAlpha(kRed, 0.5);
  lmeanmin->Draw("same");
  TLine* lmeanmax = new TLine(-0.5, avgmean * (1 + fmeanThers), 14335.5, avgmean * (1 + fmeanThers));
  lmeanmax->SetLineColorAlpha(kRed, 0.5);
  lmeanmax->Draw("same");

  c_pars->cd(2);
  hrms->SetTitle(Form("%s, avg: %0.03f", hrms->GetTitle(), avgrms));
  hrms->SetStats(0);
  hrms->Draw();
  TLine* lrmsmin = new TLine(-0.5, avgrms * (1 - frmsThers), 14335.5, avgrms * (1 - frmsThers));
  lrmsmin->SetLineColorAlpha(kRed, 0.5);
  lrmsmin->Draw("same");
  TLine* lrmsmax = new TLine(-0.5, avgrms * (1 + frmsThers), 14335.5, avgrms * (1 + frmsThers));
  lrmsmax->SetLineColorAlpha(kRed, 0.5);
  lrmsmax->Draw("same");

  c_pars->cd(3);
  gPad->SetGridy();
  hfrac->SetStats(0);
  hfrac->Draw();
  TLine* lfrac = new TLine(-0.5, ffracThers, 14335.5, ffracThers);
  lfrac->SetLineColorAlpha(kRed, 0.5);
  lfrac->Draw("same");

  c_pars->Print(Form("cdcdedx_bdcal_qapars_%s.pdf", saddSfx.data()));
  c_pars->Print(Form("cdcdedx_bdcal_qapars_%s.root", saddSfx.data()));
  delete c_pars;
  //3 for stats
  TCanvas* cstats = new TCanvas("cstats", "cstats", 1000, 500);
  cstats->SetBatch(kTRUE);
  cstats->Divide(2, 1);

  cstats->cd(1);
  auto hestats = getObjectPtr<TH1I>("hestats");
  if (hestats) {
    hestats->SetName(Form("htstats_%s", saddSfx.data()));
    hestats->SetStats(0);
    hestats->DrawCopy("");
  }

  cstats->cd(2);
  auto htstats = getObjectPtr<TH1I>("htstats");
  if (htstats) {
    hestats->SetName(Form("htstats_%s", saddSfx.data()));
    htstats->DrawCopy("");
    hestats->SetStats(0);
  }

  cstats->Print(Form("cdcdedx_bdcal_qastats_%s.pdf", saddSfx.data()));
  delete cstats;

  //create bad/dead wire map using files created above
  createBadWireMap(cdeadwire, cbadwire);

  const unsigned newtotalwire = cdeadwire[0] + cbadwire[0] + cgoodwire[0];
  if (newtotalwire != nwireCDC) {
    B2INFO("CDCDedxBadWires: wire counting is wrong: new total " << newtotalwire << " wires");
    return c_NotEnoughData;
  }

  if (cdeadwire[0] + cbadwire[0] > 1000) {
    B2INFO("CDCDedxBadWires: too many bad+dead wires: new total " << cdeadwire[0] + cbadwire[0] << " wires");
    return c_NotEnoughData;
  }

  B2INFO("dE/dx Badwire calibration done: " << m_vdefectwires.size() << " wires");
  CDCDedxBadWires* c_badwires = new CDCDedxBadWires(m_vdefectwires);
  saveCalibration(c_badwires, "CDCDedxBadWires");
  return c_OK;

}

//------------------------------------
void CDCDedxBadWireAlgorithm::getExpRunInfo()
{

  int cruns = 0;
  for (auto expRun : getRunList()) {
    if (cruns == 0)B2INFO("CDCDedxBadWires: start exp " << expRun.first << " and run " << expRun.second << "");
    cruns++;
  }

  const auto erStart = getRunList()[0];
  int estart = erStart.first;
  int rstart = erStart.second;

  const auto erEnd = getRunList()[cruns - 1];
  int rend = erEnd.second;

  updateDBObjPtrs(1, rstart, estart);

  if (saddSfx.length() > 0)saddSfx = Form("%s_e%d_r%dr%d", saddSfx.data(), estart, rstart, rend);
  else saddSfx = Form("e%d_r%dr%d", estart, rstart, rend);

}

//------------------------------------
void CDCDedxBadWireAlgorithm::createBadWireMap(int ndead[2], int nbad[2])
{

  TCanvas* cmap = new TCanvas(Form("cmap_%s", saddSfx.data()), "CDC dE/dx bad wire status", 800, 800);
  TH2F* hxyAll = getHistoPattern("all", "allwires");
  hxyAll->SetTitle(Form("wire status map (%s)", saddSfx.data()));
  hxyAll->SetMarkerStyle(20);
  hxyAll->SetMarkerSize(0.2);
  hxyAll->SetMarkerColor(kGray);
  hxyAll->SetStats(0);
  hxyAll->Draw();

  TH2F* hxyBad = getHistoPattern(Form("cdcdedx_bdcalcal_bwirelist_%s.txt", saddSfx.data()), "bad");
  if (hxyBad) {
    hxyBad->SetTitle("");
    hxyBad->SetMarkerStyle(20);
    hxyBad->SetMarkerSize(0.3);
    hxyBad->SetMarkerColor(kRed);
    hxyBad->SetStats(0);
    hxyBad->Draw("same");
  }

  TH2F* hxyDead = getHistoPattern(Form("cdcdedx_bdcalcal_dwirelist_%s.txt", saddSfx.data()), "dead");
  if (hxyDead) {
    hxyDead->SetTitle("");
    hxyDead->SetMarkerStyle(20);
    hxyDead->SetMarkerSize(0.3);
    hxyDead->SetMarkerColor(kBlack);
    hxyDead->SetStats(0);
    hxyDead->Draw("same");
  }

  auto legend = new TLegend(0.68, 0.80, 0.90, 0.92);
  legend->SetBorderSize(0);
  legend->SetLineWidth(3);
  legend->SetHeader(Form("total defective: %d (~%0.02f%%)", ndead[0] + nbad[0], 100.*(ndead[0] + nbad[0]) / 14336.0));
  legend->AddEntry(hxyBad, Form("bad #rightarrow new:%d, old:%d", nbad[0], nbad[1]), "p");
  legend->AddEntry(hxyDead, Form("dead #rightarrow new:%d, old:%d", ndead[0], ndead[1]), "p");
  legend->Draw();

  gStyle->SetLegendTextSize(0.025);
  TPaveText* pt = new TPaveText(-0.30993, -1.470968, -0.3102707, -1.304516, "br");
  pt->SetFillColor(0);
  pt->SetFillStyle(3001);
  pt->SetLineColor(2);
  pt->SetTextFont(82);
  pt->SetTextSize(0.02258064);
  TText* t1 = pt->AddText("CDC-wire map: counter-clockwise and start from +x");
  t1->SetTextColor(kGray + 1);
  pt->Draw("same");

  cmap->SaveAs(Form("cdcdedx_bdcalcal_wiremap_%s.pdf", saddSfx.data()));
  delete cmap;
}

//------------------------------------
TH2F* CDCDedxBadWireAlgorithm::getHistoPattern(std::string badFileName, std::string suffix)
{

  int wire, nwire, twire;
  double radius, phi, x, y;

  std::ifstream infile;
  infile.open(Form("%s", badFileName.data()));

  TH2F* temp = new TH2F(Form("temp_%s_%s", saddSfx.data(), suffix.data()), "", 2400, -1.2, 1.2, 2400, -1.2, 1.2);
  if (!infile.fail()) {
    int bwires = 0;
    while (infile >> bwires) {
      nwire = getIndexVal(bwires, "nwirelayer");
      twire = getIndexVal(bwires, "twire");
      radius = getIndexVal(bwires, "rwire");
      wire = bwires - twire ;
      phi = 2.*TMath::Pi() * (float(wire) / float(nwire));
      x = radius * cos(phi);
      y = radius * sin(phi);
      temp->Fill(x, y);
    }
  } else {
    for (int iwires = 0; iwires < 14336; iwires++) {
      nwire = getIndexVal(iwires, "nwirelayer");
      twire = getIndexVal(iwires, "twire");
      radius = getIndexVal(iwires, "rwire");
      wire = iwires - twire ;
      phi = 2.*TMath::Pi() * (float(wire) / float(nwire));
      x = radius * cos(phi);
      y = radius * sin(phi);
      temp->Fill(x, y);
    }
  }
  return temp;
}

//------------------------------------
double CDCDedxBadWireAlgorithm::getIndexVal(int iWire, std::string what)
{

  //radius of each CDC layer
  const double r[56] = {
    16.80,  17.80,  18.80,  19.80,  20.80,  21.80,  22.80,  23.80,
    25.70,  27.52,  29.34,  31.16,  32.98,  34.80,
    36.52,  38.34,  40.16,  41.98,  43.80,  45.57,
    47.69,  49.46,  51.28,  53.10,  54.92,  56.69,
    58.41,  60.18,  62.00,  63.82,  65.64,  67.41,
    69.53,  71.30,  73.12,  74.94,  76.76,  78.53,
    80.25,  82.02,  83.84,  85.66,  87.48,  89.25,
    91.37,  93.14,  94.96,  96.78,  98.60, 100.37,
    102.09, 103.86, 105.68, 107.50, 109.32, 111.14
  };

  Int_t totalWireiLayer = 0 ;
  double myreturn = 0;
  for (Int_t iLayer = 0; iLayer < 56; iLayer++) {
    int iSuperLayer = (iLayer - 2) / 6;
    if (iSuperLayer <= 0)iSuperLayer = 1;
    int nWireiLayer = 160 + (iSuperLayer - 1) * 32;
    totalWireiLayer += nWireiLayer;

    if (iWire < totalWireiLayer) {
      if (what == "layer")myreturn = iLayer;
      else if (what == "nwirelayer") myreturn = nWireiLayer;
      else if (what == "twire")  myreturn = totalWireiLayer - nWireiLayer;
      else if (what == "rwire")  myreturn = r[iLayer] / 100.;
      else std::cout << "Invalid return :0 " << std::endl;
      break;
    }
  }
  return myreturn;
}

//------------------------------------
void CDCDedxBadWireAlgorithm::setTextCosmetics(TPaveText*& pt)
{
  //pt->SetFillColor(0);
  pt->SetTextAlign(11);
  pt->SetFillStyle(3001);
  pt->SetLineColor(2);
  pt->SetTextFont(82);
  pt->SetTextSize(0.04258064);
}
