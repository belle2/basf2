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
#include <ecl/dbobjects/ECLAutoCovariance.h>

/* ROOT headers. */
#include <TFile.h>
#include <TGraph.h>
#include <TH2I.h>
#include"TMatrixDSym.h"
#include"TDecompChol.h"

using namespace Belle2;
using namespace ECL;

/**-----------------------------------------------------------------------------------------------*/
eclAutocovarianceCalibrationC3Algorithm::eclAutocovarianceCalibrationC3Algorithm():
  CalibrationAlgorithm("eclAutocovarianceCalibrationC3Collector")
{
  setDescription(
    "Computes the covariance matrix for each crystal"
  );
}

CalibrationAlgorithm::EResult eclAutocovarianceCalibrationC3Algorithm::calibrate()
{


  ///** Put root into batch mode so that we don't try to open a graphics window */
  gROOT->SetBatch();

  ///** vectors to store information for output root file */
  std::vector<double> cryIDs;
  std::vector<double> noiseMatrix00Vector;
  std::vector<double> totalCountsVector;
  std::vector<double> invertAttempts;

  ///**-----------------------------------------------------------------------------------------------*/
  ///** Histogram containing the data collected by eclAutocovarianceCalibrationC3Collector*/
  auto CovarianceMatrixInfoVsCrysID = getObjectPtr<TH2F>("CovarianceMatrixInfoVsCrysID");

  ECLAutoCovariance* Autocovariances = new ECLAutoCovariance();

  for (int ID = 0; ID < ECLElementNumbers::c_NCrystals; ID++) {

    double totalCounts = CovarianceMatrixInfoVsCrysID->GetBinContent(CovarianceMatrixInfoVsCrysID->GetBin(ID + 1,
                         m_numberofADCPoints + 1));

    if (totalCounts < m_TotalCountsThreshold) {
      B2INFO("eclAutocovarianceCalibrationC3Algorithm: warning total entries for cell ID " << ID + 1 << " is only: " << totalCounts <<
             " Requirement is m_TotalCountsThreshold: " << m_TotalCountsThreshold);
      /** We require all crystals to have a minimum number of waveforms available.  If c_NotEnoughData is returned then the next run will be appended.  */
      return c_NotEnoughData;
    }

    TMatrixDSym NoiseMatrix;
    NoiseMatrix.ResizeTo(m_numberofADCPoints, m_numberofADCPoints);
    for (int i = 0; i < m_numberofADCPoints; i++) {
      for (int j = 0; j < m_numberofADCPoints; j++) {
        int index = abs(i - j);
        NoiseMatrix(i, j) = double(CovarianceMatrixInfoVsCrysID->GetBinContent(CovarianceMatrixInfoVsCrysID->GetBin(ID + 1,
                                   index + 1))) / (totalCounts - 1) / (double(m_numberofADCPoints - index));
      }
    }

    TMatrixDSym NoiseMatrixReduced(m_numberofADCPoints);
    for (int i = 0; i < m_numberofADCPoints; i++) {
      for (int j = 0; j < m_numberofADCPoints; j++) {
        NoiseMatrixReduced(i, j) = (NoiseMatrix(0, abs(i - j)));
      }
    }

    bool invert_successful = 0;
    int invert_attempt = 0;
    double tempAutoCov[m_numberofADCPoints];
    for (int i = 0; i < m_numberofADCPoints; i++) tempAutoCov[i] = NoiseMatrixReduced(0, i);
    std::vector<double> buf(m_numberofADCPoints);
    while (invert_successful == 0) {

      Autocovariances->setAutoCovariance(ID + 1, tempAutoCov);
      Autocovariances->getAutoCovariance(ID + 1, buf.data());

      TMatrixDSym NoiseMatrix_check(m_numberofADCPoints);
      for (int i = 0; i < m_numberofADCPoints; i++) {
        for (int j = 0; j < m_numberofADCPoints; j++) {
          NoiseMatrix_check(i, j) = buf[abs(i - j)];
        }
      }

      TDecompChol dc(NoiseMatrix_check);
      invert_successful = dc.Invert(NoiseMatrix_check);
      if (invert_successful == 0) {

        if (invert_attempt > 4) {
          B2INFO("eclAutocovarianceCalibrationC3Algorithm iD " << ID << " invert_attempt limit reached " << invert_attempt);
          B2INFO("eclAutocovarianceCalibrationC3Algorithm setting m_u2 to zero");
          m_u2 = 0.0;
        }

        B2INFO("eclAutocovarianceCalibrationC3Algorithm iD " << ID << " invert_attempt " << invert_attempt);

        for (int i = 0; i < m_numberofADCPoints; i++) B2INFO("old[" << i << "] = " <<  tempAutoCov[i]);
        for (int i = 1; i < m_numberofADCPoints; i++) tempAutoCov[i] *= (m_u2 / (1. + exp((i - m_u0) / m_u1)));
        for (int i = 0; i < m_numberofADCPoints; i++) B2INFO("new[" << i << "] = " <<  tempAutoCov[i]);

      }
      invert_attempt++;
    }

    cryIDs.push_back(ID + 1);
    noiseMatrix00Vector.push_back(tempAutoCov[0]);
    totalCountsVector.push_back(totalCounts);
    invertAttempts.push_back(invert_attempt);

  }

  /** Saving Calibration Results */
  saveCalibration(Autocovariances, "ECLAutoCovariance");

  /** Preparing TGraphs for output file */
  auto gnoiseMatrix00Vector = new TGraph(cryIDs.size(), cryIDs.data(), noiseMatrix00Vector.data());
  gnoiseMatrix00Vector->SetName("gnoiseMatrix00Vector");
  gnoiseMatrix00Vector->SetMarkerStyle(20);
  auto gtotalCountsVector = new TGraph(cryIDs.size(), cryIDs.data(), totalCountsVector.data());
  gtotalCountsVector->SetName("gtotalCountsVector");
  gtotalCountsVector->SetMarkerStyle(20);
  auto ginvertAttempts = new TGraph(cryIDs.size(), cryIDs.data(), invertAttempts.data());
  ginvertAttempts->SetName("ginvertAttempts");
  ginvertAttempts->SetMarkerStyle(20);

  /** Write out the basic histograms in all cases */
  TString fName = m_outputName;
  TDirectory::TContext context;
  TFile* histfile = new TFile(fName, "recreate");
  histfile->cd();
  gnoiseMatrix00Vector->Write();
  gtotalCountsVector->Write();
  ginvertAttempts->Write();
  histfile->Close();
  delete histfile;

  return c_OK;
}
