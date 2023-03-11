/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/calibration/eclAutocovarianceCalibrationC3Algorithm.h>

/* ECL headers. */
#include <ecl/dataobjects/ECLElementNumbers.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>

/* ROOT headers. */
#include <TFile.h>
#include <TGraph.h>
#include <TH2I.h>

using namespace std;
using namespace Belle2;
using namespace ECL;

/**-----------------------------------------------------------------------------------------------*/
eclAutocovarianceCalibrationC3Algorithm::eclAutocovarianceCalibrationC3Algorithm():
  CalibrationAlgorithm("eclAutocovarianceCalibrationC3Collector")
{
  setDescription(
    "Perform energy calibration of ecl crystals by fitting a Novosibirsk function to energy deposited by photons in e+e- --> gamma gamma"
  );
}

CalibrationAlgorithm::EResult eclAutocovarianceCalibrationC3Algorithm::calibrate()
{


  ///** Put root into batch mode so that we don't try to open a graphics window */
  //gROOT->SetBatch();

  ///**-----------------------------------------------------------------------------------------------*/
  ///** Histograms containing the data collected by eclGammaGammaECollectorModule */
  auto BaselineInfoVsCrysID = getObjectPtr<TH2F>("CovarianceMatrixInfoVsCrysID");

  for (int crysID = 0; crysID < ECLElementNumbers::c_NCrystals; crysID++) {

    float totalCounts = BaselineInfoVsCrysID->GetBinContent(crysID + 1, 32);


    //cryIDs.push_back(crysID + 1);
    //baselines.push_back(baseline);

    B2INFO("eclAutocovarianceCalibrationC3Algorithm crysID totalCounts  " << crysID << " " << totalCounts << " " << totalCounts);

  }

  //auto gBaselineVsCrysID = new TGraph(cryIDs.size(), cryIDs.data(), baselines.data());
  //gBaselineVsCrysID->SetName("gBaselineVsCrysID");

  /** Write out the basic histograms in all cases */
  TString fName = m_outputName;
  TFile* histfile = new TFile(fName, "recreate");

  BaselineInfoVsCrysID->Write();
  //gBaselineVsCrysID->Write();

  //ECLCrystalCalib* PPThreshold = new ECLCrystalCalib();
  //PPThreshold->setCalibVector(cryIDs, baselines);
  //saveCalibration(PPThreshold, "ECLAutocovarianceCalibrationC3Baseline");

  return c_OK;
}
