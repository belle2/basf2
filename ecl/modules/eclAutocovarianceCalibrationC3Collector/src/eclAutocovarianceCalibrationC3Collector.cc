/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//This module`
#include <ecl/modules/eclAutocovarianceCalibrationC3Collector/eclAutocovarianceCalibrationC3Collector.h>

//Root
#include <TH2F.h>

//Framework
#include <framework/dataobjects/EventMetaData.h>

//ECL
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Modules
//-----------------------------------------------------------------
REG_MODULE(eclAutocovarianceCalibrationC3Collector)
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// constructor
eclAutocovarianceCalibrationC3CollectorModule::eclAutocovarianceCalibrationC3CollectorModule() : CalibrationCollectorModule(),
  m_ECLAutocovarianceCalibrationC1Threshold("ECLAutocovarianceCalibrationC1Threshold"),
  m_ECLAutocovarianceCalibrationC2Baseline("ECLAutocovarianceCalibrationC2Baseline")
{
  // Set module properties
  setDescription("Module to export histogram of noise in waveforms from random trigger events");
  setPropertyFlags(c_ParallelProcessingCertified);
}

void eclAutocovarianceCalibrationC3CollectorModule::prepare()
{

  /**----------------------------------------------------------------------------------------*/
  B2INFO("eclAutocovarianceCalibrationC3Collector: Experiment = " << m_evtMetaData->getExperiment() << "  run = " <<
         m_evtMetaData->getRun());

  /**----------------------------------------------------------------------------------------*/
  /** Create the histograms and register them in the data store */
  auto CovarianceMatrixInfoVsCrysID = new TH2F("CovarianceMatrixInfoVsCrysID", "", 8736, 0, 8736, 32, 0, 32);
  registerObject<TH2F>("CovarianceMatrixInfoVsCrysID", CovarianceMatrixInfoVsCrysID);

  m_PeakToPeakThresholds = m_ECLAutocovarianceCalibrationC1Threshold->getCalibVector();

  m_Baselines = m_ECLAutocovarianceCalibrationC2Baseline->getCalibVector();

  m_eclDsps.registerInDataStore();
  m_eclDigits.registerInDataStore();

}


void eclAutocovarianceCalibrationC3CollectorModule::collect()
{

  const int NumDsp = m_eclDsps.getEntries();

  //Random Trigger Event
  if (NumDsp == 8736) {

    for (auto& aECLDsp : m_eclDsps) {

      const int id = aECLDsp.getCellId() - 1;

      int minADC = aECLDsp.getDspA()[0];
      int maxADC = aECLDsp.getDspA()[0];

      for (int i = 0; i < 31; i++) {

        if (aECLDsp.getDspA()[i] < minADC) minADC = aECLDsp.getDspA()[i];
        if (aECLDsp.getDspA()[i] > maxADC) maxADC = aECLDsp.getDspA()[i];

      }

      int PeakToPeak = maxADC - minADC;

      if (PeakToPeak < m_PeakToPeakThresholds[id]) {

        for (int i = 0; i < 31; i++) {

          float value_i = aECLDsp.getDspA()[i] - m_Baselines[id];

          for (int j = 0; j < 31; j++) {

            int tempIndex = abs(i - j);
            float value_j = aECLDsp.getDspA()[j] - m_Baselines[id];

            float currentValue = getObjectPtr<TH2>("CovarianceMatrixInfoVsCrysID")->GetBinContent(id + 1, tempIndex + 1);

            getObjectPtr<TH2>("CovarianceMatrixInfoVsCrysID")->SetBinContent(id + 1, tempIndex + 1, currentValue + (value_i * value_j));

          }
        }
        float countValue = getObjectPtr<TH2>("CovarianceMatrixInfoVsCrysID")->GetBinContent(id + 1, 32);
        getObjectPtr<TH2>("CovarianceMatrixInfoVsCrysID")->SetBinContent(id + 1, 32, countValue + 1.0);
      }
    }
  }
}
