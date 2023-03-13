/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/calibration/eclAutocovarianceCalibrationC2Algorithm.h>

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
eclAutocovarianceCalibrationC2Algorithm::eclAutocovarianceCalibrationC2Algorithm():
  CalibrationAlgorithm("eclAutocovarianceCalibrationC2Collector")
{
  setDescription(
    "Perform energy calibration of ecl crystals by fitting a Novosibirsk function to energy deposited by photons in e+e- --> gamma gamma"
  );
}

CalibrationAlgorithm::EResult eclAutocovarianceCalibrationC2Algorithm::calibrate()
{


  ///** Put root into batch mode so that we don't try to open a graphics window */
  //gROOT->SetBatch();

  ///**-----------------------------------------------------------------------------------------------*/
  ///** Histograms containing the data collected by eclGammaGammaECollectorModule */
  auto m_BaselineInfoVsCrysID = getObjectPtr<TH2F>("m_BaselineInfoVsCrysID");

  std::vector<float> cryIDs;
  std::vector<float> baselines;

  for (int crysID = 0; crysID < ECLElementNumbers::c_NCrystals; crysID++) {

    float totalCounts = m_BaselineInfoVsCrysID->GetBinContent(m_BaselineInfoVsCrysID->GetBin(crysID + 1, 32));
    float baseline = 0.0;
    for (int i = 0; i < 31; i++) {
      //B2INFO(i<<" "<<m_BaselineInfoVsCrysID->Fill(crysID,i,0)/totalCounts<<" "<<m_BaselineInfoVsCrysID->Fill(crysID,i,0));
      baseline += float(m_BaselineInfoVsCrysID->GetBinContent(m_BaselineInfoVsCrysID->GetBin(crysID + 1, i + 1)));
    }
    baseline /= 31.0;
    baseline /= totalCounts;

    B2INFO(crysID << " baseline: " << baseline);

    cryIDs.push_back(crysID + 1);
    baselines.push_back(baseline);

    B2INFO("eclAutocovarianceCalibrationC2Algorithm crysID baseline totalCounts  " << crysID << " " << baseline << " " << totalCounts);

  }

  auto gBaselineVsCrysID = new TGraph(cryIDs.size(), cryIDs.data(), baselines.data());
  gBaselineVsCrysID->SetName("gBaselineVsCrysID");

  /** Write out the basic histograms in all cases */
  TString fName = m_outputName;
  TFile* histfile = new TFile(fName, "recreate");

  m_BaselineInfoVsCrysID->Write();
  gBaselineVsCrysID->Write();

  ECLCrystalCalib* PPThreshold = new ECLCrystalCalib();
  PPThreshold->setCalibVector(baselines, cryIDs);
  saveCalibration(PPThreshold, "ECLAutocovarianceCalibrationC2Baseline");

  return c_OK;
}
