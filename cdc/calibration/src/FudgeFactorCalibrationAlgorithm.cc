/**************************************************************************
- * basf2 (Belle II Analysis Software Framework)                           *
- * Author: The Belle II Collaboration                                     *
- *                                                                        *
- * See git log for contributors and copyright holders.                    *
- * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
- **************************************************************************/
#include <cdc/calibration/FudgeFactorCalibrationAlgorithm.h>
#include <calibration/CalibrationAlgorithm.h>
#include <TError.h>
#include <TROOT.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TFile.h>
#include <TTree.h>
#include <TStopwatch.h>

#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/logging/Logger.h>

//#include "getSigma68WithUncertainty.h"

using namespace std;
using namespace Belle2;
using namespace CDC;

FudgeFactorCalibrationAlgorithm::FudgeFactorCalibrationAlgorithm(): CalibrationAlgorithm("CDCFudgeFactorCalibrationCollector")
{

  setDescription(
    " -------------------------- T0 Calibration Algorithm -------------------------\n"
  );
}

CalibrationAlgorithm::EResult FudgeFactorCalibrationAlgorithm::calibrate()
{
  B2INFO("Start calibration");

  gROOT->SetBatch(1);
  gErrorIgnoreLevel = 3001;
  // We are potentially using data from several runs at once during execution
  // (which may have different DBObject values). So in general you would need to
  // average them, or aply them to the correct collector data.

  // However since this is the geometry lets assume it is fixed for now.
  const auto exprun = getRunList()[0];
  B2INFO("ExpRun used for DB Geometry : " << exprun.first << " " << exprun.second);
  updateDBObjPtrs(1, exprun.second, exprun.first);

  // CDCGeometryPar basically constructs a ton of objects and other DB objects.
  // Normally we'd call updateDBObjPtrs to set the values of the requested DB objects.
  // But in CDCGeometryPar the DB objects get used during the constructor so they must
  // be set before/during the constructor.

  //reading data from rootfile
  auto tree = getObjectPtr<TTree>("tree");
  auto hEvtT0 =   getObjectPtr<TH1F>("hEventT0");
  auto hNDF_pos =   getObjectPtr<TH1F>("hNDF_pos");
  auto hNDF_neg =   getObjectPtr<TH1F>("hNDF_neg");
  auto hPval_pos =   getObjectPtr<TH1F>("hPval_pos");
  auto hPval_neg =   getObjectPtr<TH1F>("hPval_neg");

  auto hnCDC_pos =   getObjectPtr<TH1F>("hnCDC_pos");
  auto hnCDC_neg =   getObjectPtr<TH1F>("hnCDC_neg");


  auto hdPt =   getObjectPtr<TH1F>("hdPt");
  auto hdD0 =   getObjectPtr<TH1F>("hdD0");
  auto hdZ0 =   getObjectPtr<TH1F>("hdZ0");

  auto hdPt_cm = getObjectPtr<TH1F>("hdPt_cm");
  auto hdPtPt_cm = getObjectPtr<TH2F>("hdPtPt_cm");
  auto hdTheta_cm =   getObjectPtr<TH1F>("hdTheta_cm");
  auto hdPhi0_cm =   getObjectPtr<TH1F>("hdPhi0_cm");


  B2INFO("Storing histograms");
  TFile* fout = new TFile(m_histName.c_str(), "RECREATE");
  fout->cd();
  //  TDirectory* top = gDirectory;
  //  if(tree) tree->Write();
  if (hEvtT0) hEvtT0->Write();

  if (hPval_pos) hPval_pos->Write();
  if (hPval_neg) hPval_neg->Write();

  if (hNDF_pos) hNDF_pos->Write();
  if (hNDF_neg) hNDF_neg->Write();

  if (hnCDC_pos) hnCDC_pos->Write();
  if (hnCDC_neg) hnCDC_neg->Write();

  if (hdPt) hdPt->Write();
  if (hdD0) hdD0->Write();
  if (hdZ0) hdZ0->Write();

  if (hdPt_cm) hdPt_cm->Write();
  if (hdPtPt_cm) hdPtPt_cm->Write();
  if (hdPhi0_cm) hdPhi0_cm->Write();
  if (hdTheta_cm) hdTheta_cm->Write();
  TTree* newtree = tree->CloneTree();
  fout->Write();
  fout->Close();
  return c_OK;
}
