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
  fdedxBin(250),
  fdedxMin(0.0),
  fdedxMax(5.0),
  fmeanThers(2.0),
  frmsThers(1.5),
  ffracThers(5.0),
  bmakePlots(true),
  bprintLog(true),
  saddSfx(""),
  nwireCDC(14336)
{
  // Set module properties
  setDescription("A calibration algorithm for CDC dE/dx bad wires");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------
CalibrationAlgorithm::EResult CDCDedxBadWireAlgorithm::calibrate()
{

  // Get data objects
  auto ttree = getObjectPtr<TTree>("tree");
  if (ttree->GetEntries() < 1000)return c_NotEnoughData;

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

  //check stat with last three layers
  bool lowstat = true;
  for (unsigned int jwire = 13184; jwire < nwireCDC; ++jwire) {
    if (wirededx[jwire].size() >= 0) {
      lowstat = false;
      break;
    }
  }

  if (lowstat) {
    std::cout << "No enough to do badwire calibration" << std::endl;
    return c_NotEnoughData;
  }

  getExpRunInfo();

  TH1D* hdedx = new TH1D("hdedx", "temp", fdedxBin, fdedxMin, fdedxMax);
  TH1D* hmean = new TH1D(Form("hmean_%s", saddSfx.data()), "dedx mean vs wire#", nwireCDC, -0.5, 14335.5);
  TH1D* hrms = new TH1D(Form("hrms_%s", saddSfx.data()), "dedx rms vs wire#", nwireCDC, -0.5, 14335.5);
  TH1D* hfrac = new TH1D(Form("hfrac_%s", saddSfx.data()), "dedx high-frac vs wire#", nwireCDC, -0.5, 14335.5);

  std::ofstream outbadwireFile, outdeadwireFile;
  outbadwireFile.open(Form("cdcdedx_badwire_badlist_%s.txt", saddSfx.data()));
  outdeadwireFile.open(Form("cdcdedx_badwire_deadlist_%s.txt", saddSfx.data()));

  std::stringstream psname; psname << Form("cdcdedx_badwire_dedx_%s.pdf[", saddSfx.data());
  TCanvas* ctmp = new TCanvas("tmp", "tmp", 1200, 1200);
  if (bmakePlots) {
    ctmp->Divide(4, 4);
    ctmp->SetBatch(kTRUE);
    ctmp->Print(psname.str().c_str());
    psname.str(""); psname << Form("cdcdedx_badwire_dedx_%s.pdf", saddSfx.data());
  }

  int dwire = 0;
  std::vector<double> m_defectwire;
  for (unsigned int jwire = 0; jwire < nwireCDC; ++jwire) {

    hdedx->SetName(Form("hdedx_%s_w%d", saddSfx.data(), jwire));
    hdedx->SetTitle(Form("wire# %d (%s); dedx; entries", jwire, saddSfx.data()));

    double highfrac = 0.0, therssum = 0.0, normalsum = 0.0;
    for (unsigned int jhit = 0; jhit < wirededx[jwire].size(); ++jhit) {
      if (wirededx[jwire][jhit] < fdedxMax) {
        normalsum += wirededx[jwire][jhit];
        hdedx->Fill(wirededx[jwire][jhit]);
      } else {
        therssum += 5.5;
      }
    }

    if (normalsum != 0) highfrac = 100. * double(therssum) / double(normalsum + therssum);

    hfrac->SetBinContent(jwire, highfrac);
    hmean->SetBinContent(jwire, hdedx->GetMean());
    hrms->SetBinContent(jwire, hdedx->GetRMS());

    bool isdeadWire = false, isbadWire = false;
    if (hdedx->Integral() <= 100) {
      outdeadwireFile <<  jwire << std::endl;
      dwire++;
      isdeadWire = true;
    } else {
      if (hdedx->GetMean() <  fmeanThers || hdedx->GetRMS() < frmsThers) continue;
      outbadwireFile <<  jwire << std::endl;
      dwire++;
      isbadWire = true;
    }


    if (isdeadWire || isbadWire) {

      std::cout << jwire  << " size --> " << wirededx[jwire].size() <<  std::endl;

      ctmp->cd(dwire % 16);
      if (isdeadWire) {
        hdedx->SetTitle(Form("dead(partial) wire# = %d, %s;dedxhit;entries", jwire, saddSfx.data()));
        hdedx->SetFillColorAlpha(kRed, 0.60);
      } else {
        hdedx->SetTitle(Form("wire# = %d, bad, %s;dedxhit;entries", jwire, saddSfx.data()));
        hdedx->SetFillColorAlpha(kYellow, 0.60);
      }

      hdedx->DrawCopy("hist");

      if ((dwire % 16) == 0 || (jwire = 14335))  {
        ctmp->Print(psname.str().c_str());
        gPad->Clear();
      }
      m_defectwire.push_back(0.0);
    } else {
      m_defectwire.push_back(1.0);
    }
    hdedx->Reset();
  }

  outbadwireFile.close();
  outdeadwireFile.close();

  if (bmakePlots) {

    psname.str(""); psname << Form("cdcdedx_badwire_dedx_%s.pdf]", saddSfx.data());
    ctmp->Print(psname.str().c_str());
    delete ctmp;

    std::string sfx[3] = {"highfrac", "mean", "rms"};
    TCanvas* cQu = new TCanvas("cQu", "cQu", 1000, 500);
    for (int ic = 0; ic <= 0; ic++) {
      cQu->cd();
      gPad->SetGridy();
      if (ic == 0) hfrac->Draw();
      else if (ic == 1)hmean->Draw();
      else if (ic == 2)hrms->Draw();
      cQu->Print(Form("cdcdedx_badwire_qa_%s_%s.pdf", sfx[ic].data(), saddSfx.data()));
    }

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

    cstats->Print(Form("cdcdedx_badwire_qa_stats_%s.pdf", saddSfx.data()));
    delete cstats;
  }

  //too many dead+bad wires (>10%)
  if (dwire > 1400)return c_NotEnoughData;

  B2INFO("dE/dx Badwire calibration done: " << m_defectwire.size() << " wires");
  CDCDedxBadWires* c_badwires = new CDCDedxBadWires(m_defectwire);
  saveCalibration(c_badwires, "CDCDedxBadWires");
  return c_OK;

}

void CDCDedxBadWireAlgorithm::getExpRunInfo()
{

  int countrun = 0;
  for (auto expRun : getRunList()) {
    if (bprintLog)std::cout << "\t" << expRun.first << "," << expRun.second << std::endl;
    countrun++;
  }

  const auto erStart = getRunList()[0];
  const auto erEnd = getRunList()[countrun - 1];
  int estart = erStart.first;
  int rstart = erStart.second;
  int eend = erEnd.first;
  int rend = erEnd.second;

  updateDBObjPtrs(1, rstart, estart);
  if (!m_DBBadWires.isValid())
    B2FATAL("There is no valid payload for CDCDedxBadWires");

  if (saddSfx.length() > 0)saddSfx = Form("_%d_%d_%d_%d_%s", estart, rstart, eend, rend, saddSfx.data());
  else saddSfx = Form("_%d_%d_%d_%d", estart, rstart, eend, rend);

}
