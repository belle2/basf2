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
  /** Histograms containing the data collected by eclWaveformTemplateCalibrationC1Collector */
  auto maxResvsCrysID = getObjectPtr<TH2F>("maxResvsCrysID");

  std::vector<float> cellIDs;
  std::vector<float> maxResiduals;
  std::vector<float> Counts;
  std::vector<float> maxResidualsError(ECLElementNumbers::c_NCrystals);

  for (int id = 0; id < ECLElementNumbers::c_NCrystals; id++) {

    TH1F* hMaxResx = (TH1F*)maxResvsCrysID->ProjectionY("hMaxResx", id + 1, id + 1);

    int Total = hMaxResx->GetEntries();

    if (Total < m_minWaveformLimit) {
      B2INFO("eclWaveformTemplateCalibrationC1Algorithm: warning total entries for cell ID " << id + 1 << " is only: " << Total <<
             " Requirement is : " << m_minWaveformLimit);
      /** We require all crystals to have a minimum number of waveforms available.  If c_NotEnoughData is returned then the next run will be appended.  */
      return c_NotEnoughData;
    }

    int subTotal = 0;
    float fraction = 0.0;
    int counter = 0;

    float fractionLimit = m_fractionLimitGeneral;

    /** If number of waveforms is low use most of them */
    if (Total < m_LowCountThreshold) fractionLimit = m_fractionLimitLowCounts;


    /** determining the threshold needed to select corresponding fraction of lowest noise waveforms  */
    while (fraction < fractionLimit) {
      subTotal += hMaxResx->GetBinContent(counter);
      fraction = ((float)subTotal) / ((float)Total);
      counter++;
    }

    cellIDs.push_back(id + 1);
    maxResiduals.push_back(hMaxResx->GetBinLowEdge(counter + 1));
    Counts.push_back(Total);

    B2INFO("eclWaveformTemplateCalibrationC1Algorithm: id counter fraction Total maxResiduals[id]" << id << " " << counter << " " <<
           fraction <<
           " " << Total << " " << maxResiduals[id]);

    hMaxResx->Delete();
  }

  /** Saving thresholds to database */
  ECLCrystalCalib* PPThreshold = new ECLCrystalCalib();
  PPThreshold->setCalibVector(maxResiduals, maxResidualsError);
  saveCalibration(PPThreshold, "eclWaveformTemplateCalibrationC1MaxResLimit");
  B2INFO("eclWaveformTemplateCalibrationC1Algorithm: successfully stored ECLAutocovarianceCalibrationC1Threshold constants");

  /** Write out the basic histograms in all cases */
  auto gmaxResvsCellID = new TGraph(cellIDs.size(), cellIDs.data(), maxResiduals.data());
  gmaxResvsCellID->SetName("gmaxResvsCellID");
  auto gTotalvsCellID = new TGraph(cellIDs.size(), cellIDs.data(), Counts.data());
  gTotalvsCellID->SetName("gTotalvsCellID");

  TString fName = m_outputName;
  TFile* histfile = new TFile(fName, "recreate");
  histfile->cd();
  gmaxResvsCellID->Write();
  gTotalvsCellID->Write();
  histfile->Close();
  delete histfile;

  return c_OK;
}
