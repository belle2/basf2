/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/calibration/eclWaveformTemplateCalibrationC1Algorithm.h>

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
eclWaveformTemplateCalibrationC1Algorithm::eclWaveformTemplateCalibrationC1Algorithm():
  CalibrationAlgorithm("eclWaveformTemplateCalibrationC1Collector")
{
  setDescription(
    "Used to determine the baseline noise level of crystals in e+e- --> gamma gamma"
  );
}

CalibrationAlgorithm::EResult eclWaveformTemplateCalibrationC1Algorithm::calibrate()
{

  /** Put root into batch mode so that we don't try to open a graphics window */
  gROOT->SetBatch();

  /**-----------------------------------------------------------------------------------------------*/
  /** Histograms containing the data collected by eclGammaGammaECollectorModule */
  auto varXvsCrysID = getObjectPtr<TH2F>("varXvsCrysID");

  std::vector<float> cryIDs;
  std::vector<float> Varxs;
  std::vector<float> Counts;

  for (int id = 0; id < ECLElementNumbers::c_NCrystals; id++) {

    TH1F* hVarx = (TH1F*)varXvsCrysID->ProjectionY("hVarx", id + 1, id + 1);

    int Total = hVarx->GetEntries();
    int subTotal = 0;
    float fraction = 0.0;
    int counter = 0;

    while (fraction < m_lowestEnergyFraction) {
      subTotal += hVarx->GetBinContent(counter);
      fraction = ((float)subTotal) / ((float)Total);
      counter++;
    }

    cryIDs.push_back(id + 1);
    Varxs.push_back(counter);
    Counts.push_back(Total);

    B2INFO("eclWaveformTemplateCalibrationC1Algorithm: id counter fraction Total " << id << " " << counter << " " << fraction <<
           " " << Total);

    hVarx->Delete();
  }

  auto gvarXvsCrysID = new TGraph(cryIDs.size(), cryIDs.data(), Varxs.data());
  gvarXvsCrysID->SetName("gvarXvsCrysID");
  auto gTotalvsCrysID = new TGraph(cryIDs.size(), cryIDs.data(), Counts.data());
  gTotalvsCrysID->SetName("gTotalvsCrysID");

  /** Write out the basic histograms in all cases */
  TString fName = m_outputName;
  TFile* histfile = new TFile(fName, "recreate");

  //histfile->cd();
  gvarXvsCrysID->Write();
  varXvsCrysID->Write();
  gTotalvsCrysID->Write();
  //histfile->Close();

  ECLCrystalCalib* PPThreshold = new ECLCrystalCalib();
  PPThreshold->setCalibVector(Varxs, cryIDs);
  saveCalibration(PPThreshold, "eclWaveformTemplateCalibrationC1VarSq");
  B2INFO("eclWaveformTemplateCalibrationC1Algorithm: successfully stored ECLAutocovarianceCalibrationC1Threshold constants");

  return c_OK;
}
