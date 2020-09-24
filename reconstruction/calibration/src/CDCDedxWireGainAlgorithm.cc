/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jikumar, jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/calibration/CDCDedxWireGainAlgorithm.h>
#include <TCanvas.h>
#include <TMath.h>
#include <TLine.h>
#include <TStyle.h>
#include <algorithm>
#include <iostream>
#include <TH1I.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
CDCDedxWireGainAlgorithm::CDCDedxWireGainAlgorithm() :
  CalibrationAlgorithm("CDCDedxElectronCollector"),
  m_badWireFPath(""),
  m_badWireFName(""),
  isMakePlots(true),
  isMergePayload(true),
  isLTruc(false),
  isLayerScale(true),
  fdEdxBins(250),
  fdEdxMin(0.0),
  fdEdxMax(5.0),
  fTrucMin(0.05),
  fTrucMax(0.75),
  fTrucLbin(1),
  fTrucHbin(1)
{
  // Set module properties
  setDescription("A calibration algorithm for CDC dE/dx wire gains");
}


//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------
CalibrationAlgorithm::EResult CDCDedxWireGainAlgorithm::calibrate()
{

  // Get data objects
  auto ttree = getObjectPtr<TTree>("tree");
  if (ttree->GetEntries() < 100)return c_NotEnoughData;

  const auto expRun = getRunList()[0];
  updateDBObjPtrs(1, expRun.second, expRun.first);

  std::vector<int>* wire = 0;
  std::vector<double>* dedxhit = 0;
  ttree->SetBranchAddress("wire", &wire);
  ttree->SetBranchAddress("dedxhit", &dedxhit);

  // dedxhit vector to store dE/dx values for each wire
  std::vector<std::vector<double>> wirededx(14336, std::vector<double>());
  for (int i = 0; i < ttree->GetEntries(); ++i) {
    ttree->GetEvent(i);
    for (unsigned int j = 0; j < wire->size(); ++j) {
      wirededx[wire->at(j)].push_back(dedxhit->at(j));
    }
  }

  //25-75 is estimated values for 250 bins
  Int_t lBinILayer = 25,  hBinILayer = 75;
  Int_t lBinOLayer = 25,  hBinOLayer = 75;

  if (!isLTruc) {

    TH1D* hILayer = new TH1D("hILayer", "Inner Layer;dedxhit;entries", fdEdxBins, fdEdxMin, fdEdxMax);
    TH1D* hOLayer = new TH1D("hOLayer", "Outer Layer;dedxhit;entries", fdEdxBins, fdEdxMin, fdEdxMax);

    for (unsigned int jwire = 0; jwire < 14336; ++jwire) {
      for (unsigned int jdedxhit = 0; jdedxhit < wirededx[jwire].size(); ++jdedxhit) {
        double ihitdedx = wirededx[jwire][jdedxhit];
        if (jwire > 0 && jwire < 160 * 8)hILayer->Fill(ihitdedx);
        else hOLayer->Fill(ihitdedx);
      }
    }
    getTrucationBins(hILayer, lBinILayer, hBinILayer);
    getTrucationBins(hOLayer, lBinOLayer, hBinOLayer);

    if (isMakePlots) {
      TCanvas* ctem = new TCanvas("Layerhisto", "Inner and Outer Layer dedxhit dist", 900, 400);
      ctem->Divide(2, 1);
      ctem->cd(1);
      hILayer->SetFillColor(kYellow);
      double lowedge = hILayer->GetXaxis()->GetBinLowEdge(lBinILayer);
      double upedge = hILayer->GetXaxis()->GetBinUpEdge(hBinILayer);
      hILayer->SetTitle(Form("%s, trunc range: %0.02f - %0.02f", hILayer->GetTitle(), lowedge, upedge));
      hILayer->Draw("histo");
      TH1D* hILayerClone = (TH1D*)hILayer->Clone("hILClone");
      hILayerClone->GetXaxis()->SetRange(lBinILayer, hBinILayer);
      hILayerClone->SetFillColor(kAzure + 1);
      hILayerClone->Draw("same histo");
      ctem->cd(2);
      lowedge = hOLayer->GetXaxis()->GetBinLowEdge(lBinOLayer);
      upedge = hOLayer->GetXaxis()->GetBinUpEdge(hBinOLayer);
      hOLayer->SetTitle(Form("%s trunc range: %0.02f - %0.02f", hOLayer->GetTitle(), lowedge, upedge));
      hOLayer->SetFillColor(kYellow);
      hOLayer->Draw("histo");
      TH1D* hOLayerClone = (TH1D*)hOLayer->Clone("hOLClone");
      hOLayerClone->GetXaxis()->SetRange(lBinOLayer, hBinOLayer);
      hOLayerClone->SetFillColor(kAzure + 1);
      hOLayerClone->Draw("same histo");
      ctem->SaveAs("cdcdedx_wiregain_layerdists.pdf");
    }
  }


  TCanvas* ctmp = new TCanvas("tmp", "tmp", 1200, 1200);
  std::stringstream psname; psname << "cdcdedx_wiregain_wiredists.pdf[";
  if (isMakePlots) {
    ctmp->Divide(4, 4);
    ctmp->SetBatch(kTRUE);
    ctmp->Print(psname.str().c_str());
    psname.str(""); psname << "cdcdedx_wiregain_wiredists.pdf";
  }

  //initialisation of wire gains
  double iWireTruncMean[14336];
  for (Int_t jwire = 0; jwire < 14336; jwire++) {
    iWireTruncMean[jwire] = 1.0;
  }

  //Calculations part
  TH1D* htempPerWire = new TH1D("htempPerWire", "blah-blah", fdEdxBins, fdEdxMin, fdEdxMax);

  for (unsigned int jwire = 0; jwire < 14336; ++jwire) {

    htempPerWire->SetName(Form("htempPerWire_%d", jwire));
    htempPerWire->SetTitle(Form("dedxhit-dist, wire # = %d;dedxhit;entries", jwire));
    for (unsigned int jdedxhit = 0; jdedxhit < wirededx[jwire].size(); ++jdedxhit) {
      htempPerWire->Fill(wirededx[jwire][jdedxhit]);
    }

    double truncMean = 1.0;
    int startfrom = 1, endat = 1;

    if (htempPerWire->Integral() == 0) {
      truncMean  = 0.0; //dead wire
    } else if (htempPerWire->Integral() < 500) {
      truncMean  = 1.0; //partial dead or bad wire
    } else {
      if (!isLTruc) {
        if (jwire < 160 * 8) {
          startfrom = lBinILayer; endat = hBinILayer;
        } else {
          startfrom = lBinOLayer; endat = hBinOLayer;
        }
      } else {
        getTrucationBins(htempPerWire, startfrom, endat);
      }

      double binweights = 0.0, sumofbc = 0.0;
      for (int ibin = startfrom; ibin <= endat; ibin++) {
        if (htempPerWire->GetBinContent(ibin) > 0) {
          binweights += (htempPerWire->GetBinContent(ibin) * htempPerWire->GetBinCenter(ibin));
          sumofbc += htempPerWire->GetBinContent(ibin);
        }
      }
      if (sumofbc > 0)truncMean  = binweights / sumofbc;
      else truncMean = 1.0;
    }

    if (truncMean < 0)truncMean = 1.0; // not <=0 as 0 is reseved for dead wire
    iWireTruncMean[jwire] =  truncMean;

    if (isMakePlots) {
      ctmp->cd(jwire % 16 + 1);
      htempPerWire->SetFillColor(kYellow);
      htempPerWire->SetTitle(Form("%s, rel. #mu_{truc} %0.04f", htempPerWire->GetTitle(), iWireTruncMean[jwire]));
      htempPerWire->DrawCopy("hist");
      TH1D* htempPerWireClone = (TH1D*)htempPerWire->Clone(Form("htempPerWireClone_%d", jwire));
      htempPerWireClone->GetXaxis()->SetRange(startfrom, endat);
      htempPerWireClone->SetFillColor(kAzure + 1);
      htempPerWireClone->Draw("same histo");
      // printf("--> wire %u, rel gain = %0.04f \n", jwire, iWireTruncMean[jwire]);
      if ((jwire + 1) % 16 == 0)ctmp->Print(psname.str().c_str());
    }
    htempPerWire->Reset();
  }//end of relative gains

  delete htempPerWire;
  if (isMakePlots) {
    psname.str(""); psname << "cdcdedx_wiregain_wiredists.pdf]";
    ctmp->Print(psname.str().c_str());
    delete ctmp;

    TCanvas* cstats = new TCanvas("cstats", "cstats", 1000, 500);
    cstats->SetBatch(kTRUE);
    cstats->Divide(2, 1);
    cstats->cd(1);
    auto hestats = getObjectPtr<TH1I>("hestats");
    if (hestats) {
      hestats->SetStats(0);
      hestats->DrawCopy("");
    }
    cstats->cd(2);
    auto htstats = getObjectPtr<TH1I>("htstats");
    if (htstats) {
      htstats->DrawCopy("");
      hestats->SetStats(0);
    }
    cstats->Print(Form("cdcdedx_wiregain_stats.pdf"));
    delete cstats;

  }

  //changing to vector array
  std::vector<double> dedxTruncmean;
  for (Int_t jwire = 0; jwire < 14336; jwire++) {
    dedxTruncmean.push_back(iWireTruncMean[jwire]);
  }

  generateNewPayloads(dedxTruncmean);
  return c_OK;
}


