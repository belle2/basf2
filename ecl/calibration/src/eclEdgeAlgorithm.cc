#include <ecl/calibration/eclEdgeAlgorithm.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include "TH1F.h"
#include "TString.h"
#include "TFile.h"
#include "TDirectory.h"

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
  histfile->Close();


  //-----------------------------------------------------------------------------------
  //..Store the payloads

  //..Edges of the crystals in theta
  std::vector<float> tempTheta;
  std::vector<float> tempUnc(8736, 0.);
  for (int cellID = 1; cellID <= 8736; cellID++) {
    tempTheta.push_back(eclCrystalEdgeTheta->GetBinContent(cellID));
  }
  ECLCrystalCalib* crystalTheta = new ECLCrystalCalib();
  crystalTheta->setCalibVector(tempTheta, tempUnc);
  saveCalibration(crystalTheta, "ECLCrystalThetaEdge");
  B2RESULT("eclEdgeAlgorithm: successfully stored payload ECLCrystalThetaEdge");

  //..Edges of the crystals in phi
  std::vector<float> tempPhi;
  for (int cellID = 1; cellID <= 8736; cellID++) {
    tempPhi.push_back(eclCrystalEdgePhi->GetBinContent(cellID));
  }
  ECLCrystalCalib* crystalPhi = new ECLCrystalCalib();
  crystalPhi->setCalibVector(tempPhi, tempUnc);
  saveCalibration(crystalPhi, "ECLCrystalPhiEdge");
  B2RESULT("eclEdgeAlgorithm: successfully stored payload ECLCrystalPhiEdge");

  return c_OK;
}
