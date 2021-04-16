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
#include <TFitResult.h>

using namespace std;
using namespace Belle2;

SVD3SampleCoGTimeCalibrationAlgorithm::SVD3SampleCoGTimeCalibrationAlgorithm(const std::string& str) :
  CalibrationAlgorithm("SVDTimeCalibrationCollector")
{
  setDescription("SVD3SampleCoGTimeCalibration calibration algorithm");
  m_id = str;
}

CalibrationAlgorithm::EResult SVD3SampleCoGTimeCalibrationAlgorithm::calibrate()
{

  gROOT->SetBatch(true);

  int ladderOfLayer[4] = {7, 10, 12, 16};
  int sensorOnLayer[4] = {2, 3, 4, 5};

  auto timeCal = new Belle2::SVDCoGCalibrationFunction();
  auto payload = new Belle2::SVD3SampleCoGTimeCalibrations::t_payload(*timeCal, m_id);

  std::unique_ptr<TF1> pol3(new TF1("pol3", "[0] + [1]*x + [2]*[3]*[3]*x - [2]*[3]*x*x + [2]*x*x*x/3", -10,
                                    80)); // In the local study, Y. Uematsu tuned the range to (31.5,48), because the correlation is not exactly like pol3. (The deviation appears at around 48, very naive.) However, original value (-10,80) also seems working.
  // apply parameter limits to be monotonic. upper limits are loose.
  pol3->SetParLimits(1, 0, 100);
  pol3->SetParLimits(2, 0, 0.1);

  FileStat_t info;
  int cal_rev = 1;
  while (gSystem->GetPathInfo(Form("algorithm_3SampleCoG_output_rev_%d.root", cal_rev), info) == 0)
    cal_rev++;
  std::unique_ptr<TFile> f(new TFile(Form("algorithm_3SampleCoG_output_rev_%d.root", cal_rev), "RECREATE"));

  auto m_tree = new TTree(Form("rev_%d", cal_rev), "RECREATE");
  int layer_num, ladder_num, sensor_num, view, ndf;
  float a, b, c, d, a_err, b_err, c_err, d_err, chi2, p;
  m_tree->Branch("layer", &layer_num, "layer/I");
  m_tree->Branch("ladder", &ladder_num, "ladder/I");
  m_tree->Branch("sensor", &sensor_num, "sensor/I");
  m_tree->Branch("isU", &view, "isU/I");
  m_tree->Branch("a", &a, "a/F");
  m_tree->Branch("b", &b, "b/F");
  m_tree->Branch("c", &c, "c/F");
  m_tree->Branch("d", &d, "d/F");
  m_tree->Branch("a_err", &a_err, "a_err/F");
  m_tree->Branch("b_err", &b_err, "b_err/F");
  m_tree->Branch("c_err", &c_err, "c_err/F");
  m_tree->Branch("d_err", &d_err, "d_err/F");
  m_tree->Branch("chi2", &chi2, "chi2/F");
  m_tree->Branch("ndf", &ndf, "ndf/I");
  m_tree->Branch("p", &p, "p/F");

  for (int layer = 0; layer < 4; layer++) {
    layer_num = layer + 3;
    for (int ladder = 0; ladder < (int)ladderOfLayer[layer]; ladder++) {
      ladder_num = ladder + 1;
      for (int sensor = 0; sensor < (int)sensorOnLayer[layer]; sensor++) {
        sensor_num = sensor + 1;
        for (view  = 1; view > -1; view--) {
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
            f->Close();
            gSystem->Unlink(Form("algorithm_3SampleCoG_output_rev_%d.root", cal_rev));
            return c_NotEnoughData;
          }
          for (int i = 1; i <= hEventT0vsCoG->GetNbinsX(); i++) {
            for (int j = 1; j <= hEventT0vsCoG->GetNbinsY(); j++) {
              if (hEventT0vsCoG->GetBinContent(i, j) < max(2, int(hEventT0vsCoG->GetEntries() * 0.001))) {
                hEventT0vsCoG->SetBinContent(i, j, 0);
              }
            }
          }
          TProfile* pfx = hEventT0vsCoG->ProfileX();
          std::string name = "pfx_" + std::string(hEventT0vsCoG->GetName());
          pfx->SetName(name.c_str());
          // set initial value for d, which is quadratic in the formula.
          pol3->SetParameter(3, 40);
          TFitResultPtr tfr = pfx->Fit("pol3", "QMRS");
          double par[4];
          pol3->GetParameters(par);
          // double meanT0 = hEventT0->GetMean();
          // double meanT0NoSync = hEventT0nosync->GetMean();
          timeCal->set_current(1);
          // timeCal->set_current(2);
          timeCal->set_pol3parameters(par[0], par[1] + par[2]*par[3]*par[3], -par[2]*par[3], par[2] / 3);
          payload->set(layer_num, ladder_num, sensor_num, bool(view), 1, *timeCal);
          f->cd();
          hEventT0->Write();
          hEventT0vsCoG->Write();
          hEventT0nosync->Write();
          pfx->Write();

          a = par[0]; b = par[1]; c = par[2]; d = par[3];
          a_err = tfr->ParError(0); b_err = tfr->ParError(1); c_err = tfr->ParError(2); d_err = tfr->ParError(3);
          chi2 = tfr->Chi2();
          ndf = tfr->Ndf();
          p = tfr->Prob();
          m_tree->Fill();

        }
      }
    }
  }
  m_tree->Write();
  f->Close();
  saveCalibration(payload, "SVD3SampleCoGTimeCalibrations");

  //delete f;

  // probably not needed - would trigger re-doing the collection
  // if ( ... too large corrections ... ) return c_Iterate;
  return c_OK;
}

bool SVD3SampleCoGTimeCalibrationAlgorithm::isBoundaryRequired(const Calibration::ExpRun& currentRun)
{
  float meanRawTimeL3V = 0;
  // auto eventT0Hist = getObjectPtr<TH1F>("hEventT0FromCDC");
  auto rawTimeL3V = getObjectPtr<TH1F>("hRawTimeL3V");
  // float meanEventT0 = eventT0Hist->GetMean();
  if (!rawTimeL3V) {
    if (m_previousRawTimeMeanL3V)
      meanRawTimeL3V = m_previousRawTimeMeanL3V.value();
  } else {
    if (rawTimeL3V->GetEntries() > m_minEntries)
      meanRawTimeL3V = rawTimeL3V->GetMean();
    else {
      if (m_previousRawTimeMeanL3V)
        meanRawTimeL3V = m_previousRawTimeMeanL3V.value();
    }
  }
  if (!m_previousRawTimeMeanL3V) {
    B2INFO("Setting start payload boundary to be the first run ("
           << currentRun.first << "," << currentRun.second << ")");
    m_previousRawTimeMeanL3V.emplace(meanRawTimeL3V);

    return true;
  } else if (abs(meanRawTimeL3V - m_previousRawTimeMeanL3V.value()) > m_allowedTimeShift) {
    B2INFO("Histogram mean has shifted from " << m_previousRawTimeMeanL3V.value()
           << " to " << meanRawTimeL3V << ". We are requesting a new payload boundary for ("
           << currentRun.first << "," << currentRun.second << ")");
    m_previousRawTimeMeanL3V.emplace(meanRawTimeL3V);
    return true;
  } else {
    return false;
  }
}