void CDCDedxWireGainAlgorithm::generateNewPayloads(std::vector<double> dedxTruncmean)
{

  const auto expRun = getRunList()[0];
  updateDBObjPtrs(1, expRun.second, expRun.first);

  if (isMergePayload) {
    //previous dead but active now ==> abs/merge gain = rel (this is abs by default)
    //previous dead and dead now ==> abs/merge gain = 0.0
    //previous active and active now ==> abs/merge gain = pre*rel
    //previous active but dead now ==> abs/merge gain = 0.0
    //bool refchange = m_DBWireGains.hasChanged(); //for future or manjor processing
    B2INFO("Saving merged wiregains for (Exp, Run) : (" << expRun.first << "," << expRun.second << ")");
    for (unsigned int iwire = 0; iwire < 14336; iwire++) {
      double pre = m_DBWireGains->getWireGain(iwire);
      double rel = dedxTruncmean.at(iwire);
      if (pre != 0.0)dedxTruncmean.at(iwire) *= (double)m_DBWireGains->getWireGain(iwire); //merged or 0.0
      B2INFO("WG for wire [" << iwire << "], previous = " << pre << ", relative = " << rel << ", merged = " << dedxTruncmean.at(iwire));
    }
  } else {
    B2INFO("Saving relative wiregains for (Exp, Run) : (" << expRun.first << "," << expRun.second << ")");
  }

  double layeravg = 1.0;
  if (isLayerScale) {
    layeravg = getLayerAverage(dedxTruncmean);
    for (unsigned int iwire = 0; iwire < 14336; iwire++) {
      //merged or 0.0 for all bad/dead wire
      if (layeravg != 0.0) {
        dedxTruncmean.at(iwire) /= layeravg;
      }
    }
  }

  //saving final constants in a histograms for validation
  if (isMakePlots) {

    TCanvas* cLConst = new TCanvas("cLConst", "cLConst", 1600, 1000);
    std::stringstream psnameL; psnameL << "cdcdedx_wiregain_layerconst.pdf[";
    cLConst->Divide(2, 2);
    cLConst->SetBatch(kTRUE);
    cLConst->Print(psnameL.str().c_str());
    psnameL.str(""); psnameL << "cdcdedx_wiregain_layerconst.pdf";

    TH1D* hWGConst = new TH1D("hWGConst", "wiregain constant; wire numbers;<dedxhit>", 14336, -0.5, 14335.5);
    if (isMergePayload)hWGConst->SetTitle(Form("abs-const: %s", hWGConst->GetTitle()));
    else hWGConst->SetTitle(Form("rel-const: %s", hWGConst->GetTitle()));
    TH1D* hWGConstVar = new TH1D("hWGConstVar", "wiregain variation; wire gains; nentries", 400, -0.5, 3.5);
    if (isMergePayload)hWGConstVar->SetTitle(Form("abs-const: %s", hWGConstVar->GetTitle()));
    else hWGConstVar->SetTitle(Form("rel-const: %s", hWGConstVar->GetTitle()));

    int toWire = 0;
    int countwire = 0, allbadwire = 0;
    for (int iLayer = 0; iLayer < 56; iLayer++) {

      int iSuperLayer = (iLayer - 2) / 6;
      if (iSuperLayer <= 0)iSuperLayer = 1; //hack for wire#
      int nWireiLayer = 160 + (iSuperLayer - 1) * 32;

      int fromWire = countwire;
      toWire = toWire + nWireiLayer;

      TH1D* hLayerConst = new TH1D(Form("hWireConst_L%d", iLayer), "blah-blah", nWireiLayer, fromWire * 1.0, toWire * 1.0);
      if (isMergePayload)hLayerConst->SetTitle(Form("abs-const: Layer = %d; wire numbers;<dedxhit>", iLayer));
      else hLayerConst->SetTitle(Form("rel-const: Layer = %d; wire numbers;<dedxhit>", iLayer));

      int iwire = 0, layerbadwire = 0;
      for (int jwire = fromWire; jwire < toWire; jwire++) {
        countwire++;
        iwire++;
        hLayerConst->SetBinContent(iwire, dedxTruncmean.at(jwire));
        hWGConstVar->Fill(dedxTruncmean.at(jwire));
        if (iLayer < 32 && (iwire % 10 == 0))hLayerConst->GetXaxis()->SetBinLabel(iwire, Form("w%d", jwire));
        else if (iLayer >= 32 && (iwire % 15 == 0))hLayerConst->GetXaxis()->SetBinLabel(iwire, Form("w%d", jwire));
        hWGConst->SetBinContent(countwire, dedxTruncmean.at(jwire));
        if (dedxTruncmean.at(jwire) == 0) {
          allbadwire++;
          layerbadwire++;
        }
        if ((countwire) % 500 == 0)hWGConst->GetXaxis()->SetBinLabel(countwire, Form("w%d", countwire));
      }

      cLConst->cd(iLayer % 4 + 1);
      gStyle->SetOptStat("ne");
      double fraclbad = (100.0 * layerbadwire) / nWireiLayer;
      if (isLayerScale)hLayerConst->SetTitle(Form("%s, avg = %0.04f, nbadwires = %d(%0.02f%%)", hLayerConst->GetTitle(),
                                                    flayerAvg.at(iLayer) / layeravg, layerbadwire, fraclbad));
      else hLayerConst->SetTitle(Form("%s, nbadwires = %d(%0.02f%%)", hLayerConst->GetTitle(), layerbadwire, fraclbad));
      if (iLayer < 8)hLayerConst->GetYaxis()->SetRangeUser(-0.1, 4.0);
      else hLayerConst->GetYaxis()->SetRangeUser(-0.1, 2.0);
      hLayerConst->SetFillColorAlpha(kAzure, 0.10);
      hLayerConst->LabelsOption("u", "X");
      hLayerConst->DrawCopy("hist");
      if (isLayerScale) {
        TLine* tlc = new TLine();
        tlc->SetLineColor(kRed);
        tlc->SetX1(fromWire); tlc->SetX2(toWire);
        tlc->SetY1(flayerAvg.at(iLayer) / layeravg); tlc->SetY2(flayerAvg.at(iLayer) / layeravg);
        tlc->DrawClone("same");
        delete tlc;
      }
      if ((iLayer + 1) % 4 == 0)cLConst->Print(psnameL.str().c_str());
      hLayerConst->Reset();
      delete hLayerConst;
    }

    psnameL.str(""); psnameL << "cdcdedx_wiregain_layerconst.pdf]";
    cLConst->Print(psnameL.str().c_str());
    delete cLConst;

    TCanvas* cConst = new TCanvas("cConst", "cConst", 900, 500);
    cConst->cd();
    cConst->SetGridy(1);
    hWGConst->LabelsOption("u", "X");
    hWGConst->GetYaxis()->SetRangeUser(-0.1, hWGConst->GetMaximum() * 1.05);
    double fracabad = (100.0 * allbadwire) / 14336.0;
    if (isMergePayload)hWGConst->SetTitle(Form("merged %s, nbadwires = %d (%0.02f%%)", hWGConst->GetTitle(), allbadwire, fracabad));
    else hWGConst->SetTitle(Form("relative %s, nbadwires = %d (%0.02f%%)", hWGConst->GetTitle(), allbadwire, fracabad));
    // hWGConst->SetStats(0);
    hWGConst->LabelsDeflate();
    hWGConst->Draw("");
    cConst->SaveAs("cdcdedx_wiregain_allconstants.pdf");

    TCanvas* cConstvar = new TCanvas("cConstvar", "cConstvar", 500, 400);
    cConstvar->cd();
    if (isMergePayload)hWGConstVar->SetTitle(Form("merged %s, nbadwires = %d (%0.02f%%)", hWGConstVar->GetTitle(), allbadwire,
                                                    fracabad));
    else hWGConstVar->SetTitle(Form("relative %s, bad wire = %d (%0.02f)", hWGConstVar->GetTitle(), allbadwire, fracabad));
    //hWGConst->SetStats(0);
    hWGConstVar->SetFillColorAlpha(kAzure, 0.10);
    hWGConstVar->Draw("");
    cConstvar->SaveAs("cdcdedx_wiregain_constantsvar.pdf");
  }

  B2INFO("dE/dx Calibration done for " << dedxTruncmean.size() << " CDC wires");
  CDCDedxWireGain* gains = new CDCDedxWireGain(dedxTruncmean);
  saveCalibration(gains, "CDCDedxWireGain");
}


