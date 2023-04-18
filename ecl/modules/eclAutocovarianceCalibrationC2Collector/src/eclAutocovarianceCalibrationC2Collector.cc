/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//This module`
#include <ecl/modules/eclAutocovarianceCalibrationC2Collector/eclAutocovarianceCalibrationC2Collector.h>

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
REG_MODULE(eclAutocovarianceCalibrationC2Collector);
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// constructor
eclAutocovarianceCalibrationC2CollectorModule::eclAutocovarianceCalibrationC2CollectorModule() : CalibrationCollectorModule(),
  m_ECLAutocovarianceCalibrationC1Threshold("ECLAutocovarianceCalibrationC1Threshold")
{
  // Set module properties
  setDescription("Module to export histogram corresponding to sum of waveforms from random trigger events. Used to compute baseline");
  setPropertyFlags(c_ParallelProcessingCertified);
}

void eclAutocovarianceCalibrationC2CollectorModule::prepare()
{

  /**----------------------------------------------------------------------------------------*/
  B2INFO("eclAutocovarianceCalibrationC2Collector: Experiment = " << m_evtMetaData->getExperiment() << "  run = " <<
         m_evtMetaData->getRun());

  /**----------------------------------------------------------------------------------------*/
  /** Create the histograms and register them in the data store */
  m_BaselineInfoVsCrysID = new TH2F("BaselineInfoVsCrysID", "Baseline for each crystal;crystal ID;Baseline (ADC)",
                                    ECLElementNumbers::c_NCrystals, 0, ECLElementNumbers::c_NCrystals,
                                    32, 0, 32);
  registerObject<TH2F>("m_BaselineInfoVsCrysID", m_BaselineInfoVsCrysID);

  m_PeakToPeakThresholds = m_ECLAutocovarianceCalibrationC1Threshold->getCalibVector();

  for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++) {

    for (int j = 0; j < 32; j++) myHist[i][j] = 0.0;

    B2INFO("i m_PeakToPeakThresholds " << i << " " << m_PeakToPeakThresholds[i]);
  }

  m_eclDsps.registerInDataStore();

}


void eclAutocovarianceCalibrationC2CollectorModule::collect()
{

  const int NumDsp = m_eclDsps.getEntries();

  //Random Trigger Events have waveform for each crystal
  if (NumDsp == ECLElementNumbers::c_NCrystals) {

    for (auto& aECLDsp : m_eclDsps) {

      const int id = aECLDsp.getCellId() - 1;

      //Peak to peak amplitude used to gauge noise level
      float PeakToPeak = (float) aECLDsp.computePeaktoPeakAmp();

      if (PeakToPeak < m_PeakToPeakThresholds[id]) {

        for (int i = 0; i < 31; i++) myHist[id][i] += aECLDsp.getDspA()[i];

        myHist[id][31]++;

      }

    }
  }
}

void eclAutocovarianceCalibrationC2CollectorModule::closeRun()
{
  for (int i = 0; i < 8736; i++) {
    for (int j = 0; j < 32; j++) {
      getObjectPtr<TH2>("m_BaselineInfoVsCrysID")->SetBinContent(i + 1, j + 1, myHist[i][j]);
      B2INFO(i << " " << j << " " << getObjectPtr<TH2>("m_BaselineInfoVsCrysID")->GetBinContent(i + 1, j + 1));
    }
  }
}
