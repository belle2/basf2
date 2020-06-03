/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Corona, Giulia Casarosa                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <svd/calibration/SVD3SampleCoGTimeCalibrationAlgorithm.h>

#include <svd/dbobjects/SVDCoGCalibrationFunction.h>
#include <svd/calibration/SVD3SampleCoGTimeCalibrations.h>

#include <TF1.h>
#include <TProfile.h>
#include <TH2F.h>
#include <framework/logging/Logger.h>
#include <iostream>
#include <TString.h>

using namespace std;
using namespace Belle2;

SVD3SampleCoGTimeCalibrationAlgorithm::SVD3SampleCoGTimeCalibrationAlgorithm(const std::string& str) :
  CalibrationAlgorithm("SVDCoGTimeCalibrationCollector")
{
  setDescription("SVD3SampleCoGTimeCalibration calibration algorithm");
  m_id = str;
}

CalibrationAlgorithm::EResult SVD3SampleCoGTimeCalibrationAlgorithm::calibrate()
{

  int ladderOfLayer[4] = {7, 10, 12, 16};
  int sensorOnLayer[4] = {2, 3, 4, 5};

  auto timeCal = new Belle2::SVDCoGCalibrationFunction();
  auto payload = new Belle2::SVD3SampleCoGTimeCalibrations::t_payload(*timeCal, m_id);

  TF1* pol3 = new TF1("pol3", "[0] + [1]*x + [2]*x*x + [3]*x*x*x", -50, 80);
  pol3->SetParameters(-40, 0.5, 0.05, 0.0005);
  TF1* pol5 = new TF1("pol5", "[0] + [1]*x + [2]*x*x + [3]*x*x*x + [4]*x*x*x*x + [5]*x*x*x*x*x", -100, 100);
  pol5->SetParameters(-50, 1.5, 0.01, 0.0001, 0.00001, 0.000001);

  TFile* f = new TFile("algorithm_3SampleCoG_output.root", "RECREATE");

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
          cout << " " << endl;
          cout << typeid(hEventT0vsCoG).name() << " " << hEventT0vsCoG->GetName() << " " << hEventT0vsCoG->GetEntries() << endl;
          if (layer_num == 3 && hEventT0vsCoG->GetEntries() < m_minEntries) {
            cout << " " << endl;
            cout << hEventT0vsCoG->GetName() << " " << hEventT0vsCoG->GetEntries() << " Entries required: " << m_minEntries << endl;
            cout << "Not enough data, adding one run to the collector" << endl;
            return c_NotEnoughData;
          }
          cout << " " << endl;
          for (int i = 0; i < hEventT0vsCoG->GetNbinsX(); i++) {
            for (int j = 0; j < hEventT0vsCoG->GetNbinsY(); j++) {
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
  saveCalibration(payload, "SVD3SampleCoGTimeCalibrations");

  delete f;

  // probably not needed - would trigger re-doing the collection
  // if ( ... too large corrections ... ) return c_Iterate;
  return c_OK;
}

bool SVD3SampleCoGTimeCalibrationAlgorithm::isBoundaryRequired(const Calibration::ExpRun& currentRun)
{
  auto rawCoGTimeL3VHist = getObjectPtr<TH1F>("hRawCoGTimeL3V");
  float meanRawCoG = rawCoGTimeL3VHist->GetMean();
  if (!m_previousRawCoG) {
    B2INFO("Setting start payload boundary to be the first run ("
           << currentRun.first << "," << currentRun.second << ")");
    m_previousRawCoG.emplace(meanRawCoG);
    return true;
  } else {
    if (abs(meanRawCoG - m_previousRawCoG.value()) > m_allowedTimeShift) {
      B2INFO("Histogram mean has shifted from " << m_previousRawCoG.value()
             << " to " << meanRawCoG << ". We are requesting a new payload boundary for ("
             << currentRun.first << "," << currentRun.second << ")");
      m_previousRawCoG.emplace(meanRawCoG);
      return true;
    }
  }
  return false;
}

