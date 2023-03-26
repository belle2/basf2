/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//This module`
#include <ecl/modules/eclWaveformTemplateCalibrationC2Collector/eclWaveformTemplateCalibrationC2Collector.h>

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
REG_MODULE(eclWaveformTemplateCalibrationC2Collector)
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// constructor
eclWaveformTemplateCalibrationC2CollectorModule::eclWaveformTemplateCalibrationC2CollectorModule() : CalibrationCollectorModule(),
  m_eclWaveformTemplateCalibrationC1VarSq("eclWaveformTemplateCalibrationC1VarSq")
{
  // Set module properties
  setDescription("Module to export histogram of noise in waveforms from random trigger events");
  addParam("MinEnergyThreshold", m_MinEnergyThreshold, "Energy threshold of online fit result for Fitting Waveforms (GeV).", 2.0);
  addParam("MaxEnergyThreshold", m_MaxEnergyThreshold, "Energy threshold of online fit result for Fitting Waveforms (GeV).", 4.0);
  addParam("MinCellID", m_MinCellID, "", 0);
  addParam("MaxCellID", m_MaxCellID, "", 0);
  setPropertyFlags(c_ParallelProcessingCertified);
}

void eclWaveformTemplateCalibrationC2CollectorModule::prepare()
{

  /**----------------------------------------------------------------------------------------*/
  B2INFO("eclWaveformTemplateCalibrationC2Collector: Experiment = " << m_evtMetaData->getExperiment() << "  run = " <<
         m_evtMetaData->getRun());

  /**----------------------------------------------------------------------------------------*/
  /** Create the histograms and register them in the data store */
  varXvsCrysID = new TH2F("varXvsCrysID", "", 8736, 0, 8736, 1000, 0, 1000);
  registerObject<TH2F>("varXvsCrysID", varXvsCrysID);

  m_eclDsps.registerInDataStore();
  m_eclDigits.registerInDataStore();

  m_MaxVarX = m_eclWaveformTemplateCalibrationC1VarSq->getCalibVector();


  m_ADCtoEnergy.resize(ECLElementNumbers::c_NCrystals);
  if (m_CrystalElectronics.isValid()) {
    for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++)
      m_ADCtoEnergy[i] = m_CrystalElectronics->getCalibVector()[i];
  }
  if (m_CrystalEnergy.isValid()) {
    for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++)
      m_ADCtoEnergy[i] *= m_CrystalEnergy->getCalibVector()[i];
  }


  auto tree = new TTree("tree", "");
  tree->Branch("CellID", &m_CellID,      "m_CellID/I");
  tree->Branch("ADC0", &m_ADC0,      "m_ADC0/I");
  tree->Branch("ADC1", &m_ADC1,      "m_ADC1/I");
  tree->Branch("ADC2", &m_ADC2,      "m_ADC2/I");
  tree->Branch("ADC3", &m_ADC3,      "m_ADC3/I");
  tree->Branch("ADC4", &m_ADC4,      "m_ADC4/I");
  tree->Branch("ADC5", &m_ADC5,      "m_ADC5/I");
  tree->Branch("ADC6", &m_ADC6,      "m_ADC6/I");
  tree->Branch("ADC7", &m_ADC7,      "m_ADC7/I");
  tree->Branch("ADC8", &m_ADC8,      "m_ADC8/I");
  tree->Branch("ADC9", &m_ADC9,      "m_ADC9/I");
  tree->Branch("ADC10", &m_ADC10,      "m_ADC10/I");
  tree->Branch("ADC11", &m_ADC11,      "m_ADC11/I");
  tree->Branch("ADC12", &m_ADC12,      "m_ADC12/I");
  tree->Branch("ADC13", &m_ADC13,      "m_ADC13/I");
  tree->Branch("ADC14", &m_ADC14,      "m_ADC14/I");
  tree->Branch("ADC15", &m_ADC15,      "m_ADC15/I");
  tree->Branch("ADC16", &m_ADC16,      "m_ADC16/I");
  tree->Branch("ADC17", &m_ADC17,      "m_ADC17/I");
  tree->Branch("ADC18", &m_ADC18,      "m_ADC18/I");
  tree->Branch("ADC19", &m_ADC19,      "m_ADC19/I");
  tree->Branch("ADC20", &m_ADC20,      "m_ADC20/I");
  tree->Branch("ADC21", &m_ADC21,      "m_ADC21/I");
  tree->Branch("ADC22", &m_ADC22,      "m_ADC22/I");
  tree->Branch("ADC23", &m_ADC23,      "m_ADC23/I");
  tree->Branch("ADC24", &m_ADC24,      "m_ADC24/I");
  tree->Branch("ADC25", &m_ADC25,      "m_ADC25/I");
  tree->Branch("ADC26", &m_ADC26,      "m_ADC26/I");
  tree->Branch("ADC27", &m_ADC27,      "m_ADC27/I");
  tree->Branch("ADC28", &m_ADC28,      "m_ADC28/I");
  tree->Branch("ADC29", &m_ADC29,      "m_ADC29/I");
  tree->Branch("ADC30", &m_ADC30,      "m_ADC30/I");
  registerObject<TTree>("tree", tree);

}


