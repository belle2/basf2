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

SVDCoGTimeCalibrationAlgorithm::SVDCoGTimeCalibrationAlgorithm(std::string str) :
  CalibrationAlgorithm("SVDCoGTimeCalibrationCollector")
{
  setDescription("SVDCoGTimeCalibration calibration algorithm");
  m_id = str;
}

CalibrationAlgorithm::EResult SVDCoGTimeCalibrationAlgorithm::calibrate()
{

  int layer_num = 0;
  int ladder_num = 0;
  int sensor_num = 0;

  int ladderOfLayer[4] = {7, 10, 12, 16};
  int sensorOnLayer[4] = {2, 3, 4, 5};

  auto timeCal = new Belle2::SVDCoGCalibrationFunction();
  auto payload = new Belle2::SVDCoGTimeCalibrations::t_payload(*timeCal, m_id);

  TF1* pol3 = new TF1("pol3", "[0] + [1]*x + [2]*x*x + [3]*x*x*x", -50, 80);
  pol3->SetParameters(-40, 0.5, 0.05, 0.0005);
  TF1* pol5 = new TF1("pol5", "[0] + [1]*x + [2]*x*x + [3]*x*x*x + [4]*x*x*x*x + [5]*x*x*x*x*x", -100, 100);
  pol5->SetParameters(-50, 1.5, 0.01, 0.0001, 0.00001, 0.000001);

  TH1F* par0U = new TH1F("par0U", " ", 100, -50, 0);
  TH1F* par1U = new TH1F("par1U", " ", 100, -2, 2);
  TH1F* par2U = new TH1F("par2U", " ", 100, -0.5, 0.5);
  TH1F* par3U = new TH1F("par3U", " ", 100, -0.01, 0.01);

  TH1F* par0V = new TH1F("par0V", " ", 100, -50, 50);
  TH1F* par1V = new TH1F("par1V", " ", 100, -10, 10);
  TH1F* par2V = new TH1F("par2V", " ", 150, -0.5, 1);
  TH1F* par3V = new TH1F("par3V", " ", 100, -0.02, 0.01);
  /*
  VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();

  cout << "STARTING GEOMETRY CICLE" << endl;

  for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD)) {
    layer_num =  layer.getLayerNumber();
    cout << layer_num << endl;
    for (auto ladder : geoCache.getLadders(layer)) {
      ladder_num = ladder.getLadderNumber();
      for (Belle2::VxdID sensor :  geoCache.getSensors(ladder)) {
        sensor_num = sensor.getSensorNumber();
        for (int view = SVDHistograms<TH2F>::VIndex ; view < SVDHistograms<TH2F>::UIndex + 1; view++) {*/

  TFile* f = new TFile("algorithm_output.root", "RECREATE");
  for (int layer = 0; layer < 4; layer++) {
    layer_num = layer + 3;
    for (int ladder = 0; ladder < (int)ladderOfLayer[layer]; ladder++) {
      ladder_num = ladder + 1;
      for (int sensor = 0; sensor < (int)sensorOnLayer[layer]; sensor++) {
        sensor_num = sensor + 1;
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
            cout << hEventT0vsCoG->GetName() << " " << hEventT0vsCoG->GetEntries() << endl;
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
          double meanT0 = hEventT0->GetMean();
          double meanT0NoSync = hEventT0nosync->GetMean();
          timeCal->set_current(1);
          // timeCal->set_current(2);
          timeCal->set_pol3parameters(par[0], par[1], par[2], par[3]);
          if (view == 1) {
            par0U->Fill(par[0]);
            par1U->Fill(par[1]);
            par2U->Fill(par[2]);
            par3U->Fill(par[3]);
          } else {
            par0V->Fill(par[0]);
            par1V->Fill(par[1]);
            par2V->Fill(par[2]);
            par3V->Fill(par[3]);
          }
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
  par0U->Write();
  par1U->Write();
  par2U->Write();
  par3U->Write();
  par0V->Write();
  par1V->Write();
  par2V->Write();
  par3V->Write();
  f->Close();
  saveCalibration(payload, "SVDCoGTimeCalibrations");
  delete par0U;
  delete par1U;
  delete par2U;
  delete par3U;
  delete par0V;
  delete par1V;
  delete par2V;
  delete par3V;

  // probably not needed - would trigger re-doing the collection
  // if ( ... too large corrections ... ) return c_Iterate;
  return c_OK;
  delete f;
}
