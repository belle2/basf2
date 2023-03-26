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
REG_MODULE(eclWaveformTemplateCalibrationC1Collector)
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
  setPropertyFlags(c_ParallelProcessingCertified);
}

void eclWaveformTemplateCalibrationC1CollectorModule::prepare()
{

  /**----------------------------------------------------------------------------------------*/
  B2INFO("eclWaveformTemplateCalibrationC1Collector: Experiment = " << m_evtMetaData->getExperiment() << "  run = " <<
         m_evtMetaData->getRun());

  /**----------------------------------------------------------------------------------------*/
  /** Create the histograms and register them in the data store */
  varXvsCrysID = new TH2F("varXvsCrysID", "", 8736, 0, 8736, 1000, 0, 1000);
  registerObject<TH2F>("varXvsCrysID", varXvsCrysID);

  m_eclDsps.registerInDataStore();
  m_eclDigits.registerInDataStore();

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

    //setting relation of eclDSP to aECLDigit
    const ECLDigit* d = nullptr;
    for (const auto& aECLDigit : m_eclDigits) {
      if (aECLDigit.getCellId() - 1 == id) {
        d = &aECLDigit;
        aECLDsp.addRelationTo(&aECLDigit);
        break;
      }
    }
    if (d == nullptr) continue;

    //estimating crystal energy
    double energy = d->getAmp() * m_ADCtoEnergy[id];

    // only select high energy crystals
    if (energy < m_MinEnergyThreshold)  continue;
    if (energy > m_MaxEnergyThreshold)  continue;

    //compute mean of baseline
    float baseline = 0.0;
    for (int i = 0; i < m_baselineLimit; i++) baseline += aECLDsp.getDspA()[i];
    baseline /= ((float) m_baselineLimit);

    //compute baseline values minus baseline mean
    std::vector<float> baselineSubtracted(m_baselineLimit);
    for (int i = 0; i < m_baselineLimit; i++) baselineSubtracted[i] = (aECLDsp.getDspA()[i] - baseline);

    //compute varience of baseline
    float varX = 0.0;
    for (int i = 0; i < m_baselineLimit; i++) varX += (baselineSubtracted[i] * baselineSubtracted[i]);
    varX /= m_baselineLimit - 1;

    //save result to histogram
    varXvsCrysID->Fill(id, varX);

  }

}

void eclWaveformTemplateCalibrationC1CollectorModule::closeRun()
{
  for (int i = 0; i < 8736; i++) {
    for (int j = 0; j < 1000; j++) {
      getObjectPtr<TH2>("varXvsCrysID")->SetBinContent(i + 1, j + 1, varXvsCrysID->GetBinContent(i + 1, j + 1));
    }
  }
}
