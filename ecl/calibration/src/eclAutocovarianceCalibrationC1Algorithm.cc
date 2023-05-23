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
    "Determine noise threshold for waveforms to be used in computing the covariance matrix"
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
  std::vector<float> PPampsErrorVector;

  std::vector<int> cellIDsWithLowEntries;

  for (int crysID = 0; crysID < ECLElementNumbers::c_NCrystals; crysID++) {

    TH1F* hPP = (TH1F*)PPVsCrysID->ProjectionY("hPP", crysID + 1, crysID + 1);

    int Total = hPP->GetEntries();
    int subTotal = 0;
    float fraction = 0.0;
    int counter = 0;

    if (Total < m_TotalCountsThreshold) {
      B2INFO("eclAutocovarianceCalibrationC1Algorithm: warning total entries for cell ID " << crysID + 1 << " is only: " << Total <<
             " Requirement is m_TotalCountsThreshold: " << m_TotalCountsThreshold);
      counter = -1;
      /** We require all crystals to have a minimum number of waveforms available.  If c_NotEnoughData is returned then the next run will be appended.  */
      return c_NotEnoughData;
    } else {
      while (fraction < m_lowestEnergyFraction) {
        subTotal += hPP->GetBinContent(counter);
        fraction = ((float)subTotal) / ((float)Total);
        counter++;
      }
    }

    cellIDs.push_back(crysID + 1);
    PPamps.push_back(counter);
    PPampsErrorVector.push_back(0);

    B2INFO("eclAutocovarianceCalibrationC1Algorithm: crysID counter fraction Total " << crysID << " " << counter << " " << fraction <<
           " " << Total);

  }

  /** Write out the noise threshold vs Cell ID*/
  auto gPPVsCellID = new TGraph(cellIDs.size(), cellIDs.data(), PPamps.data());
  gPPVsCellID->SetName("gPPVsCellID");

  TString fName = m_outputName;
  TFile* histfile = new TFile(fName, "recreate");
  histfile->cd();
  PPVsCrysID->Write();
  gPPVsCellID->Write();

  /**-----------------------------------------------------------------------------------------------*/
  /** Write output to DB */
  ECLCrystalCalib* PPThreshold = new ECLCrystalCalib();
  PPThreshold->setCalibVector(PPamps, PPampsErrorVector);
  saveCalibration(PPThreshold, "ECLAutocovarianceCalibrationC1Threshold");
  B2INFO("eclAutocovarianceCalibrationC1Algorithm: successfully stored ECLAutocovarianceCalibrationC1Threshold constants");

  return c_OK;
}
