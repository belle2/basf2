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

using namespace std;
using namespace Belle2;
using namespace ECL;

/**-----------------------------------------------------------------------------------------------*/
eclAutocovarianceCalibrationC3Algorithm::eclAutocovarianceCalibrationC3Algorithm():
  CalibrationAlgorithm("eclAutocovarianceCalibrationC3Collector")
{
  setDescription(
    "Perform energy calibration of ecl crystals by fitting a Novosibirsk function to energy deposited by photons in e+e- --> gamma gamma"
  );
}

CalibrationAlgorithm::EResult eclAutocovarianceCalibrationC3Algorithm::calibrate()
{


  ///** Put root into batch mode so that we don't try to open a graphics window */
  gROOT->SetBatch();

  int m_CountLimit = 1000;

  ///**-----------------------------------------------------------------------------------------------*/
  ///** Histogram containing the data collected by eclAutocovarianceCalibrationC3Collector*/
  auto CovarianceMatrixInfoVsCrysID = getObjectPtr<TH2F>("CovarianceMatrixInfoVsCrysID");

  ECLAutoCovariance* Autocovariances = new ECLAutoCovariance();

  for (int crysID = 0; crysID < ECLElementNumbers::c_NCrystals; crysID++) {

    float totalCounts = CovarianceMatrixInfoVsCrysID->GetBinContent(CovarianceMatrixInfoVsCrysID->GetBin(crysID + 1, 32));

    if (totalCounts < m_CountLimit) {
      B2INFO("eclAutocovarianceCalibrationC3Algorithm: Warning Below Count Limit: crysID totalCounts m_CountLimit: " << crysID << " " <<
             totalCounts << " " << m_CountLimit);
    }

    TMatrixDSym NoiseMatrix;
    NoiseMatrix.ResizeTo(31, 31);
    for (int i = 0; i < 31; i++) {
      for (int j = 0; j < 31; j++) {
        int index = abs(i - j);
        NoiseMatrix(i, j) = float(CovarianceMatrixInfoVsCrysID->GetBinContent(CovarianceMatrixInfoVsCrysID->GetBin(crysID + 1,
                                  index + 1))) / totalCounts / (float(31.0 - index));
      }
    }

    double tempAutoCov[31];

    for (int i = 0; i < 31; i++) tempAutoCov[i] = NoiseMatrix(0, i);
    Autocovariances->setAutoCovariance(crysID + 1, tempAutoCov);

    TDecompChol dc(NoiseMatrix);
    bool InvertStatus = dc.Invert(NoiseMatrix);
    if (InvertStatus == false)  B2INFO("eclAutocovarianceCalibrationC3Algorithm crysID InvertStatus [0][0] totalCounts: " << crysID <<
                                         " " << InvertStatus << " " << NoiseMatrix(0, 0) << " " << totalCounts);

  }

  /** Saving Calibration Results */
  saveCalibration(Autocovariances, "ECLAutocovarianceCalibrationC3Autocovariances");

  return c_OK;
}
