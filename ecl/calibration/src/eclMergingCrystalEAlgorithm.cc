/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <ecl/calibration/eclMergingCrystalEAlgorithm.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include "TH1F.h"
#include "TString.h"
#include "TFile.h"
#include "TDirectory.h"

using namespace std;
using namespace Belle2;
using namespace ECL;
using namespace Calibration;

/**-----------------------------------------------------------------------------------------------*/
eclMergingCrystalEAlgorithm::eclMergingCrystalEAlgorithm(): CalibrationAlgorithm("DummyCollector"),
  m_existingGammaGamma("ECLCrystalEnergyGammaGamma"), m_existingMuMu("ECLCrystalEnergyMuMu"),
  m_existing5x5("ECLCrystalEnergyee5x5"), m_existing(m_payloadName)
{
  setDescription(
    "Perform energy calibration of ecl crystals by combining previous values from the DB for different calibrations."
  );
}

CalibrationAlgorithm::EResult eclMergingCrystalEAlgorithm::calibrate()
{
  //------------------------------------------------------------------------
  // Get the input run list (should be only 1) for us to use to update the DBObjectPtrs
  auto runs = getRunList();
  // Take the first run
  ExpRun chosenRun = runs.front();
  B2INFO("merging using the ExpRun (" << chosenRun.second << "," << chosenRun.first << ")");
  // After here your DBObjPtrs are correct
  updateDBObjPtrs(1, chosenRun.second, chosenRun.first);

  //------------------------------------------------------------------------
  // Test the DBObjects we want to exist and fail if not all of them do.
  bool allObjectsFound = true;
  // Test that the DBObjects are valid
  if (!m_existingGammaGamma) {
    allObjectsFound = false;
    B2ERROR("No valid DBObject found for 'ECLCrystalEnergyGammaGamma'");
  }
  if (!m_existingMuMu) {
    allObjectsFound = false;
    B2ERROR("No valid DBObject found for 'ECLCrystalEnergyMuMu'");
  }
  if (!m_existing5x5) {
    allObjectsFound = false;
    B2ERROR("No valid DBObject found for 'ECLCrystalEnergyee5x5'");
  }
  if (!m_existing) {
    allObjectsFound = false;
    B2ERROR("No valid DBObject found for 'ECLCrystalEnergy'");
  }

  if (allObjectsFound) {
    B2INFO("Valid objects found for both 'ECLCrystalEnergy' and 'ECLCrystalEnergyGammaGamma'");
  } else {
    B2INFO("Exiting with failure");
    return c_Failure;
  }

  //------------------------------------------------------------------------
  /** Get the vectors from the input payloads */
  vector<float> gammaGammaCalib = m_existingGammaGamma->getCalibVector();
  vector<float> gammaGammaCalibUnc = m_existingGammaGamma->getCalibUncVector();

  vector<float> existingCalib = m_existing->getCalibVector();
  vector<float> existingCalibUnc = m_existing->getCalibUncVector();

  //------------------------------------------------------------------------
  /** Calculate new ECLCrystalEnergy from existing payloads */
  //..For now, use Gamma Gamma if available; otherwise, use existing value
  vector<float> newCalib(m_numCrystals);
  vector<float> newCalibUnc(m_numCrystals);

  for (int ic = 0; ic < m_numCrystals; ic++) {
    if (gammaGammaCalib[ic] > 0.) {
      newCalib[ic] = gammaGammaCalib[ic];
      newCalibUnc[ic] = gammaGammaCalibUnc[ic];
    } else {
      newCalib[ic] = existingCalib[ic];
      newCalibUnc[ic] = existingCalibUnc[ic];
    }
  }

  //------------------------------------------------------------------------
  /** Write out a few values for quality control purposes */
  for (int ic = 0; ic < 9000; ic += 1000) {
    B2INFO(ic + 1 << " " << existingCalib[ic] << " " << existingCalibUnc[ic] << " "
           << gammaGammaCalib[ic] << " " << gammaGammaCalibUnc[ic] << " "
           << newCalib[ic] << " " << newCalibUnc[ic]);
  }

  //------------------------------------------------------------------------
  //..Histograms of existing calibration, new calibration, and ratio new/old

  // Just in case, we remember the current TDirectory so we can return to it
  TDirectory* executeDir = gDirectory;

  TString fname = m_payloadName;
  fname += ".root";
  TFile hfile(fname, "recreate");

  TString htitle = m_payloadName;
  htitle += " existing values;cellID";
  TH1F* existingPayload = new TH1F("existingPayload", htitle, m_numCrystals, 1, 8737);

  htitle = m_payloadName;
  htitle += " new values;cellID";
  TH1F* newPayload = new TH1F("newPayload", htitle, m_numCrystals, 1, 8737);

  htitle = m_payloadName;
  htitle += " ratio new/old;cellID";
  TH1F* payloadRatioVsCellID = new TH1F("payloadRatioVsCellID", htitle, m_numCrystals, 1, 8737);

  htitle = m_payloadName;
  htitle += " ratio new/old";
  TH1F* payloadRatio = new TH1F("payloadRatio", htitle, 200, 0.95, 1.05);

  for (int cellID = 1; cellID <= m_numCrystals; cellID++) {
    existingPayload->SetBinContent(cellID, existingCalib[cellID - 1]);
    existingPayload->SetBinError(cellID, existingCalibUnc[cellID - 1]);

    newPayload->SetBinContent(cellID, newCalib[cellID - 1]);
    newPayload->SetBinError(cellID, newCalibUnc[cellID - 1]);

    float ratio = 1.;
    float ratioUnc = 0.;
    if (abs(existingCalib[cellID - 1]) > 1.0e-12) {
      ratio = newCalib[cellID - 1] / existingCalib[cellID - 1];
      float rUnc0 = existingCalibUnc[cellID - 1] / existingCalib[cellID - 1];
      float rUnc1 = 0.;
      if (abs(newCalib[cellID - 1]) > 1.0e-12) {rUnc1 = newCalibUnc[cellID - 1] / newCalib[cellID - 1];}
      ratioUnc = ratio * sqrt(rUnc0 * rUnc0 + rUnc1 * rUnc1);
    }

    payloadRatioVsCellID->SetBinContent(cellID, ratio);
    payloadRatioVsCellID->SetBinError(cellID, ratioUnc);

    payloadRatio->Fill(ratio);
  }
  hfile.cd();
  hfile.Write();
  hfile.Close();
  B2INFO("Debugging histograms written to " << fname);
  // Go back to original TDirectory
  executeDir->cd();

  ECLCrystalCalib* newCrystalEnergy = new ECLCrystalCalib();
  newCrystalEnergy->setCalibVector(newCalib, newCalibUnc);
  saveCalibration(newCrystalEnergy, m_payloadName);
  return c_OK;
}
