/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Corona, Giulia Casarosa                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <svd/calibration/SVDCoGTimeCalibrationAlgorithm.h>

#include <svd/dbobjects/SVDCoGCalibrationFunction.h>
#include <svd/calibration/SVDCoGTimeCalibrations.h>

#include <TF1.h>
#include <TProfile.h>
#include <TH2F.h>
#include <framework/logging/Logger.h>
#include <iostream>
#include <TString.h>

using namespace std;
using namespace Belle2;

SVDCoGTimeCalibrationAlgorithm::SVDCoGTimeCalibrationAlgorithm(const std::string& str) :
  CalibrationAlgorithm("SVDCoGTimeCalibrationCollector")
{
  setDescription("SVDCoGTimeCalibration calibration algorithm");
  m_id = str;
}

CalibrationAlgorithm::EResult SVDCoGTimeCalibrationAlgorithm::calibrate()
{

  gROOT->SetBatch(true);

  int ladderOfLayer[4] = {7, 10, 12, 16};
  int sensorOnLayer[4] = {2, 3, 4, 5};

  auto timeCal = new Belle2::SVDCoGCalibrationFunction();
  auto payload = new Belle2::SVDCoGTimeCalibrations::t_payload(*timeCal, m_id);

  std::unique_ptr<TF1> pol1(new TF1("pol1", "[0] + [1]*x", -10, 80));
  pol1->SetParameters(-40, 0.9);
  std::unique_ptr<TF1> pol3(new TF1("pol3", "[0] + [1]*x + [2]*x*x + [3]*x*x*x", -10, 80));
  pol3->SetParLimits(0, -200, 0);
  pol3->SetParLimits(1, 0, 10);
  pol3->SetParLimits(2, -1, 0);
  pol3->SetParLimits(3, 0, 1);
  std::unique_ptr<TF1> pol5(new TF1("pol5", "[0] + [1]*x + [2]*x*x + [3]*x*x*x + [4]*x*x*x*x + [5]*x*x*x*x*x", -100, 100));
  pol5->SetParameters(-50, 1.5, 0.01, 0.0001, 0.00001, 0.000001);

  std::unique_ptr<TFile> f(new TFile("algorithm_6SampleCoG_output.root", "RECREATE"));

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
          auto hEventT0vsCoG = getObjectPtr<TH2F>(Form("eventT0vsCoG__L%dL%dS%d%c", layer_num, ladder_num, sensor_num, side));
          auto hEventT0 = getObjectPtr<TH1F>(Form("eventT0__L%dL%dS%d%c", layer_num, ladder_num, sensor_num, side));
          auto hEventT0nosync = getObjectPtr<TH1F>(Form("eventT0nosync__L%dL%dS%d%c", layer_num, ladder_num, sensor_num, side));
          B2INFO("Histogram: " << hEventT0vsCoG->GetName() <<
                 " Entries (n. clusters): " << hEventT0vsCoG->GetEntries());
          if (layer_num == 3 && hEventT0vsCoG->GetEntries() < m_minEntries) {
            B2INFO("Histogram: " << hEventT0vsCoG->GetName() <<
                   " Entries (n. clusters): " << hEventT0vsCoG->GetEntries() <<
                   " Entries required: " << m_minEntries);
            B2WARNING("Not enough data, adding one run to the collector");
            return c_NotEnoughData;
          }
          for (int i = 1; i <= hEventT0vsCoG->GetNbinsX(); i++) {
            for (int j = 1; j <= hEventT0vsCoG->GetNbinsY(); j++) {
              if (hEventT0vsCoG->GetBinContent(i, j) < int(hEventT0vsCoG->GetEntries() * 0.001)) {
                hEventT0vsCoG->SetBinContent(i, j, 0);
              }
            }
          }
          TProfile* pfx = hEventT0vsCoG->ProfileX();
          std::string name = "pfx_" + std::string(hEventT0vsCoG->GetName());
          pfx->SetName(name.c_str());
          pfx->SetErrorOption("S");
          pfx->Fit("pol3", "RQ");
          double par[4];
          pol3->GetParameters(par);
          /** Fit with pol1 **/
          /*
          pfx->Fit("pol1", "RQ");
          double par[4];
          pol1->GetParameters(par);
          par[2] = 0;
          par[3] = 0;
          */
          // double meanT0 = hEventT0->GetMean();
          // double meanT0NoSync = hEventT0nosync->GetMean();
          timeCal->set_current(1);
          // timeCal->set_current(2);
          timeCal->set_pol3parameters(par[0], par[1], par[2], par[3]);
          payload->set(layer_num, ladder_num, sensor_num, bool(view), 1, *timeCal);
          f->cd();
          hEventT0->Write();
          hEventT0vsCoG->Write();
          hEventT0nosync->Write();
          pfx->Write();

        }
      }
    }
  }
  f->Close();
  saveCalibration(payload, "SVDCoGTimeCalibrations");

  //delete f;

  // probably not needed - would trigger re-doing the collection
  // if ( ... too large corrections ... ) return c_Iterate;
  return c_OK;
}

bool SVDCoGTimeCalibrationAlgorithm::isBoundaryRequired(const Calibration::ExpRun& currentRun)
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

