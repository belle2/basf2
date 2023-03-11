/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//This module`
#include <ecl/modules/eclAutocovarianceCalibrationC2Collector/eclAutocovarianceCalibrationC2Collector.h>

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
REG_MODULE(eclAutocovarianceCalibrationC2Collector)
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// constructor
eclAutocovarianceCalibrationC2CollectorModule::eclAutocovarianceCalibrationC2CollectorModule() : CalibrationCollectorModule(),
  m_ECLAutocovarianceCalibrationC1Threshold("ECLAutocovarianceCalibrationC1Threshold")
{
  // Set module properties
  setDescription("Module to export histogram of noise in waveforms from random trigger events");
  setPropertyFlags(c_ParallelProcessingCertified);
  //addParam("BaselineLimit", m_BaselineLimit, "Number of waveforms required to compute baseline", 10000);
}

void eclAutocovarianceCalibrationC2CollectorModule::prepare()
{

  /**----------------------------------------------------------------------------------------*/
  B2INFO("eclAutocovarianceCalibrationC2Collector: Experiment = " << m_evtMetaData->getExperiment() << "  run = " <<
         m_evtMetaData->getRun());

  /**----------------------------------------------------------------------------------------*/
  /** Create the histograms and register them in the data store */
  auto BaselineInfoVsCrysID = new TH2F("BaselineInfoVsCrysID", "Baseline for each crystal;crystal ID;Baseline (ADC)", 8736, 0, 8736,
                                       32, 0, 32);
  registerObject<TH2F>("BaselineInfoVsCrysID", BaselineInfoVsCrysID);

  m_PeakToPeakThresholds = m_ECLAutocovarianceCalibrationC1Threshold->getCalibVector();

  for (int i = 0; i < 8736; i++) {

    B2INFO("i m_PeakToPeakThresholds " << i << " " << m_PeakToPeakThresholds[i]);
  }

  m_eclDsps.registerInDataStore();
  m_eclDigits.registerInDataStore();

}


void eclAutocovarianceCalibrationC2CollectorModule::collect()
{

  const int NumDsp = m_eclDsps.getEntries();

  //Random Trigger Event
  if (NumDsp == 8736) {

    //B2INFO(NumDsp);

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
          float currentValue = getObjectPtr<TH2>("BaselineInfoVsCrysID")->GetBinContent(id + 1, i + 1);
          getObjectPtr<TH2>("BaselineInfoVsCrysID")->SetBinContent(id + 1, i + 1, currentValue + aECLDsp.getDspA()[i]);
        }
        float countValue = getObjectPtr<TH2>("BaselineInfoVsCrysID")->GetBinContent(id + 1, 32);
        getObjectPtr<TH2>("BaselineInfoVsCrysID")->SetBinContent(id + 1, 32, countValue + 1.0);
      }

      //if (m_counter[id] == m_BaselineLimit) {

      //  float baseline = 0.0;

      //  for (int i = 0; i < 31; i++)  baseline += m_Baseline[id][i];
      //  baseline /= (float)m_counter[id];

      //  getObjectPtr<TH1F>("BaselineVsCrysID")->SetBinContent(id + 1, baseline);

      //  m_BaselineComputed[id] = true;

      //  B2INFO(id << " " << baseline);

    }
  }
}