void CDCDedxWireGainAlgorithm::getTrucationBins(TH1D* htemp, int& binlow, int& binhigh)
{
  //calculating truncation average
  double TotalInt = htemp->Integral();
  if (TotalInt <= 0 || htemp->GetNbinsX() <= 0) {
    binlow = 1.0; binhigh = 1.0;
    return;
  } else {
    binlow = 1.0; binhigh = 1.0;
    double sumPer5 = 0.0, sumPer75 = 0.0;
    for (int ibin = 1; ibin <= htemp->GetNbinsX(); ibin++) {

      if (sumPer5  <= fTrucMin * TotalInt) {
        sumPer5 += htemp->GetBinContent(ibin);
        binlow = ibin;
      }

      if (sumPer75  <= fTrucMax * TotalInt) {
        sumPer75 += htemp->GetBinContent(ibin);
        binhigh = ibin;
      }
    }
  }
}

double CDCDedxWireGainAlgorithm::getLayerAverage(std::vector<double> tempWire)
{
  //calculating layer average
  double jLayerMeanAvg  = 0.0, OutLayerMeanSum = 0.0;
  int toWire = 0;
  int countwire = 0, OutActLayer = 0;

  TH1D* hLayerAvg = new TH1D("hLayerAvg", "Layer vs trunc mean avg; layer numbers;<dedxhit>", 56, -0.5, 55.5);

  for (int iLayer = 0; iLayer < 56; iLayer++) {

    int iSuperLayer = (iLayer - 2) / 6;
    if (iSuperLayer <= 0)iSuperLayer = 1; //hack for wire#
    int nWireiLayer = 160 + (iSuperLayer - 1) * 32;

    int fromWire = countwire; // or towire+countwire
    toWire = toWire + nWireiLayer;

    double jLayerMeanSum = 0.0;
    int jLayerActWires = 0;
    std::cout << "iLayer = " << iLayer << ", from wire = " << fromWire << ", to wire = " << toWire << std::endl;
    for (int jwire = fromWire; jwire < toWire; jwire++) {
      countwire++;
      if (tempWire.at(jwire) > 0.) { //active wire only
        jLayerMeanSum += tempWire.at(jwire);
        jLayerActWires++;
      } else continue;
    }

    if (jLayerActWires > 0)jLayerMeanAvg = jLayerMeanSum / jLayerActWires;
    else jLayerMeanAvg = 0.0;

    flayerAvg.push_back(jLayerMeanAvg);
    hLayerAvg->SetBinContent(iLayer + 1, jLayerMeanAvg);
    if ((iLayer + 1) % 2 == 0)hLayerAvg->GetXaxis()->SetBinLabel(iLayer + 1, Form("L%d", iLayer));
    std::cout << " \t --> sum = " << jLayerMeanSum << ", active wires = " << jLayerActWires << ", average = " << jLayerMeanAvg <<
              std::endl;

    if (iLayer >= 8 && jLayerMeanAvg > 0.0) {
      OutLayerMeanSum += jLayerMeanAvg;
      OutActLayer++; //if any layer is completely dead
    }
  }

  double OutLayerMeanAvg = 1.0;
  if (OutActLayer > 0) OutLayerMeanAvg = OutLayerMeanSum / OutActLayer;

  if (isMakePlots) {
    TCanvas* cLAvg = new TCanvas("clayerAvg", "clayerAvg", 800, 500);
    cLAvg->SetGridy(1);
    cLAvg->cd();
    gStyle->SetOptStat("ne");
    hLayerAvg->LabelsOption("u", "X");
    hLayerAvg->SetLineColor(kBlue);
    hLayerAvg->GetYaxis()->SetRangeUser(-0.1, hLayerAvg->GetMaximum() * 1.20);
    if (isMergePayload)hLayerAvg->SetTitle(Form("%s, avg = %0.04f (abs)", hLayerAvg->GetTitle(), OutLayerMeanAvg));
    else hLayerAvg->SetTitle(Form("%s, avg = %0.04f (rel)", hLayerAvg->GetTitle(), OutLayerMeanAvg));
    hLayerAvg->LabelsDeflate();
    hLayerAvg->Draw("");
    TLine* tl = new TLine();
    tl->SetLineColor(kRed);
    tl->SetX1(-0.5); tl->SetX2(55.5);
    tl->SetY1(OutLayerMeanAvg); tl->SetY2(OutLayerMeanAvg);
    tl->DrawClone("same");
    cLAvg->SaveAs("cdcdedx_wiregain_layeravg.pdf");
  }
  return OutLayerMeanAvg;
}