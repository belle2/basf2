/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/calibration/PXDClusterPositionCalibrationAlgorithm.h>
#include <TClonesArray.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <string>


using namespace std;
using namespace Belle2;


PXDClusterPositionCalibrationAlgorithm::PXDClusterPositionCalibrationAlgorithm():
  CalibrationAlgorithm("PXDClusterPositionCollector"),
  minClusterForShapeLikelyhood(500), minClusterForPositionOffset(2000)
{
  setDescription(
    " -------------------------- PXDClusterPositionCalibrationAlgorithm ----------------------\n"
    "                                                                                         \n"
    "  Algorithm for estimating cluster position offsets and shape likelyhoods.               \n"
    " ----------------------------------------------------------------------------------------\n"
  );
}

CalibrationAlgorithm::EResult PXDClusterPositionCalibrationAlgorithm::calibrate()
{
  /* Read data and make histo*/
  auto tree = getObjectPtr<TTree>("pxdCal");

  TClonesArray* pxdClusterArray = new TClonesArray("Belle2::PXDCluster");
  TClonesArray* pxdDigitArray = new TClonesArray("Belle2::PXDDigit");
  TClonesArray* pxdTrueHitArray = new TClonesArray("Belle2::PXDTrueHit");

  tree->SetBranchAddress("PXDClusterArray", &pxdClusterArray);
  tree->SetBranchAddress("PXDDigitArray", &pxdDigitArray);
  tree->SetBranchAddress("PXDTrueHitArray", &pxdTrueHitArray);

  const auto nEntries = tree->GetEntries();
  B2INFO("Number of entries " << nEntries);
  for (int i = 0; i < nEntries; ++i) {
    pxdClusterArray->Clear();
    pxdDigitArray->Clear();
    pxdTrueHitArray->Clear();
    tree->GetEntry(i);

    const PXDCluster* const cls = (PXDCluster*)pxdClusterArray->At(0);
    B2INFO("Cluster sensorID " << cls->getSensorID() << " charge " << cls->getCharge());

    const PXDTrueHit* const hit = (PXDTrueHit*)pxdTrueHitArray->At(0);
    B2INFO("Cluster hit u " << hit->getU() << " v " << hit->getV());

    int nDigits = pxdDigitArray->GetEntriesFast();
    for (int iDigit = 0; iDigit < nDigits; iDigit++) {
      const PXDDigit* const currdigit = (PXDDigit*)pxdDigitArray->At(iDigit);
      B2INFO("Digit ui " << currdigit->getUCellID() << " vi " << currdigit->getVCellID()  << " charge " << currdigit->getCharge());
    }
  }

  pxdClusterArray->Delete();
  pxdDigitArray->Delete();
  pxdTrueHitArray->Delete();

  // Save the hot pixel mask to database. Note that this will set the database object name to the same as the collector but you
  // are free to change it.
  //PXDMaskedPixelPar* maskedPixelsPar = new PXDMaskedPixelPar();
  //saveCalibration(maskedPixelsPar, "PXDMaskedPixelPar");

  if (getIteration() < 1) {
    B2INFO("Calibration called for iteration");
    return c_Iterate;
  }

  B2INFO("PXDClusterPosition Calibration Successful");
  return c_OK;
}
