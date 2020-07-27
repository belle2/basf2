/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Corona, Giulia Casarosa                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <svd/calibration/SVD3SampleELSTimeCalibrationAlgorithm.h>

#include <svd/dbobjects/SVDCoGCalibrationFunction.h>
#include <svd/calibration/SVD3SampleELSTimeCalibrations.h>

#include <TF1.h>
#include <TProfile.h>
#include <TH2F.h>
#include <framework/logging/Logger.h>
#include <iostream>
#include <TString.h>

using namespace std;
using namespace Belle2;

SVD3SampleELSTimeCalibrationAlgorithm::SVD3SampleELSTimeCalibrationAlgorithm(const std::string& str) :
  CalibrationAlgorithm("SVDCoGTimeCalibrationCollector")
{
  setDescription("SVD3SampleELSTimeCalibration calibration algorithm");
  m_id = str;
}

CalibrationAlgorithm::EResult SVD3SampleELSTimeCalibrationAlgorithm::calibrate()
{

  gROOT->SetBatch(true);

  int ladderOfLayer[4] = {7, 10, 12, 16};
  int sensorOnLayer[4] = {2, 3, 4, 5};

  auto timeCal = new Belle2::SVDCoGCalibrationFunction();
  auto payload = new Belle2::SVD3SampleELSTimeCalibrations::t_payload(*timeCal, m_id);

  std::unique_ptr<TF1> pol3(new TF1("pol3", "[0] + [1]*x + [2]/(x - [3])", -60,
                                    0)); // In the local study, Y. Uematsu tuned the range to (-21.5,0). Original value is (-10,80).
  pol3->SetParameter(2, -100);
  pol3->SetParameter(3, 10);
  // pol3->SetParLimits(0, -50, 0);
  // pol3->SetParLimits(1, 0, 1);
  // pol3->SetParLimits(2, -2000, 0);
  // pol3->SetParLimits(3, 0, 50);

  std::unique_ptr<TFile> f(new TFile("algorithm_3SampleELS_output.root", "RECREATE"));

  for (int layer = 0; layer < 4; layer++) {
    int layer_num = layer + 3;
    for (int ladder = 0; ladder < (int)ladderOfLayer[layer]; ladder++) {
      int ladder_num = ladder + 1;
      for (int sensor = 0; sensor < (int)sensorOnLayer[layer]; sensor++) {
        int sensor_num = sensor + 1;
        for (int view  = 1; view > -1; view--) {
          char side = 'U';
          if (view == 0)
            side = 'V';
          auto hEventT0vsELS = getObjectPtr<TH2F>(Form("eventT0vsCoG__L%dL%dS%d%c", layer_num, ladder_num, sensor_num, side));
          auto hEventT0 = getObjectPtr<TH1F>(Form("eventT0__L%dL%dS%d%c", layer_num, ladder_num, sensor_num, side));
          auto hEventT0nosync = getObjectPtr<TH1F>(Form("eventT0nosync__L%dL%dS%d%c", layer_num, ladder_num, sensor_num, side));
          B2INFO("Histogram: " << hEventT0vsELS->GetName() <<
                 " Entries (n. clusters): " << hEventT0vsELS->GetEntries());
          if (layer_num == 3 && hEventT0vsELS->GetEntries() < m_minEntries) {
            B2INFO("Histogram: " << hEventT0vsELS->GetName() <<
                   " Entries (n. clusters): " << hEventT0vsELS->GetEntries() <<
                   " Entries required: " << m_minEntries);
            B2WARNING("Not enough data, adding one run to the collector");
            return c_NotEnoughData;
          }
          for (int i = 1; i <= hEventT0vsELS->GetNbinsX(); i++) {
            for (int j = 1; j <= hEventT0vsELS->GetNbinsY(); j++) {
              if (hEventT0vsELS->GetBinContent(i, j) < int(hEventT0vsELS->GetEntries() * 0.001)) {
                hEventT0vsELS->SetBinContent(i, j, 0);
              }
            }
          }
          TProfile* pfx = hEventT0vsELS->ProfileX();
          std::string name = "pfx_" + std::string(hEventT0vsELS->GetName());
          pfx->SetName(name.c_str());
          // pfx->SetErrorOption("S");
          pfx->Fit("pol3", "RQ");
          double par[4];
          pol3->GetParameters(par);
          // double meanT0 = hEventT0->GetMean();
          // double meanT0NoSync = hEventT0nosync->GetMean();
          timeCal->set_current(3);
          timeCal->set_elsparameters(par[0], par[1], par[2], par[3]);
          payload->set(layer_num, ladder_num, sensor_num, bool(view), 1, *timeCal);
          f->cd();
          hEventT0->Write();
          hEventT0vsELS->Write();
          hEventT0nosync->Write();
          pfx->Write();

        }
      }
    }
  }
  f->Close();
  saveCalibration(payload, "SVD3SampleELSTimeCalibrations");

  //delete f;

  // probably not needed - would trigger re-doing the collection
  // if ( ... too large corrections ... ) return c_Iterate;
  return c_OK;
}

bool SVD3SampleELSTimeCalibrationAlgorithm::isBoundaryRequired(const Calibration::ExpRun& currentRun)
{
  float meanRawCoGTimeL3V = 0;
  // auto eventT0Hist = getObjectPtr<TH1F>("hEventT0FromCDST");
  auto rawCoGTimeL3V = getObjectPtr<TH1F>("hRawCoGTimeL3V");
  // float meanEventT0 = eventT0Hist->GetMean();
  if (!rawCoGTimeL3V) {
    meanRawCoGTimeL3V = m_previousRawCoGTimeMeanL3V.value();
  } else {
    meanRawCoGTimeL3V = rawCoGTimeL3V->GetMean();
  }
  if (!m_previousRawCoGTimeMeanL3V) {
    B2INFO("Setting start payload boundary to be the first run ("
           << currentRun.first << "," << currentRun.second << ")");
    m_previousRawCoGTimeMeanL3V.emplace(meanRawCoGTimeL3V);

    return true;
  } else if (abs(meanRawCoGTimeL3V - m_previousRawCoGTimeMeanL3V.value()) > m_allowedTimeShift) {
    B2INFO("Histogram mean has shifted from " << m_previousRawCoGTimeMeanL3V.value()
           << " to " << meanRawCoGTimeL3V << ". We are requesting a new payload boundary for ("
           << currentRun.first << "," << currentRun.second << ")");
    m_previousRawCoGTimeMeanL3V.emplace(meanRawCoGTimeL3V);
    return true;
  } else {
    return false;
  }
}

