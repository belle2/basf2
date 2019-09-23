/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: James Webb                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/calibration/SVDCrossTalkCalibrationsAlgorithm.h>
#include <svd/calibration/SVDCrossTalkStripsCalibrations.h>

using namespace std;
using namespace Belle2;

SVDCrossTalkCalibrationsAlgorithm::SVDCrossTalkCalibrationsAlgorithm(std::string str) :
  CalibrationAlgorithm("SVDCrossTalkCalibrationsCollector")
{
  setDescription("Calibration algorithm for SVDCrossTalkCalibrations payload");
  m_id = str;
}

CalibrationAlgorithm::EResult SVDCrossTalkCalibrationsAlgorithm::calibrate()
{
  int isCrossTalkCal = 0;
  auto payload = new Belle2::SVDCrossTalkStripsCalibrations::t_payload();

  auto tree = getObjectPtr<TTree>("HTreeCrossTalkCalib");

  TH1F* hist = new TH1F("", "", 768, 0, 768);

  if (!tree) {
    B2WARNING("No tree");
  } else if (!tree->GetEntries()) {
    B2WARNING("Empty tree");
  }

  int layer = 0;
  int ladder = 0;
  int sensor = 0;
  int side = 0;

  tree->SetBranchAddress("hist", &hist);
  tree->SetBranchAddress("layer", &layer);
  tree->SetBranchAddress("ladder", &ladder);
  tree->SetBranchAddress("sensor", &sensor);
  tree->SetBranchAddress("side", &side);



  for (int i = 0; i < tree->GetEntries(); i++) {
    tree->GetEntry(i);

    int nstrips = 768;
    if (side == 0 && layer != 3) nstrips = 512;

    for (int strip = 0; strip < nstrips; strip++) {
      isCrossTalkCal = hist->GetBinContent(strip + 1);

      payload->set(layer, ladder, sensor, side, strip, isCrossTalkCal);
    }
  }


  saveCalibration(payload, "SVDCrossTalkStripsCalibrations");

  return c_OK;

}
