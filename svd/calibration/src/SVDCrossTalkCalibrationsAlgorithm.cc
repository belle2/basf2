/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/calibration/SVDCrossTalkCalibrationsAlgorithm.h>
#include <svd/calibration/SVDCrossTalkStripsCalibrations.h>

#include <TH1F.h>

#include <iostream>

using namespace std;
using namespace Belle2;

SVDCrossTalkCalibrationsAlgorithm::SVDCrossTalkCalibrationsAlgorithm(const std::string& str) :
  CalibrationAlgorithm("SVDCrossTalkCalibrationsCollector")
{
  setDescription("Calibration algorithm for SVDCrossTalkCalibrations payload");
  m_id = str;
}

CalibrationAlgorithm::EResult SVDCrossTalkCalibrationsAlgorithm::calibrate()
{
  int isCrossTalkCal = 0;
  auto payload = new Belle2::SVDCrossTalkStripsCalibrations::t_payload(isCrossTalkCal, m_id);

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

      if (layer == 4 && ladder == 1 && sensor == 2 && side == 1
          && hist->GetEntries() < m_minEntries) { //Check that we have enough events populating the calibration
        cout << "Not enough Data: " << hist->GetEntries() << " entries found" << endl;
        return c_NotEnoughData;
      }

      payload->set(layer, ladder, sensor, side, strip, isCrossTalkCal);
    }
  }


  saveCalibration(payload, "SVDCrossTalkStripsCalibrations");

  return c_OK;

}
