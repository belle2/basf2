/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <algorithm>
#include <iostream>

#include <TCanvas.h>
#include <TH1F.h>
#include <TLine.h>
#include <TTree.h>
#include <TMath.h>

#include <reconstruction/calibration/CDCDedx1DCellAlgorithm.h>

using namespace Belle2;


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDedx1DCellAlgorithm::CDCDedx1DCellAlgorithm() :
  CalibrationAlgorithm("CDCDedxElectronCollector"),
  fnEntaBinG(128),
  fnEntaBinL(64),
  feaLE(-TMath::Pi() / 2),
  feaUE(+TMath::Pi() / 2),
  IsLocalBin(true),
  IsPrintBinMap(true),
  IsMakePlots(false)
{
  // Set module properties
  setDescription("A calibration algorithm for the CDC dE/dx entrance angle cleanup correction");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------

CalibrationAlgorithm::EResult CDCDedx1DCellAlgorithm::calibrate()
{

  //reading electron collector TREE
  auto ttree = getObjectPtr<TTree>("tree");
  if (ttree->GetEntries() < 100)return c_NotEnoughData;

  std::vector<double>* dedxhit = 0, *enta = 0;
  std::vector<int>* layer = 0;

  ttree->SetBranchAddress("dedxhit", &dedxhit);
  ttree->SetBranchAddress("layer", &layer);
  ttree->SetBranchAddress("entaRS", &enta);

  // Setting up bins for entra angle
  feaBS = (feaUE - feaLE) / fnEntaBinG;

  if (IsLocalBin)GlobalToLocalEntaBinMap(IsPrintBinMap);
  else fnEntaBinL =  fnEntaBinG;

  //enta dedx distributions for inner and outer layer
  TH1F* hILdEdxhitInEntaBin[fnEntaBinL];
  TH1F* hOLdEdxhitInEntaBin[fnEntaBinL];

  Double_t ifeaLE = 0, ifeaUE = 0;

  for (int iea = 0; iea < fnEntaBinL; iea++) {

    if (IsLocalBin) {
      ifeaLE = fEntaBinValues.at(iea);
      ifeaUE = fEntaBinValues.at(iea + 1);
    } else {
      ifeaLE = iea * feaBS - feaUE; //- because of -ive range shifting
      ifeaUE = ifeaLE + feaBS;
    }

    hILdEdxhitInEntaBin[iea] = new TH1F(Form("hILdEdxhitInEntaBin%d", iea), "bla-bla", 250, 0, 5);
    hILdEdxhitInEntaBin[iea]->SetTitle(Form("IL: dedxhit in EntA = (%0.03f to %0.03f)", ifeaLE, ifeaUE));
    hILdEdxhitInEntaBin[iea]->GetXaxis()->SetTitle("dedxhits in Inner Layer");
    hILdEdxhitInEntaBin[iea]->GetYaxis()->SetTitle("Entries");

    hOLdEdxhitInEntaBin[iea] = new TH1F(Form("hOLdEdxhitInEntaBin%d", iea), "bla-bla", 250, 0, 5);
    hOLdEdxhitInEntaBin[iea]->SetTitle(Form("OL: dedxhit in EntA = (%0.03f to %0.03f)", ifeaLE, ifeaUE));
    hOLdEdxhitInEntaBin[iea]->GetXaxis()->SetTitle("dedxhits in Outer Layer");
    hOLdEdxhitInEntaBin[iea]->GetYaxis()->SetTitle("Entries");
  }

  // Enta stats
  TH1F* hILEntaG = new TH1F("hILEntaG", "EntA: Inner Layer", fnEntaBinG, feaLE, feaUE);
  hILEntaG->GetXaxis()->SetTitle("Entrance angle (#theta)");
  hILEntaG->GetYaxis()->SetTitle("Entries");

  TH1F* hOLEntaG = new TH1F("hOLEntaG", "EntA: Outer Layer", fnEntaBinG, feaLE, feaUE);
  hOLEntaG->GetXaxis()->SetTitle("Entrance angle (#theta)");
  hOLEntaG->GetYaxis()->SetTitle("Entries");

  //rebinned histogram
  Double_t* RmapEntaValue = &fEntaBinValues[0];

  TH1F* hILEntaL = new TH1F("hILEntaL", "EntA: Inner Layer (rebin)", fnEntaBinL, RmapEntaValue);
  hILEntaL->GetXaxis()->SetTitle("Entrance angle (#theta)");
  hILEntaL->GetYaxis()->SetTitle("Entries");

  TH1F* hOLEntaL = new TH1F("hOLEntaL", "EntA: Outer Layer (rebin)", fnEntaBinL, RmapEntaValue);
  hOLEntaL->GetYaxis()->SetTitle("Entries");
  hOLEntaL->GetXaxis()->SetTitle("Entrance angle (#theta)");

  TH1F* hILdEdx_all = new TH1F("hILdEdx_all", "", 250, 0, 5);
  TH1F* hOLdEdx_all = new TH1F("hOLdEdx_all", "", 250, 0, 5);


  Int_t ibinEA = 0;
  for (int i = 0; i < ttree->GetEntries(); ++i) {

    ttree->GetEvent(i);

    for (unsigned int j = 0; j < dedxhit->size(); ++j) {

      if (dedxhit->at(j) == 0) continue;

      Double_t ieaHit = enta->at(j);
      if (ieaHit < -TMath::Pi() / 2.0) ieaHit += TMath::Pi() / 2.0;
      else if (ieaHit > TMath::Pi() / 2.0) ieaHit -= TMath::Pi() / 2.0;
      if (abs(ieaHit) > TMath::Pi() / 2.0) continue;

      //Bin corresponds to enta
      ibinEA = (ieaHit - feaLE) / feaBS ; //from 0
      if (ibinEA >= fnEntaBinG) continue; //bin stats from 0

      if (IsLocalBin) ibinEA = fEntaBinNums.at(ibinEA);


      if (layer->at(j) < 8) {
        hILEntaG->Fill(ieaHit);
        if (IsLocalBin)hILEntaL->Fill(ieaHit);
        hILdEdx_all->Fill(dedxhit->at(j));
        hILdEdxhitInEntaBin[ibinEA]->Fill(dedxhit->at(j));
      } else {
        hOLEntaG->Fill(ieaHit);
        if (IsLocalBin)hOLEntaL->Fill(ieaHit);
        hOLdEdx_all->Fill(dedxhit->at(j));
        hOLdEdxhitInEntaBin[ibinEA]->Fill(dedxhit->at(j));
      }
    }
  }

  if (IsMakePlots) {
    TCanvas* ctmpde = new TCanvas("hEntaDist", "Enta distributions", 400, 400);
    if (IsLocalBin) {
      ctmpde->SetCanvasSize(800, 400);
      ctmpde->Divide(2, 1);
      ctmpde->cd(1); hOLEntaG->SetMarkerColor(kBlue); hOLEntaG->Draw(""); hILEntaG->Draw("same");
      ctmpde->cd(2); hOLEntaL->SetMarkerColor(kBlue); hOLEntaL->Draw(""); hILEntaL->Draw("same");
    } else {
      hOLEntaG->Draw(""); hILEntaG->Draw("same");
    }
    ctmpde->SaveAs("hEntaDistributions.pdf");

    TCanvas* ctem = new TCanvas("Layerhisto", "Inner and Outer Histo", 600, 600);
    hOLdEdx_all->Draw("histo");
    hILdEdx_all->SetMarkerColor(kRed);
    hILdEdx_all->Draw("same histo");
    ctem->SaveAs("Layerhistodedxhit_OneDCorr.pdf");
  }

  double InsumPer5 = 0.0, InsumPer75 = 0.0;
  double OutsumPer5 = 0.0, OutsumPer75 = 0.0;
  double InLayInt = hILdEdx_all->Integral();
  double OutLayInt = hOLdEdx_all->Integral();

  Int_t lBinInLayer = 1,  hBinInLayer = 1;
  Int_t lBinOutLayer = 1,  hBinOutLayer = 1;

  for (int ibin = 1; ibin <= hILdEdx_all->GetNbinsX(); ibin++) {

    if (InsumPer5  <= 0.05 * InLayInt) {
      InsumPer5 += hILdEdx_all->GetBinContent(ibin);
      lBinInLayer = ibin;
    }

    if (InsumPer75  <= 0.75 * InLayInt) {
      InsumPer75 += hILdEdx_all->GetBinContent(ibin);
      hBinInLayer = ibin;
    }

    if (OutsumPer5 <= 0.05 * OutLayInt) {
      OutsumPer5 += hOLdEdx_all->GetBinContent(ibin);
      lBinOutLayer = ibin;
    }

    if (OutsumPer75 <= 0.75 * OutLayInt) {
      OutsumPer75 += hOLdEdx_all->GetBinContent(ibin);
      hBinOutLayer = ibin;
    }
  }

  //short version = 0;
  TCanvas* ctmp = new TCanvas("tmp", "tmp", 1200, 1200);
  ctmp->Divide(4, 4);
  std::stringstream psname; psname << "dedx_1dcell.pdf[";
  TLine* tl = new TLine();
  tl->SetLineColor(kRed);

  if (IsMakePlots) {
    ctmp->Print(psname.str().c_str());
    psname.str(""); psname << "dedx_1dcell.pdf";
  }

  double truncMean = 1.0, binweights = 1.0;
  int sumofbc = 1;

  TH1F* htemp = 0x0;
  std::vector<std::vector<double>> onedcors; // prev->std::vector<std::vector<double>> ones;
  std::vector<double> onedcorIorOL, onedcorIorOLtemp;

  TH1F* hILEntaConst = new TH1F("hILEntaConst", "EntA: Outer Layer", fnEntaBinG, feaLE, feaUE);
  hILEntaConst->GetXaxis()->SetTitle("Entrance angle (#theta)");
  hILEntaConst->GetYaxis()->SetTitle("Constant");

  TH1F* hOLEntaConst = new TH1F("hOLEntaConst", "EntA: Outer Layer", fnEntaBinG, feaLE, feaUE);
  hOLEntaConst->GetXaxis()->SetTitle("Entrance angle (#theta)");
  hOLEntaConst->GetYaxis()->SetTitle("Constant");

  int startfrom = 1, endat = 1;
  for (int iIOLayer = 0; iIOLayer <= 1; iIOLayer++) {

    if (iIOLayer == 0) {
      startfrom = lBinInLayer; endat = hBinInLayer;
    } else if (iIOLayer == 1)  {
      startfrom = lBinOutLayer; endat = hBinOutLayer;
    } else continue;

    //std::cout << "Layer I/O # = " << iIOLayer << std::endl;
    for (int iea = 1; iea <= fnEntaBinL; iea++) {

      Int_t ieaprime = 1; //rotation symmtery for 1<->3 and 4<->2
      if (iea <= int(0.25 * fnEntaBinL))ieaprime = iea + (0.50 * fnEntaBinL);
      else if (iea > int(0.75 * fnEntaBinL))ieaprime = iea - (0.50 * fnEntaBinL);
      else ieaprime = iea;

      if (iIOLayer == 0)htemp = (TH1F*)hILdEdxhitInEntaBin[ieaprime - 1]->Clone(Form("hL%d_Ea%d", iIOLayer, iea));
      else if (iIOLayer == 1)htemp = (TH1F*)hOLdEdxhitInEntaBin[ieaprime - 1]->Clone(Form("hL%d_Ea%d", iIOLayer, iea));
      else continue;

      truncMean  = 1.0; binweights = 0.0; sumofbc = 0;
      if (htemp->Integral() < 100) truncMean  = 1.0; //low stats
      else {
        for (int ibin = startfrom; ibin <= endat; ibin++) {
          //std::cout << " dedxhit bin = " << ibin << ", Entries =" << htemp->GetBinContent(ibin) << std::endl;
          if (htemp->GetBinContent(ibin) > 0) {
            binweights += (htemp->GetBinContent(ibin) * htemp->GetBinCenter(ibin));
            sumofbc += htemp->GetBinContent(ibin);
          }
        }
        if (sumofbc > 0)truncMean  = (double)(binweights / sumofbc);
        else truncMean = 1.0;
      }

      if (truncMean <= 0)truncMean = 1.0; //protection only
      onedcorIorOLtemp.push_back(truncMean);

      if (IsMakePlots) {
        ctmp->cd(((iea - 1) % 16) + 1);
        htemp->SetFillColor(kYellow);
        htemp->DrawClone("hist"); //clone is nessesory for pointer survival
        tl->SetX1(truncMean); tl->SetX2(truncMean);
        tl->SetY1(0); tl->SetY2(htemp->GetMaximum());
        tl->DrawClone("same");
        if (iea % 16 == 0)ctmp->Print(psname.str().c_str());
      }

      htemp->Reset();
    }

    ibinEA = 0;
    for (int iea = 0; iea < fnEntaBinG; iea++) {
      ibinEA = iea;
      if (IsLocalBin)ibinEA = fEntaBinNums.at(iea);
      onedcorIorOL.push_back(onedcorIorOLtemp.at(ibinEA));
      if (iIOLayer == 0)hILEntaConst->SetBinContent(iea + 1, onedcorIorOLtemp.at(ibinEA));
      else if (iIOLayer == 1)hOLEntaConst->SetBinContent(iea + 1, onedcorIorOLtemp.at(ibinEA));
    }

    onedcors.push_back(onedcorIorOL);
    onedcorIorOL.clear();
    onedcorIorOLtemp.clear();
  }


  if (IsMakePlots) {
    psname.str(""); psname << "dedx_1dcell.pdf]";
    ctmp->Print(psname.str().c_str());

    // //Drawing final constants
    TCanvas* cconst = new TCanvas("FinalConstantHistoMap", "Inner and Outer Histo", 800, 400);
    cconst->Divide(2, 1);
    cconst->cd(1); hILEntaConst->Draw();
    cconst->cd(2); hOLEntaConst->Draw();
    cconst->SaveAs("FinalOneDConstantMap.pdf");
  }

  B2INFO("dE/dx Calibration done for 1D cleanup correction");
  CDCDedx1DCell* gain = new CDCDedx1DCell(0, onedcors);
  saveCalibration(gain, "CDCDedx1DCell");


  delete htemp;
  delete ctmp;
  delete tl;
  return c_OK;

}

//----------------------------------------------------------------------------
void CDCDedx1DCellAlgorithm::GlobalToLocalEntaBinMap(Bool_t seeMap)
{

  if (fnEntaBinG % 16 != 0) {
    std::cout << "-- Fix bins: enta should multiple of 16.. Exiting.." << std::endl;
    return;
  }

  //Configuration in 1/4 bins only and rest symmetry
  const Int_t fnEntaBinG1by4 = fnEntaBinG / 4;

  std::vector<int> EntaBinNumsFH; //FirstHalf (pi/4 to pi/2)
  Int_t ibinL = 0 , ibinH = 0 , iBinLocal = 0, Factor = 1, jbin = 1;
  Int_t a = 0, b = 0, c = 0;
  Int_t L1 = 0, L2 = 0, L3 = 0;

  for (Int_t ibin = 1; ibin <= fnEntaBinG1by4; ibin++) {

    if (ibin <= fnEntaBinG1by4 / 4) {
      Factor = 1;
      ibinL = 0;
      ibinH = fnEntaBinG1by4 / 4;
      L1 = int((ibinH - ibinL) / Factor);
      jbin = L1 - int((ibinH - ibin) / Factor);
      a = jbin;
      iBinLocal = a;
    } else if (ibin > fnEntaBinG1by4 / 4 && ibin <= fnEntaBinG1by4 / 2) {
      Factor = 2;
      ibinL = fnEntaBinG1by4 / 4;
      ibinH = fnEntaBinG1by4 / 2;
      L2 = int((ibinH - ibinL) / Factor);
      jbin = L2 - int((ibinH - ibin) / Factor);
      b = a + jbin;
      iBinLocal = b;
    } else if (ibin > fnEntaBinG1by4 / 2 && ibin <= fnEntaBinG1by4) {
      Factor = 4;
      ibinL = fnEntaBinG1by4 / 2;
      ibinH = fnEntaBinG1by4 / 1;
      L3 = int((ibinH - ibinL) / Factor);
      jbin = L3 - int((ibinH - ibin) / Factor);
      c = b + jbin;
      iBinLocal = c;
    }
    EntaBinNumsFH.push_back(iBinLocal);
  }

  Int_t L = L1 + L2 + L3;

  std::vector<int> temp2 = EntaBinNumsFH;
  std::reverse(temp2.begin(), temp2.end());

  std::vector<int> EntaBinNumsSH; //second half (0 to pi/2)
  for (unsigned int it = 0; it < temp2.size(); ++it)EntaBinNumsSH.push_back(2 * L - temp2.at(it) + 1);

  std::vector<int> EntaBinNums1 = EntaBinNumsFH;
  EntaBinNums1.insert(EntaBinNums1.end(), EntaBinNumsSH.begin(), EntaBinNumsSH.end());
  for (unsigned int it = 0; it < EntaBinNums1.size(); ++it)EntaBinNums1.at(it) = EntaBinNums1.at(it) - 1;

  std::vector<int> EntaBinNums2;
  for (unsigned int it = 0; it < EntaBinNums1.size(); ++it)EntaBinNums2.push_back(2 * L + EntaBinNums1.at(it));

  //Final Vector of bin array
  fEntaBinNums = EntaBinNums1;
  fEntaBinNums.insert(fEntaBinNums.end(), EntaBinNums2.begin(), EntaBinNums2.end());
  if (seeMap)for (unsigned int it = 0; it < fEntaBinNums.size();
                    ++it)std::cout << "1DCell-EntA: GlobalBin = " << it << ", LocalBin = " << fEntaBinNums.at(it) << std::endl;


  TH1F* tempEnta = new TH1F("tempEnta", "tempEnta", fnEntaBinG, feaLE, feaUE);
  fEntaBinValues.push_back(tempEnta->GetBinLowEdge(1)); //first and last manual
  for (unsigned int i = 0; i < fEntaBinNums.size() - 1; ++i) {
    if (fEntaBinNums.at(i) < fEntaBinNums.at(i + 1)) {
      double binval = tempEnta->GetBinLowEdge(i + 1) + tempEnta->GetBinWidth(i + 1);
      if (TMath::Abs(binval) < 10e-5)binval = 0; //avoid infinite deep
      fEntaBinValues.push_back(binval);
    } else continue;
  }
  fEntaBinValues.push_back(tempEnta->GetBinLowEdge(fnEntaBinG) + tempEnta->GetBinWidth(fnEntaBinG));
  delete tempEnta;
}
