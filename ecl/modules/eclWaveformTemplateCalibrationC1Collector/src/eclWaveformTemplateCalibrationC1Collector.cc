/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//This module`
#include <ecl/modules/eclWaveformTemplateCalibrationC1Collector/eclWaveformTemplateCalibrationC1Collector.h>

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
REG_MODULE(eclWaveformTemplateCalibrationC1Collector);
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// constructor
eclWaveformTemplateCalibrationC1CollectorModule::eclWaveformTemplateCalibrationC1CollectorModule()
{
  // Set module properties
  setDescription("Module to export histogram of baseline noise of energetic waveforms from ee->gg events.  First step in photon template calculation.");
  addParam("MinEnergyThreshold", m_MinEnergyThreshold, "Minimum energy threshold of online fit result for Fitting Waveforms (GeV).",
           2.0);
  addParam("MaxEnergyThreshold", m_MaxEnergyThreshold, "Maximum energy threshold of online fit result for Fitting Waveforms (GeV).",
           4.0);
  addParam("baselineLimit", m_baselineLimit, "Number of ADC points used to define baseline.", 12);
  addParam("maxResvsCrysIDHistogramLimit", m_maxResvsCrysIDHistogramLimit, "upper limit of histogram.", 100);
  addParam("maxResvsCrysIDHistogramNBins", m_maxResvsCrysIDHistogramNBins, "histogram number of bins.", 1000);
  addParam("ADCFloorThreshold", m_ADCFloorThreshold, "Used to determine if waveform hit ADC floor", 10);
  setPropertyFlags(c_ParallelProcessingCertified);
}

void eclWaveformTemplateCalibrationC1CollectorModule::prepare()
{

  /**----------------------------------------------------------------------------------------*/
  /** Create the histograms and register them in the data store */
  maxResvsCrysID = new TH2F("maxResvsCrysID", "", ECLElementNumbers::c_NCrystals, 0, ECLElementNumbers::c_NCrystals,
                            m_maxResvsCrysIDHistogramNBins, 0, m_maxResvsCrysIDHistogramLimit);
  registerObject<TH2F>("maxResvsCrysID", maxResvsCrysID);

  m_eclDsps.isRequired();
  m_eclDigits.isRequired();

}

void eclWaveformTemplateCalibrationC1CollectorModule::startRun()
{
  /**----------------------------------------------------------------------------------------*/
  B2INFO("eclWaveformTemplateCalibrationC1Collector: Experiment = " << m_evtMetaData->getExperiment() << "  run = " <<
         m_evtMetaData->getRun());

  // Loading crystal calibration constants from database
  m_ADCtoEnergy.resize(ECLElementNumbers::c_NCrystals);
  if (m_CrystalElectronics.isValid()) {
    for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++)
      m_ADCtoEnergy[i] = m_CrystalElectronics->getCalibVector()[i];
  }
  if (m_CrystalEnergy.isValid()) {
    for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++)
      m_ADCtoEnergy[i] *= m_CrystalEnergy->getCalibVector()[i];
  }

}


void eclWaveformTemplateCalibrationC1CollectorModule::collect()
{

  for (auto& aECLDsp : m_eclDsps) {

    const int id = aECLDsp.getCellId() - 1;

    //estimating crystal energy
    double energy = 0.0;
    for (const auto& aECLDigit : m_eclDigits) {
      if (aECLDigit.getCellId() - 1 == id) {
        energy = aECLDigit.getAmp() *  m_ADCtoEnergy[id];
        break;
      }
    }

    // only select high energy crystals
    if (energy < m_MinEnergyThreshold)  continue;
    // avoid very high energy crystals due to potential photodiode interactions
    if (energy > m_MaxEnergyThreshold)  continue;

    // check if waveform has point below m_ADCFloorThreshold (indicates waveform hit ADC floor)
    bool skipWaveform = false;
    for (int i = 0; i < m_numberofADCPoints; i++) {
      if (aECLDsp.getDspA()[i] < m_ADCFloorThreshold) skipWaveform = true;
    }
    if (skipWaveform) continue;

    //compute mean of baseline
    float baseline = 0.0;
    for (int i = 0; i < m_baselineLimit; i++) baseline += aECLDsp.getDspA()[i];
    baseline /= ((float) m_baselineLimit);

    //compute max residual in baseline
    float maxRes = 0.0;
    for (int i = 0; i < m_baselineLimit; i++) {
      float temp = fabs(aECLDsp.getDspA()[i] - baseline);
      if (temp > maxRes)  maxRes = temp;
    }

    //save result to histogram
    maxResvsCrysID->Fill(id, maxRes);

  }

}

void eclWaveformTemplateCalibrationC1CollectorModule::closeRun()
{
  for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++) {
    for (int j = 0; j < m_maxResvsCrysIDHistogramNBins; j++) {
      getObjectPtr<TH2>("maxResvsCrysID")->SetBinContent(i + 1, j + 1, maxResvsCrysID->GetBinContent(i + 1, j + 1));
    }
  }
}
