/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This fiLe is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <TCanvas.h>
#include <TMath.h>
#include <TLine.h>
#include <TStyle.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <TH1I.h>
#include <TPaveText.h>
#include <TText.h>
#include <TLegend.h>
#include <reconstruction/calibration/CDCDedxWireGainAlgorithm.h>
using namespace Belle2;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
CDCDedxWireGainAlgorithm::CDCDedxWireGainAlgorithm() :
  CalibrationAlgorithm("CDCDedxElectronCollector"),
  nwireCDC(14336),
  isExtPlot(true),
  isMerge(true),
  isWireTruc(false),
  isLayerScale(true),
  fdEdxBins(250),
  fdEdxMin(0.0),
  fdEdxMax(5.0),
  fTrucMin(0.05),
  fTrucMax(0.75),
  saddSfx("")
{
  // Set module properties
  setDescription("A calibration algorithm for CDC dE/dx wire gains");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------
CalibrationAlgorithm::EResult CDCDedxWireGainAlgorithm::calibrate()
{

  //if not checking then copy function here;
  getExpRunInfo();

  if (!m_DBBadWires.isValid() || !m_DBWireGains.isValid())
    B2FATAL("There is no valid payload for BadWires and/or Wirgain");

  // Get data objects
  auto ttree = getObjectPtr<TTree>("tree");
  if (ttree->GetEntries() < 100)return c_NotEnoughData;

  std::vector<int>* wire = 0;
  std::vector<double>* dedxhit = 0;
  ttree->SetBranchAddress("wire", &wire);
  ttree->SetBranchAddress("dedxhit", &dedxhit);

  // dedxhit vector to store dE/dx values for each wire
  std::vector<std::vector<double>> wirededx(nwireCDC, std::vector<double>());
  for (int i = 0; i < ttree->GetEntries(); ++i) {
    ttree->GetEvent(i);
    for (unsigned int j = 0; j < wire->size(); ++j) {
      wirededx[wire->at(j)].push_back(dedxhit->at(j));
    }
  }

  //check statistics of dead or limited statiscs wire
  int minstat = 0;
  for (unsigned int jw = 0; jw < nwireCDC; ++jw) {
    if (wirededx[jw].size() == 0) minstat++;
  }

  double deadfrac = 100.0 * minstat / nwireCDC;
  if (deadfrac > 15.0) {
    B2INFO("CDCDedxWireGain: too many dead or low stats wires? " << deadfrac << "%");
    return c_NotEnoughData;
  }

  //25-75 is estimated values for 250 bins
  Int_t lbiniL = 25,  hBiniL = 75;
  Int_t lBinOL = 25,  hBinOL = 75;

  TH1D* hdedxiL = new TH1D(Form("hdedxiL_%s", saddSfx.data()), "", fdEdxBins, fdEdxMin, fdEdxMax);
  hdedxiL->SetTitle(Form("IL(%s);dedxhit;entries", saddSfx.data()));

  TH1D* hdedxOL = new TH1D(Form("hdedxOL_%s", saddSfx.data()), "", fdEdxBins, fdEdxMin, fdEdxMax);
  hdedxOL->SetTitle(Form("OL (%s);dedxhit;entries", saddSfx.data()));

  if (!isWireTruc) { //if superlayer based truncation
    for (unsigned int jw = 0; jw < nwireCDC; ++jw) {
      for (unsigned int jhit = 0; jhit < wirededx[jw].size(); ++jhit) {
        double ihitdedx = wirededx[jw][jhit];
        if (jw > 0 && jw < 160 * 8)hdedxiL->Fill(ihitdedx);
        else hdedxOL->Fill(ihitdedx);
      }
    }
    getTrucationBins(hdedxiL, lbiniL, hBiniL);
    getTrucationBins(hdedxOL, lBinOL, hBinOL);
  }

  double lowedge = hdedxiL->GetXaxis()->GetBinLowEdge(lbiniL);
  double upedge = hdedxiL->GetXaxis()->GetBinUpEdge(hBiniL);

  TCanvas* cldedx = new TCanvas("cldedx", "iL/OL dedxhit dist", 900, 400);
  cldedx->Divide(2, 1);

  cldedx->cd(1);
  hdedxiL->SetFillColor(kYellow);
  hdedxiL->SetTitle(Form("%s, trunc(%0.02f - %0.02f)", hdedxiL->GetTitle(), lowedge, upedge));
  hdedxiL->Draw("histo");
  TH1D* hdedxiLC = (TH1D*)hdedxiL->Clone(Form("%s_c", hdedxiL->GetTitle()));
  hdedxiLC->GetXaxis()->SetRange(lbiniL, hBiniL);
  hdedxiLC->SetFillColor(kAzure + 1);
  hdedxiLC->Draw("same histo");

  cldedx->cd(2);
  lowedge = hdedxOL->GetXaxis()->GetBinLowEdge(lBinOL);
  upedge = hdedxOL->GetXaxis()->GetBinUpEdge(hBinOL);
  hdedxOL->SetTitle(Form("%s trunc range: %0.02f - %0.02f", hdedxOL->GetTitle(), lowedge, upedge));
  hdedxOL->SetFillColor(kYellow);
  hdedxOL->Draw("histo");
  TH1D* hdedxOLC = (TH1D*)hdedxOL->Clone(Form("%s_c", hdedxOL->GetTitle()));
  hdedxOLC->GetXaxis()->SetRange(lBinOL, hBinOL);
  hdedxOLC->SetFillColor(kAzure + 1);
  hdedxOLC->Draw("same histo");

  cldedx->SaveAs(Form("cdcdedx_wgcal_layerdedx_%s.pdf", saddSfx.data()));
  delete cldedx;

  //wire dedx
  TCanvas* cgwire = new TCanvas("cgwire", "tmp", 1200, 1200);
  std::stringstream psgood; psgood << Form("cdcdedx_wgcal_gwirededx_%s.pdf[", saddSfx.data());
  cgwire->Divide(4, 4);
  cgwire->SetBatch(kTRUE);
  cgwire->Print(psgood.str().c_str());
  psgood.str("");
  psgood << Form("cdcdedx_wgcal_gwirededx_%s.pdf", saddSfx.data());

  TCanvas* cbwire = new TCanvas("cbwire", "tmp", 1200, 1200);
  std::stringstream psbad; psbad << Form("cdcdedx_wgcal_bwirededx_%s.pdf[", saddSfx.data());
  cbwire->Divide(4, 4);
  cbwire->SetBatch(kTRUE);
  cbwire->Print(psbad.str().c_str());
  psbad.str("");
  psbad << Form("cdcdedx_wgcal_bwirededx_%s.pdf", saddSfx.data());

  double itrucmean[nwireCDC];
  int nbadwire = 0, ngoodwire = 0;

  for (unsigned int jw = 0; jw < nwireCDC; ++jw) {

    TH1D* hdedxhit = new TH1D(Form("h%s_w%d", saddSfx.data(), jw), "", fdEdxBins, fdEdxMin, fdEdxMax);
    hdedxhit->SetTitle(Form("dedxhit-dist, wire: %d (%s);dedxhit;entries", jw, saddSfx.data()));
    for (unsigned int jhit = 0; jhit < wirededx[jw].size(); ++jhit) {
      hdedxhit->Fill(wirededx[jw][jhit]);
    }

    itrucmean[jw] = 1.0;
    int hitsum = hdedxhit->Integral();

    if (hitsum < 50 || m_DBBadWires->getBadWireStatus(jw) == kTRUE) {
      cbwire->cd(nbadwire % 16 + 1);
      hdedxhit->SetFillColor(kYellow - 9);
      hdedxhit->SetTitle(Form("%s, rel. #mu_{truc} %0.03f", hdedxhit->GetTitle(), itrucmean[jw]));
      hdedxhit->DrawCopy("hist");
      if ((nbadwire + 1) % 16 == 0) {
        cbwire->Print(psbad.str().c_str());
        cbwire->Clear("D");
      }
      itrucmean[jw]  = 0.0;
      nbadwire++;
    } else {
      int start = 1, end = 1;
      if (!isWireTruc) {
        if (jw < 160 * 8) {
          start = lbiniL; end = hBiniL;
        } else {
          start = lBinOL; end = hBinOL;
        }
      } else {
        getTrucationBins(hdedxhit, start, end);
      }

      double binweights = 0.0, sumofbc = 0.0;
      for (int ibin = start; ibin <= end; ibin++) {
        if (hdedxhit->GetBinContent(ibin) > 0) {
          binweights += (hdedxhit->GetBinContent(ibin) * hdedxhit->GetBinCenter(ibin));
          sumofbc += hdedxhit->GetBinContent(ibin);
        }
      }

      if (sumofbc > 0)itrucmean[jw]  = binweights / sumofbc;
      else itrucmean[jw] = 1.0;

      cgwire->cd(ngoodwire % 16 + 1);
      hdedxhit->SetFillColor(kYellow);
      hdedxhit->SetTitle(Form("%s, rel. #mu_{truc} %0.03f", hdedxhit->GetTitle(), itrucmean[jw]));
      hdedxhit->DrawCopy("hist");
      TH1D* hdedxhitC = (TH1D*)hdedxhit->Clone(Form("%sC", hdedxhit->GetName()));
      hdedxhitC->GetXaxis()->SetRange(start, end);
      hdedxhitC->SetFillColor(kAzure + 1);
      hdedxhitC->DrawCopy("same histo");

      if ((ngoodwire + 1) % 16 == 0) {
        cgwire->Print(psgood.str().c_str());
        cgwire->Clear("D");
      }
      ngoodwire++;
    }

    if (jw == nwireCDC - 1) {
      cbwire->Print(psbad.str().c_str());
      cgwire->Print(psgood.str().c_str());
    }
    delete hdedxhit;
  }

  psgood.str("");
  psgood << Form("cdcdedx_wgcal_gwirededx_%s.pdf]", saddSfx.data());
  cgwire->Print(psgood.str().c_str());
  delete cgwire;

  psbad.str("");
  psbad << Form("cdcdedx_wgcal_bwirededx_%s.pdf]", saddSfx.data());
  cbwire->Print(psbad.str().c_str());
  delete cbwire;

  if (isExtPlot) {
    TCanvas* cstats = new TCanvas("cstats", "cstats", 800, 400);
    cstats->SetBatch(kTRUE);
    cstats->Divide(2, 1);

    cstats->cd(1);
    auto hestats = getObjectPtr<TH1I>("hestats");
    if (hestats) {
      hestats->SetName(Form("hestats_%s", saddSfx.data()));
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

    cstats->Print(Form("cdcdedx_wgcal_stats_%s.pdf", saddSfx.data()));
    delete cstats;
  }

  //changing to vector array
  std::vector<double> vdedx_tmeans;
  for (unsigned int jw = 0; jw < nwireCDC; jw++)vdedx_tmeans.push_back(itrucmean[jw]);

  createPayload(vdedx_tmeans);

  saddSfx.clear();

  return c_OK;
}

//------------------------------------
void CDCDedxWireGainAlgorithm::getExpRunInfo()
{

  int cruns = 0;
  for (auto expRun : getRunList()) {
    if (cruns == 0)B2INFO("CDCDedxWireGain: start exp " << expRun.first << " and run " << expRun.second << "");
    cruns++;
  }

  const auto erStart = getRunList()[0];
  int estart = erStart.first;
  int rstart = erStart.second;

  const auto erEnd = getRunList()[cruns - 1];
  int rend = erEnd.second;

  updateDBObjPtrs(1, rstart, estart);

  if (saddSfx.length() > 0)saddSfx = Form("%s_e%d_r%dr%d", saddSfx.data(), estart, rstart, rend);
  else  saddSfx = Form("e%d_r%dr%d", estart, rstart, rend);
}

//--------------------------
void CDCDedxWireGainAlgorithm::createPayload(std::vector<double> vdedx_tmeans)
{

  //saving final constants in a histograms for validation
  TH1D* hconstpw = new TH1D(Form("hconstpw_%s", saddSfx.data()), "", nwireCDC, -0.5, 14335.5);
  hconstpw->SetTitle(Form("wiregain rel-const  (%s); wire numbers;<dedxhit>", saddSfx.data()));
  if (isMerge)hconstpw->SetTitle(Form("wiregain abs-const  (%s); wire numbers;<dedxhit>", saddSfx.data()));

  TH1D* hconstpwvar = new TH1D(Form("hconstpwvar_%s", saddSfx.data()), "", 400, -0.5, 2.5);
  hconstpwvar->SetTitle(Form("wiregain rel-const (%s); wire gains; nentries", saddSfx.data()));
  if (isMerge)hconstpwvar->SetTitle(Form("wiregain abs-const (%s); wire gains; nentries", saddSfx.data()));

  TH1D* hrelconstpw = new TH1D(Form("hrelconstpw_%s", saddSfx.data()), "", nwireCDC, -0.5, 14335.5);
  hrelconstpw->SetTitle(Form("wiregain diff  (%s); wire numbers;<dedxhit>", saddSfx.data()));

  TH1D* hrelconstpwvar = new TH1D(Form("hrelconstpwvar_%s", saddSfx.data()), "", 400, -0.50, 2.50);
  hrelconstpwvar->SetTitle(Form("wiregain diff (%s); wire gains; nentries", saddSfx.data()));

  TH1D* hlayeravg = new TH1D(Form("hlayeravg_%s", saddSfx.data()), "", 56, -0.5, 55.5);
  hlayeravg->SetTitle(Form("layer gain avg (%s); layer numbers;<dedxhit>", saddSfx.data()));


  if (isMerge) {
    B2INFO("Saving merged wiregains for (%s) : (" << saddSfx << ")");
    for (unsigned int iw = 0; iw < nwireCDC; iw++) {
      double pre = m_DBWireGains->getWireGain(iw);
      double rel = vdedx_tmeans.at(iw);
      hrelconstpw->SetBinContent(iw + 1, rel);
      hrelconstpwvar->Fill(rel);
      if (pre != 0.0)vdedx_tmeans.at(iw) *= (double)m_DBWireGains->getWireGain(iw);
      B2INFO("merged-wireGain: [" << iw << "], previous = " << pre << ", rel = " << rel << ", merged = " << vdedx_tmeans.at(iw));
    }
  } else {
    B2INFO("absolute-wireGain: " << saddSfx.data());
  }

  double layeravg = 1.0;
  if (isLayerScale) {
    layeravg = getLayerAverage(vdedx_tmeans);
    for (unsigned int iw = 0; iw < nwireCDC; iw++) {
      if (layeravg != 0.0)vdedx_tmeans.at(iw) /= layeravg;
    }
  }

  TCanvas* clconst = new TCanvas("clconst", "", 800, 500);
  std::stringstream psnameL; psnameL << Form("cdcdedx_wgcal_layerconst_%s.pdf[", saddSfx.data());
  clconst->Divide(2, 2);
  clconst->SetBatch(kTRUE);
  clconst->Print(psnameL.str().c_str());
  psnameL.str(""); psnameL << Form("cdcdedx_wgcal_layerconst_%s.pdf", saddSfx.data());

  int twire = 0, cwire = 0;

  for (int iL = 0; iL < 56; iL++) {

    int iSL = (iL - 2) / 6;
    if (iSL <= 0)iSL = 1; //hack for wire#
    int nwires_iL = 160 + (iSL - 1) * 32;

    int fwire = cwire;
    twire = twire + nwires_iL;

    hlayeravg->SetBinContent(iL + 1, vlayerAvg.at(iL));
    if (abs((iL - 2)) % 6 == 0 || iL == 55)hlayeravg->GetXaxis()->SetBinLabel(iL + 1, Form("L%d", iL));

    TH1D* hconstpl = new TH1D(Form("hconstpwvar_l%d_%s", iL, saddSfx.data()), "", nwires_iL, fwire, twire);
    if (isMerge)hconstpl->SetTitle(Form("abs-const, layer: %d (%s); wire numbers;<dedxhit>", iL, saddSfx.data()));
    else hconstpl->SetTitle(Form("rel-const, layer: %d (%s); wire numbers;<dedxhit>", iL, saddSfx.data()));

    int iw = 0;
    for (int jw = fwire; jw < twire; jw++) {

      iw++;
      hconstpl->SetBinContent(iw, vdedx_tmeans.at(jw));
      if (iL < 32) {
        if (iw % 10 == 0)hconstpl->GetXaxis()->SetBinLabel(iw, Form("w%d", jw));
      } else {
        if (iw % 15 == 0)hconstpl->GetXaxis()->SetBinLabel(iw, Form("w%d", jw));
      }

      cwire++;
      hconstpw->SetBinContent(cwire, vdedx_tmeans.at(jw));
      if (cwire % 500 == 0)hconstpw->GetXaxis()->SetBinLabel(cwire, Form("w%d", cwire));

      hconstpwvar->Fill(vdedx_tmeans.at(jw));
    }

    clconst->cd(iL % 4 + 1);
    gStyle->SetOptStat("ne");
    if (isLayerScale)hconstpl->SetTitle(Form("%s, avg = %0.03f", hconstpl->GetTitle(), vlayerAvg.at(iL) / layeravg));
    else hconstpl->SetTitle(Form("%s, avg = %0.03f", hconstpl->GetTitle(), vlayerAvg.at(iL)));

    if (iL < 8)hconstpl->GetYaxis()->SetRangeUser(-0.1, 4.0);
    else hconstpl->GetYaxis()->SetRangeUser(-0.1, 2.0);
    hconstpl->SetFillColor(kAzure - 1);
    hconstpl->LabelsOption("u", "X");
    hconstpl->DrawCopy("hist");
    if (isLayerScale) {
      TLine* tlc = new TLine();
      tlc->SetLineColor(kRed);
      tlc->SetX1(fwire); tlc->SetX2(twire);
      tlc->SetY1(vlayerAvg.at(iL) / layeravg); tlc->SetY2(vlayerAvg.at(iL) / layeravg);
      tlc->DrawClone("same");
      delete tlc;
    }
    if ((iL + 1) % 4 == 0)clconst->Print(psnameL.str().c_str());
    hconstpl->Reset();
    delete hconstpl;
  }

  psnameL.str(""); psnameL << Form("cdcdedx_wgcal_layerconst_%s.pdf]", saddSfx.data());
  clconst->Print(psnameL.str().c_str());
  delete clconst;

  TCanvas* clayeravg = new TCanvas("clayeravg", "clayeravg", 800, 500);
  clayeravg->SetGridy(1);
  clayeravg->cd();
  gStyle->SetOptStat("ne");
  hlayeravg->LabelsOption("u", "X");
  hlayeravg->SetLineColor(kBlue);
  hlayeravg->GetYaxis()->SetRangeUser(-0.1, hlayeravg->GetMaximum() * 1.20);
  if (isMerge)hlayeravg->SetTitle(Form("%s, avg = %0.03f (abs)", hlayeravg->GetTitle(), layeravg));
  else hlayeravg->SetTitle(Form("%s, avg = %0.03f (rel)", hlayeravg->GetTitle(), layeravg));
  hlayeravg->LabelsDeflate();
  hlayeravg->Draw("");
  TLine* tl = new TLine();
  tl->SetLineColor(kRed);
  tl->SetX1(-0.5); tl->SetX2(55.5);
  tl->SetY1(layeravg); tl->SetY2(layeravg);
  tl->DrawClone("same");
  clayeravg->SaveAs(Form("cdcdedx_wgcal_layeravg_%s.pdf", saddSfx.data()));
  delete clayeravg;

  TCanvas* cwconst = new TCanvas("cwconst", "", 900, 500);
  cwconst->cd();
  cwconst->SetGridy(1);
  hconstpw->LabelsOption("u", "X");
  hconstpw->GetYaxis()->SetRangeUser(-0.1, hconstpw->GetMaximum() * 1.05);
  if (isMerge)hconstpw->SetTitle(Form("merged %s, avg = %0.03f", hconstpw->GetTitle(), layeravg));
  else hconstpw->SetTitle(Form("relative %s, avg = %0.03f", hconstpw->GetTitle(), layeravg));
  hconstpw->LabelsDeflate();
  hconstpw->Draw("");
  hrelconstpw->SetLineColor(kRed);
  hrelconstpw->Draw("same");
  cwconst->SaveAs(Form("cdcdedx_wgcal_wireconst_%s.pdf", saddSfx.data()));
  delete cwconst;

  TCanvas* cwconstvar = new TCanvas("cwconstvar", "", 500, 400);
  cwconstvar->cd();
  if (isMerge)cwconstvar->SetTitle(Form("merged %s, avg = %0.03f", hconstpw->GetTitle(), layeravg));
  else cwconstvar->SetTitle(Form("relative %s, avg = %0.03f", hconstpw->GetTitle(), layeravg));
  hconstpwvar->SetFillColor(kAzure);
  hrelconstpwvar->SetFillColor(kRed);
  hconstpwvar->Scale(1 / hconstpwvar->GetMaximum());
  hrelconstpwvar->Scale(1 / hrelconstpwvar->GetMaximum());
  hconstpwvar->Draw("hist");
  hrelconstpwvar->Draw("hist same");
  cwconstvar->SaveAs(Form("cdcdedx_wgcal_wireconstvar_%s.pdf", saddSfx.data()));
  delete cwconstvar;

  std::ofstream fgoodwires;
  fgoodwires.open(Form("cdcdedx_wgcal_wireconst_%s.txt", saddSfx.data()));
  for (unsigned int iw = 0; iw < vdedx_tmeans.size(); iw++) {
    if (vdedx_tmeans.at(iw) > 0)fgoodwires << iw << "" << vdedx_tmeans.at(iw) << std::endl;
  }
  fgoodwires.close();

  B2INFO("dE/dx Calibration done for " << vdedx_tmeans.size() << " CDC wires");
  CDCDedxWireGain* gains = new CDCDedxWireGain(vdedx_tmeans);
  saveCalibration(gains, "CDCDedxWireGain");
}

//--------------------------
void CDCDedxWireGainAlgorithm::getTrucationBins(TH1D* hdedxhit, int& binlow, int& binhigh)
{

  //calculating truncation average
  double sum = hdedxhit->Integral();
  if (sum <= 0 || hdedxhit->GetNbinsX() <= 0) {
    binlow = 1.0; binhigh = 1.0;
    return;
  } else {
    binlow = 1.0; binhigh = 1.0;
    double sumPer5 = 0.0, sumPer75 = 0.0;
    for (int ibin = 1; ibin <= hdedxhit->GetNbinsX(); ibin++) {

      if (sumPer5  <= fTrucMin * sum) {
        sumPer5 += hdedxhit->GetBinContent(ibin);
        binlow = ibin;
      }

      if (sumPer75  <= fTrucMax * sum) {
        sumPer75 += hdedxhit->GetBinContent(ibin);
        binhigh = ibin;
      }
    }
  }
}

//--------------------------
double CDCDedxWireGainAlgorithm::getLayerAverage(std::vector<double> tempWire)
{

  //calculating layer average
  double outsum = 0.0;
  int twire = 0;
  int cwire = 0, outlayers = 0;


  for (int iL = 0; iL < 56; iL++) {

    int iSL = (iL - 2) / 6;
    if (iSL <= 0)iSL = 1; //hack for wire#
    int nwires_iL = 160 + (iSL - 1) * 32;

    int fwire = cwire; // or twire+cwire
    twire = twire + nwires_iL;

    double jlayersum = 0.0;
    int jlayercount = 0;
    for (int jw = fwire; jw < twire; jw++) {
      cwire++;
      if (tempWire.at(jw) > 0.) { //active wire only
        jlayersum += tempWire.at(jw);
        jlayercount++;
      }
    }

    double jlayeravg  = 0.0;
    if (jlayercount > 0)jlayeravg = jlayersum / jlayercount;
    vlayerAvg.push_back(jlayeravg);
    B2INFO("\t CDCDedxBadWires: iL = " << iL << ", sum = " << jlayersum << ", active-wires = " << jlayercount
           << ", avg = " << jlayeravg);

    if (iL >= 8 && jlayeravg > 0.0) {
      outsum += jlayeravg;
      outlayers++; //if any layer is completely dead
    }
  }

  double outlayeravg = 1.0;
  if (outlayers > 0) outlayeravg = outsum / outlayers;

  return outlayeravg;
}