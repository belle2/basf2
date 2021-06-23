#include <ecl/calibration/eclEdgeAlgorithm.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include "TH1F.h"
#include "TMath.h"
#include "TString.h"
#include "TFile.h"
#include "TDirectory.h"
#include <iostream>

using namespace std;
using namespace Belle2;
using namespace ECL;
using namespace Calibration;


/**-------------------------------------------------------------------------------------*/
eclEdgeAlgorithm::eclEdgeAlgorithm(): CalibrationAlgorithm("eclEdgeCollector")
{
  setDescription(
    "Generate payloads ECLCrystalThetaEdge and ECLCrystalPhiEdge found by eclEdgeCollector"
  );
}


/**-------------------------------------------------------------------------------------*/
CalibrationAlgorithm::EResult eclEdgeAlgorithm::calibrate()
{

  //-----------------------------------------------------------------------------------
  //..Read in histograms created by the collector
  auto eclCrystalX = getObjectPtr<TH1F>("eclCrystalX");
  auto eclCrystalY = getObjectPtr<TH1F>("eclCrystalY");
  auto eclCrystalZ = getObjectPtr<TH1F>("eclCrystalZ");
  auto eclCrystalR = getObjectPtr<TH1F>("eclCrystalR");
  auto eclCrystalTheta = getObjectPtr<TH1F>("eclCrystalTheta");
  auto eclCrystalPhi = getObjectPtr<TH1F>("eclCrystalPhi");
  auto eclCrystalDirTheta = getObjectPtr<TH1F>("eclCrystalDirTheta");
  auto eclCrystalDirPhi = getObjectPtr<TH1F>("eclCrystalDirPhi");
  auto eclCrystalEdgeTheta = getObjectPtr<TH1F>("eclCrystalEdgeTheta");
  auto eclCrystalEdgePhi = getObjectPtr<TH1F>("eclCrystalEdgePhi");
  auto eclEdgeCounter = getObjectPtr<TH1F>("eclEdgeCounter");

  //..And write them to disk
  TFile* histfile = new TFile("eclEdgeAlgorithm.root", "recreate");
  eclCrystalX->Write();
  eclCrystalY->Write();
  eclCrystalZ->Write();
  eclCrystalR->Write();
  eclCrystalTheta->Write();
  eclCrystalPhi->Write();
  eclCrystalDirTheta->Write();
  eclCrystalDirPhi->Write();
  eclCrystalEdgeTheta->Write();
  eclCrystalEdgePhi->Write();
  eclEdgeCounter->Write();

  //-----------------------------------------------------------------------------------
  //..Set up

  //..Number of collector calls. Intended to be 1.
  const int nCalls = (int)(eclEdgeCounter->GetBinContent(1) + 0.0001);

  //..First crysID of each thetaID will be handy
  int firstCrysID[69] = {};
  for (int thetaID = 1; thetaID < 69; thetaID++) {
    firstCrysID[thetaID] = firstCrysID[thetaID - 1] + m_crystalsPerRing[thetaID - 1];
  }

  //..Histograms of width payloads
  TH1F* eclCrystalWidthTheta = new TH1F("eclCrystalWidthTheta", "Width of each crystal in theta;cellID;crystal width (rad)", 8736, 1,
                                        8737);
  TH1F* eclCrystalWidthPhi = new TH1F("eclCrystalWidthPhi", "Width of each crystal in phi;cellID;crystal width (rad)", 8736, 1, 8737);

  //-----------------------------------------------------------------------------------
  //..Edges of the crystals in theta
  std::vector<float> tempThetaEdge;
  std::vector<float> tempUnc(8736, 0.);
  for (int cellID = 1; cellID <= 8736; cellID++) {
    tempThetaEdge.push_back(eclCrystalEdgeTheta->GetBinContent(cellID) / nCalls);
  }
  ECLCrystalCalib* crystalThetaEdge = new ECLCrystalCalib();
  crystalThetaEdge->setCalibVector(tempThetaEdge, tempUnc);
  saveCalibration(crystalThetaEdge, "ECLCrystalThetaEdge");
  B2RESULT("eclEdgeAlgorithm: successfully stored payload ECLCrystalThetaEdge");

  //-----------------------------------------------------------------------------------
  //..Edges of the crystals in phi
  std::vector<float> tempPhiEdge;
  for (int cellID = 1; cellID <= 8736; cellID++) {
    tempPhiEdge.push_back(eclCrystalEdgePhi->GetBinContent(cellID) / nCalls);
  }
  ECLCrystalCalib* crystalPhiEdge = new ECLCrystalCalib();
  crystalPhiEdge->setCalibVector(tempPhiEdge, tempUnc);
  saveCalibration(crystalPhiEdge, "ECLCrystalPhiEdge");
  B2RESULT("eclEdgeAlgorithm: successfully stored payload ECLCrystalPhiEdge");

  //-----------------------------------------------------------------------------------
  //..Width of the crystals in phi.
  //  Lower edge of next crystal in phi minus lower edge of crystal.
  std::vector<float> tempPhiWidth;
  int crysID = -1;
  for (int thetaID = 0; thetaID < 69; thetaID++) {
    for (int phiID = 0; phiID < m_crystalsPerRing[thetaID]; phiID++) {
      crysID++;
      int nextID = crysID + 1;
      if (phiID == m_crystalsPerRing[thetaID] - 1) {nextID -= m_crystalsPerRing[thetaID];}
      double width = tempPhiEdge[nextID] - tempPhiEdge[crysID];
      if (width < 0) {width += 2.*TMath::Pi();}
      tempPhiWidth.push_back(width);
    }
  }
  ECLCrystalCalib* crystalPhiWidth = new ECLCrystalCalib();
  crystalPhiWidth->setCalibVector(tempPhiWidth, tempUnc);
  saveCalibration(crystalPhiWidth, "ECLCrystalPhiWidth");
  B2RESULT("eclEdgeAlgorithm: successfully stored payload ECLCrystalPhiWidth");

  //..Also store in histogram
  for (int cellID = 1; cellID <= 8736; cellID++) {
    eclCrystalWidthPhi->SetBinContent(cellID, tempPhiWidth[cellID - 1]);
    eclCrystalWidthPhi->SetBinError(cellID, 0.);
  }
  histfile->cd();
  eclCrystalWidthPhi->Write();

  //-----------------------------------------------------------------------------------
  //..Width in theta. Look crystals in the next thetaID that overlap in phi.
  //  Last thetaID is a special case.
  std::vector<float> tempThetaWidth;
  for (int thetaID = 0; thetaID < 68; thetaID++) {
    for (int ic = firstCrysID[thetaID]; ic < firstCrysID[thetaID] + m_crystalsPerRing[thetaID]; ic++) {
      double minThetaWidth = 999.;
      double maxThetaWidth = -999.;
      for (int icnext = firstCrysID[thetaID + 1]; icnext < firstCrysID[thetaID + 1] + m_crystalsPerRing[thetaID + 1]; icnext++) {

        //..Lower edge of ic falls within icnext
        double offset = tempPhiEdge[ic] - tempPhiEdge[icnext];
        if (offset < -TMath::Pi()) {offset += 2.*TMath::Pi();}
        if (offset > TMath::Pi()) {offset -= 2.*TMath::Pi();}
        if (offset >= 0. and offset < tempPhiWidth[icnext]) {
          double width = tempThetaEdge[icnext] - tempThetaEdge[ic];
          if (width < minThetaWidth) {minThetaWidth = width;}
          if (width > maxThetaWidth) {maxThetaWidth = width;}
        }

        //..Lower edge of icnext falls within ic
        offset = tempPhiEdge[icnext] - tempPhiEdge[ic];
        if (offset < -TMath::Pi()) {offset += 2.*TMath::Pi();}
        if (offset > TMath::Pi()) {offset -= 2.*TMath::Pi();}
        if (offset >= 0. and offset < tempPhiWidth[ic]) {
          double width = tempThetaEdge[icnext] - tempThetaEdge[ic];
          if (width < minThetaWidth) {minThetaWidth = width;}
          if (width > maxThetaWidth) {maxThetaWidth = width;}
        }

      }
      tempThetaWidth.push_back(0.5 * (maxThetaWidth + minThetaWidth));
    }
  }

  //..Last thetaID; assume crystals end at nominal value from detector drawings
  const double upperThetaEdge = 2.7416;
  const int thetaID68 = 68;
  for (int ic = firstCrysID[thetaID68]; ic < firstCrysID[thetaID68] + m_crystalsPerRing[thetaID68]; ic++) {
    tempThetaWidth.push_back(upperThetaEdge - tempThetaEdge[ic]);
  }

  //..Store the payload
  ECLCrystalCalib* crystalThetaWidth = new ECLCrystalCalib();
  crystalThetaWidth->setCalibVector(tempThetaWidth, tempUnc);
  saveCalibration(crystalThetaWidth, "ECLCrystalThetaWidth");
  B2RESULT("eclEdgeAlgorithm: successfully stored payload ECLCrystalThetaWidth");

  //..Also store in histogram
  for (int cellID = 1; cellID <= 8736; cellID++) {
    eclCrystalWidthTheta->SetBinContent(cellID, tempThetaWidth[cellID - 1]);
    eclCrystalWidthTheta->SetBinError(cellID, 0.);
  }
  histfile->cd();
  eclCrystalWidthTheta->Write();

  //-----------------------------------------------------------------------------------
  //..Done
  histfile->Close();
  return c_OK;
}
