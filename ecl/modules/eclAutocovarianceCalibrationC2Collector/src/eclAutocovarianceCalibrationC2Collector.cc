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
  m_BaselineVsCrysID = new TH1D("m_BaselineVsCrysID", "Baseline for each crystal;crystal ID;Baseline (ADC)",
                                ECLElementNumbers::c_NCrystals, 0, ECLElementNumbers::c_NCrystals);
  registerObject<TH1D>("m_BaselineVsCrysID", m_BaselineVsCrysID);
  m_CounterVsCrysID = new TH1D("m_CounterVsCrysID", "Baseline for each crystal;crystal ID;Baseline (ADC)",
                               ECLElementNumbers::c_NCrystals, 0, ECLElementNumbers::c_NCrystals);
  registerObject<TH1D>("m_CounterVsCrysID", m_CounterVsCrysID);

  m_PeakToPeakThresholds = m_ECLAutocovarianceCalibrationC1Threshold->getCalibVector();

  m_eclDsps.registerInDataStore();

}


void eclAutocovarianceCalibrationC2CollectorModule::collect()
{

  const int NumDsp = m_eclDsps.getEntries();

  //Random Trigger Events have waveform for each crystal
  if (NumDsp == ECLElementNumbers::c_NCrystals) {

    for (auto& aECLDsp : m_eclDsps) {

      const int id = aECLDsp.getCellId() - 1;

      float peakToPeakThreshold =  m_PeakToPeakThresholds[id];

      //Peak to peak amplitude used to gauge noise level
      float PeakToPeak = (float) aECLDsp.computePeaktoPeakAmp();

      if (PeakToPeak < peakToPeakThreshold) {

        for (int i = 0; i < m_numberofADCPoints; i++) m_sumOfSamples[id] += aECLDsp.getDspA()[i];
        m_nSelectedWaveforms[id]++;

      }

    }
  }
}

void eclAutocovarianceCalibrationC2CollectorModule::closeRun()
{
  for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++) {
    B2INFO(i << " " << getObjectPtr<TH1>("m_BaselineVsCrysID")->GetBinContent(i + 1));
    getObjectPtr<TH1D>("m_BaselineVsCrysID")->SetBinContent(i + 1, m_sumOfSamples[i]);
    getObjectPtr<TH1D>("m_CounterVsCrysID")->SetBinContent(i + 1,  m_nSelectedWaveforms[i]);
  }
}
