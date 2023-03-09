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

  m_lowestEnergyFraction = 0.75;
}

CalibrationAlgorithm::EResult eclAutocovarianceCalibrationC2Algorithm::calibrate()
{


  ///** Put root into batch mode so that we don't try to open a graphics window */
  //gROOT->SetBatch();

  ///**-----------------------------------------------------------------------------------------------*/
  ///** Histograms containing the data collected by eclGammaGammaECollectorModule */
  auto BaselineVsCrysID = getObjectPtr<TH1F>("BaselineVsCrysID");

  std::vector<float> cryIDs;
  std::vector<float> PPamps;

  for (int crysID = 0; crysID < ECLElementNumbers::c_NCrystals; crysID++) {

    float baseline = BaselineVsCrysID->GetBinContent(crysID + 1);
    B2INFO(crysID << " " << baseline);

  }

  //  TH1D* hPP = PPVsCrysID->ProjectionY("hPP", crysID + 1, crysID + 1);

  //  int Total = hPP->GetEntries();
  //  int subTotal = 0;
  //  float fraction = 0.0;
  //  int counter = 0;

  //  while (fraction < m_lowestEnergyFraction) {
  //    subTotal += hPP->GetBinContent(counter);
  //    fraction = ((float)subTotal) / ((float)Total);
  //    counter++;
  //  }

  //  cryIDs.push_back(crysID + 1);
  //  PPamps.push_back(counter);

  //  B2INFO(crysID << " " << counter << " " << fraction);
  //}

  //auto gPPVsCrysID = new TGraph(cryIDs.size(), cryIDs.data(), PPamps.data());
  //gPPVsCrysID->SetName("gPPVsCrysID");

  ///** Write out the basic histograms in all cases */
  //TString fName = m_outputName;
  //TFile* histfile = new TFile(fName, "recreate");

  ////histfile->cd();
  //PPVsCrysID->Write();
  //gPPVsCrysID->Write();
  ////histfile->Close();

  //ECLCrystalCalib* PPThreshold = new ECLCrystalCalib();
  //PPThreshold->setCalibVector(cryIDs, PPamps);
  //saveCalibration(PPThreshold, "ECLAutocovarianceCalibrationC2Threshold");
  //B2INFO("eclAutocovarianceCalibrationC2Algorithm: successfully stored ECLAutocovarianceCalibrationC2Threshold constants");

  return c_OK;
}
