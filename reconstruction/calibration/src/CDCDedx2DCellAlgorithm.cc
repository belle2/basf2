/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/calibration/CDCDedx2DCellAlgorithm.h>

#include <reconstruction/dbobjects/CDCDedx2DCell.h>

#include <TBox.h>
#include <TCanvas.h>
#include <TH2F.h>
#include <TLine.h>
#include <TMath.h>
#include <TStyle.h>

#include <algorithm>
#include <fstream>

using namespace Belle2;


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDedx2DCellAlgorithm::CDCDedx2DCellAlgorithm():
  CalibrationAlgorithm("CDCDedxElectronCollector"),
  fnEntaBinG(128),
  fnDocaBinG(64),
  fnEntaBinL(64),
  fnDocaBinL(28),
  feaLE(-TMath::Pi() / 2),
  feaUE(+TMath::Pi() / 2),
  fdocaLE(-1.50),
  fdocaUE(1.50),
  fSetPrefix("_it0"),
  IsLocalBin(true),
  IsMakePlots(false),
  IsRS(true)
{
  // Set module properties
  setDescription("A calibration algorithm for the CDC dE/dx two dimensional correction");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------

CalibrationAlgorithm::EResult CDCDedx2DCellAlgorithm::calibrate()
{

  //reading electron collector TREE
  auto ttree = getObjectPtr<TTree>("tree");
  if (ttree->GetEntries() < 100)return c_NotEnoughData;

  std::vector<double>* dedxhit = 0, *doca = 0, *enta = 0;
  std::vector<int>* layer = 0;

  ttree->SetBranchAddress("dedxhit", &dedxhit);
  ttree->SetBranchAddress("layer", &layer);
  ttree->SetBranchAddress("docaRS", &doca);
  ttree->SetBranchAddress("entaRS", &enta);

  // Setting up bins for doca and entra angle
  feaBS = (feaUE - feaLE) / fnEntaBinG;
  fdocaBS = (fdocaUE - fdocaLE) / fnDocaBinG;

  std::vector<int> globalbinsEnta, globalbinsDoca;
  for (int ibin = 0; ibin < fnEntaBinG; ibin++)globalbinsEnta.push_back(ibin);
  for (int ibin = 0; ibin < fnDocaBinG; ibin++)globalbinsDoca.push_back(ibin);

  if (!IsLocalBin) {
    fEntaBinNums = globalbinsEnta;
    fnEntaBinL = fnEntaBinG;
  } else {
    GetVariableBin(fnEntaBinG, fEntaBinNums);
    fnEntaBinL =  fEntaBinNums.at(fEntaBinNums.size() - 1) + 1;
  }

  fDocaBinNums = globalbinsDoca;
  fnDocaBinL = fnDocaBinG;

  std::vector<std::vector<TH1F*>> hILdEdxhitInEntaDocaBin(fnEntaBinL, std::vector<TH1F*>(fnDocaBinL, 0));
  std::vector<std::vector<TH1F*>> hOLdEdxhitInEntaDocaBin(fnEntaBinL, std::vector<TH1F*>(fnDocaBinL, 0));
  Double_t ifeaLE = 0, ifeaUE = 0, ifdocaLE = 0, ifdocaUE = 0;

  for (int iea = 0; iea < fnEntaBinL; iea++) {
    if (IsLocalBin) {
      ifeaLE = fEntaBinValues.at(iea);
      ifeaUE = fEntaBinValues.at(iea + 1);
    } else {
      ifeaLE = iea * feaBS - feaUE; //- because of -ive range shifting
      ifeaUE = ifeaLE + feaBS;
    }

    for (int idoca = 0; idoca < fnDocaBinL; idoca++) {

      ifdocaLE = idoca * fdocaBS - fdocaUE;
      ifdocaUE = ifdocaLE + fdocaBS;

      hILdEdxhitInEntaDocaBin[iea][idoca] = new TH1F(Form("hILdEdxhitInEntaBin%dDocaBin%d", iea, idoca), "bla-bla", 500, 0., 5.);
      hILdEdxhitInEntaDocaBin[iea][idoca]->SetTitle(Form("IL: EntA = (%0.03f to %0.03f) and Doca = (%0.03f to %0.03f)", ifeaLE,
                                                         ifeaUE, ifdocaLE, ifdocaUE));
      hILdEdxhitInEntaDocaBin[iea][idoca]->GetXaxis()->SetTitle("dedxhits in Inner Layer");
      hILdEdxhitInEntaDocaBin[iea][idoca]->GetYaxis()->SetTitle("Entries");

      hOLdEdxhitInEntaDocaBin[iea][idoca] = new TH1F(Form("hOLdEdxhitInEntaBin%dDocaBin%d", iea, idoca), "bla-bla", 500, 0., 5.);
      hOLdEdxhitInEntaDocaBin[iea][idoca]->SetTitle(Form("OL: EntA = (%0.03f to %0.03f) and Doca = (%0.03f to %0.03f)", ifeaLE,
                                                         ifeaUE, ifdocaLE, ifdocaUE));
      hOLdEdxhitInEntaDocaBin[iea][idoca]->GetXaxis()->SetTitle("dedxhits in Outer Layer");
      hOLdEdxhitInEntaDocaBin[iea][idoca]->GetYaxis()->SetTitle("Entries");

    }
  }

  //Doca vs Enta stats
  TH2D* hILDocaEntaG = new TH2D("hILDocaEntaG", "Doca vs EntA: Inner Layer", fnDocaBinG, fdocaLE, fdocaUE, fnEntaBinG, feaLE, feaUE);
  hILDocaEntaG->GetXaxis()->SetTitle("Doca");
  hILDocaEntaG->GetYaxis()->SetTitle("Entrance angle (#theta)");

  TH2D* hOLDocaEntaG = new TH2D("hOLDocaEntaG", "Doca vs EntA: Outer Layer", fnDocaBinG, fdocaLE, fdocaUE, fnEntaBinG, feaLE, feaUE);
  hOLDocaEntaG->GetXaxis()->SetTitle("Doca");
  hOLDocaEntaG->GetYaxis()->SetTitle("Entrance angle (#theta)");

  //when local enta angle is demanded
  Double_t* RmapEntaValue = &fEntaBinValues[0];
  TH2D* hILDocaEntaL = new TH2D("hILDocaEntaL", "Doca vs EntA: Inner Layer (rebin)", fnDocaBinL, fdocaLE, fdocaUE, fnEntaBinL,
                                RmapEntaValue);
  hILDocaEntaL->GetXaxis()->SetTitle("Doca");
  hILDocaEntaL->GetYaxis()->SetTitle("Entrance angle (#theta)");

  TH2D* hOLDocaEntaL = new TH2D("hOLDocaEntaL", "Doca vs EntA: Outer Layer (rebin)", fnDocaBinL, fdocaLE, fdocaUE, fnEntaBinL,
                                RmapEntaValue);
  hOLDocaEntaL->GetXaxis()->SetTitle("Doca");
  hOLDocaEntaL->GetYaxis()->SetTitle("Entrance angle (#theta)");

  TH1D* hILdEdx_all = new TH1D("hILdEdx_all", "", 500, 0., 5.);
  TH1D* hOLdEdx_all = new TH1D("hOLdEdx_all", "", 500, 0., 5.);


  Int_t ibinEA = 0, ibinDOCA = 0;
  for (int i = 0; i < ttree->GetEntries(); ++i) {

    ttree->GetEvent(i);

    for (unsigned int j = 0; j < dedxhit->size(); ++j) {

      if (dedxhit->at(j) == 0) continue;

      Double_t ieaHit = enta->at(j);
      if (ieaHit < -TMath::Pi() / 2.0) ieaHit += TMath::Pi() / 2.0;
      else if (ieaHit > TMath::Pi() / 2.0) ieaHit -= TMath::Pi() / 2.0;
      if (abs(ieaHit) > TMath::Pi() / 2.0) continue;

      Double_t idocaHit = doca->at(j);
      if (abs(idocaHit) > 1.50)  continue;

      //Bin corresponds to enta and doca value
      ibinEA = 0;
      ibinEA = (ieaHit - feaLE) / feaBS ; //from 0

      ibinDOCA = 0;
      ibinDOCA = (idocaHit - fdocaLE) / fdocaBS;

      if (ibinEA >= fnEntaBinG || ibinDOCA >= fnDocaBinG) continue; //bin stats from 0

      if (IsLocalBin) ibinEA = fEntaBinNums.at(ibinEA);

      if (layer->at(j) < 8) {
        hILDocaEntaG->Fill(idocaHit, ieaHit);
        if (IsLocalBin)hILDocaEntaL->Fill(idocaHit, ieaHit);
        hILdEdx_all->Fill(dedxhit->at(j));
        hILdEdxhitInEntaDocaBin[ibinEA][ibinDOCA]->Fill(dedxhit->at(j));
      } else {
        hOLDocaEntaG->Fill(idocaHit, ieaHit);
        if (IsLocalBin)hOLDocaEntaL->Fill(idocaHit, ieaHit);
        hOLdEdx_all->Fill(dedxhit->at(j));
        hOLdEdxhitInEntaDocaBin[ibinEA][ibinDOCA]->Fill(dedxhit->at(j));
      }
    }
  }


  if (IsMakePlots) {
    TCanvas* ctmpde = new TCanvas("DocavsEnta", "Doca vs Enta distributions", 800, 400);
    if (IsLocalBin) {
      ctmpde->SetCanvasSize(800, 800);
      ctmpde->Divide(2, 2);
      ctmpde->cd(1); hILDocaEntaG->Draw("colz");
      ctmpde->cd(2); hOLDocaEntaG->Draw("colz");
      ctmpde->cd(3); hILDocaEntaL->Draw("colz");
      ctmpde->cd(4); hOLDocaEntaL->Draw("colz");
    } else {
      ctmpde->Divide(2, 1);
      ctmpde->cd(1); hILDocaEntaG->Draw("colz");
      ctmpde->cd(2); hOLDocaEntaG->Draw("colz");
    }
    ctmpde->SaveAs(Form("DocavsEnta_%s.pdf", fSetPrefix.data()));
    ctmpde->SaveAs(Form("DocavsEnta_%s.root", fSetPrefix.data()));

    TCanvas* ctem = new TCanvas("Layerhisto", "Inner and Outer Histo", 600, 600);
    hOLdEdx_all->Draw("histo");
    hILdEdx_all->SetMarkerColor(kRed);
    hILdEdx_all->Draw("same histo");
    ctem->SaveAs(Form("Layerhistodedxhit_TwoDCorr_%s.pdf", fSetPrefix.data()));
  }


  //Calculationg 5-75% global truncation mean
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

  short version = 1;
  TCanvas* ctmp = new TCanvas("tmp", "tmp", 1200, 1200);
  ctmp->Divide(4, 4);
  std::stringstream psname; psname << Form("dedx_2dcell_%s.pdf[", fSetPrefix.data());
  TLine* tl = new TLine();
  tl->SetLineColor(kRed);

  TBox* tb = new TBox();

  if (IsMakePlots) {
    ctmp->Print(psname.str().c_str());
    psname.str(""); psname << Form("dedx_2dcell_%s.pdf", fSetPrefix.data());
  }

  gStyle->SetOptStat("nemriou");
  TH1D* htemp = NULL;
  std::vector<TH2F> twodcors;
  TH2F tempTwoD = TH2F("tempTwoD", "dE/dx in bins of DOCA/Enta;DOCA;Entrance Angle", fnDocaBinL, fdocaLE, fdocaUE, fnEntaBinL,
                       RmapEntaValue);
  TH2F twodcor = TH2F("twodcorrection", "dE/dx in bins of DOCA/Enta;DOCA;Entrance Angle", fnDocaBinG, fdocaLE, fdocaUE, fnEntaBinG,
                      feaLE, feaUE);

  std::ofstream file2DILout;

  for (int iIOLayer = 0; iIOLayer <= 1; iIOLayer++) {

    file2DILout.open(Form("f2D_Constants_Local_Layer%d_%s.txt", iIOLayer, fSetPrefix.data()));

    int startfrom = 1, endat = 1;
    if (iIOLayer == 0) {
      startfrom = lBinInLayer; endat = hBinInLayer;
    } else  {
      startfrom = lBinOutLayer; endat = hBinOutLayer;
    }

    //std::cout << "Layer I/O # = " << iIOLayer << std::endl;
    for (int iea = 1; iea <= fnEntaBinL; iea++) {

      Int_t ieaprime = iea; //rotation symmtery for 1<->3 and 4<->2
      if (IsRS)ieaprime = GetRotationSymmericBin(fnEntaBinL, iea);

      for (int idoca = 1; idoca <= fnDocaBinL; idoca++) {

        if (iIOLayer == 0)
          htemp = (TH1D*)hILdEdxhitInEntaDocaBin[ieaprime - 1][idoca - 1]->Clone(Form("hL%d_Ea%d_Doca%d", iIOLayer, iea, idoca));
        else if (iIOLayer == 1)
          htemp = (TH1D*)hOLdEdxhitInEntaDocaBin[ieaprime - 1][idoca - 1]->Clone(Form("hL%d_Ea%d_Doca%d", iIOLayer, iea, idoca));
        else continue;

        double truncMean  = 1.0;
        if (htemp->GetEntries() < 400) truncMean  = 1.0; //low stats
        else {
          double binweights = 0.0;
          int sumofbc = 0;
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
        tempTwoD.SetBinContent(idoca, iea, truncMean); //binning starts at 1

        if (IsMakePlots) {
          ctmp->cd(((idoca - 1) % 16) + 1);
          htemp->SetTitle(Form("%s, mean = %0.4f", htemp->GetTitle(), truncMean));
          htemp->SetFillColorAlpha(kGreen, 0.30);
          if (truncMean >= 1.02 || truncMean <= 0.98)htemp->SetFillColor(kYellow);
          if (truncMean >= 1.05 || truncMean <= 0.95)htemp->SetFillColor(kOrange);
          if (truncMean >= 1.10 || truncMean <= 0.90)htemp->SetFillColor(kRed);
          htemp->DrawClone(); //clone is nessesory for pointer survival
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
          if (idoca % 16 == 0)ctmp->Print(psname.str().c_str());
        }

        file2DILout << iea << ",   \t" << idoca << ",    \t" << truncMean << "\n";
        htemp->Reset();
      }
    }

    //conversion from local to global
    ibinEA = 0, ibinDOCA = 0;
    for (int iea = 0; iea < fnEntaBinG; iea++) {
      ibinEA = iea;
      if (IsLocalBin)ibinEA = fEntaBinNums.at(iea);
      for (int idoca = 0; idoca < fnDocaBinG; idoca++) {
        ibinDOCA = idoca;
        twodcor.SetBinContent(idoca + 1, iea + 1, tempTwoD.GetBinContent(ibinDOCA + 1, ibinEA + 1));
      }
    }

    if (iIOLayer == 0)twodcor.SetTitle("InnerLayer: dE/dx in bins of DOCA/Enta");
    else if (iIOLayer == 1)twodcor.SetTitle("OuterLayer: dE/dx in bins of DOCA/Enta");
    twodcors.push_back(twodcor);
    twodcor.Reset();
    tempTwoD.Reset();
    file2DILout.close();
  }

  if (IsMakePlots) {
    psname.str(""); psname << Form("dedx_2dcell_%s.pdf]", fSetPrefix.data());
    ctmp->Print(psname.str().c_str());

    // //Drawing final constants
    TCanvas* cconst = new TCanvas("FinalConstantHistoMap", "Inner and Outer Histo", 800, 400);
    cconst->Divide(2, 1);
    cconst->cd(1); twodcors.at(0).Draw("colz");
    cconst->cd(2); twodcors.at(1).Draw("colz");
    cconst->SaveAs(Form("Final2DConstantMap_wGlobalBin_%s.pdf", fSetPrefix.data()));
    cconst->SaveAs(Form("Final2DConstantMap_wGlobalBin_%s.root", fSetPrefix.data()));
  }


  B2INFO("dE/dx calibration done for 2D correction");
  CDCDedx2DCell* gain = new CDCDedx2DCell(version, twodcors);
  saveCalibration(gain, "CDCDedx2DCell");

  delete htemp;
  delete ctmp;
  delete tl;
  delete tb;
  return c_OK;

}