void eclWaveformTemplateCalibrationC2CollectorModule::collect()
{

  int m_baselineLimit = 12;

  for (auto& aECLDsp : m_eclDsps) {

    const int id = aECLDsp.getCellId() - 1;

    if ((id + 1) < m_MinCellID)  continue;
    if ((id + 1) > m_MaxCellID)  continue;

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

    double energy = d->getAmp() * m_ADCtoEnergy[id];

    if (energy < m_MinEnergyThreshold)  continue;
    if (energy > m_MaxEnergyThreshold)  continue;

    float baseline = 0.0;
    for (int i = 0; i < m_baselineLimit; i++) baseline += aECLDsp.getDspA()[i];
    baseline /= ((float) m_baselineLimit);

    bool skipWaveform = false;
    for (int i = 0; i < 31; i++) {
      if (aECLDsp.getDspA()[i] < 10) skipWaveform = true;
    }
    if (skipWaveform) continue;

    std::vector<float> baselineSubtracted(m_baselineLimit);
    for (int i = 0; i < m_baselineLimit; i++) baselineSubtracted[i] = (aECLDsp.getDspA()[i] - baseline);

    float varX = 0.0;
    for (int i = 0; i < m_baselineLimit; i++) varX += (baselineSubtracted[i] * baselineSubtracted[i]);
    varX /= m_baselineLimit - 1;

    if (varX > m_MaxVarX[id])  continue;
    // save to tree

    m_CellID = id + 1;

    m_ADC0 = aECLDsp.getDspA()[0];
    m_ADC1 = aECLDsp.getDspA()[1];
    m_ADC2 = aECLDsp.getDspA()[2];
    m_ADC3 = aECLDsp.getDspA()[3];
    m_ADC4 = aECLDsp.getDspA()[4];
    m_ADC5 = aECLDsp.getDspA()[5];
    m_ADC6 = aECLDsp.getDspA()[6];
    m_ADC7 = aECLDsp.getDspA()[7];
    m_ADC8 = aECLDsp.getDspA()[8];
    m_ADC9 = aECLDsp.getDspA()[9];
    m_ADC10 = aECLDsp.getDspA()[10];
    m_ADC11 = aECLDsp.getDspA()[11];
    m_ADC12 = aECLDsp.getDspA()[12];
    m_ADC13 = aECLDsp.getDspA()[13];
    m_ADC14 = aECLDsp.getDspA()[14];
    m_ADC15 = aECLDsp.getDspA()[15];
    m_ADC16 = aECLDsp.getDspA()[16];
    m_ADC17 = aECLDsp.getDspA()[17];
    m_ADC18 = aECLDsp.getDspA()[18];
    m_ADC19 = aECLDsp.getDspA()[19];
    m_ADC20 = aECLDsp.getDspA()[20];
    m_ADC21 = aECLDsp.getDspA()[21];
    m_ADC22 = aECLDsp.getDspA()[22];
    m_ADC23 = aECLDsp.getDspA()[23];
    m_ADC24 = aECLDsp.getDspA()[24];
    m_ADC25 = aECLDsp.getDspA()[25];
    m_ADC26 = aECLDsp.getDspA()[26];
    m_ADC27 = aECLDsp.getDspA()[27];
    m_ADC28 = aECLDsp.getDspA()[28];
    m_ADC29 = aECLDsp.getDspA()[29];
    m_ADC30 = aECLDsp.getDspA()[30];

    getObjectPtr<TTree>("tree")->Fill();

  }
}
