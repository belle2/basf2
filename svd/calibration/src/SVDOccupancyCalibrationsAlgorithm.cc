/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <svd/calibration/SVDOccupancyCalibrationsAlgorithm.h>

#include <svd/calibration/SVDOccupancyCalibrations.h>

#include <framework/logging/Logger.h>

#include <TH1F.h>
#include <TString.h>

#include <iostream>

using namespace std;
using namespace Belle2;

SVDOccupancyCalibrationsAlgorithm::SVDOccupancyCalibrationsAlgorithm(const std::string& str) :
  CalibrationAlgorithm("SVDOccupancyCalibrationsCollector")
{
  setDescription("Calibration algorithm for SVDOccupancyCalibrations payloads");
  m_id = str;
}

CalibrationAlgorithm::EResult SVDOccupancyCalibrationsAlgorithm::calibrate()
{

  float occCal = 1.;
  auto payload = new Belle2::SVDOccupancyCalibrations::t_payload(occCal , m_id);

  auto tree = getObjectPtr<TTree>("HTreeOccupancyCalib");

  TH1F* hocc = new TH1F("", "", 768, 0, 768);

  if (!tree) {
    B2WARNING("No tree object.");
  } else if (!tree->GetEntries()) {
    B2WARNING("No data in the tree.");
  }

  int layer = 0;
  int ladder = 0;
  int sensor = 0;
  int side = 0;

  tree->SetBranchAddress("hist", &hocc);
  tree->SetBranchAddress("layer", &layer);
  tree->SetBranchAddress("ladder", &ladder);
  tree->SetBranchAddress("sensor", &sensor);
  tree->SetBranchAddress("view", &side);

  for (int i = 0; i < tree->GetEntries(); i++) {
    tree->GetEntry(i);

    int nstrips = 768;
    if (!side && layer != 3) nstrips = 512;

    for (int iterStrip = 0; iterStrip < nstrips; iterStrip++) {
      occCal = hocc->GetBinContent(iterStrip + 1);

      payload->set(layer, ladder, sensor, bool(side), iterStrip, occCal);
    }
  }

  saveCalibration(payload, "SVDOccupancyCalibrations");

  // probably not needed - would trigger re-doing the collection
  //if ( ... too large corrections ... ) return c_Iterate;
  return c_OK;

}
