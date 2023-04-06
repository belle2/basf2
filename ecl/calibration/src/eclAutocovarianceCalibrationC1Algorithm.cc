/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/calibration/eclAutocovarianceCalibrationC1Algorithm.h>

/* ECL headers. */
#include <ecl/dataobjects/ECLElementNumbers.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>

/* ROOT headers. */
#include <TFile.h>
#include <TGraph.h>
#include <TH2F.h>

using namespace std;
using namespace Belle2;
using namespace ECL;

/**-----------------------------------------------------------------------------------------------*/
eclAutocovarianceCalibrationC1Algorithm::eclAutocovarianceCalibrationC1Algorithm():
  CalibrationAlgorithm("eclAutocovarianceCalibrationC1Collector")
{
  setDescription(
    "Determine noise threshold for waveforms to be used in computing the coveriance matrix"
  );
}

CalibrationAlgorithm::EResult eclAutocovarianceCalibrationC1Algorithm::calibrate()
{

  /** Put root into batch mode so that we don't try to open a graphics window */
  gROOT->SetBatch();

  /**-----------------------------------------------------------------------------------------------*/
  /** Histograms containing the data collected by eclAutocovarianceCalibrationC1Collector*/
  auto PPVsCrysID = getObjectPtr<TH2F>("PPVsCrysID");

  std::vector<float> cellIDs;
  std::vector<float> PPamps;

  std::vector<int> cellIDsWithLowEntries;

  for (int crysID = 0; crysID < ECLElementNumbers::c_NCrystals; crysID++) {

    TH1F* hPP = (TH1F*)PPVsCrysID->ProjectionY("hPP", crysID + 1, crysID + 1);

    int Total = hPP->GetEntries();
    int subTotal = 0;
    float fraction = 0.0;
    int counter = 0;

    while (fraction < m_lowestEnergyFraction) {
      subTotal += hPP->GetBinContent(counter);
      fraction = ((float)subTotal) / ((float)Total);
      counter++;
    }

    cellIDs.push_back(crysID + 1);
    PPamps.push_back(counter);

    B2INFO("eclAutocovarianceCalibrationC1Algorithm: crysID counter fraction Total " << crysID << " " << counter << " " << fraction <<
           " " << Total);

    if (Total < 100) {
      B2INFO("eclAutocovarianceCalibrationC1Algorithm: warning total entries is only: " << Total);
      cellIDsWithLowEntries.push_back(crysID + 1);
    }
  }

  if (cellIDsWithLowEntries.size() > 10) {

    B2INFO("eclAutocovarianceCalibrationC1Algorithm: The following Cell ID's did not have enough entries:");
    for (int i = 0; i < cellIDsWithLowEntries.size(); i++)  B2INFO("Cell ID: " << cellIDsWithLowEntries[i]);
    B2INFO("eclAutocovarianceCalibrationC1Algorithm will return c_NotEnoughData");
    return c_NotEnoughData;

  }

  /** Write out the noise threshold vs Celltal ID*/
  auto gPPVsCellID = new TGraph(cellIDs.size(), cellIDs.data(), PPamps.data());
  gPPVsCellID->SetName("gPPVsCellID");

  TString fName = m_outputName;
  TFile* histfile = new TFile(fName, "recreate");
  PPVsCrysID->Write();
  gPPVsCellID->Write();

  /**-----------------------------------------------------------------------------------------------*/
  /** Write output to DB */
  ECLCrystalCalib* PPThreshold = new ECLCrystalCalib();
  PPThreshold->setCalibVector(PPamps, cellIDs);
  saveCalibration(PPThreshold, "ECLAutocovarianceCalibrationC1Threshold");
  B2INFO("eclAutocovarianceCalibrationC1Algorithm: successfully stored ECLAutocovarianceCalibrationC1Threshold constants");

  return c_OK;
}
