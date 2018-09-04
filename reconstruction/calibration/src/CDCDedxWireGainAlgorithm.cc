/**************************************************************************
1;4205;0c * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <algorithm>
#include <reconstruction/calibration/CDCDedxWireGainAlgorithm.h>
using namespace Belle2;


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
CDCDedxWireGainAlgorithm::CDCDedxWireGainAlgorithm() :
  CalibrationAlgorithm("CDCDedxElectronCollector"),
  m_badWireFPath(""),
  m_badWireFName(""),
  isRmBadwires(false),
  isMakePlots(true)
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

  TH1F* hInnerLayer = new TH1F("hInnerLayer", "", 250, 0, 5);
  TH1F* hOuterLayer = new TH1F("hOuterLayer", "", 250, 0, 5);

  for (unsigned int jwire = 0; jwire < 14336; ++jwire) {
    for (unsigned int jdedxhit = 0; jdedxhit < wirededx[jwire].size(); ++jdedxhit) {
      double ihit = wirededx[jwire][jdedxhit];
      if (jwire > 0 && jwire < 160 * 8)hInnerLayer->Fill(ihit);
      else hOuterLayer->Fill(ihit);
    }
  }

  TCanvas* ctmp = new TCanvas("tmp", "tmp", 900, 900);
  ctmp->Divide(4, 4);
  ctmp->SetBatch(kTRUE);
  std::stringstream psname; psname << "dedx_wiregains.pdf[";

  if (isMakePlots) {
    TCanvas* ctem = new TCanvas("Layerhisto", "Inner and Outer Layer dedxhit", 600, 600);
    hOuterLayer->Draw("histo");
    hInnerLayer->SetMarkerColor(kRed);
    hInnerLayer->Draw("same histo");
    ctem->SaveAs("hIOLayerLeveldEdxHits.pdf");

    //Print the histograms for quality control
    ctmp->Print(psname.str().c_str());
    psname.str(""); psname << "dedx_wiregains.pdf";

  }

  double InsumPer5 = 0.0, InsumPer75 = 0.0;
  double OutsumPer5 = 0.0, OutsumPer75 = 0.0;
  Int_t lBinInLayer = 1, hBinInLayer = 1;
  Int_t lBinOutLayer = 1, hBinOutLayer = 1;

  double InLayInt = hInnerLayer->Integral();
  double OutLayInt = hOuterLayer->Integral();

  for (int ibin = 1; ibin <= hInnerLayer->GetNbinsX(); ibin++) {

    if (InsumPer5  <= 0.05 * InLayInt) {
      InsumPer5 += hInnerLayer->GetBinContent(ibin);
      lBinInLayer = ibin;
    }

    if (InsumPer75  <= 0.75 * InLayInt) {
      InsumPer75 += hInnerLayer->GetBinContent(ibin);
      hBinInLayer = ibin;
    }

    if (OutsumPer5 <= 0.05 * OutLayInt) {
      OutsumPer5 += hOuterLayer->GetBinContent(ibin);
      lBinOutLayer = ibin;
    }

    if (OutsumPer75 <= 0.75 * OutLayInt) {
      OutsumPer75 += hOuterLayer->GetBinContent(ibin);
      hBinOutLayer = ibin;
    }
  }

  double iWireTruncMean[14336]; //initialisation of wire gains
  for (Int_t jwire = 0; jwire < 14336; jwire++) iWireTruncMean[jwire] = 1.0;

  //------------------------> Calculations part
  TH1F* htempPerWire = new TH1F("htempPerWire", "", 250, 0, 5);
  double truncMean = 1.0, binweights = 1.0;
  int sumofbc = 1;

  for (unsigned int jwire = 0; jwire < 14336; ++jwire) {

    for (unsigned int jdedxhit = 0; jdedxhit < wirededx[jwire].size(); ++jdedxhit) {
      double ihit = wirededx[jwire][jdedxhit];
      htempPerWire->Fill(ihit);
    }

    truncMean  = 1.0;
    binweights = 0.0;
    sumofbc = 0;

    if (htempPerWire->Integral() < 10) {
      truncMean  = 1.0;
      //iWireTruncMean[jwire] =  truncMean;
    } else {
      int startfrom = 1, endat = 1;
      if (jwire < 160 * 8) {
        startfrom = lBinInLayer;
        endat = hBinInLayer;
      } else if (jwire >= 160 * 8) {
        startfrom = lBinOutLayer;
        endat = hBinOutLayer;
      }

      for (int ibin = startfrom; ibin <= endat; ibin++) {
        if (htempPerWire->GetBinContent(ibin) >= 0) {
          binweights += (htempPerWire->GetBinContent(ibin) * htempPerWire->GetBinCenter(ibin));
          sumofbc += htempPerWire->GetBinContent(ibin);
        }
      }
      if (sumofbc > 0)truncMean  = (double)(binweights / sumofbc);
      else truncMean = 1.0;
    }

    if (truncMean <= 0)truncMean = 1.0;
    iWireTruncMean[jwire] =  truncMean;
    //std::cout << "Wire ID = " << jwire << "), truncMean = " << truncMean  << std::endl;

    if (isMakePlots) {
      ctmp->cd(jwire % 16 + 1);
      htempPerWire->SetTitle(Form("dedxhit dist for wire # = %d", jwire));
      htempPerWire->SetFillColor(kYellow);
      htempPerWire->DrawCopy("hist");
      if ((jwire + 1) % 16 == 0)ctmp->Print(psname.str().c_str());
    }

    htempPerWire->Reset();
  }

  Int_t iSuperLayer = 0, nWireiLayer = 0, fromWire = 0, toWire = 0, countwire = 0;
  double LayerMeanSum[56];
  std::vector<double> AvgLayerSum;

  for (Int_t iLayer = 0; iLayer < 56; iLayer++) {
    iSuperLayer = (iLayer - 2) / 6;
    if (iSuperLayer <= 0)iSuperLayer = 1; //hack for wire#
    nWireiLayer = 160 + (iSuperLayer - 1) * 32;

    fromWire = countwire; // or towire+countwire
    toWire = toWire + nWireiLayer;

    LayerMeanSum[iLayer] = 0.0;
    int counter = 0;

    //std::cout << "Layer = " << iLayer << "), fromWire = " << fromWire << ") to wire = (" << toWire - 1 << ", total wire = "<< toWire-fromWire << std::endl;
    for (Int_t jwire = fromWire; jwire < toWire; jwire++) {
      countwire++;
      bool IsSkip = false;
      for (unsigned int kwire = 0; kwire < listofbadwires.size(); ++kwire) {
        if (jwire == listofbadwires.at(kwire)) {
          IsSkip = true;
          break;
        }
      }
      if (IsSkip)continue;

      if (iWireTruncMean[jwire] >= 0.) {
        LayerMeanSum[iLayer] += iWireTruncMean[jwire];
        counter++;
      }
    }

    if ((LayerMeanSum[iLayer] / counter) >= 0.)AvgLayerSum.push_back(LayerMeanSum[iLayer] / counter);
    else AvgLayerSum.push_back(1.0);
    //std::cout << "Layer = " << iLayer << ", Total wire mean = " << LayerMeanSum[iLayer] << ", S[iLayer] = " << AvgLayerSum.at(iLayer) << std::endl;
  }

  double ScaleFactorAvg = TMath::Mean(AvgLayerSum.begin() + 8, AvgLayerSum.end()); //frp, 8 to 56 only
  if (ScaleFactorAvg <= 0)ScaleFactorAvg = 1.0;
  //std::cout << "ScaleFactorAvg = " << ScaleFactorAvg << std::endl;

  //voting all wires with scale factor calculated from outer layer
  for (Int_t jwire = 0; jwire < 14336; jwire++) iWireTruncMean[jwire] /= ScaleFactorAvg;

  //setting wire gain to 0 for bad wires
  if (isRmBadwires)for (unsigned int kwire = 0; kwire < listofbadwires.size();
                          ++kwire) iWireTruncMean[listofbadwires.at(kwire)] = 0.0;

  //Storing final version of wiregain constants
  std::vector<double> dedxTruncmean;
  for (unsigned int kwire = 0; kwire < 14336; ++kwire) {
    dedxTruncmean.push_back(iWireTruncMean[kwire]);
    std::cout << kwire <<  " WireGain for this = " << iWireTruncMean[kwire] << std::endl;
  }

  B2INFO("dE/dx Calibration done for " << dedxTruncmean.size() << " CDC wires");
  CDCDedxWireGain* gains = new CDCDedxWireGain(dedxTruncmean);
  saveCalibration(gains, "CDCDedxWireGain");


  delete htempPerWire;
  if (isMakePlots) {
    psname.str(""); psname << "dedx_wiregains.pdf]";
    ctmp->Print(psname.str().c_str());
    delete ctmp;
  }

  return c_OK;
}