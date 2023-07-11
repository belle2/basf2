/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <svd/calibration/SVD3SampleELSTimeCalibrationAlgorithm.h>

#include <svd/dbobjects/SVDCoGCalibrationFunction.h>
#include <svd/calibration/SVD3SampleELSTimeCalibrations.h>

#include <TF1.h>
#include <TProfile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>
#include <framework/logging/Logger.h>
#include <iostream>
#include <TString.h>
#include <TFitResult.h>

using namespace std;
using namespace Belle2;

SVD3SampleELSTimeCalibrationAlgorithm::SVD3SampleELSTimeCalibrationAlgorithm(const std::string& str) :
  CalibrationAlgorithm("SVDTimeCalibrationCollector")
{
  setDescription("SVD3SampleELSTimeCalibration calibration algorithm");
  m_id = str;
}

CalibrationAlgorithm::EResult SVD3SampleELSTimeCalibrationAlgorithm::calibrate()
{

  gROOT->SetBatch(true);

  auto timeCal = new Belle2::SVDCoGCalibrationFunction();
  auto payload = new Belle2::SVD3SampleELSTimeCalibrations::t_payload(*timeCal, m_id);

  std::unique_ptr<TF1> pol1pole(new TF1("pol1pole", "[0] + [1]*x + [2]/(x - [3])", -60,
                                        0)); // In the local study, Y. Uematsu tuned the range to (-21.5,0). Original value is (-10,80).
  pol1pole->SetParameter(1, 0.6);
  pol1pole->SetParameter(2, -150);
  pol1pole->SetParLimits(1, 0, 1);
  pol1pole->SetParLimits(2, -300, 0);

  FileStat_t info;
  int cal_rev = 1;
  while (gSystem->GetPathInfo(Form("algorithm_3SampleELS_output_rev_%d.root", cal_rev), info) == 0)
    cal_rev++;
  std::unique_ptr<TFile> f(new TFile(Form("algorithm_3SampleELS_output_rev_%d.root", cal_rev), "RECREATE"));

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

  auto __hEventT0vsCoG__ = getObjectPtr<TH3F>("__hEventT0vsCoG__");
  auto __hEventT0__ = getObjectPtr<TH2F>("__hEventT0__");
  auto __hEventT0NoSync__ = getObjectPtr<TH2F>("__hEventT0NoSync__");
  auto __hBinToSensorMap__ = getObjectPtr<TH1F>("__hBinToSensorMap__");

  std::vector<TString> allSensors;
  for (int ij = 0; ij < (__hBinToSensorMap__->GetNbinsX()); ij++)
    allSensors.push_back(__hBinToSensorMap__->GetXaxis()->GetBinLabel(ij + 1));

  for (int ij = 0; ij < int(allSensors.size()); ij++) {

    auto binLabel = allSensors[ij];
    char side;
    std::sscanf(binLabel.Data(), "L%dL%dS%d%c", &layer_num, &ladder_num, &sensor_num, &side);
    view = 0;
    if (side == 'U')
      view = 1;

    B2INFO("Projecting for Sensor: " << binLabel << " with Bin Number: " << ij + 1);

    __hEventT0vsCoG__->GetZaxis()->SetRange(ij + 1, ij + 1);
    auto hEventT0vsELS  = (TH2D*)__hEventT0vsCoG__->Project3D("yxe");
    auto hEventT0       = (TH1D*)__hEventT0__->ProjectionX("hEventT0_tmp", ij + 1, ij + 1);
    auto hEventT0nosync = (TH1D*)__hEventT0NoSync__->ProjectionX("hEventT0NoSync_tmp", ij + 1, ij + 1);

    hEventT0vsELS->SetName(Form("eventT0vsCoG__L%dL%dS%d%c", layer_num, ladder_num, sensor_num, side));
    hEventT0->SetName(Form("eventT0__L%dL%dS%d%c", layer_num, ladder_num, sensor_num, side));
    hEventT0nosync->SetName(Form("eventT0nosync__L%dL%dS%d%c", layer_num, ladder_num, sensor_num, side));

    char sidePN = (side == 'U' ? 'P' : 'N');
    hEventT0vsELS->SetTitle(Form("EventT0Sync vs rawTime in %d.%d.%d %c/%c", layer_num, ladder_num, sensor_num, side, sidePN));
    hEventT0->SetTitle(Form("EventT0Sync in %d.%d.%d %c/%c", layer_num, ladder_num, sensor_num, side, sidePN));
    hEventT0nosync->SetTitle(Form("EventT0NoSync in %d.%d.%d %c/%c", layer_num, ladder_num, sensor_num, side, sidePN));

    hEventT0vsELS->SetDirectory(0);
    hEventT0->SetDirectory(0);
    hEventT0nosync->SetDirectory(0);

    B2INFO("Histogram: " << hEventT0vsELS->GetName() <<
           " Entries (n. clusters): " << hEventT0vsELS->GetEntries());
    if (layer_num == 3 && hEventT0vsELS->GetEntries() < m_minEntries) {
      B2INFO("Histogram: " << hEventT0vsELS->GetName() <<
             " Entries (n. clusters): " << hEventT0vsELS->GetEntries() <<
             " Entries required: " << m_minEntries);
      B2WARNING("Not enough data, adding one run to the collector");
      f->Close();
      gSystem->Unlink(Form("algorithm_3SampleELS_output_rev_%d.root", cal_rev));
      return c_NotEnoughData;
    }
    if (layer_num != 3 && hEventT0vsELS->GetEntries() < m_minEntries / 10) {
      B2INFO("Histogram: " << hEventT0vsELS->GetName() <<
             " Entries (n. clusters): " << hEventT0vsELS->GetEntries() <<
             " Entries required: " << m_minEntries / 10);
      B2WARNING("Not enough data, adding one run to the collector");
      f->Close();
      gSystem->Unlink(Form("algorithm_3SampleELS_output_rev_%d.root", cal_rev));
      return c_NotEnoughData;
    }
    for (int i = 1; i <= hEventT0vsELS->GetNbinsX(); i++) {
      for (int j = 1; j <= hEventT0vsELS->GetNbinsY(); j++) {
        if (hEventT0vsELS->GetBinContent(i, j) < max(2, int(hEventT0vsELS->GetEntries() * 0.001))) {
          hEventT0vsELS->SetBinContent(i, j, 0);
        }
      }
    }
    TProfile* pfx = hEventT0vsELS->ProfileX();
    std::string name = "pfx_" + std::string(hEventT0vsELS->GetName());
    pfx->SetName(name.c_str());
    // Remove non-liniarity from the fit by fixing the pole position
    pol1pole->FixParameter(3, 7);
    pfx->Fit("pol1pole", "QMRS");
    pol1pole->SetParLimits(3, 0, 15);
    TFitResultPtr tfr = pfx->Fit("pol1pole", "QMRS");
    // There is small possibility that the chi2 minimization ends at the parameter boundary.
    // In such case, we may get a completely wrong fit function, thus we refit w/o boundaries.
    // if (tfr->Chi2() > 500) {
    //   B2WARNING("Correlation fit Chi2 is too large, refit with very wide parameter boundaries.");
    //   pol1pole->SetParLimits(1, 0, 0);
    //   pol1pole->SetParLimits(2, 0, 0);
    //   pol1pole->SetParLimits(3, 0, 0);
    //   tfr = pfx->Fit("pol1pole", "QMRS");
    //   pol1pole->SetParLimits(1, 0, 1);
    //   pol1pole->SetParLimits(2, -300, 0);
    // }
    double par[4];
    pol1pole->GetParameters(par);
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

    delete pfx;
    delete hEventT0vsELS;
    delete hEventT0;
    delete hEventT0nosync;

    if (tfr.Get() == nullptr || (tfr->Status() != 0 && tfr->Status() != 4 && tfr->Status() != 4000)) {
      f->Close();
      B2FATAL("Fit to the histogram failed in SVD3SampleELSTimeCalibrationAlgorithm. "
              << "Check the 2-D histogram to clarify the reason.");
    } else {
      a = par[0]; b = par[1]; c = par[2]; d = par[3];
      a_err = tfr->ParError(0); b_err = tfr->ParError(1); c_err = tfr->ParError(2); d_err = tfr->ParError(3);
      chi2 = tfr->Chi2();
      ndf  = tfr->Ndf();
      p    = tfr->Prob();
      m_tree->Fill();
    }
  }
  m_tree->Write();
  f->Close();
  saveCalibration(payload, "SVD3SampleELSTimeCalibrations");

  //delete f;

  // probably not needed - would trigger re-doing the collection
  // if ( ... too large corrections ... ) return c_Iterate;
  return c_OK;
}

bool SVD3SampleELSTimeCalibrationAlgorithm::isBoundaryRequired(const Calibration::ExpRun& currentRun)
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
