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
#include <TH2I.h>

using namespace std;
using namespace Belle2;
using namespace ECL;

/**-----------------------------------------------------------------------------------------------*/
eclAutocovarianceCalibrationC1Algorithm::eclAutocovarianceCalibrationC1Algorithm():
  CalibrationAlgorithm("eclAutocovarianceCalibrationC1Collector")
{
  setDescription(
    "Perform energy calibration of ecl crystals by fitting a Novosibirsk function to energy deposited by photons in e+e- --> gamma gamma"
  );

  m_lowestEnergyFraction = 0.75;
}

CalibrationAlgorithm::EResult eclAutocovarianceCalibrationC1Algorithm::calibrate()
{

  /** Put root into batch mode so that we don't try to open a graphics window */
  gROOT->SetBatch();

  /**-----------------------------------------------------------------------------------------------*/
  /** Histograms containing the data collected by eclGammaGammaECollectorModule */
  auto PPVsCrysID = getObjectPtr<TH2I>("PPVsCrysID");

  std::vector<float> cryIDs;
  std::vector<float> PPamps;

  for (int crysID = 0; crysID < ECLElementNumbers::c_NCrystals; crysID++) {

    TH1D* hPP = PPVsCrysID->ProjectionY("hPP", crysID + 1, crysID + 1);

    int Total = hPP->GetEntries();
    int subTotal = 0;
    float fraction = 0.0;
    int counter = 0;

    while (fraction < m_lowestEnergyFraction) {
      subTotal += hPP->GetBinContent(counter);
      fraction = ((float)subTotal) / ((float)Total);
      counter++;
    }

    cryIDs.push_back(crysID + 1);
    PPamps.push_back(counter);

    B2INFO(crysID << " " << counter << " " << fraction);
  }

  auto gPPVsCrysID = new TGraph(cryIDs.size(), cryIDs.data(), PPamps.data());
  gPPVsCrysID->SetName("gPPVsCrysID");

  /** Write out the basic histograms in all cases */
  TString fName = m_outputName;
  TFile* histfile = new TFile(fName, "recreate");

  //histfile->cd();
  PPVsCrysID->Write();
  gPPVsCrysID->Write();
  //histfile->Close();

  ECLCrystalCalib* PPThreshold = new ECLCrystalCalib();
  PPThreshold->setCalibVector(cryIDs, PPamps);
  saveCalibration(PPThreshold, "ECLAutocovarianceCalibrationC1Threshold");
  B2INFO("eclAutocovarianceCalibrationC1Algorithm: successfully stored ECLAutocovarianceCalibrationC1Threshold constants");
//
//  /**-----------------------------------------------------------------------------------------------*/
//  /** Write output to DB if requested and successful */
//  //bool DBsuccess = false;
//  //if (m_storeConst == 0 || (m_storeConst == 1 && allFitsOK)) {
//  //  DBsuccess = true;
//  //  /** Store expected energies */
//  //  std::vector<float> tempE;
//  //  std::vector<float> tempUnc;
//  //  for (int crysID = 0; crysID < ECLElementNumbers::c_NCrystals; crysID++) {
//  //    tempE.push_back(ExpEnergyperCrys->GetBinContent(crysID + 1));
//  //    tempUnc.push_back(ExpEnergyperCrys->GetBinError(crysID + 1));
//  //  }
//  //  ECLCrystalCalib* ExpectedE = new ECLCrystalCalib();
//  //  ExpectedE->setCalibVector(tempE, tempUnc);
//  //  saveCalibration(ExpectedE, "ECLExpGammaGammaE");
//  //  B2INFO("eclCosmicEAlgorithm: successfully stored expected energies ECLExpGammaGammaE");
//  //}
//
//  //hEnergy->Write();
//
//  /**-----------------------------------------------------------------------------------------------*/
//  /** Clean up histograms in case Algorithm is called again */
//  TH2I* dummy;
//  dummy = (TH2I*)gROOT->FindObject("PeakVsCrysID"); delete dummy;
//
//  /**-----------------------------------------------------------------------------------------------*/
//  /** Set the return code appropriately */
//  //if (m_storeConst == -1) {
//  //  B2INFO("eclGammaGammaEAlgorithm performed fits but was not asked to store contants");
//  //  return c_Failure;
//  //}
  return c_OK;
}
