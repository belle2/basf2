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

#include <framework/database/DBImportObjPtr.h>
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
  std::vector<double> invertStatusVector;
  std::vector<double> noiseMatrix00Vector;
  std::vector<double> totalCountsVector;

  ///**-----------------------------------------------------------------------------------------------*/
  ///** Histogram containing the data collected by eclAutocovarianceCalibrationC3Collector*/
  auto CovarianceMatrixInfoVsCrysID = getObjectPtr<TH2F>("CovarianceMatrixInfoVsCrysID");

  ECLAutoCovariance* Autocovariances = new ECLAutoCovariance();

  for (int ID = 0; ID < ECLElementNumbers::c_NCrystals; ID++) {

    float totalCounts = CovarianceMatrixInfoVsCrysID->GetBinContent(CovarianceMatrixInfoVsCrysID->GetBin(ID + 1,
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
        NoiseMatrix(i, j) = float(CovarianceMatrixInfoVsCrysID->GetBinContent(CovarianceMatrixInfoVsCrysID->GetBin(ID + 1,
                                  index + 1))) / (totalCounts - 1) / (float(m_numberofADCPoints - index));
      }
    }

    double tempAutoCov[m_numberofADCPoints];

    for (int i = 0; i < m_numberofADCPoints; i++) tempAutoCov[i] = NoiseMatrix(0, i);
    Autocovariances->setAutoCovariance(ID + 1, tempAutoCov);

    TDecompChol dc(NoiseMatrix);
    bool InvertStatus = dc.Invert(NoiseMatrix);
    if (InvertStatus == false)  B2INFO("eclAutocovarianceCalibrationC3Algorithm ID InvertStatus [0][0] totalCounts: " << ID <<
                                         " " << InvertStatus << " " << NoiseMatrix(0, 0) << " " << totalCounts);
    cryIDs.push_back(ID + 1);
    invertStatusVector.push_back(InvertStatus);
    noiseMatrix00Vector.push_back(NoiseMatrix(0, 0));
    totalCountsVector.push_back(totalCounts);

  }

  /** Saving Calibration Results */
  saveCalibration(Autocovariances, "ECLAutocovarianceCalibrationC3Autocovariances");

  /** Preparing TGraphs for output file */
  auto ginvertStatusVector = new TGraph(cryIDs.size(), cryIDs.data(), invertStatusVector.data());
  ginvertStatusVector->SetName("ginvertStatusVector");
  ginvertStatusVector->SetMarkerStyle(20);
  auto gnoiseMatrix00Vector = new TGraph(cryIDs.size(), cryIDs.data(), noiseMatrix00Vector.data());
  gnoiseMatrix00Vector->SetName("gnoiseMatrix00Vector");
  gnoiseMatrix00Vector->SetMarkerStyle(20);
  auto gtotalCountsVector = new TGraph(cryIDs.size(), cryIDs.data(), totalCountsVector.data());
  gtotalCountsVector->SetName("gtotalCountsVector");
  gtotalCountsVector->SetMarkerStyle(20);

  /** Write out the basic histograms in all cases */
  TString fName = m_outputName;
  TDirectory::TContext context;
  TFile* histfile = new TFile(fName, "recreate");
  histfile->cd();
  ginvertStatusVector->Write();
  gnoiseMatrix00Vector->Write();
  gtotalCountsVector->Write();
  histfile->Close();
  delete histfile;

  return c_OK;
}
