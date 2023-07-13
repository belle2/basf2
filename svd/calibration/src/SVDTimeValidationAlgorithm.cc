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
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
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

  auto __hClsTimeOnTracks__     = getObjectPtr<TH2F>("__hClsTimeOnTracks__");
  // auto __hClsTimeAll__          = getObjectPtr<TH2F>("__hClsTimeAll__");
  // auto __hClsDiffTimeOnTracks__ = getObjectPtr<TH2F>("__hClsDiffTimeOnTracks__");
  auto __hBinToSensorMap__      = getObjectPtr<TH1F>("__hBinToSensorMap__");

  for (int ij = 0; ij < (__hBinToSensorMap__->GetNbinsX()); ij++) {
    {
      {
        {

          auto binLabel = __hBinToSensorMap__->GetXaxis()->GetBinLabel(ij + 1);
          char side;
          int layer_num, ladder_num, sensor_num;
          std::sscanf(binLabel, "L%dL%dS%d%c", &layer_num, &ladder_num, &sensor_num, &side);

          B2INFO("Projecting for Sensor: " << binLabel << " with Bin Number: " << ij + 1);

          auto hClsTimeOnTracks = (TH1D*)__hClsTimeOnTracks__->ProjectionX("hClsTimeOnTracks_tmp", ij + 1, ij + 1);
          if (!hClsTimeOnTracks)
            B2ERROR("Histogram " << Form("clsTimeOnTracks__L%dL%dS%d%c", layer_num, ladder_num, sensor_num, side) << " not found");
          hClsTimeOnTracks->SetName(Form("clsTimeOnTracks__L%dL%dS%d%c", layer_num, ladder_num, sensor_num, side));
          char sidePN = (side == 'U' ? 'P' : 'N');
          hClsTimeOnTracks->SetTitle(Form("clsTimeOnTracks in %d.%d.%d %c/%c", layer_num, ladder_num, sensor_num, side, sidePN));
          hClsTimeOnTracks->SetDirectory(0);

          float clsTimeOnTracks_mean = hClsTimeOnTracks->GetMean();
          auto deviation = (clsTimeOnTracks_mean - eventT0_mean) / eventT0_rms;

          B2DEBUG(27, "Histogram: " << hClsTimeOnTracks->GetName() <<
                  " Entries (n. clusters): " << hClsTimeOnTracks->GetEntries() <<
                  " Mean: " << clsTimeOnTracks_mean <<
                  " Deviation: " << deviation << " EventT0 RMS");
          if (abs(deviation) > m_allowedDeviationMean)
            B2ERROR("Histogram: " << hClsTimeOnTracks->GetName() << " deviates from EventT0 by" << deviation << " times the EventT0 RMS");

          delete hClsTimeOnTracks;
        }
      }
    }
  }
  return c_OK;
}
