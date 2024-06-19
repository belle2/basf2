/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//This module`
#include <ecl/modules/eclAutocovarianceCalibrationC3Collector/eclAutocovarianceCalibrationC3Collector.h>

#include <iostream>
//Framework
#include <framework/dataobjects/EventMetaData.h>

//ECL
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Modules
//-----------------------------------------------------------------
REG_MODULE(eclAutocovarianceCalibrationC3Collector);
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// constructor
eclAutocovarianceCalibrationC3CollectorModule::eclAutocovarianceCalibrationC3CollectorModule() : CalibrationCollectorModule(),
  m_ECLAutocovarianceCalibrationC1Threshold("ECLAutocovarianceCalibrationC1Threshold")
{
  // Set module properties
  setDescription("Module to compute covarience matrix using waveforms from delayed Bhabha events");
  setPropertyFlags(c_ParallelProcessingCertified);
}

void eclAutocovarianceCalibrationC3CollectorModule::prepare()
{

  /**----------------------------------------------------------------------------------------*/
  /** Create the histograms and register them in the data store */
  CovarianceMatrixInfoVsCrysID = new TH2F("CovarianceMatrixInfoVsCrysID", "", ECLElementNumbers::c_NCrystals, 0,
                                          ECLElementNumbers::c_NCrystals, m_nADCWaveformPoints + 1, 0, m_nADCWaveformPoints + 1);

  registerObject<TH2F>("CovarianceMatrixInfoVsCrysID", CovarianceMatrixInfoVsCrysID);

  m_eclDsps.isRequired();

}

void eclAutocovarianceCalibrationC3CollectorModule::startRun()
{
  /**----------------------------------------------------------------------------------------*/
  B2INFO("eclAutocovarianceCalibrationC3Collector: Experiment = " << m_evtMetaData->getExperiment() << "  run = " <<
         m_evtMetaData->getRun());

  m_PeakToPeakThresholds = m_ECLAutocovarianceCalibrationC1Threshold->getCalibVector();

}


void eclAutocovarianceCalibrationC3CollectorModule::collect()
{

  std::vector<float> m_tempArray; /**< temp vector to store baseline subracted waveform */

  const int NumDsp = m_eclDsps.getEntries();

  //Random Trigger Events have waveform for each crystal
  if (NumDsp == ECLElementNumbers::c_NCrystals) {

    for (auto& aECLDsp : m_eclDsps) {

      const int id = aECLDsp.getCellId() - 1;

      //Peak to peak amplitude used to gauge noise level
      float PeakToPeak = (float) aECLDsp.computePeaktoPeakAmp();

      if (PeakToPeak < m_PeakToPeakThresholds[id]) {

        float baseline = 0;
        for (int i = 0; i < m_BaselineLimit; i++) baseline += aECLDsp.getDspA()[i];
        baseline = baseline * (1. / ((float) m_BaselineLimit));

        // Computing baseline subtracted waveform
        m_tempArray.clear();
        for (int i = 0; i < m_nADCWaveformPoints; i++) m_tempArray.push_back(aECLDsp.getDspA()[i] - baseline);

        /*
        Info on convariance matrix calucation:

        We store only nADCWaveformPoints numbers because there are several symmetries:
        -  In general the matrix is nADCWaveformPointsxnADCWaveformPoints, however, the matrix symmetric so we only compute the upper part of the matrix.
        -  We also assume the noise is random in time.  With this assumption the values of each matrix diagonal will be the same.   This allows us to define the matrix with only nADCWaveformPoints numbers: One for each diagonal (starting with the main diagonal)
           I divide by (nADCWaveformPoints - (i-j)) to average over the number of off-diagonal elements for each row.
           Using the final nADCWaveformPoints numbers the full nADCWaveformPointsxnADCWaveformPoints matrix in constructed by:
           Element 0 is the nADCWaveformPoints main diagonals
           Element 1 is the value of the 30 off diagonals
        */

        for (int i = 0; i < m_nADCWaveformPoints; i++) {

          float value_i = m_tempArray[i];

          for (int j = i; j < m_nADCWaveformPoints; j++) {

            int tempIndex = abs(i - j);

            m_CovarianceMatrixInfoVsCrysIDHistogram[id][tempIndex] += (value_i * m_tempArray[j]);

          }
        }
        m_CovarianceMatrixInfoVsCrysIDHistogram[id][m_nADCWaveformPoints]++;
      }
    }
  }

}

void eclAutocovarianceCalibrationC3CollectorModule::closeRun()
{
  for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++) {
    for (int j = 0; j < (m_nADCWaveformPoints + 1); j++) {
      getObjectPtr<TH2>("CovarianceMatrixInfoVsCrysID")->SetBinContent(i + 1, j + 1, m_CovarianceMatrixInfoVsCrysIDHistogram[i][j]);
    }
  }
}
