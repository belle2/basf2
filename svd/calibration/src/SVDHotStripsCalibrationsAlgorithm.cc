/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <svd/calibration/SVDHotStripsCalibrationsAlgorithm.h>


#include <svd/calibration/SVDHotStripsCalibrations.h>

#include <TH1F.h>
#include <framework/logging/Logger.h>
#include <iostream>
#include <TString.h>

using namespace Belle2;

SVDHotStripsCalibrationsAlgorithm::SVDHotStripsCalibrationsAlgorithm(const std::string& str) :
  CalibrationAlgorithm("SVDOccupancyCalibrationsCollector")
{
  setDescription("SVDHotStripsCalibrations calibration algorithm");
  m_id = str;
}

CalibrationAlgorithm::EResult SVDHotStripsCalibrationsAlgorithm::calibrate()
{

  int isHotStrip = 0;
  int vecHS[768];
  double stripOccAfterAbsCut[768];

  // float occCal = 1.;
  float occThr = m_absoluteOccupancyThreshold; // first absolute cut for strips with occupancy >20%

  // auto HSBitmap = new Belle2::SVDCalibrationsBitmap();
  auto payload = new Belle2::SVDHotStripsCalibrations::t_payload(isHotStrip, m_id);

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

  std::map<std::tuple<int, int, int, int>, TH1F*> map_hocc;

  for (int i = 0; i < tree->GetEntries(); i++) {
    tree->GetEntry(i);

    TH1F*& h = map_hocc[std::make_tuple(layer, ladder, sensor, side)];
    if (h == nullptr) {
      h = (TH1F*)hocc->Clone(Form("hocc_L%dL%dS%d_%d", layer, ladder, sensor, side));
    } else {
      h->Add(hocc);
    }
  }

  FileStat_t info;
  int cal_rev = 1;
  while (gSystem->GetPathInfo(Form("algorithm_SVDHotStripsCalibrations_output_rev_%d.root", cal_rev), info) == 0)
    cal_rev++;
  std::unique_ptr<TFile> f(new TFile(Form("algorithm_SVDHotStripsCalibrations_output_rev_%d.root", cal_rev), "RECREATE"));

  for (const auto& key : map_hocc) {
    std::tie(layer, ladder, sensor, side) = key.first;
    int nstrips = 768;
    if (!side && layer != 3) nstrips = 512;

    TH1F* h = (TH1F*)key.second;

    f->WriteTObject(h);

    for (int i = 0; i < nstrips; i++) { vecHS[i] = 0;}

    for (int iterStrip = 0; iterStrip < nstrips; iterStrip++) {
      float occCal = h->GetBinContent(iterStrip + 1);
      // B2INFO("Occupancy for: " << layer << "." << ladder << "." << sensor << "." << side << ", strip:" << iterStrip << ": " << occCal);

      if (occCal > occThr) {
        vecHS[iterStrip] = 1;
        stripOccAfterAbsCut[iterStrip] = 0;
      } else stripOccAfterAbsCut[iterStrip] = occCal;
    }

    // iterative procedure
    // B2INFO("Starting iterative procedure for hot strips finding");

    while (theHSFinder(stripOccAfterAbsCut, vecHS, nstrips)) {}

    for (int l = 0; l < nstrips; l++) {
      isHotStrip = (int) vecHS[l];

      payload->set(layer, ladder, sensor, bool(side), l, isHotStrip);
    }
  }

  saveCalibration(payload, "SVDHotStripsCalibrations");

  // probably not needed - would trigger re-doing the collection
  //if ( ... too large corrections ... ) return c_Iterate;
  return c_OK;
}


bool SVDHotStripsCalibrationsAlgorithm::theHSFinder(double* stripOccAfterAbsCut, int* hsflag, int nstrips)
{
  bool found = false;

  int base = nstrips;
  if (m_computeAverageOccupancyPerChip) base = 128;

  int N = nstrips / base;

  for (int sector = 0; sector < N; sector++) {

    int nafter = 0;
    double sensorOccAverage = 0;

    for (int l = sector * base; l < sector * base + base; l++) {
      sensorOccAverage += stripOccAfterAbsCut[l];
      if (stripOccAfterAbsCut[l] > 0) nafter++;
    }
    sensorOccAverage = sensorOccAverage / nafter;

    B2DEBUG(1, "Average occupancy: " << sensorOccAverage);

    for (int l = sector * base; l < sector * base + base; l++) {

      // flag additional HS by comparing each strip occupancy with the sensor/side-based average occupancy
      if (stripOccAfterAbsCut[l] > sensorOccAverage * m_relativeOccupancyWrtAvgOccupancy) {
        hsflag[l] = 1;
        found = true;
        stripOccAfterAbsCut[l] = 0;
      }
    }
  }

  return found;
}
