/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <svd/calibration/SVDTimeValidationAlgorithm.h>

#include <svd/dbobjects/SVDCoGCalibrationFunction.h>
#include <svd/calibration/SVDCoGTimeCalibrations.h>

#include <TF1.h>
#include <TProfile.h>
#include <TH2F.h>
#include <framework/logging/Logger.h>
#include <iostream>
#include <TString.h>
#include <TFitResult.h>

using namespace std;
using namespace Belle2;

SVDTimeValidationAlgorithm::SVDTimeValidationAlgorithm() :
  CalibrationAlgorithm("SVDTimeValidationCollector")
{
  setDescription("SVDTimeValidation calibration algorithm");
}

CalibrationAlgorithm::EResult SVDTimeValidationAlgorithm::calibrate()
{

  gROOT->SetBatch(true);

  auto hEventT0 = getObjectPtr<TH1F>("hEventT0");
  float eventT0_mean = 0;
  float eventT0_rms = 0;
  if (hEventT0) {
    eventT0_mean = hEventT0->GetMean();
    eventT0_rms = hEventT0->GetRMS();
  } else
    B2ERROR("Histogram with Event T0 not found");

  B2DEBUG(27, "Histogram: " << hEventT0->GetName() <<
          " Entries (n. clusters): " << hEventT0->GetEntries() <<
          " Mean: " << eventT0_mean);

  VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();
  for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD)) {
    for (auto ladder : geoCache.getLadders(layer)) {
      for (Belle2::VxdID sensor :  geoCache.getSensors(ladder)) {
        for (int view = SVDHistograms<TH1F>::VIndex ; view < SVDHistograms<TH1F>::UIndex + 1; view++) {
          char side = 'U';
          if (view == 0)
            side = 'V';
          auto layer_num = sensor.getLayerNumber();
          auto ladder_num = sensor.getLadderNumber();
          auto sensor_num = sensor.getSensorNumber();
          auto hClsTimeOnTracks = getObjectPtr<TH1F>(Form("clsTimeOnTracks__L%dL%dS%d%c", layer_num, ladder_num, sensor_num, side));
          float clsTimeOnTracks_mean = 0.;
          if (hClsTimeOnTracks)
            clsTimeOnTracks_mean = hClsTimeOnTracks->GetMean();
          else
            B2ERROR("Histogram " << Form("clsTimeOnTracks__L%dL%dS%d%c", layer_num, ladder_num, sensor_num, side) << " not found");
          auto deviation = (clsTimeOnTracks_mean - eventT0_mean) / eventT0_rms;

          B2DEBUG(27, "Histogram: " << hClsTimeOnTracks->GetName() <<
                  " Entries (n. clusters): " << hClsTimeOnTracks->GetEntries() <<
                  " Mean: " << clsTimeOnTracks_mean <<
                  " Deviation: " << deviation << " EventT0 RMS");
          if (abs(deviation) > m_allowedDeviationMean)
            B2ERROR("Histogram: " << hClsTimeOnTracks->GetName() << " deviates from EventT0 by" << deviation << " times the EventT0 RMS");

        }
      }
    }
  }
  return c_OK;
}
