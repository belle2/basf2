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

  TF1* pol = new TF1("pol", "[0] + [1]*x + [2]*x*x + [3]*x*x*x", -50, 50);
  pol->SetParameters(-50, 1.5, 0.001, 0.00001);
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

  for (int layer = 0; layer < 4; layer++) {
    layer_num = layer + 3;
    for (int ladder = 0; ladder < (int)ladderOfLayer[layer]; ladder++) {
      ladder_num = ladder + 1;
      for (int sensor = 0; sensor < (int)sensorOnLayer[layer]; sensor++) {
        sensor_num = sensor + 1;
        for (int view  = 0; view < 2; view++) {
          char side = 'U';
          if (view == 0)
            side = 'V';
          auto hEventT0vsCoG = getObjectPtr<TH2F>(Form("eventT0vsCoG__L%dL%dS%d%c", layer_num, ladder_num, sensor_num, side));
          auto hEventT0 = getObjectPtr<TH1F>(Form("eventT0__L%dL%dS%d%c", layer_num, ladder_num, sensor_num, side));
          cout << " " << endl;
          cout << typeid(hEventT0vsCoG).name() << " " << hEventT0vsCoG->GetName() << " " << hEventT0vsCoG->GetEntries() << endl;
          if (layer_num == 3 && hEventT0vsCoG->GetEntries() < 40000) {
            cout << " " << endl;
            cout << hEventT0vsCoG->GetName() << " " << hEventT0vsCoG->GetEntries() << endl;
            cout << "Not enough data, adding one run to the collector" << endl;
            return c_NotEnoughData;
          }
          cout << " " << endl;
          TProfile* pfx = hEventT0vsCoG->ProfileX();
          std::string name = "pfx_" + std::string(hEventT0vsCoG->GetName());
          pfx->SetName(name.c_str());
          pfx->Fit("pol", "Q0");
          double par[4];
          pol->GetParameters(par);
          double meanT0 = hEventT0->GetMean();
          timeCal->set_current(1);
          timeCal->set_pol3parameters(par[0] - meanT0, par[1], par[2], par[3]); // par[0] - meanT0

          payload->set(layer_num, ladder_num, sensor_num, bool(view), 1, *timeCal);

        }
      }
    }
  }
  saveCalibration(payload, "SVDCoGTimeCalibrations");

  // probably not needed - would trigger re-doing the collection
  // if ( ... too large corrections ... ) return c_Iterate;
  return c_OK;
}
