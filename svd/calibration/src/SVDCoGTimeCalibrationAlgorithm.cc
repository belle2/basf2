/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Gaetano de Marino, Tadeas Bilka                          *
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

SVDCoGTimeCalibrationAlgorithm::SVDCoGTimeCalibrationAlgorithm() : CalibrationAlgorithm("SVDCoGTimeCalibrationCollector")
{
  setDescription("SVDCoGTimeCalibration calibration algorithm");
}

CalibrationAlgorithm::EResult SVDCoGTimeCalibrationAlgorithm::calibrate()
{

  auto timeCal = new Belle2::SVDCoGCalibrationFunction();
  auto payload = new Belle2::SVDCoGTimeCalibrations::t_payload(*timeCal, "SVDCoGTimeCalibrationCAF");

  TF1* pol = new TF1("pol", "[0] + [1]*x + [2]*x*x + [3]*x*x*x", -150, 150);
  pol->SetParameters(-50, 1.5, 0.001, 0.00001);

  TH2F* hEventT0vsCoG = new TH2F(" ", " ", 300, -150, 150, 300, -150, 150);
  //TProfile* pfx = new TProfile("hprof", " ", 300, -150, 150);

  TFile* f = new TFile("profileOutput.root", "RECREATE");

  auto tree = getObjectPtr<TTree>("HTreeCoGTimeCalib");

  if (!tree) {
    B2WARNING("No tree object.");
  } else if (!tree->GetEntries()) {
    B2WARNING("No data in the tree.");
  }

  int layer = 0;
  int ladder = 0;
  int sensor = 0;
  int side = 0;

  tree->SetBranchAddress("hist", &hEventT0vsCoG);
  tree->SetBranchAddress("layer", &layer);
  tree->SetBranchAddress("ladder", &ladder);
  tree->SetBranchAddress("sensor", &sensor);
  tree->SetBranchAddress("view", &side);

  for (int i = 0; i < tree->GetEntries(); i++) {
    tree->GetEntry(i);
    TProfile* pfx = hEventT0vsCoG->ProfileX();
    std::string name = "pfx_" + std::string(hEventT0vsCoG->GetName());
    pfx->SetName(name.c_str());
    pfx->Fit("pol", "0");
    double par[4];
    pol->GetParameters(par);
    timeCal->set_current(1);
    timeCal->set_pol3parameters(par[0], par[1], par[2], par[3]);

    payload->set(layer, ladder, sensor, bool(side), 1, *timeCal);
    f->cd();
    pfx->Write();
    hEventT0vsCoG->Clear();
  }
  f->Close();
  saveCalibration(payload, "SVDCoGTimeCalibrations");

  // probably not needed - would trigger re-doing the collection
  //if ( ... too large corrections ... ) return c_Iterate;
  return c_OK;
}
