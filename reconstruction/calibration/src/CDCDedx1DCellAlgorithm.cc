/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <algorithm>
#include <iostream>

#include <TCanvas.h>
#include <TH1D.h>
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
  fSetPrefix("_it0"),
  IsLocalBin(true),
  IsMakePlots(false),
  IsRS(true)
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

  std::vector<int> globalbins;
  for (int ibin = 0; ibin < fnEntaBinG; ibin++)globalbins.push_back(ibin);

  if (!IsLocalBin) {
    fEntaBinNums = globalbins;
    fnEntaBinL = fnEntaBinG;
  } else {
    GetVariableBin(fnEntaBinG, fEntaBinNums);
    fnEntaBinL =  fEntaBinNums.at(fEntaBinNums.size() - 1) + 1;
  }

  //enta dedx distributions for inner and outer layer
  std::vector<TH1D*> hILdEdxhitInEntaBin(fnEntaBinL, 0);
  std::vector<TH1D*> hOLdEdxhitInEntaBin(fnEntaBinL, 0);

  Double_t ifeaLE = 0, ifeaUE = 0;

  for (int iea = 0; iea < fnEntaBinL; iea++) {

    if (IsLocalBin) {
      ifeaLE = fEntaBinValues.at(iea);
      ifeaUE = fEntaBinValues.at(iea + 1);
    } else {
      ifeaLE = iea * feaBS - feaUE; //- because of -ive range shifting
      ifeaUE = ifeaLE + feaBS;
    }

    hILdEdxhitInEntaBin[iea] = new TH1D(Form("hILdEdxhitInEntaBin%d", iea), "bla-bla", 250, 0., 5.);
    hILdEdxhitInEntaBin[iea]->SetTitle(Form("IL: dedxhit in EntA = (%0.03f to %0.03f)", ifeaLE, ifeaUE));
    hILdEdxhitInEntaBin[iea]->GetXaxis()->SetTitle("dedxhits in Inner Layer");
    hILdEdxhitInEntaBin[iea]->GetYaxis()->SetTitle("Entries");

    hOLdEdxhitInEntaBin[iea] = new TH1D(Form("hOLdEdxhitInEntaBin%d", iea), "bla-bla", 250, 0., 5.);
    hOLdEdxhitInEntaBin[iea]->SetTitle(Form("OL: dedxhit in EntA = (%0.03f to %0.03f)", ifeaLE, ifeaUE));
    hOLdEdxhitInEntaBin[iea]->GetXaxis()->SetTitle("dedxhits in Outer Layer");
    hOLdEdxhitInEntaBin[iea]->GetYaxis()->SetTitle("Entries");
  }

  // Enta stats
  TH1D* hILEntaG = new TH1D("hILEntaG", "EntA: Inner Layer", fnEntaBinG, feaLE, feaUE);
  hILEntaG->GetXaxis()->SetTitle("Entrance angle (#theta)");
  hILEntaG->GetYaxis()->SetTitle("Entries");

  TH1D* hOLEntaG = new TH1D("hOLEntaG", "EntA: Outer Layer", fnEntaBinG, feaLE, feaUE);
  hOLEntaG->GetXaxis()->SetTitle("Entrance angle (#theta)");
  hOLEntaG->GetYaxis()->SetTitle("Entries");

  //rebinned histogram
  Double_t* RmapEntaValue = &fEntaBinValues[0];

  TH1D* hILEntaL = new TH1D("hILEntaL", "EntA: Inner Layer (assym bin)", fnEntaBinL, RmapEntaValue);
  hILEntaL->GetXaxis()->SetTitle("Entrance angle (#theta)");
  hILEntaL->GetYaxis()->SetTitle("Entries");

  TH1D* hOLEntaL = new TH1D("hOLEntaL", "EntA: Outer Layer (assym bin)", fnEntaBinL, RmapEntaValue);
  hOLEntaL->GetYaxis()->SetTitle("Entries");
  hOLEntaL->GetXaxis()->SetTitle("Entrance angle (#theta)");

  TH1D* hILdEdx_all = new TH1D("hILdEdx_all", "", 250, 0., 5.);
  TH1D* hOLdEdx_all = new TH1D("hOLdEdx_all", "", 250, 0., 5.);

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
      ctmpde->cd(1);  gPad->SetLogy(); hOLEntaG->SetMarkerColor(kBlue); hOLEntaG->Draw(""); hILEntaG->Draw("same");
      ctmpde->cd(2);  gPad->SetLogy(); hOLEntaL->SetMarkerColor(kBlue); hOLEntaL->Draw(""); hILEntaL->Draw("same");
    } else {
      hOLEntaG->Draw(""); hILEntaG->Draw("same");
    }
    ctmpde->SaveAs(Form("hEntaDistributions_%s.pdf", fSetPrefix.data()));

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

  if (IsMakePlots) {

    TCanvas* ctem = new TCanvas("Layerhisto", "Inner and Outer Histo", 1200, 600);
    ctem->Divide(2, 1);
    ctem->cd(1);
    hILdEdx_all->SetMarkerColor(kBlue);
    hILdEdx_all->Draw("histo");

    TLine* tlInF = new TLine();
    tlInF->SetLineColor(kBlue);
    tlInF->SetX1(InsumPer5); tlInF->SetX2(InsumPer5);
    tlInF->SetY1(0); tlInF->SetY2(hILdEdx_all->GetMaximum());
    tlInF->DrawClone("same");

    TLine* tlInL = new TLine();
    tlInL->SetLineColor(kBlue);
    tlInL->SetX1(InsumPer75); tlInL->SetX2(InsumPer75);
    tlInL->SetY1(0); tlInL->SetY2(hILdEdx_all->GetMaximum());
    tlInL->DrawClone("same");

    ctem->cd(2);
    hOLdEdx_all->Draw("histo");
    hOLdEdx_all->SetMarkerColor(kRed);

    TLine* tlOutF = new TLine();
    tlOutF->SetLineColor(kBlue);
    tlOutF->SetX1(OutsumPer5); tlOutF->SetX2(OutsumPer5);
    tlOutF->SetY1(0); tlOutF->SetY2(hOLdEdx_all->GetMaximum());
    tlOutF->DrawClone("same");

    TLine* tlOutL = new TLine();
    tlOutL->SetLineColor(kBlue);
    tlOutL->SetX1(OutsumPer75); tlOutL->SetX2(OutsumPer75);
    tlOutL->SetY1(0); tlOutL->SetY2(hOLdEdx_all->GetMaximum());
    tlOutL->DrawClone("same");
    ctem->SaveAs(Form("Layerhistodedxhit_OneDCorr_%s.pdf", fSetPrefix.data()));
  }

  //short version = 0;
  TCanvas* ctmp = new TCanvas("tmp", "tmp", 1200, 1200);
  ctmp->Divide(4, 4);
  std::stringstream psname; psname << Form("dedx_1dcell_%s.pdf[", fSetPrefix.data());
  TLine* tl = new TLine();
  tl->SetLineColor(kRed);

  TBox* tb = new TBox();

  if (IsMakePlots) {
    ctmp->Print(psname.str().c_str());
    psname.str(""); psname << Form("dedx_1dcell_%s.pdf", fSetPrefix.data());
  }

  TH1D* htemp = 0x0;
  std::vector<std::vector<double>> onedcors; // prev->std::vector<std::vector<double>> ones;
  std::vector<double> onedcorIorOL, onedcorIorOLtemp;

  TH1D* hILEntaConst = new TH1D("hILEntaConst", "EntA: Outer Layer", fnEntaBinG, feaLE, feaUE);
  hILEntaConst->GetXaxis()->SetTitle("Entrance angle (#theta)");
  hILEntaConst->GetYaxis()->SetTitle("Constant");

  TH1D* hOLEntaConst = new TH1D("hOLEntaConst", "EntA: Outer Layer", fnEntaBinG, feaLE, feaUE);
  hOLEntaConst->GetXaxis()->SetTitle("Entrance angle (#theta)");
  hOLEntaConst->GetYaxis()->SetTitle("Constant");

  for (int iIOLayer = 0; iIOLayer <= 1; iIOLayer++) {

    int startfrom = 1, endat = 1;

    if (iIOLayer == 0) {
      startfrom = lBinInLayer; endat = hBinInLayer;
    } else if (iIOLayer == 1)  {
      startfrom = lBinOutLayer; endat = hBinOutLayer;
    } else continue;

    for (int iea = 1; iea <= fnEntaBinL; iea++) {

      Int_t ieaprime = iea; //rotation symmtery for 1<->3 and 4<->2

      if (IsRS)ieaprime = GetRotationSymmericBin(fnEntaBinL, iea);

      if (iIOLayer == 0)htemp = (TH1D*)hILdEdxhitInEntaBin[ieaprime - 1]->Clone(Form("hL%d_Ea%d", iIOLayer, iea));
      else if (iIOLayer == 1)htemp = (TH1D*)hOLdEdxhitInEntaBin[ieaprime - 1]->Clone(Form("hL%d_Ea%d", iIOLayer, iea));
      else continue;

      double truncMean = 1.0;
      if (htemp->Integral() < 250) truncMean  = 1.0; //low stats
      else {
        double binweights = 0.0;
        int sumofbc = 0;
        for (int ibin = startfrom; ibin <= endat; ibin++) {

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
        htemp->SetFillColorAlpha(kGreen, 0.30);
        htemp->SetTitle(Form("%s, #mean = %0.2f", htemp->GetTitle(), truncMean));
        if (truncMean >= 1.02 || truncMean <= 0.98)htemp->SetFillColor(kYellow);
        if (truncMean >= 1.05 || truncMean <= 0.95)htemp->SetFillColor(kOrange);
        if (truncMean >= 1.10 || truncMean <= 0.90)htemp->SetFillColor(kRed);
        htemp->DrawClone("hist"); //clone is nessesory for pointer survival
        tl->SetLineColor(kRed);
        tl->SetX1(truncMean); tl->SetX2(truncMean);
        tl->SetY1(0); tl->SetY2(htemp->GetMaximum());
        tl->DrawClone("same");

        tb->SetLineColor(kPink);
        tb->SetLineStyle(6);
        tb->SetFillColorAlpha(kPink, 0.15);
        tb->SetX1(htemp->GetBinLowEdge(startfrom));
        tb->SetY1(0);
        tb->SetX2(htemp->GetBinLowEdge(endat));
        tb->SetY2(htemp->GetMaximum());
        tb->DrawClone("same");
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
    psname.str(""); psname << Form("dedx_1dcell_%s.pdf]", fSetPrefix.data());
    ctmp->Print(psname.str().c_str());

    // //Drawing final constants
    TCanvas* cconst = new TCanvas("FinalConstantHistoMap", "Inner and Outer Histo", 800, 400);
    cconst->Divide(2, 1);
    cconst->cd(1); hILEntaConst->Draw();
    cconst->cd(2); hOLEntaConst->Draw();
    cconst->SaveAs(Form("FinalOneDConstantMap_%s.pdf", fSetPrefix.data()));
  }

  B2INFO("dE/dx Calibration done for 1D cleanup correction");
  CDCDedx1DCell* gain = new CDCDedx1DCell(0, onedcors);
  saveCalibration(gain, "CDCDedx1DCell");

  delete htemp;
  delete ctmp;
  delete tl;
  delete tb;
  return c_OK;

}