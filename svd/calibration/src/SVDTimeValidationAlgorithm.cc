/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Corona, Giulia Casarosa                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
  auto eventT0_mean = hEventT0->GetMean();
  auto eventT0_rms = hEventT0->GetRMS();

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
          auto clsTimeOnTracks_mean = hClsTimeOnTracks->GetMean();
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
